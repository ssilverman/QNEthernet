// SPDX-FileCopyrightText: (c) 2021-2023 Shawn Silverman <shawn@pobox.com>
// SPDX-License-Identifier: MIT

// QNEthernet.cpp contains the Teensy 4.1 Ethernet implementation.
// This file is part of the QNEthernet library.

#include "QNEthernet.h"

// C++ includes
#include <algorithm>

#include <Entropy.h>
#include <EventResponder.h>

#include "QNDNSClient.h"
#include "lwip/dhcp.h"
#include "lwip/igmp.h"

namespace qindesign {
namespace network {

EthernetClass &EthernetClass::instance() {
  static EthernetClass instance;
  return instance;
}

// A reference to the singleton.
EthernetClass &Ethernet = EthernetClass::instance();

// Global definitions for Arduino
static EventResponder ethLoop;
static bool ethActive = false;

// Attach the loop() call to yield() via EventResponder.
static void attachLoopToYield() {
  ethLoop.attach([](EventResponderRef r) {
    Ethernet.loop();
    if (ethActive) {
      r.triggerEvent();
    }
  });
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

EthernetClass::EthernetClass() : EthernetClass(nullptr) {}

EthernetClass::EthernetClass(const uint8_t mac[6]) {
  if (mac != nullptr) {
    std::copy_n(mac, 6, mac_);
  } else {
    enet_getmac(mac_);
  }

  // Initialize randomness since this isn't done anymore in eth_init
#if defined(__IMXRT1062__)
  bool doEntropyInit = (CCM_CCGR6 & CCM_CCGR6_TRNG(CCM_CCGR_ON)) == 0;
#else
  bool doEntropyInit = true;
#endif
  if (doEntropyInit) {
    Entropy.Initialize();
  }
  srand(Entropy.random());

  attachLoopToYield();
}

EthernetClass::~EthernetClass() {
  end();
}

void EthernetClass::macAddress(uint8_t mac[6]) const {
  std::copy_n(mac_, 6, mac);
}

void EthernetClass::setMACAddress(const uint8_t mac[6]) {
  if (mac == nullptr) {
    // Don't do anything
    return;
  }

  if (std::equal(&mac_[0], &mac_[6], &mac[0])) {
    // Do nothing if there's no change
    return;
  }

  std::copy_n(mac, 6, mac_);
  if (netif_ == nullptr) {
    return;
  }

  dhcp_release_and_stop(netif_);  // Stop DHCP in all cases
  dhcpActive_ = false;
  netif_set_down(netif_);

  begin(netif_ip4_addr(netif_),
        netif_ip4_netmask(netif_),
        netif_ip4_gw(netif_));
}

void EthernetClass::loop() {
  enet_proc_input();

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
    // Stop any running DHCP client if we don't need one
    if (!ip4_addr_isany_val(ipaddr) ||
        !ip4_addr_isany_val(netmask) ||
        !ip4_addr_isany_val(gw)) {
      dhcp_release_and_stop(netif_);
      dhcpActive_ = false;
    }
    netif_set_down(netif_);
  }

  if (dns != INADDR_NONE) {
    setDNSServerIP(dns);
  }
  return begin(&ipaddr, &netmask, &gw);
}

bool EthernetClass::begin(const ip4_addr_t *ipaddr,
                          const ip4_addr_t *netmask,
                          const ip4_addr_t *gw) {
  // Initialize Ethernet, set up the callback, and set the netif to UP
  netif_ = enet_netif();
  enet_init(mac_, ipaddr, netmask, gw, &netifEventFunc);
#if LWIP_NETIF_HOSTNAME
  if (hostname_.length() == 0) {
    netif_set_hostname(netif_, nullptr);
  } else {
    netif_set_hostname(netif_, hostname_.c_str());
  }
#endif

  netif_set_up(netif_);

  // If this is using a manual configuration then inform the network,
  // otherwise start DHCP
  bool retval = true;
  if (!ip4_addr_isany(ipaddr) ||
      !ip4_addr_isany(netmask) ||
      !ip4_addr_isany(gw)) {
    dhcp_inform(netif_);
    dhcpActive_ = false;
  } else {
    retval = (dhcp_start(netif_) == ERR_OK);
    dhcpActive_ = retval;
  }

  ethActive = true;
  ethLoop.triggerEvent();
  return retval;
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

int EthernetClass::begin(const uint8_t mac[6]) {
  std::copy_n(mac, 6, mac_);
  return begin();
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
  std::copy_n(mac, 6, mac_);
  begin(ip, subnet, gateway, dns);
}
#pragma GCC diagnostic pop

void EthernetClass::end() {
  if (netif_ == nullptr) {
    return;
  }

  ethActive = false;

#if LWIP_MDNS_RESPONDER
  MDNS.end();
#endif  // LWIP_MDNS_RESPONDER

  DNSClient::setServer(0, INADDR_NONE);
  dhcp_release_and_stop(netif_);
  dhcpActive_ = false;
  netif_set_down(netif_);

  enet_deinit();
  netif_ = nullptr;
}

EthernetLinkStatus EthernetClass::linkStatus() const {
  return linkState() ? EthernetLinkStatus::LinkON : EthernetLinkStatus::LinkOFF;
}

bool EthernetClass::linkState() const {
  if (netif_ == nullptr) {
    return false;
  }
  return netif_is_link_up(netif_);
}

int EthernetClass::linkSpeed() const {
  return enet_link_speed();
}

bool EthernetClass::linkIsFullDuplex() const {
  return enet_link_is_full_duplex();
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
  if (netif_ == nullptr) {
    return INADDR_NONE;
  }
  return DNSClient::getServer(0);
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
  DNSClient::setServer(0, dnsServerIP);
}

bool EthernetClass::joinGroup(const IPAddress &ip) const {
  if (netif_ == nullptr) {
    return false;
  }
  ip4_addr_t groupaddr{get_uint32(ip)};
  return (igmp_joingroup_netif(netif_, &groupaddr) == ERR_OK);
}

bool EthernetClass::leaveGroup(const IPAddress &ip) const {
  if (netif_ == nullptr) {
    return false;
  }
  ip4_addr_t groupaddr{get_uint32(ip)};
  return (igmp_leavegroup_netif(netif_, &groupaddr) == ERR_OK);
}

bool EthernetClass::setMACAddressAllowed(const uint8_t mac[6],
                                         bool flag) const {
  if (netif_ == nullptr) {
    return false;
  }
#ifndef QNETHERNET_PROMISCUOUS_MODE
  enet_set_mac_address_allowed(mac, flag);
  return true;
#else
  return flag;  // Can't disallow MAC addresses
#endif  // !QNETHERNET_PROMISCUOUS_MODE
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
#endif
}

String EthernetClass::hostname() const {
#if LWIP_NETIF_HOSTNAME
  return hostname_;
#else
  return "";
#endif
}

EthernetClass::operator bool() const {
  return (netif_ != nullptr);
}

}  // namespace network
}  // namespace qindesign
