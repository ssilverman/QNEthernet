// SPDX-FileCopyrightText: (c) 2021-2023 Shawn Silverman <shawn@pobox.com>
// SPDX-License-Identifier: MIT

// QMMDNS.cpp implements MDNS.
// This file is part of the QNEthernet library.

#include "QNMDNS.h"

// C++ includes
#include <algorithm>

#include <pgmspace.h>

#include "lwip/apps/mdns.h"

#if LWIP_MDNS_RESPONDER

namespace qindesign {
namespace network {

MDNSClass &MDNSClass::instance() {
  static MDNSClass instance;
  return instance;
}

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
static bool netifAdded = false;

FLASHMEM MDNSClass::~MDNSClass() {
  end();
}

bool MDNSClass::begin(const char *hostname) {
  if (netif_default == nullptr) {
    // Return false for no netif
    return false;
  }

  if (!initialized) {
    mdns_resp_init();
    initialized = true;
  }

  // Treat nullptr hostname as not allowed
  if (hostname == nullptr) {
    return false;
  }

  if (netifAdded) {
    if (hostname_ == hostname) {
      return true;
    }
    end();
  }
  if (mdns_resp_add_netif(netif_default, hostname) != ERR_OK) {
    return false;
  }
  netifAdded = true;
  netif_ = netif_default;
  hostname_ = hostname;
  return true;
}

void MDNSClass::end() {
  if (netifAdded) {
    mdns_resp_remove_netif(netif_);
    netifAdded = false;
    netif_ = nullptr;
    hostname_ = "";
  }
}

void MDNSClass::restart() {
  if (!netifAdded) {
    return;
  }
  mdns_resp_restart(netif_);
}

// toProto converts a protocol to a protocol enum. This returns DNSSD_PROTO_TCP
// for "_tcp" and DNSSD_PROTO_UDP for all else.
static enum mdns_sd_proto toProto(const char *protocol) {
  if (String{protocol}.equalsIgnoreCase("_tcp")) {
    return DNSSD_PROTO_TCP;
  } else {
    return DNSSD_PROTO_UDP;
  }
}

bool MDNSClass::addService(const char *type, const char *protocol,
                           uint16_t port) {
  return addService(hostname_.c_str(), type, protocol, port, nullptr);
}

bool MDNSClass::addService(const char *name, const char *type,
                           const char *protocol, uint16_t port) {
  return addService(name, type, protocol, port, nullptr);
}

bool MDNSClass::addService(const char *type, const char *protocol,
                           uint16_t port,
                           std::vector<String> (*getTXTFunc)(void)) {
  return addService(hostname_.c_str(), type, protocol, port, getTXTFunc);
}

bool MDNSClass::addService(const char *name, const char *type,
                           const char *protocol, uint16_t port,
                           std::vector<String> (*getTXTFunc)(void)) {
  if (!netifAdded) {
    // Return false for no netif
    return false;
  }

  int8_t slot = mdns_resp_add_service(netif_, name, type,
                                      toProto(protocol), port, &srv_txt,
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

int MDNSClass::findService(const char *name, const char *type,
                           const char *protocol, uint16_t port) {
  Service service{true, name, type, protocol, port, nullptr};
  for (int i = 0; i < maxServices(); i++) {
    if (slots_[i] == service) {
      return i;
    }
  }
  return -1;
}

bool MDNSClass::removeService(const char *type, const char *protocol,
                              uint16_t port) {
  return removeService(hostname_.c_str(), type, protocol, port);
}

bool MDNSClass::removeService(const char *name, const char *type,
                              const char *protocol, uint16_t port) {
  if (!netifAdded) {
    // Return false for no netif
    return false;
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

void MDNSClass::announce() const {
  if (!netifAdded) {
    return;
  }
  mdns_resp_announce(netif_);
}

}  // namespace network
}  // namespace qindesign

#endif  // LWIP_MDNS_RESPONDER
