// SPDX-FileCopyrightText: (c) 2024 Shawn Silverman <shawn@pobox.com>
// SPDX-License-Identifier: AGPL-3.0-or-later

// driver_select.h chooses a driver header to include.
// This file is part of the QNEthernet library.

#pragma once

#include "qnethernet_opts.h"

// --------------------------------------------------------------------------
//  External Driver
// --------------------------------------------------------------------------

// First check for the existence of an external driver
// https://forum.pjrc.com/index.php?threads/new-lwip-based-ethernet-library-for-teensy-4-1.68066/post-345539
#if defined(__has_include)
// https://gcc.gnu.org/onlinedocs/cpp/_005f_005fhas_005finclude.html
#if __has_include(<qnethernet_external_driver.h>)
#include <qnethernet_external_driver.h>
#define QNETHERNET_INTERNAL_DRIVER_EXTERNAL
#endif  // __has_include(<qnethernet_external_driver.h>)
#endif  // defined(__has_include)

// --------------------------------------------------------------------------
//  No External Driver
// --------------------------------------------------------------------------

// Don't include anything else if an external driver has been included
#ifndef QNETHERNET_INTERNAL_DRIVER_EXTERNAL

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

#endif  // QNETHERNET_INTERNAL_DRIVER_EXTERNAL
