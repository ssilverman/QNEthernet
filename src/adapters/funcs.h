// SPDX-FileCopyrightText: (c) 2024 Shawn Silverman <shawn@pobox.com>
// SPDX-License-Identifier: AGPL-3.0-or-later

// funcs.h declares Arduino functions used in the library.
// This file is part of the QNEthernet library.

#ifdef __cplusplus
extern "C" {
#endif  // __cplusplus

#if defined(ARDUINO_TEENSY41) || defined(ARDUINO_TEENSY40)

#include <core_pins.h>

#else

unsigned long millis();
void yield();

#endif  // defined(ARDUINO_TEENSY41) || defined(ARDUINO_TEENSY40)

#ifdef __cplusplus
}  // extern "C"
#endif  // __cplusplus
