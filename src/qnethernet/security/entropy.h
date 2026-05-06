// SPDX-FileCopyrightText: (c) 2023-2026 Shawn Silverman <shawn@pobox.com>
// SPDX-License-Identifier: AGPL-3.0-or-later

// entropy.h defines functions for the TRNG (True Random Number Generator).
// This file is part of the QNEthernet library.

#pragma once

#include "qnethernet_opts.h"

#if defined(TEENSYDUINO) && defined(__IMXRT1062__)
#if !QNETHERNET_USE_ENTROPY_LIB

// C++ includes
#include <cstddef>
#include <cstdint>

namespace qindesign {
namespace security {

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
size_t trng_data(void* data, size_t size);

// Generates a random 4-byte number and returns whether successful. The result
// is stored in '*out'. If there was any entropy generation error then errno
// will be set to EIO and this returns false.
bool entropy_random(uint32_t* out);

// Generates a random number in the range [0, range) and returns whether
// successful. This uses an unbiased algorithm. The result is stored in '*out'.
//
// This returns false if:
// * There was any entropy generation error &mdash; errno will be set to EIO
// * 'range' is zero &mdash; errno will be set to EDOM
//
// See:
// * http://www.adammil.net/blog/v134_Efficiently_generating_random_numbers_without_bias.html
// * https://lemire.me/blog/2019/09/28/doubling-the-speed-of-stduniform_int_distribution-in-the-gnu-c-library/
bool entropy_random_range(uint32_t range, uint32_t* out);

}  // namespace security
}  // namespace qindesign

#endif  // !QNETHERNET_USE_ENTROPY_LIB
#endif  // defined(TEENSYDUINO) && defined(__IMXRT1062__)
