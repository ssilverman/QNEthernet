// SPDX-FileCopyrightText: (c) 2022-2026 Shawn Silverman <shawn@pobox.com>
// SPDX-License-Identifier: AGPL-3.0-or-later

// ip_tools.c implements the functions defined in ip_tools.h.
// This file is part of the QNEthernet library.

#include "qnethernet/util/ip_tools.h"

#include "lwip/def.h"
#include "lwip/ip4_addr.h"
#include "lwip/opt.h"

namespace qindesign {
namespace network {
namespace util {

#if LWIP_IPV4

uint32_t ip_addr_get_ip4_uint32(const ip_addr_t* const ip) {
  if (IP_IS_V4(ip)) {
    return ip4_addr_get_u32(ip_2_ip4(ip));
#if LWIP_IPV6
  } else if (IP_IS_V6(ip)) {
    if (ip6_addr_isipv4mappedipv6(ip_2_ip6(ip))) {
      return ip_2_ip6(ip)->addr[3];
    }
#endif  // LWIP_IPV6
  }
  return IPADDR_ANY;
}

#endif  // LWIP_IPV4

bool isBroadcast(const uint32_t ip, const uint32_t localIP,
                 const uint32_t mask) {
  if (ip == IPADDR_ANY ||  // Non-standard broadcast
      ip == IPADDR_NONE) {
    return true;
  }

  // It's a broadcast address if not a matching IP, the IP's are on
  // the same subnet (network prefix matches), and the host identifier
  // bits are all set
  // See: ip4_addr_isbroadcast_u32(addr, netif) in lwip/lpv4/ip4_addr.c
  return (localIP != ip) && ((localIP & mask) == (ip & mask)) &&
         ((ip & ~mask) == (IPADDR_BROADCAST & ~mask));
}

}  // namespace util
}  // namespace network
}  // namespace qindesign
