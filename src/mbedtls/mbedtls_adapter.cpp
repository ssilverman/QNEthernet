// SPDX-FileCopyrightText: (c) 2023 Shawn Silverman <shawn@pobox.com>
// SPDX-License-Identifier: AGPL-3.0-or-later

// mbedtls_adapter.cpp simplifies Mbed TLS integration with QNEthernet's
// altcp approach.
// This file is part of the QNEthernet library.

#include <cstdint>
#include <functional>

#include "lwip/altcp.h"
#include "lwip/altcp_tcp.h"
#include "lwip/altcp_tls.h"
#include "lwip/ip_addr.h"
#include "lwip/opt.h"

#if LWIP_ALTCP && LWIP_ALTCP_TLS && LWIP_ALTCP_TLS_MBEDTLS

// Determines if the connection should use TLS. The IP address will be NULL for
// a server connection.
extern std::function<bool(const ip_addr_t *ip, uint16_t port)>
    qnethernet_mbedtls_is_tls;

// Retrieves the certificate for a client connection. The values are initialized
// to NULL and zero, respectively, before calling this function.
//
// The IP address and port can be used to determine the certificate data,
// if needed.
extern std::function<void(const ip_addr_t &ipaddr, uint16_t port,
                          const uint8_t *&cert, size_t &cert_len)>
    qnethernet_altcp_tls_client_cert;

// Returns the certificate count for a server connection.
extern std::function<uint8_t(uint16_t port)>
    qnethernet_altcp_tls_server_cert_count;

// Retrieves the certificate and private key for a server connection. The values
// are initialized to NULL and zero before calling this function.
//
// This function will be called for each server certificate, a total of N times,
// where N is the value returned by qnethernet_altcp_tls_server_cert_count. The
// 'index' argument will be in the range zero to N-1.
//
// The port and certificate index can be used to determine the certificate data,
// if needed.
extern std::function<void(
    uint16_t port, uint8_t index,
    const uint8_t *&privkey,      size_t &privkey_len,
    const uint8_t *&privkey_pass, size_t &privkey_pass_len,
    const uint8_t *&cert,         size_t &cert_len)>
    qnethernet_altcp_tls_server_cert;

// The qnethernet_altcp_get_allocator() function fills in the given
// allocator with an appropriate allocator function and argument,
// using the IP address and port to choose one. If creating the socket
// failed then qnethernet_altcp_free_allocator() is called to free any
// resources that haven't already been freed.
//
// This returns false if the config could not be created and true otherwise.
std::function<bool(const ip_addr_t *, uint16_t, altcp_allocator_t *)>
    qnethernet_altcp_get_allocator = [](const ip_addr_t *ipaddr, uint16_t port,
                                        altcp_allocator_t *allocator) {
      if (qnethernet_mbedtls_is_tls(ipaddr, port)) {  // TLS
        allocator->alloc = &altcp_tls_alloc;
        if (ipaddr == nullptr) {  // Server
          const uint8_t *privkey;
          size_t privkey_len;
          const uint8_t *privkey_pass;
          size_t privkey_pass_len;
          const uint8_t *cert;
          size_t cert_len;
          uint8_t cert_count = qnethernet_altcp_tls_server_cert_count(port);

          struct altcp_tls_config *config =
              altcp_tls_create_config_server(cert_count);
          if (config == nullptr) {
            return false;
          }
          allocator->arg = config;
          for (uint8_t i = 0; i < cert_count; i++) {
            privkey = nullptr;
            privkey_len = 0;
            privkey_pass = nullptr;
            privkey_pass_len = 0;
            cert = nullptr;
            cert_len = 0;

            qnethernet_altcp_tls_server_cert(port, i,
                                             privkey, privkey_len,
                                             privkey_pass, privkey_pass_len,
                                             cert, cert_len);
            altcp_tls_config_server_add_privkey_cert(
                config,
                privkey, privkey_len,
                privkey_pass, privkey_pass_len,
                cert, cert_len);
          }
          if (cert != nullptr && cert_len > 0) {
            allocator->arg = altcp_tls_create_config_server_privkey_cert(
                privkey, privkey_len,
                privkey_pass, privkey_pass_len,
                cert, cert_len);
          } else {
            allocator->arg = altcp_tls_create_config_server(0);
          }
        } else {  // Client
          const uint8_t *cert = nullptr;
          size_t cert_len = 0;
          qnethernet_altcp_tls_client_cert(*ipaddr, port, cert, cert_len);
          allocator->arg = altcp_tls_create_config_client(cert, cert_len);
        }
        if (allocator->arg == nullptr) {
          return false;
        }
      } else {  // Not TLS
        allocator->alloc = &altcp_tcp_alloc;
        allocator->arg = nullptr;
      }
      return true;
    };

// The qnethernet_altcp_free_allocator() function frees any resources
// allocated with qnethernet_altcp_get_allocator() if they haven't already
// been freed. It is up to the implementation to decide if a resource
// has already been freed or not.
std::function<void(const altcp_allocator_t *)> qnethernet_altcp_free_allocator =
    [](const altcp_allocator_t *allocator) {
      // For altcp_tcp_alloc, there's nothing to free
      if (allocator->alloc == &altcp_tls_alloc) {
        struct altcp_tls_config *config =
            (struct altcp_tls_config *)allocator->arg;
        if (config != nullptr) {
          altcp_tls_free_config(config);
        }
      }
    };

#endif  // LWIP_ALTCP && LWIP_ALTCP_TLS && LWIP_ALTCP_TLS_MBEDTLS
