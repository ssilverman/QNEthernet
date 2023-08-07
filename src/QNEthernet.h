// SPDX-FileCopyrightText: (c) 2021-2023 Shawn Silverman <shawn@pobox.com>
// SPDX-License-Identifier: MIT

// QNEthernet.h defines an Arduino-style Ethernet driver for Teensy 4.1.
// This file is part of the QNEthernet library.

#ifndef QNETHERNET_ETHERNET_H_
#define QNETHERNET_ETHERNET_H_

// C++ includes
#include <cstddef>
#include <cstdint>
#include <functional>

#include <IPAddress.h>
#include <WString.h>
#include <elapsedMillis.h>

#include "QNEthernetClient.h"
#include "QNEthernetFrame.h"
#include "QNEthernetServer.h"
#include "QNEthernetUDP.h"
#include "QNMDNS.h"
#include "lwip/apps/mdns_opts.h"
#include "lwip/ip_addr.h"
#include "lwip/netif.h"
#include "lwip/opt.h"
#include "lwip/prot/ethernet.h"
#include "lwip_driver.h"
#include "security/RandomDevice.h"
#include "util/PrintUtils.h"
#include "util/ip_tools.h"

namespace qindesign {
namespace network {

// Default DHCP client timeout, in milliseconds; default is 60 seconds
#ifndef QNETHERNET_DEFAULT_DHCP_CLIENT_TIMEOUT
#define QNETHERNET_DEFAULT_DHCP_CLIENT_TIMEOUT 60'000
#endif  // !QNETHERNET_DHCP_CLIENT_TIMEOUT

// See: https://www.arduino.cc/reference/en/libraries/ethernet/ethernet.linkstatus/
enum EthernetLinkStatus {
  Unknown,
  LinkON,
  LinkOFF,
};

// Define this enum because Arduino API. Use a new `EthernetOtherHardware` with
// value -1 as the return value for anything not in the "official" Arduino list.
//
// See: https://www.arduino.cc/reference/en/libraries/ethernet/ethernet.hardwarestatus/
enum EthernetHardwareStatus {
  EthernetNoHardware = 0,
  EthernetW5100,
  EthernetW5200,
  EthernetW5500,
  EthernetOtherHardware = -1,
      // TODO: Should we also have a Teensy 4.1-specific value?
};

class EthernetClass final {
 public:
  static constexpr int kMACAddrSize = ETH_HWADDR_LEN;

  // Accesses the singleton instance.
  static EthernetClass &instance();

  // EthernetClass is neither copyable nor movable
  EthernetClass(const EthernetClass &) = delete;
  EthernetClass &operator=(const EthernetClass &) = delete;

  // Returns the maximum number of multicast groups. Note that mDNS will use
  // one group. Also note that this does not include the "all systems" group.
  static constexpr int maxMulticastGroups() {
    // Exclude the "All Systems" group
    if (MEMP_NUM_IGMP_GROUP > 0) {
      return MEMP_NUM_IGMP_GROUP - 1;
    }
    return 0;
  }

  // Gets the MTU.
  static constexpr size_t mtu() {
    return MTU;
  }

  // Returns whether promiscuous mode is enabled.
  static constexpr bool isPromiscuousMode() {
#ifdef QNETHERNET_ENABLE_PROMISCUOUS_MODE
    return true;
#else
    return false;
#endif  // QNETHERNET_ENABLE_PROMISCUOUS_MODE
  }

  // Retrieves the MAC address. This does nothing if 'mac' is NULL.
  void macAddress(uint8_t mac[6]) const;

  // Sets the MAC address. If the address is different than the current address,
  // and if the network interface is already up, then the network interface will
  // be reset and any DHCP client will be restarted.
  //
  // If the given array is NULL, then the MAC address will be set to the
  // built-in one.
  void setMACAddress(const uint8_t mac[6]);

  // Call often.
  void loop();

  // Starts Ethernet. The DHCP client will be started depending on whether it's
  // enabled. If enabled, this returns whether starting the DHCP client was
  // successful. This will always return false if no hardware is detected.
  //
  // Note that when this returns, an IP address may not yet have been acquired,
  // if DHCP is enabled. In other words, it does not block.
  //
  // See: waitForLocalIP(timeout)
  bool begin();

  // Enables or disables the DHCP client. This may be called either before or
  // after Ethernet has started. The default is true.
  //
  // If DHCP is desired and Ethernet is up, but DHCP is not active, an attempt
  // will be made to start the DHCP client if the flag is true.
  //
  // This returns whether DHCP doesn't require a restart or it does and whether
  // the restart attempt was successful.
  bool setDHCPEnabled(bool flag);

  // Returns whether the DHCP client is enabled. This is valid whether Ethernet
  // has been started or not. The default is true.
  bool isDHCPEnabled() const {
#if LWIP_DHCP
    return dhcpEnabled_;
#else
    return false;
#endif  // LWIP_DHCP
  }

  // Returns whether DHCP is active.
  bool isDHCPActive() const {
#if LWIP_DHCP
    return dhcpActive_;
#else
    return false;
#endif  // LWIP_DHCP
  }

