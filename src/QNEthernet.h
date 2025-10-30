// SPDX-FileCopyrightText: (c) 2021-2025 Shawn Silverman <shawn@pobox.com>
// SPDX-License-Identifier: AGPL-3.0-or-later

// QNEthernet.h defines an Arduino-style Ethernet driver for Teensy 4.1.
// This file is part of the QNEthernet library.

#pragma once

// C++ includes
#include <cerrno>
#include <cstddef>
#include <cstdint>
#include <functional>

#include <IPAddress.h>

#include "lwip/apps/mdns_opts.h"
#include "lwip/arch.h"
#include "lwip/netif.h"
#include "lwip/opt.h"
#include "lwip/prot/ethernet.h"
#include "qnethernet/QNEthernetClient.h"
#include "qnethernet/QNEthernetFrame.h"
#include "qnethernet/QNEthernetServer.h"
#include "qnethernet/QNEthernetUDP.h"
#include "qnethernet/QNMDNS.h"
#include "qnethernet/StaticInit.h"
#include "qnethernet/internal/optional.h"
#include "qnethernet/lwip_driver.h"
#include "qnethernet/security/RandomDevice.h"
#include "qnethernet/util/PrintUtils.h"
#include "qnethernet_opts.h"

namespace qindesign {
namespace network {

// See: https://www.arduino.cc/reference/en/libraries/ethernet/ethernet.linkstatus/
enum EthernetLinkStatus {
  Unknown,
  LinkON,
  LinkOFF,
};

// Define this enum because Arduino API. Use a new `EthernetOtherHardware` with
// value -1 as the return value for anything unknown.
//
// See: https://www.arduino.cc/reference/en/libraries/ethernet/ethernet.hardwarestatus/
enum EthernetHardwareStatus {
  EthernetNoHardware = 0,
  EthernetW5100,
  EthernetW5200,
  EthernetW5500,
  EthernetTeensy41,
  EthernetOtherHardware = -1,
};

class EthernetClass final {
 public:
  static constexpr int kMACAddrSize = ETH_HWADDR_LEN;

  // Returns a string containing the library version number.
  static const char* libraryVersion() {
    return "0.33.0-snapshot";
  }

  // Returns the maximum number of multicast groups. Note that mDNS will use
  // one group. Also note that this does not include the "all systems" group.
  //
  // This will return zero if IGMP is disabled.
  static constexpr int maxMulticastGroups() {
#if LWIP_IGMP
    // Exclude the "All Systems" group
    if (MEMP_NUM_IGMP_GROUP > 0) {
      return MEMP_NUM_IGMP_GROUP - 1;
    }
#endif  // LWIP_IGMP
    return 0;
  }

  // Gets the MTU.
  static constexpr size_t mtu() {
    return MTU;
  }

  // Returns whether promiscuous mode is enabled.
  static constexpr bool isPromiscuousMode() {
#if QNETHERNET_ENABLE_PROMISCUOUS_MODE
    return true;
#else
    return false;
#endif  // QNETHERNET_ENABLE_PROMISCUOUS_MODE
  }

  // Returns the set of driver capabilities. This might not be available until
  // after the driver is initialized with one of the begin() calls.
  //
  // Notes:
  // * If the link state is not detectable then it must be managed
  //   with 'setLinkState(flag)'.
  const DriverCapabilities& driverCapabilities() const {
    return driverCapabilities_;
  }

  // Gets the interface name. This will return an empty string if Ethernet is
  // not initialized.
  const char* interfaceName() const {
    return ifName_;
  }

  // Returns a pointer to the current MAC address. If it has not yet been
  // accessed, then this first retrieves the system MAC address from the driver.
  const uint8_t* macAddress();

  // Retrieves the MAC address. If 'mac' is not NULL and if the MAC address has
  // not yet been accessed, then this first retrieves the system MAC address
  // from the driver.
  //
  // Note that if the driver can't set the MAC address then this will retrieve
  // the currently set one.
  void macAddress(uint8_t mac[kMACAddrSize]);

  // Sets the MAC address. If the address is different than the current address,
  // and if the network interface is already up, then the network interface will
  // be reset and any DHCP client will be restarted.
  //
  // If the given array is NULL then the MAC address will be set to the system
  // one, retrieving it from the driver if it has not yet been accessed.
  //
  // If the driver has not yet been initialized then the MAC address will be set
  // to the given address until initialization, where the driver may change it
  // if the MAC address can't be set.
  //
  // See: driverCapabilities()
  void setMACAddress(const uint8_t mac[kMACAddrSize]);

  // Call often.
  void loop();

