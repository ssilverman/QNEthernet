// SPDX-FileCopyrightText: (c) 2026 Shawn Silverman <shawn@pobox.com>
// SPDX-License-Identifier: AGPL-3.0-or-later

// chrono_clocks.cpp implements the "inline-like" parts of chrono_clocks.h. This
// is only here because 'inline' members are only supported in C++17 or later.
// This file is part of the QNEthernet library.

#include "qnethernet/util/chrono_clocks.h"

namespace qindesign {
namespace network {
namespace util {

#ifdef F_CPU

// --------------------------------------------------------------------------
//  arm_high_resolution_clock -- Only Potentially Supported on ARM
// --------------------------------------------------------------------------

#ifdef __arm__

// DWT Control Register
static const auto kDWT_CTRL   = reinterpret_cast<volatile uint32_t*>(0xE0001000);
// DWT Cycle Count Register
static const auto kDWT_CYCCNT = reinterpret_cast<volatile uint32_t*>(0xE0001004);
// Debug Exeption and Monitor Control Register
static const auto kDEMCR      = reinterpret_cast<volatile uint32_t*>(0xE000EDFC);

// Values
static constexpr auto kDEMCR_TRCENA       = uint32_t{1 << 24};
static constexpr auto kDWT_CTRL_CYCCNTENA = uint32_t{1 <<  0};
static constexpr auto kDWT_CTRL_NOCYCCNT  = uint32_t{1 << 25};

uint32_t arm_high_resolution_clock_count() {
  return *kDWT_CYCCNT;
}

bool arm_high_resolution_clock::init() {
  // First enable DWT and check
  if ((*kDEMCR & kDEMCR_TRCENA) == 0) {
    *kDEMCR |= kDEMCR_TRCENA;

    // Check that it was enabled
    if ((*kDEMCR & kDEMCR_TRCENA) == 0) {
      return false;
    }
  }

  // Next, check the obvious feature presence
  if ((*kDWT_CTRL & kDWT_CTRL_NOCYCCNT) != 0) {
    return false;
  }

  // Next, check if the cycle count is enabled
  if ((*kDWT_CTRL & kDWT_CTRL_CYCCNTENA) == 0) {
    *kDWT_CTRL |= kDWT_CTRL_CYCCNTENA;

    // Check that it was enabled
    if ((*kDWT_CTRL & kDWT_CTRL_CYCCNTENA) == 0) {
      return false;
    }
  }

  return true;
}
#else

uint32_t arm_high_resolution_clock_count() {
  return 0;
}

bool arm_high_resolution_clock::init() {
  return false;
}

#endif  // __arm__

#endif  // F_CPU

}  // namespace util
}  // namespace network
}  // namespace qindesign
