// SPDX-FileCopyrightText: (c) 2025-2026 Shawn Silverman <shawn@pobox.com>
// SPDX-License-Identifier: AGPL-3.0-or-later

// c11_compat.h defines some C11 compatibility things.
// This file is part of the QNEthernet library.

#pragma once

#if (__cplusplus >= 201703L) || (__STDC_VERSION__ >= 202311L)
#define ATTRIBUTE_ALWAYS_INLINE [[gnu::always_inline]]
#define ATTRIBUTE_DEPRECATED(A) __attribute__((deprecated(A)))
#define ATTRIBUTE_FALLTHROUGH [[fallthrough]]
#define ATTRIBUTE_MAYBE_UNUSED [[maybe_unused]]
#define ATTRIBUTE_NODISCARD [[nodiscard]]
#define ATTRIBUTE_WEAK [[gnu::weak]]
#else
#define ATTRIBUTE_ALWAYS_INLINE __attribute((always_inline))
#define ATTRIBUTE_DEPRECATED(A) [[deprecated(A)]]
#define ATTRIBUTE_FALLTHROUGH __attribute((fallthrough))
#define ATTRIBUTE_MAYBE_UNUSED __attribute__((unused))
// It's hard to silence the warnings, even if casting a result to void
#define ATTRIBUTE_NODISCARD /*__attribute__((warn_unused_result))*/
#define ATTRIBUTE_WEAK __attribute__((weak))
#endif  // >= C++23
