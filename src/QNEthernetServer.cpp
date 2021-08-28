// QNEthernetServer.cpp contains the EthernetServer implementation.
// This file is part of the QNEthernet library.
// (c) 2021 Shawn Silverman

#include "QNEthernetServer.h"

// C++ includes
#include <algorithm>
#include <atomic>
#include <utility>

namespace qindesign {
namespace network {

void EthernetServer::errFunc(void *arg, err_t err) {
  if (arg == nullptr) {
    return;
  }

  EthernetServer *server = reinterpret_cast<EthernetServer *>(arg);

  // TODO: Tell server what the error was

  if (err != ERR_OK) {
    // TODO: Lock if not single-threaded
    if (err != ERR_CLSD && err != ERR_ABRT) {
      std::atomic_signal_fence(std::memory_order_acquire);
      if (tcp_close(server->pcb_) != ERR_OK) {
        tcp_abort(server->pcb_);
      }
    }
    server->pcb_ = nullptr;
    std::atomic_signal_fence(std::memory_order_release);
  }
}

err_t EthernetServer::acceptFunc(void *arg, struct tcp_pcb *newpcb, err_t err) {
  if (err != ERR_OK || newpcb == nullptr || arg == nullptr) {
    return ERR_VAL;
  }

  EthernetServer *server = reinterpret_cast<EthernetServer *>(arg);

  // TODO: Tell server what the error was

  // TODO: Lock if not single-threaded

  // See: https://quuxplusone.github.io/blog/2021/03/03/push-back-emplace-back/

  server->clients_.push_back(new ConnectionHolder());
  std::atomic_signal_fence(std::memory_order_acq_rel);
  auto &holder = server->clients_.back();
  holder->connected = true;
  holder->state = new ConnectionState(newpcb);
  holder->state->connect(holder,
                         &EthernetClient::recvFunc,
                         &EthernetClient::errFunc);
  const ConnectionHolder *pHolder = holder;
  holder->state->removeFunc = [server, pHolder](ConnectionState *state) {
    // TODO: Lock if not single-threaded
    std::atomic_signal_fence(std::memory_order_acquire);
    auto it = std::find_if(server->clients_.cbegin(), server->clients_.cend(),
                           [pHolder](const auto &holder) {
                             return holder == pHolder;
                           });
    if (it != server->clients_.cend()) {
      server->clients_.erase(it);
      std::atomic_signal_fence(std::memory_order_release);
    }
  };
  std::atomic_signal_fence(std::memory_order_release);

  // Add a receive listener so we can see who has data available
  // tcp_arg(newpcb, server);
  // tcp_recv(newpcb, &recvFunc);

  return ERR_OK;
}

EthernetServer::EthernetServer(uint16_t port)
    : port_(port) {}

EthernetServer::~EthernetServer() {
  if (tcp_close(pcb_) != ERR_OK) {
    tcp_abort(pcb_);
  }
  pcb_ = nullptr;
}

void EthernetServer::begin() {
  if (pcb_ == nullptr) {
    pcb_ = tcp_new();
    if (pcb_ == nullptr) {
      return;
    }
    tcp_arg(pcb_, this);
    tcp_err(pcb_, &errFunc);
  }

  // Try to bind
  if (tcp_bind(pcb_, IP_ADDR_ANY, port_) != ERR_OK) {
    tcp_abort(pcb_);
    pcb_ = nullptr;
    return;
  }

  // Try to listen
  tcp_pcb *pcb = tcp_listen(pcb_);
  if (pcb == nullptr) {
    tcp_abort(pcb_);
    pcb_ = nullptr;
    return;
  }
  pcb_ = pcb;
  tcp_accept(pcb_, &acceptFunc);
}

EthernetClient EthernetServer::accept() {
  // TODO: Lock if not single-threaded
  std::atomic_signal_fence(std::memory_order_acquire);
  while (!clients_.empty()) {
    for (auto it = clients_.begin(); it != clients_.end(); ++it) {
      auto *holder = *it;
      clients_.erase(it);  // This invalidates the iterator

      ConnectionState *state = holder->state;
      if (state == nullptr) {
        // Restart the loop because the iterator is invalid
        break;
      }
      state->removeFunc = nullptr;  // The holder is already removed
      std::atomic_signal_fence(std::memory_order_release);
      return EthernetClient{state};
    }
  }
  std::atomic_signal_fence(std::memory_order_release);
  return EthernetClient{};
}

EthernetClient EthernetServer::available() {
  // TODO: Lock if not single-threaded
  std::atomic_signal_fence(std::memory_order_acquire);
  for (auto &holder : clients_) {
    const ConnectionState *state = holder->state;
    if (state == nullptr ||
        state->inBufPos < 0 ||
        state->inBuf.size() <= static_cast<size_t>(state->inBufPos)) {
      continue;
    }
    return EthernetClient{holder};
  }
  return EthernetClient{};
}

EthernetServer::operator bool() {
  std::atomic_signal_fence(std::memory_order_acquire);
  return (pcb_ != nullptr);
}

size_t EthernetServer::write(uint8_t b) {
  // TODO: Lock if not single-threaded
  std::atomic_signal_fence(std::memory_order_acquire);
  for (const auto &holder : clients_) {
    const ConnectionState *state = holder->state;
    if (state == nullptr) {
      continue;
    }
    if (tcp_sndbuf(state->pcb) < 1) {
      tcp_output(state->pcb);
    }
    if (tcp_sndbuf(state->pcb) >= 1) {
      tcp_write(state->pcb, &b, 1, TCP_WRITE_FLAG_COPY);
    }
  }
  return 1;
}

size_t EthernetServer::write(const uint8_t *buffer, size_t size) {
  if (size > UINT16_MAX) {
    size = UINT16_MAX;
  }
  uint16_t size16 = size;
  // TODO: Lock if not single-threaded
  std::atomic_signal_fence(std::memory_order_acquire);
  for (const auto &holder : clients_) {
    const ConnectionState *state = holder->state;
    if (state == nullptr) {
      continue;
    }
    if (tcp_sndbuf(state->pcb) < size) {
      tcp_output(state->pcb);
    }
    uint16_t len = std::min(size16, tcp_sndbuf(state->pcb));
    if (len > 0) {
      tcp_write(state->pcb, buffer, len, TCP_WRITE_FLAG_COPY);
    }
  }
  return size;
}

int EthernetServer::availableForWrite() {
  return 0;
}

void EthernetServer::flush() {
  // TODO: Lock if not single-threaded
  for (const auto &holder : clients_) {
    const ConnectionState *state = holder->state;
    if (state == nullptr) {
      continue;
    }
    tcp_output(state->pcb);
  }
}

}  // namespace network
}  // namespace qindesign
