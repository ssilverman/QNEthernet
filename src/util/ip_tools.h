// SPDX-FileCopyrightText: (c) 2022 Shawn Silverman <shawn@pobox.com>
// SPDX-License-Identifier: MIT

// ip_tools.h defines some utilities for working with IP addresses.
// This file is part of the QNEthernet library.

#ifndef QNETHERNET_UTIL_IPTOOLS_H_
#define QNETHERNET_UTIL_IPTOOLS_H_

// C++ includes
#include <cstdint>

#include <IPAddress.h>

#include "lwip/ip_addr.h"

namespace qindesign {
namespace network {

// Gets the 32-bit IPv4 address from the given ip_addr_t. This takes into
// consideration IPv4-mapped IPv6 addresses. This will return zero for
// non-IPv4-mapped addresses.
uint32_t ip_addr_get_ip4_uint32(const ip_addr_t *ip);

// Missing IPAddress operators
bool operator==(const IPAddress &a, const IPAddress &b);
bool operator!=(const IPAddress &a, const IPAddress &b);

// Gets the 32-bit address from the given const IPAddress.
uint32_t get_uint32(const IPAddress &ip);

}  // namespace network
}  // namespace qindesign

#endif  // QNETHERNET_UTIL_IPTOOLS_H_
