// EthernetUDP implementation.
// (c) 2021 Shawn Silverman

#include "EthernetUDP.h"

// C++ includes
#include <algorithm>
#include <atomic>

#include "Ethernet.h"

extern const int kMTU;

namespace qindesign {
namespace network {

// Maximum UDP packet size.
// Subtract UDP header size and minimum IPv4 header size.
const int kMaxUDPSize = kMTU - 8 - 20;

static std::atomic_flag lock_ = ATOMIC_FLAG_INIT;

void EthernetUDP::recvFunc(void *arg, struct udp_pcb *pcb, struct pbuf *p,
                           const ip_addr_t *addr, u16_t port) {
  EthernetUDP *udp = reinterpret_cast<EthernetUDP *>(arg);

  if (p == nullptr) {
    udp->stop();
    return;
  }

  struct pbuf *pHead = p;

  // TODO: Lock if not single-threaded
  udp->inPacket_.clear();
  udp->inPacket_.reserve(p->tot_len);
  // TODO: Limit vector size
  while (p != nullptr) {
    unsigned char *data = reinterpret_cast<unsigned char *>(p->payload);
    udp->inPacket_.insert(udp->inPacket_.end(), &data[0], &data[p->len]);
    p = p->next;
  }
  udp->inAddr_ = addr->addr;
  udp->inPort_ = port;
  std::atomic_signal_fence(std::memory_order_release);

  pbuf_free(pHead);
}

EthernetUDP::EthernetUDP()
    : pcb_(nullptr),
      inPacket_{},
      packet_{},
      packetPos_(-1),
      inAddr_{INADDR_NONE},
      inPort_(0),
      hasOutPacket_(false),
      outIpaddr_{0},
      outPort_(0),
      outPacket_{} {}

EthernetUDP::~EthernetUDP() {
  stop();
}

uint8_t EthernetUDP::begin(uint16_t localPort) {
  if (pcb_ == nullptr) {
    pcb_ = udp_new();
  }
  if (pcb_ == nullptr) {
    return false;
  }
  if (udp_bind(pcb_, IP_ANY_TYPE, localPort) != ERR_OK) {
    return false;
  }

  if (inPacket_.capacity() < kMaxUDPSize) {
    inPacket_.reserve(kMaxUDPSize);
  }
  if (packet_.capacity() < kMaxUDPSize) {
    packet_.reserve(kMaxUDPSize);
  }

  udp_recv(pcb_, &recvFunc, this);

  return true;
}

uint8_t EthernetUDP::beginMulticast(IPAddress ip, uint16_t localPort) {
  if (pcb_ == nullptr) {
    pcb_ = udp_new();
  }
  if (pcb_ == nullptr) {
    return false;
  }

  // Check for a multicast address
  if ((static_cast<uint32_t>(ip) & 0xF0000000) != 0xE0000000) {
    return false;
  }

  const ip_addr_t ipaddr = IPADDR4_INIT(static_cast<uint32_t>(ip));
  if (udp_bind(pcb_, &ipaddr, localPort) != ERR_OK) {
    return false;
  }

  if (inPacket_.capacity() < kMaxUDPSize) {
    inPacket_.reserve(kMaxUDPSize);
  }
  if (packet_.capacity() < kMaxUDPSize) {
    packet_.reserve(kMaxUDPSize);
  }

  udp_recv(pcb_, recvFunc, this);

  return true;
}

void EthernetUDP::stop() {
  if (pcb_ == nullptr) {
    return;
  }
  udp_remove(pcb_);
  pcb_ = nullptr;
}

// --------------------------------------------------------------------------
//  Reception
// --------------------------------------------------------------------------

int EthernetUDP::parsePacket() {
  if (pcb_ == nullptr) {
    return 0;
  }

  // TODO: Lock if not single-threaded
  std::atomic_signal_fence(std::memory_order_acquire);
  packet_ = inPacket_;
  inPacket_.clear();

  if (packet_.size() > 0) {
    packetPos_ = 0;
    return packet_.size();
  } else {
    packetPos_ = -1;
    return 0;
  }
}

inline bool EthernetUDP::isAvailable() {
  return (0 <= packetPos_ && static_cast<size_t>(packetPos_) < packet_.size());
}

int EthernetUDP::available() {
  if (!isAvailable()) {
    return 0;
  }
  return packet_.size() - packetPos_;
}

int EthernetUDP::read() {
  if (!isAvailable()) {
    return -1;
  }
  return packet_[packetPos_++];
}

int EthernetUDP::read(unsigned char *buffer, size_t len) {
  return read(reinterpret_cast<char *>(buffer), len);
}

int EthernetUDP::read(char *buffer, size_t len) {
  if (len == 0 || !isAvailable()) {
    return 0;
  }
  len = std::min(len, packet_.size() - packetPos_);
  std::copy_n(&packet_.data()[packetPos_], len, buffer);
  packetPos_ += len;
  return len;
}

int EthernetUDP::peek() {
  if (!isAvailable()) {
    return -1;
  }
  return packet_[packetPos_];
}

void EthernetUDP::flush() {
  packetPos_ = -1;
}

IPAddress EthernetUDP::remoteIP() {
  // TODO: Lock if not single-threaded
  std::atomic_signal_fence(std::memory_order_acquire);
  return inAddr_;
}

uint16_t EthernetUDP::remotePort() {
  // TODO: Lock if not single-threaded
  std::atomic_signal_fence(std::memory_order_acquire);
  return inPort_;
}

// --------------------------------------------------------------------------
//  Transmission
// --------------------------------------------------------------------------

int EthernetUDP::beginPacket(IPAddress ip, uint16_t port) {
  if (pcb_ == nullptr) {
    pcb_ = udp_new();
  }
  if (pcb_ == nullptr) {
    return false;
  }
  if (outPacket_.capacity() < kMaxUDPSize) {
    outPacket_.reserve(kMaxUDPSize);
  }

  outIpaddr_ = IPADDR4_INIT(static_cast<uint32_t>(ip));
  hasOutPacket_ = true;
  outPacket_.clear();
  return true;
}

int EthernetUDP::beginPacket(const char *host, uint16_t port) {
  if (pcb_ == nullptr) {
    pcb_ = udp_new();
  }
  if (pcb_ == nullptr) {
    return false;
  }
  if (outPacket_.capacity() < kMaxUDPSize) {
    outPacket_.reserve(kMaxUDPSize);
  }

  if (!ipaddr_aton(host, &outIpaddr_)) {
    return false;
  }
  hasOutPacket_ = true;
  outPacket_.clear();
  return true;
}

int EthernetUDP::endPacket() {
  if (!hasOutPacket_) {
    return false;
  }
  hasOutPacket_ = false;

  // Note: Use PBUF_RAM for TX
  struct pbuf *p = pbuf_alloc(PBUF_TRANSPORT, outPacket_.size(), PBUF_RAM);
  if (p == nullptr) {
    return false;
  }
  pbuf_take(p, outPacket_.data(), outPacket_.size());
  outPacket_.clear();
  bool retval = (udp_sendto(pcb_, nullptr, &outIpaddr_, outPort_) == ERR_OK);
  pbuf_free(p);
  return retval;
}

size_t EthernetUDP::write(uint8_t b) {
  if (!hasOutPacket_) {
    return 0;
  }
  // TODO: Limit vector size
  outPacket_.push_back(b);
  return 1;
}

size_t EthernetUDP::write(const uint8_t *buffer, size_t size) {
  if (!hasOutPacket_ || size == 0) {
    return 0;
  }
  if (size > UINT16_MAX) {
    size = UINT16_MAX;
  }
  // TODO: Limit vector size
  outPacket_.insert(outPacket_.end(), &buffer[0], &buffer[size]);
  return size;
}

}  // namespace network
}  // namespace qindesign
