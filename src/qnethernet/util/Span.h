// SPDX-FileCopyrightText: (c) 2024 Shawn Silverman <shawn@pobox.com>
// SPDX-License-Identifier: AGPL-3.0-or-later

// Span.h defines spans of things.
// This file is part of the QNEthernet library.

#pragma once

#include <cstdint>

namespace qindesign {
namespace network {
namespace util {

// Span holds a pointer to T and a size. This initialized with NULL and 0.
template <typename T>
struct Span {
  T *v = nullptr;
  size_t size = 0;

  // Returns whether the span is empty. A span is considered empty when the
  // pointer is NULL or the size is zero.
  bool empty() const {
    return (v == nullptr) || (size == 0);
  }

  // Sets the pointer to NULL and the size to zero.
  void clear() {
    v = nullptr;
    size = 0;
  }
};

// A Span of unchanging uint8_t's.
using ByteSpan = Span<const uint8_t>;

}  // namespace util
}  // namespace network
}  // namespace qindesign
