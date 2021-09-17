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

namespace qindesign {
namespace network {

// Global definitions for Arduino
static EventResponder ethLoop;
EthernetClass Ethernet;
static bool ethActive = false;

// Start the loop() call in yield() via EventResponder.
void startLoopInYield() {
  ethActive = true;
  ethLoop.attach([](EventResponderRef r) {
    EthernetClass::loop();
    if (ethActive) {
      r.triggerEvent();
    }
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
}

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

// Declare this static object.
elapsedMillis EthernetClass::loopTimer_;

void EthernetClass::loop() {
  enet_proc_input();

  if (loopTimer_ >= 125) {
    enet_poll();
    loopTimer_ = 0;
  }
}

bool EthernetClass::begin() {
  begin(INADDR_NONE, INADDR_NONE, INADDR_NONE);

  return (dhcp_start(netif_) == ERR_OK);
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

  // Initialize Ethernet, set up the callback, and set the netif to UP
  netif_ = enet_netif();
  enet_init(mac_, &ipaddr, &netmask, &gw, &netifEventFunc);
  netif_set_up(netif_);

  // If this is using a manual configuration then inform the network
  if (!(const_cast<IPAddress &>(ip) == INADDR_NONE) ||
      !(const_cast<IPAddress &>(mask) == INADDR_NONE) ||
      !(const_cast<IPAddress &>(gateway) == INADDR_NONE)) {
    dhcp_inform(netif_);
  }

  startLoopInYield();
}

bool EthernetClass::waitForLocalIP(uint32_t timeout) {
  if (netif_ == nullptr) {
    return false;
  }

  elapsedMillis timer;
  while (netif_ip_addr4(netif_)->addr == 0 && timer < timeout) {
    yield();
  }
  return (netif_ip_addr4(netif_)->addr != 0);
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

  ethActive = false;
  ethLoop.detach();

  dhcp_release_and_stop(netif_);
  dns_setserver(0, IP_ADDR_ANY);
  netif_set_down(netif_);
  netif_ = nullptr;

  enet_deinit();
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
