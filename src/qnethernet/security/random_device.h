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
#include <cstddef>
#include <cstdint>
#include <limits>
#include <string>

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

  // Calls random_device("default").
  random_device();

  // Creates a random_device. This currently ignores the token.
  explicit random_device(const std::string& token);

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
  double entropy() const noexcept;

  result_type operator()();

  // ------------------------------------------------------------------------
  // Methods not part of the std::random_device API
  // ------------------------------------------------------------------------

  // Fills a buffer with random values. This will return the number of bytes
  // actually filled. If the returned value is less than the requested size then
  // there was an entropy generation error.
  size_t operator()(uint8_t* const buf, const size_t size);

  // Returns the number of entropy bytes available without having to
  // generate more.
  size_t available();
};

}  // namespace security
}  // namespace qindesign
