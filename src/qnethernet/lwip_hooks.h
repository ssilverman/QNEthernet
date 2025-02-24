// SPDX-FileCopyrightText: (c) 2022-2025 Shawn Silverman <shawn@pobox.com>
// SPDX-License-Identifier: AGPL-3.0-or-later

// lwip_hooks.h contains lwIP hook declarations.
// This file is part of the QNEthernet library.

#pragma once

#include "lwip/arch.h"
#include "lwip/err.h"
#include "lwip/ip_addr.h"
#include "lwip/netif.h"
#include "lwip/opt.h"
#include "lwip/pbuf.h"
#include "qnethernet_opts.h"

#if QNETHERNET_ENABLE_RAW_FRAME_SUPPORT

#define LWIP_HOOK_UNKNOWN_ETH_PROTOCOL(p, netif) \
  unknown_eth_protocol((p), (netif))

err_t unknown_eth_protocol(struct pbuf *p, struct netif *netif);

#endif  // QNETHERNET_ENABLE_RAW_FRAME_SUPPORT

#if LWIP_TCP && QNETHERNET_ENABLE_SECURE_TCP_ISN

#define LWIP_HOOK_TCP_ISN(local_ip, local_port, remote_ip, remote_port) \
  calc_tcp_isn((local_ip), (local_port), (remote_ip), (remote_port))

u32_t calc_tcp_isn(const ip_addr_t *local_ip, u16_t local_port,
                   const ip_addr_t *remote_ip, u16_t remote_port);

#endif  // LWIP_TCP && QNETHERNET_ENABLE_SECURE_TCP_ISN