  // Starts Ethernet. The DHCP client will be started depending on whether it's
  // enabled. If enabled, this returns whether starting the DHCP client was
  // successful. This will always return false if no hardware is detected.
  //
  // Note that when this returns, an IP address may not yet have been acquired,
  // if DHCP is enabled. In other words, it does not block.
  //
  // This also retrieves the system MAC address from the driver if it has not
  // yet been accessed.
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

  // Renews any active DHCP lease. This returns true if the request was sent
  // successfully and false if DHCP is not active or there was an error.
  //
  // If this returns false and there was an error then errno will be set.
  bool renewDHCP() const;

  // Waits, up to the specified timeout, for an IP address and returns whether
  // one was acquired. The timeout is in milliseconds.
  //
  // If the network is not enabled then this will return false immediately and
  // errno will be set to ENETDOWN.
  bool waitForLocalIP(uint32_t timeout) const;

  // Starts Ethernet with the given address configuration. This is similar to
  // `begin(ipaddr, netmask, gw, dns)`, but doesn't set the DNS address.
  bool begin(const IPAddress& ipaddr,
             const IPAddress& netmask,
             const IPAddress& gateway);

  // Starts Ethernet with the given address configuration. If the IP address is
  // INADDR_NONE then this will start a DHCP client, if enabled, and attempt to
  // assign an address that way. Otherwise, no DHCP client will be started.
  //
  // This returns whether bringing up the interface, and possibly the DHCP
  // client, was successful.
  //
  // This also retrieves the system MAC address from the driver if it has not
  // yet been accessed.
  //
  // This returns whether starting Ethernet was successful.
  bool begin(const IPAddress& ipaddr,
             const IPAddress& netmask,
             const IPAddress& gateway,
             const IPAddress& dns);

  // Waits, up to the specified timeout, for a link to be detected. This returns
  // whether a link was detected. The timeout is in milliseconds.
  //
  // If the network is not enabled then this will return false immediately and
  // errno will be set to ENETDOWN.
  bool waitForLink(uint32_t timeout) const;

  // Shuts down the Ethernet peripheral(s) and stops all services.
  void end();

  // Returns the link status, one of the EthernetLinkStatus enumerators. This
  // will return Unknown if the hardware hasn't yet been probed, LinkON if
  // linkState() returns true, and LinkOFF if linkState() returns false.
  //
  // This function is defined by the Arduino API.
  EthernetLinkStatus linkStatus() const;

  // Returns the interface-level link state. It may be managed manually with
  // setLinkState(flag) if the driver can't detect the link.
  //
  // See: driverCapabilities()
  bool linkState() const;

  // Manually sets the link state. This is useful when using the loopback
  // feature. Network operations will usually fail unless there's a link.
  //
  // If the network is not enabled then this will return false immediately and
  // errno will be set to ENETDOWN.
  void setLinkState(bool flag) const;

  // Returns the link speed in Mbps. This is only valid if the link is up and
  // the driver can read the link state.
  //
  // See: driverCapabilities()
  int linkSpeed() const;

  // Returns the link duplex mode, true for full and false for half. This is
  // only valid if the link is up and the driver can read the link state.
  //
  // See: driverCapabilities()
  bool linkIsFullDuplex() const;

  // Returns whether a crossover cable is detected. This is only valid if the
  // link is up and the driver can read the link state.
  //
  // See: driverCapabilities()
  bool linkIsCrossover() const;

  // Sets a link state callback.
  //
  // Note that no network tasks should be done from inside the listener.
  void onLinkState(const std::function<void(bool state)> cb) {
    linkStateCB_ = cb;
  }

  // Sets an address changed callback. This will be called if any of the three
  // addresses changed: IP address, subnet mask, or gateway.
  //
  // Note that no network tasks should be done from inside the listener.
  void onAddressChanged(const std::function<void()> cb) {
#if LWIP_IPV4 || LWIP_IPV6
    addressChangedCB_ = cb;
#endif  // LWIP_IPV4 || LWIP_IPV6
  }

  // Sets an interface status callback. This will be called AFTER the interface
  // is up but BEFORE the interface goes down.
  //
  // Note that no network tasks should be done from inside the listener.
  void onInterfaceStatus(const std::function<void(bool status)> cb) {
    interfaceStatusCB_ = cb;
  }

  // Returns the interface status, true for UP and false for DOWN.
  bool interfaceStatus() const;

  // These functions are defined by the Arduino API:

  IPAddress localIP() const;
  IPAddress subnetMask() const;
  IPAddress gatewayIP() const;

