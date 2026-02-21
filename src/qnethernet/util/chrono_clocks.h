// SPDX-FileCopyrightText: (c) 2026 Shawn Silverman <shawn@pobox.com>
// SPDX-License-Identifier: AGPL-3.0-or-later

// chrono_clocks.h defines several Clocks for use with std::chrono.
//
// The clocks conform to the Clock C++ named requirement.
// See: https://www.cppreference.com/w/cpp/named_req/Clock.html
//
// This file is part of the QNEthernet library.

#pragma once

#include <chrono>
#include <cstdint>
#include <ratio>
#include <type_traits>

#include "qnethernet/compat/c++11_compat.h"

namespace qindesign {
namespace network {
namespace util {

extern "C" {
void qnethernet_hal_disable_interrupts();
void qnethernet_hal_enable_interrupts();
uint32_t qnethernet_hal_millis();
}  // extern "C"

// This was used for guidance:
// https://github.com/luni64/TeensyHelpers/tree/master/src/teensy_clock

// chrono_steady_clock implements a std::chrono wrapper for TimeFunc().
//
// It properly handles 32-bit wraparound as long as it's polled more frequently
// than the wraparound period. Also, the count can be cast to a 32-bit value to
// get the same result as TimeFunc() would give you.
//
// The poll() function must be called at least as often as the wraparound
// period, otherwise it's best just to use this clock to calculate short
// differences that are smaller than that period. Note that now() calls poll().
//
// It conforms to the Clock C++ named requirement.
// See: https://www.cppreference.com/w/cpp/named_req/Clock.html
template <typename P, uint32_t (*TimeFunc)(), bool (*InitFunc)() = nullptr>
class chrono_steady_clock {
 public:
  using rep        = int64_t;
  using period     = P;
  using duration   = std::chrono::duration<rep, period>;
  using time_point = std::chrono::time_point<chrono_steady_clock>;

  static constexpr bool is_steady = true;

  chrono_steady_clock() = delete;

  // Gets the wraparound period in seconds.
  static constexpr double wraparoundPeriod() {
    return std::chrono::duration_cast<std::chrono::duration<double>>(
               duration{(rep{1} << 32)})
        .count();
  }

  // Initializes the clock and returns whether successful. This may not need to
  // be called, depending on the clock. If InitFunc is NULL then this will
  // return true.
  static bool init() {
    IF_CONSTEXPR (std::is_null_pointer<decltype(InitFunc)>::value) {
      return InitFunc();
    } else {
      return true;
    }
  }

  // Polls the counter, handling wraparound. This must be called at least as
  // often as the wraparound period to avoid missing rollovers. Note that now()
  // calls this function.
  //
  // See: wraparoundPeriod()
  static rep poll() {
    qnethernet_hal_disable_interrupts();
    const uint32_t low = TimeFunc();
    if (low < prevLow) {
      // Roll over
      ++high;
    }
    prevLow = low;
    const auto t = static_cast<rep>((int64_t{high} << 32) | low);
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

// Pre-C++17, need out-of-class definition and initialization
template <typename P, uint32_t (*TimeFunc)(), bool (*InitFunc)()>
uint32_t chrono_steady_clock<P, TimeFunc, InitFunc>::prevLow = 0;
template <typename P, uint32_t (*TimeFunc)(), bool (*InitFunc)()>
uint32_t chrono_steady_clock<P, TimeFunc, InitFunc>::high = 0;

// --------------------------------------------------------------------------
//  steady_clock_ms
// --------------------------------------------------------------------------

// steady_clock_ms implements a std::chrono wrapper for qnethernet_hal_millis().
//
// The wraparound period is 2^32/1000, about 49.7 days.
using steady_clock_ms = chrono_steady_clock<std::milli, &qnethernet_hal_millis>;

#ifdef F_CPU

// --------------------------------------------------------------------------
//  arm_high_resolution_clock
// --------------------------------------------------------------------------

// Returns the current DWT_CYCCNT value.
uint32_t arm_high_resolution_clock_count();

// Initializes the clock.
bool arm_high_resolution_clock_init();

// Note: In order to get this to compile, the F_CPU variable needs to
//       be a compile-time constant

// arm_high_resolution_clock implements a std::chrono wrapper for ARM's
// DWT_CYCCNT cycle counter, on systems that support it. init() should be called
// before use.
//
// The wraparound period is 2^32/F_CPU, about 7.1 seconds at 600MHz.
using arm_high_resolution_clock =
    chrono_steady_clock<std::ratio<1, F_CPU>,
                        &arm_high_resolution_clock_count,
                        &arm_high_resolution_clock_init>;

#endif  // F_CPU

}  // namespace util
}  // namespace network
}  // namespace qindesign
