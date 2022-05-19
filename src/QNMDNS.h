// SPDX-FileCopyrightText: (c) 2021-2022 Shawn Silverman <shawn@pobox.com>
// SPDX-License-Identifier: MIT

// QNMDNS.h defines an mDNS implementation.
// This file is part of the QNEthernet library.

#ifndef QNE_MDNS_H_
#define QNE_MDNS_H_

// C++ includes
#include <vector>

#include <WString.h>

#include "lwip/apps/mdns_opts.h"
#include "lwip/netif.h"

#if LWIP_MDNS_RESPONDER

namespace qindesign {
namespace network {

// MDNS provides mDNS responder functionality.
class MDNSClass final {
 public:
  // Accesses the singleton instance.
  static MDNSClass &instance() {
    return instance_;
  }

  // MDNSClass is neither copyable nor movable
  MDNSClass(const MDNSClass &) = delete;
  MDNSClass &operator=(const MDNSClass &) = delete;

  ~MDNSClass();

  // Returns the maximum number of services this can support.
  static constexpr int maxServices() {
    return MDNS_MAX_SERVICES;
  }

  // Starts the mDNS responder and uses the given hostname as the name. This
  // returns whether the call was successful.
  bool begin(const String &hostname);

  // Attempts to stop the mDNS responder. Returns whether the call
  // was successful.
  bool end();

  // Adds a service. The protocol will be set to "_udp" for anything other than
  // "_tcp". The strings should have a "_" prefix.
  //
  // No TXT records are added.
  //
  // This calls `addService(name, type, protocol, port, nullptr)` with the host
  // name as the service name.
  bool addService(const String &type, const String &protocol, uint16_t port);

  // Adds a service. The protocol will be set to "_udp" for anything other than
  // "_tcp". The strings should have a "_" prefix.
  //
  // No TXT records are added.
  //
  // This calls `addService(name, type, protocol, port, nullptr)`.
  bool addService(const String &name, const String &type,
                  const String &protocol, uint16_t port);

  // Adds a service. The protocol will be set to "_udp" for anything other than
  // "_tcp". The strings should have a "_" prefix.
  //
  // This calls `addService(name, type, protocol, port, getTXTFunc)` with the
  // host name as the service name.
  bool addService(const String &type, const String &protocol, uint16_t port,
                  std::vector<String> (*getTXTFunc)(void));

  // Adds a service. The protocol will be set to "_udp" for anything other than
  // "_tcp". The strings should have a "_" prefix.
  //
  // The `getTXTFunc` parameter is the function associated with this service to
  // retreive its TXT record parts. The entire TXT record can be a can be a
  // maximum of 255 bytes, including length bytes, and each item in the record
  // can be a maximum of 63 bytes. The function may be NULL, in which case no
  // items are added.
  bool addService(const String &name, const String &type,
                  const String &protocol, uint16_t port,
                  std::vector<String> (*getTXTFunc)(void));

  // Removes a service. The host name is used as the service name.
  bool removeService(const String &type, const String &protocol, uint16_t port);

  // Removes a service.
  bool removeService(const String &name, const String &type,
                     const String &protocol, uint16_t port);

  // The following functions are for periodically re-announcing the services.
  // Service entries seem to disappear after the TTL and sometimes earlier, so a
  // re-announcement may be required.

  // Returns the TTL, in seconds.
  uint32_t ttl() const;

  // Performs an announcement. This isn't usually necessary for
  // normal operation.
  void announce() const;

 private:
  struct Service {
    bool operator==(const Service &other) {
      if (!valid || !other.valid) {
        // Invalid services compare unequal
        return false;
      }
      if (this == &other) {
        return true;
      }

      // Don't compare the functions
      return (name == other.name) &&
             (type == other.type) &&
             (protocol == other.protocol) &&
             (port == other.port);
    }

    // Resets this service to be invalid and empty.
    void reset() {
      valid = false;
      type = "";
      protocol = "";
      port = 0;
      getTXTFunc = nullptr;
    }

    bool valid = false;
    String name;
    String type;
    String protocol;
    uint16_t port;
    std::vector<String> (*getTXTFunc)(void);
  };

  MDNSClass() = default;

  // Finds the slot for the given service. This returns -1 if the service could
  // not be found.
  int findService(const String &name, const String &type,
                  const String &protocol, uint16_t port);

  struct netif *netif_;
  String hostname_;

  // Holds information about all the slots.
  Service slots_[MDNS_MAX_SERVICES];

  // The singleton instance.
  static MDNSClass instance_;
};

}  // namespace network
}  // namespace qindesign

#endif  // LWIP_MDNS_RESPONDER

#endif  // QNE_MDNS_H_
