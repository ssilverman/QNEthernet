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
#include <lwip/tcp.h>
#include <lwipopts.h>

namespace qindesign {
namespace network {

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

err_t EthernetClient::connectedFunc(void *arg, struct tcp_pcb *tpcb, err_t err) {
  if (arg == nullptr || tpcb == nullptr) {
    return ERR_VAL;
  }

  ConnectionHolder *holder = reinterpret_cast<ConnectionHolder *>(arg);

  // TODO: Tell client what the error was

  // TODO: Lock if not single-threaded
  holder->connected = (err == ERR_OK);

  if (holder->state != nullptr && err != ERR_OK) {
    if (err != ERR_CLSD && err != ERR_ABRT) {
      if (tcp_close(tpcb) != ERR_OK) {
        tcp_abort(tpcb);
      }
    }
    delete holder->state;
    holder->state = nullptr;
  }
  return ERR_OK;
}

// Check if there's data available in the buffer.
inline bool isAvailable(const ConnectionState *state) {
  return (state != nullptr &&
          0 <= state->inBufPos && state->inBufPos < state->inBuf.size());
}

// Copy any remaining data from the state to the "remaining" buffer.
void maybeCopyRemaining(ConnectionHolder *holder) {
  if (isAvailable(holder->state)) {
    holder->remaining.clear();
    holder->remaining.insert(
        holder->remaining.end(),
        holder->state->inBuf.cbegin() + holder->state->inBufPos,
        holder->state->inBuf.cend());
    holder->remainingPos = 0;
  }
}

void EthernetClient::errFunc(void *arg, err_t err) {
  if (arg == nullptr) {
    return;
  }

  ConnectionHolder *holder = reinterpret_cast<ConnectionHolder *>(arg);

  // TODO: Tell client what the error was

  // TODO: Lock if not single-threaded
  holder->connected = (err == ERR_OK);

  if (holder->state != nullptr && err != ERR_OK) {
    if (err != ERR_CLSD && err != ERR_ABRT) {
      if (tcp_close(holder->state->pcb) != ERR_OK) {
        tcp_abort(holder->state->pcb);
      }
    }

    // Copy any buffered data
    maybeCopyRemaining(holder);

    delete holder->state;
    holder->state = nullptr;
  }
}

err_t EthernetClient::recvFunc(void *arg, struct tcp_pcb *tpcb, struct pbuf *p,
                               err_t err) {
  if (arg == nullptr || tpcb == nullptr) {
    return ERR_VAL;
  }

  ConnectionHolder *holder = reinterpret_cast<ConnectionHolder *>(arg);

  // Check for any errors, and if so, clean up
  if (p == nullptr || err != ERR_OK) {
    holder->connected = false;

    if (p != nullptr) {
      tcp_recved(tpcb, p->tot_len);
      pbuf_free(p);
    }

    // TODO: Tell client what the error was

    // TODO: Lock if not single-threaded

    // Copy any buffered data
    maybeCopyRemaining(holder);

    if (holder->state != nullptr) {
      delete holder->state;
      holder->state = nullptr;

      if (err != ERR_CLSD && err != ERR_ABRT) {
        if (tcp_close(tpcb) != ERR_OK) {
          tcp_abort(tpcb);
          return ERR_ABRT;
        }
      }
    }
    return ERR_OK;
  }

  struct pbuf *pHead = p;

  // TODO: Lock if not single-threaded
  holder->connected = true;

  ConnectionState *state = holder->state;
  if (state != nullptr) {
    auto &v = state->inBuf;

    // Check that we can store all the data
    size_t rem = v.capacity() - v.size() + state->inBufPos;
    if (rem < p->tot_len) {
      tcp_recved(tpcb, rem);
      return ERR_INPROGRESS;  // ERR_MEM?
    }

    // If there isn't enough space at the end, move all the data in the buffer
    // to the top
    if (v.capacity() - v.size() < p->tot_len) {
      size_t n = v.size() - state->inBufPos;
      if (n > 0) {
        std::copy_n(v.begin() + state->inBufPos, n, v.begin());
        v.resize(n);
      } else {
        v.clear();
      }
      state->inBufPos = 0;
    }

    // Copy all the data
    while (p != nullptr) {
      unsigned char *data = reinterpret_cast<unsigned char *>(p->payload);
      v.insert(v.end(), &data[0], &data[p->len]);
      p = p->next;
    }

    tcp_recved(tpcb, pHead->tot_len);
    pbuf_free(pHead);
  }

  return ERR_OK;
}

EthernetClient::EthernetClient() : EthernetClient(&connHolder_) {}

EthernetClient::EthernetClient(ConnectionState *state)
    : EthernetClient(&connHolder_) {
  pHolder_->state = state;
  if (state != nullptr) {
    pHolder_->connected = true;
  }
}

EthernetClient::EthernetClient(ConnectionHolder *holder)
    : connTimeout_(1000),
      lookupHost_{},
      lookupIP_{INADDR_NONE},
      lookupFound_(false),
      pHolder_(holder) {
  if (holder == nullptr) {
    pHolder_ = &connHolder_;
  }
}

EthernetClient::~EthernetClient() {
  if (pHolder_ == &connHolder_) {
    stop();
  }
}

// --------------------------------------------------------------------------
//  Connection
// --------------------------------------------------------------------------

int EthernetClient::connect(IPAddress ip, uint16_t port) {
  // First close any existing connection
  // TODO: Lock if not single-threaded
  if (pHolder_->state != nullptr) {
    // Re-connect, even if to the same destination
    if (tcp_close(pHolder_->state->pcb) != ERR_OK) {
      tcp_abort(pHolder_->state->pcb);
    }
    pHolder_->state = nullptr;
  }

  // Set to internally managed
  pHolder_ = &connHolder_;
  pHolder_->connected = false;
  pHolder_->remainingPos = 0;
  pHolder_->remaining.clear();

  // Try to allocate
  tcp_pcb *pcb = tcp_new();
  if (pcb == nullptr) {
    return false;
  }

  // Try to bind
  if (tcp_bind(pcb, IP_ADDR_ANY, 0) != ERR_OK) {
    tcp_abort(pcb);  // TODO: Should we abort here or do nothing?
    return false;
  }

  pHolder_->state = new ConnectionState(pcb);
  pHolder_->state->connect(pHolder_, &recvFunc, &errFunc);

  // Try to connect
  ip_addr_t ipaddr = IPADDR4_INIT(static_cast<uint32_t>(ip));
  if (tcp_connect(pcb, &ipaddr, port, &connectedFunc) != ERR_OK) {
    tcp_abort(pcb);  // TODO: Should we abort here or do nothing?
    return false;
  }

  // Wait for a connection
  elapsedMillis timer;
  do {
    // NOTE: Depends on Ethernet loop being called from yield()
    delay(10);
  } while (!pHolder_->connected && timer < connTimeout_);
  if (!pHolder_->connected) {
    if (tcp_close(pcb) != ERR_OK) {
      tcp_abort(pcb);
    }
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
      do {
        // NOTE: Depends on Ethernet loop being called from yield()
        delay(10);
      } while (lookupIP_ == INADDR_NONE && timer < 2000);
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
  // TODO: Lock if not single-threaded
  return !pHolder_->remaining.empty() || pHolder_->connected;
}

EthernetClient::operator bool() {
  // TODO: Lock if not single-threaded
  return pHolder_->connected;
}

void EthernetClient::setConnectionTimeout(uint16_t timeout) {
  connTimeout_ = timeout;
}

void EthernetClient::stop() {
  // TODO: Lock if not single-threaded
  const ConnectionState *state = pHolder_->state;
  if (state == nullptr) {
    // This can happen if this object was moved to another
    // or if the connection was disconnected
    return;
  }

  // First try to flush any data
  // TODO: Should we wait a little bit?
  tcp_output(state->pcb);

  if (tcp_close(state->pcb) != ERR_OK) {
    tcp_abort(state->pcb);
  } else {
    elapsedMillis timer;
    do {
      // NOTE: Depends on Ethernet loop being called from yield()
      delay(10);
    } while (pHolder_->connected && timer < connTimeout_);
  }
}

uint16_t EthernetClient::localPort() const {
  // TODO: Lock if not single-threaded
  const ConnectionState *state = pHolder_->state;
  if (state == nullptr) {
    return 0;
  }
  return state->pcb->local_port;
}

IPAddress EthernetClient::remoteIP() const {
  // TODO: Lock if not single-threaded
  const ConnectionState *state = pHolder_->state;
  if (state == nullptr) {
    return INADDR_NONE;
  }
  return state->pcb->remote_ip.addr;
}

uint16_t EthernetClient::remotePort() const {
  // TODO: Lock if not single-threaded
  const ConnectionState *state = pHolder_->state;
  if (state == nullptr) {
    return 0;
  }
  return state->pcb->remote_port;
}

// --------------------------------------------------------------------------
//  Transmission
// --------------------------------------------------------------------------

size_t EthernetClient::write(uint8_t b) {
  // TODO: Lock if not single-threaded
  const ConnectionState *state = pHolder_->state;
  if (state == nullptr) {
    return 0;
  }

  if (tcp_sndbuf(state->pcb) >= 1) {
    if (tcp_write(state->pcb, &b, 1, TCP_WRITE_FLAG_COPY) != ERR_OK) {
      return 0;
    }

    // Wait for some data to flush
    do {
      delay(10);
    } while (pHolder_->connected && tcp_sndbuf(state->pcb) == 0);
    return 1;
  }
  return 0;
}

size_t EthernetClient::write(const uint8_t *buf, size_t size) {
  if (size == 0) {
    return 0;
  }

  // TODO: Lock if not single-threaded
  const ConnectionState *state = pHolder_->state;
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

    // Wait for some data to flush
    do {
      delay(10);
    } while (pHolder_->connected && tcp_sndbuf(state->pcb) == 0);
    return size;
  }
  return 0;
}

int EthernetClient::availableForWrite() {
  // TODO: Lock if not single-threaded
  const ConnectionState *state = pHolder_->state;
  if (state == nullptr) {
    return 0;
  }
  return tcp_sndbuf(state->pcb);
}

void EthernetClient::flush() {
  // TODO: Lock if not single-threaded
  const ConnectionState *state = pHolder_->state;
  if (state == nullptr) {
    return;
  }
  tcp_output(state->pcb);

  // Wait for some data to flush
  do {
    delay(10);
  } while (pHolder_->connected && tcp_sndbuf(state->pcb) == 0);
}

// --------------------------------------------------------------------------
//  Reception
// --------------------------------------------------------------------------

int EthernetClient::available() {
  // TODO: Lock if not single-threaded
  if (!pHolder_->remaining.empty()) {
    return pHolder_->remaining.size() - pHolder_->remainingPos;
  }

  const ConnectionState *state = pHolder_->state;
  if (state == nullptr) {
    return 0;
  }
  delay(10);  // Allow data to come in
  if (!isAvailable(state)) {
    return 0;
  }
  return state->inBuf.size() - state->inBufPos;
}

int EthernetClient::read() {
  // TODO: Lock if not single-threaded
  if (!pHolder_->remaining.empty()) {
    int c = pHolder_->remaining[pHolder_->remainingPos++];
    if (pHolder_->remainingPos >= pHolder_->remaining.size()) {
      pHolder_->remaining.clear();
      pHolder_->remainingPos = 0;
    }
    return c;
  }

  ConnectionState *state = pHolder_->state;
  if (state == nullptr) {
    return 0;
  }
  delay(10);  // Allow data to come in
  if (!isAvailable(state)) {
    return -1;
  }
  return state->inBuf[state->inBufPos++];
}

int EthernetClient::read(uint8_t *buf, size_t size) {
  if (size == 0) {
    return 0;
  }

  // TODO: Lock if not single-threaded
  auto &rem = pHolder_->remaining;
  if (!rem.empty()) {
    size = std::min(size, rem.size() - pHolder_->remainingPos);
    std::copy_n(rem.cbegin() + pHolder_->remainingPos, size, buf);
    pHolder_->remainingPos += size;
    if (pHolder_->remainingPos >= rem.size()) {
      pHolder_->remaining.clear();
      pHolder_->remainingPos = 0;
    }
    return size;
  }

  ConnectionState *state = pHolder_->state;
  if (state == nullptr) {
    return 0;
  }
  delay(10);  // Allow data to come in
  if (!isAvailable(state)) {
    return 0;
  }
  size = std::min(size, state->inBuf.size() - state->inBufPos);
  std::copy_n(&state->inBuf.data()[state->inBufPos], size, buf);
  state->inBufPos += size;
  return size;
}

int EthernetClient::peek() {
  // TODO: Lock if not single-threaded
  if (!pHolder_->remaining.empty()) {
    return pHolder_->remaining[pHolder_->remainingPos];
  }

  const ConnectionState *state = pHolder_->state;
  if (state == nullptr) {
    return 0;
  }
  delay(10);  // Allow data to come in
  if (!isAvailable(state)) {
    return -1;
  }
  return state->inBuf[state->inBufPos];
}

}  // namespace network
}  // namespace qindesign
