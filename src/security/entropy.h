// SPDX-FileCopyrightText: (c) 2023 Shawn Silverman <shawn@pobox.com>
// SPDX-License-Identifier: MIT

// entropy.h defines functions for the TRNG (True Random Number Generator).
// This file is part of the QNEthernet library.

#ifndef QNETHERNET_USE_ENTROPY_LIB

#ifndef QNETHERNET_UTIL_ENTROPY_H_
#define QNETHERNET_UTIL_ENTROPY_H_

#ifdef __cplusplus
extern "C" {
#endif  // __cplusplus

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

// Initializes the TRNG. This always starts by enabling the clock.
void trng_init();

// Uninitializes the TRNG. The last thing this does is disable the clock.
void trng_deinit();

// Returns whether the TRNG has been started. This checks the clock.
bool trng_is_started();

// Fills data from the entropy pool. This returns whether successful.
bool trng_data(uint8_t *data, size_t size);

#ifdef __cplusplus
}  // extern "C"
#endif  // __cplusplus

#endif  // QNETHERNET_UTIL_ENTROPY_H_

#endif  // !QNETHERNET_USE_ENTROPY_LIB
