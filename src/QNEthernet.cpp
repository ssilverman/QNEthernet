// SPDX-FileCopyrightText: (c) 2021-2023 Shawn Silverman <shawn@pobox.com>
// SPDX-License-Identifier: MIT

// QNEthernet.cpp contains the Teensy 4.1 Ethernet implementation.
// This file is part of the QNEthernet library.

#include "QNEthernet.h"

// C++ includes
#include <algorithm>
#include <cstdlib>

#include <EventResponder.h>
#include <pgmspace.h>

#include "QNDNSClient.h"
#include "lwip/dhcp.h"
#include "lwip/igmp.h"

namespace qindesign {

namespace security {

// The UniformRandomBitGenerator instance.
RandomDevice randomDevice;

}  // namespace security

namespace network {

EthernetClass &EthernetClass::instance() {
  static EthernetClass instance;
  return instance;
}

// A reference to the singleton.
EthernetClass &Ethernet = EthernetClass::instance();

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

  // Initialize randomness
  std::srand(security::randomDevice());
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

  begin(netif_ip4_addr(netif_),
        netif_ip4_netmask(netif_),
        netif_ip4_gw(netif_));
}

void EthernetClass::loop() {
  enet_proc_input();

#if LWIP_NETIF_LOOPBACK || LWIP_HAVE_LOOPIF
  // Poll the netif to allow for loopback
  if (netif_ != nullptr) {
    netif_poll(netif_);
  }
#endif  // LWIP_NETIF_LOOPBACK || LWIP_HAVE_LOOPIF

  if (pollTimer_ >= kPollInterval) {
    enet_poll();
    pollTimer_ = 0;
  }
}

bool EthernetClass::begin() {
  return begin(INADDR_NONE, INADDR_NONE, INADDR_NONE);
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

  if (dns != INADDR_NONE) {
    setDNSServerIP(dns);
  }
  return begin(&ipaddr, &netmask, &gw);
}

bool EthernetClass::begin(const ip4_addr_t *ipaddr,
                          const ip4_addr_t *netmask,
                          const ip4_addr_t *gw) {
  if (!enet_has_hardware()) {
    return false;
  }

  if (netif_ != nullptr) {
    netif_set_down(netif_);
  }

  // Initialize Ethernet, set up the callback, and set the netif to UP
  netif_ = enet_netif();
  if (!enet_init(mac_, ipaddr, netmask, gw, &netifEventFunc)) {
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

  // If this is using a manual configuration then inform the network,
  // otherwise start DHCP
  bool retval = true;
#if LWIP_DHCP
  if (!ip4_addr_isany(ipaddr) ||
      !ip4_addr_isany(netmask) ||
      !ip4_addr_isany(gw)) {
    // Don't send a DHCP INFORM message because we don't want the other
    // parameters potentially sent by the server; it also seems to interfere
    // with any first subsequent DHCP requests
    // dhcp_inform(netif_);
    dhcpActive_ = false;
    dhcpDesired_ = false;
  } else if (dhcpEnabled_) {
    retval = (dhcp_start(netif_) == ERR_OK);
    dhcpActive_ = retval;
    dhcpDesired_ = true;
  }
#endif  // LWIP_DHCP

  attachLoopToYield();
  return retval;
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

  elapsedMillis timer;
  while (ip4_addr_isany_val(*netif_ip4_addr(netif_)) && timer < timeout) {
    yield();
  }
  return (!ip4_addr_isany_val(*netif_ip4_addr(netif_)));
}

bool EthernetClass::waitForLink(uint32_t timeout) const {
  if (netif_ == nullptr) {
    return false;
  }

  elapsedMillis timer;
  while (!netif_is_link_up(netif_) && timer < timeout) {
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
#pragma GCC diagnostic pop

void EthernetClass::end() {
  if (netif_ == nullptr) {
    return;
  }

  if (loopAttached) {
    loopAttached = false;
    ethLoop.clearEvent();
    ethLoop.detach();
  }

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
  }
  dhcpDesired_ = false;
#endif  // LWIP_DHCP

  enet_deinit();
  netif_ = nullptr;
}

EthernetLinkStatus EthernetClass::linkStatus() const {
  if (enet_is_unknown()) {
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
  return phy_link_speed();
}

bool EthernetClass::linkIsFullDuplex() const {
  return phy_link_is_full_duplex();
}

bool EthernetClass::linkIsCrossover() const {
  return phy_link_is_crossover();
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
#if LWIP_DNS
  if (netif_ == nullptr) {
    return INADDR_NONE;
  }
  return DNSClient::getServer(0);
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
#if LWIP_DNS
  DNSClient::setServer(0, dnsServerIP);
#endif  // LWIP_DNS
}

EthernetHardwareStatus EthernetClass::hardwareStatus() const {
  if (enet_has_hardware()) {
    return EthernetOtherHardware;
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
  if (netif_ == nullptr) {
    return false;
  }
#ifndef QNETHERNET_ENABLE_PROMISCUOUS_MODE
  return enet_set_mac_address_allowed(mac, flag);
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

}  // namespace network
}  // namespace qindesign
