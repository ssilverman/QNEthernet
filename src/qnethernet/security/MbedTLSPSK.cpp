// SPDX-FileCopyrightText: (c) 2024 Shawn Silverman <shawn@pobox.com>
// SPDX-License-Identifier: AGPL-3.0-or-later

// MbedTLSPSK.cpp implements MbedTLSPSK.
// This file is part of the QNEthernet library.

#include "MbedTLSPSK.h"

namespace qindesign {
namespace security {

bool MbedTLSPSK::empty() const {
  return psk_.empty() && id_.empty();
}

}  // namespace security
}  // namespace qindesign
