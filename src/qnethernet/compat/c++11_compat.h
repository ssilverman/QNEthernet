// SPDX-FileCopyrightText: (c) 2025-2026 Shawn Silverman <shawn@pobox.com>
// SPDX-License-Identifier: AGPL-3.0-or-later

// c++11_compat.h defines some C++11 compatibility functions.
// This file is part of the QNEthernet library.

#pragma once

#include <memory>
#include <utility>

namespace qindesign {
namespace network {
namespace compat {

// Unknown attributes are not specified to be ignored before C++17
// See: https://en.cppreference.com/w/cpp/language/attributes.html
#if __cplusplus < 201703L
#define ATTRIBUTE_FALLTHROUGH
#define ATTRIBUTE_NODISCARD
#else
#define ATTRIBUTE_FALLTHROUGH [[fallthrough]]
#define ATTRIBUTE_NODISCARD [[nodiscard]]
#endif  // C++ < 17

// Creates a new std::unique_ptr.
template <typename T, typename... Args>
[[gnu::always_inline]]
inline std::unique_ptr<T> make_unique(Args&&... args) {
#if __cplusplus < 201402L
  return std::unique_ptr<T>(new T(std::forward<Args>(args)...));
#else
  return std::make_unique<T>(std::forward<Args>(args)...);
#endif  // C++ < 14
}

}  // namespace compat
}  // namespace network
}  // namespace qindesign
