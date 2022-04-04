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

EthernetServer::EthernetServer(uint16_t port)
    : port_(port),
      listening_(false) {}

EthernetServer::~EthernetServer() {
}

void EthernetServer::begin() {
  begin(false);
}

void EthernetServer::begin(bool reuse) {
  listening_ = internal::ConnectionManager::instance().listen(port_, reuse);
}

bool EthernetServer::end() const {
  return internal::ConnectionManager::instance().stopListening(port_);
}

EthernetClient EthernetServer::accept() const {
  auto conn = internal::ConnectionManager::instance().findConnected(port_);
  EthernetClass::loop();
  if (conn != nullptr) {
    internal::ConnectionManager::instance().remove(conn);
    return EthernetClient{conn};
  }
  return EthernetClient{};
}

EthernetClient EthernetServer::available() const {
  auto conn = internal::ConnectionManager::instance().findAvailable(port_);
  EthernetClass::loop();
  if (conn != nullptr) {
    return EthernetClient{conn};
  }
  return EthernetClient{};
}

EthernetServer::operator bool() {
  if (!listening_) {
    return false;
  }
  listening_ = internal::ConnectionManager::instance().isListening(port_);
  return listening_;
}

size_t EthernetServer::write(uint8_t b) {
  return internal::ConnectionManager::instance().write(port_, b);
}

size_t EthernetServer::write(const uint8_t *buffer, size_t size) {
  return internal::ConnectionManager::instance().write(port_, buffer, size);
}

int EthernetServer::availableForWrite() {
  return internal::ConnectionManager::instance().availableForWrite(port_);
}

void EthernetServer::flush() {
  internal::ConnectionManager::instance().flush(port_);
}

}  // namespace network
}  // namespace qindesign
