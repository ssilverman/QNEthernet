// SPDX-FileCopyrightText: (c) 2022-2026 Shawn Silverman <shawn@pobox.com>
// SPDX-License-Identifier: AGPL-3.0-or-later

// CircularBuffer.h defines a circular buffer.
//
// This file is part of the QNEthernet library.

#pragma once

// C++ includes
#include <cstddef>
#include <memory>

template <typename T>
class CircularBuffer {
 public:
  explicit CircularBuffer(size_t capacity)
      : capacity_(capacity),
        buf_{std::make_unique<T[]>(capacity)} {}

  bool empty() const {
    return (size() == 0);
  }

  size_t size() const {
    return size_;
  }

  size_t capacity() const {
    return capacity_;
  }

  T get() {
    if (empty()) {
      return T{};
    }

    const size_t oldTail = tail_;
    tail_ = (tail_ + 1) % capacity();
    --size_;
    return buf_[oldTail];
  }

  void put(const T& t) {
    buf_[head_] = t;
    if (size() == capacity()) {
      tail_ = (tail_ + 1) % capacity();
    } else {
      ++size_;
    }
    head_ = (head_ + 1) % capacity();
  }

  void clear() {
    size_ = 0;
    head_ = 0;
    tail_ = 0;
  }

  T& operator[](size_t n) {
    return get(*this, n);
  }

  const T& operator[](size_t n) const {
    return get(*this, n);
  }

 private:
  // Handles both const and non-const cases
  template <typename U>
  static T& get(U& t, size_t n) {
    return t.buf_[(t.tail_ + n) % t.capacity()];
  }

  const size_t capacity_;

  std::unique_ptr<T[]> buf_;

  size_t size_ = 0;
  size_t head_ = 0;
  size_t tail_ = 0;
};
