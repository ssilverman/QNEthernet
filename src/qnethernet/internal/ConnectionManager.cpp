// SPDX-FileCopyrightText: (c) 2021-2025 Shawn Silverman <shawn@pobox.com>
// SPDX-License-Identifier: AGPL-3.0-or-later

// ConnectionManager.cpp implements the connection manager.
// This file is part of the QNEthernet library.

#include "qnethernet/internal/ConnectionManager.h"

#if LWIP_TCP

// C++ includes
#include <algorithm>
#include <limits>

#include "QNEthernet.h"
#include "lwip/arch.h"
#include "lwip/ip.h"
#if LWIP_ALTCP
#include "lwip/tcp.h"
#endif  // LWIP_ALTCP

#if LWIP_ALTCP
// This is a function that fills in the given 'altcp_allocator_t' with an
// allocator function and an argument. The values are used by 'altcp_new()' to
// create the appropriate socket type. This returns whether the allocator and
// argument were successfully created or assigned.
//
// The arguments indicate what the calling code is trying to do:
// 1. If ipaddr is NULL then the application is trying to listen.
// 2. If ipaddr is not NULL then the application is trying to connect.
//
// If the socket could not be created, then qnethernet_altcp_free_allocator() is
// called with the same allocator.
extern std::function<bool(const ip_addr_t *ipaddr, uint16_t port,
                          altcp_allocator_t &allocator)>
    qnethernet_altcp_get_allocator;

// This function is called if qnethernet_altcp_get_allocator() returns true and
// the socket could not be created. It is called with the same allocator as
// qnethernet_altcp_get_allocator(). This is an opportunity to free the argument
// if it has not already been freed.
extern std::function<void(const altcp_allocator_t &allocator)>
    qnethernet_altcp_free_allocator;
#endif  // LWIP_ALTCP

namespace qindesign {
namespace network {
namespace internal {

ConnectionManager &ConnectionManager::instance() {
  static ConnectionManager instance;
  return instance;
}

// Connection state callback.
err_t ConnectionManager::connectedFunc(void *const arg,
                                       struct altcp_pcb *const tpcb,
                                       const err_t err) {
  if (arg == nullptr || tpcb == nullptr) {
    return ERR_ARG;
  }

  const auto holder = static_cast<ConnectionHolder *>(arg);

  holder->lastError = err;
  holder->connected = (err == ERR_OK);

  if (err != ERR_OK) {
    holder->state = nullptr;

    if (err != ERR_CLSD && err != ERR_ABRT) {
      if (altcp_close(tpcb) != ERR_OK) {
        altcp_abort(tpcb);
        return ERR_ABRT;
      }
    }
  }
  return ERR_OK;
}

// Check if there's data available in the buffer.
static inline bool isAvailable(const std::unique_ptr<ConnectionState> &state) {
  return (/*0 <= state->bufPos &&*/ state->bufPos < state->buf.size());
}

// Copy any remaining data from the state to the "remaining" buffer. This first
// clears the 'remaining' buffer.
//
// This assumes holder->state != NULL.
static void maybeCopyRemaining(ConnectionHolder *const holder) {
  std::vector<uint8_t> &v = holder->remaining;
  const auto &state = holder->state;

  // Reset the 'remaining' buffer
  v.clear();
  holder->remainingPos = 0;

  if (isAvailable(state)) {
    v.insert(v.cend(), state->buf.cbegin() + state->bufPos, state->buf.cend());
  }
}

// Error callback.
void ConnectionManager::errFunc(void *const arg, const err_t err) {
  if (arg == nullptr) {
    return;
  }

  const auto holder = static_cast<ConnectionHolder *>(arg);

  holder->lastError = err;
  holder->connected = (err == ERR_OK);

  if (holder->state != nullptr && err != ERR_OK) {
    // Copy any buffered data
    maybeCopyRemaining(holder);

    holder->state = nullptr;
  }
}

// Data reception callback.
err_t ConnectionManager::recvFunc(void *const arg, struct altcp_pcb *const tpcb,
                                  struct pbuf *const p, const err_t err) {
  if (arg == nullptr || tpcb == nullptr) {
    return ERR_ARG;
  }

