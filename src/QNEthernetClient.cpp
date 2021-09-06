// SPDX-FileCopyrightText: (c) 2021 Shawn Silverman <shawn@pobox.com>
// SPDX-License-Identifier: MIT

// QNEthernetClient.cpp contains the EthernetClient implementation.
// This file is part of the QNEthernet library.

#include "QNEthernetClient.h"

// C++ includes
#include <algorithm>

#include <elapsedMillis.h>
#include <lwip/dns.h>
#include <lwip/netif.h>
#include <lwip/opt.h>
#include <lwip/tcp.h>
#include "ConnectionManager.h"

namespace qindesign {
namespace network {

// DNS lookup timeout.
static constexpr uint32_t kDNSLookupTimeout =
    DNS_MAX_RETRIES * DNS_TMR_INTERVAL;

void EthernetClient::dnsFoundFunc(const char *name, const ip_addr_t *ipaddr,
                                  void *callback_arg) {
  if (callback_arg == nullptr || ipaddr == nullptr) {
    return;
  }

  EthernetClient *client = reinterpret_cast<EthernetClient *>(callback_arg);

  // Also check the host name in case there was some previous request pending
  if (client->lookupHost_ == name) {
    client->lookupIP_ = ipaddr->addr;
    client->lookupFound_ = true;
  }
}

EthernetClient::EthernetClient() : EthernetClient(nullptr) {}

EthernetClient::EthernetClient(std::shared_ptr<ConnectionHolder> conn)
    : connTimeout_(1000),
      lookupHost_{},
      lookupIP_{INADDR_NONE},
      lookupFound_(false),
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
  ip_addr_t addr;
  lookupHost_ = host;
  lookupIP_ = INADDR_NONE;
  lookupFound_ = false;
  switch (dns_gethostbyname(host, &addr, &dnsFoundFunc, this)) {
    case ERR_OK:
      return connect(addr.addr, port);
    case ERR_INPROGRESS: {
      elapsedMillis timer;
      while (lookupIP_ == INADDR_NONE && timer < kDNSLookupTimeout) {
        // NOTE: Depends on Ethernet loop being called from yield()
        yield();
      }
      if (lookupFound_) {
        return connect(lookupIP_, port);
      }
      return false;
    }
    case ERR_ARG:
    default:
      return false;
  }
}

uint8_t EthernetClient::connected() {
  if (conn_ == nullptr) {
    return false;
  }
  if (!conn_->connected && conn_->remaining.empty()) {
    conn_ = nullptr;
    return false;
  }
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
    yield();  // Maybe some data gets out
    // NOTE: yield() requires a re-check of the state

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

uint16_t EthernetClient::localPort() {
  if (!(*this)) {
    return 0;
  }

  const auto &state = conn_->state;
  if (state == nullptr) {
    return 0;
  }
  return state->pcb->local_port;
}

IPAddress EthernetClient::remoteIP() {
  if (!(*this)) {
    return INADDR_NONE;
  }

  const auto &state = conn_->state;
  if (state == nullptr) {
    return INADDR_NONE;
  }
  return state->pcb->remote_ip.addr;
}

uint16_t EthernetClient::remotePort() {
  if (!(*this)) {
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

size_t EthernetClient::write(uint8_t b) {
  if (!(*this)) {
    return 0;
  }
  const auto &state = conn_->state;
  if (state == nullptr) {
    return 0;
  }

  size_t written = 0;

  if (tcp_sndbuf(state->pcb) >= 1) {
    if (tcp_write(state->pcb, &b, 1, TCP_WRITE_FLAG_COPY) != ERR_OK) {
      return 0;
    }
    written = 1;
  }

  // Potentially wait for some data to flush
  if (conn_->connected && tcp_sndbuf(state->pcb) == 0) {
    yield();
  }
  return written;
}

size_t EthernetClient::write(const uint8_t *buf, size_t size) {
  if (!(*this)) {
    return 0;
  }

  const auto &state = conn_->state;
  if (state == nullptr) {
    return 0;
  }

  if (size > UINT16_MAX) {
    size = UINT16_MAX;
  }
  size_t sndBufSize = tcp_sndbuf(state->pcb);
  size = std::min(size, sndBufSize);
  if (size > 0) {
    if (tcp_write(state->pcb, buf, size, TCP_WRITE_FLAG_COPY) != ERR_OK) {
      return 0;
    }
  }

  // Potentially wait for some data to flush
  if (conn_->connected && tcp_sndbuf(state->pcb) == 0) {
    yield();
  }
  return size;
}

int EthernetClient::availableForWrite() {
  if (!(*this)) {
    return 0;
  }
  const auto &state = conn_->state;
  if (state == nullptr) {
    return 0;
  }

  // Potentially wait for some data to flush (known to be connected)
  if (tcp_sndbuf(state->pcb) == 0) {
    yield();
  }
  return tcp_sndbuf(state->pcb);
}

void EthernetClient::flush() {
  if (!(*this)) {
    return;
  }
  const auto &state = conn_->state;
  if (state == nullptr) {
    return;
  }
  tcp_output(state->pcb);

  // Potentially wait for some data to flush
  if (conn_->connected && tcp_sndbuf(state->pcb) == 0) {
    yield();
  }
}

// --------------------------------------------------------------------------
//  Reception
// --------------------------------------------------------------------------

// Check if there's data available in the buffer.
static inline bool isAvailable(const std::unique_ptr<ConnectionState> &state) {
  return (state != nullptr) &&  // Necessary because a yield() may reset state
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
  yield();  // Allow data to come in
  // NOTE: yield() requires a re-check of the state
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
  yield();  // Allow data to come in
  // NOTE: yield() requires a re-check of the state
  if (!isAvailable(state)) {
    return -1;
  }
  return state->buf[state->bufPos++];
}

int EthernetClient::read(uint8_t *buf, size_t size) {
  if (conn_ == nullptr) {
    return 0;
  }

  // TODO: Lock if not single-threaded
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
  if (size == 0) {
    return 0;
  }

  const auto &state = conn_->state;
  if (state == nullptr) {
    return 0;
  }
  yield();  // Allow data to come in
  // NOTE: yield() requires a re-check of the state
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
  yield();  // Allow data to come in
  // NOTE: yield() requires a re-check of the state
  if (!isAvailable(state)) {
    return -1;
  }
  return state->buf[state->bufPos];
}

}  // namespace network
}  // namespace qindesign
