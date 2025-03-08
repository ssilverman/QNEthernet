// SPDX-FileCopyrightText: (c) 2022-2025 Shawn Silverman <shawn@pobox.com>
// SPDX-License-Identifier: AGPL-3.0-or-later

// QNEthernetFrame.cpp contains an EthernetFrame implementation.
// This file is part of the QNEthernet library.

#include "qnethernet/QNEthernetFrame.h"

#if QNETHERNET_ENABLE_RAW_FRAME_SUPPORT

// C++ includes
#include <algorithm>

#include <avr/pgmspace.h>

#include "QNEthernet.h"
#include "lwip/arch.h"
#include "lwip/prot/ieee.h"
#include "lwip/sys.h"

extern "C" {
void qnethernet_hal_disable_interrupts(void);
void qnethernet_hal_enable_interrupts(void);
}  // extern "C"

#ifndef FLASHMEM
#define FLASHMEM
#endif  // !FLASHMEM

extern "C" {
err_t unknown_eth_protocol(struct pbuf *const p, struct netif *const netif) {
#if ETH_PAD_SIZE
  pbuf_remove_header(p, ETH_PAD_SIZE);
#endif  // ETH_PAD_SIZE

  return qindesign::network::EthernetFrameClass::recvFunc(p, netif);
}
}  // extern "C"

namespace qindesign {
namespace network {

// A reference to the singleton.
STATIC_INIT_DEFN(EthernetFrameClass, EthernetFrame);

err_t EthernetFrameClass::recvFunc(struct pbuf *const p,
                                   struct netif *const netif) {
  LWIP_UNUSED_ARG(netif);

  const uint32_t timestamp = sys_now();

  struct pbuf *pNext = p;

  // Push (replace the head)
  Frame &frame = EthernetFrame.inBuf_[EthernetFrame.inBufHead_];
  frame.data.clear();
  frame.data.reserve(p->tot_len);
  // TODO: Limit vector size
  while (pNext != nullptr) {
    uint8_t *const data = static_cast<uint8_t *>(pNext->payload);
    frame.data.insert(frame.data.cend(), &data[0], &data[pNext->len]);
    pNext = pNext->next;
  }
  frame.receivedTimestamp = timestamp;

  // Increment the size
  if (EthernetFrame.inBufSize_ != 0 &&
      EthernetFrame.inBufTail_ == EthernetFrame.inBufHead_) {
    // Full
    EthernetFrame.inBufTail_ =
        (EthernetFrame.inBufTail_ + 1) % EthernetFrame.inBuf_.size();
    EthernetFrame.droppedReceiveCount_++;
  } else {
    EthernetFrame.inBufSize_++;
  }
  EthernetFrame.inBufHead_ =
      (EthernetFrame.inBufHead_ + 1) % EthernetFrame.inBuf_.size();

  pbuf_free(p);
  EthernetFrame.totalReceiveCount_++;