  const auto holder = static_cast<ConnectionHolder *>(arg);

  holder->lastError = err;

  struct pbuf *pNext = p;
  const auto &state = holder->state;

  // Check for errors and null packets
  // Null packets mean the connection is closed
  if (p == nullptr || err != ERR_OK) {
    holder->connected = false;

    if (state != nullptr) {
      // Copy any buffered data
      maybeCopyRemaining(holder);

      if (p != nullptr) {
        // Copy pbuf contents
        while (pNext != nullptr) {
          const auto data = static_cast<const uint8_t *>(pNext->payload);
          holder->remaining.insert(holder->remaining.cend(), &data[0],
                                   &data[pNext->len]);
          pNext = pNext->next;
        }
      }
    }

    if (p != nullptr) {
      altcp_recved(tpcb, p->tot_len);
      pbuf_free(p);
    }

    holder->state = nullptr;

    if (err != ERR_CLSD && err != ERR_ABRT) {
      if (altcp_close(tpcb) != ERR_OK) {
        altcp_abort(tpcb);
        return ERR_ABRT;
      }
    }

    return ERR_OK;  // Return from error or closed connection
  }

  // We are connected
  // Copy all the data

  holder->connected = true;

  if (state != nullptr) {
    std::vector<uint8_t> &v = state->buf;

    // Check that we can store all the data
    const size_t rem = v.capacity() - v.size() + state->bufPos;
    if (rem < p->tot_len) {
      altcp_recved(tpcb, rem);
      return ERR_INPROGRESS;  // ERR_MEM? Other?
    }

    // If there isn't enough space at the end, move all the data in the buffer
    // to the top
    if (v.capacity() - v.size() < p->tot_len) {
      const size_t n = v.size() - state->bufPos;
      if (n > 0) {
        std::copy_n(v.cbegin() + state->bufPos, n, v.begin());
        v.resize(n);
      } else {
        v.clear();
      }
      state->bufPos = 0;
    }

    // Copy all the data from the pbuf
    while (pNext != nullptr) {
      const auto data = static_cast<const uint8_t *>(pNext->payload);
      v.insert(v.cend(), &data[0], &data[pNext->len]);
      pNext = pNext->next;
    }
  }

  altcp_recved(tpcb, p->tot_len);
  pbuf_free(p);

