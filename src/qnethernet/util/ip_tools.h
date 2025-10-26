// SPDX-FileCopyrightText: (c) 2022,2024 Shawn Silverman <shawn@pobox.com>
// SPDX-License-Identifier: AGPL-3.0-or-later

// ip_tools.h defines some utilities for working with IP addresses.
// This file is part of the QNEthernet library.

#pragma once

// C++ includes
#include <cstdint>

#include <IPAddress.h>

#include "lwip/ip_addr.h"

namespace qindesign {
namespace network {
namespace util {

#if LWIP_IPV4

// Gets the 32-bit IPv4 address from the given ip_addr_t. This takes into
// consideration IPv4-mapped IPv6 addresses. This will return zero for
// non-IPv4-mapped addresses.
uint32_t ip_addr_get_ip4_uint32(const ip_addr_t* ip);

#endif  // LWIP_IPV4

}  // namespace util
}  // namespace network
}  // namespace qindesign