  // Waits, up to the specified timeout, for an IP address and returns whether
  // one was acquired. The timeout is in milliseconds.
  bool waitForLocalIP(uint32_t timeout) const;

  // Starts Ethernet with the given address configuration. If all of the
  // addresses are INADDR_NONE then this will start a DHCP client, if enabled,
  // and attempt to assign an address that way. Otherwise, no DHCP client will
  // be started.
  //
  // This returns whether bringing up the interface, and possibly the DHCP
  // client, was successful.
  //
  // This calls `begin(ipaddr, netmask, gw, INADDR_NONE)`.
  bool begin(const IPAddress &ipaddr,
             const IPAddress &netmask,
             const IPAddress &gateway);

  // Starts Ethernet with the given address configuration. If ipaddr, netmask,
  // and gateway are all INADDR_NONE then this will start a DHCP client, if
  // enabled. This only sets the DNS server if `dns` is not INADDR_NONE; there
  // is no change if `dns` is INADDR_NONE.
  //
  // This returns whether starting Ethernet was successful.
  bool begin(const IPAddress &ipaddr,
             const IPAddress &netmask,
             const IPAddress &gateway,
             const IPAddress &dns);

  // Waits, up to the specified timeout, for a link to be detected. This returns
  // whether a link was detected. The timeout is in milliseconds.
  bool waitForLink(uint32_t timeout) const;

  // Shuts down the Ethernet peripheral(s) and stops all services.
  void end();

  // Returns the link status, one of the EthernetLinkStatus enumerators. This
  // will return Unknown if the hardware hasn't yet been probed.
  EthernetLinkStatus linkStatus() const;

  // Returns the link state, true for link and false for no or unknown link.
  bool linkState() const;

  // Manually sets the link state. This is useful when using the loopback
  // feature. Network operations will usually fail unless there's a link.
  void setLinkState(bool flag) const;

  // Returns the link speed in Mbps. This is only valid if the link is up.
  int linkSpeed() const;

  // Returns the link duplex mode, true for full and false for half. This is
  // only valid if the link is up.
  bool linkIsFullDuplex() const;

  // Returns whether a crossover cable is detected. This is only valid if the
  // link is up.
  bool linkIsCrossover() const;

  // Sets a link state callback.
  //
  // Note that no network tasks should be done from inside the listener.
  void onLinkState(std::function<void(bool state)> cb) {
    linkStateCB_ = cb;
  }

  // Sets an address changed callback. This will be called if any of the three
  // addresses changed: IP address, subnet mask, or gateway.
  //
  // Note that no network tasks should be done from inside the listener.
  void onAddressChanged(std::function<void()> cb) {
    addressChangedCB_ = cb;
  }

  // Sets an interface status callback. This will be called AFTER the interface
  // is up but BEFORE the interface goes down.
  //
  // Note that no network tasks should be done from inside the listener.
  void onInterfaceStatus(std::function<void(bool status)> cb) {
    interfaceStatusCB_ = cb;
  }

  // Returns the interface status, true for UP and false for DOWN.
  bool interfaceStatus() const;

  IPAddress localIP() const;
  IPAddress subnetMask() const;
  IPAddress gatewayIP() const;

  // Returns INADDR_NONE if DNS is disabled.
  IPAddress dnsServerIP() const;

  // Returns the broadcast IP address. This is equal to:
  // localIP | ~subnetMask
  IPAddress broadcastIP() const;

  // None of the following address setting functions do anything unless the
  // system is initialized after a `begin` call
  void setLocalIP(const IPAddress &localIP) const;
  void setSubnetMask(const IPAddress &subnetMask) const;
  void setGatewayIP(const IPAddress &gatewayIP) const;

  // Does nothing if DNS is disabled.
  void setDNSServerIP(const IPAddress &dnsServerIP) const;

  // The MAC addresses are used in the following begin() functions
  [[deprecated("See begin() and waitForLocalIP(timeout)")]]
  int begin(const uint8_t mac[6],
            uint32_t timeout = QNETHERNET_DEFAULT_DHCP_CLIENT_TIMEOUT);
  [[deprecated("See begin(ip, subnet, gateway)")]]
  void begin(const uint8_t mac[6], const IPAddress &ip);
  [[deprecated("See begin(ip, subnet, gateway, dns)")]]
  void begin(const uint8_t mac[6], const IPAddress &ip, const IPAddress &dns);
  [[deprecated("See begin(ip, subnet, gateway, dns)")]]
  void begin(const uint8_t mac[6], const IPAddress &ip,
             const IPAddress &dns, const IPAddress &gateway);
  [[deprecated("See begin(ip, subnet, gateway, dns)")]]
  void begin(const uint8_t mac[6], const IPAddress &ip,
             const IPAddress &dns, const IPAddress &gateway,
             const IPAddress &subnet);

  EthernetHardwareStatus hardwareStatus() const;