  // Returns the DNS server address at index zero. This returns INADDR_NONE if
  // DNS is disabled.
  //
  // This function is defined by the Arduino API.
  IPAddress dnsServerIP() const;

  // Returns the DNS server IP at the specified index. This returns INADDR_NONE
  // if there is no configured server at that index or if DNS is disabled.
  IPAddress dnsServerIP(int index) const;

  // Returns the broadcast IP address. This is equal to:
  // localIP | ~subnetMask
  //
  // This will return 255.255.255.255 if Ethernet is not initialized.
  IPAddress broadcastIP() const;

  // None of the following address-setting functions do anything unless the
  // system is initialized after a `begin` call
  //
  // These functions are defined by the Arduino API

  void setLocalIP(const IPAddress& ip) const;
  void setSubnetMask(const IPAddress& subnetMask) const;
  void setGatewayIP(const IPAddress& ip) const;

  // Sets the DNS server address at index zero. This does nothing if DNS is
  // is disabled.
  void setDNSServerIP(const IPAddress& ip) const;

  // Sets a specific DNS server IP. This does nothing if the index is not in the
  // range [0, DNSClient::maxServers()).
  void setDNSServerIP(int index, const IPAddress& ip) const;

  // The MAC addresses are used in the following begin() functions. If NULL or
  // the driver can't set the MAC address, then the system MAC address is first
  // retrieved from the driver.
  //
  // Wish: Boolean returns
  // Technically, the non-DHCP begin() functions aren't supposed to
  // return anything.
  //
  // These functions are defined by the Arduino API.

  int begin(const uint8_t mac[kMACAddrSize],
            uint32_t timeout = QNETHERNET_DEFAULT_DHCP_CLIENT_TIMEOUT);
  [[deprecated("See begin(ip, subnet, gateway)")]]
  bool begin(const uint8_t mac[kMACAddrSize], const IPAddress& ip);
  [[deprecated("See begin(ip, subnet, gateway, dns)")]]
  bool begin(const uint8_t mac[kMACAddrSize], const IPAddress& ip,
             const IPAddress& dns);
  [[deprecated("See begin(ip, subnet, gateway, dns)")]]
  bool begin(const uint8_t mac[kMACAddrSize], const IPAddress& ip,
             const IPAddress& dns, const IPAddress& gateway);
  bool begin(const uint8_t mac[kMACAddrSize], const IPAddress& ip,
             const IPAddress& dns, const IPAddress& gateway,
             const IPAddress& subnet);

  // This function is defined by the Arduino API.
  EthernetHardwareStatus hardwareStatus() const;

  // This function is defined by the Arduino API.
  void init(const int sspin) {
    chipSelectPin_ = sspin;
  }

  // Deprecated and unused functions
  //
  // These functions are defined by the Arduino API

  [[deprecated("DHCP maintained internally")]]
  uint8_t maintain() const { return 0; }
  [[deprecated("See TCP_MAXRTX")]]
  void setRetransmissionCount(const uint8_t number) const {
    LWIP_UNUSED_ARG(number);
  }
  [[deprecated("Handled internally")]]
  void setRetransmissionTimeout(const uint16_t milliseconds) const {
    LWIP_UNUSED_ARG(milliseconds);
  }

  // These call something equivalent
  //
  // These functions are defined by the Arduino API

  void MACAddress(uint8_t mac[kMACAddrSize]) { macAddress(mac); }
  void setDnsServerIP(const IPAddress& dnsServerIP) const {
    setDNSServerIP(dnsServerIP);
  }

  // Joins a multicast group. This returns whether the call was successful.
  //
  // The lwIP stack keeps track of a group "use count", so this function can be
  // called multiple times, paired with a matching number of leaveGroup() calls.
  //
  // This always returns false if `LWIP_IGMP` is disabled.
  //
  // If this returns false and there was an error then errno will be set.
  bool joinGroup(const IPAddress& ip) const;

  // Leaves a multicast group. This returns whether the call was successful.
  //
  // The lwIP stack keeps track of a group "use count", so calling this function
  // should match the number of joinGroup() calls.
  //
  // This always returns false if `LWIP_IGMP` is disabled.
  //
  // If this returns false and there was an error then errno will be set.
  bool leaveGroup(const IPAddress& ip) const;

  // Sets whether Ethernet frames addressed to the given MAC address are allowed
  // or disallowed through the Ethernet filter. Note that this is not meant to
  // be used for joining or leaving multicast groups; see `joinGroup(ip)` and
  // `leaveGroup(ip)` instead.
  //
  // Because the underlying system may use a hash of the MAC address, it's
  // possible for there to be collisions. This means that it's not always
  // possible to disallow an address once it's been allowed. This will return
  // false in that case and true otherwise.
  //
  // If the network is not enabled then this will return false immediately and
  // errno will be set to ENETDOWN.
  bool setMACAddressAllowed(const uint8_t mac[kMACAddrSize], bool flag) const;

