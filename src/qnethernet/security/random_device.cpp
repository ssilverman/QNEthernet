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
uint32_t qnethernet_hal_entropy();
}  // extern "C"

// Device ensures there's only one initialization and deinitialization.
class Device {
 public:
  Device() {
    qnethernet_hal_init_entropy();
  }

  ~Device() noexcept {
    qnethernet_hal_deinit_entropy();
  }

  // Disallow copying and moving
  Device(const Device&) = delete;
  Device(Device&&) = delete;
  Device& operator=(const Device&) = delete;
  Device& operator=(Device&&) = delete;
};

// Initialize entropy.
static Device device;

random_device::random_device() : random_device("default") {}

random_device::random_device(const std::string& token) {}

double random_device::entropy() const {
  return qnethernet_hal_estimate_entropy();
}

random_device::result_type random_device::operator()() {
  return qnethernet_hal_entropy();
}

}  // namespace security
}  // namespace qindesign
