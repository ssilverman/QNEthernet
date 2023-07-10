// SPDX-FileCopyrightText: (c) 2021-2023 Shawn Silverman <shawn@pobox.com>
// SPDX-License-Identifier: MIT

// lwip_unsupported.h defines Ethernet functions for unsupported boards.
// This file is part of the QNEthernet library.

#ifndef QNETHERNET_LWIP_UNSUPPORTED_H_
#define QNETHERNET_LWIP_UNSUPPORTED_H_

#if !defined(ARDUINO_TEENSY41)

// C includes
#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

#include "lwip/ip_addr.h"
#include "lwip/netif.h"
#include "lwip/prot/ethernet.h"

#ifdef __cplusplus
extern "C" {
#endif  // __cplusplus

#define MTU 0
#define MAX_FRAME_LEN 0

bool enet_is_unknown();
void enet_get_mac(uint8_t *mac);
bool enet_has_hardware();
bool enet_init(const uint8_t mac[ETH_HWADDR_LEN],
               const ip4_addr_t *ipaddr,
               const ip4_addr_t *netmask,
               const ip4_addr_t *gw,
               netif_ext_callback_fn callback);
void enet_deinit();
struct netif *enet_netif();
void enet_proc_input(void);
void enet_poll();
int phy_link_speed();
bool phy_link_is_full_duplex();
bool phy_link_is_crossover();
bool enet_output_frame(const uint8_t *frame, size_t len);

#ifndef QNETHERNET_ENABLE_PROMISCUOUS_MODE
bool enet_join_group(const ip4_addr_t *group);
bool enet_leave_group(const ip4_addr_t *group);
bool enet_set_mac_address_allowed(const uint8_t *mac, bool allow);
#endif  // !QNETHERNET_ENABLE_PROMISCUOUS_MODE

#ifdef __cplusplus
}  // extern "C"
#endif  // __cplusplus

#endif  // !ARDUINO_TEENSY41

#endif  // QNETHERNET_LWIP_UNSUPPORTED_H_
