// SPDX-FileCopyrightText: (c) 2021-2024 Shawn Silverman <shawn@pobox.com>
// SPDX-License-Identifier: AGPL-3.0-or-later

// QMMDNS.cpp implements MDNS.
// This file is part of the QNEthernet library.

#include "QNMDNS.h"

#if LWIP_MDNS_RESPONDER

// C++ includes
#include <algorithm>
#include <cerrno>
#include <cstring>

#include <avr/pgmspace.h>

#include "lwip/err.h"

#ifndef FLASHMEM
#define FLASHMEM
#endif  // !FLASHMEM

namespace qindesign {
namespace network {

// A reference to the singleton.
STATIC_INIT_DEFN(MDNSClass, MDNS);

static void srv_txt(struct mdns_service *service, void *txt_userdata) {
  // TODO: Not clear yet why we need at least an empty TXT record for SRV to appear
  if (txt_userdata == nullptr) {
    err_t err = mdns_resp_add_service_txtitem(service, "", 0);
    errno = err_to_errno(err);
    return;
  }

  auto fn = reinterpret_cast<std::vector<String> (*)()>(txt_userdata);
  std::vector<String> list = fn();
  if (list.empty()) {
    err_t err = mdns_resp_add_service_txtitem(service, "", 0);
    errno = err_to_errno(err);
    return;
  }

  for (const auto &item : list) {
    const char *txt = item.c_str();
    uint8_t len = std::min(item.length(), (unsigned int){MDNS_LABEL_MAXLEN});
    err_t res = mdns_resp_add_service_txtitem(service, txt, len);
    LWIP_ERROR("mdns add service txt failed\n", (res == ERR_OK),
               errno = err_to_errno(res);
               return);
  }
}

static bool initialized = false;
static bool netifAdded = false;

FLASHMEM MDNSClass::MDNSClass()
    : netif_(nullptr),
      hostname_{'\0'} {}

FLASHMEM MDNSClass::~MDNSClass() {
  end();
}

bool MDNSClass::begin(const char *hostname) {
  if (netif_default == nullptr) {
    // Return false for no netif
    errno = ENOTCONN;
    return false;
  }

  if (!initialized) {
    mdns_resp_init();
    initialized = true;
  }

  // Treat nullptr hostname as not allowed
  if (hostname == nullptr) {
    errno = EINVAL;
    return false;
  }

  err_t err;
  if (netifAdded) {
    if (std::strlen(hostname_) == std::strlen(hostname) &&
        std::strncmp(hostname_, hostname, std::strlen(hostname_)) == 0) {
      return true;
    }
    if ((err = mdns_resp_rename_netif(netif_, hostname)) != ERR_OK) {
      errno = err_to_errno(err);
      return false;
    }
  } else {
    if ((err = mdns_resp_add_netif(netif_default, hostname)) != ERR_OK) {
      errno = err_to_errno(err);
      return false;
    }

    netifAdded = true;
    netif_ = netif_default;
  }

  std::strncpy(hostname_, hostname, sizeof(hostname_) - 1);
  hostname_[sizeof(hostname_) - 1] = '\0';
  return true;
}

void MDNSClass::end() {
  if (netifAdded) {
    err_t err = mdns_resp_remove_netif(netif_);
    netifAdded = false;
    netif_ = nullptr;
    hostname_[0] = '\0';
    if (err != ERR_OK) {
      errno = err_to_errno(err);
    }
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
  return addService(hostname_, type, protocol, port, nullptr);
}

bool MDNSClass::addService(const char *name, const char *type,
                           const char *protocol, uint16_t port) {
  return addService(name, type, protocol, port, nullptr);
}

bool MDNSClass::addService(const char *type, const char *protocol,
                           uint16_t port,
                           std::vector<String> (*getTXTFunc)(void)) {
  return addService(hostname_, type, protocol, port, getTXTFunc);
}

bool MDNSClass::addService(const char *name, const char *type,
                           const char *protocol, uint16_t port,
                           std::vector<String> (*getTXTFunc)(void)) {
  if (!netifAdded) {
    // Return false for no netif
    errno = ENOTCONN;
    return false;
  }

  enum mdns_sd_proto proto = toProto(protocol);

  int8_t slot = mdns_resp_add_service(netif_, name, type,
                                      proto, port, &srv_txt,
                                      reinterpret_cast<void *>(getTXTFunc));
  if (slot < 0 || maxServices() <= slot) {
    if (slot >= 0) {
      // Remove if the addition was successful but we couldn't add it
      mdns_resp_del_service(netif_, slot);
      errno = ENOBUFS;
    } else {
      errno = err_to_errno(slot);
    }
    return false;
  }

  Service service{true, name, type, proto, port, getTXTFunc};
  slots_[slot] = service;
  return true;
}

int MDNSClass::findService(const char *name, const char *type,
                           const char *protocol, uint16_t port) {
  Service service{true, name, type, toProto(protocol), port, nullptr};
  for (int i = 0; i < maxServices(); i++) {
    if (slots_[i] == service) {
      return i;
    }
  }
  return -1;
}

bool MDNSClass::removeService(const char *type, const char *protocol,
                              uint16_t port) {
  return removeService(hostname_, type, protocol, port);
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

  err_t err;
  if ((err = mdns_resp_del_service(netif_, found)) != ERR_OK) {
    errno = err_to_errno(err);
    return false;
  }
  return true;
}

MDNSClass::operator bool() const {
  return netif_ != nullptr;
}

void MDNSClass::announce() const {
  if (!netifAdded) {
    errno = ENOTCONN;
    return;
  }
  mdns_resp_announce(netif_);
}

bool MDNSClass::Service::operator==(const Service &other) const {
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
         (proto == other.proto) &&
         (port == other.port);
}

// Resets this service to be invalid and empty.
void MDNSClass::Service::reset() {
  valid = false;
  name = "";
  type = "";
  proto = DNSSD_PROTO_UDP;
  port = 0;
  getTXTFunc = nullptr;
}

}  // namespace network
}  // namespace qindesign

#endif  // LWIP_MDNS_RESPONDER
