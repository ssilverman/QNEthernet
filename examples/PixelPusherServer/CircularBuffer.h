// SPDX-FileCopyrightText: (c) 2022-2023 Shawn Silverman <shawn@pobox.com>
// SPDX-License-Identifier: MIT

// CircularBuffer.h defines a circular buffer.
//
// This file is part of the QNEthernet library.

#ifndef CIRCULARBUFFER_H_
#define CIRCULARBUFFER_H_

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

  size_t empty() const {
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
    tail_ = (tail_ + 1)%capacity_;
    size_--;
    return element;
  }

  void put(const T &t) {
    buf_[head_] = t;
    if (size_ == capacity_) {
      tail_ = (tail_ + 1)%capacity_;
    } else {
      size_++;
    }
    head_ = (head_ + 1)%capacity_;
  }

  void clear() {
    head_ = 0;
    tail_ = 0;
    size_ = 0;
  }

  T &operator[](size_t n) {
    return buf_[(tail_ + n)%capacity];
  }

  const T &operator[](size_t n) const {
    return buf_[(tail_ + n)%capacity];
  }

 private:
  const size_t capacity_;
  std::unique_ptr<T[]> buf_;
  size_t size_;
  size_t head_;
  size_t tail_;
};

#endif  // CIRCULARBUFFER_H_
