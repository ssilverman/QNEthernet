// SPDX-FileCopyrightText: (c) 2025-2026 Shawn Silverman <shawn@pobox.com>
// SPDX-License-Identifier: AGPL-3.0-or-later

// c11_compat.h defines some C11 compatibility things.
// This file is part of the QNEthernet library.

#pragma once

#if (__cplusplus >= 201703L) || (__STDC_VERSION__ >= 202311L)
#define ATTRIBUTE_NODISCARD [[nodiscard]]
#define ATTRIBUTE_FALLTHROUGH [[fallthrough]]
#else
#define ATTRIBUTE_NODISCARD
#define ATTRIBUTE_FALLTHROUGH
#endif  // >= C++23
