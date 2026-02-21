// SPDX-FileCopyrightText: (c) 2021-2026 Shawn Silverman <shawn@pobox.com>
// SPDX-License-Identifier: AGPL-3.0-or-later

// lwip_driver.h defines Ethernet interface functions.
// Based on code from manitou48 and others:
// https://github.com/PaulStoffregen/teensy41_ethernet
// This file is part of the QNEthernet library.

#pragma once

// C includes
#if !defined(__cplusplus) && (__STDC_VERSION__ < 202311L)
#include <stdbool.h>
#endif  // !C++ && (C < 23)
#include <stddef.h>
#include <stdint.h>

#include "lwip/ip_addr.h"
#include "lwip/netif.h"
#include "lwip/opt.h"
#include "lwip/pbuf.h"
#include "lwip/prot/ethernet.h"
#include "qnethernet/compat/c11_compat.h"
#include "qnethernet/driver_select.h"
#include "qnethernet_opts.h"

// Check things that are supposed to be set properly by the driver headers
STATIC_ASSERT(MTU >= 0, "MTU must be defined and > 0");
STATIC_ASSERT(MAX_FRAME_LEN >= 0, "MAX_FRAME_LEN must be defined and >= 0");

enum {
  MIN_FRAME_LEN = 60,
};

// STATIC_ASSERT(MIN_FRAME_LEN >= 0, "MIN_FRAME_LEN must be >= 0");

// Check some sizes
STATIC_ASSERT(ETH_PAD_SIZE <= UINT16_MAX, "ETH_PAD_SIZE must be <= UINT16_MAX");

// Requirements for driver-specific headers:
// 1. Define MTU
// 2. Define MAX_FRAME_LEN (not including the 4-byte FCS (frame check sequence))
//    1. It will usually be MTU + 14 (2 6-byte MACs, 2-byte tag) + 4 (VLAN)
// 3. If necessary, define lwIP options (see lwip/opt.h) with appropriate values
//    for your driver. For example, Ethernet padding, checksum generation, and
//    checksum checking.

// How to create a driver:
// 1. Create a header that defines MTU and MAX_FRAME_LEN. Don't forget to use
//    either `#pragma once` or a #define guard.
// 2. Create driver source and include lwip_driver.h. Implement all the
//    `driver_x()` functions. It can be written in either C or C++. If C++ then
//    make sure to use `extern "C"` around those functions.
// 3. Adjust the driver selection logic in driver_select.h to define an
//    appropriate macro (such as INTERNAL_DRIVER_Y) when the desired driver
//    condition is satisfied.
// 4. Include your driver header in the correct place in driver_select.h.
// 5. In your driver source, gate the whole file(s) on the macro you chose
//    above. Of course, test the macro after the lwip_driver.h include.
//    (Example: INTERNAL_DRIVER_Y)
// 6. Add lwIP options (see lwip/opt.h) with appropriate values for your driver.
//    For example, Ethernet padding, checksum generation, and checksum checking.
// 7. Optionally update EthernetClass::hardwareStatus() to return an appropriate
//    enum value. If no change is made, the default 'EthernetOtherHardware' will
//    be returned if hardware is found (driver_has_hardware() returns true).

// How to create an external driver that isn't inside the distribution:
// 1. Create a header named "qnethernet_external_driver.h" that defines MTU and
//    MAX_FRAME_LEN. MAX_FRAME_LEN should not include the 4-byte FCS. Don't
//    forget to use either `#pragma once` or a #define guard.
// 2. Add lwIP options (see lwip/opt.h) with appropriate values for your driver.
//    For example, Ethernet padding, checksum generation, and checksum checking.
// 3. Create driver source and include lwip_driver.h. Implement all the
//    `driver_x()` functions. It can be written in either C or C++. If C++ then
//    make sure to use `extern "C"` around those functions.

