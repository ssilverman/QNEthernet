// SPDX-FileCopyrightText: (c) 2021 Shawn Silverman <shawn@pobox.com>
// SPDX-License-Identifier: MIT

// QNMDNS.h defines an mDNS implementation.
// This file is part of the QNEthernet library.

#ifndef QNE_MDNS_H_
#define QNE_MDNS_H_

// C++ includes
#include <memory>
#include <tuple>

#include <WString.h>
#include <lwip/netif.h>
#include <lwipopts.h>

namespace qindesign {
namespace network {

// MDNS provides mDNS responder functionality.
class MDNSClass final {
 public:
  // Initializes mDNS. This should only be called once.
  MDNSClass() = default;
  ~MDNSClass() = default;

  // Start the mDNS responder. This returns whether the call was successful.
  bool begin(const String &host);

  // Attempt to stop the mDNS responder. Returns whether the call
  // was successful.
  bool end();

  // Add a service. The protocol will be set to "_udp" for anything other than
  // "_tcp". The strings should have a "_" prefix. This uses the host name as
  // the servie name.
  bool addService(const String &type, const String &protocol, uint16_t port);

  // Remove a service.
  bool removeService(const String &type, const String &protocol, uint16_t port);

 private:
  // Find the slot for the given service. This returns -1 if the service could
  // not be found.
  int findService(const String &type, const String &protocol, uint16_t port);

  struct netif *netif_;
  String host_;

  // Holds information about all the slots.
  std::unique_ptr<std::tuple<String, String, uint16_t>>
      slots_[MDNS_MAX_SERVICES];
};

}  // namespace network
}  // namespace qindesign

extern qindesign::network::MDNSClass MDNS;

#endif  // QNE_MDNS_H_
