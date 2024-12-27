// SPDX-FileCopyrightText: (c) 2024 Shawn Silverman <shawn@pobox.com>
// SPDX-License-Identifier: AGPL-3.0-or-later

// MbedTLSPSK.h defines an object that holds an MbedTLS pre-shared key.
// This file is part of the QNEthernet library.

// C++ includes
#include <cstdint>
#include <vector>

#include <mbedtls/ssl.h>

#include "util/Span.h"

namespace qindesign {
namespace security {

class MbedTLSPSK {
 public:
  MbedTLSPSK() = default;
  ~MbedTLSPSK() = default;

  // Allow both copying and moving
  MbedTLSPSK(const MbedTLSPSK &) = default;
  MbedTLSPSK &operator=(const MbedTLSPSK &) = default;
  MbedTLSPSK(MbedTLSPSK &&other) = default;
  MbedTLSPSK &operator=(MbedTLSPSK &&other) = default;

  // Returns whether both the PSK and ID are empty.
  bool empty() const;

  // Returns a reference to the internal PSK vector.
  const std::vector<uint8_t> &psk() const {
    return psk_;
  }

  // Returns a reference to the internal PSK ID vector.
  const std::vector<uint8_t> &id() const {
    return id_;
  }

 private:
  std::vector<uint8_t> psk_;
  std::vector<uint8_t> id_;
};

}  // namespace security
}  // namespace qindesign
