// SPDX-FileCopyrightText: (c) 2021 Shawn Silverman <shawn@pobox.com>
// SPDX-License-Identifier: MIT

// QNEthernet.cpp contains the Teensy 4.1 Ethernet implementation.
// This file is part of the QNEthernet library.

#include "QNEthernet.h"

// C++ includes
#include <algorithm>

#include <Entropy.h>
#include <EventResponder.h>
#include <lwip/dhcp.h>
#include <lwip/dns.h>
#include <lwip/ip_addr.h>
#include <lwip/udp.h>
#include "lwip_t41.h"

extern const int kMTU;

// Global definitions for Arduino.
static EventResponder ethLoop;
qindesign::network::EthernetClass Ethernet;

// Start the loop() call in yield() via EventResponder.
void startLoopInYield() {
  ethLoop.attach([](EventResponderRef r) {
    Ethernet.loop();
    r.triggerEvent();
  });
  ethLoop.triggerEvent();
}

namespace qindesign {
namespace network {

EthernetClass::EthernetClass() : EthernetClass(nullptr) {}

EthernetClass::EthernetClass(const uint8_t mac[kMACAddrSize]) {
  if (mac != nullptr) {
    std::copy_n(mac, kMACAddrSize, mac_);
  } else {
    enet_getmac(mac_);
  }

  // Initialize randomness since this isn't done anymore in eth_init
  Entropy.Initialize();
  srand(Entropy.random());
}

EthernetClass::~EthernetClass() {
  end();
}

void EthernetClass::macAddress(uint8_t mac[kMACAddrSize]) const {
  std::copy_n(mac_, kMACAddrSize, mac);
}

int EthernetClass::mtu() const {
  return kMTU;
}

void EthernetClass::loop() {
  enet_proc_input();

  if (loopTimer_ >= 125) {
    enet_poll();
    loopTimer_ = 0;
  }
}

bool EthernetClass::begin() {
  enet_init(mac_, NULL, NULL, NULL);
  netif_ = netif_default;
  netif_set_up(netif_);

  bool retval = (dhcp_start(netif_) == ERR_OK);
  startLoopInYield();
  return retval;
}

void EthernetClass::begin(const IPAddress &ip,
                          const IPAddress &mask,
                          const IPAddress &gateway) {
  // NOTE: The uint32_t cast doesn't currently work on const IPAddress
  ip_addr_t ipaddr =
      IPADDR4_INIT(static_cast<uint32_t>(const_cast<IPAddress &>(ip)));
  ip_addr_t netmask =
      IPADDR4_INIT(static_cast<uint32_t>(const_cast<IPAddress &>(mask)));
  ip_addr_t gw =
      IPADDR4_INIT(static_cast<uint32_t>(const_cast<IPAddress &>(gateway)));

  enet_init(mac_, &ipaddr, &netmask, &gw);
  netif_ = netif_default;
  netif_set_up(netif_);

  startLoopInYield();
}

int EthernetClass::begin(const uint8_t mac[6]) {
  std::copy_n(mac, kMACAddrSize, mac_);
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
  std::copy_n(mac, kMACAddrSize, mac_);
  begin(ip, subnet, gateway);
  setDNSServerIP(dns);
}
#pragma GCC diagnostic pop

void EthernetClass::end() {
  if (netif_ == nullptr) {
    return;
  }

  dhcp_stop(netif_);
  dns_setserver(0, IP_ADDR_ANY);
  netif_set_down(netif_);
  netif_ = nullptr;

  enet_deinit();
}

bool EthernetClass::linkStatus() const {
  if (netif_ == nullptr) {
    return false;
  }
  return netif_is_link_up(netif_);
}

IPAddress EthernetClass::localIP() const {
  if (netif_ == nullptr) {
    return INADDR_NONE;
  }
  return IPAddress{netif_ip_addr4(netif_)->addr};
}

IPAddress EthernetClass::subnetMask() const {
  if (netif_ == nullptr) {
    return INADDR_NONE;
  }
  return IPAddress{netif_ip_netmask4(netif_)->addr};
}

IPAddress EthernetClass::gatewayIP() const {
  if (netif_ == nullptr) {
    return INADDR_NONE;
  }
  return IPAddress{netif_ip_gw4(netif_)->addr};
}

IPAddress EthernetClass::dnsServerIP() const {
  if (netif_ == nullptr) {
    return INADDR_NONE;
  }
  return IPAddress{dns_getserver(0)->addr};
}

void EthernetClass::setLocalIP(const IPAddress &localIP) {
  if (netif_ == nullptr) {
    return;
  }

  const ip_addr_t ipaddr =
      IPADDR4_INIT(static_cast<uint32_t>(const_cast<IPAddress &>(localIP)));
  netif_set_ipaddr(netif_, &ipaddr);
}

void EthernetClass::setSubnetMask(const IPAddress &subnetMask) {
  if (netif_ == nullptr) {
    return;
  }

  const ip_addr_t netmask =
      IPADDR4_INIT(static_cast<uint32_t>(const_cast<IPAddress &>(subnetMask)));
  netif_set_netmask(netif_, &netmask);
}

void EthernetClass::setGatewayIP(const IPAddress &gatewayIP) {
  if (netif_ == nullptr) {
    return;
  }

  const ip_addr_t gw =
      IPADDR4_INIT(static_cast<uint32_t>(const_cast<IPAddress &>(gatewayIP)));
  netif_set_gw(netif_, &gw);
}

void EthernetClass::setDNSServerIP(const IPAddress &dnsServerIP) {
  if (netif_ == nullptr) {
    return;
  }

  const ip_addr_t dnsserver =
      IPADDR4_INIT(static_cast<uint32_t>(const_cast<IPAddress &>(dnsServerIP)));
  dns_setserver(0, &dnsserver);
}

}  // namespace network
}  // namespace qindesign
