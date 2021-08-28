// EthernetClient implementation.
// (c) 2021 Shawn Silverman

#include "EthernetClient.h"

// C++ includes
#include <algorithm>
#include <atomic>

#include <elapsedMillis.h>
#include <lwip/dns.h>
#include <lwip/netif.h>
#include <lwip/tcp.h>
#include <lwipopts.h>

namespace qindesign {
namespace network {

static std::atomic_flag bufLock_ = ATOMIC_FLAG_INIT;
static std::atomic_flag connectionLock_ = ATOMIC_FLAG_INIT;

void EthernetClient::dnsFoundFunc(const char *name, const ip_addr_t *ipaddr,
                                  void *callback_arg) {
  if (callback_arg == nullptr || ipaddr == nullptr) {
    return;
  }

  EthernetClient *client = reinterpret_cast<EthernetClient *>(callback_arg);

  // Also check the host name in case there was some previous request pending
  std::atomic_signal_fence(std::memory_order_acquire);
  if (client->lookupHost_ == name) {
    client->lookupIP_ = ipaddr->addr;
    client->lookupFound_ = true;
    std::atomic_signal_fence(std::memory_order_release);
  }
}

err_t EthernetClient::connectedFunc(void *arg, struct tcp_pcb *tpcb, err_t err) {
  if (arg == nullptr || tpcb == nullptr) {
    return ERR_VAL;
  }

  ConnectionHolder *state = reinterpret_cast<ConnectionHolder *>(arg);

  // TODO: Tell client what the error was

  // TODO: Lock if not single-threaded
  std::atomic_signal_fence(std::memory_order_acquire);
  if (state->client != nullptr) {
    state->client->connected_ = (err == ERR_OK);
    std::atomic_signal_fence(std::memory_order_release);
  }
  if (err != ERR_OK) {
    if (tcp_close(tpcb) != ERR_OK) {
      tcp_abort(tpcb);
    }
    delete state;
    std::atomic_signal_fence(std::memory_order_release);
  }
  return ERR_OK;
}

void EthernetClient::maybeCopyRemaining() {
  if (isAvailable()) {
    remaining_.clear();
    remaining_.insert(remaining_.end(),
               state_->inBuf.begin() + state_->inBufPos,
               state_->inBuf.end());
    remainingPos_ = 0;
  }
}

void EthernetClient::errFunc(void *arg, err_t err) {
  if (arg == nullptr) {
    return;
  }

  ConnectionHolder *state = reinterpret_cast<ConnectionHolder *>(arg);

  // TODO: Tell client what the error was

  // TODO: Lock if not single-threaded
  std::atomic_signal_fence(std::memory_order_acquire);
  if (state->client != nullptr) {
    state->client->connected_ = (err == ERR_OK);
    std::atomic_signal_fence(std::memory_order_release);
  }
  if (err != ERR_OK) {
    if (tcp_close(state->pcb) != ERR_OK) {
      tcp_abort(state->pcb);
    }

    // Copy any buffered data
    if (state->client != nullptr) {
      state->client->maybeCopyRemaining();
    }

    delete state;
    std::atomic_signal_fence(std::memory_order_release);
  }
}

err_t EthernetClient::recvFunc(void *arg, struct tcp_pcb *tpcb, struct pbuf *p,
                               err_t err) {
  if (arg == nullptr || tpcb == nullptr) {
    return ERR_VAL;
  }

  ConnectionHolder *state = reinterpret_cast<ConnectionHolder *>(arg);

  // Check for any errors, and if so, clean up
  if (p == nullptr || err != ERR_OK) {
    if (p != nullptr) {
      tcp_recved(tpcb, p->tot_len);
      pbuf_free(p);
    }

    // TODO: Tell client what the error was

    // TODO: Lock if not single-threaded

    // Copy any buffered data
    std::atomic_signal_fence(std::memory_order_acquire);
    if (state->client != nullptr) {
      state->client->maybeCopyRemaining();
    }

    delete state;
    std::atomic_signal_fence(std::memory_order_release);
    if (tcp_close(tpcb) != ERR_OK) {
      tcp_abort(tpcb);
      return ERR_ABRT;
    }
    return ERR_OK;
  }

  struct pbuf *pHead = p;

  // TODO: Lock if not single-threaded
  std::atomic_signal_fence(std::memory_order_acquire);
  if (state->client != nullptr) {
    state->client->connected_ = (err == ERR_OK);
  }

  std::vector<unsigned char> &v = state->inBuf;
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

  while (p != nullptr) {
    unsigned char *data = reinterpret_cast<unsigned char *>(p->payload);
    v.insert(v.end(), &data[0], &data[p->len]);
    p = p->next;
  }
  std::atomic_signal_fence(std::memory_order_release);

  tcp_recved(tpcb, pHead->tot_len);
  pbuf_free(pHead);

  return ERR_OK;
}

EthernetClient::EthernetClient() : EthernetClient(nullptr, false) {}

EthernetClient::EthernetClient(ConnectionHolder *state, bool externallyManaged)
    : connected_(false),
      connTimeout_(1000),
      lookupHost_{},
      lookupIP_{INADDR_NONE},
      state_(state),
      stateExternallyManaged_(externallyManaged),
      remainingPos_(0) {
  if (state != nullptr) {
    connected_ = true;

    auto removeFn = state->removeFunc;
    state->removeFunc = [removeFn](ConnectionHolder *state) {
      if (removeFn != nullptr) {
        removeFn(state);
      }

      // Unlink this state
      if (state->client != nullptr) {
        state->client->state_ = nullptr;
      }
    };
  }
}

EthernetClient::~EthernetClient() {
  if (!stateExternallyManaged_) {
    stop();
  }
}

// --------------------------------------------------------------------------
//  Connection
// --------------------------------------------------------------------------

int EthernetClient::connect(IPAddress ip, uint16_t port) {
  // First close any existing connection
  // TODO: Lock if not single-threaded
  connected_ = false;
  remainingPos_ = 0;
  remaining_.clear();
  std::atomic_signal_fence(std::memory_order_release);
  std::atomic_signal_fence(std::memory_order_acquire);
  if (state_ != nullptr) {
    // Close any existing connection
    // Re-connect, even if to the same destination
    if (tcp_close(state_->pcb) != ERR_OK) {
      tcp_abort(state_->pcb);
    }
    // state_->pcb = nullptr;  // Reuse the state
  }

  // Try to allocate
  tcp_pcb *pcb = tcp_new();
  if (pcb == nullptr) {
    // TODO: Delete state_?
    state_ = nullptr;
    std::atomic_signal_fence(std::memory_order_release);
    return false;
  }

  // Try to bind
  if (tcp_bind(pcb, IP_ADDR_ANY, 0) != ERR_OK) {
    tcp_abort(pcb);  // TODO: Should we abort here or do nothing?
    // TODO: Delete state_?
    state_ = nullptr;
    std::atomic_signal_fence(std::memory_order_release);
    return false;
  }

  std::atomic_signal_fence(std::memory_order_acquire);
  if (state_ == nullptr) {
    state_ = new ConnectionHolder();
    state_->removeFunc = [](ConnectionHolder *state) {
      // Unlink this state
      if (state->client != nullptr) {
        state->client->state_ = nullptr;
      }
    };
  }
  state_->init(this, pcb, &recvFunc, &errFunc);

  // Try to connect
  ip_addr_t ipaddr = IPADDR4_INIT(static_cast<uint32_t>(ip));
  if (tcp_connect(pcb, &ipaddr, port, &connectedFunc) != ERR_OK) {
    tcp_abort(pcb);  // TODO: Should we abort here or do nothing?
    // TODO: Delete state_?
    state_ = nullptr;
    std::atomic_signal_fence(std::memory_order_release);
    return false;
  }
  std::atomic_signal_fence(std::memory_order_release);
  return true;
}

int EthernetClient::connect(const char *host, uint16_t port) {
  ip_addr_t addr;
  lookupHost_ = host;
  lookupIP_ = INADDR_NONE;
  lookupFound_ = false;
  std::atomic_signal_fence(std::memory_order_release);
  switch (dns_gethostbyname(host, &addr, &dnsFoundFunc, this)) {
    case ERR_OK:
      return connect(addr.addr, port);
    case ERR_INPROGRESS: {
      elapsedMillis timer;
      do {
        // NOTE: Depends on Ethernet loop being called from yield()
        delay(10);
        std::atomic_signal_fence(std::memory_order_acquire);
      } while (lookupIP_ == INADDR_NONE && timer < connTimeout_);
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
  std::atomic_signal_fence(std::memory_order_acquire);
  if (remaining_.empty() && state_ == nullptr) {
    return false;
  }
  return !remaining_.empty() || connected_;
}

EthernetClient::operator bool() {
  struct netif *netif = netif_default;
  if (netif == nullptr) {
    return false;
  }
  return netif_is_up(netif) &&
         netif_is_link_up(netif) &&  // TODO: Should we also check for link up?
         (netif_ip_addr4(netif)->addr != 0);
}

void EthernetClient::setConnectionTimeout(uint16_t timeout) {
  connTimeout_ = timeout;
}

void EthernetClient::stop() {
  // TODO: Lock if not single-threaded
  std::atomic_signal_fence(std::memory_order_acquire);
  if (state_ == nullptr) {
    // This can happen if this object was moved to another
    // or if the connection was disconnected
    return;
  }
  if (tcp_close(state_->pcb) != ERR_OK) {
    tcp_abort(state_->pcb);
  } else {
    elapsedMillis timer;
    do {
      // NOTE: Depends on Ethernet loop being called from yield()
      delay(10);
    } while (connected_ && timer < connTimeout_);
  }
  // TODO: Delete state_?
  state_ = nullptr;
  std::atomic_signal_fence(std::memory_order_release);
}

// --------------------------------------------------------------------------
//  Transmission
// --------------------------------------------------------------------------

size_t EthernetClient::write(uint8_t b) {
  // TODO: Lock if not single-threaded
  std::atomic_signal_fence(std::memory_order_acquire);
  if (state_ == nullptr) {
    return 0;
  }

  if (tcp_sndbuf(state_->pcb) >= 1) {
    if (tcp_write(state_->pcb, &b, 1, TCP_WRITE_FLAG_COPY) != ERR_OK) {
      return 0;
    }
    return 1;
  }
  return 0;
}

size_t EthernetClient::write(const uint8_t *buf, size_t size) {
  // TODO: Lock if not single-threaded
  std::atomic_signal_fence(std::memory_order_acquire);
  if (state_ == nullptr || size == 0) {
    return 0;
  }

  if (size > UINT16_MAX) {
    size = UINT16_MAX;
  }
  size_t sndBufSize = tcp_sndbuf(state_->pcb);
  size = std::min(size, sndBufSize);
  if (size > 0) {
    if (tcp_write(state_->pcb, buf, size, TCP_WRITE_FLAG_COPY) != ERR_OK) {
      return 0;
    }
    // TODO: When do we call tcp_output?
    return size;
  }
  return 0;
}

int EthernetClient::availableForWrite() {
  // TODO: Lock if not single-threaded
  std::atomic_signal_fence(std::memory_order_acquire);
  if (state_ == nullptr) {
    return 0;
  }
  return tcp_sndbuf(state_->pcb);
}

void EthernetClient::flush() {
  // TODO: Lock if not single-threaded
  std::atomic_signal_fence(std::memory_order_acquire);
  if (state_ == nullptr) {
    return;
  }
  tcp_output(state_->pcb);
}

// --------------------------------------------------------------------------
//  Reception
// --------------------------------------------------------------------------

inline bool EthernetClient::isAvailable() {
  return (state_ != nullptr &&
          0 <= state_->inBufPos &&
          static_cast<size_t>(state_->inBufPos) < state_->inBuf.size());
}

int EthernetClient::available() {
  // TODO: Lock if not single-threaded
  std::atomic_signal_fence(std::memory_order_acquire);
  if (!remaining_.empty()) {
    return remaining_.size() - remainingPos_;
  }
  if (!isAvailable()) {
    return 0;
  }
  return state_->inBuf.size() - state_->inBufPos;
}

int EthernetClient::read() {
  // TODO: Lock if not single-threaded
  std::atomic_signal_fence(std::memory_order_acquire);
  if (!remaining_.empty()) {
    int c = remaining_[remainingPos_++];
    if (static_cast<size_t>(remainingPos_) >= remaining_.size()) {
      remaining_.clear();
    }
    return c;
  }
  if (!isAvailable()) {
    return -1;
  }
  return state_->inBuf[state_->inBufPos++];
}

int EthernetClient::read(uint8_t *buf, size_t size) {
  if (size == 0) {
    return 0;
  }

  // TODO: Lock if not single-threaded
  std::atomic_signal_fence(std::memory_order_acquire);
  if (!remaining_.empty()) {
    size = std::min(size, remaining_.size() - static_cast<size_t>(remainingPos_));
    std::copy_n(remaining_.begin() + remainingPos_, size, buf);
    remainingPos_ += size;
    if (static_cast<size_t>(remainingPos_) >= remaining_.size()) {
      remaining_.clear();
    }
    std::atomic_signal_fence(std::memory_order_release);
    return size;
  }
  if (!isAvailable()) {
    return 0;
  }
  size = std::min(size, state_->inBuf.size() - state_->inBufPos);
  std::copy_n(&state_->inBuf.data()[state_->inBufPos], size, buf);
  state_->inBufPos += size;
  std::atomic_signal_fence(std::memory_order_release);
  return size;
}

int EthernetClient::peek() {
  // TODO: Lock if not single-threaded
  std::atomic_signal_fence(std::memory_order_acquire);
  if (!remaining_.empty()) {
    return remaining_[remainingPos_];
  }
  if (!isAvailable()) {
    return -1;
  }
  return state_->inBuf[state_->inBufPos];
}

}  // namespace network
}  // namespace qindesign
