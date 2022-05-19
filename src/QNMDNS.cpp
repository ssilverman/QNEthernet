// SPDX-FileCopyrightText: (c) 2021-2022 Shawn Silverman <shawn@pobox.com>
// SPDX-License-Identifier: MIT

// QMMDNS.cpp implements MDNS.
// This file is part of the QNEthernet library.

#include "QNMDNS.h"

// C++ includes
#include <algorithm>

#include "lwip/apps/mdns.h"

#if LWIP_MDNS_RESPONDER

namespace qindesign {
namespace network {

// Recommended DNS TTL value, in seconds, per RFC 6762 "Multicast DNS".
static constexpr uint32_t kTTL = 120;

// Define the singleton instance.
MDNSClass MDNSClass::instance_;

// A reference to the singleton.
MDNSClass &MDNS = MDNSClass::instance();

static void srv_txt(struct mdns_service *service, void *txt_userdata) {
  if (txt_userdata == nullptr) {
    return;
  }

  auto fn = reinterpret_cast<std::vector<String> (*)()>(txt_userdata);
  std::vector<String> list = fn();
  if (list.empty()) {
    return;
  }

  for (const auto &item : list) {
    const char *txt = item.c_str();
    uint8_t len = std::min(item.length(), 63U);
    err_t res = mdns_resp_add_service_txtitem(service, txt, len);
    LWIP_ERROR("mdns add service txt failed\n", (res == ERR_OK), return );
  }
}

static bool initialized = false;

MDNSClass::~MDNSClass() {
  end();
}

bool MDNSClass::begin(const String &hostname) {
  netif_ = netif_default;
  if (netif_ == nullptr) {
    return false;
  }

  if (!initialized) {
    mdns_resp_init();
    initialized = true;
  }

  if (mdns_resp_add_netif(netif_, hostname.c_str(), kTTL) != ERR_OK) {
    netif_ = nullptr;
    return false;
  }
  hostname_ = hostname;
  return true;
}

bool MDNSClass::end() {
  if (netif_ == nullptr) {
    // Return true for no netif
    return true;
  }
  bool retval = (mdns_resp_remove_netif(netif_) == ERR_OK);
  netif_ = nullptr;
  return retval;
}

// toProto converts a protocol to a protocol enum. This returns DNSSD_PROTO_TCP
// for "_tcp" and DNSSD_PROTO_UDP for all else.
enum mdns_sd_proto toProto(const String &protocol) {
  if (protocol.equalsIgnoreCase("_tcp")) {
    return DNSSD_PROTO_TCP;
  } else {
    return DNSSD_PROTO_UDP;
  }
}

bool MDNSClass::addService(const String &type, const String &protocol,
                           uint16_t port) {
  return addService(hostname_, type, protocol, port, nullptr);
}

bool MDNSClass::addService(const String &name, const String &type,
                          const String &protocol, uint16_t port) {
  return addService(name, type, protocol, port, nullptr);
}

bool MDNSClass::addService(const String &type, const String &protocol,
                           uint16_t port,
                           std::vector<String> (*getTXTFunc)(void)) {
  return addService(hostname_, type, protocol, port, getTXTFunc);
}

bool MDNSClass::addService(const String &name, const String &type,
                           const String &protocol, uint16_t port,
                           std::vector<String> (*getTXTFunc)(void)) {
  if (netif_ == nullptr) {
    return false;
  }

  int8_t slot = mdns_resp_add_service(netif_, hostname_.c_str(), type.c_str(),
                                      toProto(protocol), port, kTTL, &srv_txt,
                                      reinterpret_cast<void *>(getTXTFunc));
  if (slot < 0 || maxServices() <= slot) {
    if (slot >= 0) {
      // Remove if the addition was successful but we couldn't add it
      mdns_resp_del_service(netif_, slot);
    }
    return false;
  }

  Service service{true, name, type, protocol, port, getTXTFunc};
  slots_[slot] = service;
  return true;
}

int MDNSClass::findService(const String &name, const String &type,
                           const String &protocol, uint16_t port) {
  Service service{true, name, type, protocol, port, nullptr};
  for (int i = 0; i < maxServices(); i++) {
    if (slots_[i] == service) {
      return i;
    }
  }
  return -1;
}

bool MDNSClass::removeService(const String &type, const String &protocol,
                              uint16_t port) {
  return removeService(hostname_, type, protocol, port);
}

bool MDNSClass::removeService(const String &name, const String &type,
                              const String &protocol, uint16_t port) {
  if (netif_ == nullptr) {
    // Return true for no netif
    return true;
  }

  // Find a matching service
  int found = findService(name, type, protocol, port);
  if (found < 0) {
    return false;
  }
  if (found < maxServices()) {
    slots_[found].reset();
  }
  return (mdns_resp_del_service(netif_, found) == ERR_OK);
}

uint32_t MDNSClass::ttl() const {
  return kTTL;
}

void MDNSClass::announce() const {
  if (netif_ == nullptr) {
    return;
  }
  mdns_resp_announce(netif_);
}

}  // namespace network
}  // namespace qindesign

#endif  // LWIP_MDNS_RESPONDER
