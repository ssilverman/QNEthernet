// SPDX-FileCopyrightText: (c) 2022 Shawn Silverman <shawn@pobox.com>
// SPDX-License-Identifier: MIT

// QNEthernetFrame.h defines the raw frame interface.
// This file is part of the QNEthernet library.

#ifndef QNE_ETHERNETFRAME_H_
#define QNE_ETHERNETFRAME_H_

// C++ includes
#include <cstdint>
#include <vector>

#include <Stream.h>

#include "lwip/err.h"
#include "lwip/netif.h"
#include "lwip/pbuf.h"
#include "lwip_t41.h"

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
  static EthernetFrameClass &instance() {
    return instance_;
  }

  // EthernetFrameClass is neither copyable nor movable
  EthernetFrameClass(const EthernetFrameClass &) = delete;
  EthernetFrameClass &operator=(const EthernetFrameClass &) = delete;

  // Returns the maximum frame length. This includes any padding and the FCS
  // (Frame Check Sequence, the CRC value). Subtract 4 to exclude the FCS.
  static constexpr int maxFrameLen() {
    return MAX_FRAME_LEN;
  }

  // Returns the minimum frame length. This includes any padding and the FCS
  // (Frame Check Sequence, the CRC value). Subtract 4 to exclude the FCS.
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

  // Sends the frame. This is similar to EthernetUDP::endPacket().
  //
  // The FCS (Frame Check Sequence, the CRC value) should not be included in the
  // frame data.
  //
  // This will return false if:
  // 1. The frame was not started,
  // 2. Ethernet was not started, or
  // 3. The length is not in the range 60-(maxFrameLen()-4) (excludes the FCS).
  bool endFrame();

  // Sends a frame and returns whether the send was successful. This causes less
  // overhead than beginFrame()/write()/endFrame().
  //
  // The FCS (Frame Check Sequence, the CRC value) should not be included in the
  // frame data.
  //
  // This will return false if:
  // 1. Ethernet was not started,
  // 2. The frame is NULL, or
  // 3. The length is not in the range 60-(maxFrameLen()-4) (excludes the FCS).
  bool send(const uint8_t *frame, size_t len) const;

  // Bring Print::write functions into scope
  using Print::write;

  // The write functions limit the allowed size to Ethernet.maxFrameLen()-4
  size_t write(uint8_t b) override;
  size_t write(const uint8_t *buffer, size_t size) override;

  // Receiving UDP packets
  int parseFrame();
  int available() override;
  int read() override;
  int read(unsigned char *buffer, size_t len);

  // A NULL buffer allows the caller to skip bytes without having to read into
  // a buffer.
  int read(char *buffer, size_t len);

  int peek() override;
  void flush() override {}

  // Returns max{(Ethernet.maxFrameLen() - 4) - "written", 0}.
  int availableForWrite() override;

  // Returns a pointer to the received frame data.
  const unsigned char *data() const;

 private:
  EthernetFrameClass() = default;
  ~EthernetFrameClass() = default;

  static err_t recvFunc(struct pbuf *p, struct netif *netif);

  // Checks if there's data still available in the packet.
  bool isAvailable() const;

  // Received frame; updated every time one is received
  std::vector<unsigned char> inFrame_;  // Holds received frames

  // Frame being processed by the caller
  std::vector<unsigned char> frame_;    // Holds the frame being read
  int framePos_ = -1;                   // -1 if not currently reading a frame

  // Outgoing frames
  bool hasOutFrame_ = false;
  std::vector<unsigned char> outFrame_;

  // The singleton instance.
  static EthernetFrameClass instance_;

  friend err_t ::unknown_eth_protocol(struct pbuf *p, struct netif *netif);
};

}  // namespace network
}  // namespace qindesign

#endif  // QNE_ETHERNETFRAME_H_
