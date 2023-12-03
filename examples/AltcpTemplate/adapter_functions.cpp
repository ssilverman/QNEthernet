// SPDX-FileCopyrightText: (c) 2023 Shawn Silverman <shawn@pobox.com>
// SPDX-License-Identifier: AGPL-3.0-or-later

// adapter_functions.cpp implements the QNEthernet altcp adapter functions.
//
// This file is part of the QNEthernet library.

#if LWIP_ALTCP

#include <cstdint>
#include <cstdio>
#include <functional>

#include <lwip/altcp.h>
#include <lwip/altcp_tcp.h>
#include <lwip/altcp_tls.h>
#include <lwip/apps/altcp_proxyconnect.h>
#include <lwip/ip_addr.h>

extern bool kUseProxy;
extern struct altcp_proxyconnect_config proxyConfig;

// The qnethernet_altcp_get_allocator() function fills in the given
// allocator with an appropriate allocator function and argument,
// using the IP address and port to choose one. If creating the socket
// failed then qnethernet_altcp_free_allocator() is called to free any
// resources that haven't already been freed.
std::function<bool(const ip_addr_t *, uint16_t, altcp_allocator_t &)>
    qnethernet_altcp_get_allocator = [](const ip_addr_t *ipaddr, uint16_t port,
                                        altcp_allocator_t &allocator) {
      printf("[[qnethernet_altcp_get_allocator(%s, %u): %s]]\r\n",
             (ipaddr == nullptr) ? "(null)" : ipaddr_ntoa(ipaddr), port,
             (ipaddr == nullptr) ? "Listen" : "Connect");

      if (kUseProxy) {
        if (ipaddr != nullptr) {
          printf("[[Using proxy: %s:%u]]\r\n",
                 ipaddr_ntoa(&proxyConfig.proxy_addr), proxyConfig.proxy_port);
          allocator.alloc = &altcp_proxyconnect_alloc;
          allocator.arg   = &proxyConfig;
        } else {
          allocator.alloc = &altcp_tcp_alloc;
          allocator.arg = nullptr;
        }
      } else {
        switch (port) {
          case 80:
            // We could also do this in the default case; assigning
            // the regular TCP allocator for only port 80 is just
            // for illustration
            allocator.alloc = &altcp_tcp_alloc;
            allocator.arg   = nullptr;
            break;
#if LWIP_ALTCP_TLS
          // Remove or comment out this case if there's no TLS implementation
          // that can assign a config
          case 443:
            allocator.alloc = &altcp_tls_alloc;
            allocator.arg   = get_altcp_tls_config();  // TBD by you, the user
            break;
#endif  // LWIP_ALTCP_TLS
          default:
            // Returning false here is just to illustrate that
            // nothing's been filled in; we could also just assign the
            // regular TCP allocator
            return false;
        }
      }
      return true;
    };

// The qnethernet_altcp_free_allocator() function frees any resources
// allocated with qnethernet_altcp_get_allocator() if they haven't already
// been freed. It is up to the implementation to decide if a resource
// has already been freed or not.
std::function<void(const altcp_allocator_t &)> qnethernet_altcp_free_allocator =
    [](const altcp_allocator_t &allocator) {
      printf("[[qnethernet_altcp_free_allocator()]]\r\n");
      // For the proxyConfig and for altcp_tcp_alloc,
      // there's nothing to free
#if LWIP_ALTCP_TLS
      if (allocator.alloc == &altcp_tls_alloc) {
        struct altcp_tls_config *config =
            (struct altcp_tls_config *)allocator.arg;
        altcp_tls_free_config(config);  // <-- Example without can-free check
            // Implementation MUST NOT free if already freed
      }
#endif  // LWIP_ALTCP_TLS
    };

#endif  // LWIP_ALTCP
