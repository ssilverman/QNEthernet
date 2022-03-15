// SPDX-FileCopyrightText: (c) 2022 Shawn Silverman <shawn@pobox.com>
// SPDX-License-Identifier: MIT

// lwIP hooks for QNEthernet library on Teensy 4.1.
// This file is part of the QNEthernet library.

#ifndef QNE_LWIPHOOKS_H_
#define QNE_LWIPHOOKS_H_

#include "lwip/err.h"
#include "lwip/netif.h"
#include "lwip/pbuf.h"

#define LWIP_HOOK_UNKNOWN_ETH_PROTOCOL(p, netif) \
  unknown_eth_protocol((p), (netif))

err_t unknown_eth_protocol(struct pbuf *p, struct netif *netif);

#endif  // QNE_LWIPHOOKS_H_
