// SPDX-FileCopyrightText: (c) 2022 Shawn Silverman <shawn@pobox.com>
// SPDX-License-Identifier: MIT

// QNEthernetFrame.cpp contains an EthernetFrame implementation.
// This file is part of the QNEthernet library.

#include "QNEthernetFrame.h"

// C++ includes
#include <algorithm>

#include "QNEthernet.h"
#include "lwip/prot/ieee.h"

extern "C" {
err_t unknown_eth_protocol(struct pbuf *p, struct netif *netif) {
  return qindesign::network::EthernetFrameClass::recvFunc(p, netif);
}
}  // extern "C"

namespace qindesign {
namespace network {

// Define the singleton instance.
EthernetFrameClass EthernetFrameClass::instance_;

// A reference to the singleton.
EthernetFrameClass &EthernetFrame = EthernetFrameClass::instance();

err_t EthernetFrameClass::recvFunc(struct pbuf *p, struct netif *netif) {
  struct pbuf *pHead = p;

  EthernetFrame.inFrame_.clear();
  EthernetFrame.inFrame_.reserve(p->tot_len);
  // TODO: Limit vector size
  while (p != nullptr) {
    unsigned char *data = reinterpret_cast<unsigned char *>(p->payload);
    EthernetFrame.inFrame_.insert(EthernetFrame.inFrame_.end(),
                                  &data[0], &data[p->len]);
    p = p->next;
  }

  EthernetFrame.hasIEEE1588Timestamp_ = pHead->timestampValid;
  EthernetFrame.ieee1588Timestamp_ = pHead->timestamp;

  pbuf_free(pHead);

  return ERR_OK;
}

// --------------------------------------------------------------------------
//  Reception
// --------------------------------------------------------------------------

int EthernetFrameClass::parseFrame() {
  frame_ = inFrame_;
  inFrame_.clear();

  EthernetClass::loop();  // Allow the stack to move along

  if (frame_.size() > 0) {
    framePos_ = 0;
    return frame_.size();
  } else {
    framePos_ = -1;
    return 0;
  }
}

inline bool EthernetFrameClass::isAvailable() const {
  return (0 <= framePos_ && static_cast<size_t>(framePos_) < frame_.size());
}

int EthernetFrameClass::available() {
  if (!isAvailable()) {
    return 0;
  }
  return frame_.size() - framePos_;
}

int EthernetFrameClass::read() {
  if (!isAvailable()) {
    return -1;
  }
  return frame_[framePos_++];
}

int EthernetFrameClass::read(unsigned char *buffer, size_t len) {
  return read(reinterpret_cast<char *>(buffer), len);
}

int EthernetFrameClass::read(char *buffer, size_t len) {
  if (len == 0 || !isAvailable()) {
    return 0;
  }
  len = std::min(len, frame_.size() - framePos_);
  std::copy_n(&frame_.data()[framePos_], len, buffer);
  framePos_ += len;
  return len;
}

int EthernetFrameClass::peek() {
  if (!isAvailable()) {
    return -1;
  }
  return frame_[framePos_];
}

// --------------------------------------------------------------------------
//  Transmission
// --------------------------------------------------------------------------

void EthernetFrameClass::beginFrame() {
  if (outFrame_.capacity() < maxFrameLen()) {
    outFrame_.reserve(maxFrameLen());
  }

  hasOutFrame_ = true;
  outFrame_.clear();
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
  return endFrame(false);
}

bool EthernetFrameClass::endFrameWithTimestamp() {
  return endFrame(true);
}

bool EthernetFrameClass::endFrame(bool doTimestamp) {
  if (!hasOutFrame_) {
    return false;
  }
  hasOutFrame_ = false;

  bool retval = enet_output_frame(outFrame_.data(), outFrame_.size(),
                                  doTimestamp);
  outFrame_.clear();
  return retval;
}

bool EthernetFrameClass::send(const uint8_t *frame, size_t len) {
  return send(frame, len, false);
}

bool EthernetFrameClass::sendWithTimestamp(const uint8_t *frame, size_t len) {
  return send(frame, len, true);
}

bool EthernetFrameClass::send(const uint8_t *frame, size_t len,
                              bool doTimestamp) {
  return enet_output_frame(frame, len, doTimestamp);
}

bool EthernetFrameClass::ieee1588Timestamp(uint32_t *timestamp) const {
  // NOTE: This is not "concurrent safe"
  if (hasIEEE1588Timestamp_) {
    if (timestamp != nullptr) {
      *timestamp = ieee1588Timestamp_;
    }
    return true;
  }
  return false;
}

bool EthernetFrameClass::ieee1588TXTimestamp(uint32_t *timestamp) {
  return enet_read_1588_tx_timestamp(timestamp);
}

size_t EthernetFrameClass::write(uint8_t b) {
  if (!hasOutFrame_) {
    return 0;
  }
  // TODO: Limit vector size
  outFrame_.push_back(b);
  return 1;
}

size_t EthernetFrameClass::write(const uint8_t *buffer, size_t size) {
  if (!hasOutFrame_ || size == 0) {
    return 0;
  }
  if (size > UINT16_MAX) {
    size = UINT16_MAX;
  }
  // TODO: Limit vector size
  outFrame_.insert(outFrame_.end(), &buffer[0], &buffer[size]);
  return size;
}

}  // namespace network
}  // namespace qindesign
