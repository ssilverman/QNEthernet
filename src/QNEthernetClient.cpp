// SPDX-FileCopyrightText: (c) 2021-2023 Shawn Silverman <shawn@pobox.com>
// SPDX-License-Identifier: MIT

// QNEthernetClient.cpp contains the EthernetClient implementation.
// This file is part of the QNEthernet library.

#include "QNEthernetClient.h"

#if LWIP_TCP

// C++ includes
#include <algorithm>
#include <cstring>

#include <elapsedMillis.h>

#include "QNDNSClient.h"
#include "QNEthernet.h"
#include "internal/ConnectionManager.h"
#include "lwip/altcp.h"
#include "lwip/dns.h"
#include "lwip/netif.h"
#include "util/PrintUtils.h"
#include "util/ip_tools.h"

namespace qindesign {
namespace network {

// connect() return values.
// See: https://www.arduino.cc/reference/en/libraries/ethernet/client.connect/
// Note: The example on that page is not correct. Because non-zero values,
//       including negative values, are converted to a value of 'true' when
//       using them as a bool, it assumes a successful connection even when
//       connect() returns an error.
enum class ConnectReturns {
  SUCCESS = 1,
  TIMED_OUT = -1,
  INVALID_SERVER = -2,
  TRUNCATED = -3,
  INVALID_RESPONSE = -4,
};

#if LWIP_DNS
// DNS lookup timeout.
static constexpr uint32_t kDNSLookupTimeout =
    DNS_MAX_RETRIES * DNS_TMR_INTERVAL;
#endif  // LWIP_DNS

EthernetClient::EthernetClient() : EthernetClient(nullptr) {}

EthernetClient::EthernetClient(std::shared_ptr<internal::ConnectionHolder> conn)
    : connTimeout_(1000),
      pendingConnect_(false),
      conn_(conn) {}

EthernetClient::~EthernetClient() {
  // Questionable not to call close(), but copy semantics demand that we don't
}

// --------------------------------------------------------------------------
//  Connection
// --------------------------------------------------------------------------

int EthernetClient::connect(IPAddress ip, uint16_t port) {
  ip_addr_t ipaddr IPADDR4_INIT(get_uint32(ip));
  return connect(&ipaddr, port, true);
}

int EthernetClient::connect(const char *host, uint16_t port) {
#if LWIP_DNS
  IPAddress ip;
  if (!DNSClient::getHostByName(host, ip, kDNSLookupTimeout)) {
    return static_cast<int>(ConnectReturns::INVALID_SERVER);
  }
  return connect(ip, port);
#else
  return static_cast<int>(ConnectReturns::INVALID_SERVER);
#endif  // LWIP_DNS
}

int EthernetClient::connectNoWait(const IPAddress &ip, uint16_t port) {
  ip_addr_t ipaddr IPADDR4_INIT(get_uint32(ip));
  return connect(&ipaddr, port, false);
}

int EthernetClient::connectNoWait(const char *host, uint16_t port) {
#if LWIP_DNS
  IPAddress ip;
  if (!DNSClient::getHostByName(host, ip, kDNSLookupTimeout)) {
    return static_cast<int>(ConnectReturns::INVALID_SERVER);
  }
  return connectNoWait(ip, port);
#else
  return static_cast<int>(ConnectReturns::INVALID_SERVER);
#endif  // LWIP_DNS
}

int EthernetClient::connect(const ip_addr_t *ipaddr, uint16_t port, bool wait) {
  // First close any existing connection (without waiting)
  close();

  conn_ = internal::ConnectionManager::instance().connect(ipaddr, port);
  if (conn_ == nullptr) {
    return 0;
  }

  pendingConnect_ = !wait;

  // Wait for a connection
  if (wait) {
    elapsedMillis timer;
    // NOTE: conn_ could be set to NULL somewhere during the yield
    while (conn_ != nullptr && !conn_->connected && timer < connTimeout_) {
      // NOTE: Depends on Ethernet loop being called from yield()
      yield();
    }
    if (conn_ == nullptr || !conn_->connected) {
      close();
      return static_cast<int>(ConnectReturns::TIMED_OUT);
    }
  }

  return static_cast<int>(ConnectReturns::SUCCESS);
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

void EthernetClient::setConnectionTimeout(uint16_t timeout) {
  connTimeout_ = timeout;
}

void EthernetClient::setNoDelay(bool flag) {
  if (conn_ == nullptr) {
    return;
  }
  const auto &state = conn_->state;
  if (state == nullptr) {
    return;
  }
  if (flag) {
    altcp_nagle_disable(state->pcb);
  } else {
    altcp_nagle_enable(state->pcb);
  }
}

bool EthernetClient::isNoDelay() {
  if (conn_ == nullptr) {
    return false;
  }
  const auto &state = conn_->state;
  if (state == nullptr) {
    return false;
  }
  return altcp_nagle_disabled(state->pcb);
}

void EthernetClient::stop() {
  close(true);
}

void EthernetClient::close() {
  close(false);
}

void EthernetClient::close(bool wait) {
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
      if (altcp_close(state->pcb) != ERR_OK) {
        altcp_abort(state->pcb);
      } else if (wait) {
        elapsedMillis timer;
        // NOTE: conn_ could be set to NULL somewhere during the yield
        while (conn_ != nullptr && conn_->connected && timer < connTimeout_) {
          // NOTE: Depends on Ethernet loop being called from yield()
          yield();
        }
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
  if (!static_cast<bool>(*this)) {
    return 0;
  }

  const auto &state = conn_->state;
  if (state == nullptr) {
    return 0;
  }

#if LWIP_ALTCP
  return altcp_get_port(state->pcb, 1);
#else
  uint16_t port;
  altcp_get_tcp_addrinfo(state->pcb, 1, nullptr, &port);
  return port;
#endif  // LWIP_ALTCP
}

IPAddress EthernetClient::remoteIP() {
  if (!static_cast<bool>(*this)) {
    return INADDR_NONE;
  }

  const auto &state = conn_->state;
  if (state == nullptr) {
    return INADDR_NONE;
  }

#if LWIP_ALTCP
  return ip_addr_get_ip4_uint32(altcp_get_ip(state->pcb, 0));
#else
  ip_addr_t ip;
  altcp_get_tcp_addrinfo(state->pcb, 0, &ip, nullptr);
  return ip_addr_get_ip4_uint32(&ip);
#endif  // LWIP_ALTCP
}

uint16_t EthernetClient::remotePort() {
  if (!static_cast<bool>(*this)) {
    return 0;
  }

  const auto &state = conn_->state;
  if (state == nullptr) {
    return 0;
  }

#if LWIP_ALTCP
  return altcp_get_port(state->pcb, 0);
#else
  uint16_t port;
  altcp_get_tcp_addrinfo(state->pcb, 0, nullptr, &port);
  return port;
#endif  // LWIP_ALTCP
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

size_t EthernetClient::writeFully(uint8_t b) {
  return writeFully(&b, 1);
}

size_t EthernetClient::writeFully(const char *buf) {
  return writeFully(reinterpret_cast<const uint8_t *>(buf), strlen(buf));
}

size_t EthernetClient::writeFully(const char *buf, size_t size) {
  return writeFully(reinterpret_cast<const uint8_t *>(buf), size);
}

size_t EthernetClient::writeFully(const uint8_t *buf, size_t size) {
  // Don't use connected() as the "connected" check because that will
  // return true if there's data available, and the loop doesn't check
  // for data available. Instead, use operator bool().

  return util::writeFully(*this, buf, size,
                          [&]() { return !static_cast<bool>(*this); });
}

size_t EthernetClient::write(uint8_t b) {
  if (!static_cast<bool>(*this)) {
    return 0;
  }
  const auto &state = conn_->state;
  if (state == nullptr) {
    return 0;
  }

  size_t sndBufSize = altcp_sndbuf(state->pcb);
  if (sndBufSize == 0) {  // Possibly flush if there's no space
    altcp_output(state->pcb);
    Ethernet.loop();  // Loop to allow incoming data
    sndBufSize = altcp_sndbuf(state->pcb);
  }

  size_t written = 0;
  if (sndBufSize >= 1) {
    if (altcp_write(state->pcb, &b, 1, TCP_WRITE_FLAG_COPY) == ERR_OK) {
      written = 1;
    }
  }
  Ethernet.loop();  // Loop to allow incoming TCP data
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
    Ethernet.loop();  // Loop to allow incoming TCP data
    return 0;
  }

  size_t sndBufSize = altcp_sndbuf(state->pcb);
  if (sndBufSize == 0) {  // Possibly flush if there's no space
    altcp_output(state->pcb);
    Ethernet.loop();  // Loop to allow incoming data
    sndBufSize = altcp_sndbuf(state->pcb);
  }
  size = std::min(size, sndBufSize);
  if (size > 0) {
    if (altcp_write(state->pcb, buf, size, TCP_WRITE_FLAG_COPY) != ERR_OK) {
      size = 0;
    }
  }

  Ethernet.loop();  // Loop to allow incoming TCP data
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

int EthernetClient::available() {
  if (conn_ == nullptr) {
    return 0;
  }

  // For non-blocking connect
  if (pendingConnect_) {
    watchPendingConnect();
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
  Ethernet.loop();  // Allow data to come in
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

  // For non-blocking connect
  if (pendingConnect_) {
    watchPendingConnect();
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
  Ethernet.loop();  // Allow data to come in
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

  // For non-blocking connect
  if (pendingConnect_) {
    watchPendingConnect();
    return 0;
  }

  auto &rem = conn_->remaining;
  if (!rem.empty()) {
    if (size == 0) {
      return 0;
    }
    size = std::min(size, rem.size() - conn_->remainingPos);
    if (buf != nullptr) {
      std::copy_n(rem.cbegin() + conn_->remainingPos, size, buf);
    }
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

  Ethernet.loop();  // Allow data to come in
  if (size == 0) {
    return 0;
  }

  // NOTE: loop() requires a re-check of the state
  if (!isAvailable(state)) {
    return 0;
  }
  size = std::min(size, state->buf.size() - state->bufPos);
  if (buf != nullptr) {
    std::copy_n(&state->buf.data()[state->bufPos], size, buf);
  }
  state->bufPos += size;
  return size;
}

int EthernetClient::peek() {
  if (conn_ == nullptr) {
    return -1;
  }

  // For non-blocking connect
  if (pendingConnect_) {
    watchPendingConnect();
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
  Ethernet.loop();  // Allow data to come in
  // NOTE: loop() requires a re-check of the state
  if (!isAvailable(state)) {
    return -1;
  }
  return state->buf[state->bufPos];
}

}  // namespace network
}  // namespace qindesign

#endif  // LWIP_TCP
