// SPDX-FileCopyrightText: (c) 2023-2026 Shawn Silverman <shawn@pobox.com>
// SPDX-License-Identifier: AGPL-3.0-or-later

// random_device.cpp implements random_device.
// This file is part of the QNEthernet library.

#include "qnethernet/security/random_device.h"

// C++ includes
#include <cstdint>

#include "qnethernet/platforms/pgmspace.h"

namespace qindesign {
namespace security {

extern "C" {
void qnethernet_hal_init_entropy();
void qnethernet_hal_deinit_entropy();
double qnethernet_hal_estimate_entropy();
size_t qnethernet_hal_entropy_available();
uint32_t qnethernet_hal_entropy();
size_t qnethernet_hal_fill_entropy(void* buf, size_t size);
}  // extern "C"

static void ensureInitialized() {
  // This is a lightweight alternative to std::call_once
  static bool initialized = []() {
    qnethernet_hal_init_entropy();
    return true;
  }();
  (void)initialized;
}

random_device::random_device() : random_device("default") {}

random_device::random_device(const std::string& token) {
  (void)token;
  ensureInitialized();
}

double random_device::entropy() const noexcept {
  return qnethernet_hal_estimate_entropy();
}

random_device::result_type random_device::operator()() {
  return static_cast<result_type>(qnethernet_hal_entropy());
}

size_t random_device::operator()(uint8_t* const buf, const size_t size) {
  return qnethernet_hal_fill_entropy(buf, size);
}

size_t random_device::available() {
  return qnethernet_hal_entropy_available();
}

}  // namespace security
}  // namespace qindesign
