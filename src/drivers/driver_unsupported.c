// SPDX-FileCopyrightText: (c) 2021-2024 Shawn Silverman <shawn@pobox.com>
// SPDX-License-Identifier: AGPL-3.0-or-later

// driver_unsupported.c contains the unsupported Ethernet interface implementation.
// This file is part of the QNEthernet library.

#include "lwip_driver.h"

#if defined(QNETHERNET_INTERNAL_DRIVER_UNSUPPORTED)

size_t driver_get_mtu() {
  return 0;
}

size_t driver_get_max_frame_len() {
  return 0;
}

bool driver_is_unknown() {
  return false;
}

bool driver_is_link_state_detectable() {
  return false;
}

void qnethernet_hal_get_system_mac_address(uint8_t mac[ETH_HWADDR_LEN]);

void driver_get_system_mac(uint8_t mac[ETH_HWADDR_LEN]) {
  qnethernet_hal_get_system_mac_address(mac);
}

void driver_set_mac(const uint8_t mac[ETH_HWADDR_LEN]) {
  LWIP_UNUSED_ARG(mac);
}

bool driver_has_hardware() {
  return false;
}

void driver_set_chip_select_pin(int pin) {
  LWIP_UNUSED_ARG(pin);
}

bool driver_init(const uint8_t mac[ETH_HWADDR_LEN]) {
  LWIP_UNUSED_ARG(mac);
  return false;
}

void driver_deinit() {
}

void driver_proc_input(struct netif *netif) {
  LWIP_UNUSED_ARG(netif);
}

void driver_poll(struct netif *netif) {
  LWIP_UNUSED_ARG(netif);
}

int driver_link_speed() {
  return 0;
}

bool driver_link_is_full_duplex() {
  return false;
}

bool driver_link_is_crossover() {
  return false;
}

err_t driver_output(struct pbuf *p) {
  LWIP_UNUSED_ARG(p);
  return ERR_IF;
}

#if QNETHERNET_ENABLE_RAW_FRAME_SUPPORT
bool driver_output_frame(const uint8_t *frame, size_t len) {
  LWIP_UNUSED_ARG(frame);
  LWIP_UNUSED_ARG(len);
  return false;
}
#endif  // QNETHERNET_ENABLE_RAW_FRAME_SUPPORT

// --------------------------------------------------------------------------
//  MAC Address Filtering
// --------------------------------------------------------------------------

#if !QNETHERNET_ENABLE_PROMISCUOUS_MODE

bool driver_set_mac_address_allowed(const uint8_t mac[ETH_HWADDR_LEN],
                                    bool allow) {
  LWIP_UNUSED_ARG(mac);
  LWIP_UNUSED_ARG(allow);
  return false;
}

#endif  // !QNETHERNET_ENABLE_PROMISCUOUS_MODE

#endif  // QNETHERNET_INTERNAL_DRIVER_UNSUPPORTED
