// SPDX-FileCopyrightText: (c) 2021-2024 Shawn Silverman <shawn@pobox.com>
// SPDX-License-Identifier: AGPL-3.0-or-later

// QNEthernet.cpp contains the Teensy 4.1 Ethernet implementation.
// This file is part of the QNEthernet library.

#include "QNEthernet.h"

// C++ includes
#include <algorithm>
#include <cstdlib>

#if defined(__has_include) && __has_include(<EventResponder.h>)
#include <EventResponder.h>
#endif  // defined(__has_include) && __has_include(<EventResponder.h>)

#include "QNDNSClient.h"
#include "adapters/funcs.h"
#include "adapters/pgmspace.h"
#include "lwip/dhcp.h"
#include "lwip/igmp.h"

namespace qindesign {
namespace network {

// A reference to the singleton.
STATIC_INIT_DEFN(EthernetClass, Ethernet);

#if defined(__has_include) && __has_include(<EventResponder.h>)
// Global definitions for Arduino
static EventResponder ethLoop;
static bool loopAttached = false;

// Attach the loop() call to yield() via EventResponder.
static void attachLoopToYield() {
  if (loopAttached) {
    return;
  }
  loopAttached = true;
  ethLoop.attach([](EventResponderRef r) {
    Ethernet.loop();
    r.triggerEvent();
  });
  ethLoop.triggerEvent();
}
#else
#error "Need to hook into or replace yield()"
// Example yield() implementation:
// void yield() {
//   static bool busy = false;  // Prevent reentrancy
//   if (!busy && Ethernet) {
//     busy = true;
//     Ethernet.loop();
//     busy = false;
//   }
// }
#endif  // defined(__has_include) && __has_include(<EventResponder.h>)

void EthernetClass::netifEventFunc(struct netif *netif,
                                   netif_nsc_reason_t reason,
                                   const netif_ext_callback_args_t *args) {
  if (netif != Ethernet.netif_) {
    return;
  }

  if (reason & LWIP_NSC_LINK_CHANGED) {
    if (Ethernet.linkStateCB_ != nullptr && args != nullptr) {
      Ethernet.linkStateCB_(args->link_changed.state != 0);
    }
  }

  if (reason & LWIP_NSC_IPV4_SETTINGS_CHANGED) {
    if (Ethernet.addressChangedCB_ != nullptr) {
      Ethernet.addressChangedCB_();
    }
  }

  if (reason & LWIP_NSC_STATUS_CHANGED) {
    if (Ethernet.interfaceStatusCB_ != nullptr && args != nullptr) {
      Ethernet.interfaceStatusCB_(args->status_changed.state != 0);
    }
  }
}

FLASHMEM EthernetClass::EthernetClass() : EthernetClass(nullptr) {}

FLASHMEM EthernetClass::EthernetClass(const uint8_t mac[6]) {
  if (mac != nullptr) {
    std::copy_n(mac, 6, mac_);
  } else {
    enet_get_mac(mac_);
  }
}

FLASHMEM EthernetClass::~EthernetClass() {
  end();
}

void EthernetClass::macAddress(uint8_t mac[6]) const {
  if (mac != nullptr) {
    std::copy_n(mac_, 6, mac);
  }
}

void EthernetClass::setMACAddress(const uint8_t mac[6]) {
  uint8_t m[6];
  if (mac == nullptr) {
    // Use the system MAC address
    enet_get_mac(m);
    mac = m;
  }

  if (std::equal(&mac_[0], &mac_[6], &mac[0])) {
    // Do nothing if there's no change
    return;
  }

  std::copy_n(mac, 6, mac_);
  if (netif_ == nullptr) {
    return;
  }

#if LWIP_DHCP
  if (dhcpActive_) {
    dhcp_release_and_stop(netif_);  // Stop DHCP in all cases
    dhcpActive_ = false;
  }
#endif  // LWIP_DHCP

  if (start()) {
    (void)maybeStartDHCP();
  }
  // TODO: Return value?
}

void EthernetClass::loop() {
  enet_proc_input();

#if LWIP_NETIF_LOOPBACK || LWIP_HAVE_LOOPIF
  // Poll the netif to allow for loopback
  if (netif_ != nullptr) {
    netif_poll(netif_);
  }
#endif  // LWIP_NETIF_LOOPBACK || LWIP_HAVE_LOOPIF

  if ((millis() - lastPollTime_) >= kPollInterval) {
    enet_poll();
    lastPollTime_ = millis();
  }
}

bool EthernetClass::begin() {
  if (!start()) {
    return false;
  }

  netif_set_addr(netif_, IP4_ADDR_ANY4, IP4_ADDR_ANY4, IP4_ADDR_ANY4);
  return maybeStartDHCP();
}

bool EthernetClass::begin(const IPAddress &ip,
                          const IPAddress &mask,
                          const IPAddress &gateway) {
  return begin(ip, mask, gateway, INADDR_NONE);
}

bool EthernetClass::begin(const IPAddress &ip,
                          const IPAddress &mask,
                          const IPAddress &gateway,
                          const IPAddress &dns) {
  // NOTE: The uint32_t cast doesn't currently work on const IPAddress
  ip4_addr_t ipaddr{get_uint32(ip)};
  ip4_addr_t netmask{get_uint32(mask)};
  ip4_addr_t gw{get_uint32(gateway)};

  if (netif_ != nullptr) {
#if LWIP_DHCP
    // Stop any running DHCP client if we don't need one
    if (dhcpActive_ &&
        (!ip4_addr_isany_val(ipaddr) ||
         !ip4_addr_isany_val(netmask) ||
         !ip4_addr_isany_val(gw))) {
      dhcp_release_and_stop(netif_);
      dhcpActive_ = false;
    }
#endif  // LWIP_DHCP
  }

  if (!start()) {
    return false;
  }

  // Set this before setting the address so any address listeners will see
  // a valid DNS server
  if (dns != INADDR_NONE) {
    setDNSServerIP(dns);
  }

  netif_set_addr(netif_, &ipaddr, &netmask, &gw);
  return maybeStartDHCP();
}

bool EthernetClass::maybeStartDHCP() {
  // If this is using a manual configuration then inform the network,
  // otherwise start DHCP
#if LWIP_DHCP
  bool retval = true;
  const ip4_addr_t *ipaddr  = netif_ip4_addr(netif_);
  const ip4_addr_t *netmask = netif_ip4_netmask(netif_);
  const ip4_addr_t *gw      = netif_ip4_gw(netif_);

  if (!ip4_addr_isany(ipaddr) ||
      !ip4_addr_isany(netmask) ||
      !ip4_addr_isany(gw)) {
    // Don't send a DHCP INFORM message because we don't want the other
    // parameters potentially sent by the server; it also seems to interfere
    // with any first subsequent DHCP requests
    // dhcp_inform(netif_);
    dhcpActive_ = false;
    dhcpDesired_ = false;
  } else if (dhcpEnabled_ && !dhcpActive_) {
    retval = (dhcp_start(netif_) == ERR_OK);
    dhcpActive_ = retval;
    dhcpDesired_ = true;
  }
  return retval;
#else
  return true;
#endif  // LWIP_DHCP
}

bool EthernetClass::start() {
  if (!driver_has_hardware()) {
    return false;
  }

  if (netif_ != nullptr) {
    netif_set_down(netif_);
  }

  // Initialize Ethernet, set up the callback, and set the netif to UP
  netif_ = enet_netif();
  driver_set_chip_select_pin(chipSelectPin_);
  if (!enet_init(mac_, &netifEventFunc)) {
    return false;
  }

#if LWIP_NETIF_HOSTNAME
  if (hostname_.length() == 0) {
    netif_set_hostname(netif_, nullptr);
  } else {
    netif_set_hostname(netif_, hostname_.c_str());
  }
#endif  // LWIP_NETIF_HOSTNAME

  netif_set_up(netif_);

#if defined(__has_include) && __has_include(<EventResponder.h>)
  attachLoopToYield();
#endif  // defined(__has_include) && __has_include(<EventResponder.h>)

  return true;
}

bool EthernetClass::setDHCPEnabled(bool flag) {
#if LWIP_DHCP
  dhcpEnabled_ = flag;
  if (netif_ == nullptr) {
    return true;
  }

  bool retval = true;
  if (flag) {  // DHCP enabled
    if (dhcpDesired_ && !dhcpActive_) {
      retval = (dhcp_start(netif_) == ERR_OK);
      dhcpActive_ = retval;
    }
  } else {  // DHCP disabled
    if (dhcpActive_) {
      dhcp_release_and_stop(netif_);
      dhcpActive_ = false;
    }
  }
  return retval;
#else
  return false;
#endif  // LWIP_DHCP
}

bool EthernetClass::waitForLocalIP(uint32_t timeout) const {
  if (netif_ == nullptr) {
    return false;
  }

  uint32_t t = millis();
  while (ip4_addr_isany_val(*netif_ip4_addr(netif_)) &&
         (millis() - t) < timeout) {
    yield();
  }
  return (!ip4_addr_isany_val(*netif_ip4_addr(netif_)));
}

bool EthernetClass::waitForLink(uint32_t timeout) const {
  if (netif_ == nullptr) {
    return false;
  }

  uint32_t t = millis();
  while (!netif_is_link_up(netif_) && (millis() - t) < timeout) {
    yield();
  }
  return netif_is_link_up(netif_);
}

int EthernetClass::begin(const uint8_t mac[6], uint32_t timeout) {
  uint8_t m[6];
  if (mac == nullptr) {
    enet_get_mac(m);
    mac = m;
  }
  std::copy_n(mac_, 6, m);  // Cache the current MAC address
  std::copy_n(mac, 6, mac_);

  if (!begin()) {
    std::copy_n(m, 6, mac_);  // Restore what was there before
    return false;
  }

  // Wait for an IP address, for Arduino API compatibility
  return waitForLocalIP(timeout);
}

#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wdeprecated-declarations"
void EthernetClass::begin(const uint8_t mac[6], const IPAddress &ip) {
  begin(mac, ip, IPAddress{ip[0], ip[1], ip[2], 1},
        IPAddress{ip[0], ip[1], ip[2], 1}, IPAddress{255, 255, 255, 0});
}

void EthernetClass::begin(const uint8_t mac[6], const IPAddress &ip,
                          const IPAddress &dns) {
  begin(mac, ip, dns, IPAddress{ip[0], ip[1], ip[2], 1},
        IPAddress{255, 255, 255, 0});
}

void EthernetClass::begin(const uint8_t mac[6], const IPAddress &ip,
                          const IPAddress &dns, const IPAddress &gateway) {
  begin(mac, ip, dns, gateway, IPAddress{255, 255, 255, 0});
}
#pragma GCC diagnostic pop

void EthernetClass::begin(const uint8_t mac[6], const IPAddress &ip,
                          const IPAddress &dns, const IPAddress &gateway,
                          const IPAddress &subnet) {
  // This doesn't return any error, so if the MAC is NULL then use the built-in
  uint8_t m[6];
  if (mac == nullptr) {
    enet_get_mac(m);
    mac = m;
  }
  std::copy_n(mac_, 6, m);  // Cache the current MAC address
  std::copy_n(mac, 6, mac_);

  if (!begin(ip, subnet, gateway, dns)) {
    std::copy_n(m, 6, mac_);  // Restore the previous
  }
}

void EthernetClass::end() {
  if (netif_ == nullptr) {
    return;
  }

#if defined(__has_include) && __has_include(<EventResponder.h>)
  if (loopAttached) {
    loopAttached = false;
    ethLoop.clearEvent();
    ethLoop.detach();
  }
#endif  // defined(__has_include) && __has_include(<EventResponder.h>)

#if LWIP_MDNS_RESPONDER
  MDNS.end();
#endif  // LWIP_MDNS_RESPONDER

#if LWIP_DNS
  DNSClient::setServer(0, INADDR_NONE);
#endif  // LWIP_DNS
#if LWIP_DHCP
  if (dhcpActive_) {
    dhcp_release_and_stop(netif_);
    dhcpActive_ = false;
  } else {
    netif_set_addr(netif_, IP4_ADDR_ANY4, IP4_ADDR_ANY4, IP4_ADDR_ANY4);
  }
  dhcpDesired_ = false;
#else
  netif_set_addr(netif_, IP4_ADDR_ANY4, IP4_ADDR_ANY4, IP4_ADDR_ANY4);
#endif  // LWIP_DHCP

  netif_set_link_down(netif_);
  netif_set_down(netif_);

  enet_deinit();
  netif_ = nullptr;
}

EthernetLinkStatus EthernetClass::linkStatus() const {
  if (driver_is_unknown()) {
    return EthernetLinkStatus::Unknown;
  }
  return linkState() ? EthernetLinkStatus::LinkON : EthernetLinkStatus::LinkOFF;
}

bool EthernetClass::linkState() const {
  if (netif_ == nullptr) {
    return false;
  }
  return netif_is_link_up(netif_);
}

void EthernetClass::setLinkState(bool flag) const {
  if (netif_ == nullptr) {
    return;
  }
  if (flag) {
    netif_set_link_up(netif_);
  } else {
    netif_set_link_down(netif_);
  }
}

int EthernetClass::linkSpeed() const {
  return driver_link_speed();
}

bool EthernetClass::linkIsFullDuplex() const {
  return driver_link_is_full_duplex();
}

bool EthernetClass::linkIsCrossover() const {
  return driver_link_is_crossover();
}

bool EthernetClass::interfaceStatus() const {
  if (netif_ == nullptr) {
    return false;
  }
  return netif_is_up(netif_);
}

IPAddress EthernetClass::localIP() const {
  if (netif_ == nullptr) {
    return INADDR_NONE;
  }
  return ip4_addr_get_u32(netif_ip4_addr(netif_));
}

IPAddress EthernetClass::subnetMask() const {
  if (netif_ == nullptr) {
    return INADDR_NONE;
  }
  return ip4_addr_get_u32(netif_ip4_netmask(netif_));
}

IPAddress EthernetClass::gatewayIP() const {
  if (netif_ == nullptr) {
    return INADDR_NONE;
  }
  return ip4_addr_get_u32(netif_ip4_gw(netif_));
}

IPAddress EthernetClass::dnsServerIP() const {
  return dnsServerIP(0);
}

IPAddress EthernetClass::dnsServerIP(int index) const {
#if LWIP_DNS
  return DNSClient::getServer(index);
#else
  return INADDR_NONE;
#endif  // LWIP_DNS
}

IPAddress EthernetClass::broadcastIP() const {
  if (netif_ == nullptr) {
    return INADDR_NONE;
  }
  return ip4_addr_get_u32(netif_ip4_addr(netif_)) |
         ~ip4_addr_get_u32(netif_ip4_netmask(netif_));
}

void EthernetClass::setLocalIP(const IPAddress &localIP) const {
  if (netif_ == nullptr) {
    return;
  }
  ip4_addr_t ipaddr{get_uint32(localIP)};
  netif_set_ipaddr(netif_, &ipaddr);
}

void EthernetClass::setSubnetMask(const IPAddress &subnetMask) const {
  if (netif_ == nullptr) {
    return;
  }
  ip4_addr_t netmask{get_uint32(subnetMask)};
  netif_set_netmask(netif_, &netmask);
}

void EthernetClass::setGatewayIP(const IPAddress &gatewayIP) const {
  if (netif_ == nullptr) {
    return;
  }
  ip4_addr_t gw{get_uint32(gatewayIP)};
  netif_set_gw(netif_, &gw);
}

void EthernetClass::setDNSServerIP(const IPAddress &dnsServerIP) const {
  setDNSServerIP(0, dnsServerIP);
}

void EthernetClass::setDNSServerIP(int index, const IPAddress &ip) const {
#if LWIP_DNS
  DNSClient::setServer(index, ip);
#endif  // LWIP_DNS
}

EthernetHardwareStatus EthernetClass::hardwareStatus() const {
  if (driver_has_hardware()) {
#if defined(QNETHERNET_INTERNAL_DRIVER_W5500)
    return EthernetW5500;
#elif defined(QNETHERNET_INTERNAL_DRIVER_TEENSY41)
    return EthernetTeensy41;
#else
    return EthernetOtherHardware;
#endif  // Which driver
  }
  return EthernetNoHardware;
}

bool EthernetClass::joinGroup(const IPAddress &ip) const {
#if LWIP_IGMP
  if (netif_ == nullptr) {
    return false;
  }
  ip4_addr_t groupaddr{get_uint32(ip)};
  return (igmp_joingroup_netif(netif_, &groupaddr) == ERR_OK);
#else
  return false;
#endif  // LWIP_IGMP
}

bool EthernetClass::leaveGroup(const IPAddress &ip) const {
#if LWIP_IGMP
  if (netif_ == nullptr) {
    return false;
  }
  ip4_addr_t groupaddr{get_uint32(ip)};
  return (igmp_leavegroup_netif(netif_, &groupaddr) == ERR_OK);
#else
  return false;
#endif  // LWIP_IGMP
}

bool EthernetClass::setMACAddressAllowed(const uint8_t mac[6],
                                         bool flag) const {
  if (netif_ == nullptr || mac == nullptr) {
    return false;
  }
#if !QNETHERNET_ENABLE_PROMISCUOUS_MODE
  return driver_set_mac_address_allowed(mac, flag);
#else
  return flag;  // Can't disallow MAC addresses
#endif  // !QNETHERNET_ENABLE_PROMISCUOUS_MODE
}

void EthernetClass::setHostname(const char *hostname) {
#if LWIP_NETIF_HOSTNAME
  hostname_ = hostname;
  if (netif_ != nullptr) {
    if (hostname_.length() == 0) {
      netif_set_hostname(netif_, nullptr);
    } else {
      netif_set_hostname(netif_, hostname);
    }
  }
#endif  // LWIP_NETIF_HOSTNAME
}

String EthernetClass::hostname() const {
#if LWIP_NETIF_HOSTNAME
  return hostname_;
#else
  return "";
#endif  // LWIP_NETIF_HOSTNAME
}

EthernetClass::operator bool() const {
  return (netif_ != nullptr);
}

bool EthernetClass::hostByName(const char *hostname, IPAddress &ip) {
#if LWIP_DNS
  if (netif_ == nullptr) {
    return false;
  }
  return DNSClient::getHostByName(hostname, ip,
                                  QNETHERNET_DEFAULT_DNS_LOOKUP_TIMEOUT);
#else
  return false;
#endif  // LWIP_DNS
}

}  // namespace network
}  // namespace qindesign
