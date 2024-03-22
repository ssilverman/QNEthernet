// SPDX-FileCopyrightText: (c) 2023-2024 Shawn Silverman <shawn@pobox.com>
// SPDX-License-Identifier: AGPL-3.0-or-later

// mbedtls_entropy.c defines the entropy generation function for Mbed TLS.
//
// This file is part of the QNEthernet library.

#include <lwip/opt.h>

#if LWIP_ALTCP && LWIP_ALTCP_TLS

#include <lwip/apps/altcp_tls_mbedtls_opts.h>

#if LWIP_ALTCP_TLS_MBEDTLS

#include <stddef.h>

#if (defined(TEENSYDUINO) && defined(__IMXRT1062__)) && \
    !QNETHERNET_USE_ENTROPY_LIB

#include <security/entropy.h>

int mbedtls_hardware_poll([[maybe_unused]] void *data,
                          unsigned char *output, size_t len, size_t *olen) {
  size_t out = trng_data(output, len);
  if (olen != NULL) {
    *olen = out;
  }
  return 0;
}

#else

#include <lwip/arch.h>

int mbedtls_hardware_poll([[maybe_unused]] void *data,
                          unsigned char *output, size_t len, size_t *olen) {
  for (size_t i = len; i-- > 0; ) {
    *(output++) = LWIP_RAND();
  }
  if (olen != NULL) {
    *olen = len;
  }
  return 0;
}

#endif  // Which implementation

#endif  // LWIP_ALTCP_TLS_MBEDTLS
#endif  // LWIP_ALTCP && LWIP_ALTCP_TLS
