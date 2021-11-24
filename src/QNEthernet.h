// SPDX-FileCopyrightText: (c) 2021 Shawn Silverman <shawn@pobox.com>
// SPDX-License-Identifier: MIT

// QNEthernet.h defines an Arduino-style Ethernet driver for Teensy 4.1.
// This file is part of the QNEthernet library.

#ifndef QNE_ETHERNET_H_
#define QNE_ETHERNET_H_

// C++ includes
#include <cstddef>
#include <cstdint>
#include <functional>

#include <IPAddress.h>
#include <WString.h>
#include <elapsedMillis.h>

#include "QNEthernetClient.h"
#include "QNEthernetServer.h"
#include "QNEthernetUDP.h"
#include "lwip/netif.h"
#include "lwip/opt.h"
#include "lwip/prot/ethernet.h"

namespace qindesign {
namespace network {

// Define this enum because Arduino API.
enum EthernetLinkStatus {
  LinkOFF,
  LinkON,
  Unknown,
};

class EthernetClass final {
 public:
  static constexpr int kMACAddrSize = ETH_HWADDR_LEN;

  // Creates a new network interface. This sets the MAC address to the built-in
  // MAC address. This calls the other constructor with a NULL address.
  EthernetClass();

  // Creates a new network interface. This sets the MAC address to the given MAC
  // address. If the given address is NULL then this uses the built-in
  // MAC address.
  EthernetClass(const uint8_t mac[kMACAddrSize]);

  ~EthernetClass();

  // Return the maximum number of multicast groups. Note that mDNS will use
  // one group.
  static constexpr int maxMulticastGroups() {
    // Exclude the "All Systems" group
    if (MEMP_NUM_IGMP_GROUP > 0) {
      return MEMP_NUM_IGMP_GROUP - 1;
    }
    return 0;
  }

  // Retrieve the MAC address.
  void macAddress(uint8_t mac[kMACAddrSize]) const;

  // Set the MAC address. If the address is different than the current address,
  // and if the network interface is already up, then the network interface will
  // be reset and any DHCP client will be restarted. This does nothing if the
  // given array is NULL.
  void setMACAddress(uint8_t mac[kMACAddrSize]);

  // Get the MTU.
  int mtu() const;

  // Call often.
  static void loop();

  // Start Ethernet and a DHCP client. This returns whether starting the DHCP
  // client was successful. Note that when this returns, an IP address may not
  // yet have been acquired.
  //
  // See: waitForLocalIP(timeout)
  bool begin();

  // Waits, up to the specified timeout, for an IP address and returns whether
  // one was acquired.
  bool waitForLocalIP(uint32_t timeout);

  // Start Ethernet with the given address configuration. If all of the
  // addresses are INADDR_NONE then this will start a DHCP client and attempt
  // to assign an address that way. Otherwise, no DHCP client will be started.
  //
  // This returns whether bringing up the interface, and possibly the DHCP
  // client, was successful.
  bool begin(const IPAddress &ipaddr,
             const IPAddress &netmask,
             const IPAddress &gw);

  // Shut down the Ethernet peripheral(s).
  void end();

  // Return the link status, one of the EthernetLinkStatus enumerators. This
  // will never return Unknown.
  EthernetLinkStatus linkStatus() const;

  // Return the link state, true for link and false for no link.
  bool linkState() const;

  // Return the link speed in Mbps.
  int linkSpeed() const;

  // Set a link state callback.
  void onLinkState(std::function<void(bool state)> cb) {
    linkStateCB_ = cb;
  }

  // Set an address changed callback. This will be called if any of the three
  // addresses changed.
  void onAddressChanged(std::function<void()> cb) {
    addressChangedCB_ = cb;
  }

  IPAddress localIP() const;
  IPAddress subnetMask() const;
  IPAddress gatewayIP() const;
  IPAddress dnsServerIP() const;

  // None of the following address setting functions do anything unless the
  // system is initialized after a `begin` call
  void setLocalIP(const IPAddress &localIP);
  void setSubnetMask(const IPAddress &subnetMask);
  void setGatewayIP(const IPAddress &gatewayIP);
  void setDNSServerIP(const IPAddress &dnsServerIP);

  // The MAC addresses are used in the following begin() functions
  [[deprecated]] int begin(const uint8_t mac[6]);
  [[deprecated]] void begin(const uint8_t mac[6], const IPAddress &ip);
  [[deprecated]] void begin(const uint8_t mac[6], const IPAddress &ip,
                            const IPAddress &dns);
  [[deprecated]] void begin(const uint8_t mac[6], const IPAddress &ip,
                            const IPAddress &dns, const IPAddress &gateway);
  [[deprecated]] void begin(const uint8_t mac[6], const IPAddress &ip,
                            const IPAddress &dns, const IPAddress &gateway,
                            const IPAddress &subnet);

  // Deprecated and unused functions
  [[deprecated]] int hardwareStatus() const { return 0; }
  [[deprecated]] void init(uint8_t sspin) const {}
  [[deprecated]] void MACAddress(uint8_t mac[6]) const { macAddress(mac); }
  [[deprecated]] uint8_t maintain() const { return 0; }
  [[deprecated]] void setDnsServerIP(const IPAddress &dnsServerIP) {
    setDNSServerIP(dnsServerIP);
  }
  [[deprecated]] void setRetransmissionCount(uint8_t number) {}
  [[deprecated]] void setRetransmissionTimeout(uint16_t milliseconds) {}

  // Send a raw Ethernet frame. This returns whether the send was successful.
  //
  // This will return false if:
  // 1. Ethernet was not started,
  // 2. The frame is NULL, or
  // 3. The length is not in the range 64-1522.
  bool sendRaw(const uint8_t *frame, size_t len);

  // Join a multicast group. This returns whether the call was successful.
  bool joinGroup(const IPAddress &ip);

  // Leave a multicast group. This returns whether the call was successful.
  bool leaveGroup(const IPAddress &ip);

  // Set the DHCP client option 12 hostname. The default is NULL.
  void setHostname(const char *hostname);

  // Get the DHCP client option 12 hostname. The default is NULL.
  const char *hostname();

 private:
  static void netifEventFunc(struct netif *netif, netif_nsc_reason_t reason,
                             const netif_ext_callback_args_t *args);

  // Start Ethernet. See the public version of this function, with IPAddress
  // parameters, for information about what this does.
  bool begin(const ip_addr_t *ipaddr,
             const ip_addr_t *netmask,
             const ip_addr_t *gw);

  static elapsedMillis loopTimer_;

  uint8_t mac_[kMACAddrSize];
#ifdef LWIP_NETIF_HOSTNAME
  String hostname_{"teensy-lwip"};  // Empty means no hostname
#endif
  struct netif *netif_ = nullptr;

  // Callbacks
  std::function<void(bool state)> linkStateCB_ = nullptr;
  std::function<void()> addressChangedCB_ = nullptr;
};

// Instance for interacting with the library.
extern EthernetClass Ethernet;

// Lets user code use stdout.
extern Print *stdPrint;

}  // namespace network
}  // namespace qindesign

#endif  // QNE_ETHERNET_H_
