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
#include <time.h>

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

// Returns the link speed in Mbps.
int enet_link_speed();

// Returns the link duplex mode, true for full and false for half.
bool enet_link_is_full_duplex();

// Outputs a raw ethernet frame. This returns false if frame is NULL or if the
// length is not in the range 60-(MAX_FRAME_LEN-4) (excludes the FCS (frame
// check sequence)). This also returns false if Ethernet is not initialized. The
// frame is timestamped if `enet_timestamp_next_frame()` was called first.
//
// This adds any extra padding bytes given by ETH_PAD_SIZE.
bool enet_output_frame(const uint8_t *frame, size_t len);

#ifndef QNETHERNET_PROMISCUOUS_MODE
// For joining and leaving multicast groups; these call
// enet_set_mac_address_allowed() with the MAC addresses related to the given
// multicast group. Note that this affects low-level MAC filtering and not the
// IP stack's use of multicast groups.
void enet_join_group(const ip4_addr_t *group);
void enet_leave_group(const ip4_addr_t *group);

// Allows or disallows frames addressed to the specified MAC address. This is
// not meant to be used for joining or leaving a multicast group at the IP
// layer; use the IP stack for that.
//
// Because the underlying system uses a hash of the MAC address, it's possible
// for there to be collisions. This means that it's not always possible to
// disallow an address once it's been allowed.
void enet_set_mac_address_allowed(const uint8_t *mac, bool allow);
#endif  // !QNETHERNET_PROMISCUOUS_MODE

// --------------------------------------------------------------------------
//  IEEE 1588 functions
// --------------------------------------------------------------------------

// Initializes and enables the IEEE 1588 timer and functionality. The internal
// time is reset to zero.
void enet_ieee1588_init();

// Deinitializes and stops the IEEE 1588 timer.
void enet_ieee1588_deinit();

// Tests if the IEEE 1588 timer is enabled.
bool enet_ieee1588_is_enabled();

// Reads the IEEE 1588 timer. This returns whether successful.
//
// This will return false if the argument is NULL.
bool enet_ieee1588_read_timer(struct timespec *t);

// Writes the IEEE 1588 timer. This returns whether successful.
//
// This will return false if the argument is NULL.
bool enet_ieee1588_write_timer(const struct timespec *t);

// Adds an offset to the current timer value. Uses the read and write
// functions under the hood. This returns whether successful.
bool enet_ieee1588_offset_timer(int64_t ns);

// Tells the driver to timestamp the next transmitted frame.
void enet_ieee1588_timestamp_next_frame();

// Returns whether an IEEE 1588 transmit timestamp is available. If available
// and the parameter is not NULL then it is assigned to `*timestamp`. This
// clears the timestamp state so that a subsequent call will return false.
//
// This function is used after sending a packet having its transmit timestamp
// sent. Note that this only returns the latest value, so if a second
// timestamped packet is sent before retrieving the timestamp for the first
// then this will return the second timestamp (if already available).
bool enet_ieee1588_read_and_clear_tx_timestamp(struct timespec *timestamp);

// Directly adjust the correction increase and correction period. To adjust the
// timer in "nanoseconds per second", see `enet_ieee1588_adjust_freq`. This
// returns whether successful.
//
// This will return false if:
// 1. The correction increment is not in the range 0-127, or
// 2. The correction period is not in the range 0-(2^31-1).
bool enet_ieee1588_adjust_timer(uint32_t corrInc, uint32_t corrPeriod);

// Adjust the correction in nanoseconds per second. This uses
// `enet_ieee1588_adjust_timer()` under the hood.
bool enet_ieee1588_adjust_freq(int nsps);

// Sets the channel mode for the given channel. This does not set the output
// compare pulse modes. This returns whether successful.
//
// This will return false if:
// 1. The channel is unknown,
// 2. The mode is one of the output compare pulse modes, or
// 3. The mode is a reserved value or unknown.
bool enet_ieee1588_set_channel_mode(int channel, int mode);

// Sets the output compare pulse mode and pulse width for the given channel.
// This returns whether successful.
//
// This will return false if:
// 1. The channel is unknown,
// 2. The pulse width is not in the range 1-32.
bool enet_ieee1588_set_channel_output_pulse_width(int channel,
                                                  int pulseWidth);

// Sets the channel compare value. This returns whether successful.
//
// This will return false for an unknown channel.
bool enet_ieee1588_set_channel_compare_value(int channel, uint32_t value);

// Gets the channel compare value. This returns whether successful.
//
// This will return false for an unknown channel.
bool enet_ieee1588_get_channel_compare_value(int channel, uint32_t *value);

// Retrieves and then clears the status for the given channel. This will
// return false for an unknown channel.
bool enet_ieee1588_get_and_clear_channel_status(int channel);

// Enables or disables timer interrupt generation for a channel. This will
// return false for an unknown channel.
bool enet_ieee1588_set_channel_interrupt_enable(int channel, bool enable);

#ifdef __cplusplus
}  // extern "C"
#endif

#endif  // ARDUINO_TEENSY41

#endif  // QNE_LWIP_T41_H_
