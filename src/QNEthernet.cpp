// SPDX-FileCopyrightText: (c) 2021 Shawn Silverman <shawn@pobox.com>
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
#include "lwip/dns.h"
#include "lwip/igmp.h"
#include "lwip/ip_addr.h"
#include "lwip/udp.h"

namespace qindesign {
namespace network {

// Global definitions for Arduino
static EventResponder ethLoop;
EthernetClass Ethernet;
static bool ethActive = false;

// Attach the loop() call to yield() via EventResponder.
static void attachLoopToYield() {
  ethLoop.attach([](EventResponderRef r) {
    EthernetClass::loop();
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
}

EthernetClass::EthernetClass() : EthernetClass(nullptr) {}

EthernetClass::EthernetClass(const uint8_t mac[kMACAddrSize]) {
  if (mac != nullptr) {
    std::copy_n(mac, kMACAddrSize, mac_);
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

void EthernetClass::macAddress(uint8_t mac[kMACAddrSize]) const {
  std::copy_n(mac_, kMACAddrSize, mac);
}

void EthernetClass::setMACAddress(uint8_t mac[kMACAddrSize]) {
  if (mac == nullptr) {
    // Don't do anything
    return;
  }

  if (std::equal(&mac_[0], &mac_[kMACAddrSize], &mac[0])) {
    // Do nothing if there's no change
    return;
  }

  std::copy_n(mac, kMACAddrSize, mac_);
  if (netif_ == nullptr) {
    return;
  }

  dhcp_release_and_stop(netif_);  // Stop DHCP in all cases
  netif_set_down(netif_);

  ip_addr_t ipaddr;
  ip_addr_t netmask;
  ip_addr_t gw;
  ip_addr_set(&ipaddr, netif_ip_addr4(netif_));
  ip_addr_set(&netmask, netif_ip_netmask4(netif_));
  ip_addr_set(&gw, netif_ip_gw4(netif_));

  begin(&ipaddr, &netmask, &gw);
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
  return begin(INADDR_NONE, INADDR_NONE, INADDR_NONE);
}

bool EthernetClass::begin(const IPAddress &ip,
                          const IPAddress &mask,
                          const IPAddress &gateway) {
  // NOTE: The uint32_t cast doesn't currently work on const IPAddress
  ip_addr_t ipaddr =
      IPADDR4_INIT(static_cast<uint32_t>(const_cast<IPAddress &>(ip)));
  ip_addr_t netmask =
      IPADDR4_INIT(static_cast<uint32_t>(const_cast<IPAddress &>(mask)));
  ip_addr_t gw =
      IPADDR4_INIT(static_cast<uint32_t>(const_cast<IPAddress &>(gateway)));

  if (netif_ != nullptr) {
    // Stop any running DHCP client if we don't need one
    if (ipaddr.addr != IPADDR_ANY ||
        netmask.addr != IPADDR_ANY ||
        gw.addr != IPADDR_ANY) {
      dhcp_release_and_stop(netif_);
    }
    netif_set_down(netif_);
  }

  return begin(&ipaddr, &netmask, &gw);
}

bool EthernetClass::begin(const ip_addr_t *ipaddr,
                          const ip_addr_t *netmask,
                          const ip_addr_t *gw) {
  // Initialize Ethernet, set up the callback, and set the netif to UP
  netif_ = enet_netif();
  enet_init(mac_, ipaddr, netmask, gw, &netifEventFunc);
#ifdef LWIP_NETIF_HOSTNAME
  if (hostname_.length() == 0) {
    netif_set_hostname(netif_, nullptr);
  } else {
    netif_set_hostname(netif_, hostname_.c_str());
  }
#endif

  // Watch for final multicast joins and leaves so we can configure
  // them properly
  netif_set_igmp_mac_filter(netif_,
                            [](struct netif *netif, const ip4_addr_t *group,
                               enum netif_mac_filter_action action) -> err_t {
                              switch (action) {
                                case NETIF_ADD_MAC_FILTER:
                                  enet_join_group(group);
                                  break;
                                case NETIF_DEL_MAC_FILTER:
                                  enet_leave_group(group);
                                  break;
                                default:
                                  break;
                              }
                              return ERR_OK;
                            });

  netif_set_up(netif_);

  // If this is using a manual configuration then inform the network,
  // otherwise start DHCP
  bool retval = true;
  if (ipaddr->addr != IPADDR_ANY ||
      netmask->addr != IPADDR_ANY ||
      gw->addr != IPADDR_ANY) {
    dhcp_inform(netif_);
  } else {
    retval = (dhcp_start(netif_) == ERR_OK);
  }

  ethActive = true;
  ethLoop.triggerEvent();
  return retval;
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

  dns_setserver(0, IP_ADDR_ANY);
  dhcp_release_and_stop(netif_);
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
  DNSClient::setServer(0, dnsServerIP);
}

bool EthernetClass::sendRaw(const uint8_t *frame, size_t len) {
  if (netif_ == nullptr) {
    return false;
  }
  return enet_output_frame(frame, len);
}

bool EthernetClass::joinGroup(const IPAddress &ip) {
  if (netif_ == nullptr) {
    return false;
  }
  const ip_addr_t groupaddr =
      IPADDR4_INIT(static_cast<uint32_t>(const_cast<IPAddress &>(ip)));
  return (igmp_joingroup_netif(netif_, &groupaddr) == ERR_OK);
}

bool EthernetClass::leaveGroup(const IPAddress &ip) {
  if (netif_ == nullptr) {
    return false;
  }
  const ip_addr_t groupaddr =
      IPADDR4_INIT(static_cast<uint32_t>(const_cast<IPAddress &>(ip)));
  return (igmp_leavegroup_netif(netif_, &groupaddr) == ERR_OK);
}

void EthernetClass::setHostname(const String &hostname) {
#ifdef LWIP_NETIF_HOSTNAME
  hostname_ = hostname;
  if (netif_ != nullptr) {
    if (hostname.length() == 0) {
      netif_set_hostname(netif_, nullptr);
    } else {
      netif_set_hostname(netif_, hostname.c_str());
    }
  }
#endif
}

String EthernetClass::hostname() {
#ifdef LWIP_NETIF_HOSTNAME
  return hostname_;
#else
  return "";
#endif
}

}  // namespace network
}  // namespace qindesign
