// EthernetClient implementation.
// (c) 2021 Shawn Silverman

#include "EthernetClient.h"

// C++ includes
#include <algorithm>
#include <atomic>

#include <lwip/dns.h>
#include <lwipopts.h>
#include "Ethernet.h"
#include "SpinLock.h"

namespace qindesign {
namespace network {

static std::atomic_flag bufLock_ = ATOMIC_FLAG_INIT;
static std::atomic_flag connectionLock_ = ATOMIC_FLAG_INIT;

void EthernetClient::dnsFoundFunc(const char *name, const ip_addr_t *ipaddr,
                                  void *callback_arg) {
  // EthernetClient *client = static_cast<EthernetClient *>(callback_arg);
}

err_t EthernetClient::connectedFunc(void *arg, struct tcp_pcb *tpcb, err_t err) {
  EthernetClient *client = static_cast<EthernetClient *>(arg);

  SpinLock lock{connectionLock_};
  client->connecting_ = false;
  client->connected_ = (err == ERR_OK);
  std::atomic_signal_fence(std::memory_order_release);
  return ERR_OK;
}

void EthernetClient::errFunc(void *arg, err_t err) {
  EthernetClient *client = static_cast<EthernetClient *>(arg);

  SpinLock lock{connectionLock_};
  client->connecting_ = false;
  client->connected_ = (err == ERR_OK);
  std::atomic_signal_fence(std::memory_order_release);
}

err_t EthernetClient::recvFunc(void *arg, struct tcp_pcb *tpcb, struct pbuf *p,
                               err_t err) {
  EthernetClient *client = static_cast<EthernetClient *>(arg);

  if (p == nullptr || err != ERR_OK) {
    if (p != nullptr) {
      tcp_recved(tpcb, p->tot_len);
      pbuf_free(p);
    }

    SpinLock lock{connectionLock_};
    client->connecting_ = false;
    client->connected_ = false;
    std::atomic_signal_fence(std::memory_order_release);
    if (tcp_close(tpcb) != ERR_OK) {
      tcp_abort(tpcb);
      return ERR_ABRT;
    }
    return ERR_OK;
  }

  struct pbuf *pHead = p;

  {
    SpinLock lock{bufLock_};
    std::vector<unsigned char> &v = client->inBuf_;

    std::atomic_signal_fence(std::memory_order_acquire);
    size_t rem = v.capacity() - v.size() + client->inBufPos_;
    if (rem < p->tot_len) {
      tcp_recved(tpcb, rem);
      return ERR_INPROGRESS;  // ERR_MEM?
    }

    // If there isn't enough space at the end, move all the data in the buffer
    // to the top
    if (v.capacity() - v.size() < p->tot_len) {
      size_t n = v.size() - client->inBufPos_;
      if (n > 0) {
        std::copy_n(v.begin() + client->inBufPos_, n, v.begin());
        v.resize(n);
      } else {
        v.clear();
      }
      client->inBufPos_ = 0;
      std::atomic_signal_fence(std::memory_order_release);
    }

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

EthernetClient::EthernetClient()
    : pcb_(nullptr),
      connecting_(false),
      connected_(false),
      inBuf_{},
      inBufPos_(0) {}

EthernetClient::EthernetClient(tcp_pcb *pcb)
    : pcb_(pcb),
      connecting_(false),
      connected_(true),
      inBuf_{},
      inBufPos_(0) {
  inBuf_.reserve(TCP_WND);

  // Set up the connection
  tcp_arg(pcb_, this);
  tcp_err(pcb_, &errFunc);
  tcp_recv(pcb_, &recvFunc);
}

EthernetClient::~EthernetClient() {
  stop();
}

int EthernetClient::connect(IPAddress ip, uint16_t port) {
  if (pcb_ == nullptr) {
    pcb_ = tcp_new();
    if (pcb_ != nullptr) {
      tcp_arg(pcb_, this);
      tcp_err(pcb_, &errFunc);
      tcp_recv(pcb_, &recvFunc);
      inBuf_.reserve(TCP_WND);
    }
  }
  if (pcb_ == nullptr) {
    return false;
  }

  if (tcp_bind(pcb_, IP_ADDR_ANY, 0) != ERR_OK) {
    return false;
  }

  connecting_ = true;
  ip_addr_t ipaddr = IPADDR4_INIT(static_cast<uint32_t>(ip));
  if (tcp_connect(pcb_, &ipaddr, port, &connectedFunc) != ERR_OK) {
    return false;
  }
  return true;
}

int EthernetClient::connect(const char *host, uint16_t port) {
  ip_addr_t addr;
  switch (dns_gethostbyname(host, &addr, &dnsFoundFunc, this)) {
    case ERR_OK:
      return connect(IPAddress{addr.addr}, port);
    case ERR_INPROGRESS:
      // TODO: Implement this
      return false;
    case ERR_ARG:
    default:
      return false;
  }
}

size_t EthernetClient::write(uint8_t b) {
  if (pcb_ == nullptr) {
    return 0;
  }

  if (tcp_sndbuf(pcb_) >= 1) {
    if (tcp_write(pcb_, &b, 1, TCP_WRITE_FLAG_COPY) != ERR_OK) {
      return 0;
    }
    return 1;
  }
  return 0;
}

size_t EthernetClient::write(const uint8_t *buf, size_t size) {
  if (pcb_ == nullptr || size == 0) {
    return 0;
  }

  if (size > UINT16_MAX) {
    size = UINT16_MAX;
  }
  size_t sndBufSize = tcp_sndbuf(pcb_);
  size = std::min(size, sndBufSize);
  if (size > 0) {
    if (tcp_write(pcb_, buf, size, TCP_WRITE_FLAG_COPY) != ERR_OK) {
      return 0;
    }
    // TODO: When do we call tcp_output?
    return size;
  }
  return 0;
}

int EthernetClient::availableForWrite() {
  if (pcb_ == nullptr) {
    return 0;
  }
  return tcp_sndbuf(pcb_);
}

inline bool EthernetClient::isAvailable() {
  return (0 <= inBufPos_ && static_cast<size_t>(inBufPos_) < inBuf_.size());
}

int EthernetClient::available() {
  SpinLock lock{bufLock_};
  std::atomic_signal_fence(std::memory_order_acquire);
  return inBuf_.size() - inBufPos_;
}

int EthernetClient::read() {
  SpinLock lock{bufLock_};
  std::atomic_signal_fence(std::memory_order_acquire);
  if (!isAvailable()) {
    return -1;
  }
  return inBuf_[inBufPos_++];
}

int EthernetClient::read(uint8_t *buf, size_t size) {
  SpinLock lock{bufLock_};
  std::atomic_signal_fence(std::memory_order_acquire);
  if (size == 0 || !isAvailable()) {
    return 0;
  }
  size = std::min(size, inBuf_.size() - inBufPos_);
  std::copy_n(&inBuf_.data()[inBufPos_], size, buf);
  inBufPos_ += size;
  std::atomic_signal_fence(std::memory_order_release);
  return size;
}

int EthernetClient::peek() {
  SpinLock lock{bufLock_};
  std::atomic_signal_fence(std::memory_order_acquire);
  if (!isAvailable()) {
    return -1;
  }
  return inBuf_[inBufPos_];
}

void EthernetClient::flush() {
  if (pcb_ == nullptr) {
    return;
  }
  tcp_output(pcb_);
}

void EthernetClient::stop() {
  if (pcb_ == nullptr) {
    return;
  }
  if (tcp_close(pcb_) != ERR_OK) {
    tcp_abort(pcb_);
  }
}

uint8_t EthernetClient::connected() {
  SpinLock lock{connectionLock_};
  std::atomic_signal_fence(std::memory_order_acquire);
  return connecting_ || connected_;
}

EthernetClient::operator bool() {
  return pcb_ != nullptr;
}

}  // namespace network
}  // namespace qindesign
