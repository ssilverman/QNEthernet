// SPDX-FileCopyrightText: (c) 2022-2025 Shawn Silverman <shawn@pobox.com>
// SPDX-License-Identifier: AGPL-3.0-or-later

// QNEthernetFrame.h defines the raw frame interface.
// This file is part of the QNEthernet library.

#pragma once

#include "qnethernet_opts.h"

#if QNETHERNET_ENABLE_RAW_FRAME_SUPPORT

// C++ includes
#include <cstddef>
#include <cstdint>
#include <vector>

#include <Stream.h>

#include "lwip/err.h"
#include "lwip/netif.h"
#include "lwip/pbuf.h"
#include "lwip/prot/ethernet.h"
#include "lwip_driver.h"
#include "qnethernet/StaticInit.h"
#include "qnethernet/internal/PrintfChecked.h"
#include "qnethernet/internal/optional.h"

extern "C" {
err_t unknown_eth_protocol(struct pbuf* p, struct netif* netif);
}  // extern "C"

namespace qindesign {
namespace network {

// Provides an API for unknown raw Ethernet frames, similar to the UDP API.
//
// The following known Ethernet frame types won't be received by this API:
// 1. IPv4 (0x0800)
// 2. ARP  (0x0806)
// 3. IPv6 (0x86DD) (if enabled)
class EthernetFrameClass final : public Stream, public internal::PrintfChecked {
 public:
  // Returns the maximum frame length. This includes any padding and the 4-byte
  // FCS (Frame Check Sequence, the CRC value). Subtract 4 to exclude the FCS.
  //
  // Note that this size includes VLAN frames, which are 4 bytes larger.
  // Also note that the padding does not need to be managed by the caller.
  static constexpr int maxFrameLen() {
    return MAX_FRAME_LEN;
  }

  // Returns the minimum frame length. This includes any padding and the 4-byte
  // FCS (Frame Check Sequence, the CRC value). Subtract 4 to exclude the FCS.
  //
  // Note that the padding does not need to be managed by the caller, meaning
  // frames smaller than this size are allowed; the system will insert padding
  // as needed.
  static constexpr int minFrameLen() {
    return 64;
  }

  // Starts a fresh frame. This is similar to EthernetUDP::beginPacket().
  void beginFrame();

  // Starts a frame and writes the given addresses and EtherType/length. This is
  // similar to EthernetUDP::beginPacket().
  void beginFrame(const uint8_t dstAddr[ETH_HWADDR_LEN],
                  const uint8_t srcAddr[ETH_HWADDR_LEN],
                  uint16_t typeOrLen);

  // Starts a VLAN-tagged frame and writes the given addresses, VLAN info, and
  // EtherType/length. This is similar to EthernetUDP::beginPacket().
  void beginVLANFrame(const uint8_t dstAddr[ETH_HWADDR_LEN],
                      const uint8_t srcAddr[ETH_HWADDR_LEN],
                      uint16_t vlanInfo, uint16_t typeOrLen);

  // Sends the frame. This is similar to EthernetUDP::endPacket(). This clears
  // the accumulated data regardless of what is returned.
  //
  // The 4-byte FCS (Frame Check Sequence, the CRC value) should not be included
  // in the user-provided frame data.
  //
  // This will return false if:
  // 1. The frame was not started,
  // 2. Ethernet was not started,
  // 3. The length is not in the range 14-(maxFrameLen()-8) for non-VLAN frames
  //    or 18-(maxFrameLen()-4) for VLAN frames (excludes the FCS), or
  // 4. There's no room in the output buffers.
  bool endFrame();

  // Sends a frame and returns whether the send was successful. This causes less
  // overhead than beginFrame()/write()/endFrame().
  //
  // The 4-byte FCS (Frame Check Sequence, the CRC value) should not be included
  // in the user-provided frame data.
  //
  // This will return false if:
  // 1. Ethernet was not started,
  // 2. The frame is NULL,
  // 3. The length is not in the range 14-(maxFrameLen()-8) for non-VLAN frames
  //    or 18-(maxFrameLen()-4) for VLAN frames (excludes the FCS), or
  // 4. There's no room in the output buffers.
  bool send(const void* frame, size_t len) const;

  // Use the one from here instead of the one from Print
  using internal::PrintfChecked::printf;

  // Bring Print::write functions into scope
  using Print::write;

  // The write functions limit the allowed size to Ethernet.maxFrameLen()-4
  size_t write(uint8_t b) override;
  size_t write(const uint8_t* buffer, size_t size) override;

  // Receiving frames
  int parseFrame();
  int available() override;
  int read() override;

  // A NULL buffer allows the caller to skip bytes without having to read into
  // a buffer.
  int read(void* buffer, size_t len);

