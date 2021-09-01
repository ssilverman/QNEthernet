// SPDX-FileCopyrightText: (c) 2021 Shawn Silverman <shawn@pobox.com>
// SPDX-License-Identifier: MIT

// QNMDNS.h defines an mDNS implementation.
// This file is part of the QNEthernet library.

#ifndef QNE_MDNS_H_
#define QNE_MDNS_H_

// C++ includes
#include <vector>

#include <WString.h>
#include <lwip/netif.h>
#include <lwip/apps/mdns_opts.h>

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
  // the service name.
  //
  // The `getTXTFunc` parameter is the function associated with this service to
  // retreive its TXT record parts. The entire TXT record can be a can be a
  // maximum of 255 bytes, including length bytes, and each item in the record
  // can be a maximum of 63 bytes. The function may be NULL, in which case no
  // items are added.
  bool addService(const String &type, const String &protocol, uint16_t port,
                  std::vector<String> (*getTXTFunc)(void) = nullptr);

  // Remove a service.
  bool removeService(const String &type, const String &protocol, uint16_t port);

  // The following functions are for periodically re-announcing the services.
  // Service entries seem to disappear after the TTL and sometimes earlier, so a
  // re-announcement may be required.

  // Return the TTL, in seconds.
  uint32_t ttl() const;

  // Perform the announcement.
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
      return (type == other.type) &&
             (protocol == other.protocol) &&
             (port == other.port);
    }

    // Reset this service to be invalid and empty.
    void reset() {
      valid = false;
      type = "";
      protocol = "";
      port = 0;
      getTXTFunc = nullptr;
    }

    bool valid = false;
    String type;
    String protocol;
    uint16_t port;
    std::vector<String> (*getTXTFunc)(void);
  };

  // Find the slot for the given service. This returns -1 if the service could
  // not be found.
  int findService(const String &type, const String &protocol, uint16_t port);

  struct netif *netif_;
  String host_;

  // Holds information about all the slots.
  Service slots_[MDNS_MAX_SERVICES];
};

extern MDNSClass MDNS;

}  // namespace network
}  // namespace qindesign

#endif  // QNE_MDNS_H_
