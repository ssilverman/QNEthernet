// SPDX-FileCopyrightText: (c) 2023 Shawn Silverman <shawn@pobox.com>
// SPDX-License-Identifier: AGPL-3.0-or-later

// allocators.cpp defines the allocator functions for altcp integration
// with QNEthernet.
//
// This file is part of the QNEthernet library.

#include <cstdint>
#include <cstdio>
#include <functional>

#include <lwip/altcp.h>
#include <lwip/altcp_tcp.h>
#include <lwip/altcp_tls.h>
#include <lwip/ip_addr.h>
#include <lwip/opt.h>

#if LWIP_ALTCP

// The qnethernet_get_allocator() function fills in the given
// allocator with an appropriate allocator function and argument,
// using the IP address and port to choose one. If creating the socket
// failed then qnethernet_free_allocator() is called to free any
// resources that haven't already been freed.
std::function<bool(const ip_addr_t *, uint16_t, altcp_allocator_t *)>
    qnethernet_get_allocator = [](const ip_addr_t *ipaddr, uint16_t port,
                                  altcp_allocator_t *allocator) {
      printf("[[qnethernet_get_allocator(%s, %u): %s]]\r\n",
             ipaddr_ntoa(ipaddr), port,
             (ipaddr == NULL) ? "Listen" : "Connect");

      switch (port) {
        case 80:
          allocator->alloc = &altcp_tcp_alloc;
          allocator->arg   = nullptr;
          break;

#if LWIP_ALTCP_TLS && LWIP_ALTCP_TLS_MBEDTLS
        case 443:
          allocator->alloc = &altcp_tls_alloc;
          if (ipaddr == nullptr) {
            printf("qnethernet_get_allocator: creating server config...\r\n");
            // Create with no certificates; just for illustration
            allocator->arg = altcp_tls_create_config_server(0);
          } else {
            printf("qnethernet_get_allocator: creating client config...\r\n");
            // Create with no certificates; just for illustration
            allocator->arg = altcp_tls_create_config_client(nullptr, 0);
          }
          if (allocator->arg == nullptr) {
            printf("qnethernet_get_allocator: config is NULL\r\n");
            return false;
          }
          break;
#endif  // LWIP_ALTCP_TLS && LWIP_ALTCP_TLS_MBEDTLS

        default:
          // No allocator, the connection will fail
          break;
      }
      return true;
    };

// The qnethernet_free_allocator() function frees any resources
// allocated with qnethernet_get_allocator() if they haven't already
// been freed. It is up to the implementation to decide if a resource
// has already been freed or not.
std::function<void(const altcp_allocator_t *)> qnethernet_free_allocator =
    [](const altcp_allocator_t *allocator) {
      printf("[[qnethernet_free_allocator()]]\r\n");
      // For altcp_tcp_alloc, there's nothing to free
#if LWIP_ALTCP_TLS && LWIP_ALTCP_TLS_MBEDTLS
      if (allocator->alloc == &altcp_tls_alloc) {
        struct altcp_tls_config *config =
            (struct altcp_tls_config *)allocator->arg;
        if (config != nullptr) {
          altcp_tls_free_config(config);  // <-- Example without can-free check
              // Implementation MUST NOT free if already freed
        } else {
          printf("qnethernet_free_allocator: config is NULL\r\n");
        }
      }
#endif  // LWIP_ALTCP_TLS && LWIP_ALTCP_TLS_MBEDTLS
    };

#endif  // LWIP_ALTCP
