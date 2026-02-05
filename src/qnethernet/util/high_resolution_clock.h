// SPDX-FileCopyrightText: (c) 2026 Shawn Silverman <shawn@pobox.com>
// SPDX-License-Identifier: AGPL-3.0-or-later

// steady_clock_ms.h implements something similar to std::chrono::steady_clock,
// but backed by the ARM DWT_CYCCNT cycle counter. It properly handles 32-bit
// wraparound, as long as it's polled more frequently than the
// wraparound period.
//
// It conforms to the Clock C++ named requirement.
// See: https://www.cppreference.com/w/cpp/named_req/Clock.html
//
// This file is part of the QNEthernet library.

#pragma once

#ifndef __arm__
#error "Only supported on an ARM processor"
#endif  // !__arm__

#include <chrono>
#include <cstdint>
#include <ratio>

namespace qindesign {
namespace network {
namespace util {

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

extern "C" {
void qnethernet_hal_disable_interrupts();
void qnethernet_hal_enable_interrupts();
}  // extern "C"

// This was used as guidance:
// https://github.com/luni64/TeensyHelpers/tree/master/src/teensy_clock

// high_resolution_clock implements a std::chrono wrapper for ARM's DWT_CYCCNT
// cycle counter and properly handles 32-bit wraparound. The value can be cast
// to 32-bits to get the same results as reading the raw cycle counter would
// give you.
//
// The poll() function must be called at least as often as the wraparound
// period, otherwise it's best just to use this clock to calculate short
// differences that are smaller than that period. Note that now() calls poll().
//
// The wraparound period is 2^32/F_CPU, about 7.1 seconds at 600MHz. This value
// can be retrieved from the wraparoundPeriod() function.
//
// It conforms to the Clock C++ named requirement.
// See: https://www.cppreference.com/w/cpp/named_req/Clock.html
class high_resolution_clock {
 public:
  using rep        = int64_t;
  using period     = std::ratio<1, F_CPU>;
  using duration   = std::chrono::duration<rep, period>;
  using time_point = std::chrono::time_point<high_resolution_clock>;

  static constexpr bool is_steady = true;

  high_resolution_clock() = delete;

  // Initializes the cycle counter and returns whether it's supported. This uses
  // heuristics and isn't guaranteed to work for all cases.
  static bool init() {
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

  // Gets the wraparound period in seconds.
  static constexpr double wraparoundPeriod() {
    return std::chrono::duration_cast<std::chrono::duration<double>>(
               duration{(rep{1} << 32)})
        .count();
  }

  // Polls the counter, handling wraparound. This must be called at least as
  // often as the wraparound period, 2^32/F_CPU seconds, to avoid missing
  // rollovers. Note that now() calls this function.
  static rep poll() {
    qnethernet_hal_disable_interrupts();
    const uint32_t low = *kDWT_CYCCNT;
    if (low < prevLow) {
      // Roll over
      ++high;
    }
    prevLow = low;
    const rep t = (rep{high} << 32) | low;
    qnethernet_hal_enable_interrupts();

    return t;
  }

  // Returns the current time.
  static time_point now() {
    return time_point{duration{poll()}};
  }

 private:
  // Tracking wraparound depends on now() being called at least as
  // often as the wraparound period
  // Note: In C++17 or later, we could also specify these as 'inline'
  //       and initialize them here
  static uint32_t prevLow;
  static uint32_t high;  // Opting for 32-bit instead of 64-bit for space
};

}  // namespace util
}  // namespace network
}  // namespace qindesign
