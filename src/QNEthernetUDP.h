// SPDX-FileCopyrightText: (c) 2021 Shawn Silverman <shawn@pobox.com>
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
#include <WString.h>
#include <lwip/dns.h>
#include <lwip/ip_addr.h>
#include <lwip/udp.h>

namespace qindesign {
namespace network {

class EthernetUDP final : public UDP {
 public:
  EthernetUDP();
  ~EthernetUDP();

  // Start listening on a port. This returns true if successful and false if the
  // port is in use. This calls begin(localPort, false).
  uint8_t begin(uint16_t localPort) override;

  // Start listening on a port and set the SO_REUSEADDR socket option according
  // to the `reuse` parameter. This returns whether the attempt was successful.
  uint8_t begin(uint16_t localPort, bool reuse);

  uint8_t beginMulticast(IPAddress ip, uint16_t port) override;
  void stop() override;

  // Sending UDP packets
  int beginPacket(IPAddress ip, uint16_t port) override;
  int beginPacket(const char *host, uint16_t port) override;
  int endPacket() override;

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

 private:
  static void dnsFoundFunc(const char *name, const ip_addr_t *ipaddr,
                           void *callback_arg);
  static void recvFunc(void *arg, struct udp_pcb *pcb, struct pbuf *p,
                       const ip_addr_t *addr, u16_t port);

  // Check if there's data still available in the packet.
  bool isAvailable() const;

  udp_pcb *pcb_;

  std::vector<unsigned char> inPacket_;  // Holds received packets
  std::vector<unsigned char> packet_;    // Holds the packet being read
  int packetPos_;                        // -1 if not currently reading a packet

  // Source of incoming packet
  IPAddress inAddr_;
  volatile uint16_t inPort_;

  // Outgoing packets
  bool hasOutPacket_;
  ip_addr_t outIpaddr_;
  uint16_t outPort_;
  std::vector<unsigned char> outPacket_;

  // DNS lookups
  String lookupHost_;   // For matching DNS lookups
  IPAddress lookupIP_;  // Set by a DNS lookup
  volatile bool lookupFound_;
};

}  // namespace network
}  // namespace qindesign

#endif  // #ifndef QNE_ETHERNETUDP_H_
