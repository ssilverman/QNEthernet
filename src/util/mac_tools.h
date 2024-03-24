// SPDX-FileCopyrightText: (c) 2024 Shawn Silverman <shawn@pobox.com>
// SPDX-License-Identifier: AGPL-3.0-or-later

// mac_tools.h defines some utilities for working with MAC addresses.
// This file is part of the QNEthernet library.

#pragma once

// C includes
#include <stdint.h>

#include "lwip/prot/ethernet.h"

#ifdef __cplusplus
extern "C" {
#endif  // __cplusplus

// Gets the system MAC address. This will either be some platform-specific value
// or a predefined value.
void qnethernet_get_system_mac_address(uint8_t mac[ETH_HWADDR_LEN]);

#ifdef __cplusplus
}  // extern "C"
#endif  // __cplusplus
