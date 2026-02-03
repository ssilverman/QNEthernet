// SPDX-FileCopyrightText: (c) 2021-2026 Shawn Silverman <shawn@pobox.com>
// SPDX-License-Identifier: AGPL-3.0-or-later

// QNEthernetUDP.cpp contains the EthernetUDP implementation.
// This file is part of the QNEthernet library.

#include "qnethernet/QNEthernetUDP.h"

#if LWIP_UDP

// C++ includes
#include <algorithm>
#include <cerrno>
#include <limits>
#include <utility>

#include "QNEthernet.h"
#include "lwip/debug.h"
#include "lwip/dns.h"
#include "lwip/err.h"
#include "lwip/ip.h"
#include "lwip/prot/ip4.h"
#include "lwip/sys.h"
#include "qnethernet/QNDNSClient.h"
#include "qnethernet/util/ip_tools.h"
#include "qnethernet_opts.h"

namespace qindesign {
namespace network {

// Total header size: Minimum IPv4 header size + UDP header size.
static constexpr size_t kHeaderSize = IP_HLEN + 8;

// Maximum UDP payload size without fragmentation.
static constexpr size_t kMaxPayloadSize =
    (EthernetClass::mtu() >= kHeaderSize) ? (EthernetClass::mtu() - kHeaderSize)
                                          : 0;

// Maximum possible payload size.
static constexpr size_t kMaxPossiblePayloadSize =
    (std::numeric_limits<uint16_t>::max() >= kHeaderSize)
        ? (std::numeric_limits<uint16_t>::max() - kHeaderSize)
        : 0;
static_assert(kMaxPossiblePayloadSize <= std::numeric_limits<uint16_t>::max(),
              "Max. possible payload size overflow");

void EthernetUDP::recvFunc(void* const arg, struct udp_pcb* const pcb,
                           struct pbuf* const p,
                           const ip_addr_t* const addr, const u16_t port) {
  if (pcb == nullptr) {
    return;
  }

  const auto udp = static_cast<EthernetUDP*>(arg);

  if (p == nullptr) {
    udp->stop();
    return;
  }

  const uint32_t timestamp = sys_now();

  struct pbuf* pNext = p;

  // Push
  if (udp->inBuf_.full()) {
    ++udp->droppedReceiveCount_;
  }
  Packet& packet = udp->inBuf_.put();
  packet.clear();
  if (pNext->tot_len > 0) {
    packet.data.reserve(pNext->tot_len);
    // TODO: Limit vector size
    while (pNext != nullptr) {
      const auto data = static_cast<const uint8_t*>(pNext->payload);
      (void)packet.data.insert(packet.data.cend(), &data[0], &data[pNext->len]);
      pNext = pNext->next;
    }
  }
  packet.addr = *addr;
  packet.port = port;
  packet.destAddr = *ip4_current_dest_addr();
  packet.receivedTimestamp = timestamp;
  packet.diffServ = pcb->tos;
  packet.ttl = pcb->ttl;

  (void)pbuf_free(p);

  ++udp->totalReceiveCount_;
}

EthernetUDP::EthernetUDP() : EthernetUDP(1) {}

EthernetUDP::EthernetUDP(const size_t capacity)
    : inBuf_(std::max(capacity, size_t{1})) {}

EthernetUDP::~EthernetUDP() noexcept {
  stop();
}

void EthernetUDP::setReceiveQueueCapacity(const size_t capacity) {
  // ATOMIC_BLOCK(ATOMIC_RESTORESTATE) {
  inBuf_.setCapacity(capacity);
  // }
}

uint8_t EthernetUDP::begin(const uint16_t localPort) {
  return begin(localPort, false);
}

bool EthernetUDP::beginWithReuse(const uint16_t localPort) {
  return begin(localPort, true);
}

bool EthernetUDP::tryCreatePCB() {
  if (pcb_ == nullptr) {
    pcb_ = udp_new_ip_type(IPADDR_TYPE_ANY);
    if (pcb_ == nullptr) {
      Ethernet.loop();  // Allow the stack to move along
      errno = ENOMEM;
      return false;
    }
  }
  return true;
}

bool EthernetUDP::begin(const uint16_t localPort, const bool reuse) {
  if (listening_) {
    if ((pcb_->local_port == localPort) && (listenReuse_ == reuse)) {
      return true;
    }
    stop();
  }
  if (!tryCreatePCB()) {
    return false;
  }

  // Try to bind
  if (reuse) {
    ip_set_option(pcb_, SOF_REUSEADDR);
  }

  const err_t err = udp_bind(pcb_, IP_ANY_TYPE, localPort);
  if (err != ERR_OK) {
    stop();
    errno = err_to_errno(err);
    return false;
  }

  listening_ = true;
  listenReuse_ = reuse;

  // Don't reserve memory because that might exhaust the heap
  // for (Packet& p : inBuf_) {
  //   p.data.reserve(kMaxPayloadSize);
  // }
  // if (packet_.data.capacity() < kMaxPayloadSize) {
  //   packet_.data.reserve(kMaxPayloadSize);
  // }

  udp_recv(pcb_, &recvFunc, this);

  // Reset some state
  droppedReceiveCount_ = 0;
  totalReceiveCount_ = 0;

  return true;
}

uint8_t EthernetUDP::beginMulticast(const IPAddress ip,
                                    const uint16_t localPort) {
  return beginMulticast(ip, localPort, false);
}

bool EthernetUDP::beginMulticastWithReuse(const IPAddress& ip,
                                          const uint16_t localPort) {
  return beginMulticast(ip, localPort, true);
}

bool EthernetUDP::beginMulticast(const IPAddress& ip, const uint16_t localPort,
                                 const bool reuse) {
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
    (void)Ethernet.leaveGroup(multicastIP_);
    listeningMulticast_ = false;
    multicastIP_ = INADDR_NONE;
  }

