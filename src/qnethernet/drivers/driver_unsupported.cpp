// SPDX-FileCopyrightText: (c) 2021-2026 Shawn Silverman <shawn@pobox.com>
// SPDX-License-Identifier: AGPL-3.0-or-later

// driver_unsupported.cpp contains the unsupported Ethernet interface implementation.
// This file is part of the QNEthernet library.

#include "qnethernet/lwip_driver.h"

#include "lwip/err.h"
#include "qnethernet/platforms/pgmspace.h"

#if defined(QNETHERNET_INTERNAL_DRIVER_UNSUPPORTED)

namespace qindesign {
namespace network {
namespace driver {

FLASHMEM void get_capabilities(DriverCapabilities* const dc) {
  dc->isMACSettable                = false;
  dc->isLinkStateDetectable        = false;
  dc->isLinkSpeedDetectable        = false;
  dc->isLinkSpeedSettable          = false;
  dc->isLinkFullDuplexDetectable   = false;
  dc->isLinkFullDuplexSettable     = false;
  dc->isAutoNegotiationSettable    = false;
  dc->isLinkCrossoverDetectable    = false;
  dc->isAutoNegotiationRestartable = false;
  dc->isPHYResettable              = false;
}

bool is_unknown() {
  return false;
}

extern "C" {
void qnethernet_hal_get_system_mac_address(uint8_t mac[ETH_HWADDR_LEN]);
}  // extern "C"

void get_system_mac(uint8_t mac[ETH_HWADDR_LEN]) {
  qnethernet_hal_get_system_mac_address(mac);
}

bool get_mac(uint8_t mac[ETH_HWADDR_LEN]) {
  get_system_mac(mac);
  return true;
}

bool set_mac(const uint8_t mac[ETH_HWADDR_LEN]) {
  (void)mac;

  return false;
}

bool has_hardware() {
  return false;
}

void set_chip_select_pin(const int pin) {
  (void) pin;
}

bool init() {
  return false;
}

void deinit() {
}

struct pbuf* proc_input(struct netif* const netif, const int counter) {
  (void)netif;
  (void)counter;

  return NULL;
}

void poll(struct netif* const netif) {
  (void)netif;
}

void get_link_info(LinkInfo* const li) {
  (void)li;
}

bool set_link(const LinkSettings* const ls) {
  (void)ls;

  return false;
}

err_t output(struct pbuf* const p) {
  (void)p;

  return ERR_IF;
}

#if QNETHERNET_ENABLE_RAW_FRAME_SUPPORT
bool output_frame(const void* const frame, const size_t len) {
  (void)frame;
  (void)len;

  return false;
}
#endif  // QNETHERNET_ENABLE_RAW_FRAME_SUPPORT

// --------------------------------------------------------------------------
//  MAC Address Filtering
// --------------------------------------------------------------------------

#if !QNETHERNET_ENABLE_PROMISCUOUS_MODE

bool set_incoming_mac_address_allowed(const uint8_t mac[ETH_HWADDR_LEN],
                                             const bool allow) {
  (void)mac;
  (void)allow;

  return false;
}

#endif  // !QNETHERNET_ENABLE_PROMISCUOUS_MODE

// --------------------------------------------------------------------------
//  Notifications from Upper Layers
// --------------------------------------------------------------------------

void notify_manual_link_state(const bool flag) {
  (void)flag;
}

// --------------------------------------------------------------------------
//  Link Functions
// --------------------------------------------------------------------------

void restart_auto_negotiation() {
}

void reset_phy() {
}

}  // namespace driver
}  // namespace network
}  // namespace qindesign

#endif  // QNETHERNET_INTERNAL_DRIVER_UNSUPPORTED
