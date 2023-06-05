// SPDX-FileCopyrightText: (c) 2021-2023 Shawn Silverman <shawn@pobox.com>
// SPDX-License-Identifier: MIT

// QNEthernetServer.cpp contains the EthernetServer implementation.
// This file is part of the QNEthernet library.

#include "QNEthernetServer.h"

// C++ includes
#include <memory>

#include "QNEthernet.h"
#include "internal/ConnectionManager.h"

namespace qindesign {
namespace network {

EthernetServer::EthernetServer()
#if LWIP_TCP
    : port_(-1),
      reuse_(false),
      listening_(false)
#endif  // LWIP_TCP
{}

EthernetServer::EthernetServer(uint16_t port)
#if LWIP_TCP
    : port_(port),
      reuse_(false),
      listening_(false)
#endif  // LWIP_TCP
{}

EthernetServer::~EthernetServer() {
  end();
}

void EthernetServer::begin() {
#if LWIP_TCP
  if (port_ < 0) {
    return;
  }
  begin(static_cast<uint16_t>(port_), false);
#endif  // LWIP_TCP
}

bool EthernetServer::beginWithReuse() {
#if LWIP_TCP
  if (port_ < 0) {
    return false;
  }
  return begin(static_cast<uint16_t>(port_), true);
#else
  return false;
#endif  // LWIP_TCP
}

bool EthernetServer::begin(uint16_t port) {
#if LWIP_TCP
  return begin(port, false);
#else
  return false;
#endif  // LWIP_TCP
}

bool EthernetServer::beginWithReuse(uint16_t port) {
#if LWIP_TCP
  return begin(port, true);
#else
  return false;
#endif  // LWIP_TCP
}

#if LWIP_TCP
bool EthernetServer::begin(uint16_t port, bool reuse) {
  // Only call end() if parameters have changed
  if (listening_) {
    if (port_ == port && reuse_ == reuse) {
      return true;
    }
    end();  // TODO: Should we call end() only if the new begin is successful?
  }

  // Only change the port if listening was successful
  listening_ = internal::ConnectionManager::instance().listen(port, reuse);
  if (listening_) {
    port_ = port;
    reuse_ = reuse;
  }
  return listening_;
}
#endif  // LWIP_TCP

void EthernetServer::end() {
#if LWIP_TCP
  if (listening_) {
    listening_ = false;
    internal::ConnectionManager::instance().stopListening(port_);
  }
#endif  // LWIP_TCIP
}

EthernetClient EthernetServer::accept() const {
#if LWIP_TCP
  if (port_ >= 0) {
    auto conn = internal::ConnectionManager::instance().findConnected(port_);
    Ethernet.loop();
    if (conn != nullptr) {
      internal::ConnectionManager::instance().remove(conn);
      return EthernetClient{conn};
    }
  }
#endif  // LWIP_TCP
  return EthernetClient{};
}

EthernetClient EthernetServer::available() const {
#if LWIP_TCP
  if (port_ >= 0) {
    auto conn = internal::ConnectionManager::instance().findAvailable(port_);
    Ethernet.loop();
    if (conn != nullptr) {
      return EthernetClient{conn};
    }
  }
#endif  // LWIP_TCP
  return EthernetClient{};
}

EthernetServer::operator bool() const {
#if LWIP_TCP
  return listening_;
#else
  return false;
#endif  // LWIP_TCP
}

size_t EthernetServer::write(uint8_t b) {
#if LWIP_TCP
  if (port_ < 0) {
    return 1;
  }
  return internal::ConnectionManager::instance().write(port_, b);
#else
  return 0;
#endif  // LWIP_TCP
}

size_t EthernetServer::write(const uint8_t *buffer, size_t size) {
#if LWIP_TCP
  if (port_ < 0) {
    return size;
  }
  return internal::ConnectionManager::instance().write(port_, buffer, size);
#else
  return 0;
#endif  // LWIP_TCP
}

int EthernetServer::availableForWrite() {
#if LWIP_TCP
  if (port_ < 0) {
    return 0;
  }
  return internal::ConnectionManager::instance().availableForWrite(port_);
#else
  return 0;
#endif  // LWIP_TCP
}

void EthernetServer::flush() {
#if LWIP_TCP
  if (port_ < 0) {
    return;
  }
  internal::ConnectionManager::instance().flush(port_);
#endif  // LWIP_TCP
}

}  // namespace network
}  // namespace qindesign
