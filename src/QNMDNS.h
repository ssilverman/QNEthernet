// SPDX-FileCopyrightText: (c) 2021-2023 Shawn Silverman <shawn@pobox.com>
// SPDX-License-Identifier: MIT

// QNMDNS.h defines an mDNS implementation.
// This file is part of the QNEthernet library.

#ifndef QNETHERNET_MDNS_H_
#define QNETHERNET_MDNS_H_

// C++ includes
#include <cstdint>
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
  static MDNSClass &instance();

  // MDNSClass is neither copyable nor movable
  MDNSClass(const MDNSClass &) = delete;
  MDNSClass &operator=(const MDNSClass &) = delete;

  // Returns the maximum number of services this can support.
  static constexpr int maxServices() {
    return MDNS_MAX_SERVICES;
  }

  // Starts the mDNS responder and uses the given hostname as the name. This
  // returns whether the call was successful.
  //
  // This first calls end() if the responder is already running and the hostname
  // is different.
  bool begin(const char *hostname);

  // Stops the mDNS responder.
  void end();

  // Returns the hostname. This will return an empty string if the responder
  // is not currently running.
  String hostname() const {
    return hostname_;
  }

  // Restarts the responder. This is useful when the cable has been disconnected
  // for a while and then reconnected.
  //
  // This isn't normally needed because the responder already watches for
  // link reconnect.
  void restart();

  // Adds a service. The protocol will be set to "_udp" for anything other than
  // "_tcp". The strings should have a "_" prefix.
  //
  // No TXT records are added.
  //
  // This calls `addService(name, type, protocol, port, nullptr)` with the host
  // name as the service name.
  bool addService(const char *type, const char *protocol, uint16_t port);

  // Adds a service. The protocol will be set to "_udp" for anything other than
  // "_tcp". The strings should have a "_" prefix.
  //
  // No TXT records are added.
  //
  // This calls `addService(name, type, protocol, port, nullptr)`.
  bool addService(const char *name, const char *type,
                  const char *protocol, uint16_t port);

  // Adds a service. The protocol will be set to "_udp" for anything other than
  // "_tcp". The strings should have a "_" prefix.
  //
  // This calls `addService(name, type, protocol, port, getTXTFunc)` with the
  // host name as the service name.
  bool addService(const char *type, const char *protocol, uint16_t port,
                  std::vector<String> (*getTXTFunc)(void));

  // Adds a service. The protocol will be set to "_udp" for anything other than
  // "_tcp". The strings should have a "_" prefix.
  //
  // The `getTXTFunc` parameter is the function associated with this service to
  // retreive its TXT record parts. The entire TXT record can be a can be a
  // maximum of 255 bytes, including length bytes, and each item in the record
  // can be a maximum of 63 bytes. The function may be NULL, in which case no
  // items are added.
  bool addService(const char *name, const char *type,
                  const char *protocol, uint16_t port,
                  std::vector<String> (*getTXTFunc)(void));

  // Removes a service. The host name is used as the service name. This will
  // return whether the service was removed.
  bool removeService(const char *type, const char *protocol, uint16_t port);

  // Removes a service and returns whether the service was removed.
  bool removeService(const char *name, const char *type,
                     const char *protocol, uint16_t port);

  // Returns whether mDNS has been started.
  explicit operator bool() const {
    return netif_ != nullptr;
  }

  // The following functions are for periodically re-announcing the services.
  // They are not normally needed.

  // Performs an announcement. This isn't usually necessary for
  // normal operation.
  void announce() const;

 private:
  struct Service final {
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
  ~MDNSClass();

  // Finds the slot for the given service. This returns -1 if the service could
  // not be found.
  int findService(const char *name, const char *type,
                  const char *protocol, uint16_t port);

  struct netif *netif_ = nullptr;
  String hostname_;

  // Holds information about all the slots.
  Service slots_[MDNS_MAX_SERVICES];
};

}  // namespace network
}  // namespace qindesign

#endif  // LWIP_MDNS_RESPONDER

#endif  // QNETHERNET_MDNS_H_
