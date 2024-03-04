// SPDX-FileCopyrightText: (c) 2024 Shawn Silverman <shawn@pobox.com>
// SPDX-License-Identifier: AGPL-3.0-or-later

// funcs.h declares Arduino functions used in the library. This avoids having
// to include <Arduino.h>, which might include a lot of stuff. It is expected
// that this file might need to change, depending on what's being compiled.
// This file is part of the QNEthernet library.

#pragma once

#if defined(TEENSYDUINO)

// Teensyduino (currently at v1.59) declares millis() as static, so we need to
// include this header instead of declaring the function ourselves.
#include <core_pins.h>

#else

#ifdef __cplusplus
extern "C" {
#endif  // __cplusplus

unsigned long millis();
void yield();

#ifdef __cplusplus
}  // extern "C"
#endif  // __cplusplus

#endif  // defined(TEENSYDUINO)
