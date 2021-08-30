// SPDX-FileCopyrightText: (c) 2021 Shawn Silverman <shawn@pobox.com>
// SPDX-License-Identifier: MIT

// QNEthernetServer.cpp contains the EthernetServer implementation.
// This file is part of the QNEthernet library.

#include "QNEthernetServer.h"

// C++ includes
#include <algorithm>
#include <utility>

#include "ConnectionManager.h"
#include "QNEthernetClient.h"

namespace qindesign {
namespace network {

EthernetServer::EthernetServer(uint16_t port)
    : port_(port),
      listening_(false) {}

EthernetServer::~EthernetServer() {
}

void EthernetServer::begin() {
  listening_ = ConnectionManager::instance().listen(port_);
}

EthernetClient EthernetServer::accept() {
  std::shared_ptr<ConnectionHolder> conn =
      ConnectionManager::instance().findConnected(port_);
  if (conn != nullptr) {
    ConnectionManager::instance().remove(conn);
    return EthernetClient{conn};
  }
  return EthernetClient{};
}

EthernetClient EthernetServer::available() {
  std::shared_ptr<ConnectionHolder> conn =
      ConnectionManager::instance().findAvailable(port_);
  if (conn != nullptr) {
    return EthernetClient{conn};
  }
  return EthernetClient{};
}

EthernetServer::operator bool() {
  if (!listening_) {
    return false;
  }
  listening_ = ConnectionManager::instance().isListening(port_);
  return listening_;
}

size_t EthernetServer::write(uint8_t b) {
  return ConnectionManager::instance().write(port_, b);
}

size_t EthernetServer::write(const uint8_t *buffer, size_t size) {
  return ConnectionManager::instance().write(port_, buffer, size);
}

int EthernetServer::availableForWrite() {
  return 0;
}

void EthernetServer::flush() {
  ConnectionManager::instance().flush(port_);
}

}  // namespace network
}  // namespace qindesign
