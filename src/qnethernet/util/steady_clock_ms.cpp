// SPDX-FileCopyrightText: (c) 2026 Shawn Silverman <shawn@pobox.com>
// SPDX-License-Identifier: AGPL-3.0-or-later

// steady_clock_ms.cpp implements the "inline-like" parts of steady_clock_ms.h.
// This is only here because 'inline' members are only supported in C++17
// or later.
// This file is part of the QNEthernet library.

#include "qnethernet/util/steady_clock_ms.h"

namespace qindesign {
namespace network {
namespace util {

decltype(steady_clock_ms::prevLow) steady_clock_ms::prevLow = 0;
decltype(steady_clock_ms::high) steady_clock_ms::high = 0;

}  // namespace util
}  // namespace network
}  // namespace qindesign
