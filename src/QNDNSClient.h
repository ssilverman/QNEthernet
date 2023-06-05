// SPDX-FileCopyrightText: (c) 2021-2023 Shawn Silverman <shawn@pobox.com>
// SPDX-License-Identifier: MIT

// QNDNSClient.h defines the DNS client interface.
// This file is part of the QNEthernet library.

#ifndef QNETHERNET_DNSCLIENT_H_
#define QNETHERNET_DNSCLIENT_H_

#include "lwip/opt.h"

#if LWIP_DNS

// C++ includes
#include <cstdint>
#include <functional>

#include <IPAddress.h>

#include "lwip/ip_addr.h"

namespace qindesign {
namespace network {

// Interfaces with lwIP's DNS functions.
class DNSClient final {
 public:
  // Returns the maximum number of DNS servers.
  static constexpr int maxServers() {
    return DNS_MAX_SERVERS;
  }

  // Sets the specified DNS server address. This will return whether setting the
  // address was successful. This will return false if the index is not in the
  // range [0, maxServers()).
  static bool setServer(int index, const IPAddress &ip);

  // Gets the specified DNS server address. This will return INADDR_NONE if
  // the address is not set or the index is out of range.
  static IPAddress getServer(int index);

  // Looks up a host by name. This calls the callback when it has a result. This
  // returns whether the call was successful. If the call was not successful,
  // the callback is not called. Possible errors include:
  // * Invalid hostname
  // * The DNS client is not initialized
  // * The callback equates to nullptr
  //
  // The callback will be passed a NULL IP address if the lookup failed or if
  // there was any other error.
  //
  // If the timeout has been reached then the callback will no longer be called.
  // The timeout is ignored if it is zero.
  static bool getHostByName(const char *hostname,
                            std::function<void(const ip_addr_t *)> callback,
                            uint32_t timeout);

  // Looks up a host by name and wait for the given timeout, in milliseconds.
  // This returns whether the given IP address object was filled in and there
  // was no error. Possible errors include:
  // * Invalid hostname
  // * The DNS client is not initialized
  static bool getHostByName(const char *hostname, IPAddress &ip,
                            uint32_t timeout);

 private:
  // DNS request state.
  struct Request final {
    bool found = false;
    IPAddress ip;
    std::function<void(const ip_addr_t *)> callback;
    uint32_t startTime;
    uint32_t timeout;
  };

  DNSClient() = default;
  ~DNSClient() = default;

  static void dnsFoundFunc(const char *name, const ip_addr_t *ipaddr,
                           void *callback_arg);
};

}  // namespace network
}  // namespace qindesign

#endif  // LWIP_DNS

#endif  // QNETHERNET_DNSCLIENT_H_
