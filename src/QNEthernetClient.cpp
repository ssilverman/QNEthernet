// SPDX-FileCopyrightText: (c) 2021 Shawn Silverman <shawn@pobox.com>
// SPDX-License-Identifier: MIT

// QNEthernetClient.cpp contains the EthernetClient implementation.
// This file is part of the QNEthernet library.

#include "QNEthernetClient.h"

// C++ includes
#include <algorithm>
#include <cstring>

#include <elapsedMillis.h>

#include "ConnectionManager.h"
#include "QNDNSClient.h"
#include "QNEthernet.h"
#include "lwip/dns.h"
#include "lwip/netif.h"
#include "lwip/opt.h"
#include "lwip/tcp.h"

namespace qindesign {
namespace network {

// DNS lookup timeout.
static constexpr uint32_t kDNSLookupTimeout =
    DNS_MAX_RETRIES * DNS_TMR_INTERVAL;

EthernetClient::EthernetClient() : EthernetClient(nullptr) {}

EthernetClient::EthernetClient(std::shared_ptr<ConnectionHolder> conn)
    : connTimeout_(1000),
      conn_(conn) {}

EthernetClient::~EthernetClient() {
  // Questionable not to call stop(), but copy semantics demand that we don't
  // stop();
}

// --------------------------------------------------------------------------
//  Connection
// --------------------------------------------------------------------------

int EthernetClient::connect(IPAddress ip, uint16_t port) {
  // First close any existing connection
  stop();

  ip_addr_t ipaddr = IPADDR4_INIT(static_cast<uint32_t>(ip));
  conn_ = ConnectionManager::instance().connect(&ipaddr, port);
  if (conn_ == nullptr) {
    return false;
  }

  // Wait for a connection
  elapsedMillis timer;
  while (!conn_->connected && timer < connTimeout_) {
    // NOTE: Depends on Ethernet loop being called from yield()
    yield();
  }
  if (!conn_->connected) {
    stop();
    return false;
  }

  return true;
}

int EthernetClient::connect(const char *host, uint16_t port) {
  IPAddress ip;
  if (!DNSClient::getHostByName(host, ip, kDNSLookupTimeout)) {
    return false;
  }
  return connect(ip, port);
}

uint8_t EthernetClient::connected() {
  if (conn_ == nullptr) {
    return false;
  }
  if (!conn_->connected && conn_->remaining.empty()) {
    conn_ = nullptr;
    return false;
  }
  EthernetClass::loop();  // Allow information to come in
  return true;
}

EthernetClient::operator bool() {
  if (conn_ == nullptr) {
    return false;
  }
  if (!conn_->connected) {
    if (conn_->remaining.empty()) {
      conn_ = nullptr;
    }
    return false;
  }
  EthernetClass::loop();  // Allow information to come in
  return true;
}

void EthernetClient::setConnectionTimeout(uint16_t timeout) {
  connTimeout_ = timeout;
}

void EthernetClient::stop() {
  if (conn_ == nullptr) {
    return;
  }

  const auto &state = conn_->state;
  if (state == nullptr) {
    // This can happen if this object was moved to another
    // or if the connection was disconnected
    conn_ = nullptr;
    return;
  }

  if (conn_->connected) {
    // First try to flush any data
    tcp_output(state->pcb);
    EthernetClass::loop();  // Maybe some TCP data gets in
    // NOTE: loop() requires a re-check of the state

    if (state != nullptr) {
      if (tcp_close(state->pcb) != ERR_OK) {
        tcp_abort(state->pcb);
      } else {
        elapsedMillis timer;
        while (conn_->connected && timer < connTimeout_) {
          // NOTE: Depends on Ethernet loop being called from yield()
          yield();
        }
      }
    }
  }

  conn_ = nullptr;
}

void EthernetClient::close() {
  if (conn_ == nullptr) {
    return;
  }

  const auto &state = conn_->state;
  if (state == nullptr) {
    conn_ = nullptr;
    return;
  }

  if (conn_->connected) {
    // First try to flush any data
    tcp_output(state->pcb);
    EthernetClass::loop();  // Maybe some TCP data gets in
    // NOTE: loop() requires a re-check of the state

    if (state != nullptr) {
      if (tcp_close(state->pcb) != ERR_OK) {
        tcp_abort(state->pcb);
      }
    }
  }

  conn_ = nullptr;
}

void EthernetClient::closeOutput() {
  if (!static_cast<bool>(*this)) {
    return;
  }
  const auto &state = conn_->state;
  if (state == nullptr) {
    return;
  }

  // First try to flush any data
  tcp_output(state->pcb);
  EthernetClass::loop();  // Maybe some TCP data gets in
  // NOTE: loop() requires a re-check of the state

  if (state != nullptr) {
    tcp_shutdown(state->pcb, 0, 1);
  }
}

uint16_t EthernetClient::localPort() {
  if (!static_cast<bool>(*this)) {
    return 0;
  }

  const auto &state = conn_->state;
  if (state == nullptr) {
    return 0;
  }
  return state->pcb->local_port;
}

IPAddress EthernetClient::remoteIP() {
  if (!static_cast<bool>(*this)) {
    return INADDR_NONE;
  }

  const auto &state = conn_->state;
  if (state == nullptr) {
    return INADDR_NONE;
  }
  return state->pcb->remote_ip.addr;
}

uint16_t EthernetClient::remotePort() {
  if (!static_cast<bool>(*this)) {
    return 0;
  }

  const auto &state = conn_->state;
  if (state == nullptr) {
    return 0;
  }
  return state->pcb->remote_port;
}

// --------------------------------------------------------------------------
//  Transmission
// --------------------------------------------------------------------------

void EthernetClient::writeFully(uint8_t b) {
  writeFully(&b, 1);
}

void EthernetClient::writeFully(const char *buf) {
  writeFully(reinterpret_cast<const uint8_t *>(buf), strlen(buf));
}

void EthernetClient::writeFully(const char *buf, size_t size) {
  writeFully(reinterpret_cast<const uint8_t *>(buf), size);
}

void EthernetClient::writeFully(const uint8_t *buf, size_t size) {
  // Don't use client.connected() as the "connected" check because
  // that will return true if there's data available, and this loop
  // does not check for data available.
  while (size > 0 && static_cast<bool>(*this)) {
    size_t written = write(buf, size);
    size -= written;
    buf += written;
  }
}

size_t EthernetClient::write(uint8_t b) {
  if (!static_cast<bool>(*this)) {
    return 0;
  }
  const auto &state = conn_->state;
  if (state == nullptr) {
    return 0;
  }

  size_t sndBufSize = tcp_sndbuf(state->pcb);
  if (sndBufSize == 0) {  // Possibly flush if there's no space
    tcp_output(state->pcb);
    sndBufSize = tcp_sndbuf(state->pcb);
  }

  size_t written = 0;
  if (sndBufSize >= 1) {
    if (tcp_write(state->pcb, &b, 1, TCP_WRITE_FLAG_COPY) == ERR_OK) {
      written = 1;
    }
  }
  EthernetClass::loop();  // Loop to allow incoming TCP data
  return written;
}

size_t EthernetClient::write(const uint8_t *buf, size_t size) {
  if (!static_cast<bool>(*this)) {
    return 0;
  }

  const auto &state = conn_->state;
  if (state == nullptr) {
    return 0;
  }

  if (size == 0) {
    EthernetClass::loop();  // Loop to allow incoming TCP data
    return 0;
  }

  size_t sndBufSize = tcp_sndbuf(state->pcb);
  if (sndBufSize == 0) {  // Possibly flush if there's no space
    tcp_output(state->pcb);
    sndBufSize = tcp_sndbuf(state->pcb);
  }
  size = std::min(size, sndBufSize);
  if (size > 0) {
    if (tcp_write(state->pcb, buf, size, TCP_WRITE_FLAG_COPY) != ERR_OK) {
      size = 0;
    }
  }

  EthernetClass::loop();  // Loop to allow incoming TCP data
  return size;
}

int EthernetClient::availableForWrite() {
  if (!static_cast<bool>(*this)) {
    return 0;
  }
  const auto &state = conn_->state;
  if (state == nullptr) {
    return 0;
  }

  // Maybe flush
  if (tcp_sndbuf(state->pcb) == 0) {
    tcp_output(state->pcb);
  }

  EthernetClass::loop();  // Loop to allow incoming TCP data
  return tcp_sndbuf(state->pcb);
}

void EthernetClient::flush() {
  if (!static_cast<bool>(*this)) {
    return;
  }
  const auto &state = conn_->state;
  if (state == nullptr) {
    return;
  }

  tcp_output(state->pcb);
  EthernetClass::loop();  // Loop to allow incoming TCP data
}

// --------------------------------------------------------------------------
//  Reception
// --------------------------------------------------------------------------

// Check if there's data available in the buffer.
static inline bool isAvailable(const std::unique_ptr<ConnectionState> &state) {
  return (state != nullptr) &&  // Necessary because loop() may reset state
         (0 <= state->bufPos && state->bufPos < state->buf.size());
}

int EthernetClient::available() {
  if (conn_ == nullptr) {
    return 0;
  }

  if (!conn_->remaining.empty()) {
    return conn_->remaining.size() - conn_->remainingPos;
  }

  if (!conn_->connected) {
    conn_ = nullptr;
    return 0;
  }

  const auto &state = conn_->state;
  if (state == nullptr) {
    return 0;
  }
  EthernetClass::loop();  // Allow data to come in
  // NOTE: loop() requires a re-check of the state
  if (!isAvailable(state)) {
    return 0;
  }
  size_t retval = state->buf.size() - state->bufPos;
  return retval;
}

int EthernetClient::read() {
  if (conn_ == nullptr) {
    return -1;
  }

  if (!conn_->remaining.empty()) {
    int c = conn_->remaining[conn_->remainingPos++];
    if (conn_->remainingPos >= conn_->remaining.size()) {
      conn_->remaining.clear();
      conn_->remainingPos = 0;
    }
    return c;
  }

  if (!conn_->connected) {
    conn_ = nullptr;
    return -1;
  }

  const auto &state = conn_->state;
  if (state == nullptr) {
    return 0;
  }
  EthernetClass::loop();  // Allow data to come in
  // NOTE: loop() requires a re-check of the state
  if (!isAvailable(state)) {
    return -1;
  }
  return state->buf[state->bufPos++];
}

int EthernetClient::read(uint8_t *buf, size_t size) {
  if (conn_ == nullptr) {
    return 0;
  }

  auto &rem = conn_->remaining;
  if (!rem.empty()) {
    if (size == 0) {
      return 0;
    }
    size = std::min(size, rem.size() - conn_->remainingPos);
    std::copy_n(rem.cbegin() + conn_->remainingPos, size, buf);
    conn_->remainingPos += size;
    if (conn_->remainingPos >= rem.size()) {
      conn_->remaining.clear();
      conn_->remainingPos = 0;
    }
    return size;
  }

  if (!conn_->connected) {
    conn_ = nullptr;
    return 0;
  }

  const auto &state = conn_->state;
  if (state == nullptr) {
    return 0;
  }

  EthernetClass::loop();  // Allow data to come in
  if (size == 0) {
    return 0;
  }

  // NOTE: loop() requires a re-check of the state
  if (!isAvailable(state)) {
    return 0;
  }
  size = std::min(size, state->buf.size() - state->bufPos);
  std::copy_n(&state->buf.data()[state->bufPos], size, buf);
  state->bufPos += size;
  return size;
}

int EthernetClient::peek() {
  if (conn_ == nullptr) {
    return -1;
  }

  if (!conn_->remaining.empty()) {
    return conn_->remaining[conn_->remainingPos];
  }

  if (!conn_->connected) {
    conn_ = nullptr;
    return -1;
  }

  const auto &state = conn_->state;
  if (state == nullptr) {
    return 0;
  }
  EthernetClass::loop();  // Allow data to come in
  // NOTE: loop() requires a re-check of the state
  if (!isAvailable(state)) {
    return -1;
  }
  return state->buf[state->bufPos];
}

}  // namespace network
}  // namespace qindesign
