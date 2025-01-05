// SPDX-FileCopyrightText: (c) 2021-2024 Shawn Silverman <shawn@pobox.com>
// SPDX-License-Identifier: AGPL-3.0-or-later

// QMMDNS.cpp implements MDNS.
// This file is part of the QNEthernet library.

#include "qnethernet/QNMDNS.h"

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

static void srv_txt(struct mdns_service *const service,
                    void *const txt_userdata) {
  // TODO: Not clear yet why we need at least an empty TXT record for SRV to appear
  if (txt_userdata == nullptr) {
    const err_t err = mdns_resp_add_service_txtitem(service, "", 0);
    errno = err_to_errno(err);
    return;
  }

  const auto fn = reinterpret_cast<std::vector<String> (*)()>(txt_userdata);
  const std::vector<String> list = fn();
  if (list.empty()) {
    const err_t err = mdns_resp_add_service_txtitem(service, "", 0);
    errno = err_to_errno(err);
    return;
  }

  for (const auto &item : list) {
    const char *const txt = item.c_str();
    const uint8_t len =
        std::min(item.length(), (unsigned int)(MDNS_LABEL_MAXLEN));
    const err_t res = mdns_resp_add_service_txtitem(service, txt, len);
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

bool MDNSClass::begin(const char *const hostname) {
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

  if (netifAdded) {
    if (std::strlen(hostname_) == std::strlen(hostname) &&
        std::strncmp(hostname_, hostname, std::strlen(hostname_)) == 0) {
      return true;
    }
    const err_t err = mdns_resp_rename_netif(netif_, hostname);
    if (err != ERR_OK) {
      errno = err_to_errno(err);
      return false;
    }
  } else {
    const err_t err = mdns_resp_add_netif(netif_default, hostname);
    if (err != ERR_OK) {
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
    const err_t err = mdns_resp_remove_netif(netif_);
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
static enum mdns_sd_proto toProto(const char *const protocol) {
  if (String{protocol}.equalsIgnoreCase("_tcp")) {
    return mdns_sd_proto::DNSSD_PROTO_TCP;
  } else {
    return mdns_sd_proto::DNSSD_PROTO_UDP;
  }
}

bool MDNSClass::addService(const char *const type, const char *const protocol,
                           uint16_t port) {
  return addService(hostname_, type, protocol, port, nullptr);
}

bool MDNSClass::addService(const char *const name, const char *const type,
                           const char *const protocol, const uint16_t port) {
  return addService(name, type, protocol, port, nullptr);
}

bool MDNSClass::addService(const char *const type, const char *const protocol,
                           const uint16_t port,
                           std::vector<String> (*const getTXTFunc)()) {
  return addService(hostname_, type, protocol, port, getTXTFunc);
}

bool MDNSClass::addService(const char *const name, const char *const type,
                           const char *const protocol, const uint16_t port,
                           std::vector<String> (*getTXTFunc)()) {
  if (!netifAdded) {
    // Return false for no netif
    errno = ENOTCONN;
    return false;
  }

  const enum mdns_sd_proto proto = toProto(protocol);

  const int8_t slot =
      mdns_resp_add_service(netif_, name, type, proto, port, &srv_txt,
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

  slots_[slot].set(true, name, type, proto, port, getTXTFunc);
  return true;
}

int MDNSClass::findService(const char *const name, const char *const type,
                           const char *const protocol, const uint16_t port) {
  for (int i = 0; i < maxServices(); i++) {
    if (slots_[i].equals(true, name, type, toProto(protocol), port)) {
      return i;
    }
  }
  return -1;
}

bool MDNSClass::removeService(const char *const type,
                              const char *const protocol,
                              const uint16_t port) {
  return removeService(hostname_, type, protocol, port);
}

bool MDNSClass::removeService(const char *const name, const char *const type,
                              const char *protocol, const uint16_t port) {
  if (!netifAdded) {
    // Return false for no netif
    return false;
  }

  // Find a matching service
  const int found = findService(name, type, protocol, port);
  if (found < 0) {
    return false;
  }
  if (found < maxServices()) {
    slots_[found].reset();
  }

  const err_t err = mdns_resp_del_service(netif_, found);
  if (err != ERR_OK) {
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

MDNSClass::Service::Service() {
  reset();
}

void MDNSClass::Service::set(bool valid, const char *name, const char *type,
                             enum mdns_sd_proto proto, uint16_t port,
                             std::vector<String> (*const getTXTFunc)()) {
  valid_ = valid;
  std::strncpy(name_, name, sizeof(name_) - 1);
  std::strncpy(type_, type, sizeof(type_) - 1);
  proto_ = proto;
  port_ = port;
  getTXTFunc_ = getTXTFunc;
}

bool MDNSClass::Service::equals(const bool valid, const char *const name,
                                const char *const type,
                                const enum mdns_sd_proto proto,
                                uint16_t port) const {
  if (!valid_ || !valid) {
    // Invalid services compare unequal
    return false;
  }

  // Don't compare the functions
  return (std::strncmp(name_, name, sizeof(name_)) == 0) &&
         (std::strncmp(type_, type, sizeof(type_)) == 0) &&
         (proto_ == proto) &&
         (port_ == port);
}

// Resets this service to be invalid and empty.
void MDNSClass::Service::reset() {
  valid_ = false;
  name_[0] = '\0';
  type_[0] = '\0';
  proto_ = mdns_sd_proto::DNSSD_PROTO_UDP;
  port_ = 0;
  getTXTFunc_ = nullptr;
}

}  // namespace network
}  // namespace qindesign

#endif  // LWIP_MDNS_RESPONDER
