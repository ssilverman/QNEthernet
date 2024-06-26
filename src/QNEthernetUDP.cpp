// SPDX-FileCopyrightText: (c) 2021-2024 Shawn Silverman <shawn@pobox.com>
// SPDX-License-Identifier: AGPL-3.0-or-later

// QNEthernetUDP.cpp contains the EthernetUDP implementation.
// This file is part of the QNEthernet library.

#include "QNEthernetUDP.h"

#if LWIP_UDP

// C++ includes
#include <algorithm>
#include <cerrno>

#include "QNDNSClient.h"
#include "QNEthernet.h"
#include "lwip/arch.h"
#include "lwip/dns.h"
#include "lwip/err.h"
#include "lwip/ip.h"
#include "lwip/sys.h"
#include "qnethernet_opts.h"
#include "util/ip_tools.h"

namespace qindesign {
namespace network {

// Total header size: Minimum IPv4 header size + UDP header size.
static constexpr size_t kHeaderSize = 20 + 8;

// // Maximum UDP payload size without fragmentation.
// static constexpr size_t kMaxPayloadSize = EthernetClass::mtu() - kHeaderSize;

// Maximum possible payload size.
static constexpr size_t kMaxPossiblePayloadSize = UINT16_MAX - kHeaderSize;

void EthernetUDP::recvFunc(void *arg, struct udp_pcb *pcb, struct pbuf *p,
                           const ip_addr_t *addr, u16_t port) {
  if (arg == nullptr || pcb == nullptr) {
    return;
  }

  EthernetUDP *udp = static_cast<EthernetUDP *>(arg);

  if (p == nullptr) {
    udp->stop();
    return;
  }

  uint32_t timestamp = sys_now();

  struct pbuf *pHead = p;

  // Push (replace the head)
  Packet &packet = udp->inBuf_[udp->inBufHead_];
  packet.data.clear();
  if (p->tot_len > 0) {
    packet.data.reserve(p->tot_len);
    // TODO: Limit vector size
    while (p != nullptr) {
      uint8_t *data = static_cast<uint8_t *>(p->payload);
      packet.data.insert(packet.data.end(), &data[0], &data[p->len]);
      p = p->next;
    }
  }
  packet.addr = *addr;
  packet.port = port;
  packet.receivedTimestamp = timestamp;
  packet.diffServ = pcb->tos;

  // Increment the size
  if (udp->inBufSize_ != 0 && udp->inBufTail_ == udp->inBufHead_) {
    // Full
    udp->inBufTail_ = (udp->inBufTail_ + 1) % udp->inBuf_.size();
  } else {
    udp->inBufSize_++;
  }
  udp->inBufHead_ = (udp->inBufHead_ + 1) % udp->inBuf_.size();

  pbuf_free(pHead);
}

EthernetUDP::EthernetUDP() : EthernetUDP(1) {}

EthernetUDP::EthernetUDP(size_t queueSize)
    : pcb_(nullptr),
      listening_(false),
      listenReuse_(false),
      listeningMulticast_(false),
      inBuf_(std::max(queueSize, size_t{1})),
      inBufTail_(0),
      inBufHead_(0),
      inBufSize_(0),
      packetPos_(-1),
      hasOutPacket_(false) {}

EthernetUDP::~EthernetUDP() {
  stop();
}

void EthernetUDP::setReceiveQueueSize(size_t size) {
  if (size == inBuf_.size()) {
    return;
  }

  size = std::max(size, size_t{1});

  // Keep all the newest elements
  // ATOMIC_BLOCK(ATOMIC_RESTORESTATE) {
    if (size <= inBufSize_) {
      // Keep all the newest packets
      if (inBufTail_ != 0) {
        size_t n = (inBufTail_ + (inBufSize_ - size)) % inBuf_.size();
        std::rotate(inBuf_.begin(), inBuf_.begin() + n, inBuf_.end());
      }
      inBuf_.resize(size);
      inBufHead_ = 0;
      inBufSize_ = size;
    } else {
      if (inBufTail_ != 0) {
        std::rotate(inBuf_.begin(), inBuf_.begin() + inBufTail_, inBuf_.end());
      }
      inBuf_.resize(size);
      inBufHead_ = inBufSize_;

      // Don't reserve memory because that might exhaust the heap
      // for (size_t i = oldSize; i < size; i++) {
      //   inBuf_[i].data.reserve(kMaxPayloadSize);
      // }
    }
    inBufTail_ = 0;
  // }

  inBuf_.shrink_to_fit();
}

uint8_t EthernetUDP::begin(uint16_t localPort) {
  return begin(localPort, false);
}

bool EthernetUDP::beginWithReuse(uint16_t localPort) {
  return begin(localPort, true);
}

void EthernetUDP::tryCreatePCB() {
  if (pcb_ == nullptr) {
    pcb_ = udp_new_ip_type(IPADDR_TYPE_ANY);
    if (pcb_ == nullptr) {
      Ethernet.loop();  // Allow the stack to move along
    }
  }
}

bool EthernetUDP::begin(uint16_t localPort, bool reuse) {
  if (listening_) {
    if (pcb_->local_port == localPort && listenReuse_ == reuse) {
      return true;
    }
    stop();
  }
  tryCreatePCB();
  if (pcb_ == nullptr) {
    errno = ENOMEM;
    return false;
  }

  // Try to bind
  if (reuse) {
    ip_set_option(pcb_, SOF_REUSEADDR);
  }

  err_t err;
  if ((err = udp_bind(pcb_, IP_ANY_TYPE, localPort))!= ERR_OK) {
    stop();
    errno = err_to_errno(err);
    return false;
  }

  listening_ = true;
  listenReuse_ = reuse;

  // Don't reserve memory because that might exhaust the heap
  // for (Packet &p : inBuf_) {
  //   p.data.reserve(kMaxPayloadSize);
  // }
  // if (packet_.data.capacity() < kMaxPayloadSize) {
  //   packet_.data.reserve(kMaxPayloadSize);
  // }

  udp_recv(pcb_, &recvFunc, this);

  return true;
}

uint8_t EthernetUDP::beginMulticast(IPAddress ip, uint16_t localPort) {
  return beginMulticast(ip, localPort, false);
}

bool EthernetUDP::beginMulticastWithReuse(IPAddress ip, uint16_t localPort) {
  return beginMulticast(ip, localPort, true);
}

bool EthernetUDP::beginMulticast(IPAddress ip, uint16_t localPort,
                                 bool reuse) {
  if (!begin(localPort, reuse)) {
    return false;
  }

  if (!Ethernet.joinGroup(ip)) {
    stop();
    return false;
  }
  listeningMulticast_ = true;
  multicastIP_ = ip;
  return true;
}

uint16_t EthernetUDP::localPort() const {
  if (pcb_ == nullptr) {
    return 0;
  }
  return pcb_->local_port;
}

void EthernetUDP::stop() {
  if (pcb_ == nullptr) {
    return;
  }

  if (listeningMulticast_) {
    Ethernet.leaveGroup(multicastIP_);
    listeningMulticast_ = false;
    multicastIP_ = INADDR_NONE;
  }

  udp_remove(pcb_);
  pcb_ = nullptr;
  listening_ = false;
  listenReuse_ = false;

  packet_.clear();
}

EthernetUDP::operator bool() const {
  return listening_;
}

bool EthernetUDP::setOutgoingDiffServ(uint8_t ds) {
  tryCreatePCB();
  if (pcb_ == nullptr) {
    errno = ENOMEM;
    return false;
  }
  pcb_->tos = ds;
  return true;
}

uint8_t EthernetUDP::outgoingDiffServ() const {
  if (pcb_ == nullptr) {
    return 0;
  }
  return pcb_->tos;
}

void EthernetUDP::Packet::clear() {
  data.clear();
  addr = *IP_ANY_TYPE;
  port = 0;
  receivedTimestamp = 0;
}

// --------------------------------------------------------------------------
//  Reception
// --------------------------------------------------------------------------

int EthernetUDP::parsePacket() {
  if (pcb_ == nullptr) {
    return -1;
  }

  Ethernet.loop();  // Allow the stack to move along

  if (inBufSize_ == 0) {
    packetPos_ = -1;
    return -1;
  }

  // Pop (from the tail)
  packet_ = inBuf_[inBufTail_];
  inBuf_[inBufTail_].clear();
  inBufTail_ = (inBufTail_ + 1) % inBuf_.size();
  inBufSize_--;

  packetPos_ = 0;
  return packet_.data.size();
}

inline bool EthernetUDP::isAvailable() const {
  return (0 <= packetPos_) &&
         (static_cast<size_t>(packetPos_) < packet_.data.size());
}

int EthernetUDP::available() {
  if (!isAvailable()) {
    return 0;
  }
  return packet_.data.size() - packetPos_;
}

int EthernetUDP::read() {
  if (!isAvailable()) {
    return -1;
  }
  return packet_.data[packetPos_++];
}

int EthernetUDP::read(uint8_t *buffer, size_t len) {
  if (len == 0 || !isAvailable()) {
    return 0;
  }
  len = std::min(len, packet_.data.size() - packetPos_);
  if (buffer != nullptr) {
    std::copy_n(&packet_.data.data()[packetPos_], len, buffer);
  }
  packetPos_ += len;
  return len;
}

int EthernetUDP::read(char *buffer, size_t len) {
  return read(reinterpret_cast<uint8_t *>(buffer), len);
}

int EthernetUDP::peek() {
  if (!isAvailable()) {
    return -1;
  }
  return packet_.data[packetPos_];
}

void EthernetUDP::flush() {
  // This flushes the input:
  // packetPos_ = -1;
  // Instead, do a no-op.
}

size_t EthernetUDP::size() const {
  return packet_.data.size();
}

const uint8_t *EthernetUDP::data() const {
  return packet_.data.data();
}

IPAddress EthernetUDP::remoteIP() {
#if LWIP_IPV4
  return ip_addr_get_ip4_uint32(&packet_.addr);
#else
  return INADDR_NONE;
#endif  // LWIP_IPV4
}

uint16_t EthernetUDP::remotePort() {
  return packet_.port;
}

uint32_t EthernetUDP::receivedTimestamp() const {
  return packet_.receivedTimestamp;
}

uint8_t EthernetUDP::receivedDiffServ() const {
  return packet_.diffServ;
}

// --------------------------------------------------------------------------
//  Transmission
// --------------------------------------------------------------------------

int EthernetUDP::beginPacket(IPAddress ip, uint16_t port) {
#if LWIP_IPV4
  ip_addr_t ipaddr IPADDR4_INIT(get_uint32(ip));
  return beginPacket(&ipaddr, port);
#else
  return false;
#endif  // LWIP_IPV4
}

int EthernetUDP::beginPacket(const char *host, uint16_t port) {
#if LWIP_DNS
  IPAddress ip;
  if (!DNSClient::getHostByName(host, ip,
                                QNETHERNET_DEFAULT_DNS_LOOKUP_TIMEOUT)) {
    return false;
  }
  return beginPacket(ip, port);
#else
  LWIP_UNUSED_ARG(host);
  LWIP_UNUSED_ARG(port);
  return false;
#endif  // LWIP_DNS
}

bool EthernetUDP::beginPacket(const ip_addr_t *ipaddr, uint16_t port) {
  tryCreatePCB();
  if (pcb_ == nullptr) {
    errno = ENOMEM;
    return false;
  }

  // Don't reserve memory because that might exhaust the heap
  // if (outPacket_.data.capacity() < kMaxPayloadSize) {
  //   outPacket_.data.reserve(kMaxPayloadSize);
  // }

  outPacket_.addr = *ipaddr;
  outPacket_.port = port;
  hasOutPacket_ = true;
  outPacket_.data.clear();
  return true;
}

int EthernetUDP::endPacket() {
  if (!hasOutPacket_) {
    return false;
  }
  hasOutPacket_ = false;

  // Note: Use PBUF_RAM for TX
  struct pbuf *p = pbuf_alloc(PBUF_TRANSPORT, outPacket_.data.size(), PBUF_RAM);
  if (p == nullptr) {
    outPacket_.clear();
    Ethernet.loop();  // Allow the stack to move along
    errno = ENOMEM;
    return false;
  }

  // pbuf_take() considers NULL data an error
  err_t err;
  if (!outPacket_.data.empty() &&
      (err = pbuf_take(p, outPacket_.data.data(), outPacket_.data.size())) != ERR_OK) {
    pbuf_free(p);
    errno = err_to_errno(err);
    return false;
  }

  // Repeat until not ERR_WOULDBLOCK because the low-level driver returns that
  // if there are no internal TX buffers available
  do {
    err = udp_sendto(pcb_, p, &outPacket_.addr, outPacket_.port);
    if (err != ERR_WOULDBLOCK) {
      break;
    }

    // udp_sendto() may have added a header
    if (p->tot_len > outPacket_.data.size()) {
      pbuf_remove_header(p, p->tot_len - outPacket_.data.size());
    }
  } while (true);

  outPacket_.clear();
  pbuf_free(p);

  if (err != ERR_OK) {
    errno = err_to_errno(err);
    return false;
  }
  return true;
}

bool EthernetUDP::send(const IPAddress &ip, uint16_t port,
                       const uint8_t *data, size_t len) {
#if LWIP_IPV4
  ip_addr_t ipaddr IPADDR4_INIT(get_uint32(ip));
  return send(&ipaddr, port, data, len);
#else
  return false;
#endif  // LWIP_IPV4
}

bool EthernetUDP::send(const char *host, uint16_t port,
                       const uint8_t *data, size_t len) {
#if LWIP_DNS
  IPAddress ip;
  if (!DNSClient::getHostByName(host, ip,
                                QNETHERNET_DEFAULT_DNS_LOOKUP_TIMEOUT)) {
    return false;
  }
  return send(ip, port, data, len);
#else
  LWIP_UNUSED_ARG(host);
  LWIP_UNUSED_ARG(port);
  LWIP_UNUSED_ARG(data);
  LWIP_UNUSED_ARG(len);
  return false;
#endif  // LWIP_DNS
}

bool EthernetUDP::send(const ip_addr_t *ipaddr, uint16_t port,
                       const uint8_t *data, size_t len) {
  if (len > kMaxPossiblePayloadSize) {
    errno = ENOBUFS;
    return false;
  }
  tryCreatePCB();
  if (pcb_ == nullptr) {
    errno = ENOMEM;
    return false;
  }

  // Note: Use PBUF_RAM for TX
  struct pbuf *p = pbuf_alloc(PBUF_TRANSPORT, len, PBUF_RAM);
  if (p == nullptr) {
    Ethernet.loop();  // Allow the stack to move along
    errno = ENOMEM;
    return false;
  }

  // pbuf_take() considers NULL data an error
  err_t err;
  if (len != 0 && (err = pbuf_take(p, data, len)) != ERR_OK) {
    pbuf_free(p);
    errno = err_to_errno(err);
    return false;
  }

  // Repeat until not ERR_WOULDBLOCK because the low-level driver returns that
  // if there are no internal TX buffers available
  do {
    err = udp_sendto(pcb_, p, ipaddr, port);
    if (err != ERR_WOULDBLOCK) {
      break;
    }

    // udp_sendto() may have added a header
    if (p->tot_len > len) {
      pbuf_remove_header(p, p->tot_len - len);
    }
  } while (true);

  pbuf_free(p);

  if (err != ERR_OK) {
    errno = err_to_errno(err);
    return false;
  }
  return true;
}

size_t EthernetUDP::write(uint8_t b) {
  if (!hasOutPacket_) {
    return 0;
  }
  if (outPacket_.data.size() >= kMaxPossiblePayloadSize) {
    return 0;
  }
  outPacket_.data.push_back(b);
  return 1;
}

size_t EthernetUDP::write(const uint8_t *buffer, size_t size) {
  if (!hasOutPacket_ || size == 0) {
    return 0;
  }
  size = std::min(kMaxPossiblePayloadSize - outPacket_.data.size(), size);
  outPacket_.data.insert(outPacket_.data.end(), &buffer[0], &buffer[size]);
  return size;
}

int EthernetUDP::availableForWrite() {
  if (!hasOutPacket_) {
    return 0;
  }
  if (outPacket_.data.size() > kMaxPossiblePayloadSize) {
    return 0;
  }
  return kMaxPossiblePayloadSize - outPacket_.data.size();
}

}  // namespace network
}  // namespace qindesign

#endif  // LWIP_UDP
