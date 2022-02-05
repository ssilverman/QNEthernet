// SPDX-FileCopyrightText: (c) 2021-2022 Shawn Silverman <shawn@pobox.com>
// SPDX-License-Identifier: MIT

// ConnectionManager.cpp implements the connection manager.
// This file is part of the QNEthernet library.

#include "ConnectionManager.h"

// C++ includes
#include <algorithm>
#include <limits>

#include <core_pins.h>

#include "QNEthernet.h"
#include "lwip/ip.h"

namespace qindesign {
namespace network {
namespace internal {

ConnectionManager ConnectionManager::manager;

// Connection state callback.
err_t ConnectionManager::connectedFunc(void *arg, struct tcp_pcb *tpcb,
                                       err_t err) {
  if (arg == nullptr || tpcb == nullptr) {
    return ERR_ARG;
  }

  ConnectionHolder *holder = reinterpret_cast<ConnectionHolder *>(arg);

  holder->lastError = err;
  holder->connected = (err == ERR_OK);

  if (err != ERR_OK) {
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

// Check if there's data available in the buffer.
static inline bool isAvailable(const std::unique_ptr<ConnectionState> &state) {
  return (0 <= state->bufPos && state->bufPos < state->buf.size());
}

// Copy any remaining data from the state to the "remaining" buffer. This first
// clears the 'remaining' buffer.
//
// This assumes holder->state != NULL.
void maybeCopyRemaining(ConnectionHolder *holder) {
  auto &v = holder->remaining;
  const auto &state = holder->state;

  // Reset the 'remaining' buffer
  v.clear();
  holder->remainingPos = 0;

  if (isAvailable(state)) {
    v.insert(v.end(), state->buf.cbegin() + state->bufPos, state->buf.cend());
  }
}

// Error callback.
void ConnectionManager::errFunc(void *arg, err_t err) {
  if (arg == nullptr) {
    return;
  }

  ConnectionHolder *holder = reinterpret_cast<ConnectionHolder *>(arg);

  holder->lastError = err;
  holder->connected = (err == ERR_OK);

  if (holder->state != nullptr && err != ERR_OK) {
    // Copy any buffered data
    maybeCopyRemaining(holder);

    tcp_pcb *pcb = holder->state->pcb;
    holder->state = nullptr;

    if (err != ERR_CLSD && err != ERR_ABRT) {
      if (tcp_close(pcb) != ERR_OK) {
        tcp_abort(pcb);
      }
    }
  }
}

// Data reception callback.
err_t ConnectionManager::recvFunc(void *arg, struct tcp_pcb *tpcb,
                                  struct pbuf *p, err_t err) {
  if (arg == nullptr || tpcb == nullptr) {
    return ERR_ARG;
  }

  ConnectionHolder *holder = reinterpret_cast<ConnectionHolder *>(arg);

  holder->lastError = err;

  struct pbuf *pHead = p;
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
        while (p != nullptr) {
          unsigned char *data = reinterpret_cast<unsigned char *>(p->payload);
          holder->remaining.insert(holder->remaining.end(),
                                   &data[0], &data[p->len]);
          p = p->next;
        }
      }
    }

    if (pHead != nullptr) {
      tcp_recved(tpcb, pHead->tot_len);
      pbuf_free(pHead);
    }

    holder->state = nullptr;

    if (err != ERR_CLSD && err != ERR_ABRT) {
      if (tcp_close(tpcb) != ERR_OK) {
        tcp_abort(tpcb);
        return ERR_ABRT;
      }
    }

    return ERR_OK;  // Return from error or closed connection
  }

  // We are connected
  // Copy all the data

  holder->connected = true;

