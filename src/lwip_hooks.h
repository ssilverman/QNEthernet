// SPDX-FileCopyrightText: (c) 2022 Shawn Silverman <shawn@pobox.com>
// SPDX-License-Identifier: MIT

// lwIP hooks for QNEthernet library on Teensy 4.1.
// This file is part of the QNEthernet library.

err_t unknown_eth_protocol(struct pbuf *p, struct netif *netif);

#define LWIP_HOOK_UNKNOWN_ETH_PROTOCOL(p, netif) \
  unknown_eth_protocol((p), (netif))
