// SPDX-FileCopyrightText: (c) 2025 Shawn Silverman <shawn@pobox.com>
// SPDX-License-Identifier: AGPL-3.0-or-later

// QNPing.h defines the Ping interface.
// This file is part of the QNEthernet library.

#pragma once

#include "lwip/opt.h"

#if LWIP_RAW

// C++ includes
#include <cstddef>
#include <cstdint>
#include <functional>
#include <vector>

#include <IPAddress.h>

#include "lwip/arch.h"
#include "lwip/ip_addr.h"
#include "lwip/pbuf.h"
#include "lwip/raw.h"

namespace qindesign {
namespace network {

// PingData holds ping request or reply data.
struct PingData {
  IPAddress ip;
  uint8_t ttl         = QNETHERNET_DEFAULT_PING_TTL;
  uint16_t id         = QNETHERNET_DEFAULT_PING_ID;
  uint16_t seq        = 0;
  const uint8_t* data = nullptr;
  size_t dataSize     = 0;
};

// Interfaces with lwIP's ICMP functions to perform a ping.
class Ping final {
 public:
  // Function type for receiving ping replies. data may be NULL and dataSize
  // will be zero if there's no echo reply data.
  using replyf = std::function<void(const PingData& reply)>;

  // Creates a new Ping object with no reply callback.
  Ping() = default;

  // Creates a new Ping object with the given reply callback.
  Ping(replyf f)
      : replyf_(f) {}

  ~Ping();

  // Disallow copying, but allow moving
  Ping(const Ping&) = delete;
  Ping(Ping&&) = default;
  Ping& operator=(const Ping&) = delete;
  Ping& operator=(Ping&&) = default;

  // Sets the callback to the given function.
  void setCallback(replyf f) {
    replyf_ = f;
  }

  // Sends an Echo Request ICMP packet to the given IP address.
  //
  // If this returns false and there was an error then errno will be set.
  bool send(const PingData& req);

 private:
  // Receives raw packets.
  static u8_t recvFunc(void* arg, struct raw_pcb* pcb, struct pbuf* p,
                       const ip_addr_t* addr);

  // Attempts to create and bind the internal PCB if it's not already set. If
  // unsuccessful, this calls Ethernet.loop(), sets errno to ENOMEM, and returns
  // false. This returns true if the PCB is set.
  bool tryCreatePCB();

  replyf replyf_;
  struct raw_pcb* pcb_ = nullptr;
  bool bound_ = false;  // Whether the PCB is bound

  std::vector<uint8_t> dataBuf_;
};

}  // namespace network
}  // namespace qindesign

#endif  // LWIP_RAW
