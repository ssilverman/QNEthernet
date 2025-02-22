// SPDX-FileCopyrightText: (c) 2025 Shawn Silverman <shawn@pobox.com>
// SPDX-License-Identifier: AGPL-3.0-or-later

// siphash.h defines functions for computing a SipHash.
// This file is part of the QNEthernet library.

#pragma once

#include <stddef.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif  // __cplusplus

// Calculates a SipHash-c-d-64. The key is expected to contain 16 bytes.
uint64_t siphash(size_t c, size_t d,
                 const void *key,
                 const void *msg, size_t len);

#ifdef __cplusplus
}  // extern "C"
#endif  // __cplusplus
