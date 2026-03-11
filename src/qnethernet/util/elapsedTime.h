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
  using rep        = typename duration::rep;

  elapsedTime() : elapsedTime(duration::zero()) {}

  explicit elapsedTime(const rep d)
      : base_{Clock::now() - duration{d}} {}

  template <typename R, typename P>
  explicit elapsedTime(std::chrono::duration<R, P> d)
      : elapsedTime(std::chrono::duration_cast<duration>(d).count()) {}

  // Rule of zero: No declared destructors, copy, or move operations;
  // they will be defaulted

  // Returns this as a duration. This is useful when the object won't
  // automatically convert.
  duration dur() const {
    return static_cast<duration>(*this);
  }

  // Resets the timer back to zero.
  void reset() {
    *this = duration::zero();
  }

  operator duration() const {
    return Clock::now() - base_;
  }

  // For operator overloading, see also:
  // https://en.cppreference.com/w/cpp/language/operators.html

  // Assigns a duration count to the timer. The count type is duration::rep.
  elapsedTime& operator=(const rep d) {
    base_ = Clock::now() - duration{d};
    return *this;
  }

  template <typename R, typename P>
  elapsedTime& operator=(const std::chrono::duration<R, P>& d) {
    *this = std::chrono::duration_cast<duration>(d).count();
    return *this;
  }

  elapsedTime& operator+=(const rep d) {
    base_ -= duration{d};
    return *this;
  }

  template <typename R, typename P>
  elapsedTime& operator+=(const std::chrono::duration<R, P>& d) {
    base_ -= std::chrono::duration_cast<duration>(d);
    return *this;
  }

  elapsedTime& operator-=(const rep d) {
    base_ += duration{d};
    return *this;
  }

  template <typename R, typename P>
  elapsedTime& operator-=(const std::chrono::duration<R, P>& d) {
    base_ += std::chrono::duration_cast<duration>(d);
    return *this;
  }

  friend elapsedTime operator+(elapsedTime lhs, const rep rhs) {
    lhs += rhs;
    return lhs;
  }

  template <typename R, typename P>
  friend elapsedTime operator+(elapsedTime lhs,
                               const std::chrono::duration<R, P>& rhs) {
    lhs += std::chrono::duration_cast<duration>(rhs);
    return lhs;
  }

  friend elapsedTime operator-(elapsedTime lhs, const rep rhs) {
    lhs -= rhs;
    return lhs;
  }

  template <typename R, typename P>
  friend elapsedTime operator-(elapsedTime lhs,
                               const std::chrono::duration<R, P>& rhs) {
    lhs -= std::chrono::duration_cast<duration>(rhs);
    return lhs;
  }

  friend bool operator==(const elapsedTime& lhs, const rep rhs) {
    return (static_cast<duration>(lhs).count() == rhs);
  }

  template <typename R, typename P>
  friend bool operator==(const elapsedTime& lhs,
                         const std::chrono::duration<R, P>& rhs) {
    return (static_cast<duration>(lhs) ==
            std::chrono::duration_cast<duration>(rhs));
  }

  friend bool operator==(const rep lhs, const elapsedTime& rhs) {
    return (lhs == static_cast<duration>(rhs).count());
  }

  template <typename R, typename P>
  friend bool operator==(const std::chrono::duration<R, P>& lhs,
                         const elapsedTime& rhs) {
    // using type = typename std::remove_cv<
    //     typename std::remove_reference<decltype(lhs)>::type>::type;
    return (lhs == std::chrono::duration_cast<std::chrono::duration<R, P>>(
                       static_cast<duration>(rhs)));
  }

  friend bool operator!=(const elapsedTime& lhs, const rep rhs) {
    return !(lhs == rhs);
  }

  template <typename R, typename P>
  friend bool operator!=(const elapsedTime& lhs,
                         const std::chrono::duration<R, P>& rhs) {
    return !(lhs == rhs);
  }

  friend bool operator!=(const rep lhs, const elapsedTime& rhs) {
    return !(lhs == rhs);
  }

  template <typename R, typename P>
  friend bool operator!=(const std::chrono::duration<R, P>& lhs,
                         const elapsedTime& rhs) {
    return !(lhs == rhs);
  }

  friend bool operator<(const elapsedTime& lhs, const rep rhs) {
    return static_cast<duration>(lhs).count() < rhs;
  }

  template <typename R, typename P>
  friend bool operator<(const elapsedTime& lhs,
                        const std::chrono::duration<R, P>& rhs) {
    return static_cast<duration>(lhs) < rhs;
  }

  friend bool operator<(const rep lhs, const elapsedTime& rhs) {
    return lhs < static_cast<duration>(rhs).count();
  }

  template <typename R, typename P>
  friend bool operator<(const std::chrono::duration<R, P>& lhs,
                        const elapsedTime& rhs) {
    // using type = typename std::remove_cv<
    //     typename std::remove_reference<decltype(lhs)>::type>::type;
    return lhs < std::chrono::duration_cast<std::chrono::duration<R, P>>(
                     static_cast<duration>(rhs));
  }

  friend bool operator>(const elapsedTime& lhs, const rep rhs) {
    return rhs < lhs;
  }

  template <typename R, typename P>
  friend bool operator>(const elapsedTime& lhs,
                        const std::chrono::duration<R, P>& rhs) {
    return rhs < lhs;
  }

  friend bool operator>(const rep lhs, const elapsedTime& rhs) {
    return rhs < lhs;
  }

  template <typename R, typename P>
  friend bool operator>(const std::chrono::duration<R, P>& lhs,
                        const elapsedTime& rhs) {
    return rhs < lhs;
  }

  friend bool operator<=(const elapsedTime& lhs, const rep rhs) {
    return !(lhs > rhs);
  }

  template <typename R, typename P>
  friend bool operator<=(const elapsedTime& lhs,
                         const std::chrono::duration<R, P>& rhs) {
    return !(lhs > rhs);
  }

  friend bool operator<=(const rep lhs, const elapsedTime& rhs) {
    return !(lhs > rhs);
  }

  template <typename R, typename P>
  friend bool operator<=(const std::chrono::duration<R, P>& lhs,
                         const elapsedTime& rhs) {
    return !(lhs > rhs);
  }

  friend bool operator>=(const elapsedTime& lhs, const rep rhs) {
    return !(lhs < rhs);
  }

  template <typename R, typename P>
  friend bool operator>=(const elapsedTime& lhs,
                         const std::chrono::duration<R, P>& rhs) {
    return !(lhs < rhs);
  }

  friend bool operator>=(const rep lhs, const elapsedTime& rhs) {
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
