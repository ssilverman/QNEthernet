// SPDX-FileCopyrightText: (c) 2022-2024 Shawn Silverman <shawn@pobox.com>
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
        buf_{std::make_unique<T[]>(capacity)},
        size_(0),
        head_(0),
        tail_(0) {}

  bool empty() const {
    return size_ == 0;
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

    T element = buf_[tail_];
    tail_ = (tail_ + 1) % capacity_;
    size_--;
    return element;
  }

  void put(const T &t) {
    buf_[head_] = t;
    if (size_ == capacity_) {
      tail_ = (tail_ + 1) % capacity_;
    } else {
      size_++;
    }
    head_ = (head_ + 1) % capacity_;
  }

  void clear() {
    head_ = 0;
    tail_ = 0;
    size_ = 0;
  }

  T &operator[](size_t n) {
    return get(*this, n, capacity_);
  }

  const T &operator[](size_t n) const {
    return get(*this, n, capacity_);
  }

 private:
  // Handles both const and non-const cases
  template <typename U>
  static inline auto &get(U &t, size_t n, size_t capacity) {
    return t.buf_[(t.tail_ + n) % capacity];
  }

  const size_t capacity_;

  std::unique_ptr<T[]> buf_;

  size_t size_;
  size_t head_;
  size_t tail_;
};
