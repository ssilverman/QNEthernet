// SPDX-FileCopyrightText: (c) 2022-2023 Shawn Silverman <shawn@pobox.com>
// SPDX-License-Identifier: MIT

// QNEthernetFrame.h defines the raw frame interface.
// This file is part of the QNEthernet library.

#ifndef QNETHERNET_ETHERNETFRAME_H_
#define QNETHERNET_ETHERNETFRAME_H_

#ifdef QNETHERNET_ENABLE_RAW_FRAME_SUPPORT

// C++ includes
#include <cstddef>
#include <cstdint>
#include <vector>

#include <Stream.h>

#include "lwip/err.h"
#include "lwip/netif.h"
#include "lwip/pbuf.h"
#include "lwip_driver.h"

extern "C" {
err_t unknown_eth_protocol(struct pbuf *p, struct netif *netif);
}  // extern "C"

namespace qindesign {
namespace network {

// Provides an API for unknown raw Ethernet frames, similar to the UDP API.
//
// The following known Ethernet frame types won't be received by this API:
// 1. IPv4 (0x0800)
// 2. ARP  (0x0806)
// 3. IPv6 (0x86DD) (if enabled)
class EthernetFrameClass final : public Stream {
 public:
  // Accesses the singleton instance.
  static EthernetFrameClass &instance();

  // EthernetFrameClass is neither copyable nor movable
  EthernetFrameClass(const EthernetFrameClass &) = delete;
  EthernetFrameClass &operator=(const EthernetFrameClass &) = delete;

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
  void beginFrame(const uint8_t dstAddr[6], const uint8_t srcAddr[6],
                  uint16_t typeOrLen);

  // Starts a VLAN-tagged frame and writes the given addresses, VLAN info, and
  // EtherType/length. This is similar to EthernetUDP::beginPacket().
  void beginVLANFrame(const uint8_t dstAddr[6], const uint8_t srcAddr[6],
                      uint16_t vlanInfo, uint16_t typeOrLen);

  // Sends the frame. This is similar to EthernetUDP::endPacket(). This clears
  // the accumulated data regardless of what is returned.
  //
  // The 4-byte FCS (Frame Check Sequence, the CRC value) should not be included
  // in the user-provided frame data.
  //
  // This will return false if:
  // 1. The frame was not started,
  // 2. Ethernet was not started, or
  // 3. The length is not in the range 14-(maxFrameLen()-8) for non-VLAN frames
  //    or 18-(maxFrameLen()-4) for VLAN frames (excludes the FCS).
  bool endFrame();

  // Sends a frame and returns whether the send was successful. This causes less
  // overhead than beginFrame()/write()/endFrame().
  //
  // The 4-byte FCS (Frame Check Sequence, the CRC value) should not be included
  // in the user-provided frame data.
  //
  // This will return false if:
  // 1. Ethernet was not started,
  // 2. The frame is NULL, or
  // 3. The length is not in the range 14-(maxFrameLen()-8) for non-VLAN frames
  //    or 18-(maxFrameLen()-4) for VLAN frames (excludes the FCS).
  bool send(const uint8_t *frame, size_t len) const;

  // Bring Print::write functions into scope
  using Print::write;

  // The write functions limit the allowed size to Ethernet.maxFrameLen()-4
  size_t write(uint8_t b) override;
  size_t write(const uint8_t *buffer, size_t size) override;

  // Receiving frames
  int parseFrame();
  int available() override;
  int read() override;
  int read(uint8_t *buffer, size_t len);

  // A NULL buffer allows the caller to skip bytes without having to read into
  // a buffer.
  int read(char *buffer, size_t len);

  int peek() override;
  void flush() override {}

  // Returns max{(Ethernet.maxFrameLen() - 4) - "written", 0}.
  int availableForWrite() override;

  // Returns the total size of the received packet data. This is only valid if a
  // frame has been received with parseFrame().
  size_t size() const;

  // Returns a pointer to the received frame data. This is only valid if a frame
  // has been received with parseFrame().
  const uint8_t *data() const;

  // Returns the approximate frame arrival time, measured with millis(). This
  // is only valid if a frame has been received with parseFrame().
  //
  // This is useful in the case where frames have been queued and the caller
  // needs the approximate arrival time. Frames are timestamped when the unknown
  // ethernet protocol receive callback is called.
  uint32_t receivedTimestamp() const;

  // Sets the receive queue size. This will use a minimum of 1.
  //
  // If the new size is smaller than the number of elements in the queue then
  // all the oldest frames that don't fit are dropped.
  //
  // This disables interrupts while changing the queue so as not to interfere
  // with the receive function if called from an ISR.
  void setReceiveQueueSize(size_t size);

  // Returns the current receive queue size.
  size_t receiveQueueSize() const {
    return inBuf_.size();
  }

 private:
  struct Frame final {
    std::vector<uint8_t> data;
    volatile uint32_t receivedTimestamp = 0;  // Approximate arrival time

    // Clears all the data.
    void clear();
  };

  EthernetFrameClass();
  ~EthernetFrameClass() = default;

  static err_t recvFunc(struct pbuf *p, struct netif *netif);

  // Checks if there's data still available in the packet.
  bool isAvailable() const;

  // Received frame; updated every time one is received
  std::vector<Frame> inBuf_;  // Holds received frames
  size_t inBufTail_ = 0;
  size_t inBufHead_ = 0;
  size_t inBufSize_ = 0;

  // Frame being processed by the caller
  Frame frame_;        // Holds the frame being read
  int framePos_ = -1;  // -1 if not currently reading a frame

  // Outgoing frames
  bool hasOutFrame_ = false;
  Frame outFrame_;

  friend err_t ::unknown_eth_protocol(struct pbuf *p, struct netif *netif);
};

}  // namespace network
}  // namespace qindesign

#endif  // QNETHERNET_ENABLE_RAW_FRAME_SUPPORT

#endif  // QNETHERNET_ETHERNETFRAME_H_
