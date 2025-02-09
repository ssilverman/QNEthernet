// SPDX-FileCopyrightText: (c) 2024 Shawn Silverman <shawn@pobox.com>
// SPDX-License-Identifier: AGPL-3.0-or-later

// mbedtls_funcs.c implements functions required for MbedTLS.

#include "mbedtls_funcs.h"

// C includes
#include <stddef.h>
#include <stdint.h>
#include <string.h>

#include <mbedtls/ctr_drbg.h>
#include <mbedtls/entropy.h>

// Forward declarations
size_t qnethernet_hal_fill_entropy(uint8_t *buf, size_t len);
uint32_t qnethernet_hal_millis(void);

// Global random state
static bool s_randInit = false;
static mbedtls_ctr_drbg_context s_ctr_drbg;
static mbedtls_entropy_context s_entropy;

int (*const qnethernet_mbedtls_rand_f_rng)(void *, unsigned char *,
                                           size_t) = mbedtls_ctr_drbg_random;
void *const qnethernet_mbedtls_rand_p_rng = &s_ctr_drbg;

bool qnethernet_mbedtls_init_entropy(mbedtls_ssl_config *conf) {
  if (!s_randInit) {
    mbedtls_ctr_drbg_init(&s_ctr_drbg);
    mbedtls_entropy_init(&s_entropy);

    // Build a nonce
    uint8_t nonce[128];
    uint8_t *pNonce = nonce;
    size_t sizeRem = sizeof(nonce);
    while (sizeRem != 0) {  // TODO: What to do on failure?
      size_t size = qnethernet_hal_fill_entropy(pNonce, sizeRem);
      sizeRem -= size;
      pNonce += size;
    }

    int ret = mbedtls_ctr_drbg_seed(&s_ctr_drbg, mbedtls_entropy_func,
                                    &s_entropy, nonce, sizeof(nonce));
    memset(nonce, 0, sizeof(nonce));  // Clear the nonce after use
    if (ret) {
      return false;
    }

    s_randInit = true;
  }

  if (conf != NULL) {
    mbedtls_ssl_conf_rng(conf, qnethernet_mbedtls_rand_f_rng,
                         qnethernet_mbedtls_rand_p_rng);
  }
  return true;
}

// Gets entropy for MbedTLS.
int mbedtls_hardware_poll(void *const data,
                          unsigned char *const output, const size_t len,
                          size_t *const olen) {
  (void)data;

  size_t filled = qnethernet_hal_fill_entropy(output, len);
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
