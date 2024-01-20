// SPDX-FileCopyrightText: (c) 2023-2024 Shawn Silverman <shawn@pobox.com>
// SPDX-License-Identifier: AGPL-3.0-or-later

// RandomDevice.h defines a class that conforms to the
// UniformRandomBitGenerator C++ named requirement.
// This file is part of the QNEthernet library.

#pragma once

#include <cstdint>
#include <limits>

#include "StaticInit.h"

namespace qindesign {
namespace security {

// RandomDevice implements UniformRandomBitGenerator.
class RandomDevice {
 public:
  typedef uint32_t result_type;

  // Accesses the singleton instance.
  static RandomDevice &instance();

  // Disallow copying and moving
  RandomDevice(const RandomDevice &) = delete;
  RandomDevice &operator=(const RandomDevice &) = delete;

  static constexpr result_type min() {
    return std::numeric_limits<result_type>::min();
  }

  static constexpr result_type max() {
    return std::numeric_limits<result_type>::max();
  }

  result_type operator()();

 private:
  RandomDevice();
  ~RandomDevice() = default;

  friend class StaticInit<RandomDevice>;
};

STATIC_INIT_DECL(RandomDevice, randomDevice);

}  // namespace security
}  // namespace qindesign
