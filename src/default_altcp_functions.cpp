// SPDX-FileCopyrightText: (c) 2023-2024 Shawn Silverman <shawn@pobox.com>
// SPDX-License-Identifier: AGPL-3.0-or-later

// default_altcp_functions.cpp provides default implementations of QNEthernet's
// altcp interface functions.
// This file is part of the QNEthernet library.

#include "lwip/opt.h"
#include "qnethernet_opts.h"

#if LWIP_ALTCP && QNETHERNET_ENABLE_ALTCP_DEFAULT_FUNCTIONS

#include <cstdint>
#include <functional>

#include "lwip/altcp.h"
#include "lwip/altcp_tcp.h"
#include "lwip/ip_addr.h"

// This implementation uses the TCP allocator and returns true.
[[gnu::weak]]
std::function<bool(const ip_addr_t *, uint16_t, altcp_allocator_t &)>
    qnethernet_altcp_get_allocator = [](const ip_addr_t *ipaddr, uint16_t port,
                                        altcp_allocator_t &allocator) {
      allocator.alloc = &altcp_tcp_alloc;
      allocator.arg   = nullptr;
      return true;
    };

// This implementation does nothing.
[[gnu::weak]]
std::function<void(const altcp_allocator_t &)> qnethernet_altcp_free_allocator =
    [](const altcp_allocator_t &allocator) {};

#endif  // LWIP_ALTCP && QNETHERNET_ENABLE_ALTCP_DEFAULT_FUNCTIONS