  // Sets the DHCP client option 12 hostname. The empty string will set the
  // hostname to nothing. The default is "qnethernet-lwip".
  //
  // The new hostname will take effect the next time a new configuration is
  // negotiated with the DHCP server. This could be at system start or when a
  // renewal happens. Ideally, it should be called before 'begin()' if you'd
  // like the hostname to be set at system start.
  //
  // This does nothing if 'LWIP_NETIF_HOSTNAME' is not enabled.
  void setHostname(const char* hostname);

  // Gets the DHCP client option 12 hostname. An empty string means that no
  // hostname is set. The default is "qnethernet-lwip".
  //
  // This returns the empty string if 'LWIP_NETIF_HOSTNAME' is not enabled.
  const char* hostname() const {
#if LWIP_NETIF_HOSTNAME
    return hostname_;
#else
    errno = ENOSYS;
    return "";
#endif  // LWIP_NETIF_HOSTNAME
  }

  // Tests if Ethernet is initialized.
  explicit operator bool() const {
    return (netif_ != nullptr);
  }

  // Convenience function that tries to resolve the given hostname into an IP
  // address. This returns whether successful.
  //
  // This uses QNETHERNET_DEFAULT_DNS_LOOKUP_TIMEOUT as the timeout.
  //
  // If the network is not enabled then this will return false immediately.
  //
  // If this returns false and there was an error then errno will be set.
  bool hostByName(const char* hostname, IPAddress& ip) const;

 private:
  static constexpr uint32_t kPollInterval = 125;  // About 8 times a second

  // Creates a new network interface. The MAC address will be unset.
  EthernetClass() = default;

  ~EthernetClass();

  // EthernetClass is neither copyable nor movable
  // See also: https://en.cppreference.com/w/cpp/language/rule_of_three
  EthernetClass(const EthernetClass&) = delete;
  EthernetClass& operator=(const EthernetClass&) = delete;

  static void netifEventFunc(struct netif* netif, netif_nsc_reason_t reason,
                             const netif_ext_callback_args_t* args);

  // Possibly start the DHCP client, given the current address settings. This
  // returns whether successful. This reads the netif's current ip4 settings.
  [[nodiscard]]
  bool maybeStartDHCP();

  // Starts Ethernet. See the public version of this function, with IPAddress
  // parameters, for information about what this does. This always attempts to
  // restart the netif, including bringing the link and interface down.
  //
  // This assumes that mac_ has a value.
  [[nodiscard]]
  bool start();

  // Starts Ethernet with the given address configuration. If the IP address is
  // INADDR_NONE then DHCP will be started. This sets the DNS address if the
  // argument is not NULL.
  bool begin(const IPAddress& ipaddr,
             const IPAddress& netmask,
             const IPAddress& gateway,
             const IPAddress* dns);

  // Driver configuration
  DriverCapabilities driverCapabilities_;

  int chipSelectPin_ = -1;

  uint32_t lastPollTime_ = 0;

  internal::optional<uint8_t[kMACAddrSize]> mac_;
#if LWIP_NETIF_HOSTNAME
  char hostname_[256]{QNETHERNET_DEFAULT_HOSTNAME};  // Empty means no hostname
#endif  // LWIP_NETIF_HOSTNAME
  struct netif* netif_ = nullptr;
  char ifName_[6]{'\0'};  // Two letters plus up to 3-digit number

#if LWIP_DHCP
  bool dhcpEnabled_ = true;
  bool dhcpDesired_ = false;  // Whether the user wants static or dynamic IP
  bool dhcpActive_  = false;
#endif  // LWIP_DHCP

  // Callbacks
  std::function<void(bool state)> linkStateCB_;
#if LWIP_IPV4 || LWIP_IPV6
  std::function<void()> addressChangedCB_;
#endif  // LWIP_IPV4 || LWIP_IPV6
  std::function<void(bool status)> interfaceStatusCB_;

  friend class StaticInit<EthernetClass>;
};

// Instance for interacting with the library.
STATIC_INIT_DECL(EthernetClass, Ethernet);

#if QNETHERNET_CUSTOM_WRITE
// stdout output.
extern Print* stdoutPrint;

// stderr output.
extern Print* stderrPrint;
#endif  // QNETHERNET_CUSTOM_WRITE

}  // namespace network
}  // namespace qindesign
