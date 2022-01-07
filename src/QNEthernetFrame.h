// SPDX-FileCopyrightText: (c) 2022 Shawn Silverman <shawn@pobox.com>
// SPDX-License-Identifier: MIT

// QNEthernetFrame.h defines the raw frame interface.
// This file is part of the QNEthernet library.

#ifndef QNE_ETHERNETFRAME_H_
#define QNE_ETHERNETFRAME_H_

// C++ includes
#include <cstdint>
#include <vector>

#include <IPAddress.h>
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

// Provides an API for raw Ethernet frames, similar to the UDP API.
class EthernetFrameClass final : public Stream {
 public:
  // Accesses the singleton instance.
  static EthernetFrameClass &instance() {
    return instance_;
  }

  // EthernetFrameClass is neither copyable nor movable.
  EthernetFrameClass(const EthernetFrameClass &) = delete;
  EthernetFrameClass &operator=(const EthernetFrameClass &) = delete;

  // Returns the maximum frame length. This includes any padding and the FCS
  // (Frame Check Sequence, the CRC value).
  static constexpr int maxFrameLen() {
    return MAX_FRAME_LEN;
  }

  // Starts a fresh frame. This is similar to EthernetUDP::beginPacket().
  void beginFrame();

  // Sends the frame. This is similar to EthernetUDP::endPacket().
  //
  // The FCS (Frame Check Sequence, the CRC value) should not be included in the
  // frame data.
  //
  // This will return false if:
  // 1. The frame was not started, or
  // 2. The length is not in the range 60-1518. This excludes the FCS field.
  bool endFrame();

  // Bring Print::write functions into scope
  using Print::write;

  size_t write(uint8_t b) override;
  size_t write(const uint8_t *buffer, size_t size) override;

  // Receiving UDP packets
  int parseFrame();
  int available() override;
  int read() override;
  int read(unsigned char *buffer, size_t len);
  int read(char *buffer, size_t len);
  int peek() override;
  void flush() override {}

 private:
  EthernetFrameClass() = default;
  ~EthernetFrameClass() = default;

  static err_t recvFunc(struct pbuf *p, struct netif *netif);

  // Checks if there's data still available in the packet.
  bool isAvailable() const;

  std::vector<unsigned char> inFrame_;  // Holds received frames
  std::vector<unsigned char> frame_;    // Holds the frame being read
  int framePos_ = -1;                   // -1 if not currently reading a frame

  // Outgoing packets
  bool hasOutFrame_ = false;
  std::vector<unsigned char> outFrame_;

  // The singleton instance.
  static EthernetFrameClass instance_;

  friend err_t ::unknown_eth_protocol(struct pbuf *p, struct netif *netif);
};

}  // namespace network
}  // namespace qindesign

#endif  // #ifndef QNE_ETHERNETFRAME_H_
