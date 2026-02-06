// SPDX-FileCopyrightText: (c) 2026 Shawn Silverman <shawn@pobox.com>
// SPDX-License-Identifier: AGPL-3.0-or-later

// elapsedTime.h implements an elapsed-time class, similar to elapsedMillis
// and elapsedMicros written by Paul Stoffregen. The clock to be used is a
// template parameter and is expected to conform to the Clock C++ named
// requirement. It should also be a "steady" clock; that is, 'is_steady'
// returns true.
// See: https://www.cppreference.com/w/cpp/named_req/Clock.html
// This file is part of the QNEthernet library.

#pragma once

#include <chrono>

namespace qindesign {
namespace network {
namespace util {

template <typename Clock>
class elapsedTime {
 public:
  elapsedTime() : elapsedTime(Clock::duration::zero()) {}

  elapsedTime(Clock::duration d) : base_{Clock::now() - d} {}

  // Rule of zero: No declared destructors, copy, or move operations;
  // they will be defaulted

  operator (Clock::duration)() const {
    return Clock::now() - base_;
  }

  // For operator overloading, see also:
  // https://en.cppreference.com/w/cpp/language/operators.html

  elapsedTime& operator=(const Clock::duration d) {
    base_ = Clock::now() - d;
    return *this;
  }

  elapsedTime& operator+=(const Clock::duration d) {
    base_ -= d;
    return *this;
  }

  elapsedTime& operator-=(const Clock::duration d) {
    base_ += d;
    return *this;
  }

  friend elapsedTime operator+(elapsedTime lhs, const Clock::duration rhs) {
    lhs += rhs;
    return lhs;
  }

  friend elapsedTime operator-(elapsedTime lhs, const Clock::duration rhs) {
    lhs -= rhs;
    return lhs;
  }

 private:
  Clock::time_point base_;
};

}  // namespace util
}  // namespace network
}  // namespace qindesign