  udp_remove(pcb_);
  pcb_ = nullptr;
  listening_ = false;
  listenReuse_ = false;
}

bool EthernetUDP::setOutgoingDiffServ(const uint8_t ds) {
  if (!tryCreatePCB()) {
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

bool EthernetUDP::setOutgoingTTL(const uint8_t ttl) {
  if (!tryCreatePCB()) {
    return false;
  }
  pcb_->ttl = ttl;
  return true;
}

uint8_t EthernetUDP::outgoingTTL() const {
  if (pcb_ == nullptr) {
    return 0;
  }
  return pcb_->ttl;
}

void EthernetUDP::Packet::clear() {
  diffServ = 0;
  ttl = 0;
  data.clear();
  addr = *IP_ANY_TYPE;
  port = 0;
  destAddr = *IP_ANY_TYPE;
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

  if (inBuf_.empty()) {
    packetPos_ = -1;
    return -1;
  }

  // Pop
  packet_ = std::move(inBuf_.get());

  packetPos_ = 0;
  return static_cast<int>(packet_.data.size());
}

inline bool EthernetUDP::isAvailable() const {
  return (0 <= packetPos_) &&
         (static_cast<size_t>(packetPos_) < packet_.data.size());
}

int EthernetUDP::available() {
  if (!isAvailable()) {
    return 0;
  }
  return static_cast<int>(packet_.data.size() - packetPos_);
}

int EthernetUDP::read() {
  if (!isAvailable()) {
    return -1;
  }
  return packet_.data[packetPos_++];
}

int EthernetUDP::read(uint8_t* const buffer, const size_t len) {
  if ((len == 0) || !isAvailable()) {
    return 0;
  }
  const size_t actualLen = std::min(len, packet_.data.size() - packetPos_);
  if (buffer != nullptr) {
    (void)std::copy_n(&packet_.data.data()[packetPos_], actualLen, buffer);
  }
  packetPos_ += actualLen;
  return actualLen;
}

int EthernetUDP::read(char* const buffer, const size_t len) {
  return read(reinterpret_cast<uint8_t*>(buffer), len);
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

IPAddress EthernetUDP::remoteIP() {
#if LWIP_IPV4
  return util::ip_addr_get_ip4_uint32(&packet_.addr);
#else
  errno = ENOSYS;
  return INADDR_NONE;
#endif  // LWIP_IPV4
}

IPAddress EthernetUDP::destIP() const {
#if LWIP_IPV4
  return util::ip_addr_get_ip4_uint32(&packet_.destAddr);
#else
  errno = ENOSYS;
  return INADDR_NONE;
#endif  // LWIP_IPV4
}

// --------------------------------------------------------------------------
//  Transmission
// --------------------------------------------------------------------------

int EthernetUDP::beginPacket(const IPAddress ip, const uint16_t port) {
#if LWIP_IPV4
  const ip_addr_t ipaddr IPADDR4_INIT(static_cast<uint32_t>(ip));
  return beginPacket(&ipaddr, port);
#else
  errno = ENOSYS;
  return false;
#endif  // LWIP_IPV4
}

int EthernetUDP::beginPacket(const char* const host, const uint16_t port) {
#if LWIP_DNS
  IPAddress ip;
  if (!DNSClient::getHostByName(host, ip)) {
    return false;
  }
  return beginPacket(ip, port);
#else
  LWIP_UNUSED_ARG(host);
  LWIP_UNUSED_ARG(port);
  errno = ENOSYS;
  return false;
#endif  // LWIP_DNS
}

bool EthernetUDP::beginPacket(const ip_addr_t* const ipaddr,
                              const uint16_t port) {
  if (!tryCreatePCB()) {
    return false;
  }

  // Don't reserve memory because that might exhaust the heap
  // if (outPacket_.data.capacity() < kMaxPayloadSize) {
  //   outPacket_.data.reserve(kMaxPayloadSize);
  // }

  outPacket_.value.addr = *ipaddr;
  outPacket_.value.port = port;
  outPacket_.value.data.clear();
  outPacket_.has_value = true;
  return true;
}

// Repeat send retry until the driver doesn't return ERR_WOULDBLOCK.
ATTRIBUTE_NODISCARD
static inline err_t sendWhileWouldBlock(const err_t err, udp_pcb* const pcb,
                                        pbuf* const p,
                                        const ip_addr_t* const ip,
                                        const uint16_t port,
                                        const uint16_t dataSize) {
  err_t retval = err;

  // Repeat until not ERR_WOULDBLOCK because the low-level driver returns that
  // if there are no internal TX buffers available
  do {
    retval = udp_sendto(pcb, p, ip, port);
    if (retval != ERR_WOULDBLOCK) {
      break;
    }

    // udp_sendto() may have added a header
    if (p->tot_len > dataSize) {
      LWIP_ASSERT("Expected removed header",
                  pbuf_remove_header(p, p->tot_len - dataSize) == 0);
    }
  } while (true);

  return retval;
}

int EthernetUDP::endPacket() {
  if (!outPacket_.has_value) {
    return false;
  }
  Packet& op = outPacket_.value;

  // op.data.size() should be <= UINT16_MAX
  LWIP_ASSERT("Output packet too large",
              op.data.size() <= std::numeric_limits<uint16_t>::max());
  auto outSize = static_cast<uint16_t>(op.data.size());

  // Note: Use PBUF_RAM for TX
  struct pbuf* const p = pbuf_alloc(PBUF_TRANSPORT, outSize, PBUF_RAM);
  if (p == nullptr) {
    outPacket_.has_value = false;
    op.clear();

    Ethernet.loop();  // Allow the stack to move along
    errno = ENOMEM;
    return false;
  }

  // pbuf_take() considers NULL data an error
  err_t err;
  if (!op.data.empty() &&
      ((err = pbuf_take(p, op.data.data(), outSize)) != ERR_OK)) {
    (void)pbuf_free(p);

    outPacket_.has_value = false;
    op.clear();

    Ethernet.loop();  // Allow the stack to move along
    errno = err_to_errno(err);
    return false;
  }

  err = sendWhileWouldBlock(err, pcb_, p, &op.addr, op.port, outSize);

  outPacket_.has_value = false;
  op.clear();

  (void)pbuf_free(p);

  if (err != ERR_OK) {
    errno = err_to_errno(err);
    return false;
  }
  return true;
}

bool EthernetUDP::send(const IPAddress& ip, const uint16_t port,
                       const void* const data, const size_t len) {
#if LWIP_IPV4
  const ip_addr_t ipaddr IPADDR4_INIT(static_cast<uint32_t>(ip));
  return send(&ipaddr, port, data, len);
#else
  errno = ENOSYS;
  return false;
#endif  // LWIP_IPV4
}

bool EthernetUDP::send(const char* const host, const uint16_t port,
                       const void* const data, const size_t len) {
#if LWIP_DNS
  IPAddress ip;
  if (!DNSClient::getHostByName(host, ip)) {
    return false;
  }
  return send(ip, port, data, len);
#else
  LWIP_UNUSED_ARG(host);
  LWIP_UNUSED_ARG(port);
  LWIP_UNUSED_ARG(data);
  LWIP_UNUSED_ARG(len);
  errno = ENOSYS;
  return false;
#endif  // LWIP_DNS
}

bool EthernetUDP::send(const ip_addr_t* const ipaddr, const uint16_t port,
                       const void* const data, const size_t len) {
  if (len > kMaxPossiblePayloadSize) {
    errno = ENOBUFS;
    return false;
  }
  if (!tryCreatePCB()) {
    return false;
  }

  // Adjust the length's type to 16 bits
  // kMaxPossiblePayloadSize is < UINT16_MAX
  const auto adjLen = static_cast<uint16_t>(len);

  // Note: Use PBUF_RAM for TX
  struct pbuf* const p = pbuf_alloc(PBUF_TRANSPORT, adjLen, PBUF_RAM);
  if (p == nullptr) {
    Ethernet.loop();  // Allow the stack to move along
    errno = ENOMEM;
    return false;
  }

  // pbuf_take() considers NULL data an error
  err_t err;
  if ((adjLen != 0) && ((err = pbuf_take(p, data, adjLen)) != ERR_OK)) {
    (void)pbuf_free(p);
    errno = err_to_errno(err);
    return false;
  }

  err = sendWhileWouldBlock(err, pcb_, p, ipaddr, port, adjLen);

  (void)pbuf_free(p);

  if (err != ERR_OK) {
    errno = err_to_errno(err);
    return false;
  }
  return true;
}

size_t EthernetUDP::write(const uint8_t b) {
  if (!outPacket_.has_value) {
    return 0;
  }
  if (outPacket_.value.data.size() >= kMaxPossiblePayloadSize) {
    return 0;
  }
  outPacket_.value.data.push_back(b);
  return 1;
}

size_t EthernetUDP::write(const uint8_t* const buffer, const size_t size) {
  if (!outPacket_.has_value || (size == 0)) {
    return 0;
  }
  const size_t actualSize =
      std::min(kMaxPossiblePayloadSize - outPacket_.value.data.size(), size);
  (void)outPacket_.value.data.insert(outPacket_.value.data.cend(), &buffer[0],
                                     &buffer[actualSize]);
  return actualSize;
}

int EthernetUDP::availableForWrite() {
  if (!outPacket_.has_value) {
    return 0;
  }
  if (outPacket_.value.data.size() >= kMaxPossiblePayloadSize) {
    return 0;
  }
  return static_cast<int>(kMaxPossiblePayloadSize -
                          outPacket_.value.data.size());
}

}  // namespace network
}  // namespace qindesign

#endif  // LWIP_UDP
