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
#include <climits>
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
#if (UINT_MAX != UINT32_MAX)
#warning "unsigned int size does not match uint32_t size"
#endif
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

  // Returns a random value from the hardware entropy generator. If entropy
  // generation fails, this will throw a std::runtime_error.
  result_type operator()();

  // ------------------------------------------------------------------------
  // Methods not part of the std::random_device API
  // ------------------------------------------------------------------------

  // Tries to read random data into the given buffer. If this reads less than
  // the number of requested bytes then there was an entropy generation error
  // and a std::runtime_error is thrown.
  void operator()(void* buf, size_t size);

  // Returns the number of entropy bytes available without having to
  // generate more.
  size_t available();
};

}  // namespace security
}  // namespace qindesign
