// SPDX-FileCopyrightText: (c) 2021-2023 Shawn Silverman <shawn@pobox.com>
// SPDX-License-Identifier: MIT

// lwip_unsupported.c contains the unsupported Ethernet interface implementation.
// This file is part of the QNEthernet library.

#if !defined(ARDUINO_TEENSY41)

#include "lwip_driver.h"

bool enet_is_unknown() {
  return false;
}

void enet_get_mac(uint8_t *mac) {
}

bool enet_has_hardware() {
  return false;
}

bool enet_init(const uint8_t mac[ETH_HWADDR_LEN],
               const ip4_addr_t *ipaddr,
               const ip4_addr_t *netmask,
               const ip4_addr_t *gw,
               netif_ext_callback_fn callback) {
  return false;
}

void enet_deinit() {
}

struct netif *enet_netif() {
  return NULL;
}

void enet_proc_input(void) {
}

void enet_poll() {
}

int phy_link_speed() {
  return 0;
}

bool phy_link_is_full_duplex() {
  return false;
}

bool phy_link_is_crossover() {
  return false;
}

bool enet_output_frame(const uint8_t *frame, size_t len) {
  return false;
}

// --------------------------------------------------------------------------
//  MAC Address Filtering
// --------------------------------------------------------------------------

#ifndef QNETHERNET_ENABLE_PROMISCUOUS_MODE

bool enet_set_mac_address_allowed(const uint8_t *mac, bool allow) {
  return false;
}

bool enet_join_group(const ip4_addr_t *group) {
  return false;
}

bool enet_leave_group(const ip4_addr_t *group) {
  return false;
}

#endif  // !QNETHERNET_ENABLE_PROMISCUOUS_MODE

#endif  // !ARDUINO_TEENSY41
