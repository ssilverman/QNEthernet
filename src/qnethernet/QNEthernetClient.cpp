// SPDX-FileCopyrightText: (c) 2021-2025 Shawn Silverman <shawn@pobox.com>
// SPDX-License-Identifier: AGPL-3.0-or-later

// QNEthernetClient.cpp contains the EthernetClient implementation.
// This file is part of the QNEthernet library.

#include "qnethernet/QNEthernetClient.h"

#if LWIP_TCP

// C++ includes
#include <algorithm>
#include <cerrno>
#include <cstring>

#include "QNEthernet.h"
#include "lwip/altcp.h"
#include "lwip/arch.h"
#include "lwip/priv/altcp_priv.h"
#include "lwip/dns.h"
#include "lwip/err.h"
#include "lwip/netif.h"
#include "lwip/sys.h"
#if LWIP_ALTCP
#include "lwip/tcp.h"
#endif  // LWIP_ALTCP
#include "qnethernet/QNDNSClient.h"
#include "qnethernet/internal/ConnectionManager.h"
#include "qnethernet/util/PrintUtils.h"
#include "qnethernet/util/ip_tools.h"
#include "qnethernet_opts.h"

extern "C" void yield();

namespace qindesign {
namespace network {

// Old connect() return values:
//   SUCCESS          =  1
//   TIMED_OUT        = -1
//   INVALID_SERVER   = -2
//   TRUNCATED        = -3
//   INVALID_RESPONSE = -4

EthernetClient::EthernetClient() : EthernetClient(nullptr) {}

EthernetClient::EthernetClient(
    const std::shared_ptr<internal::ConnectionHolder> conn)
    : connTimeout_(1000),
      pendingConnect_(false),
      connTimeoutEnabled_(true),
      conn_(conn) {}

EthernetClient::~EthernetClient() = default;
    // Questionable not to call close(), but copy semantics demand that we don't

// --------------------------------------------------------------------------
//  Connection
// --------------------------------------------------------------------------

int EthernetClient::connect(const IPAddress ip, const uint16_t port) {
#if LWIP_IPV4
  const ip_addr_t ipaddr IPADDR4_INIT(static_cast<uint32_t>(ip));
  return connect(&ipaddr, port, connTimeoutEnabled_);
#else
  LWIP_UNUSED_ARG(ip);
  LWIP_UNUSED_ARG(port);
  return false;
#endif  // LWIP_IPV4
}

int EthernetClient::connect(const char *const host, const uint16_t port) {
#if LWIP_DNS
  IPAddress ip;
  if (!DNSClient::getHostByName(host, ip,
                                QNETHERNET_DEFAULT_DNS_LOOKUP_TIMEOUT)) {
    // INVALID_SERVER (-2)
    return false;
  }
  return connect(ip, port);
#else
  LWIP_UNUSED_ARG(host);
  LWIP_UNUSED_ARG(port);
  return false;
#endif  // LWIP_DNS
}

bool EthernetClient::connectNoWait(const IPAddress &ip, const uint16_t port) {
#if LWIP_IPV4
  const ip_addr_t ipaddr IPADDR4_INIT(static_cast<uint32_t>(ip));
  return connect(&ipaddr, port, false);
#else
  LWIP_UNUSED_ARG(ip);
  LWIP_UNUSED_ARG(port);
  return false;
#endif  // LWIP_IPV4
}

bool EthernetClient::connectNoWait(const char *const host,
                                   const uint16_t port) {
#if LWIP_DNS
  IPAddress ip;
  if (!DNSClient::getHostByName(host, ip,
                                QNETHERNET_DEFAULT_DNS_LOOKUP_TIMEOUT)) {
    // INVALID_SERVER (-2)
    return false;
  }
  return connectNoWait(ip, port);
#else
  LWIP_UNUSED_ARG(host);
  LWIP_UNUSED_ARG(port);
  return false;
#endif  // LWIP_DNS
}

bool EthernetClient::connect(const ip_addr_t *const ipaddr, const uint16_t port,
                             const bool wait) {
  // First close any existing connection (without waiting)
  close(false);

  conn_ = internal::ConnectionManager::instance().connect(ipaddr, port);
  if (conn_ == nullptr) {
    return false;
  }

  pendingConnect_ = !wait;

  // Wait for a connection
  if (wait) {
    const uint32_t t = sys_now();
    // NOTE: conn_ could be set to NULL somewhere during the yield
    while (conn_ != nullptr && !conn_->connected &&
           (sys_now() - t) < connTimeout_) {
      yield();
#if !QNETHERNET_DO_LOOP_IN_YIELD
      Ethernet.loop();
#endif  // !QNETHERNET_DO_LOOP_IN_YIELD
    }
    if (conn_ == nullptr || !conn_->connected) {
      close(false);
      // TIMED_OUT (-1)
      return false;
    }
  }

  // SUCCESS (1)
  return true;
}

bool EthernetClient::connecting() {
  if (conn_ == nullptr || !pendingConnect_) {
    return false;
  }
  return watchPendingConnect() && !conn_->connected;
}

bool EthernetClient::watchPendingConnect() {
  if (conn_->state == nullptr) {
    conn_ = nullptr;
    return false;
  }
  pendingConnect_ = !conn_->connected;
  Ethernet.loop();  // Move the state along
  return true;
}

uint8_t EthernetClient::connected() {
  if (conn_ == nullptr) {
    return false;
  }

  // For non-blocking connect
  if (pendingConnect_) {
    return watchPendingConnect() && conn_->connected;
  }

  if (!conn_->connected && conn_->remaining.empty()) {
    conn_ = nullptr;
    return false;
  }
  Ethernet.loop();  // Allow information to come in
  return true;
}

EthernetClient::operator bool() {
  if (conn_ == nullptr) {
    return false;
  }

  // For non-blocking connect
  if (pendingConnect_) {
    return watchPendingConnect() && conn_->connected;
  }

  if (!conn_->connected) {
    if (conn_->remaining.empty()) {
      conn_ = nullptr;
    }
    return false;
  }
  Ethernet.loop();  // Allow information to come in
  return true;
}

void EthernetClient::stop() {
  close(connTimeoutEnabled_);
}

void EthernetClient::close() {
  close(false);
}

void EthernetClient::close(const bool wait) {
#if LWIP_ALTCP
  LWIP_UNUSED_ARG(wait);
#endif  // LWIP_ALTCP

  if (conn_ == nullptr) {
    return;
  }

  auto &state = conn_->state;
  if (state == nullptr) {
    // This can happen if this object was moved to another
    // or if the connection was disconnected
    conn_ = nullptr;
    return;
  }

  if (pendingConnect_ || conn_->connected) {
    if (!pendingConnect_) {
      // First try to flush any data
      altcp_output(state->pcb);
      Ethernet.loop();  // Maybe some TCP data gets in
      // NOTE: loop() requires a re-check of the state
    } else if (!conn_->connected) {
      if (altcp_close(state->pcb) != ERR_OK) {
        altcp_abort(state->pcb);
      }
      state = nullptr;
    }

    if (state != nullptr) {
      const err_t err = altcp_close(state->pcb);  // In altcp, removes TCP callbacks
      if (err != ERR_OK) {
        altcp_abort(state->pcb);
      }
#if LWIP_ALTCP
      // Altcp sockets don't seem to receive close or error events
      conn_->connected = false;
      state = nullptr;
#else
      if (err == ERR_OK && wait) {
        const uint32_t t = sys_now();
        // TODO: Make this work for altcp, if possible
        // NOTE: conn_ could be set to NULL somewhere during the yield
        while (conn_ != nullptr && conn_->connected &&
               (sys_now() - t) < connTimeout_) {
          yield();
#if !QNETHERNET_DO_LOOP_IN_YIELD
          Ethernet.loop();
#endif  // !QNETHERNET_DO_LOOP_IN_YIELD
        }
      }
#endif  // LWIP_ALTCP
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
  altcp_output(state->pcb);
  Ethernet.loop();  // Maybe some TCP data gets in
  // NOTE: loop() requires a re-check of the state

  if (state != nullptr) {
    altcp_shutdown(state->pcb, 0, 1);
  }
}

void EthernetClient::abort() {
  if (conn_ == nullptr) {
    return;
  }

  const auto &state = conn_->state;
  if (state != nullptr) {
    altcp_abort(state->pcb);
  }
  conn_ = nullptr;
}

uint16_t EthernetClient::localPort() {
  uint16_t port;
  if (!getAddrInfo(true, nullptr, &port)) {
    return 0;
  }
  return port;
}

IPAddress EthernetClient::remoteIP() {
#if LWIP_IPV4
  ip_addr_t ip;
  if (!getAddrInfo(false, &ip, nullptr)) {
    return INADDR_NONE;
  }
  return util::ip_addr_get_ip4_uint32(&ip);
#else
  return INADDR_NONE;
#endif  // LWIP_IPV4
}

uint16_t EthernetClient::remotePort() {
  uint16_t port;
  if (!getAddrInfo(false, nullptr, &port)) {
    return 0;
  }
  return port;
}

IPAddress EthernetClient::localIP() {
#if LWIP_IPV4
  ip_addr_t ip;
  if (!getAddrInfo(true, &ip, nullptr)) {
    return INADDR_NONE;
  }
  return util::ip_addr_get_ip4_uint32(&ip);
#else
  return INADDR_NONE;
#endif  // LWIP_IPV4
}

bool EthernetClient::getAddrInfo(const bool local,
                                 ip_addr_t *const addr, u16_t *const port) {
  if (!static_cast<bool>(*this)) {
    return false;
  }

  const auto &state = conn_->state;
  if (state == nullptr) {
    return false;
  }

  altcp_get_tcp_addrinfo(state->pcb, local, addr, port);
  return true;
}

uintptr_t EthernetClient::connectionId() {
  if (conn_ != nullptr && conn_->connected) {
    const auto &state = conn_->state;
    if (state != nullptr) {
      return reinterpret_cast<uintptr_t>(state->pcb);
    }
  }
  return 0;
}

// --------------------------------------------------------------------------
//  Transmission
// --------------------------------------------------------------------------

size_t EthernetClient::writeFully(const uint8_t b) {
  return writeFully(&b, 1);
}

size_t EthernetClient::writeFully(const char *const buf) {
  return writeFully(buf, std::strlen(buf));
}

size_t EthernetClient::writeFully(const void *const buf, const size_t size) {
  // Don't use connected() as the "connected" check because that will
  // return true if there's data available, and the loop doesn't check
  // for data available. Instead, use operator bool().

  return util::writeFully(*this, buf, size, this);
}

size_t EthernetClient::write(const uint8_t b) {
  return write(&b, 1);
}

size_t EthernetClient::write(const uint8_t *const buf, const size_t size) {
  if (!static_cast<bool>(*this)) {
    return 0;
  }

  const auto &state = conn_->state;
  if (state == nullptr) {
    return 0;
  }

  if (size == 0) {
    Ethernet.loop();  // Loop to allow incoming TCP data
    return 0;
  }

  size_t sndBufSize = altcp_sndbuf(state->pcb);
  if (sndBufSize == 0) {  // Possibly flush if there's no space
    altcp_output(state->pcb);
    Ethernet.loop();  // Loop to allow incoming data
    if (state == nullptr) {  // Re-check the state
      return 0;
    }
    sndBufSize = altcp_sndbuf(state->pcb);
  }

  size_t actualSize = std::min(size, sndBufSize);
  if (actualSize > 0) {
    const err_t err =
        altcp_write(state->pcb, buf, actualSize, TCP_WRITE_FLAG_COPY);
    if (err != ERR_OK) {
      errno = err_to_errno(err);
      actualSize = 0;
    }
#if QNETHERNET_FLUSH_AFTER_WRITE
    altcp_output(state->pcb);
#endif  // QNETHERNET_FLUSH_AFTER_WRITE
  }

  Ethernet.loop();  // Loop to allow incoming TCP data
  return actualSize;
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
  if (altcp_sndbuf(state->pcb) == 0) {
    altcp_output(state->pcb);
  }

  Ethernet.loop();  // Loop to allow incoming TCP data
  // NOTE: loop() requires a re-check of the state
  if (state == nullptr) {
    return 0;
  }
  return altcp_sndbuf(state->pcb);
}

void EthernetClient::flush() {
  if (!static_cast<bool>(*this)) {
    return;
  }
  const auto &state = conn_->state;
  if (state == nullptr) {
    return;
  }

  altcp_output(state->pcb);
  Ethernet.loop();  // Loop to allow incoming TCP data
}

// --------------------------------------------------------------------------
//  Reception
// --------------------------------------------------------------------------

// Check if there's data available in the buffer.
static inline bool isAvailable(
    const std::unique_ptr<internal::ConnectionState> &state) {
  return (state != nullptr) &&  // Necessary because loop() may reset state
         (/*0 <= state->bufPos &&*/ state->bufPos < state->buf.size());
}

#define CHECK_STATE(R)           \
  if (conn_ == nullptr) {        \
    return 0;                    \
  }                              \
  /* For non-blocking connect */ \
  if (pendingConnect_) {         \
    watchPendingConnect();       \
    return 0;                    \
  }

#define GET_STATE_AND_LOOP_OR_CLOSE(R) \
  if (!conn_->connected) {             \
    conn_ = nullptr;                   \
    return (R);                        \
  }                                    \
  const auto &state = conn_->state;    \
  if (state == nullptr) {              \
    return (R);                        \
  }                                    \
  Ethernet.loop();  /* Allow data to come in */

int EthernetClient::available() {
  CHECK_STATE(0)

  if (!conn_->remaining.empty()) {
    return conn_->remaining.size() - conn_->remainingPos;
  }

  GET_STATE_AND_LOOP_OR_CLOSE(0)

  // NOTE: loop() requires a re-check of the state
  if (!isAvailable(state)) {
    return 0;
  }
  return state->buf.size() - state->bufPos;
}

int EthernetClient::read() {
  CHECK_STATE(-1)

  if (!conn_->remaining.empty()) {
    const int c = conn_->remaining[conn_->remainingPos++];
    if (conn_->remainingPos >= conn_->remaining.size()) {
      conn_->remaining.clear();
      conn_->remainingPos = 0;
    }
    return c;
  }

  GET_STATE_AND_LOOP_OR_CLOSE(-1)

  // NOTE: loop() requires a re-check of the state
  if (!isAvailable(state)) {
    return -1;
  }
  return state->buf[state->bufPos++];
}

int EthernetClient::read(uint8_t *const buf, const size_t size) {
  CHECK_STATE(0)

  const auto &rem = conn_->remaining;
  if (!rem.empty()) {
    if (size == 0) {
      return 0;
    }
    const size_t actualSize = std::min(size, rem.size() - conn_->remainingPos);
    if (buf != nullptr) {
      std::copy_n(rem.cbegin() + conn_->remainingPos, actualSize, buf);
    }
    conn_->remainingPos += actualSize;
    if (conn_->remainingPos >= rem.size()) {
      conn_->remaining.clear();
      conn_->remainingPos = 0;
    }
    return static_cast<int>(actualSize);
  }

  GET_STATE_AND_LOOP_OR_CLOSE(0)

  if (size == 0) {
    return 0;
  }

  // NOTE: loop() requires a re-check of the state
  if (!isAvailable(state)) {
    return 0;
  }
  const size_t actualSize = std::min(size, state->buf.size() - state->bufPos);
  if (buf != nullptr) {
    std::copy_n(&state->buf.data()[state->bufPos], actualSize, buf);
  }
  state->bufPos += actualSize;
  return actualSize;
}

int EthernetClient::peek() {
  CHECK_STATE(-1)

  if (!conn_->remaining.empty()) {
    return conn_->remaining[conn_->remainingPos];
  }

  GET_STATE_AND_LOOP_OR_CLOSE(-1)

  // NOTE: loop() requires a re-check of the state
  if (!isAvailable(state)) {
    return -1;
  }
  return state->buf[state->bufPos];
}

#undef CHECK_STATE
#undef GET_STATE_AND_LOOP_OR_CLOSE

// --------------------------------------------------------------------------
//  State and Socket Options
// --------------------------------------------------------------------------

#if !LWIP_ALTCP || defined(LWIP_DEBUG)
// LWIP_DEBUG is required for altcp_dbg_get_tcp_state(), but not for
// tcp_dbg_get_tcp_state(), for some reason
tcp_state EthernetClient::status() const {
  if (conn_ == nullptr) {
    return tcp_state::CLOSED;
  }

  const auto &state = conn_->state;
  if (state == nullptr) {
    return tcp_state::CLOSED;
  }

#if !LWIP_ALTCP
  return tcp_dbg_get_tcp_state(state->pcb);
#else
  return altcp_dbg_get_tcp_state(state->pcb);
#endif  // !LWIP_ALTCP
}
#endif  // !LWIP_ALTCP || defined(LWIP_DEBUG)

#define GET_STATE(R)                \
  if (conn_ == nullptr) {           \
    return (R);                     \
  }                                 \
  const auto &state = conn_->state; \
  if (state == nullptr) {           \
    return (R);                     \
  }

// Gets the innermost PCB from the state. For altcp, the PCB's are nested.
static inline tcp_pcb *innermost(
    const std::unique_ptr<internal::ConnectionState> &state) {
#if LWIP_ALTCP
  altcp_pcb *innermost = state->pcb;
  while (innermost->inner_conn != nullptr) {
    innermost = innermost->inner_conn;
  }
  return static_cast<tcp_pcb *>(innermost->state);
#else
  return state->pcb;
#endif  // LWIP_ALTCP
}

bool EthernetClient::setNoDelay(const bool flag) {
  GET_STATE(false)

  if (flag) {
    altcp_nagle_disable(state->pcb);
  } else {
    altcp_nagle_enable(state->pcb);
  }
  return true;
}

bool EthernetClient::isNoDelay() const {
  GET_STATE(false)

  return altcp_nagle_disabled(state->pcb);
}

bool EthernetClient::setOutgoingDiffServ(const uint8_t ds) {
  GET_STATE(false)
  innermost(state)->tos = ds;
  return true;
}

uint8_t EthernetClient::outgoingDiffServ() const {
  GET_STATE(0)
  return innermost(state)->tos;
}

bool EthernetClient::setOutgoingTTL(const uint8_t ttl) {
  GET_STATE(false)
  innermost(state)->ttl = ttl;
  return true;
}

uint8_t EthernetClient::outgoingTTL() const {
  GET_STATE(0)
  return innermost(state)->ttl;
}

#undef GET_STATE

}  // namespace network
}  // namespace qindesign

#endif  // LWIP_TCP
