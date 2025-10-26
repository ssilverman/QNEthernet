// SPDX-FileCopyrightText: (c) 2021-2025 Shawn Silverman <shawn@pobox.com>
// SPDX-License-Identifier: AGPL-3.0-or-later

// QNEthernetServer.cpp contains the EthernetServer implementation.
// This file is part of the QNEthernet library.

#include "qnethernet/QNEthernetServer.h"

#if LWIP_TCP

// C++ includes
#include <memory>

#include "QNEthernet.h"
#include "qnethernet/internal/ConnectionManager.h"

namespace qindesign {
namespace network {

EthernetServer::EthernetServer(const uint16_t port)
    : port_{true, port} {}

EthernetServer::~EthernetServer() {
  end();
}

int32_t EthernetServer::port() const {
  if (!port_.has_value) {
    return -1;
  }
  return (listeningPort_ > 0) ? listeningPort_ : port_.value;
}

void EthernetServer::begin() {
  if (!port_.has_value) {
    return;
  }
  begin(port_.value, false);
}

bool EthernetServer::beginWithReuse() {
  if (!port_.has_value) {
    return false;
  }
  return begin(port_.value, true);
}

bool EthernetServer::begin(const uint16_t port) {
  return begin(port, false);
}

bool EthernetServer::beginWithReuse(const uint16_t port) {
  return begin(port, true);
}

bool EthernetServer::begin(const uint16_t port, const bool reuse) {
  // Only call end() if parameters have changed
  if (listeningPort_ > 0) {
    // If the request port is zero then choose another port
    if ((port != 0) && (port_ == port) && (reuse_ == reuse)) {
      return true;
    }
    end();  // TODO: Should we call end() only if the new begin is successful?
  }

  // Only change the port if listening was successful
  const int32_t p = internal::ConnectionManager::instance().listen(port, reuse);
  if (p > 0) {
    listeningPort_ = p;
    port_ = {true, (port == 0) ? uint16_t{0} : static_cast<uint16_t>(p)};
    reuse_ = reuse;
    return true;
  }
  return false;
}

void EthernetServer::end() {
  if (listeningPort_ > 0) {
    internal::ConnectionManager::instance().stopListening(listeningPort_);
    listeningPort_ = 0;
  }
  port_ = {false, 0};
}

EthernetClient EthernetServer::accept() const {
  if (listeningPort_ > 0) {
    const auto conn =
        internal::ConnectionManager::instance().findUnacknowledged(
            listeningPort_);
    Ethernet.loop();
    if (conn != nullptr) {
      conn->accepted = true;
      return EthernetClient{conn};
    }
  }
  return EthernetClient{};
}

EthernetClient EthernetServer::available() const {
  if (listeningPort_ > 0) {
    const auto conn =
        internal::ConnectionManager::instance().findAvailable(listeningPort_);
    Ethernet.loop();
    if (conn != nullptr) {
      return EthernetClient{conn};
    }
  }
  return EthernetClient{};
}

EthernetServer::operator bool() const {
  return listeningPort_ > 0;
}

size_t EthernetServer::write(const uint8_t b) {
  if (listeningPort_ == 0) {
    return 1;
  }
  return internal::ConnectionManager::instance().write(listeningPort_, b);
}

size_t EthernetServer::write(const uint8_t* const buffer, const size_t size) {
  if (listeningPort_ == 0) {
    return size;
  }
  return internal::ConnectionManager::instance().write(listeningPort_,
                                                       buffer, size);
}

int EthernetServer::availableForWrite() {
  if (listeningPort_ == 0) {
    return 0;
  }
  return internal::ConnectionManager::instance().availableForWrite(
      listeningPort_);
}

void EthernetServer::flush() {
  if (listeningPort_ == 0) {
    return;
  }
  internal::ConnectionManager::instance().flush(listeningPort_);
}

}  // namespace network
}  // namespace qindesign

#endif  // LWIP_TCP
