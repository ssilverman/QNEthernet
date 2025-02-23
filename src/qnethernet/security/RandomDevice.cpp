// SPDX-FileCopyrightText: (c) 2023-2025 Shawn Silverman <shawn@pobox.com>
// SPDX-License-Identifier: AGPL-3.0-or-later

// RandomDevice.cpp implements RandomDevice.
// This file is part of the QNEthernet library.

#include "qnethernet/security/RandomDevice.h"

// C++ includes
#include <cstdint>

#include <avr/pgmspace.h>

#ifndef FLASHMEM
#define FLASHMEM
#endif  // !FLASHMEM

namespace qindesign {
namespace security {

extern "C" {
void qnethernet_hal_init_entropy(void);
uint32_t qnethernet_hal_entropy(void);
}  // extern "C"

STATIC_INIT_DEFN(RandomDevice, randomDevice);

RandomDevice &RandomDevice::instance() {
  return randomDevice;
}

FLASHMEM RandomDevice::RandomDevice() {
  qnethernet_hal_init_entropy();
}

RandomDevice::result_type RandomDevice::operator()() {
  return qnethernet_hal_entropy();
}

}  // namespace security
}  // namespace qindesign