  int peek() override;
  void flush() override {}

  // Returns max{(Ethernet.maxFrameLen() - 4) - "written", 0}.
  int availableForWrite() override;

  // Returns the total size of the received packet data. This is only valid if a
  // frame has been received with parseFrame() or has been created.
  size_t size() const {
    return frame_.data.size();
  }

  // Returns a pointer to the received frame data. This is only valid if a frame
  // has been received with parseFrame() or has been created. This may return
  // NULL if the size is zero.
  const uint8_t* data() const {
    return frame_.data.data();
  }

  // Returns the approximate frame arrival time, measured with sys_now(). This
  // is only valid if a frame has been received with parseFrame().
  //
  // This is useful in the case where frames have been queued and the caller
  // needs the approximate arrival time. Frames are timestamped when the unknown
  // ethernet protocol receive callback is called.
  uint32_t receivedTimestamp() const {
    return frame_.receivedTimestamp;
  }

  // Returns a pointer to the destination MAC. This is only valid if a frame has
  // been received with parseFrame() or has had this value written.
  //
  // This returns data().
  const uint8_t* destinationMAC() const;

  // Returns a pointer to the source MAC. This is only valid if a frame has been
  // received with parseFrame() or has had this value written.
  //
  // This returns data() + 6.
  const uint8_t* sourceMAC() const;

  // Returns the EtherType/length value immediately following the source MAC.
  // Note that VLAN frames are handled specially. This is only valid if a frame
  // has been received with parseFrame() or has had this value written.
  //
  // This returns the two bytes starting at data() + 12, or zero if there aren't
  // at least 14 bytes.
  uint16_t etherTypeOrLength() const;

  // Returns a pointer to the payload immediately following the
  // EthernetType/length field. Note that VLAN frames are handled specially.
  // This is only valid if a frame has been received with parseFrame() or has
  // had at least 14 bytes written.
  //
  // This returns data() + 14.
  const uint8_t* payload() const;

  // Sets the receive queue capacity. This will use a minimum of 1.
  //
  // If the new capacity is smaller than the number of elements in the queue
  // then all the oldest frames that don't fit are dropped.
  //
  // This disables interrupts while changing the queue so as not to interfere
  // with the receive function if called from an ISR.
  void setReceiveQueueCapacity(size_t capacity);

  // Returns the receive queue capacity.
  size_t receiveQueueCapacity() const {
    return inBuf_.size();
  }

  // Returns the number of frames currently in the receive queue.
  size_t receiveQueueSize() const {
    return inBufSize_;
  }

  // Returns the total number of dropped received frames since reception was
  // started. Note that this is the count of dropped frames at the layer above
  // the driver.
  uint32_t droppedReceiveCount() const {
    return droppedReceiveCount_;
  }

  // Returns the total number of received frames, including dropped frames,
  // since reception was started. Note that this is the count at the layer above
  // the driver.
  uint32_t totalReceiveCount() const {
    return totalReceiveCount_;
  }

  // Clears any outgoing packet and the incoming queue.
  void clear();

 private:
  struct Frame final {
    std::vector<uint8_t> data;
    volatile uint32_t receivedTimestamp = 0;  // Approximate arrival time

    // Clears all the data.
    void clear();
  };

  EthernetFrameClass();
  ~EthernetFrameClass() = default;

  // EthernetFrameClass is neither copyable nor movable
  EthernetFrameClass(const EthernetFrameClass&) = delete;
  EthernetFrameClass& operator=(const EthernetFrameClass&) = delete;

  static err_t recvFunc(struct pbuf* p, struct netif* netif);

  // Checks if there's data still available in the packet.
  bool isAvailable() const;

  // Received frame; updated every time one is received
  std::vector<Frame> inBuf_;  // Holds received frames
  size_t inBufTail_ = 0;
  size_t inBufHead_ = 0;
  size_t inBufSize_ = 0;

  // Frame being processed by the caller
  Frame frame_;   // Holds the frame being read
  int framePos_ = -1;  // -1 if not currently reading a frame

  // Outgoing frames
  internal::optional<Frame> outFrame_;

  // Stats
  uint32_t droppedReceiveCount_ = 0;
  uint32_t totalReceiveCount_   = 0;

  friend class StaticInit<EthernetFrameClass>;
  friend err_t ::unknown_eth_protocol(struct pbuf* p, struct netif* netif);
};

// Instance for using raw Ethernet frames.
STATIC_INIT_DECL(EthernetFrameClass, EthernetFrame);

}  // namespace network
}  // namespace qindesign

#endif  // QNETHERNET_ENABLE_RAW_FRAME_SUPPORT
