// EthernetClient implementation.
// (c) 2021 Shawn Silverman

#include "EthernetClient.h"

// C++ includes
#include <algorithm>
#include <atomic>

#include <elapsedMillis.h>
#include <lwip/dns.h>
#include <lwip/netif.h>
#include <lwipopts.h>

namespace qindesign {
namespace network {

static std::atomic_flag bufLock_ = ATOMIC_FLAG_INIT;
static std::atomic_flag connectionLock_ = ATOMIC_FLAG_INIT;

void EthernetClient::dnsFoundFunc(const char *name, const ip_addr_t *ipaddr,
                                  void *callback_arg) {
  EthernetClient *client = reinterpret_cast<EthernetClient *>(callback_arg);

  // Also check the host name in case there was some previous request pending
  std::atomic_signal_fence(std::memory_order_acquire);
  if ((ipaddr != nullptr) && (client->lookupHost_ == name)) {
    client->lookupIP_ = ipaddr->addr;
    client->lookupFound_ = true;
    std::atomic_signal_fence(std::memory_order_release);
  }
}

err_t EthernetClient::connectedFunc(void *arg, struct tcp_pcb *tpcb, err_t err) {
  ConnectionHolder *state = reinterpret_cast<ConnectionHolder *>(arg);

  // TODO: Tell client what the error was

  // TODO: Lock if not single-threaded
  state->connecting = false;
  state->connected = (err == ERR_OK);
  if (err != ERR_OK) {
    if (tcp_close(tpcb) != ERR_OK) {
      tcp_abort(tpcb);
    }
    state->pcb = nullptr;
  }
  std::atomic_signal_fence(std::memory_order_release);
  return ERR_OK;
}

void EthernetClient::errFunc(void *arg, err_t err) {
  ConnectionHolder *state = reinterpret_cast<ConnectionHolder *>(arg);

  // TODO: Tell client what the error was

  // TODO: Lock if not single-threaded
  state->connecting = false;
  state->connected = (err == ERR_OK);
  std::atomic_signal_fence(std::memory_order_acquire);
  if (tcp_close(state->pcb) != ERR_OK) {
    tcp_abort(state->pcb);
  }
  state->pcb = nullptr;
  std::atomic_signal_fence(std::memory_order_release);
}

err_t EthernetClient::recvFunc(void *arg, struct tcp_pcb *tpcb, struct pbuf *p,
                               err_t err) {
  ConnectionHolder *state = reinterpret_cast<ConnectionHolder *>(arg);

  if (p == nullptr || err != ERR_OK) {
    if (p != nullptr) {
      tcp_recved(tpcb, p->tot_len);
      pbuf_free(p);
    }

    // TODO: Tell client what the error was

    // TODO: Lock if not single-threaded
    state->connecting = false;
    state->connected = false;
    state->pcb = nullptr;
    std::atomic_signal_fence(std::memory_order_release);
    if (tcp_close(tpcb) != ERR_OK) {
      tcp_abort(tpcb);
      return ERR_ABRT;
    }
    return ERR_OK;
  }

  struct pbuf *pHead = p;

  // TODO: Lock if not single-threaded

  std::vector<unsigned char> &v = state->inBuf;

  std::atomic_signal_fence(std::memory_order_acquire);
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
    std::atomic_signal_fence(std::memory_order_release);
  }

  while (p != nullptr) {
    unsigned char *data = reinterpret_cast<unsigned char *>(p->payload);
    v.insert(v.end(), &data[0], &data[p->len]);
    p = p->next;
  }

  tcp_recved(tpcb, pHead->tot_len);
  pbuf_free(pHead);

