// SPDX-FileCopyrightText: (c) 2021-2024 Shawn Silverman <shawn@pobox.com>
// SPDX-License-Identifier: AGPL-3.0-or-later

// lwip_driver.h defines Ethernet interface functions.
// Based on code from manitou48 and others:
// https://github.com/PaulStoffregen/teensy41_ethernet
// This file is part of the QNEthernet library.

#pragma once

// C includes
#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

#include "lwip/ip_addr.h"
#include "lwip/netif.h"
#include "lwip/pbuf.h"
#include "lwip/prot/ethernet.h"
#include "qnethernet_opts.h"

// Requirements for driver-specific headers:
// 1. Define MTU
// 2. Define MAX_FRAME_LEN (including the 4-byte FCS (frame check sequence))

// How to create a driver:
// 1. Create a header that defines MTU and MAX_FRAME_LEN. Don't forget to use
//    either `#pragma once` or a #define guard.
// 2. Create driver source and include lwip_driver.h. Implement all the
//    `driver_x()` functions. It can be written in either C or C++. If C++ then
//    make sure to use `extern "C"` around those functions.
// 3. Adjust the following driver selection logic to define an appropriate macro
//    (such as INTERNAL_DRIVER_Y) when the desired driver condition
//    is satisfied.
// 4. Include your driver header in the correct place in the logic below.
// 5. In your driver source, gate the whole file(s) on the macro you chose
//    above. Of course, test the macro after the lwip_driver.h include.
//    (Example: INTERNAL_DRIVER_Y)
// 6. Update lwipopts.h with appropriate values for your driver.
// 7. Optionally update EthernetClass::hardwareStatus() to return an appropriate
//    enum value. If no change is made, the default 'EthernetOtherHardware' will
//    be returned if hardware is found (driver_has_hardware() returns true).

// Select a driver
#if defined(QNETHERNET_DRIVER_W5500)
#include "drivers/driver_w5500.h"
#define QNETHERNET_INTERNAL_DRIVER_W5500
#elif defined(ARDUINO_TEENSY41)
#include "drivers/driver_teensy41.h"
#define QNETHERNET_INTERNAL_DRIVER_TEENSY41
#else
#include "drivers/driver_unsupported.h"
#define QNETHERNET_INTERNAL_DRIVER_UNSUPPORTED
#endif  // Driver selection

#ifdef __cplusplus
extern "C" {
#endif  // __cplusplus

// --------------------------------------------------------------------------
//  Driver Interface
// --------------------------------------------------------------------------

// It can be assumed that any parameters passed in will not be NULL.

// Returns if the hardware hasn't yet been probed.
bool driver_is_unknown();

// Gets the built-in Ethernet MAC address.
//
// For systems without a built-in address, this should retrieve some default.
void driver_get_system_mac(uint8_t *mac);

// Sets the internal MAC address.
void driver_set_mac(const uint8_t mac[ETH_HWADDR_LEN]);

// Determines if there's Ethernet hardware. If the hardware hasn't yet been
// probed (driver_is_unknown() would return 'true'), then this will check
// the hardware.
bool driver_has_hardware();

// Sets the SPI chip select pin given in Ethernet.init(). The pin will be -1 if
// it has not been initialized.
void driver_set_chip_select_pin(int pin);

// Does low-level initialization. This returns whether the initialization
// was successful.
bool driver_init(const uint8_t mac[ETH_HWADDR_LEN]);

// Uninitializes the driver.
void driver_deinit();

// Processes any input and passes any received frames to the netif.
void driver_proc_input(struct netif *netif);

// Polls anything that needs to be polled, for example, the link status.
void driver_poll(struct netif *netif);

// Returns the link speed in Mbps. The value is only valid if the link is up.
int driver_link_speed();

// Returns the link duplex mode, true for full and false for half. The value is
// only valid if the link is up.
bool driver_link_is_full_duplex();

// Returns whether a crossover cable is detected. The value is only valid if the
// link is up.
bool driver_link_is_crossover();

// Outputs the given pbuf data.
//
// Note that the data will already contain any extra ETH_PAD_SIZE bytes.
err_t driver_output(struct pbuf *p);

// Outputs a raw Ethernet frame and returns whether successful.
//
// This should add any extra padding bytes given by ETH_PAD_SIZE.
bool driver_output_frame(const uint8_t *frame, size_t len);

#if !QNETHERNET_ENABLE_PROMISCUOUS_MODE

// Allows or disallows frames addressed to the specified MAC address. This is
// not meant to be used for joining or leaving a multicast group at the IP
// layer; use the IP stack for that.
//
// Because the underlying system might use a hash of the MAC address, it's
// possible for there to be collisions. This means that it's not always possible
// to disallow an address once it's been allowed.
//
// This returns true if adding or removing the MAC was successful. If an address
// has a collision, then it can't be removed and this will return false. This
// will also return false if 'mac' is NULL. Otherwise, this will return true.
//
// Note that this function may be passed a NULL MAC address.
bool driver_set_mac_address_allowed(const uint8_t *mac, bool allow);

#endif  // !QNETHERNET_ENABLE_PROMISCUOUS_MODE

// --------------------------------------------------------------------------
//  Public Interface
// --------------------------------------------------------------------------

// Returns the MTU.
inline int enet_get_mtu() {
  return MTU;
}

// Returns the maximum frame length. This includes the 4-byte FCS (frame
// check sequence).
inline int enet_get_max_frame_len() {
  return MAX_FRAME_LEN;
}

// Gets the built-in Ethernet MAC address. This does nothing if 'mac' is NULL.
//
// For systems without a built-in address, this should retrieve some default.
void enet_get_mac(uint8_t *mac);

// Initializes Ethernet and returns whether successful. This does not set the
// interface to "up".
//
// This may be called more than once, but if the MAC address has changed then
// the interface is first removed and then re-added.
//
// It is suggested to initialize the random number generator before
// calling this.
bool enet_init(const uint8_t mac[ETH_HWADDR_LEN],
               netif_ext_callback_fn callback);

// Shuts down the Ethernet stack and driver.
void enet_deinit();

// Gets a pointer to the netif structure. This is useful for the netif callback
// before the default netif has been assigned.
struct netif *enet_netif();

// Processes any Ethernet input. This is meant to be called often by the
// main loop.
void enet_proc_input();

// Polls the stack (if needed) and Ethernet link status.
void enet_poll();

// Outputs a raw ethernet frame. This returns false if frame is NULL or if the
// length is not in the correct range. The proper range is 14-(MAX_FRAME_LEN-8)
// for non-VLAN frames and 18-(MAX_FRAME_LEN-4) for VLAN frames. Note that these
// ranges exclude the 4-byte FCS (frame check sequence).
//
// This returns the result of driver_output_frame(), if the frame checks pass.
bool enet_output_frame(const uint8_t *frame, size_t len);

#if !QNETHERNET_ENABLE_PROMISCUOUS_MODE

// For joining and leaving multicast groups; these call
// driver_set_mac_address_allowed() with the MAC addresses related to the given
// multicast group. Note that this affects low-level MAC filtering and not the
// IP stack's use of multicast groups.
//
// If 'group' is NULL then these return false. Otherwise, these return the
// result of 'enet_set_mac_address_allowed()'.
bool enet_join_group(const ip4_addr_t *group);
bool enet_leave_group(const ip4_addr_t *group);

#endif  // !QNETHERNET_ENABLE_PROMISCUOUS_MODE

#ifdef __cplusplus
}  // extern "C"
#endif  // __cplusplus
