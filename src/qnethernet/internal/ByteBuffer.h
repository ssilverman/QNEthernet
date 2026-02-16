// SPDX-FileCopyrightText: (c) 2022-2026 Shawn Silverman <shawn@pobox.com>
// SPDX-License-Identifier: AGPL-3.0-or-later

// ByteBuffer.h defines a std::array-backed circular buffer for
// internal use.
// This file is part of the QNEthernet library.

#pragma once

// C++ includes
#include <algorithm>
#include <array>
#include <cassert>
#include <cstddef>
#include <limits>
#include <type_traits>
#include <utility>

#include "lwip/err.h"
#include "lwip/pbuf.h"
#include "qnethernet/compat/c++11_compat.h"

namespace qindesign {
namespace network {
namespace internal {

template <size_t N>
class ByteBuffer {
 public:
  ByteBuffer() = default;

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

  ATTRIBUTE_NODISCARD
  size_t remaining() const {
    return capacity() - size();
  }

  // Reads data from the buffer into the given pre-allocated pbuf. This returns
  // an error if:
  // * There isn't enough data to fill the pbuf
  // * pbuf_take() or pbuf_take_at() returns an error
  //
  // If this returns an error (not ERR_OK) then the buffer's size is
  // not reduced.
  ATTRIBUTE_NODISCARD
  err_t read(pbuf* const p) {
    if (p->tot_len > size()) {
      return ERR_BUF;
    }

    const size_t start = tail_ % capacity();
    const size_t newTail = (tail_ + p->tot_len) % capacity();
    const size_t end = newTail;

    err_t err;
    if (start < end) {
      err = pbuf_take(p, &buf_[start], p->tot_len);
    } else {
      // It's safe to cast the sizes to uint16_t because the frame sizes are
      // limited to UINT16_MAX
      err =
          pbuf_take(p, &buf_[start], static_cast<uint16_t>(capacity() - start));
      if (err == ERR_OK) {
        err = pbuf_take_at(p, &buf_[0], static_cast<uint16_t>(end),
                           static_cast<uint16_t>(capacity() - start));
      }
    }
    if (err == ERR_OK) {
      tail_ = newTail;
      size_ -= p->tot_len;
    }

    return err;
  }

  ATTRIBUTE_NODISCARD
  size_t read(void* const buf, size_t size) {
    uint8_t* const bytes = static_cast<uint8_t*>(buf);

    size = std::min(size, this->size());
    if (size == 0) {
      return 0;
    }

    const size_t start = tail_ % capacity();
    tail_ = (tail_ + size) % capacity();
    const size_t end = tail_;

    if (start < end) {
      std::copy_n(&buf_[start], size, &bytes[0]);
    } else {
      std::copy_n(&buf_[start], capacity() - start, &bytes[0]);
      std::copy_n(&buf_[0], end, &bytes[capacity() - start]);
    }

    size_ -= size;

    return size;
  }

  ATTRIBUTE_NODISCARD
  size_t write(const void* const buf, size_t size) {
    const uint8_t* const bytes = static_cast<const uint8_t*>(buf);

    size = std::min(size, remaining());
    if (size == 0) {
      return 0;
    }

    const size_t start = head_ % capacity();
    head_ = (head_ + size) % capacity();
    const size_t end = head_;

    if (start < end) {
      std::copy_n(&bytes[0], size, &buf_[start]);
    } else {
      std::copy_n(&bytes[0], capacity() - start, &buf_[start]);
      std::copy_n(&bytes[capacity() - start], end, &buf_[0]);
    }

    size_ += size;

    return size;
  }

  void clear() {
    size_ = 0;
    head_ = 0;
    tail_ = 0;
  }

 private:
  std::array<uint8_t, N> buf_;

  size_t size_ = 0;
  size_t head_ = 0;  // New elements go here
  size_t tail_ = 0;  // Old elements are read from here
};

}  // namespace internal
}  // namespace network
}  // namespace qindesign
