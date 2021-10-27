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
#include <elapsedMillis.h>
#include <lwip/netif.h>
#include <lwip/prot/ethernet.h>

#include "QNEthernetClient.h"
#include "QNEthernetServer.h"
#include "QNEthernetUDP.h"

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

  EthernetClass();
  EthernetClass(const uint8_t mac[kMACAddrSize]);

  ~EthernetClass();

  // Retrieve the MAC address.
  void macAddress(uint8_t mac[kMACAddrSize]) const;

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

  void begin(const IPAddress &ipaddr,
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
  [[deprecated]] void setMACAddress(uint8_t mac[6]) {}
  [[deprecated]] void setRetransmissionCount(uint8_t number) {}
  [[deprecated]] void setRetransmissionTimeout(uint16_t milliseconds) {}

  // Send a raw Ethernet frame. This returns whether the send was successful.
  //
  // This will return false if:
  // 1. Ethernet was not started,
  // 2. The frame is NULL, or
  // 3. The length is not in the range 64-1522.
  bool sendRaw(const uint8_t *frame, size_t len);

 private:
  static void netifEventFunc(struct netif *netif, netif_nsc_reason_t reason,
                             const netif_ext_callback_args_t *args);

  static elapsedMillis loopTimer_;

  uint8_t mac_[kMACAddrSize];
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
