// SPDX-FileCopyrightText: (c) 2021-2022 Shawn Silverman <shawn@pobox.com>
// SPDX-License-Identifier: MIT

// QNEthernetServer.cpp contains the EthernetServer implementation.
// This file is part of the QNEthernet library.

#include "QNEthernetServer.h"

// C++ includes
#include <memory>

#include "QNEthernet.h"
#include "QNEthernetClient.h"
#include "internal/ConnectionManager.h"

namespace qindesign {
namespace network {

EthernetServer::EthernetServer()
    : port_(-1),
      listening_(false) {}

EthernetServer::EthernetServer(uint16_t port)
    : port_(port),
      listening_(false) {}

EthernetServer::~EthernetServer() {
  end();
}

void EthernetServer::begin() {
  begin(false);
}

bool EthernetServer::begin(bool reuse) {
  if (port_ < 0) {
    return false;
  }
  return begin(port_, reuse);
}

bool EthernetServer::begin(uint16_t port) {
  return begin(port, false);
}

bool EthernetServer::begin(uint16_t port, bool reuse) {
  if (port_ >= 0 && port_ != port) {
    end();  // TODO: Should we call end() only if the new begin is successful?
  }
  port_ = port;
  listening_ = internal::ConnectionManager::instance().listen(port_, reuse);
  return listening_;
}

bool EthernetServer::end() {
  if (port_ < 0) {
    return true;
  }
  if (internal::ConnectionManager::instance().stopListening(port_)) {
    listening_ = false;
    return true;
  }
  return false;
}

EthernetClient EthernetServer::accept() const {
  if (port_ >= 0) {
    auto conn = internal::ConnectionManager::instance().findConnected(port_);
    EthernetClass::loop();
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
    EthernetClass::loop();
    if (conn != nullptr) {
      return EthernetClient{conn};
    }
  }
  return EthernetClient{};
}

EthernetServer::operator bool() {
  if (!listening_ || port_ < 0) {
    return false;
  }
  listening_ = internal::ConnectionManager::instance().isListening(port_);
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
