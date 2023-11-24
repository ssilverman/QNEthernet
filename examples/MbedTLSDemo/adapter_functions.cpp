// SPDX-FileCopyrightText: (c) 2023 Shawn Silverman <shawn@pobox.com>
// SPDX-License-Identifier: AGPL-3.0-or-later

// adapter_functions.cpp implements the altcp_tls_adapter functions.
//
// This file is part of the QNEthernet library.

#include <lwip/opt.h>

#if LWIP_ALTCP && LWIP_ALTCP_TLS

#include <lwip/apps/altcp_tls_mbedtls_opts.h>

#if LWIP_ALTCP_TLS_MBEDTLS

#include <cinttypes>
#include <cstddef>
#include <cstdint>
#include <cstdio>
#include <functional>

#include <lwip/ip_addr.h>

// Determines if a connection should use TLS.
std::function<bool(const ip_addr_t *, uint16_t)> qnethernet_altcp_is_tls =
    [](const ip_addr_t *ipaddr, uint16_t port) {
      // Given the IP address and port, determine if the connection
      // needs to use TLS
      printf("[[qnethernet_altcp_is_tls(%s, %" PRIu16 "): %s]]\r\n",
             (ipaddr == nullptr) ? "(null)" : ipaddr_ntoa(ipaddr), port,
             (ipaddr == nullptr) ? "Listen" : "Connect");

      if (port == 443) {
        if (ipaddr == nullptr) {
          printf("qnethernet_altcp_is_tls: creating server config...\r\n");
        } else {
          printf("qnethernet_altcp_is_tls: creating client config...\r\n");
        }
        return true;
      }
      return false;
    };

// Gets the client certificate data.
std::function<void(const ip_addr_t &, uint16_t, const uint8_t *&, size_t &)>
    qnethernet_altcp_tls_client_cert =
        [](const ip_addr_t &ipaddr, uint16_t port,
           const uint8_t *&cert, size_t &cert_len) {
          printf("[[qnethernet_altcp_tls_client_cert(%s, %" PRIu16 ")]]"
                 " No certificate\r\n",
                 ipaddr_ntoa(&ipaddr), port);
          // To set certificate data, set:
          // cert = pointer to certificate data
          // cert_len = the certificate length
          // You can optionally use the IP address and port to
          // determine which data to use
        };

// Gets the server certificate count.
std::function<uint8_t(uint16_t)> qnethernet_altcp_tls_server_cert_count =
    [](uint16_t port) {
      // The number of times qnethernet_altcp_tls_server_cert will
      // get called
      return 0;
    };

// Gets the server certificate data.
std::function<void(uint16_t, uint8_t,
                   const uint8_t *&, size_t &,
                   const uint8_t *&, size_t &,
                   const uint8_t *&, size_t &)>
    qnethernet_altcp_tls_server_cert =
        [](uint16_t port, uint8_t index,
           const uint8_t *&privkey,      size_t &privkey_len,
           const uint8_t *&privkey_pass, size_t &privkey_pass_len,
           const uint8_t *&cert,         size_t &cert_len) {
          printf("[[qnethernet_altcp_tls_server_cert(port %" PRIu16 ","
                 " index %" PRIu8 ")]]\r\n",
                 port, index);
          // To set certificate data, simply set the values;
          // they're all references
          // You can optionally use the port and certificate index to
          // determine which data to use
        };

#endif  // LWIP_ALTCP_TLS_MBEDTLS
#endif  // LWIP_ALTCP && LWIP_ALTCP_TLS