  return ERR_OK;
}

FLASHMEM EthernetFrameClass::EthernetFrameClass()
    : inBuf_(1),
      inBufTail_(0),
      inBufHead_(0),
      inBufSize_(0),
      frame_{},
      framePos_(-1),
      hasOutFrame_(false),
      outFrame_{},
      droppedReceiveCount_(0),
      totalReceiveCount_(0) {
  setReceiveQueueCapacity(1);
}

void EthernetFrameClass::Frame::clear() {
  data.clear();
  receivedTimestamp = 0;
}

void EthernetFrameClass::clear() {
  // Outgoing
  hasOutFrame_ = false;
  outFrame_.clear();

  // Incoming
  for (Frame &f : inBuf_) {
    f.clear();
  }
  inBufTail_ = 0;
  inBufHead_ = 0;
  inBufSize_ = 0;
}

// --------------------------------------------------------------------------
//  Reception
// --------------------------------------------------------------------------

int EthernetFrameClass::parseFrame() {
  if (inBufSize_ == 0) {
    framePos_ = -1;
    return -1;
  }

  // Pop (from the tail)
  frame_ = inBuf_[inBufTail_];
  inBuf_[inBufTail_].clear();
  inBufTail_ = (inBufTail_ + 1) % inBuf_.size();
  inBufSize_--;

  Ethernet.loop();  // Allow the stack to move along

  if (frame_.data.size() > 0) {
    framePos_ = 0;
    return frame_.data.size();
  } else {
    framePos_ = -1;
    return 0;
  }
}

inline bool EthernetFrameClass::isAvailable() const {
  return (0 <= framePos_ &&
          static_cast<size_t>(framePos_) < frame_.data.size());
}

int EthernetFrameClass::available() {
  if (!isAvailable()) {
    return 0;
  }
  return frame_.data.size() - framePos_;
}

int EthernetFrameClass::read() {
  if (!isAvailable()) {
    return -1;
  }
  return frame_.data[framePos_++];
}

int EthernetFrameClass::read(void *const buffer, const size_t len) {
  if (len == 0 || !isAvailable()) {
    return 0;
  }
  const size_t actualLen = std::min(len, frame_.data.size() - framePos_);
  if (buffer != nullptr) {
    std::copy_n(&frame_.data.data()[framePos_], actualLen,
                static_cast<uint8_t *>(buffer));
  }
  framePos_ += actualLen;
  return actualLen;
}

int EthernetFrameClass::peek() {
  if (!isAvailable()) {
    return -1;
  }
  return frame_.data[framePos_];
}

const uint8_t *EthernetFrameClass::destinationMAC() const {
  return data();
}

const uint8_t *EthernetFrameClass::sourceMAC() const {
  return data() + 6;
}

uint16_t EthernetFrameClass::etherTypeOrLength() const {
  if (size() < 14) {
    return 0;
  }
  const uint8_t *const p = data();
  return (uint16_t{p[12]} << 8) | uint16_t{p[13]};
}

const uint8_t *EthernetFrameClass::payload() const {
  return data() + 14;
}

void EthernetFrameClass::setReceiveQueueCapacity(const size_t capacity) {
  if (capacity == inBuf_.size()) {
    return;
  }

  const size_t actualCap = std::max(capacity, size_t{1});

  // Keep all the newest elements
  qnethernet_hal_disable_interrupts();
  if (actualCap <= inBufSize_) {
    // Keep all the newest frames
    if (inBufTail_ != 0) {
      const size_t n = (inBufTail_ + (inBufSize_ - actualCap)) % inBuf_.size();
      std::rotate(inBuf_.begin(), inBuf_.begin() + n, inBuf_.end());
    }
    inBuf_.resize(actualCap);
    inBufHead_ = 0;
    inBufSize_ = actualCap;
  } else {
    if (inBufTail_ != 0) {
      std::rotate(inBuf_.begin(), inBuf_.begin() + inBufTail_, inBuf_.end());
    }
    inBuf_.resize(actualCap);
    inBufHead_ = inBufSize_;

    // Don't reserve memory because that might exhaust the heap
    // for (size_t i = oldSize; i < actualCap; i++) {
    //   inBuf_[i].data.reserve(maxFrameLen());
    // }
  }
  inBufTail_ = 0;
  qnethernet_hal_enable_interrupts();

  inBuf_.shrink_to_fit();
}

// --------------------------------------------------------------------------
//  Transmission
// --------------------------------------------------------------------------

void EthernetFrameClass::beginFrame() {
  // Don't reserve memory because that might exhaust the heap
  // if (outFrame_.data.capacity() < maxFrameLen()) {
  //   outFrame_.data.reserve(maxFrameLen());
  // }

  hasOutFrame_ = true;
  outFrame_.data.clear();
}

void EthernetFrameClass::beginFrame(const uint8_t dstAddr[ETH_HWADDR_LEN],
                                    const uint8_t srcAddr[ETH_HWADDR_LEN],
                                    const uint16_t typeOrLength) {
  beginFrame();
  write(dstAddr, ETH_HWADDR_LEN);
  write(srcAddr, ETH_HWADDR_LEN);
  write(static_cast<uint8_t>(typeOrLength >> 8));
  write(static_cast<uint8_t>(typeOrLength));
}

void EthernetFrameClass::beginVLANFrame(const uint8_t dstAddr[ETH_HWADDR_LEN],
                                        const uint8_t srcAddr[ETH_HWADDR_LEN],
                                        const uint16_t vlanInfo,
                                        const uint16_t typeOrLength) {
  beginFrame(dstAddr, srcAddr, ETHTYPE_VLAN);
  write(static_cast<uint8_t>(vlanInfo >> 8));
  write(static_cast<uint8_t>(vlanInfo));
  write(static_cast<uint8_t>(typeOrLength >> 8));
  write(static_cast<uint8_t>(typeOrLength));
}

bool EthernetFrameClass::endFrame() {
  if (!hasOutFrame_) {
    return false;
  }
  hasOutFrame_ = false;

  const bool retval =
      enet_output_frame(outFrame_.data.data(), outFrame_.data.size());
  outFrame_.clear();
  return retval;
}

bool EthernetFrameClass::send(const void *const frame, const size_t len) const {
  return enet_output_frame(frame, len);
}

size_t EthernetFrameClass::write(const uint8_t b) {
  if (!hasOutFrame_ || availableForWrite() <= 0) {
    return 0;
  }
  outFrame_.data.push_back(b);
  return 1;
}

size_t EthernetFrameClass::write(const uint8_t *const buffer,
                                 const size_t size) {
  const int avail = availableForWrite();
  if (!hasOutFrame_ || size == 0 || avail <= 0) {
    return 0;
  }

  const size_t actualSize = std::min(size, static_cast<size_t>(avail));
  outFrame_.data.insert(outFrame_.data.end(), &buffer[0], &buffer[actualSize]);
  return actualSize;
}

int EthernetFrameClass::availableForWrite() {
  if (!hasOutFrame_) {
    return 0;
  }
  if (outFrame_.data.size() > (maxFrameLen() - 4)) {
    return 0;
  }
  return (maxFrameLen() - 4) - outFrame_.data.size();
}

}  // namespace network
}  // namespace qindesign

#endif  // QNETHERNET_ENABLE_RAW_FRAME_SUPPORT
