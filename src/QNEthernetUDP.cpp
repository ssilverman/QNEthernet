// SPDX-FileCopyrightText: (c) 2021-2022 Shawn Silverman <shawn@pobox.com>
// SPDX-License-Identifier: MIT

// QNEthernetUDP.cpp contains the EthernetUDP implementation.
// This file is part of the QNEthernet library.

#include "QNEthernetUDP.h"

// C++ includes
#include <algorithm>

#include "QNDNSClient.h"
#include "QNEthernet.h"
#include "lwip/dns.h"
#include "lwip/ip.h"
#include "util/ip_tools.h"

namespace qindesign {
namespace network {

// Maximum UDP packet size.
// Subtract UDP header size and minimum IPv4 header size.
constexpr size_t kMaxUDPSize = EthernetClass::mtu() - 8 - 20;

// DNS lookup timeout.
static constexpr uint32_t kDNSLookupTimeout =
    DNS_MAX_RETRIES * DNS_TMR_INTERVAL;

void EthernetUDP::recvFunc(void *arg, struct udp_pcb *pcb, struct pbuf *p,
                           const ip_addr_t *addr, u16_t port) {
  if (arg == nullptr || pcb == nullptr) {
    return;
  }

  EthernetUDP *udp = reinterpret_cast<EthernetUDP *>(arg);

  if (p == nullptr) {
    udp->stop();
    return;
  }

  struct pbuf *pHead = p;

  udp->inPacket_.clear();
  udp->inPacket_.reserve(p->tot_len);
  // TODO: Limit vector size
  while (p != nullptr) {
    unsigned char *data = reinterpret_cast<unsigned char *>(p->payload);
    udp->inPacket_.insert(udp->inPacket_.end(), &data[0], &data[p->len]);
    p = p->next;
  }
  udp->inAddr_ = *addr;
  udp->inPort_ = port;

  pbuf_free(pHead);
}

EthernetUDP::EthernetUDP()
    : pcb_(nullptr),
      inPacket_{},
      inAddr_{*IP_ANY_TYPE},
      inPort_(0),
      packet_{},
      packetPos_(-1),
      addr_{*IP_ANY_TYPE},
      port_(0),
      hasOutPacket_(false),
      outAddr_{*IP_ANY_TYPE},
      outPort_(0),
      outPacket_{} {}

EthernetUDP::~EthernetUDP() {
  stop();
}

uint8_t EthernetUDP::begin(uint16_t localPort) {
  return begin(localPort, false);
}

uint8_t EthernetUDP::begin(uint16_t localPort, bool reuse) {
  if (pcb_ == nullptr) {
    pcb_ = udp_new();
  }
  if (pcb_ == nullptr) {
    return false;
  }

  // Try to bind
  if (reuse) {
    ip_set_option(pcb_, SOF_REUSEADDR);
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
  return beginMulticast(ip, localPort, false);
}

uint8_t EthernetUDP::beginMulticast(IPAddress ip, uint16_t localPort,
                                    bool reuse) {
  if (!begin(localPort, reuse)) {
    return false;
  }

  if (!Ethernet.joinGroup(ip)) {
    stop();
    return false;
  }
  return true;
}

uint16_t EthernetUDP::localPort() {
  if (pcb_ == nullptr) {
    return 0;
  }
  return pcb_->local_port;
}

void EthernetUDP::stop() {
  if (pcb_ == nullptr) {
    return;
  }
  udp_remove(pcb_);
  pcb_ = nullptr;

  addr_ = *IP_ANY_TYPE;
  port_ = 0;
}

// --------------------------------------------------------------------------
//  Reception
// --------------------------------------------------------------------------

int EthernetUDP::parsePacket() {
  if (pcb_ == nullptr) {
    return 0;
  }

  packet_ = inPacket_;
  addr_ = inAddr_;
  port_ = inPort_;
  inPacket_.clear();

  EthernetClass::loop();  // Allow the stack to move along

  if (packet_.size() > 0) {
    packetPos_ = 0;
    return packet_.size();
  } else {
    packetPos_ = -1;
    return 0;
  }
}

inline bool EthernetUDP::isAvailable() const {
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
  if (len == 0 || !isAvailable()) {
    return 0;
  }
  len = std::min(len, packet_.size() - packetPos_);
  if (buffer != nullptr) {
    std::copy_n(&packet_.data()[packetPos_], len, buffer);
  }
  packetPos_ += len;
  return len;
}

int EthernetUDP::read(char *buffer, size_t len) {
  return read(reinterpret_cast<unsigned char *>(buffer), len);
}

int EthernetUDP::peek() {
  if (!isAvailable()) {
    return -1;
  }
  return packet_[packetPos_];
}

void EthernetUDP::flush() {
  // This flushes the input:
  // packetPos_ = -1;
  // Instead, do a no-op.
}

const unsigned char *EthernetUDP::data() const {
  return packet_.data();
}

IPAddress EthernetUDP::remoteIP() {
  return ip_addr_get_ip4_uint32(&addr_);
}

uint16_t EthernetUDP::remotePort() {
  return port_;
}

// --------------------------------------------------------------------------
//  Transmission
// --------------------------------------------------------------------------

int EthernetUDP::beginPacket(IPAddress ip, uint16_t port) {
  ip_addr_t ipaddr IPADDR4_INIT(get_uint32(ip));
  return beginPacket(&ipaddr, port);
}

int EthernetUDP::beginPacket(const char *host, uint16_t port) {
  IPAddress ip;
  if (!DNSClient::getHostByName(host, ip, kDNSLookupTimeout)) {
    return false;
  }
  return beginPacket(ip, port);
}

bool EthernetUDP::beginPacket(const ip_addr_t *ipaddr, uint16_t port) {
  if (pcb_ == nullptr) {
    pcb_ = udp_new();
  }
  if (pcb_ == nullptr) {
    return false;
  }
  if (outPacket_.capacity() < kMaxUDPSize) {
    outPacket_.reserve(kMaxUDPSize);
  }

  outAddr_ = *ipaddr;
  outPort_ = port;
  hasOutPacket_ = true;
  outPacket_.clear();
  return true;
}

int EthernetUDP::endPacket() {
  if (!hasOutPacket_) {
    return false;
  }
  hasOutPacket_ = false;

  if (outPacket_.size() > UINT16_MAX) {
    outPacket_.clear();
    return false;
  }

  // Note: Use PBUF_RAM for TX
  struct pbuf *p = pbuf_alloc(PBUF_TRANSPORT, outPacket_.size(), PBUF_RAM);
  if (p == nullptr) {
    return false;
  }
  pbuf_take(p, outPacket_.data(), outPacket_.size());
  outPacket_.clear();
  bool retval = (udp_sendto(pcb_, p, &outAddr_, outPort_) == ERR_OK);
  pbuf_free(p);
  return retval;
}

bool EthernetUDP::send(const IPAddress &ip, uint16_t port,
                       const uint8_t *data, size_t len) {
  ip_addr_t ipaddr IPADDR4_INIT(get_uint32(ip));
  return send(&ipaddr, port, data, len);
}

bool EthernetUDP::send(const char *host, uint16_t port,
                       const uint8_t *data, size_t len) {
  IPAddress ip;
  if (!DNSClient::getHostByName(host, ip, kDNSLookupTimeout)) {
    return false;
  }
  return send(ip, port, data, len);
}

bool EthernetUDP::send(const ip_addr_t *ipaddr, uint16_t port,
                       const uint8_t *data, size_t len) {
  if (len > UINT16_MAX) {
    return false;
  }
  if (pcb_ == nullptr) {
    pcb_ = udp_new();
  }
  if (pcb_ == nullptr) {
    return false;
  }

  // Note: Use PBUF_RAM for TX
  struct pbuf *p = pbuf_alloc(PBUF_TRANSPORT, len, PBUF_RAM);
  if (p == nullptr) {
    return false;
  }
  pbuf_take(p, data, len);
  bool retval = (udp_sendto(pcb_, p, ipaddr, port) == ERR_OK);
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
