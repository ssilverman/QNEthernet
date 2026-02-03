// SPDX-FileCopyrightText: (c) 2022-2026 Shawn Silverman <shawn@pobox.com>
// SPDX-License-Identifier: AGPL-3.0-or-later

// QNEthernetFrame.cpp contains an EthernetFrame implementation.
// This file is part of the QNEthernet library.

#include "qnethernet/QNEthernetFrame.h"

#if QNETHERNET_ENABLE_RAW_FRAME_SUPPORT

// C++ includes
#include <algorithm>
#include <utility>

#include "QNEthernet.h"
#include "lwip/arch.h"
#include "lwip/debug.h"
#include "lwip/prot/ieee.h"
#include "lwip/sys.h"
#include "qnethernet/platforms/pgmspace.h"

extern "C" {
void qnethernet_hal_disable_interrupts(void);
void qnethernet_hal_enable_interrupts(void);
}  // extern "C"

extern "C" {
err_t unknown_eth_protocol(struct pbuf* const p, struct netif* const netif) {
#if ETH_PAD_SIZE
  LWIP_ASSERT("Expected removed ETH_PAD_SIZE header",
              pbuf_remove_header(p, ETH_PAD_SIZE) == 0);
#endif  // ETH_PAD_SIZE

  return qindesign::network::EthernetFrameClass::recvFunc(p, netif);
}
}  // extern "C"

namespace qindesign {
namespace network {

// A reference to the singleton.
STATIC_INIT_DEFN(EthernetFrameClass, EthernetFrame);

err_t EthernetFrameClass::recvFunc(struct pbuf* const p,
                                   struct netif* const netif) {
  LWIP_UNUSED_ARG(netif);

  const uint32_t timestamp = sys_now();

  struct pbuf* pNext = p;

  // Push
  if (EthernetFrame.inBuf_.full()) {
    ++EthernetFrame.droppedReceiveCount_;
  }
  Frame& frame = EthernetFrame.inBuf_.put();
  frame.data.clear();
  if (p->tot_len > 0) {
    frame.data.reserve(p->tot_len);
    // TODO: Limit vector size
    while (pNext != nullptr) {
      const auto data = static_cast<const uint8_t*>(pNext->payload);
      (void)frame.data.insert(frame.data.cend(), &data[0], &data[pNext->len]);
      pNext = pNext->next;
    }
  }
  frame.receivedTimestamp = timestamp;

  (void)pbuf_free(p);
  ++EthernetFrame.totalReceiveCount_;