  return ERR_OK;
}

// Accepted connection callback.
err_t ConnectionManager::acceptFunc(void *const arg,
                                    struct altcp_pcb *const newpcb,
                                    const err_t err) {
  if (newpcb == nullptr || arg == nullptr) {
    return ERR_ARG;
  }

  const auto m = static_cast<ConnectionManager *>(arg);

  if (err != ERR_OK) {
    if (err != ERR_CLSD && err != ERR_ABRT) {
      if (altcp_close(newpcb) != ERR_OK) {
        altcp_abort(newpcb);
        return ERR_ABRT;
      }
    }
    return ERR_OK;
  }

  // Create and add the connection

  auto holder = std::make_shared<ConnectionHolder>();
  holder->lastError = err;
  holder->connected = true;
  holder->state = std::make_unique<ConnectionState>(newpcb, holder.get());
  holder->accepted = false;
  altcp_err(newpcb, &errFunc);
  altcp_recv(newpcb, &recvFunc);
  m->addConnection(holder);

  return ERR_OK;
}

void ConnectionManager::addConnection(
    const std::shared_ptr<ConnectionHolder> &holder) {
  connections_.push_back(holder);
  holder->state->removeFunc = [this, holder](ConnectionState *state) {
    LWIP_UNUSED_ARG(state);

    // Remove the connection from the list
    const auto it =
        std::find(connections_.cbegin(), connections_.cend(), holder);
    if (it != connections_.cend()) {
      connections_.erase(it);
    }
  };
}

static altcp_pcb *create_altcp_pcb(const ip_addr_t *const ipaddr,
                                   const uint16_t port, const u8_t ip_type) {
#if LWIP_ALTCP
  altcp_pcb *pcb = nullptr;
  altcp_allocator_t allocator{nullptr, nullptr};
  if (qnethernet_altcp_get_allocator(ipaddr, port, allocator)) {
    pcb = altcp_new_ip_type(&allocator, ip_type);
    if (pcb == nullptr) {
      qnethernet_altcp_free_allocator(allocator);
    }
  }
  return pcb;
#else
  LWIP_UNUSED_ARG(ipaddr);
  LWIP_UNUSED_ARG(port);
  return altcp_new_ip_type(nullptr, ip_type);
#endif  // LWIP_ALTCP
}

std::shared_ptr<ConnectionHolder> ConnectionManager::connect(
    const ip_addr_t *const ipaddr, const uint16_t port) {
  if (ipaddr == nullptr) {
    Ethernet.loop();  // Allow the stack to move along
    return nullptr;
  }

  altcp_pcb *const pcb = create_altcp_pcb(ipaddr, port, IP_GET_TYPE(ipaddr));
  if (pcb == nullptr) {
    Ethernet.loop();  // Allow the stack to move along
    return nullptr;
  }

  // Try to bind
  if (altcp_bind(pcb, IP_ANY_TYPE, 0) != ERR_OK) {
    altcp_abort(pcb);
    Ethernet.loop();  // Allow the stack to move along
    return nullptr;
  }

  // Connect listeners
  auto holder = std::make_shared<ConnectionHolder>();
  holder->state = std::make_unique<ConnectionState>(pcb, holder.get());
  holder->accepted = true;
  altcp_err(pcb, &errFunc);
  altcp_recv(pcb, &recvFunc);

  // Try to connect
  if (altcp_connect(pcb, ipaddr, port, &connectedFunc) != ERR_OK) {
    // holder->state will be removed when holder is removed
    altcp_abort(pcb);
    Ethernet.loop();  // Allow the stack to move along
    return nullptr;
  }

  addConnection(holder);
  return holder;
}

int32_t ConnectionManager::listen(const uint16_t port, const bool reuse) {
  altcp_pcb *pcb = create_altcp_pcb(nullptr, port, IPADDR_TYPE_ANY);
  if (pcb == nullptr) {
    Ethernet.loop();  // Allow the stack to move along
    return -1;
  }

  // Try to bind
  if (reuse) {
#if LWIP_ALTCP
    altcp_pcb *innermost = pcb;
    while (innermost->inner_conn != nullptr) {
      innermost = innermost->inner_conn;
    }
    ip_set_option(static_cast<tcp_pcb *>(innermost->state), SOF_REUSEADDR);
#else
    ip_set_option(pcb, SOF_REUSEADDR);
#endif  // LWIP_ALTCP
  }
  if (altcp_bind(pcb, IP_ANY_TYPE, port) != ERR_OK) {
    altcp_abort(pcb);
    Ethernet.loop();  // Allow the stack to move along
    return -1;
  }

  // Try to listen
  altcp_pcb *const pcbNew = altcp_listen(pcb);
  if (pcbNew == nullptr) {
    altcp_abort(pcb);
    Ethernet.loop();  // Allow the stack to move along
    return -1;
  }
  pcb = pcbNew;

  // Finally, accept connections
  listeners_.push_back(pcb);
  altcp_arg(pcb, this);
  altcp_accept(pcb, &acceptFunc);

  uint16_t actualPort = port;
  if (port == 0) {
    altcp_get_tcp_addrinfo(pcb, true, nullptr, &actualPort);
  }
  return actualPort;
}

// Gets the local port from the given tcp_pcb.
static uint16_t getLocalPort(altcp_pcb *pcb) {
#if LWIP_ALTCP
  return altcp_get_port(pcb, 1);
#else
  uint16_t port;
  altcp_get_tcp_addrinfo(pcb, 1, nullptr, &port);
  return port;
#endif  // LWIP_ALTCP
}

bool ConnectionManager::isListening(const uint16_t port) const {
  const auto it = std::find_if(
      listeners_.cbegin(), listeners_.cend(), [port](altcp_pcb *const elem) {
        return (elem != nullptr) && (getLocalPort(elem) == port);
      });
  return (it != listeners_.cend());
}

bool ConnectionManager::stopListening(const uint16_t port) {
  const auto it = std::find_if(
      listeners_.cbegin(), listeners_.cend(), [port](altcp_pcb *const elem) {
        return (elem != nullptr) && (getLocalPort(elem) == port);
      });
  if (it == listeners_.cend()) {
    return false;
  }
  altcp_pcb *const pcb = *it;
  listeners_.erase(it);
  if (altcp_close(pcb) != ERR_OK) {
    altcp_abort(pcb);
  }
  return true;
}

std::shared_ptr<ConnectionHolder> ConnectionManager::findUnacknowledged(
    const uint16_t port) const {
  const auto it = std::find_if(
      connections_.cbegin(), connections_.cend(), [port](const auto &elem) {
        const auto &state = elem->state;
        return (state != nullptr) && !elem->accepted &&
               (getLocalPort(state->pcb) == port);
      });
  if (it != connections_.cend()) {
    return *it;
  }
  return nullptr;
}

// TODO: Should we implement some kind of fairness?
std::shared_ptr<ConnectionHolder> ConnectionManager::findAvailable(
    const uint16_t port) const {
  const auto it = std::find_if(
      connections_.cbegin(), connections_.cend(), [port](const auto &elem) {
        const auto &state = elem->state;
        return (state != nullptr) &&
               (getLocalPort(state->pcb) == port) &&
               isAvailable(state);
      });
  if (it != connections_.cend()) {
    return *it;
  }
  return nullptr;
}

bool ConnectionManager::remove(
    const std::shared_ptr<ConnectionHolder> &holder) {
  const auto it = std::find(connections_.cbegin(), connections_.cend(), holder);
  if (it != connections_.cend()) {
    const auto &state = (*it)->state;
    if (state != nullptr) {
      state->removeFunc = nullptr;
    }
    connections_.erase(it);
    return true;
  }
  return false;
}

size_t ConnectionManager::write(const uint16_t port, const uint8_t b) {
  return write(port, &b, 1);
}

size_t ConnectionManager::write(const uint16_t port,
                                const void *const b, const size_t len) {
  const size_t actualLen = std::min(len, size_t{UINT16_MAX});
  const uint16_t size16 = actualLen;
  iterateConnections([port, b, size16](struct altcp_pcb *pcb) {
    if (getLocalPort(pcb) != port) {
      return;
    }
    if (altcp_sndbuf(pcb) < size16) {
      if (altcp_output(pcb) != ERR_OK) {
        return;
      }
      Ethernet.loop();
    }
    const uint16_t len = std::min(size16, altcp_sndbuf(pcb));
    if (len > 0) {
      altcp_write(pcb, b, len, TCP_WRITE_FLAG_COPY);
    }
  });
  Ethernet.loop();
  return actualLen;
}

void ConnectionManager::flush(const uint16_t port) {
  iterateConnections([port](struct altcp_pcb *pcb) {
    if (getLocalPort(pcb) != port) {
      return;
    }
    altcp_output(pcb);
    Ethernet.loop();
  });
  Ethernet.loop();
}

int ConnectionManager::availableForWrite(const uint16_t port) {
  uint16_t min = std::numeric_limits<uint16_t>::max();
  bool found = false;
  iterateConnections([port, &min, &found](struct altcp_pcb *pcb) {
    if (getLocalPort(pcb) != port) {
      return;
    }
    min = std::min(min, altcp_sndbuf(pcb));
    found = true;
  });
  if (!found) {
    return 0;
  }
  return min;
}

void ConnectionManager::abortAll() {
  iterateConnections([](struct altcp_pcb *pcb) { altcp_abort(pcb); });
}

void ConnectionManager::iterateConnections(
    std::function<void(struct altcp_pcb *pcb)> f) {
  std::for_each(connections_.cbegin(), connections_.cend(),
                [&f](const auto &elem) {
                  const auto &state = elem->state;
                  if (state != nullptr) {
                    f(state->pcb);
                  }
                });
}

void ConnectionManager::iterateListeners(
    std::function<void(struct altcp_pcb *pcb)> f) {
  std::for_each(listeners_.cbegin(), listeners_.cend(), f);
}

}  // namespace internal
}  // namespace network
}  // namespace qindesign

#endif  // LWIP_TCP
