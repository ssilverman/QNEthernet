// SPDX-FileCopyrightText: (c) 2024 Shawn Silverman <shawn@pobox.com>
// SPDX-License-Identifier: AGPL-3.0-or-later

// mbedtls_funcs.h defines application-specific things about the
// MbedTLS functions.

#pragma once

// C includes
#include <stdbool.h>

#include <mbedtls/ssl.h>

#ifdef __cplusplus
extern "C" {
#endif  // __cplusplus

// Initializes the random context and sets it up for the config configuration.
// The config object may be NULL. This will return whether the initialization
// succeeded.
//
// This uses a single global context.
bool qnethernet_mbedtls_init_entropy(mbedtls_ssl_config *conf);

extern int (*const qnethernet_mbedtls_rand_f_rng)(void *, unsigned char *,
                                                  size_t);
extern void *const qnethernet_mbedtls_rand_p_rng;

#ifdef __cplusplus
}  // extern "C"
#endif  // __cplusplus
