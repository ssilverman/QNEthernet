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

// Check for definitions in case c11_compat.h was included before this one

// Unknown attributes are not specified to be ignored before C++17
// See: https://en.cppreference.com/w/cpp/language/attributes.html
#if __cplusplus < 201703L
#ifndef ATTRIBUTE_ALWAYS_INLINE
#define ATTRIBUTE_ALWAYS_INLINE __attribute__((always_inline))
#endif
#ifndef ATTRIBUTE_DEPRECATED
#define ATTRIBUTE_DEPRECATED(A) __attribute__((deprecated(A)))
#endif
#ifndef ATTRIBUTE_FALLTHROUGH
#define ATTRIBUTE_FALLTHROUGH __attribute__((fallthrough))
#endif
#ifndef ATTRIBUTE_FORMAT
#define ATTRIBUTE_FORMAT(A, B, C) __attribute__((format(A, B, C)))
#endif
#ifndef ATTRIBUTE_MAYBE_UNUSED
#define ATTRIBUTE_MAYBE_UNUSED __attribute__((unused))
#endif
#ifndef ATTRIBUTE_NODISCARD
// It's hard to silence the warnings, even if casting a result to void
#define ATTRIBUTE_NODISCARD /*__attribute__((warn_unused_result))*/
#endif
#ifndef ATTRIBUTE_WEAK
#define ATTRIBUTE_WEAK __attribute__((weak))
#endif
#else
#ifndef ATTRIBUTE_ALWAYS_INLINE
#define ATTRIBUTE_ALWAYS_INLINE [[gnu::always_inline]]
#endif
#ifndef ATTRIBUTE_DEPRECATED
#define ATTRIBUTE_DEPRECATED(A) [[deprecated(A)]]
#endif
#ifndef ATTRIBUTE_FALLTHROUGH
#define ATTRIBUTE_FALLTHROUGH [[fallthrough]]
#endif
#ifndef ATTRIBUTE_FORMAT
#define ATTRIBUTE_FORMAT(A, B, C) [[gnu::format(A, B, C)]]
#endif
#ifndef ATTRIBUTE_MAYBE_UNUSED
#define ATTRIBUTE_MAYBE_UNUSED [[maybe_unused]]
#endif
#ifndef ATTRIBUTE_NODISCARD
#define ATTRIBUTE_NODISCARD [[nodiscard]]
#endif
#ifndef ATTRIBUTE_WEAK
#define ATTRIBUTE_WEAK [[gnu::weak]]
#endif
#endif  // C++ < 17

// Creates a new std::unique_ptr.
template <typename T, typename... Args>
ATTRIBUTE_ALWAYS_INLINE
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
