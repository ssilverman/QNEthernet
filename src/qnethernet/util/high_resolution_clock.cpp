// SPDX-FileCopyrightText: (c) 2026 Shawn Silverman <shawn@pobox.com>
// SPDX-License-Identifier: AGPL-3.0-or-later

// high_resolution_clock.cpp implements the "inline-like" parts of
// high_resolution_clock.h. This is only here because 'inline' members are only
// supported in C++17 or later.
// This file is part of the QNEthernet library.

#include "qnethernet/util/high_resolution_clock.h"

namespace qindesign {
namespace network {
namespace util {

decltype(high_resolution_clock::prevLow) high_resolution_clock::prevLow = 0;
decltype(high_resolution_clock::high) high_resolution_clock::high = 0;

}  // namespace util
}  // namespace network
}  // namespace qindesign
