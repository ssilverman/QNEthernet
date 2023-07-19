// SPDX-FileCopyrightText: (c) 2022-2023 Shawn Silverman <shawn@pobox.com>
// SPDX-License-Identifier: MIT

// QNEthernetFrame.cpp contains an EthernetFrame implementation.
// This file is part of the QNEthernet library.

#ifdef QNETHERNET_ENABLE_RAW_FRAME_SUPPORT

#include "QNEthernetFrame.h"

// C++ includes
#include <algorithm>

#include <pgmspace.h>
#include <util/atomic.h>

#include "QNEthernet.h"
#include "lwip/prot/ieee.h"

extern "C" {
err_t unknown_eth_protocol(struct pbuf *p, struct netif *netif) {
#if ETH_PAD_SIZE
  pbuf_remove_header(p, ETH_PAD_SIZE);
#endif  // ETH_PAD_SIZE

  return qindesign::network::EthernetFrameClass::recvFunc(p, netif);
}
}  // extern "C"

namespace qindesign {
namespace network {

EthernetFrameClass &EthernetFrameClass::instance() {
  static EthernetFrameClass instance;
  return instance;
}

// A reference to the singleton.
EthernetFrameClass &EthernetFrame = EthernetFrameClass::instance();

err_t EthernetFrameClass::recvFunc(struct pbuf *p,
                                   [[maybe_unused]] struct netif *netif) {
  uint32_t timestamp = millis();

  struct pbuf *pHead = p;

  // Push (replace the head)
  Frame &frame = EthernetFrame.inBuf_[EthernetFrame.inBufHead_];
  frame.data.clear();
  frame.data.reserve(p->tot_len);
  // TODO: Limit vector size
  while (p != nullptr) {
    uint8_t *data = static_cast<uint8_t *>(p->payload);
    frame.data.insert(frame.data.end(), &data[0], &data[p->len]);
    p = p->next;
  }
  frame.receivedTimestamp = timestamp;

  // Increment the size
  if (EthernetFrame.inBufSize_ != 0 &&
      EthernetFrame.inBufTail_ == EthernetFrame.inBufHead_) {
    // Full
    EthernetFrame.inBufTail_ =
        (EthernetFrame.inBufTail_ + 1) % EthernetFrame.inBuf_.size();
  } else {
    EthernetFrame.inBufSize_++;
  }
  EthernetFrame.inBufHead_ =
      (EthernetFrame.inBufHead_ + 1) % EthernetFrame.inBuf_.size();

  pbuf_free(pHead);

  return ERR_OK;
}

FLASHMEM EthernetFrameClass::EthernetFrameClass()
    : inBuf_(1) {
  setReceiveQueueSize(1);
}

void EthernetFrameClass::Frame::clear() {
  data.clear();
  receivedTimestamp = 0;
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

int EthernetFrameClass::read(uint8_t *buffer, size_t len) {
  if (len == 0 || !isAvailable()) {
    return 0;
  }
  len = std::min(len, frame_.data.size() - framePos_);
  if (buffer != nullptr) {
    std::copy_n(&frame_.data.data()[framePos_], len, buffer);
  }
  framePos_ += len;
  return len;
}

int EthernetFrameClass::read(char *buffer, size_t len) {
  return read(reinterpret_cast<uint8_t *>(buffer), len);
}

int EthernetFrameClass::peek() {
  if (!isAvailable()) {
    return -1;
  }
  return frame_.data[framePos_];
}

size_t EthernetFrameClass::size() const {
  return frame_.data.size();
}

const uint8_t *EthernetFrameClass::data() const {
  return frame_.data.data();
}

uint32_t EthernetFrameClass::receivedTimestamp() const {
  return frame_.receivedTimestamp;
}

void EthernetFrameClass::setReceiveQueueSize(size_t size) {
  if (size == inBuf_.size()) {
    return;
  }

  size = std::max(size, size_t{1});

  // Keep all the newest elements
  ATOMIC_BLOCK(ATOMIC_RESTORESTATE) {
    if (size <= inBufSize_) {
      // Keep all the newest frames
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
      //   inBuf_[i].data.reserve(maxFrameLen());
      // }
    }
    inBufTail_ = 0;
  }

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

void EthernetFrameClass::beginFrame(const uint8_t dstAddr[6],
                                    const uint8_t srcAddr[6],
                                    uint16_t typeOrLength) {
  beginFrame();
  write(dstAddr, 6);
  write(srcAddr, 6);
  write(static_cast<uint8_t>(typeOrLength >> 8));
  write(static_cast<uint8_t>(typeOrLength));
}

void EthernetFrameClass::beginVLANFrame(const uint8_t dstAddr[6],
                                        const uint8_t srcAddr[6],
                                        uint16_t vlanInfo,
                                        uint16_t typeOrLength) {
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

  bool retval = enet_output_frame(outFrame_.data.data(), outFrame_.data.size());
  outFrame_.clear();
  return retval;
}

bool EthernetFrameClass::send(const uint8_t *frame, size_t len) const {
  return enet_output_frame(frame, len);
}

size_t EthernetFrameClass::write(uint8_t b) {
  if (!hasOutFrame_ || availableForWrite() <= 0) {
    return 0;
  }
  outFrame_.data.push_back(b);
  return 1;
}

size_t EthernetFrameClass::write(const uint8_t *buffer, size_t size) {
  int avail = availableForWrite();
  if (!hasOutFrame_ || size == 0 || avail <= 0) {
    return 0;
  }

  size = std::min(size, static_cast<size_t>(avail));
  outFrame_.data.insert(outFrame_.data.end(), &buffer[0], &buffer[size]);
  return size;
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
