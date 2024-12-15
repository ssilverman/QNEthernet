// SPDX-FileCopyrightText: (c) 2024 Shawn Silverman <shawn@pobox.com>
// SPDX-License-Identifier: AGPL-3.0-or-later

// mbedtls_funcs.c implements functions required for MbedTLS.

// C includes
#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

#include <lwip/arch.h>
#include <mbedtls/ctr_drbg.h>
#include <mbedtls/entropy.h>
#include <mbedtls/ssl.h>

// Forward declarations
size_t qnethernet_hal_fill_rand(uint8_t *buf, size_t len);
uint32_t qnethernet_hal_millis(void);

// Global random state
static bool s_randInit = false;
static mbedtls_ctr_drbg_context s_drbg;
static mbedtls_entropy_context s_entropy;

// Initializes the random context and sets it up for the config configuration.
// The config object may be NULL.
//
// This uses a single global context.
void qnethernet_mbedtls_init_rand(mbedtls_ssl_config *conf) {
  if (!s_randInit) {
    mbedtls_ctr_drbg_init(&s_drbg);
    mbedtls_entropy_init(&s_entropy);

    // Build a nonce
    uint8_t nonce[128];
    uint8_t *pNonce = nonce;
    size_t sizeRem = sizeof(nonce);
    while (sizeRem != 0) {
      size_t size = qnethernet_hal_fill_rand(pNonce, sizeRem);
      sizeRem -= size;
      pNonce += size;
    }

    mbedtls_ctr_drbg_seed(&s_drbg, mbedtls_entropy_func, &s_entropy,
                          nonce, sizeof(nonce));

    s_randInit = true;
  }

  if (conf != NULL) {
    mbedtls_ssl_conf_rng(conf, mbedtls_ctr_drbg_random, &s_drbg);
  }
}

// Gets entropy for MbedTLS.
int mbedtls_hardware_poll(void *const data,
                          unsigned char *const output, const size_t len,
                          size_t *const olen) {
  LWIP_UNUSED_ARG(data);

  size_t filled = qnethernet_hal_fill_rand(output, len);
  if (olen != NULL) {
    *olen = filled;
  }
  return 0;  // Success
}

mbedtls_ms_time_t mbedtls_ms_time(void) {
  static int64_t top = 0;
  static uint32_t last = 0;
  uint32_t t = qnethernet_hal_millis();
  if (t < last) {
    top += (INT64_C(1) << 32);
  }
  last = t;
  return top | t;
}