  return ERR_OK;
}

EthernetClient::EthernetClient()
    : connTimeout_(1000),
      lookupHost_{},
      lookupIP_{INADDR_NONE} {}

EthernetClient::EthernetClient(tcp_pcb *pcb) : EthernetClient() {
  state_.pcb = pcb;
  state_.connected = true;
  state_.inBuf.reserve(TCP_WND);

  // Set up the connection
  tcp_arg(state_.pcb, &state_);
  tcp_err(state_.pcb, &errFunc);
  tcp_recv(state_.pcb, &recvFunc);
}

EthernetClient::~EthernetClient() {
  stop();
}

// --------------------------------------------------------------------------
//  Connection
// --------------------------------------------------------------------------

int EthernetClient::connect(IPAddress ip, uint16_t port) {
  state_.connecting = false;
  state_.connected = false;

  // Close any existing connection
  // Re-connect, even if to the same destination
  if (state_.pcb != nullptr) {
    if (tcp_close(state_.pcb) != ERR_OK) {
      tcp_abort(state_.pcb);
    }
    state_.pcb = nullptr;
  }

  tcp_pcb *pcb = tcp_new();
  if (pcb == nullptr) {
    return false;
  }
  tcp_arg(pcb, &state_);
  tcp_err(pcb, &errFunc);
  tcp_recv(pcb, &recvFunc);
  state_.inBuf.reserve(TCP_WND);
  state_.pcb = pcb;

  // Try to bind
  if (tcp_bind(pcb, IP_ADDR_ANY, 0) != ERR_OK) {
    tcp_abort(pcb);  // TODO: Should we abort here or do nothing?
    state_.pcb = nullptr;
    return false;
  }

  // Try to connect
  state_.connecting = true;
  ip_addr_t ipaddr = IPADDR4_INIT(static_cast<uint32_t>(ip));
  if (tcp_connect(pcb, &ipaddr, port, &connectedFunc) != ERR_OK) {
    tcp_abort(pcb);  // TODO: Should we abort here or do nothing?
    state_.pcb = nullptr;
    state_.connecting = false;
    return false;
  }
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
  return state_.connecting || state_.connected;
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

// --------------------------------------------------------------------------
//  Transmission
// --------------------------------------------------------------------------

size_t EthernetClient::write(uint8_t b) {
  if (state_.pcb == nullptr) {
    return 0;
  }

  if (tcp_sndbuf(state_.pcb) >= 1) {
    if (tcp_write(state_.pcb, &b, 1, TCP_WRITE_FLAG_COPY) != ERR_OK) {
      return 0;
    }
    return 1;
  }
  return 0;
}

size_t EthernetClient::write(const uint8_t *buf, size_t size) {
  if (state_.pcb == nullptr || size == 0) {
    return 0;
  }

  if (size > UINT16_MAX) {
    size = UINT16_MAX;
  }
  size_t sndBufSize = tcp_sndbuf(state_.pcb);
  size = std::min(size, sndBufSize);
  if (size > 0) {
    if (tcp_write(state_.pcb, buf, size, TCP_WRITE_FLAG_COPY) != ERR_OK) {
      return 0;
    }
    // TODO: When do we call tcp_output?
    return size;
  }
  return 0;
}

int EthernetClient::availableForWrite() {
  if (state_.pcb == nullptr) {
    return 0;
  }
  return tcp_sndbuf(state_.pcb);
}

// --------------------------------------------------------------------------
//  Reception
// --------------------------------------------------------------------------

inline bool EthernetClient::isAvailable() {
  return (0 <= state_.inBufPos &&
          static_cast<size_t>(state_.inBufPos) < state_.inBuf.size());
}

int EthernetClient::available() {
  // TODO: Lock if not single-threaded
  std::atomic_signal_fence(std::memory_order_acquire);
  return state_.inBuf.size() - state_.inBufPos;
}

int EthernetClient::read() {
  // TODO: Lock if not single-threaded
  std::atomic_signal_fence(std::memory_order_acquire);
  if (!isAvailable()) {
    return -1;
  }
  return state_.inBuf[state_.inBufPos++];
}

int EthernetClient::read(uint8_t *buf, size_t size) {
  // TODO: Lock if not single-threaded
  std::atomic_signal_fence(std::memory_order_acquire);
  if (size == 0 || !isAvailable()) {
    return 0;
  }
  size = std::min(size, state_.inBuf.size() - state_.inBufPos);
  std::copy_n(&state_.inBuf.data()[state_.inBufPos], size, buf);
  state_.inBufPos += size;
  std::atomic_signal_fence(std::memory_order_release);
  return size;
}

int EthernetClient::peek() {
  // TODO: Lock if not single-threaded
  std::atomic_signal_fence(std::memory_order_acquire);
  if (!isAvailable()) {
    return -1;
  }
  return state_.inBuf[state_.inBufPos];
}

void EthernetClient::flush() {
  // TODO: Lock if not single-threaded
  if (state_.pcb == nullptr) {
    return;
  }
  tcp_output(state_.pcb);
}

void EthernetClient::stop() {
  // TODO: Lock if not single-threaded
  if (state_.pcb == nullptr) {
    return;
  }
  if (tcp_close(state_.pcb) != ERR_OK) {
    tcp_abort(state_.pcb);
  } else {
    elapsedMillis timer;
    do {
      // NOTE: Depends on Ethernet loop being called from yield()
      delay(10);
    } while ((state_.connecting || state_.connected) && timer < connTimeout_);
  }
  state_.pcb = nullptr;
  std::atomic_signal_fence(std::memory_order_release);
}

}  // namespace network
}  // namespace qindesign
