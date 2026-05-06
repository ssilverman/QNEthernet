// SPDX-FileCopyrightText: (c) 2023-2026 Shawn Silverman <shawn@pobox.com>
// SPDX-License-Identifier: AGPL-3.0-or-later

// random_device.cpp implements random_device.
// This file is part of the QNEthernet library.

#include "qnethernet/entropy/random_device.h"

// C++ includes
#include <stdexcept>

#include "qnethernet/platforms/pgmspace.h"

namespace qindesign {
namespace entropy {

extern "C" {
void qnethernet_hal_init_entropy();
void qnethernet_hal_deinit_entropy();
double qnethernet_hal_estimate_entropy(size_t typeSize);
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
  return qnethernet_hal_estimate_entropy(sizeof(result_type));
}

random_device::result_type random_device::operator()() {
  result_type r;
  (*this)(&r, sizeof(r));
  return r;
}

void random_device::operator()(void* const buf, const size_t size) {
  if (qnethernet_hal_fill_entropy(buf, size) != size) {
#if defined(__cpp_exceptions)
    throw std::runtime_error("generation");
#else
    std::__throw_runtime_error("generation");
#endif  // __cpp_exceptions
  }
}

size_t random_device::available() {
  return qnethernet_hal_entropy_available();
}

}  // namespace entropy
}  // namespace qindesign
