// SPDX-FileCopyrightText: (c) 2026 Shawn Silverman <shawn@pobox.com>
// SPDX-License-Identifier: AGPL-3.0-or-later

// macro_funcs.h defines macro-like functions.
// This file is part of the QNEthernet library.

#pragma once

#include <stddef.h>
#include <stdint.h>

#include "qnethernet/compat/c11_compat.h"

#ifdef __cplusplus
extern "C" {
#endif  // __cplusplus

// #define CLRSET(reg, clear, set) ((reg) = ((reg) & ~(clear)) | (set))

// Clears some bits in a 32-bit register and then sets others.
ATTRIBUTE_ALWAYS_INLINE
static inline void clearAndSet32(volatile uint32_t* const reg,
                                 const uint32_t clear, const uint32_t set) {
  *reg = (*reg & ~clear) | set;
}

/*
// See: https://gcc.gnu.org/onlinedocs/gcc/Typeof.html
#define min(a, b) __extension__ ({   \
  typeof(a) _a = (a);  \
  typeof(b) _b = (b);  \
  (_a < _b) ? _a : _b; \
})
*/

// Returns the minimum of two size_t variables.
ATTRIBUTE_NODISCARD ATTRIBUTE_ALWAYS_INLINE
static inline size_t min_size(size_t a, size_t b) {
  return (a < b) ? a : b;
}

#ifdef __cplusplus
}  // extern "C"
#endif  // __cplusplus
