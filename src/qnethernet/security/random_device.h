// SPDX-FileCopyrightText: (c) 2023-2026 Shawn Silverman <shawn@pobox.com>
// SPDX-License-Identifier: AGPL-3.0-or-later

// random_device.h defines a class that conforms to the
// UniformRandomBitGenerator C++ named requirement and
// mimics std::random_device.
//
// See:
// * https://www.cppreference.com/w/cpp/named_req/UniformRandomBitGenerator.html
// * https://en.cppreference.com/w/cpp/numeric/random/random_device.html
//
// This file is part of the QNEthernet library.

#pragma once

// C++ includes
#include <limits>

namespace qindesign {
namespace security {

// random_device implements UniformRandomBitGenerator and
// mimics std::random_device.
//
// See:
// * https://www.cppreference.com/w/cpp/named_req/UniformRandomBitGenerator.html
// * https://en.cppreference.com/w/cpp/numeric/random/random_device.html
class random_device {
 public:
  using result_type = unsigned int;

  // Accesses the singleton instance.
  static random_device& instance();

  random_device() = default;
  ~random_device() = default;

  // Disallow copying and moving
  random_device(const random_device&) = delete;
  random_device(random_device&&) = delete;
  random_device& operator=(const random_device&) = delete;
  random_device& operator=(random_device&&) = delete;

  static constexpr result_type min() {
    return std::numeric_limits<result_type>::min();
  }

  static constexpr result_type max() {
    return std::numeric_limits<result_type>::max();
  }

  // Obtains the entropy estimate for the non-deterministic random number
  // generator. For a deterministic generator, this will return zero.
  //
  // See:
  // https://en.cppreference.com/w/cpp/numeric/random/random_device/entropy.html
  double entropy() const;

  result_type operator()();
};

}  // namespace security
}  // namespace qindesign
