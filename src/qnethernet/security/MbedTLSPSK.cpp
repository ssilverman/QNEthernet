// SPDX-FileCopyrightText: (c) 2024-2025 Shawn Silverman <shawn@pobox.com>
// SPDX-License-Identifier: AGPL-3.0-or-later

// MbedTLSPSK.cpp implements MbedTLSPSK.
// This file is part of the QNEthernet library.

#include "qnethernet/security/MbedTLSPSK.h"

namespace qindesign {
namespace security {

void MbedTLSPSK::setPSK(const uint8_t *const buf, const size_t len) {
  if (buf != nullptr) {
    psk_.assign(&buf[0], &buf[len]);
  } else {
    psk_.clear();
  }
}

void MbedTLSPSK::setId(const uint8_t *const buf, const size_t len) {
  if (buf != nullptr) {
    id_.assign(&buf[0], &buf[len]);
  } else {
    id_.clear();
  }
}

bool MbedTLSPSK::empty() const {
  return psk_.empty() && id_.empty();
}

}  // namespace security
}  // namespace qindesign
