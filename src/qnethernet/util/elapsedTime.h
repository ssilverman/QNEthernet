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
#include <type_traits>

namespace qindesign {
namespace network {
namespace util {

template <typename Clock>
class elapsedTime {
 public:
  using duration   = typename Clock::duration;
  using time_point = typename Clock::time_point;

  elapsedTime() : elapsedTime(duration::zero()) {}

  template <typename R, typename P>
  elapsedTime(std::chrono::duration<R, P> d)
      : base_{Clock::now() - std::chrono::duration_cast<duration>(d)} {}

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

  template <typename R, typename P>
  elapsedTime& operator=(const std::chrono::duration<R, P>& d) {
    base_ = Clock::now() - std::chrono::duration_cast<duration>(d);
    return *this;
  }

  template <typename R, typename P>
  elapsedTime& operator+=(const std::chrono::duration<R, P>& d) {
    base_ -= std::chrono::duration_cast<duration>(d);
    return *this;
  }

  template <typename R, typename P>
  elapsedTime& operator-=(const std::chrono::duration<R, P>& d) {
    base_ += std::chrono::duration_cast<duration>(d);
    return *this;
  }

  template <typename R, typename P>
  friend elapsedTime operator+(elapsedTime lhs,
                               const std::chrono::duration<R, P>& rhs) {
    lhs += std::chrono::duration_cast<duration>(rhs);
    return lhs;
  }

  template <typename R, typename P>
  friend elapsedTime operator-(elapsedTime lhs,
                               std::chrono::duration<R, P>& rhs) {
    lhs -= std::chrono::duration_cast<duration>(rhs);
    return lhs;
  }

  template <typename R, typename P>
  friend bool operator==(const elapsedTime& lhs,
                         const std::chrono::duration<R, P>& rhs) {
    return (static_cast<duration>(lhs) ==
            std::chrono::duration_cast<duration>(rhs));
  }

  template <typename R, typename P>
  friend bool operator==(const std::chrono::duration<R, P>& lhs,
                         const elapsedTime& rhs) {
    // using type = typename std::remove_cv<
    //     typename std::remove_reference<decltype(lhs)>::type>::type;
    return (lhs == std::chrono::duration_cast<std::chrono::duration<R, P>>(
                       static_cast<duration>(rhs)));
  }

  template <typename R, typename P>
  friend bool operator!=(const elapsedTime& lhs,
                         const std::chrono::duration<R, P>& rhs) {
    return !(lhs == rhs);
  }

  template <typename R, typename P>
  friend bool operator!=(const std::chrono::duration<R, P>& lhs,
                         const elapsedTime& rhs) {
    return !(lhs == rhs);
  }

  template <typename R, typename P>
  friend bool operator<(const elapsedTime& lhs,
                        const std::chrono::duration<R, P>& rhs) {
    return static_cast<duration>(lhs) < rhs;
  }

  template <typename R, typename P>
  friend bool operator<(const std::chrono::duration<R, P>& lhs,
                        const elapsedTime& rhs) {
    // using type = typename std::remove_cv<
    //     typename std::remove_reference<decltype(lhs)>::type>::type;
    return lhs < std::chrono::duration_cast<std::chrono::duration<R, P>>(
                     static_cast<duration>(rhs));
  }

  template <typename R, typename P>
  friend bool operator>(const elapsedTime& lhs,
                        const std::chrono::duration<R, P>& rhs) {
    return rhs < lhs;
  }

  template <typename R, typename P>
  friend bool operator>(const std::chrono::duration<R, P>& lhs,
                        const elapsedTime& rhs) {
    return rhs < lhs;
  }

  template <typename R, typename P>
  friend bool operator<=(const elapsedTime& lhs,
                         const std::chrono::duration<R, P>& rhs) {
    return !(lhs > rhs);
  }

  template <typename R, typename P>
  friend bool operator<=(const std::chrono::duration<R, P>& lhs,
                         const elapsedTime& rhs) {
    return !(lhs > rhs);
  }

  template <typename R, typename P>
  friend bool operator>=(const elapsedTime& lhs,
                         const std::chrono::duration<R, P>& rhs) {
    return !(lhs < rhs);
  }

  template <typename R, typename P>
  friend bool operator>=(const std::chrono::duration<R, P>& lhs,
                         const elapsedTime& rhs) {
    return !(lhs < rhs);
  }

 private:
  time_point base_;
};

}  // namespace util
}  // namespace network
}  // namespace qindesign
