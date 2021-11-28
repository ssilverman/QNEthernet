// SPDX-FileCopyrightText: (c) 2021-2022 Shawn Silverman <shawn@pobox.com>
// SPDX-License-Identifier: MIT

// lwip_t41.h defines Teensy 4.1 Ethernet functions.
// Based on code from Paul Stoffregen and others:
// https://github.com/PaulStoffregen/teensy41_ethernet
// This file is part of the QNEthernet library.

#ifndef QNE_LWIP_T41_H_
#define QNE_LWIP_T41_H_

#ifdef ARDUINO_TEENSY41

// C includes
#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

#include "lwip/ip_addr.h"
#include "lwip/netif.h"
#include "lwip/prot/ethernet.h"

#ifdef __cplusplus
extern "C" {
#endif

#define MTU 1500
#define MAX_FRAME_LEN 1522

// Get the built-in Ethernet MAC address.
void enet_getmac(uint8_t *mac);

// Initializes Ethernet. This does not set the interface to "up".
//
// This may be called more than once, but if the MAC address has changed then
// the interface is first removed and then re-added.
//
// It is suggested to initialize the random number generator with srand before
// calling this.
void enet_init(const uint8_t macaddr[ETH_HWADDR_LEN],
               const ip_addr_t *ipaddr,
               const ip_addr_t *netmask,
               const ip_addr_t *gw,
               netif_ext_callback_fn callback);

// Disables Ethernet.
void enet_deinit();

// Gets a pointer to the netif structure. This is useful for the netif callback
// before the default netif has been assigned.
struct netif *enet_netif();

// Processes any Ethernet input. This is meant to be called often by the
// main loop.
void enet_proc_input(void);

// Polls Ethernet link status.
void enet_poll();

// Returns the link speed in Mbps.
int enet_link_speed();

// Outputs a raw ethernet frame. This returns false if frame is NULL or if the
// length is not in the range 64-kMaxFrameLen. This also returns false if
// Ethernet is not initialized. This does not timestamp the packet.
bool enet_output_frame(const uint8_t *frame, size_t len);

// For joining and leaving multicast groups
void enet_join_group(const ip_addr_t *group);
void enet_leave_group(const ip_addr_t *group);

// -----------
//  IEEE 1588
// -----------

// Reads the IEEE 1588 timer.
uint32_t enet_read_1588_timer();

#ifdef __cplusplus
}  // extern "C"
#endif

#endif  // ARDUINO_TEENSY41

#endif  // QNE_LWIP_T41_H_
