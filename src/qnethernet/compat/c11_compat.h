// SPDX-FileCopyrightText: (c) 2025-2026 Shawn Silverman <shawn@pobox.com>
// SPDX-License-Identifier: AGPL-3.0-or-later

// c11_compat.h defines some C11 compatibility things.
// This file is part of the QNEthernet library.

#pragma once

// Check for definitions in case c++11_compat.h was included before this one

#if (__cplusplus >= 201703L) || (__STDC_VERSION__ >= 202311L)
#ifndef ATTRIBUTE_ALWAYS_INLINE
#define ATTRIBUTE_ALWAYS_INLINE [[gnu::always_inline]]
#endif
#ifndef ATTRIBUTE_DEPRECATED
#define ATTRIBUTE_DEPRECATED(A) __attribute__((deprecated(A)))
#endif
#ifndef ATTRIBUTE_FALLTHROUGH
#define ATTRIBUTE_FALLTHROUGH [[fallthrough]]
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
#else
#ifndef ATTRIBUTE_ALWAYS_INLINE
#define ATTRIBUTE_ALWAYS_INLINE __attribute((always_inline))
#endif
#ifndef ATTRIBUTE_DEPRECATED
#define ATTRIBUTE_DEPRECATED(A) [[deprecated(A)]]
#endif
#ifndef ATTRIBUTE_FALLTHROUGH
#define ATTRIBUTE_FALLTHROUGH __attribute((fallthrough))
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
#endif  // >= C++23
