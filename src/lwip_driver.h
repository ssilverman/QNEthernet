// SPDX-FileCopyrightText: (c) 2021-2023 Shawn Silverman <shawn@pobox.com>
// SPDX-License-Identifier: MIT

// lwip_driver.h defines Ethernet interface functions.
// Based on code from Paul Stoffregen and others:
// https://github.com/PaulStoffregen/teensy41_ethernet
// This file is part of the QNEthernet library.

#ifndef QNETHERNET_LWIP_DRIVER_H_
#define QNETHERNET_LWIP_DRIVER_H_

// C includes
#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

#include "lwip/ip_addr.h"
#include "lwip/netif.h"
#include "lwip/prot/ethernet.h"

// Requirements for platform-specific headers:
// 1. Define MTU
// 2. Define MAX_FRAME_LEN

#if defined(ARDUINO_TEENSY41)
#include "lwip_t41.h"
#else
#include "lwip_unsupported.h"
#endif  // ARDUINO_TEENSY41

#ifdef __cplusplus
extern "C" {
#endif  // __cplusplus

// Returns the MTU.
inline int enet_get_mtu() {
  return MTU;
}

// Returns. the max. frame length.
inline int enet_get_max_frame_len() {
  return MAX_FRAME_LEN;
}

// Returns if the hardware hasn't yet been probed.
bool enet_is_unknown();

// Gets the built-in Ethernet MAC address. This does nothing if 'mac' is NULL.
void enet_get_mac(uint8_t *mac);

// Determines if there's Ethernet hardware.
bool enet_has_hardware();

// Initializes Ethernet and returns whether successful. This does not set the
// interface to "up".
//
// This may be called more than once, but if the MAC address has changed then
// the interface is first removed and then re-added.
//
// It is suggested to initialize the random number generator with srand() before
// calling this.
bool enet_init(const uint8_t mac[ETH_HWADDR_LEN],
               const ip4_addr_t *ipaddr,
               const ip4_addr_t *netmask,
               const ip4_addr_t *gw,
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

// Returns the link speed in Mbps. The value is only valid if the link is up.
int phy_link_speed();

// Returns the link duplex mode, true for full and false for half. The value is
// only valid if the link is up.
bool phy_link_is_full_duplex();

// Returns whether a crossover cable is detected. The value is only valid if the
// link is up.
bool phy_link_is_crossover();

// Outputs a raw ethernet frame. This returns false if frame is NULL or if the
// length is not in the correct range. The proper range is 14-(MAX_FRAME_LEN-8)
// for non-VLAN frames and 18-(MAX_FRAME_LEN-4) for VLAN frames. Note that these
// ranges exclude the 4-byte FCS (frame check sequence).
//
// This also returns false if Ethernet is not initialized.
//
// This adds any extra padding bytes given by ETH_PAD_SIZE.
bool enet_output_frame(const uint8_t *frame, size_t len);

#ifndef QNETHERNET_ENABLE_PROMISCUOUS_MODE

// For joining and leaving multicast groups; these call
// enet_set_mac_address_allowed() with the MAC addresses related to the given
// multicast group. Note that this affects low-level MAC filtering and not the
// IP stack's use of multicast groups.
//
// If 'group' is NULL then these return false. Otherwise, these return the
// result of 'enet_set_mac_address_allowed()'.
bool enet_join_group(const ip4_addr_t *group);
bool enet_leave_group(const ip4_addr_t *group);

// Allows or disallows frames addressed to the specified MAC address. This is
// not meant to be used for joining or leaving a multicast group at the IP
// layer; use the IP stack for that.
//
// Because the underlying system uses a hash of the MAC address, it's possible
// for there to be collisions. This means that it's not always possible to
// disallow an address once it's been allowed.
//
// This returns true if adding or removing the MAC was successful. If an address
// has a collision, then it can't be removed and this will return false. This
// will also return false if 'mac' is NULL. Otherwise, this will return true.
bool enet_set_mac_address_allowed(const uint8_t *mac, bool allow);

#endif  // !QNETHERNET_ENABLE_PROMISCUOUS_MODE

#ifdef __cplusplus
}  // extern "C"
#endif  // __cplusplus

#endif  // QNETHERNET_LWIP_DRIVER_H_
