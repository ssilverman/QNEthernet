// SPDX-FileCopyrightText: (c) 2021-2025 Shawn Silverman <shawn@pobox.com>
// SPDX-License-Identifier: AGPL-3.0-or-later

// driver_unsupported.c contains the unsupported Ethernet interface implementation.
// This file is part of the QNEthernet library.

#include "qnethernet/lwip_driver.h"

#include <avr/pgmspace.h>

#include "lwip/arch.h"
#include "lwip/err.h"

#ifndef FLASHMEM
#define FLASHMEM
#endif  // !FLASHMEM

#if defined(QNETHERNET_INTERNAL_DRIVER_UNSUPPORTED)

FLASHMEM void driver_get_capabilities(struct DriverCapabilities *const dc) {
  dc->isMACSettable              = false;
  dc->isLinkStateDetectable      = false;
  dc->isLinkSpeedDetectable      = false;
  dc->isLinkSpeedSettable        = false;
  dc->isLinkFullDuplexDetectable = false;
  dc->isLinkFullDuplexSettable   = false;
  dc->isLinkCrossoverDetectable  = false;
}

bool driver_is_unknown(void) {
  return false;
}

void qnethernet_hal_get_system_mac_address(uint8_t mac[ETH_HWADDR_LEN]);

void driver_get_system_mac(uint8_t mac[ETH_HWADDR_LEN]) {
  qnethernet_hal_get_system_mac_address(mac);
}

bool driver_get_mac(uint8_t mac[ETH_HWADDR_LEN]) {
  driver_get_system_mac(mac);
  return true;
}

bool driver_set_mac(const uint8_t mac[ETH_HWADDR_LEN]) {
  LWIP_UNUSED_ARG(mac);
  return false;
}

bool driver_has_hardware(void) {
  return false;
}

FLASHMEM void driver_set_chip_select_pin(const int pin) {
  LWIP_UNUSED_ARG(pin);
}

FLASHMEM bool driver_init(void) {
  return false;
}

FLASHMEM void driver_deinit(void) {
}

struct pbuf *driver_proc_input(struct netif *const netif, const int counter) {
  LWIP_UNUSED_ARG(netif);
  LWIP_UNUSED_ARG(counter);
  return NULL;
}

void driver_poll(struct netif *const netif) {
  LWIP_UNUSED_ARG(netif);
}

int driver_link_speed(void) {
  return 0;
}

bool driver_link_set_speed(const int speed) {
  LWIP_UNUSED_ARG(speed);
  return false;
}

bool driver_link_is_full_duplex(void) {
  return false;
}

bool driver_link_set_full_duplex(const bool flag) {
  LWIP_UNUSED_ARG(flag);
  return false;
}

bool driver_link_is_crossover(void) {
  return false;
}

err_t driver_output(struct pbuf *const p) {
  LWIP_UNUSED_ARG(p);
  return ERR_IF;
}

#if QNETHERNET_ENABLE_RAW_FRAME_SUPPORT
bool driver_output_frame(const void *const frame, const size_t len) {
  LWIP_UNUSED_ARG(frame);
  LWIP_UNUSED_ARG(len);
  return false;
}
#endif  // QNETHERNET_ENABLE_RAW_FRAME_SUPPORT

// --------------------------------------------------------------------------
//  MAC Address Filtering
// --------------------------------------------------------------------------

#if !QNETHERNET_ENABLE_PROMISCUOUS_MODE

bool driver_set_incoming_mac_address_allowed(const uint8_t mac[ETH_HWADDR_LEN],
                                             const bool allow) {
  LWIP_UNUSED_ARG(mac);
  LWIP_UNUSED_ARG(allow);
  return false;
}

#endif  // !QNETHERNET_ENABLE_PROMISCUOUS_MODE

#endif  // QNETHERNET_INTERNAL_DRIVER_UNSUPPORTED
