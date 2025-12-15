// SPDX-FileCopyrightText: (c) 2025 Shawn Silverman <shawn@pobox.com>
// SPDX-License-Identifier: AGPL-3.0-or-later

// c11_compat.h defines some C11 compatibility things.
// This file is part of the QNEthernet library.

#pragma once

#if defined(__cplusplus) || (__STDC__VERSION__ >= 202311L)
#define ATTRIBUTE_NODISCARD [[nodiscard]]
#else
#define ATTRIBUTE_NODISCARD
#endif  // >= C++23
