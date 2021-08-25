// Teensy 4.1 Ethernet implementation.
// (c) 2021 Shawn Silverman

#include "Ethernet.h"

// C++ includes
#include <algorithm>

#include <elapsedMillis.h>
#include <lwip/dhcp.h>
#include <lwip/dns.h>
#include <lwip/ip_addr.h>
#include <lwip/udp.h>
#include "lwip_t41.h"

namespace qindesign {
namespace network {

struct netif Ethernet::nullNetif_;

Ethernet::Ethernet() : Ethernet(nullptr) {}

Ethernet::Ethernet(const uint8_t mac[ETH_HWADDR_LEN])
    : isDefaultMAC_(mac == nullptr),
      netif_(&nullNetif_) {
  if (mac != nullptr) {
    std::copy_n(mac, ETH_HWADDR_LEN, mac_);
  }
}

Ethernet::~Ethernet() {
  netif_remove(netif_);
}

void Ethernet::loop() {
  static elapsedMillis timer;

  enet_proc_input();

  if (timer >= 100) {
    enet_poll();
    timer = 0;
  }
}

void Ethernet::begin() {
  const uint8_t *mac = (isDefaultMAC_) ? NULL : mac_;
  enet_init(mac, NULL, NULL, NULL);
  netif_ = netif_default;
  netif_set_up(netif_);

  dhcp_start(netif_);
}

void Ethernet::begin(const IPAddress &ip,
                     const IPAddress &mask,
                     const IPAddress &gateway) {
  // NOTE: The uint32_t cast doesn't currently work on const IPAddress
  ip_addr_t ipaddr =
      IPADDR4_INIT(static_cast<uint32_t>(const_cast<IPAddress &>(ip)));
  ip_addr_t netmask =
      IPADDR4_INIT(static_cast<uint32_t>(const_cast<IPAddress &>(mask)));
  ip_addr_t gw =
      IPADDR4_INIT(static_cast<uint32_t>(const_cast<IPAddress &>(gateway)));

  const uint8_t *mac = (isDefaultMAC_) ? NULL : mac_;
  enet_init(mac, &ipaddr, &netmask, &gw);
  netif_ = netif_default;
  netif_set_up(netif_);
}

bool Ethernet::linkStatus() {
  return netif_is_link_up(netif_);
}

IPAddress Ethernet::localIP() {
  return IPAddress{netif_ip_addr4(netif_)->addr};
}

IPAddress Ethernet::subnetMask() {
  return IPAddress{netif_ip_netmask4(netif_)->addr};
}

IPAddress Ethernet::gatewayIP() {
  return IPAddress{netif_ip_gw4(netif_)->addr};
}

IPAddress Ethernet::dnsServerIP() {
  return IPAddress{dns_getserver(0)->addr};
}

void Ethernet::setLocalIP(const IPAddress &localIP) {
  const ip_addr_t ipaddr =
      IPADDR4_INIT(static_cast<uint32_t>(const_cast<IPAddress &>(localIP)));
  netif_set_ipaddr(netif_, &ipaddr);
}

void Ethernet::setSubnetMask(const IPAddress &subnetMask) {
  const ip_addr_t netmask =
      IPADDR4_INIT(static_cast<uint32_t>(const_cast<IPAddress &>(subnetMask)));
  netif_set_netmask(netif_, &netmask);
}

void Ethernet::setGatewayIP(const IPAddress &gatewayIP) {
  const ip_addr_t gw =
      IPADDR4_INIT(static_cast<uint32_t>(const_cast<IPAddress &>(gatewayIP)));
  netif_set_gw(netif_, &gw);
}

void Ethernet::setDNSServerIP(const IPAddress &dnsServerIP) {
  const ip_addr_t dnsserver =
      IPADDR4_INIT(static_cast<uint32_t>(const_cast<IPAddress &>(dnsServerIP)));
  dns_setserver(0, &dnsserver);
}

}  // namespace network
}  // namespace qindesign
