// SPDX-FileCopyrightText: (c) 2022 Shawn Silverman <shawn@pobox.com>
// SPDX-License-Identifier: MIT

// QNEthernetFrame.cpp contains an EthernetFrame implementation.
// This file is part of the QNEthernet library.

#include "QNEthernetFrame.h"

// C++ includes
#include <algorithm>

#include "QNEthernet.h"

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

bool EthernetFrameClass::endFrame() {
  if (!hasOutFrame_) {
    return false;
  }
  hasOutFrame_ = false;

  bool retval = enet_output_frame(outFrame_.data(), outFrame_.size());
  outFrame_.clear();
  return retval;
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
