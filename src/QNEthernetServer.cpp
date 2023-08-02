// SPDX-FileCopyrightText: (c) 2021-2023 Shawn Silverman <shawn@pobox.com>
// SPDX-License-Identifier: MIT

// QNEthernetServer.cpp contains the EthernetServer implementation.
// This file is part of the QNEthernet library.

#include "QNEthernetServer.h"

#if LWIP_TCP

// C++ includes
#include <memory>

#include "QNEthernet.h"
#include "internal/ConnectionManager.h"

namespace qindesign {
namespace network {

EthernetServer::EthernetServer()
    : port_(-1),
      reuse_(false),
      listening_(false) {}

EthernetServer::EthernetServer(uint16_t port)
    : port_(port),
      reuse_(false),
      listening_(false) {}

EthernetServer::~EthernetServer() {
  end();
}

void EthernetServer::begin() {
  if (port_ < 0) {
    return;
  }
  begin(static_cast<uint16_t>(port_), false);
}

bool EthernetServer::beginWithReuse() {
  if (port_ < 0) {
    return false;
  }
  return begin(static_cast<uint16_t>(port_), true);
}

bool EthernetServer::begin(uint16_t port) {
  return begin(port, false);
}

bool EthernetServer::beginWithReuse(uint16_t port) {
  return begin(port, true);
}

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

void EthernetServer::end() {
  if (listening_) {
    listening_ = false;
    internal::ConnectionManager::instance().stopListening(port_);
  }
  port_ = -1;
}

EthernetClient EthernetServer::accept() const {
  if (port_ >= 0) {
    auto conn = internal::ConnectionManager::instance().findConnected(port_);
    Ethernet.loop();
    if (conn != nullptr) {
      internal::ConnectionManager::instance().remove(conn);
      return EthernetClient{conn};
    }
  }
  return EthernetClient{};
}

EthernetClient EthernetServer::available() const {
  if (port_ >= 0) {
    auto conn = internal::ConnectionManager::instance().findAvailable(port_);
    Ethernet.loop();
    if (conn != nullptr) {
      return EthernetClient{conn};
    }
  }
  return EthernetClient{};
}

EthernetServer::operator bool() const {
  return listening_;
}

size_t EthernetServer::write(uint8_t b) {
  if (port_ < 0) {
    return 1;
  }
  return internal::ConnectionManager::instance().write(port_, b);
}

size_t EthernetServer::write(const uint8_t *buffer, size_t size) {
  if (port_ < 0) {
    return size;
  }
  return internal::ConnectionManager::instance().write(port_, buffer, size);
}

int EthernetServer::availableForWrite() {
  if (port_ < 0) {
    return 0;
  }
  return internal::ConnectionManager::instance().availableForWrite(port_);
}

void EthernetServer::flush() {
  if (port_ < 0) {
    return;
  }
  internal::ConnectionManager::instance().flush(port_);
}

}  // namespace network
}  // namespace qindesign

#endif  // LWIP_TCP
