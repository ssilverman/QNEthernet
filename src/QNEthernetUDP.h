// SPDX-FileCopyrightText: (c) 2021-2022 Shawn Silverman <shawn@pobox.com>
// SPDX-License-Identifier: MIT

// QNEthernetUDP.h defines the UDP interface.
// This file is part of the QNEthernet library.

#ifndef QNE_ETHERNETUDP_H_
#define QNE_ETHERNETUDP_H_

// C++ includes
#include <cstdint>
#include <vector>

#include <IPAddress.h>
#include <Udp.h>

#include "lwip/dns.h"
#include "lwip/ip_addr.h"
#include "lwip/opt.h"
#include "lwip/udp.h"

namespace qindesign {
namespace network {

class EthernetUDP final : public UDP {
 public:
  EthernetUDP();
  ~EthernetUDP();

  // Returns the maximum number of UDP sockets.
  static constexpr int maxSockets() {
    return MEMP_NUM_UDP_PCB;
  }

  // Starts listening on a port. This returns true if successful and false if
  // the port is in use. This calls begin(localPort, false).
  uint8_t begin(uint16_t localPort) override;

  // Starts listening on a port and set the SO_REUSEADDR socket option according
  // to the `reuse` parameter. This returns whether the attempt was successful.
  uint8_t begin(uint16_t localPort, bool reuse);

  // Multicast functions make use of Ethernet.joinGroup()
  uint8_t beginMulticast(IPAddress ip, uint16_t port) override;
  uint8_t beginMulticast(IPAddress ip, uint16_t port, bool reuse);

  void stop() override;

  // Sending UDP packets
  int beginPacket(IPAddress ip, uint16_t port) override;
  int beginPacket(const char *host, uint16_t port) override;
  int endPacket() override;
  int endPacketWithTimestamp();

  // Sends a UDP packet and returns whether the attempt was successful. This
  // combines the functions of beginPacket(), write(), and endPacket(), and
  // causes less overhead.
  bool send(const uint8_t *data, size_t len);

  // Same as `send(data, len)`, but adds a timestamp to the packet.
  bool sendWithTimestamp(const uint8_t *data, size_t len);

  // Bring Print::write functions into scope
  using Print::write;

  size_t write(uint8_t b) override;
  size_t write(const uint8_t *buffer, size_t size) override;

  // Receiving UDP packets
  int parsePacket() override;
  int available() override;
  int read() override;
  int read(unsigned char *buffer, size_t len) override;
  int read(char *buffer, size_t len) override;
  int peek() override;
  void flush() override;

  IPAddress remoteIP() override;
  uint16_t remotePort() override;

  // Gets the IEEE 1588 timestamp for the received packet and assigns to the
  // `timestamp` parameter, if available. This returns whether the received
  // packet has a timestamp.
  bool ieee1588Timestamp(uint32_t *timestamp) const;

  // Attempts to retrieve the last transmitted timestamp and returns whether one
  // is available and was assigned to `*timestamp`.
  static bool ieee1588TXTimestamp(uint32_t *timestamp);

 private:
  static void recvFunc(void *arg, struct udp_pcb *pcb, struct pbuf *p,
                       const ip_addr_t *addr, u16_t port);

  // Timestamping send functions
  int endPacket(bool doTimestamp);
  bool send(const uint8_t *data, size_t len, bool doTimestamp);

  // Checks if there's data still available in the packet.
  bool isAvailable() const;

  udp_pcb *pcb_;

  // Received packet; updated every time one is received
  std::vector<unsigned char> inPacket_;  // Holds received packets

  // Packet being processed by the caller
  std::vector<unsigned char> packet_;    // Holds the packet being read
  int packetPos_;                        // -1 if not currently reading a packet

  // Source of incoming packet
  IPAddress inAddr_;
  volatile uint16_t inPort_;

  // IEEE 1588
  bool inHasIEEE1588Timestamp_ = false;  // Attached to inPacket_
  uint32_t inIEEE1588Timestamp_;         // Attached to inPacket_
  bool hasIEEE1588Timestamp_ = false;
  uint32_t ieee1588Timestamp_;

  // Outgoing packets
  bool hasOutPacket_;
  ip_addr_t outIpaddr_;
  uint16_t outPort_;
  std::vector<unsigned char> outPacket_;
};

}  // namespace network
}  // namespace qindesign

#endif  // #ifndef QNE_ETHERNETUDP_H_
