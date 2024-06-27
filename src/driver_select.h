// SPDX-FileCopyrightText: (c) 2024 Shawn Silverman <shawn@pobox.com>
// SPDX-License-Identifier: AGPL-3.0-or-later

// driver_select.h chooses a driver header to include.
// This file is part of the QNEthernet library.

#pragma once

#include "qnethernet_opts.h"

#if defined(QNETHERNET_DRIVER_W5500)
#include "drivers/driver_w5500.h"
#define QNETHERNET_INTERNAL_DRIVER_W5500
#elif defined(ARDUINO_TEENSY41)
#include "drivers/driver_teensy41.h"
#define QNETHERNET_INTERNAL_DRIVER_TEENSY41
#else
#include "drivers/driver_unsupported.h"
#define QNETHERNET_INTERNAL_DRIVER_UNSUPPORTED
#endif  // Driver selection