  // Deprecated and unused functions
  [[deprecated("No chip select pin")]]
  void init([[maybe_unused]] uint8_t sspin) const {}
  [[deprecated("DHCP maintained internally")]]
  uint8_t maintain() const { return 0; }
  [[deprecated("See TCP_MAXRTX")]]
  void setRetransmissionCount([[maybe_unused]] uint8_t number) const {}
  [[deprecated("Handled internally")]]
  void setRetransmissionTimeout([[maybe_unused]] uint16_t milliseconds) const {}

  // These call something equivalent
  void MACAddress(uint8_t mac[6]) const { macAddress(mac); }
  void setDnsServerIP(const IPAddress &dnsServerIP) const {
    setDNSServerIP(dnsServerIP);
  }

  // Joins a multicast group. This returns whether the call was successful.
  //
  // The lwIP stack keeps track of a group "use count", so this function can be
  // called multiple times, paired with a matching number of leaveGroup() calls.
  //
  // This always returns false if `LWIP_IGMP` is disabled.
  bool joinGroup(const IPAddress &ip) const;

  // Leaves a multicast group. This returns whether the call was successful.
  //
  // The lwIP stack keeps track of a group "use count", so calling this function
  // should match the number of joinGroup() calls.
  //
  // This always returns false if `LWIP_IGMP` is disabled.
  bool leaveGroup(const IPAddress &ip) const;

  // Sets whether Ethernet frames addressed to the given MAC address are allowed
  // or disallowed through the Ethernet filter. Note that this is not meant to
  // be used for joining or leaving multicast groups; see `joinGroup(ip)` and
  // `leaveGroup(ip)` instead.
  //
  // Because the underlying system uses a hash of the MAC address, it's possible
  // for there to be collisions. This means that it's not always possible to
  // disallow an address once it's been allowed. This will return false in that
  // case and true otherwise.
  bool setMACAddressAllowed(const uint8_t mac[6], bool flag) const;

  // Sets the DHCP client option 12 hostname. The empty string will set the
  // hostname to nothing. The default is "teensy-lwip".
  //
  // The new hostname will take effect the next time a new configuration is
  // negotiated with the DHCP server. This could be at system start or when a
  // renewal happens. Ideally, it should be called before 'begin()' if you'd
  // like the hostname to be set at system start.
  //
  // This does nothing if 'LWIP_NETIF_HOSTNAME' is not enabled.
  void setHostname(const char *hostname);

  // Gets the DHCP client option 12 hostname. An empty string means that no
  // hostname is set. The default is "teensy-lwip".
  //
  // This returns the empty string if 'LWIP_NETIF_HOSTNAME' is not enabled.
  String hostname() const;

  // Tests if Ethernet is initialized.
  explicit operator bool() const;

 private:
  static constexpr uint32_t kPollInterval = 125;  // About 8 times a second

  // Creates a new network interface. This sets the MAC address to the built-in
  // MAC address. This calls the other constructor with a NULL address.
  EthernetClass();

  // Creates a new network interface. This sets the MAC address to the given MAC
  // address. If the given address is NULL then this uses the built-in
  // MAC address.
  explicit EthernetClass(const uint8_t mac[6]);

  ~EthernetClass();

  static void netifEventFunc(struct netif *netif, netif_nsc_reason_t reason,
                             const netif_ext_callback_args_t *args);

  // Starts Ethernet. See the public version of this function, with IPAddress
  // parameters, for information about what this does.
  bool begin(const ip4_addr_t *ipaddr,
             const ip4_addr_t *netmask,
             const ip4_addr_t *gw);

  elapsedMillis pollTimer_;

  uint8_t mac_[6];
#if LWIP_NETIF_HOSTNAME
  String hostname_{"teensy-lwip"};  // Empty means no hostname
#endif  // LWIP_NETIF_HOSTNAME
  struct netif *netif_ = nullptr;

#if LWIP_DHCP
  bool dhcpEnabled_ = true;
  bool dhcpDesired_ = false;  // Whether the user wants static or dynamic IP
  bool dhcpActive_ = false;
#endif  // LWIP_DHCP

  // Callbacks
  std::function<void(bool state)> linkStateCB_ = nullptr;
  std::function<void()> addressChangedCB_ = nullptr;
  std::function<void(bool status)> interfaceStatusCB_ = nullptr;
};

// Instance for interacting with the library.
extern EthernetClass &Ethernet;

#if LWIP_MDNS_RESPONDER
// Instance for interacting with mDNS.
extern MDNSClass &MDNS;
#endif  // LWIP_MDNS_RESPONDER

#ifdef QNETHERNET_ENABLE_RAW_FRAME_SUPPORT
// Instance for using raw Ethernet frames.
extern EthernetFrameClass &EthernetFrame;
#endif  // QNETHERNET_ENABLE_RAW_FRAME_SUPPORT

#ifdef QNETHERNET_ENABLE_CUSTOM_WRITE
// stdout output.
extern Print *stdoutPrint;

// stderr output.
extern Print *stderrPrint;
#endif  // QNETHERNET_ENABLE_CUSTOM_WRITE

}  // namespace network

namespace security {

// A UniformRandomBitGenerator instance.
extern RandomDevice randomDevice;

}  // namespace security

}  // namespace qindesign

#endif  // QNETHERNET_ETHERNET_H_
