// SPDX-FileCopyrightText: (c) 2023 Shawn Silverman <shawn@pobox.com>
// SPDX-License-Identifier: AGPL-3.0-or-later

// mbedtls_entropy.c defines the entropy generation function for Mbed TLS.
//
// This file is part of the QNEthernet library.

#include <stddef.h>

#include <lwip/opt.h>
#include <security/entropy.h>

#if LWIP_ALTCP_TLS && LWIP_ALTCP_TLS_MBEDTLS
int mbedtls_hardware_poll(void *data,
                          unsigned char *output, size_t len, size_t *olen) {
  size_t out = trng_data(output, len);
  if (olen != NULL) {
    *olen = out;
  }
  return 0;
}
#endif  // LWIP_ALTCP_TLS && LWIP_ALTCP_TLS_MBEDTLS
