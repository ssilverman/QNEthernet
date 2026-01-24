// SPDX-FileCopyrightText: (c) 2023-2026 Shawn Silverman <shawn@pobox.com>
// SPDX-License-Identifier: AGPL-3.0-or-later

// RandomDevice.h defines a class that conforms to the
// UniformRandomBitGenerator C++ named requirement.
// See: https://www.cppreference.com/w/cpp/named_req/UniformRandomBitGenerator.html
// This file is part of the QNEthernet library.

#pragma once

// C++ includes
#include <cstdint>
#include <limits>

#include "qnethernet/StaticInit.h"

namespace qindesign {
namespace security {

// RandomDevice implements UniformRandomBitGenerator.
// See: https://www.cppreference.com/w/cpp/named_req/UniformRandomBitGenerator.html
class RandomDevice {
 public:
  typedef uint32_t result_type;

  // Accesses the singleton instance.
  static RandomDevice& instance();

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

  // Disallow copying and moving
  RandomDevice(const RandomDevice&) = delete;
  RandomDevice(RandomDevice&&) = delete;
  RandomDevice& operator=(const RandomDevice&) = delete;
  RandomDevice& operator=(RandomDevice&&) = delete;

  friend class StaticInit<RandomDevice>;
};

STATIC_INIT_DECL(RandomDevice, randomDevice);

}  // namespace security
}  // namespace qindesign
