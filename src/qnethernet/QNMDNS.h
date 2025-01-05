// SPDX-FileCopyrightText: (c) 2021-2024 Shawn Silverman <shawn@pobox.com>
// SPDX-License-Identifier: AGPL-3.0-or-later

// QNMDNS.h defines an mDNS implementation.
// This file is part of the QNEthernet library.

#pragma once

#include "lwip/apps/mdns_opts.h"

#if LWIP_MDNS_RESPONDER

// C++ includes
#include <cstdint>
#include <vector>

#include <WString.h>

#include "lwip/apps/mdns.h"
#include "lwip/netif.h"
#include "qnethernet/StaticInit.h"

namespace qindesign {
namespace network {

// MDNS provides mDNS responder functionality.
class MDNSClass final {
 public:
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
  // This renames the current state if the responder is already running and the
  // hostname is different.
  //
  // If this returns false and there was an error then errno will be set.
  bool begin(const char *hostname);

  // Stops the mDNS responder.
  //
  // If there was an error then errno will be set.
  void end();

  // Returns the hostname. This will return an empty string if the responder
  // is not currently running.
  const char *hostname() const {
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
  //
  // If this returns false and there was an error then errno will be set.
  bool addService(const char *type, const char *protocol, uint16_t port);

  // Adds a service. The protocol will be set to "_udp" for anything other than
  // "_tcp". The strings should have a "_" prefix.
  //
  // No TXT records are added.
  //
  // This calls `addService(name, type, protocol, port, nullptr)`.
  //
  // If this returns false and there was an error then errno will be set.
  bool addService(const char *name, const char *type,
                  const char *protocol, uint16_t port);

  // Adds a service. The protocol will be set to "_udp" for anything other than
  // "_tcp". The strings should have a "_" prefix.
  //
  // This calls `addService(name, type, protocol, port, getTXTFunc)` with the
  // host name as the service name.
  //
  // If this returns false and there was an error then errno will be set.
  bool addService(const char *type, const char *protocol, uint16_t port,
                  std::vector<String> (*getTXTFunc)());

  // Adds a service. The protocol will be set to "_udp" for anything other than
  // "_tcp". The strings should have a "_" prefix.
  //
  // The `getTXTFunc` parameter is the function associated with this service to
  // retreive its TXT record parts. The entire TXT record can be a can be a
  // maximum of 255 bytes, including length bytes, and each item in the record
  // can be a maximum of 63 bytes. The function may be NULL, in which case no
  // items are added.
  //
  // If this returns false and there was an error then errno will be set.
  bool addService(const char *name, const char *type,
                  const char *protocol, uint16_t port,
                  std::vector<String> (*getTXTFunc)());

  // Removes a service. The host name is used as the service name. This will
  // return whether the service was removed.
  //
  // If this returns false and there was an error then errno will be set.
  bool removeService(const char *type, const char *protocol, uint16_t port);

  // Removes a service and returns whether the service was removed.
  //
  // If this returns false and there was an error then errno will be set.
  bool removeService(const char *name, const char *type,
                     const char *protocol, uint16_t port);

  // Returns whether mDNS has been started.
  explicit operator bool() const;

  // The following functions are for periodically re-announcing the services.
  // They are not normally needed.

  // Performs an announcement. This isn't usually necessary for
  // normal operation.
  //
  // If there was an error then errno will be set.
  void announce() const;

 private:
  class Service final {
   public:
    Service();
    ~Service() = default;

    void set(bool valid, const char *name, const char *type,
             enum mdns_sd_proto proto, uint16_t port,
             std::vector<String> (*getTXTFunc)());
    bool equals(bool valid, const char *name, const char *type,
                enum mdns_sd_proto proto, uint16_t port) const;

    // Resets this service to be invalid and empty.
    void reset();

   private:
    bool valid_ = false;
    char name_[MDNS_LABEL_MAXLEN + 1];
    char type_[MDNS_LABEL_MAXLEN + 1];
    enum mdns_sd_proto proto_ = mdns_sd_proto::DNSSD_PROTO_UDP;
    uint16_t port_ = 0;
    std::vector<String> (*getTXTFunc_)() = nullptr;
  };

  MDNSClass();
  ~MDNSClass();

  // Finds the slot for the given service. This returns -1 if the service could
  // not be found.
  int findService(const char *name, const char *type,
                  const char *protocol, uint16_t port);

  struct netif *netif_;
  char hostname_[MDNS_LABEL_MAXLEN + 1];

  // Holds information about all the slots.
  Service slots_[MDNS_MAX_SERVICES];

  friend class StaticInit<MDNSClass>;
};

// Instance for interacting with mDNS.
STATIC_INIT_DECL(MDNSClass, MDNS);

}  // namespace network
}  // namespace qindesign

#endif  // LWIP_MDNS_RESPONDER
