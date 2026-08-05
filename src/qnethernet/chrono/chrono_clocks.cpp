// SPDX-FileCopyrightText: (c) 2026 Shawn Silverman <shawn@pobox.com>
// SPDX-License-Identifier: AGPL-3.0-or-later

// chrono_clocks.cpp implements the "inline-like" parts of chrono_clocks.h. This
// is only here because 'inline' members are only supported in C++17 or later.
// This file is part of the QNEthernet library.

#include "qnethernet/chrono/chrono_clocks.h"

#include "qnethernet/hardware/imxrt1060/DCB.h"
#include "qnethernet/hardware/imxrt1060/DWT.h"

namespace qindesign {
namespace chrono {

// --------------------------------------------------------------------------
//  arm_high_resolution_clock -- Only Potentially Supported on ARM
// --------------------------------------------------------------------------

#ifdef F_CPU

#ifdef __arm__

using namespace qindesign::hardware::imxrt1060;

uint32_t arm_high_resolution_clock_count() {
  return *DWT::CYCCNT::CYCCNT;
}

// Initializes the cycle counter and returns whether it's supported. This uses
// heuristics and isn't guaranteed to work for all cases.
bool arm_high_resolution_clock_init() {
  // First enable DWT and check
  if (DCB::DEMCR::TRCENA == 0) {
    DCB::DEMCR::TRCENA = 1;

    // Check that it was enabled
    if (DCB::DEMCR::TRCENA == 0) {
      return false;
    }
  }

  // Next, check the obvious feature presence
  if (DWT::CTRL::NOCYCCNT != 0) {
    return false;
  }

  // Next, check if the cycle count is enabled
  if (DWT::CTRL::CYCCNTENA == 0) {
    DWT::CTRL::CYCCNTENA = 1;

    // Check that it was enabled
    if (DWT::CTRL::CYCCNTENA == 0) {
      return false;
    }
  }

  return true;
}

#else

uint32_t arm_high_resolution_clock_count() {
  return 0;
}

bool arm_high_resolution_clock_init() {
  return false;
}

#endif  // __arm__

#endif  // F_CPU

}  // namespace chrono
}  // namespace qindesign
