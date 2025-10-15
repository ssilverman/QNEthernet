// SPDX-FileCopyrightText: (c) 2025 Shawn Silverman <shawn@pobox.com>
// SPDX-License-Identifier: AGPL-3.0-or-later

// optional.h defines a simple "optional" implementation.
// This file is part of the QNEthernet library.

namespace qindesign {
namespace network {
namespace internal {

// optional implements a simple optional type. has_value should be set to true
// before accessing the value, and false only after the value is no
// longer needed.
template <typename T>
struct optional {
  bool has_value = false;
  T value{};
};

}  // namespace internal
}  // namespace network
}  // namespace qindesign
