// SPDX-FileCopyrightText: (c) 2023-2025 Shawn Silverman <shawn@pobox.com>
// SPDX-License-Identifier: AGPL-3.0-or-later

// mbedtls_entropy.c defines the entropy generation function for Mbed TLS.
//
// This file is part of the QNEthernet library.

#include <lwip/opt.h>

#if LWIP_ALTCP && LWIP_ALTCP_TLS

#include <lwip/apps/altcp_tls_mbedtls_opts.h>

#if LWIP_ALTCP_TLS_MBEDTLS

// C includes
#include <stddef.h>

size_t qnethernet_hal_fill_entropy(void *buf, size_t size);

// Gets entropy for MbedTLS.
int mbedtls_hardware_poll(void *data,
                          unsigned char *output, size_t len,
                          size_t *olen) {
  (void)data;

  size_t filled = qnethernet_hal_fill_entropy(output, len);
  if (olen != NULL) {
    *olen = filled;
  }
  return 0;  // Success
}

#endif  // LWIP_ALTCP_TLS_MBEDTLS
#endif  // LWIP_ALTCP && LWIP_ALTCP_TLS
