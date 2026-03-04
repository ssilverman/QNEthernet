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
  using duration   = typename Clock::duration;
  using time_point = typename Clock::time_point;

  elapsedTime() : elapsedTime(duration::zero()) {}

  elapsedTime(duration d) : base_{Clock::now() - d} {}

  // Rule of zero: No declared destructors, copy, or move operations;
  // they will be defaulted

  // Returns this as a duration. This is useful when the object won't
  // automatically convert.
  duration dur() const {
    return static_cast<duration>(*this);
  }

  operator duration() const {
    return Clock::now() - base_;
  }

  // For operator overloading, see also:
  // https://en.cppreference.com/w/cpp/language/operators.html

  elapsedTime& operator=(const duration d) {
    base_ = Clock::now() - d;
    return *this;
  }

  elapsedTime& operator+=(const duration d) {
    base_ -= d;
    return *this;
  }

  elapsedTime& operator-=(const duration d) {
    base_ += d;
    return *this;
  }

  friend elapsedTime operator+(elapsedTime lhs, const duration rhs) {
    lhs += rhs;
    return lhs;
  }

  friend elapsedTime operator-(elapsedTime lhs, const duration rhs) {
    lhs -= rhs;
    return lhs;
  }

  friend bool operator==(const elapsedTime& lhs, const duration& rhs) {
    return static_cast<duration>(lhs) == rhs;
  }

  friend bool operator!=(const elapsedTime& lhs, const duration& rhs) {
    return !(lhs == rhs);
  }

  friend bool operator<(const elapsedTime& lhs, const duration& rhs) {
    return static_cast<duration>(lhs) < rhs;
  }

  friend bool operator>(const elapsedTime& lhs, const duration& rhs) {
    return rhs < lhs;
  }

  friend bool operator<=(const elapsedTime& lhs, const duration& rhs) {
    return !(lhs > rhs);
  }

  friend bool operator>=(const elapsedTime& lhs, const duration& rhs) {
    return !(lhs < rhs);
  }

 private:
  time_point base_;
};

}  // namespace util
}  // namespace network
}  // namespace qindesign