#ifdef __cplusplus
extern "C" {
#endif  // __cplusplus

// --------------------------------------------------------------------------
//  Types
// --------------------------------------------------------------------------

// Flags that indicate driver capabilities.
struct DriverCapabilities {
#ifdef __cplusplus
  DriverCapabilities()
      : isMACSettable(false),
        isLinkStateDetectable(false),
        isLinkSpeedDetectable(false),
        isLinkSpeedSettable(false),
        isLinkFullDuplexDetectable(false),
        isLinkFullDuplexSettable(false),
        isAutoNegotiationSettable(false),
        isLinkCrossoverDetectable(false),
        isAutoNegotiationRestartable(false),
        isPHYResettable(false) {}
#endif  // __cplusplus

  bool isMACSettable;
  bool isLinkStateDetectable;
  bool isLinkSpeedDetectable;
  bool isLinkSpeedSettable;
  bool isLinkFullDuplexDetectable;
  bool isLinkFullDuplexSettable;
  bool isAutoNegotiationSettable;
  bool isLinkCrossoverDetectable;
  bool isAutoNegotiationRestartable;
  bool isPHYResettable;
};

// Link information. Defaults are:
// * 100 Mbps
// * full-duplex
// * Auto-negotiation enabled
// * Crossover is false
struct LinkInfo {
#ifdef __cplusplus
  LinkInfo()
      : speed(100),
        fullNotHalfDuplex(true),
        isAutoNegotiation(true),
        isCrossover(false) {}
#endif  // __cplusplus

  int speed;
  bool fullNotHalfDuplex;
  bool isAutoNegotiation;
  bool isCrossover;
};

// Link settings. Defaults are:
// * 100 Mbps
// * full-duplex
// * Auto-negotiation enabled
//
// This is separate from LinkInfo because some things are detecatable but
// not settable.
struct LinkSettings {
#ifdef __cplusplus
  LinkSettings()
      : speed(100),
        fullNotHalfDuplex(true),
        autoNegotiation(true) {}
#endif  // __cplusplus

  int speed;
  bool fullNotHalfDuplex;
  bool autoNegotiation;
};

// --------------------------------------------------------------------------
//  Driver Interface
// --------------------------------------------------------------------------

// It can be assumed that any parameters passed in will not be NULL.

// Gets the driver capabilities and fills in the given struct. This may be
// called more than once but may not be valid until after driver_init()
// is called.
void driver_get_capabilities(struct DriverCapabilities* dc);

// Returns if the hardware hasn't yet been probed.
ATTRIBUTE_NODISCARD
bool driver_is_unknown(void);

// Gets the built-in Ethernet MAC address.
//
// For systems without a built-in address, this should retrieve some
// valid default.
void driver_get_system_mac(uint8_t mac[ETH_HWADDR_LEN]);

// Gets the internal MAC address. This will retrieve the system default if the
// internal address has not yet been set.
//
// This returns whether the address was retrieved.
ATTRIBUTE_NODISCARD
bool driver_get_mac(uint8_t mac[ETH_HWADDR_LEN]);

// Sets the internal MAC address and returns whether successful. This will be
// set as a transmitted Ethernet frame's source address.
//
// See also: driver_get_capabilities(dc)
ATTRIBUTE_NODISCARD
bool driver_set_mac(const uint8_t mac[ETH_HWADDR_LEN]);

// Determines if there's Ethernet hardware. If the hardware hasn't yet been
// probed (driver_is_unknown() would return 'true'), then this will check
// the hardware.
ATTRIBUTE_NODISCARD
bool driver_has_hardware(void);

// Sets the SPI chip select pin given in Ethernet.init(). The pin will be -1 if
// it has not been initialized.
void driver_set_chip_select_pin(int pin);

// Does low-level initialization. This returns whether the initialization
// was successful. Most functions depend on the driver being initialized.
ATTRIBUTE_NODISCARD
bool driver_init(void);

// Uninitializes the driver.
void driver_deinit(void);

// Processes any input and returns any received frames as a pbuf. The counter
// parameter indicates how many times the call has looped. This is useful so
// that drivers can know when to stop checking to give the rest of the program a
// chance to run. In other words, it is expected that this will return NULL at
// some point.
ATTRIBUTE_NODISCARD
struct pbuf* driver_proc_input(struct netif* netif, int counter);

// Polls anything that needs to be polled, for example, the link status.
void driver_poll(struct netif* netif);

// Gets info about the link. The values will only be valid when the link is up.
//
// See also: driver_get_capabilities(dc)
void driver_get_link_info(struct LinkInfo* li);

// Sets some link parameters all at once. This returns whether the call
// was successful. It is expected that this will return false if the link is
// not up.
//
// This will also return false if any of the parameters are invalid.
ATTRIBUTE_NODISCARD
bool driver_set_link(const struct LinkSettings* ls);

// Outputs the given pbuf data.
//
// Note that the data will already contain any extra ETH_PAD_SIZE bytes.
ATTRIBUTE_NODISCARD
err_t driver_output(struct pbuf* p);

#if QNETHERNET_ENABLE_RAW_FRAME_SUPPORT
// Outputs a raw Ethernet frame and returns whether successful.
//
// This should add, to the start, any extra padding bytes given by ETH_PAD_SIZE.
ATTRIBUTE_NODISCARD
bool driver_output_frame(const void* frame, size_t len);
#endif  // QNETHERNET_ENABLE_RAW_FRAME_SUPPORT

#if !QNETHERNET_ENABLE_PROMISCUOUS_MODE

// Allows or disallows frames addressed to the specified MAC address. In other
// words, this controls the allowable "destination address" of received
// Ethernet frames.
//
// This is not not meant to be used for joining or leaving a multicast group at
// the IP layer; use the IP stack for that.
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
ATTRIBUTE_NODISCARD
bool driver_set_incoming_mac_address_allowed(const uint8_t mac[ETH_HWADDR_LEN],
                                             bool allow);

#endif  // !QNETHERNET_ENABLE_PROMISCUOUS_MODE

// --------------------------------------------------------------------------
//  Notifications from Upper Layers
// --------------------------------------------------------------------------

// Notifies the driver that the system manually set the link state. It is
// assumed that if the new link state is TRUE then it should be sticky.
void driver_notify_manual_link_state(bool flag);

// --------------------------------------------------------------------------
//  Link Functions
// --------------------------------------------------------------------------

// Restarts auto-negotiation, if the driver supports it.
//
// See also: driver_get_capabilities(dc)
void driver_restart_auto_negotiation(void);

// Resets the PHY, if the driver supports it.
//
// See also: driver_get_capabilities(dc)
void driver_reset_phy(void);

// --------------------------------------------------------------------------
//  Public Interface
// --------------------------------------------------------------------------

// Returns the MTU.
ATTRIBUTE_NODISCARD
inline size_t enet_get_mtu(void) {
  return MTU;
}

// Returns the maximum frame length. This does not include the 4-byte FCS (frame
// check sequence).
ATTRIBUTE_NODISCARD
inline size_t enet_get_max_frame_len(void) {
  return MAX_FRAME_LEN;
}

// Gets the built-in Ethernet MAC address. This does nothing if 'mac' is NULL.
//
// For systems without a built-in address, this should retrieve some default.
void enet_get_system_mac(uint8_t mac[ETH_HWADDR_LEN]);

// Gets the current MAC address and returns whether it was retrieved. This does
// nothing if 'mac' is NULL.
ATTRIBUTE_NODISCARD
bool enet_get_mac(uint8_t mac[ETH_HWADDR_LEN]);

// Sets the current MAC address and returns whether it was changed. This does
// nothing if 'mac' is NULL.
ATTRIBUTE_NODISCARD
bool enet_set_mac(const uint8_t mac[ETH_HWADDR_LEN]);

// Initializes Ethernet and returns whether successful. This does not set the
// interface to "up". If the MAC is not settable or 'mac' is NULL then this will
// use the system MAC address and 'mac' will be ignored. This also fills in the
// driver capabilities struct.
//
// This may be called more than once, but if the MAC address has changed then
// the interface is first removed and then re-added.
//
// It is suggested to initialize the entropy generator with
// qnethernet_hal_init_entropy() before calling this.
//
// See also: driver_get_capabilities(dc)
ATTRIBUTE_NODISCARD
bool enet_init(const uint8_t mac[ETH_HWADDR_LEN],
               netif_ext_callback_fn callback,
               struct DriverCapabilities* dc);

// Shuts down the Ethernet stack and driver.
void enet_deinit(void);

// Gets a pointer to the netif structure. This is useful for the netif callback
// before the default netif has been assigned.
ATTRIBUTE_NODISCARD
struct netif* enet_netif(void);

// Processes any Ethernet input. This is meant to be called often by the
// main loop.
void enet_proc_input(void);

// Polls the stack (if needed) and Ethernet link status.
void enet_poll(void);

#if QNETHERNET_ENABLE_RAW_FRAME_SUPPORT
// Outputs a raw ethernet frame. This returns false if frame is NULL or if the
// length is not in the correct range. The proper range is [14, MAX_FRAME_LEN-4]
// for non-VLAN frames and [18, MAX_FRAME_LEN] for VLAN frames. Note that these
// ranges exclude the 4-byte FCS (frame check sequence).
//
// This returns the result of driver_output_frame(), if the frame checks pass.
ATTRIBUTE_NODISCARD
bool enet_output_frame(const void* frame, size_t len);
#endif  // QNETHERNET_ENABLE_RAW_FRAME_SUPPORT

#if !QNETHERNET_ENABLE_PROMISCUOUS_MODE && LWIP_IPV4

// For joining and leaving multicast groups; these call
// driver_set_incoming_mac_address_allowed() with the MAC addresses related to
// the given multicast group. Note that this affects low-level MAC filtering and
// not the IP stack's use of multicast groups.
//
// If 'group' is NULL then these return false. Otherwise, these return the
// result of 'enet_set_mac_address_allowed()'.
ATTRIBUTE_NODISCARD
bool enet_join_group(const ip4_addr_t* group);
ATTRIBUTE_NODISCARD
bool enet_leave_group(const ip4_addr_t* group);

#endif  // !QNETHERNET_ENABLE_PROMISCUOUS_MODE && LWIP_IPV4

#ifdef __cplusplus
}  // extern "C"
#endif  // __cplusplus
