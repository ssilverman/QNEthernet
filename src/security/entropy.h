// SPDX-FileCopyrightText: (c) 2023 Shawn Silverman <shawn@pobox.com>
// SPDX-License-Identifier: MIT

// entropy.h defines functions for the TRNG (True Random Number Generator).
// This file is part of the QNEthernet library.

#ifndef QNETHERNET_SECURITY_ENTROPY_H_
#define QNETHERNET_SECURITY_ENTROPY_H_

#if defined(__IMXRT1062__)
#if !defined(QNETHERNET_USE_ENTROPY_LIB)

#ifdef __cplusplus
extern "C" {
#endif  // __cplusplus

// C includes
#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

// Initializes the TRNG. This always starts by enabling the clock.
void trng_init();

// Uninitializes the TRNG. The last thing this does is disable the clock.
void trng_deinit();

// Returns whether the TRNG has been started. This checks the clock.
bool trng_is_started();

// Returns the number of bytes available in the entropy pool without having to
// restart entropy generation. Once the pool is empty, entropy generation will
// be restarted automatically.
size_t trng_available();

// Fills data from the entropy pool and keeps refreshing the pool until the
// requested size has been reached or a failure was encountered. This returns
// the number of bytes filled. If there was an entropy generation error then
// this will return a value less than 'size'.
size_t trng_data(uint8_t *data, size_t size);

// Returns a random 4-byte number from the entropy generator. If there was any
// entropy generation error then errno will be set to EAGAIN.
uint32_t entropy_random();

// Returns a random number in the range [0, range). This uses an unbiased
// algorithm. If there was any entropy generation error then errno will be set
// to EAGAIN and this will return zero. If 'range' is zero then errno will be
// set to EDOM.
//
// See: http://www.adammil.net/blog/v134_Efficiently_generating_random_numbers_without_bias.html
// See: https://lemire.me/blog/2019/09/28/doubling-the-speed-of-stduniform_int_distribution-in-the-gnu-c-library/
uint32_t entropy_random_range(uint32_t range);

#ifdef __cplusplus
}  // extern "C"
#endif  // __cplusplus

#endif  // !QNETHERNET_USE_ENTROPY_LIB
#endif  // __IMXRT1062__

#endif  // QNETHERNET_SECURITY_ENTROPY_H_
