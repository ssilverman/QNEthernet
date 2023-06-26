// SPDX-FileCopyrightText: (c) 2023 Shawn Silverman <shawn@pobox.com>
// SPDX-License-Identifier: MIT

// RandomDevice.h defines a class that conforms to the
// UniformRandomBitGenerator C++ named requirement.
// This file is part of the QNEthernet library.

#ifndef QNETHERNET_SECURITY_RANDOMDEVICE_H_
#define QNETHERNET_SECURITY_RANDOMDEVICE_H_

#include <cstdint>
#include <limits>

namespace qindesign {
namespace security {

// RandomDevice implements UniformRandomBitGenerator.
class RandomDevice {
 public:
  typedef uint32_t result_type;

  RandomDevice();
  ~RandomDevice() = default;

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
};

}  // namespace security
}  // namespace qindesign

#endif  // QNETHERNET_SECURITY_RANDOMDEVICE_H_
