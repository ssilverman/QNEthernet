// SPDX-FileCopyrightText: (c) 2025-2026 Shawn Silverman <shawn@pobox.com>
// SPDX-License-Identifier: AGPL-3.0-or-later

// QNPing.cpp implements Ping.
// This file is part of the QNEthernet library.

#include "qnethernet/QNPing.h"

#if LWIP_RAW

#include <cerrno>
#include <limits>

#include "QNEthernet.h"
#include "lwip/def.h"
#include "lwip/err.h"
#include "lwip/icmp.h"
#include "lwip/inet_chksum.h"
#include "lwip/prot/ip4.h"
#include "lwip/prot/ip.h"
#include "qnethernet/util/ip_tools.h"

namespace qindesign {
namespace network {

static constexpr size_t kEchoHdrSize = sizeof(struct icmp_echo_hdr);
static constexpr size_t kMaxOutHdrSize = IP_HLEN + kEchoHdrSize;
static_assert(kMaxOutHdrSize <= std::numeric_limits<uint16_t>::max(),
              "Max. header size overflow");

uint8_t Ping::recvFunc(void* arg, struct raw_pcb* pcb, struct pbuf* p,
                       const ip_addr_t* addr) {
  // Ensure the PCB matches and the packet is the right type and size
  Ping* const ping = static_cast<Ping*>(arg);

  const uint16_t ipHdrSize = ip_current_header_tot_len();
  const uint16_t hdrSize = ipHdrSize + kEchoHdrSize;

  if ((ping->pcb_ != pcb) ||
      (p->tot_len < hdrSize) ||
      (pbuf_get_at(p, ipHdrSize) != ICMP_ER) ||  // Type
      (pbuf_get_at(p, ipHdrSize + 1) != 0)) {    // Code
    return 0;  // Don't eat the packet
  }

  // Execute the callback
  if (ping->replyf_) {
    struct icmp_echo_hdr echo;

    const size_t copied = pbuf_copy_partial(p, &echo, kEchoHdrSize, ipHdrSize);
    if (copied != kEchoHdrSize) {
      LWIP_PLATFORM_ASSERT("Expected header copy success");
    }

    const size_t dataSize = p->tot_len - hdrSize;  // 16-bit
    const uint8_t* data = nullptr;

    if (dataSize != 0) {
      if (p->len == p->tot_len) {
        data = static_cast<uint8_t*>(p->payload) + hdrSize;
      } else {
        // Avoid churn, so use a vector instead of a byte array
        ping->dataBuf_.resize(dataSize);

        const size_t copied = pbuf_copy_partial(
            p, ping->dataBuf_.data(), static_cast<uint16_t>(dataSize), hdrSize);
        if (copied != dataSize) {
          LWIP_PLATFORM_ASSERT("Expected data copy success");
        }

        data = ping->dataBuf_.data();
      }
    }

    const PingData reply{.ip       = util::ip_addr_get_ip4_uint32(addr),
                         .ttl      = pbuf_get_at(p, 8),
                         .id       = ntohs(echo.id),
                         .seq      = ntohs(echo.seqno),
                         .data     = data,
                         .dataSize = dataSize};
    ping->replyf_(reply);
  }

  (void)pbuf_free(p);
  return 1;  // Eat the packet
}

Ping::~Ping() noexcept {
  // Ensure the PCB is removed
  if (pcb_ != nullptr) {
    raw_remove(pcb_);
    pcb_ = nullptr;
    bound_ = false;
  }
}

bool Ping::tryCreatePCB() {
  // First try to create the PCB
  if (pcb_ == nullptr) {
    pcb_ = raw_new(IP_PROTO_ICMP);
    if (pcb_ == nullptr) {
      Ethernet.loop();  // Allow the stack to move along
      errno = ENOMEM;
      return false;
    }

    raw_recv(pcb_, &recvFunc, this);
  }

  // Next, set it up
  if (!bound_) {
    const err_t err = raw_bind(pcb_, IP_ANY_TYPE);
    if (err != ERR_OK) {
      Ethernet.loop();  // Allow the stack to move along
      errno = err_to_errno(err);
      return false;
    }
    bound_ = true;
  }

  return true;
}

bool Ping::send(const PingData& req) {
  // IPv4 header size is 20
  if (req.dataSize > (std::numeric_limits<uint16_t>::max() - kMaxOutHdrSize)) {
    errno = EINVAL;
    return false;
  }

  // Create the PCB, if needed
  if (!tryCreatePCB()) {
    // errno is already set and Ethernet.loop() called
    return false;
  }

  // Allocate the IP packet
  const size_t packetSize = kEchoHdrSize + req.dataSize;
  struct pbuf* const p =
      pbuf_alloc(PBUF_IP, static_cast<uint16_t>(packetSize), PBUF_RAM);
  if (p == nullptr) {
    Ethernet.loop();  // Allow the stack to move along
    errno = ENOMEM;
    return false;
  }

  // Prepare the ICMP packet
  // Note: Not casting the pbuf payload to an echo header in case there are
  //       alignment mismatches
  struct icmp_echo_hdr echo;
  ICMPH_TYPE_SET(&echo, ICMP_ECHO);
  ICMPH_CODE_SET(&echo, 0);
  echo.chksum = 0;
  echo.id = htons(req.id);
  echo.seqno = htons(req.seq);

  // Must be before the gotos
  const ip_addr_t ipaddr IPADDR4_INIT(static_cast<uint32_t>(req.ip));
#if QNETHERNET_ENABLE_RAW_FRAME_SUPPORT || \
    !defined(QNETHERNET_INTERNAL_DRIVER_TEENSY41)
  uint16_t chksum;
#endif  // QNETHERNET_ENABLE_RAW_FRAME_SUPPORT ||
        // !defined(QNETHERNET_INTERNAL_DRIVER_TEENSY41)

  err_t err = pbuf_take(p, &echo, sizeof(echo));
  if (err != ERR_OK) {
    goto send_err;
  }
  if ((req.data != nullptr) && (req.dataSize != 0)) {
    err = pbuf_take_at(p, req.data, static_cast<uint16_t>(req.dataSize),
                       sizeof(echo));
    if (err != ERR_OK) {
      goto send_err;
    }
  }

#if QNETHERNET_ENABLE_RAW_FRAME_SUPPORT || \
    !defined(QNETHERNET_INTERNAL_DRIVER_TEENSY41)
  chksum = inet_chksum(p->payload, static_cast<uint16_t>(packetSize));
  pbuf_take_at(p, &chksum, 2, offsetof(struct icmp_echo_hdr, chksum));
#endif  // QNETHERNET_ENABLE_RAW_FRAME_SUPPORT ||
        // !defined(QNETHERNET_INTERNAL_DRIVER_TEENSY41)

  // Send the packet
  pcb_->ttl = req.ttl;
#if LWIP_MULTICAST_TX_OPTIONS
  raw_set_multicast_ttl(pcb_, req.ttl);
#endif  // LWIP_MULTICAST_TX_OPTIONS
  err = raw_sendto(pcb_, p, &ipaddr);
  if (err != ERR_OK) {
    goto send_err;
  }

  (void)pbuf_free(p);
  Ethernet.loop();  // Allow the stack to move along
  return true;

send_err:
  errno = err_to_errno(err);
  (void)pbuf_free(p);
  Ethernet.loop();  // Allow the stack to move along
  return false;
}

}  // namespace network
}  // namespace qindesign

#endif  // LWIP_RAW
