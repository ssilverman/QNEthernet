// SPDX-FileCopyrightText: (c) 2026 Shawn Silverman <shawn@pobox.com>
// SPDX-License-Identifier: AGPL-3.0-or-later

// steady_clock_ms.h implements something similar to std::chrono::steady_clock,
// but backed by qnethernet_hal_millis(). It properly handles 32-bit wraparound.
//
// It conforms to the Clock C++ named requirement.
// See: https://www.cppreference.com/w/cpp/named_req/Clock.html
//
// This file is part of the QNEthernet library.

#pragma once

#include <chrono>
#include <cstdint>
#include <ratio>

namespace qindesign {
namespace network {
namespace util {

extern "C" uint32_t qnethernet_hal_millis();

// See also: https://github.com/luni64/TeensyHelpers/tree/master/src/teensy_clock

// steady_clock_ms implements a std::chrono wrapper for qnethernet_hal_millis()
// and properly handles 32-bit wraparound. The value can be cast to 32-bits to
// get the same results as millis() would give you.
//
// It conforms to the Clock C++ named requirement.
// See: https://www.cppreference.com/w/cpp/named_req/Clock.html
class steady_clock_ms {
 public:
  using rep        = int64_t;
  using period     = std::milli;
  using duration   = std::chrono::duration<rep, period>;
  using time_point = std::chrono::time_point<steady_clock_ms>;

  static constexpr bool is_steady = true;

  steady_clock_ms() = delete;

  // Returns the current time.
  static time_point now() {
    const uint32_t low = qnethernet_hal_millis();
    if (low < prevLow) {
      // Roll over
      ++high;
    }
    prevLow = low;
    return time_point{duration{(rep{high} << 32) | low}};
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
