// SPDX-FileCopyrightText: (c) 2023 Shawn Silverman <shawn@pobox.com>
// SPDX-License-Identifier: MIT

// RandomDevice.cpp implements RandomDevice.
// This file is part of the QNEthernet library.

#include "RandomDevice.h"

#include <pgmspace.h>

#if defined(__IMXRT1062__) && !defined(QNETHERNET_USE_ENTROPY_LIB)

#include "entropy.h"

namespace qindesign {
namespace security {

FLASHMEM RandomDevice::RandomDevice() {
  if (!trng_is_started()) {
    trng_init();
  }
}

RandomDevice::result_type RandomDevice::operator()() {
  return entropy_random();
}

}  // namespace security
}  // namespace qindesign

#else

#include <Entropy.h>
#if defined(__IMXRT1062__)
#include <imxrt.h>
#endif  // __IMXRT1062__

namespace qindesign {
namespace security {

FLASHMEM RandomDevice::RandomDevice() {
#if defined(__IMXRT1062__)
  bool doEntropyInit = ((CCM_CCGR6 & CCM_CCGR6_TRNG(CCM_CCGR_ON_RUNONLY)) !=
                        CCM_CCGR6_TRNG(CCM_CCGR_ON_RUNONLY)) ||
                       ((TRNG_MCTL & TRNG_MCTL_TSTOP_OK) != 0);
#else
  bool doEntropyInit = true;
#endif  // __IMXRT1062__
  if (doEntropyInit) {
    Entropy.Initialize();
  }
}

RandomDevice::result_type RandomDevice::operator()() {
  return Entropy.random();
}

}  // namespace security
}  // namespace qindesign

#endif  // __IMXRT1062__ && !QNETHERNET_USE_ENTROPY_LIB
