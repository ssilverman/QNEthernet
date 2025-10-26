// SPDX-FileCopyrightText: (c) 2021-2025 Shawn Silverman <shawn@pobox.com>
// SPDX-License-Identifier: AGPL-3.0-or-later

// QNEthernetUDP.h defines the UDP interface.
// This file is part of the QNEthernet library.

#pragma once

#include "lwip/opt.h"

#if LWIP_UDP

// C++ includes
#include <cstddef>
#include <cstdint>
#include <vector>

#include <IPAddress.h>
#include <Udp.h>

#include "lwip/arch.h"
#include "lwip/ip_addr.h"
#include "lwip/pbuf.h"
#include "lwip/udp.h"
#include "qnethernet/internal/IPOpts.h"
#include "qnethernet/internal/PrintfChecked.h"
#include "qnethernet/internal/optional.h"

namespace qindesign {
namespace network {

class EthernetUDP : public UDP,
                    public internal::IPOpts,
                    public internal::PrintfChecked {
 public:
  EthernetUDP();

  // Creates a new UDP socket with the given receive queue capacity. It will be
  // set to a minimum of 1.
  explicit EthernetUDP(size_t capacity);

  // Disallow copying but allow moving
  EthernetUDP(const EthernetUDP&) = delete;
  EthernetUDP& operator=(const EthernetUDP&) = delete;
  EthernetUDP(EthernetUDP&&) = default;
  EthernetUDP& operator=(EthernetUDP&&) = default;

  virtual ~EthernetUDP();

  // Returns the maximum number of UDP sockets.
  static constexpr int maxSockets() {
    return MEMP_NUM_UDP_PCB;
  }

  // Returns the receive queue capacity.
  size_t receiveQueueCapacity() const {
    return inBuf_.size();
  }

  // Returns the number of packets currently in the receive queue.
  size_t receiveQueueSize() const {
    return inBufSize_;
  }

  // Changes the receive queue capacity. This will use a minimum of 1.
  //
  // If the new capacity is smaller than the number of elements in the queue
  // then all the oldest packets that don't fit are dropped.
  void setReceiveQueueCapacity(size_t capacity);

  // Returns the total number of dropped received packets since reception was
  // started. Note that this is the count of dropped packets at the layer above
  // the driver.
  uint32_t droppedReceiveCount() const {
    return droppedReceiveCount_;
  }

  // Returns the total number of received packets, including dropped packets,
  // since reception was started. Note that this is the count at the layer above
  // the driver.
  uint32_t totalReceiveCount() const {
    return totalReceiveCount_;
  }

  // Starts listening on a port. This returns true if successful and false if
  // the port is in use.
  //
  // This first calls stop() if the socket is already listening and the port or
  // _reuse_ socket option differ.
  //
  // If this returns false and there was an error then errno will be set.
  uint8_t begin(uint16_t localPort) final;  // Wish: Boolean return

  // Starts listening on a port and sets the SO_REUSEADDR socket option. This
  // returns whether the attempt was successful.
  //
  // This first calls stop() if the socket is already listening and the port or
  // _reuse_ socket option differ.
  //
  // If this returns false and there was an error then errno will be set.
  bool beginWithReuse(uint16_t localPort);

  // Multicast functions make use of Ethernet.joinGroup(). These first call the
  // appropriate `begin` functions.
  //
  // These return true if successful and false otherwise.
  //
  // If these return false and there was an error then errno will be set.
  uint8_t beginMulticast(IPAddress ip, uint16_t port) final;  // Wish: Boolean return
  bool beginMulticastWithReuse(const IPAddress& ip, uint16_t port);

  // Returns the port to which this socket is bound, or zero if it is not bound.
  uint16_t localPort() const;

  // If there was an error leaving the multicast group joined when starting to
  // listen on a multicast address then errno will be set.
  void stop() final;

  // Sending UDP packets
  // These really return Booleans
  // Wish: Boolean returns
  //
  // If these return false and there was an error then errno will be set.
  int beginPacket(IPAddress ip, uint16_t port) final;
  int beginPacket(const char* host, uint16_t port) final;
  int endPacket() final;  // Always clears accumulated data

  // Sends a UDP packet and returns whether the attempt was successful. This
  // combines the functions of beginPacket(), write(), and endPacket(), and
  // causes less overhead.
  //
  // If this returns false and there was an error then errno will be set.
  bool send(const IPAddress& ip, uint16_t port, const void* data, size_t len);

  // Calls the other send() function after performing a DNS lookup.
  //
  // If this returns false and there was an error then errno will be set.
  bool send(const char* host, uint16_t port, const void* data, size_t len);

  // Use the one from here instead of the one from Print
  using internal::PrintfChecked::printf;

  // Bring Print::write functions into scope
  using Print::write;

  size_t write(uint8_t b) final;
  size_t write(const uint8_t* buffer, size_t size) final;
  int availableForWrite() final;

  // Receiving UDP packets
  int parsePacket() final;
  int available() final;
  int read() final;

  // A NULL buffer allows the caller to skip bytes without having to read into
  // a buffer.
  int read(uint8_t* buffer, size_t len) final;

  // A NULL buffer allows the caller to skip bytes without having to read into
  // a buffer.
  int read(char* buffer, size_t len) final;

  int peek() final;
  void flush() final;

  // Returns the total size of the received packet data. This is only valid if a
  // packet has been received with parsePacket().
  size_t size() const {
    return packet_.data.size();
  }

  // Returns a pointer to the received packet data. This is only valid if a
  // packet has been received with parsePacket(). This may return NULL if the
  // size is zero.
  const uint8_t* data() const {
    return packet_.data.data();
  }

  IPAddress remoteIP() final;
  uint16_t remotePort() final {
    return packet_.port;
  }

  // Returns the approximate packet arrival time, measured with sys_now(). This
  // is only valid if a packet has been received with parsePacket().
  //
  // This is useful in the case where packets have been queued and the caller
  // needs the approximate arrival time. Packets are timestamped when the UDP
  // receive callback is called.
  uint32_t receivedTimestamp() const {
    return packet_.receivedTimestamp;
  }

  // Returns whether the socket is listening.
  explicit operator bool() const {
    return listening_;
  }

  // Sets the differentiated services (DiffServ, DS) field in the outgoing IP
  // header. The top 6 bits are the differentiated services code point (DSCP)
  // value, and the bottom 2 bits are the explicit congestion notification
  // (ECN) value.
  //
  // This attempts to create the necessary internal state, if not already
  // created, and returns whether successful. This will not be successful if the
  // internal state could not be created.
  //
  // Other functions that create the internal state: begin(), beginWithReuse(),
  // beginPacket(), and send().
  //
  // Note that this must be set again after calling stop().
  //
  // If this returns false and there was an error then errno will be set.
  bool setOutgoingDiffServ(uint8_t ds) final;

  // Returns the differentiated services (DiffServ) value from the outgoing IP
  // header. This will return zero if the internal state has not yet
  // been created.
  uint8_t outgoingDiffServ() const final;

  // Returns the received packet's DiffServ value. This is only valid if a
  // packet has been received with parsePacket().
  uint8_t receivedDiffServ() const {
    return packet_.diffServ;
  }

  // Sets the TTL field in the outgoing IP header.
  //
  // This attempts to create the necessary internal state, if not already
  // created, and returns whether successful. This will not be successful if the
  // internal state could not be created.
  //
  // Other functions that create the internal state: begin(), beginWithReuse(),
  // beginPacket(), and send().
  //
  // Note that this must be set again after calling stop().
  //
  // If this returns false and there was an error then errno will be set.
  bool setOutgoingTTL(uint8_t ttl) final;

  // Returns the TTL value from the outgoing IP header. This will return zero if
  // the internal state has not yet been created.
  uint8_t outgoingTTL() const final;

  // Returns the received packet's TTL value. This is only valid if a packet has
  // been received with parsePacket().
  uint8_t receivedTTL() const {
    return packet_.ttl;
  }

 private:
  struct Packet final {
    uint8_t diffServ = 0;
    uint8_t ttl = 0;
    std::vector<uint8_t> data;
    ip_addr_t addr = *IP_ANY_TYPE;
    volatile uint16_t port = 0;
    volatile uint32_t receivedTimestamp = 0;  // Approximate arrival time

    // Clears all the data.
    void clear();
  };

  static void recvFunc(void* arg, struct udp_pcb* pcb, struct pbuf* p,
                       const ip_addr_t* addr, u16_t port);

  // Attempts to create the internal PCB if it's not already set. If
  // unsuccessful, this calls Ethernet.loop(), sets errno to ENOMEM, and returns
  // false. This returns true if the PCB is set.
  bool tryCreatePCB();

  // Starts listening on a port and sets the SO_REUSEADDR socket option
  // according to the `reuse` parameter. This returns whether the attempt
  // was successful.
  bool begin(uint16_t localPort, bool reuse);

  // Multicast functions make use of Ethernet.joinGroup()
  //
  // If this returns false and there was an error then errno will be set.
  bool beginMulticast(const IPAddress& ip, uint16_t port, bool reuse);

  // ip_addr_t versions of transmission functions
  bool beginPacket(const ip_addr_t* ipaddr, uint16_t port);

  // If this returns false and there was an error then errno will be set.
  bool send(const ip_addr_t* ipaddr, uint16_t port,
            const void* data, size_t len);

  // Checks if there's data still available in the packet.
  bool isAvailable() const;

  udp_pcb* pcb_ = nullptr;

  // Listening parameters
  bool listening_          = false;
  bool listenReuse_        = false;
  bool listeningMulticast_ = false;
  IPAddress multicastIP_;

  // Received packet; updated every time one is received
  std::vector<Packet> inBuf_;  // Holds received packets
  size_t inBufTail_ = 0;
  size_t inBufHead_ = 0;
  size_t inBufSize_ = 0;

  // Packet being processed by the caller
  Packet packet_;       // Holds the packet being read
  int packetPos_ = -1;  // -1 if not currently reading a packet

  // Outgoing packets
  internal::optional<Packet> outPacket_;

  // Stats
  uint32_t droppedReceiveCount_ = 0;
  uint32_t totalReceiveCount_   = 0;
};

}  // namespace network
}  // namespace qindesign

#endif  // LWIP_UDP
