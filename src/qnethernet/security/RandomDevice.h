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
//
// Note that the constructor calls qnethernet_hal_init_entropy() and the
// destructor calls qnethernet_hal_deinit_entropy().
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

  // Obtains the entropy estimate for the non-deterministic random number
  // generator. For a deterministic generator, this will return zero.
  //
  // See: https://en.cppreference.com/w/cpp/numeric/random/random_device/entropy.html
  double entropy() const;

  result_type operator()();

 private:
  RandomDevice();
  ~RandomDevice() noexcept;

  // Disallow copying and moving
  RandomDevice(const RandomDevice&) = delete;
  RandomDevice(RandomDevice&&) = delete;
  RandomDevice& operator=(const RandomDevice&) = delete;
  RandomDevice& operator=(RandomDevice&&) = delete;

  friend class StaticInit<RandomDevice>;
};

STATIC_INIT_DECL(RandomDevice, randomDevice);

// Create a class that can mimic std::random_device.
//
// See: https://en.cppreference.com/w/cpp/numeric/random/random_device.html
class random_device {
 public:
  using result_type = RandomDevice::result_type;

  random_device() = default;
  ~random_device() = default;

  // Disallow copying and moving
  random_device(const random_device&) = delete;
  random_device(random_device&&) = delete;
  random_device& operator=(const random_device&) = delete;
  random_device& operator=(random_device&&) = delete;

  static constexpr result_type min() {
    return RandomDevice::min();
  }

  static constexpr result_type max() {
    return RandomDevice::max();
  }

  double entropy() const {
    return randomDevice.entropy();
  }

  result_type operator()() {
    return randomDevice();
  }
};

}  // namespace security
}  // namespace qindesign