  return ERR_OK;
}

FLASHMEM EthernetFrameClass::EthernetFrameClass()
    : inBuf_(1) {}

void EthernetFrameClass::Frame::clear() {
  data.clear();
  receivedTimestamp = 0;
}

void EthernetFrameClass::clear() {
  // Outgoing
  if (outFrame_.has_value) {
    outFrame_.has_value = false;
    outFrame_.value.clear();
  }

  // Incoming
  for (size_t i = 0; i < inBuf_.size(); ++i) {
    inBuf_[i].clear();
  }
  inBuf_.clear();
}

// --------------------------------------------------------------------------
//  Reception
// --------------------------------------------------------------------------

int EthernetFrameClass::parseFrame() {
  if (inBuf_.empty()) {
    framePos_ = -1;
    return -1;
  }

  // Pop
  frame_ = std::move(inBuf_.get());

  Ethernet.loop();  // Allow the stack to move along

  if (frame_.data.size() > 0) {
    framePos_ = 0;
    return static_cast<int>(frame_.data.size());
  } else {
    framePos_ = -1;
    return 0;
  }
}

inline bool EthernetFrameClass::isAvailable() const {
  return ((0 <= framePos_) &&
          (static_cast<size_t>(framePos_) < frame_.data.size()));
}

int EthernetFrameClass::available() {
  if (!isAvailable()) {
    return 0;
  }
  return static_cast<int>(frame_.data.size() - framePos_);
}

int EthernetFrameClass::read() {
  if (!isAvailable()) {
    return -1;
  }
  return frame_.data[framePos_++];
}

int EthernetFrameClass::read(void* const buffer, const size_t len) {
  if ((len == 0) || !isAvailable()) {
    return 0;
  }
  const size_t actualLen = std::min(len, frame_.data.size() - framePos_);
  if (buffer != nullptr) {
    (void)std::copy_n(&frame_.data.data()[framePos_], actualLen,
                      static_cast<uint8_t*>(buffer));
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

const uint8_t* EthernetFrameClass::destinationMAC() const {
  return data();
}

const uint8_t* EthernetFrameClass::sourceMAC() const {
  return data() + 6;
}

uint16_t EthernetFrameClass::etherTypeOrLength() const {
  if (size() < 14) {
    return 0;
  }
  const uint8_t* const p = data();
  return static_cast<uint16_t>((uint16_t{p[12]} << 8) | uint16_t{p[13]});
}

const uint8_t* EthernetFrameClass::payload() const {
  return data() + 14;
}

void EthernetFrameClass::setReceiveQueueCapacity(const size_t capacity) {
  qnethernet_hal_disable_interrupts();
  inBuf_.setCapacity(capacity);
  qnethernet_hal_enable_interrupts();
}

// --------------------------------------------------------------------------
//  Transmission
// --------------------------------------------------------------------------

void EthernetFrameClass::beginFrame() {
  // Don't reserve memory because that might exhaust the heap
  // if (outFrame_.value.data.capacity() < maxFrameLen()) {
  //   outFrame_.value.data.reserve(maxFrameLen());
  // }
  // outFrame_.has_value = true;

  outFrame_.value.data.clear();
  outFrame_.has_value = true;
}

void EthernetFrameClass::beginFrame(const uint8_t dstAddr[ETH_HWADDR_LEN],
                                    const uint8_t srcAddr[ETH_HWADDR_LEN],
                                    const uint16_t typeOrLength) {
  beginFrame();
  LWIP_ASSERT(
      "Expected write success",
      (write(dstAddr, ETH_HWADDR_LEN) +
       write(srcAddr, ETH_HWADDR_LEN) +
       write(static_cast<uint8_t>(typeOrLength >> 8)) +
       write(static_cast<uint8_t>(typeOrLength))) == (2*ETH_HWADDR_LEN + 2));
}

void EthernetFrameClass::beginVLANFrame(const uint8_t dstAddr[ETH_HWADDR_LEN],
                                        const uint8_t srcAddr[ETH_HWADDR_LEN],
                                        const uint16_t vlanInfo,
                                        const uint16_t typeOrLength) {
  beginFrame(dstAddr, srcAddr, ETHTYPE_VLAN);
  LWIP_ASSERT("Expected write success",
              (write(static_cast<uint8_t>(vlanInfo >> 8)) +
               write(static_cast<uint8_t>(vlanInfo)) +
               write(static_cast<uint8_t>(typeOrLength >> 8)) +
               write(static_cast<uint8_t>(typeOrLength))) == 4);
}

bool EthernetFrameClass::endFrame() {
  if (!outFrame_.has_value) {
    return false;
  }

  const bool retval = enet_output_frame(outFrame_.value.data.data(),
                                        outFrame_.value.data.size());
  outFrame_.has_value = false;
  outFrame_.value.clear();
  return retval;
}

bool EthernetFrameClass::send(const void* const frame, const size_t len) const {
  return enet_output_frame(frame, len);
}

size_t EthernetFrameClass::write(const uint8_t b) {
  if (!outFrame_.has_value || (availableForWrite() <= 0)) {
    return 0;
  }
  outFrame_.value.data.push_back(b);
  return 1;
}

size_t EthernetFrameClass::write(const uint8_t* const buffer,
                                 const size_t size) {
  const int avail = availableForWrite();
  if (!outFrame_.has_value || (size == 0) || (avail <= 0)) {
    return 0;
  }

  const size_t actualSize = std::min(size, static_cast<size_t>(avail));
  (void)outFrame_.value.data.insert(outFrame_.value.data.cend(), &buffer[0],
                                    &buffer[actualSize]);
  return actualSize;
}

int EthernetFrameClass::availableForWrite() {
  if (!outFrame_.has_value) {
    return 0;
  }
  if (outFrame_.value.data.size() > (maxFrameLen() - 4)) {
    return 0;
  }
  return static_cast<int>((maxFrameLen() - 4) - outFrame_.value.data.size());
}

}  // namespace network
}  // namespace qindesign

#endif  // QNETHERNET_ENABLE_RAW_FRAME_SUPPORT