  if (state != nullptr) {
    auto &v = state->buf;

    // Check that we can store all the data
    size_t rem = v.capacity() - v.size() + state->bufPos;
    if (rem < p->tot_len) {
      tcp_recved(tpcb, rem);
      return ERR_INPROGRESS;  // ERR_MEM? Other?
    }

    // If there isn't enough space at the end, move all the data in the buffer
    // to the top
    if (v.capacity() - v.size() < p->tot_len) {
      size_t n = v.size() - state->bufPos;
      if (n > 0) {
        std::copy_n(v.begin() + state->bufPos, n, v.begin());
        v.resize(n);
      } else {
        v.clear();
      }
      state->bufPos = 0;
    }

    // Copy all the data from the pbuf
    while (p != nullptr) {
      unsigned char *data = reinterpret_cast<unsigned char *>(p->payload);
      v.insert(v.end(), &data[0], &data[p->len]);
      p = p->next;
    }
  }

  tcp_recved(tpcb, pHead->tot_len);
  pbuf_free(pHead);

  return ERR_OK;
}

// Accepted connection callback.
err_t ConnectionManager::acceptFunc(void *arg, struct tcp_pcb *newpcb,
                                    err_t err) {
  if (newpcb == nullptr || arg == nullptr) {
    return ERR_ARG;
  }

  ConnectionManager *m = reinterpret_cast<ConnectionManager *>(arg);

  if (err != ERR_OK) {
    if (err != ERR_CLSD && err != ERR_ABRT) {
      if (tcp_close(newpcb) != ERR_OK) {
        tcp_abort(newpcb);
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
  tcp_err(newpcb, &errFunc);
  tcp_recv(newpcb, &recvFunc);
  m->addConnection(holder);

  return ERR_OK;
}

void ConnectionManager::addConnection(
    const std::shared_ptr<ConnectionHolder> &holder) {
  connections_.push_back(holder);
  holder->state->removeFunc = [this, holder](ConnectionState *state) {
    // Remove the connection from the list
    auto it =
        std::find_if(connections_.begin(), connections_.end(),
                     [holder](const auto &elem) { return elem == holder; });
    if (it != connections_.end()) {
      connections_.erase(it);
    }
  };
}

std::shared_ptr<ConnectionHolder> ConnectionManager::connect(
    const ip_addr_t *ipaddr, uint16_t port) {
  // Try to allocate
  tcp_pcb *pcb = tcp_new();
  if (pcb == nullptr) {
    return nullptr;
  }

  // Try to bind
  if (tcp_bind(pcb, IP_ADDR_ANY, 0) != ERR_OK) {
    tcp_abort(pcb);
    return nullptr;
  }

  // Connect listeners
  auto holder = std::make_shared<ConnectionHolder>();
  holder->state = std::make_unique<ConnectionState>(pcb, holder.get());
  tcp_err(pcb, &errFunc);
  tcp_recv(pcb, &recvFunc);

  // Try to connect
  if (tcp_connect(pcb, ipaddr, port, &connectedFunc) != ERR_OK) {
    // holder->state will be removed when holder is removed
    tcp_abort(pcb);
    return nullptr;
  }

  addConnection(holder);
  return holder;
}

bool ConnectionManager::listen(uint16_t port, bool reuse) {
  // Try to allocate
  tcp_pcb *pcb = tcp_new();
  if (pcb == nullptr) {
    return false;
  }

  // Try to bind
  if (reuse) {
    ip_set_option(pcb, SOF_REUSEADDR);
  }
  if (tcp_bind(pcb, IP_ADDR_ANY, port) != ERR_OK) {
    tcp_abort(pcb);
    return false;
  }

  // Try to listen
  tcp_pcb *pcbNew = tcp_listen(pcb);
  if (pcbNew == nullptr) {
    tcp_abort(pcb);
    return false;
  }
  pcb = pcbNew;

  // Finally, accept connections
  listeners_.push_back(pcb);
  tcp_arg(pcb, this);
  tcp_accept(pcb, &acceptFunc);

  return true;
}

bool ConnectionManager::isListening(uint16_t port) const {
  auto it = std::find_if(
      listeners_.begin(), listeners_.end(), [port](const auto &elem) {
        return (elem != nullptr) && (elem->local_port == port);
      });
  return (it != listeners_.end());
}

bool ConnectionManager::stopListening(uint16_t port) {
  auto it = std::find_if(
      listeners_.begin(), listeners_.end(), [port](const auto &elem) {
        return (elem != nullptr) && (elem->local_port == port);
      });
  if (it == listeners_.end()) {
    return false;
  }
  tcp_pcb *pcb = *it;
  listeners_.erase(it);
  return (tcp_close(pcb) == ERR_OK);
}

std::shared_ptr<ConnectionHolder> ConnectionManager::findConnected(
    uint16_t port) const {
  auto it = std::find_if(
      connections_.begin(), connections_.end(), [port](const auto &elem) {
        const auto &state = elem->state;
        return (state != nullptr) && (state->pcb->local_port == port);
      });
  if (it != connections_.end()) {
    return *it;
  }
  return nullptr;
}

// TODO: Should we implement some kind of fairness?
std::shared_ptr<ConnectionHolder> ConnectionManager::findAvailable(
    uint16_t port) const {
  auto it = std::find_if(
      connections_.begin(), connections_.end(), [port](const auto &elem) {
        const auto &state = elem->state;
        return (state != nullptr) &&
               (state->pcb->local_port == port) &&
               isAvailable(state);
      });
  if (it != connections_.end()) {
    return *it;
  }
  return nullptr;
}

bool ConnectionManager::remove(
    const std::shared_ptr<ConnectionHolder> &holder) {
  auto it =
      std::find_if(connections_.begin(), connections_.end(),
                   [holder](const auto &elem) { return (elem == holder); });
  if (it != connections_.end()) {
    const auto &state = (*it)->state;
    if (state != nullptr) {
      state->removeFunc = nullptr;
    }
    connections_.erase(it);
    return true;
  }
  return false;
}

size_t ConnectionManager::write(uint16_t port, uint8_t b) {
  std::for_each(connections_.begin(), connections_.end(),
                [port, b](const auto &elem) {
                  const auto &state = elem->state;
                  if (state == nullptr || state->pcb->local_port != port) {
                    return;
                  }
                  if (tcp_sndbuf(state->pcb) < 1) {
                    if (tcp_output(state->pcb) != ERR_OK) {
                      return;
                    }
                    EthernetClass::loop();
                  }
                  if (tcp_sndbuf(state->pcb) >= 1) {
                    tcp_write(state->pcb, &b, 1, TCP_WRITE_FLAG_COPY);
                  }
                });
  EthernetClass::loop();
  return 1;
}

size_t ConnectionManager::write(uint16_t port, const uint8_t *b, size_t len) {
  if (len > UINT16_MAX) {
    len = UINT16_MAX;
  }
  uint16_t size16 = len;
  std::for_each(connections_.begin(), connections_.end(),
                [port, b, size16](const auto &elem) {
                  const auto &state = elem->state;
                  if (state == nullptr || state->pcb->local_port != port) {
                    return;
                  }
                  if (tcp_sndbuf(state->pcb) < size16) {
                    if (tcp_output(state->pcb) != ERR_OK) {
                      return;
                    }
                    EthernetClass::loop();
                  }
                  uint16_t len = std::min(size16, tcp_sndbuf(state->pcb));
                  if (len > 0) {
                    tcp_write(state->pcb, b, len, TCP_WRITE_FLAG_COPY);
                  }
                });
  EthernetClass::loop();
  return len;
}

void ConnectionManager::flush(uint16_t port) {
  std::for_each(connections_.begin(), connections_.end(),
                [port](const auto &elem) {
                  const auto &state = elem->state;
                  if (state == nullptr || state->pcb->local_port != port) {
                    return;
                  }
                  tcp_output(state->pcb);
                });
  EthernetClass::loop();
}

int ConnectionManager::availableForWrite(uint16_t port) {
  uint16_t min = std::numeric_limits<uint16_t>::max();
  bool found = false;
  std::for_each(connections_.begin(), connections_.end(),
                [port, &min, &found](const auto &elem) {
                  const auto &state = elem->state;
                  if (state == nullptr || state->pcb->local_port != port) {
                    return;
                  }
                  min = std::min(min, tcp_sndbuf(state->pcb));
                  found = true;
                });
  if (!found) {
    return 0;
  }
  return min;
}

}  // namespace internal
}  // namespace network
}  // namespace qindesign
