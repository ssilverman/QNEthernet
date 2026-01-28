// SPDX-FileCopyrightText: (c) 2022-2026 Shawn Silverman <shawn@pobox.com>
// SPDX-License-Identifier: AGPL-3.0-or-later

// CircularBuffer.h defines a std::vector-backed circular buffer for
// internal use. It's currently used by EthernetUDP and EthernetFrame.
// This file is part of the QNEthernet library.

#pragma once

// C++ includes
#include <algorithm>
#include <cassert>
#include <cstddef>
#include <utility>
#include <vector>

#include "qnethernet/compat/c++11_compat.h"

namespace qindesign {
namespace network {
namespace internal {

template <typename T>
class CircularBuffer {
 public:
  // Creates a new circular buffer with the given capacity. If zero is specified
  // then a value of 1 will be used instead.
  explicit CircularBuffer(const size_t capacity)
      : buf_(std::max(capacity, size_t{1})) {}

  ATTRIBUTE_NODISCARD
  bool empty() const {
    return (size() == 0);
  }

  ATTRIBUTE_NODISCARD
  bool full() const {
    return (size() == capacity());
  }

  ATTRIBUTE_NODISCARD
  size_t size() const {
    return size_;
  }

  ATTRIBUTE_NODISCARD
  size_t capacity() const {
    return buf_.size();
  }

  // Sets a new capacity. If the new capacity is smaller than the current
  // capacity then all the newest elements are kept. If a capacity of zero is
  // requested then the value will be set to 1.
  void setCapacity(const size_t cap) {
    if (cap == capacity()) {
      return;
    }

    const size_t actualCap = std::max(cap, size_t{1});

    size_t n;
    if (actualCap <= size_) {
      // Keep all the newest elements
      // Rotate the new tail (after lopping off the too-long end) to
      // position 0
      n = (tail_ + (size() - actualCap)) % capacity();
      head_ = 0;  // Because it's now full
      size_ = actualCap;
    } else {
      // Rotate the tail to position 0 before resizing because the
      // elements may be wrapped to the smaller size
      n = tail_;
      head_ = size_;
      // Size stays the same
    }
    if (n != 0) {
      (void)std::rotate(buf_.begin(), buf_.begin() + n, buf_.end());
    }
    buf_.resize(actualCap);
    tail_ = 0;

    // TODO: Should we shrink_to_fit?
    // buf_.shrink_to_fit();
  }

  // Pops the oldest element. If this is empty then a new T{} is returned. The
  // caller may want to std::move() the returned element.
  ATTRIBUTE_NODISCARD
  T get() {
    if (empty()) {
      return T{};
    }

    const size_t oldTail = tail_;
    tail_ = (tail_ + 1) % capacity();
    --size_;

    // We're moving out so probably don't need to clear the old value
    // It's just left in an unspecified state
    return std::move(buf_[oldTail]);
  }

  // Stores and returns a reference to a latest element. If the buffer is full
  // then the returned value will be a reference to the overwritten element. It
  // may be in an unspecified state.
  ATTRIBUTE_NODISCARD
  T& put() {
    T& t = buf_[head_];
    if (full()) {
      tail_ = (tail_ + 1) % capacity();
    } else {
      ++size_;
    }
    head_ = (head_ + 1) % capacity();
    return t;
  }

  void clear() {
    size_ = 0;
    head_ = 0;
    tail_ = 0;
  }

  ATTRIBUTE_NODISCARD
  T& operator[](size_t n) {
    return get(*this, n);
  }

  ATTRIBUTE_NODISCARD
  const T& operator[](size_t n) const {
    return get(*this, n);
  }

 private:
  // Gets the nth element. It is expected that n < size, but if it isn't, this
  // will assert.
  // Handles both const and non-const cases.
  template <typename U>
  ATTRIBUTE_NODISCARD
  static T& get(U& t, const size_t n) {
    assert(n < t.size());
    return t.buf_[(t.tail_ + n) % t.capacity()];
  }

  std::vector<T> buf_;

  size_t size_ = 0;
  size_t head_ = 0;
  size_t tail_ = 0;
};

}  // namespace internal
}  // namespace network
}  // namespace qindesign
