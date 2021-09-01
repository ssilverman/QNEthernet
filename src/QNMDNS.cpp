// SPDX-FileCopyrightText: (c) 2021 Shawn Silverman <shawn@pobox.com>
// SPDX-License-Identifier: MIT

// QMMDNS.cpp implements MDNS.
// This file is part of the QNEthernet library.

#include "QNMDNS.h"

// C++ includes
#include <algorithm>

#include <lwip/apps/mdns.h>

// Global definitions for Arduino
qindesign::network::MDNSClass MDNS;

namespace qindesign {
namespace network {

// Count array elements.
template <typename T, size_t N>
static constexpr size_t countof(const T (&)[N]) {
  return N;
}

// Recommended DNS TTL value, in seconds, per RFC 6762 "Multicast DNS".
static constexpr uint32_t kTTL = 120;

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

bool MDNSClass::begin(const String &host) {
  netif_ = netif_default;
  if (netif_ == nullptr) {
    return false;
  }

  mdns_resp_init();

  if (mdns_resp_add_netif(netif_, host.c_str(), kTTL) != ERR_OK) {
    netif_ = nullptr;
    return false;
  }
  host_ = host;
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
                           uint16_t port,
                           std::vector<String> (*getTXTFunc)(void)) {
  if (netif_ == nullptr) {
    return false;
  }

  int8_t slot = mdns_resp_add_service(netif_, host_.c_str(), type.c_str(),
                                      toProto(protocol), port, kTTL, &srv_txt,
                                      reinterpret_cast<void *>(getTXTFunc));
  if (slot < 0 || countof(slots_) <= static_cast<size_t>(slot)) {
    if (slot >= 0) {
      // Remove if the addition was successful but we couldn't add it
      mdns_resp_del_service(netif_, slot);
    }
    return false;
  }

  Service service{true, type, protocol, port, getTXTFunc};
  slots_[slot] = service;
  return true;
}

int MDNSClass::findService(const String &type, const String &protocol,
                           uint16_t port) {
  Service service{true, type, protocol, port};
  for (size_t i = 0; i < countof(slots_); i++) {
    if (slots_[i] == service) {
      return i;
    }
  }
  return -1;
}

bool MDNSClass::removeService(const String &type, const String &protocol,
                              uint16_t port) {
  if (netif_ == nullptr) {
    // Return true for no netif
    return true;
  }

  // Find a matching service
  int found = findService(type, protocol, port);
  if (found < 0) {
    return false;
  }
  if (static_cast<size_t>(found) < countof(slots_)) {
    slots_[found].reset();
  }
  return (mdns_resp_del_service(netif_, found) == ERR_OK);
}

}  // namespace network
}  // namespace qindesign
