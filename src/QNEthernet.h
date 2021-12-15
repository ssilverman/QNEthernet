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

  // Returns the maximum number of multicast groups. Note that mDNS will use
  // one group.
  static constexpr int maxMulticastGroups() {
    // Exclude the "All Systems" group
    if (MEMP_NUM_IGMP_GROUP > 0) {
      return MEMP_NUM_IGMP_GROUP - 1;
    }
    return 0;
  }

  // Retrieves the MAC address.
  void macAddress(uint8_t mac[kMACAddrSize]) const;

  // Sets the MAC address. If the address is different than the current address,
  // and if the network interface is already up, then the network interface will
  // be reset and any DHCP client will be restarted. This does nothing if the
  // given array is NULL.
  void setMACAddress(uint8_t mac[kMACAddrSize]);

  // Get the MTU.
  int mtu() const;

  // Call often.
  static void loop();

  // Starts Ethernet and a DHCP client. This returns whether starting the DHCP
  // client was successful. Note that when this returns, an IP address may not
  // yet have been acquired.
  //
  // See: waitForLocalIP(timeout)
  bool begin();

  // Waits, up to the specified timeout, for an IP address and returns whether
  // one was acquired.
  bool waitForLocalIP(uint32_t timeout);

  // Starts Ethernet with the given address configuration. If all of the
  // addresses are INADDR_NONE then this will start a DHCP client and attempt
  // to assign an address that way. Otherwise, no DHCP client will be started.
  //
  // This returns whether bringing up the interface, and possibly the DHCP
  // client, was successful.
  bool begin(const IPAddress &ipaddr,
             const IPAddress &netmask,
             const IPAddress &gw);

  // Shuts down the Ethernet peripheral(s).
  void end();

  // Returns the link status, one of the EthernetLinkStatus enumerators. This
  // will never return Unknown.
  EthernetLinkStatus linkStatus() const;

  // Returns the link state, true for link and false for no link.
  bool linkState() const;

  // Returns the link speed in Mbps.
  int linkSpeed() const;

  // Sets a link state callback.
  void onLinkState(std::function<void(bool state)> cb) {
    linkStateCB_ = cb;
  }

  // Sets an address changed callback. This will be called if any of the three
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

  // Sends a raw Ethernet frame. This returns whether the send was successful.
  //
  // This will return false if:
  // 1. Ethernet was not started,
  // 2. The frame is NULL, or
  // 3. The length is not in the range 64-1522.
  bool sendRaw(const uint8_t *frame, size_t len);

  // Joins a multicast group. This returns whether the call was successful.
  //
  // The lwIP stack keeps track of a group "use count", so this function can be
  // called multiple times, paired with a matching number of leaveGroup() calls.
  bool joinGroup(const IPAddress &ip);

  // Leaves a multicast group. This returns whether the call was successful.
  //
  // The lwIP stack keeps track of a group "use count", so calling this function
  // should match the number of joinGroup() calls.
  bool leaveGroup(const IPAddress &ip);

  // Sets the DHCP client option 12 hostname. The empty string will set the
  // hostname to nothing. The default is "teensy-lwip".
  //
  // The new hostname will take effect the next time a new configuration is
  // negotiated with the DHCP server. This could be at system start or when a
  // renewal happens. Ideally, it should be called before 'begin()' if you'd
  // like the hostname to be set at system start.
  void setHostname(const String &hostname);

  // Gets the DHCP client option 12 hostname. An empty string means that no
  // hostname is set. The default is "teensy-lwip".
  String hostname();

 private:
  static void netifEventFunc(struct netif *netif, netif_nsc_reason_t reason,
                             const netif_ext_callback_args_t *args);

  // Starts Ethernet. See the public version of this function, with IPAddress
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
