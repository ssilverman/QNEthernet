// SPDX-FileCopyrightText: (c) 2023-2025 Shawn Silverman <shawn@pobox.com>
// SPDX-License-Identifier: MIT

// lwIP hook definitions for QNEthernet library on Teensy 4.1.
// This file is part of the QNEthernet library.

#include "lwip_hooks.h"

#if LWIP_TCP && QNETHERNET_ENABLE_SECURE_TCP_ISN

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
#include <string.h>

#include "qnethernet/security/siphash.h"

// Key
static bool s_haveKey = false;
static uint8_t s_key[16];  // Filled with entropy on first use
static uint8_t s_msg[2*sizeof(u16_t) + 2*sizeof(ip_addr_t)];

// Forward declarations
size_t qnethernet_hal_fill_entropy(void *buf, size_t size);
uint32_t qnethernet_hal_micros();

// The algorithm used here follows the suggestions of RFC 6528.
// See:
// * https://datatracker.ietf.org/doc/html/rfc6528
// * https://github.com/torvalds/linux/blob/3666f666e99600518ab20982af04a078bbdad277/net/core/secure_seq.c#L136
// * https://github.com/openbsd/src/blob/bd92615b04061daa96ce60d53401eedbb5c09e0d/sys/netinet/tcp_subr.c#L140
// * https://web.archive.org/web/20031205021237/http://razor.bindview.com/publish/papers/tcpseq.html

u32_t calc_tcp_isn(const ip_addr_t *const local_ip, const u16_t local_port,
                   const ip_addr_t *const remote_ip, const u16_t remote_port) {
  if (!s_haveKey) {
    // Initialize the key
    qnethernet_hal_fill_entropy(s_key, sizeof(s_key));
    s_haveKey = true;
  }

  // Attempt to hash in order of higher entropy -> lower entropy
  uint8_t *pMsg = s_msg;
  memcpy(pMsg, &local_port, sizeof(u16_t));
  pMsg += sizeof(u16_t);
  memcpy(pMsg, &remote_port, sizeof(u16_t));
  pMsg += sizeof(u16_t);
  memcpy(pMsg, remote_ip, sizeof(ip_addr_t));
  pMsg += sizeof(ip_addr_t);
  memcpy(pMsg, local_ip, sizeof(ip_addr_t));

  uint32_t hash = siphash(2, 4, s_key, s_msg, sizeof(s_msg));
  return hash + qnethernet_hal_micros();
}

#endif  // LWIP_TCP && QNETHERNET_ENABLE_SECURE_TCP_ISN
