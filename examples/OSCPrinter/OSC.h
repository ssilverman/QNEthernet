// SPDX-FileCopyrightText: (c) 2021-2022 Shawn Silverman <shawn@pobox.com>
// SPDX-License-Identifier: MIT

// OSC.h contains OSC function declarations.
// This file is part of the QNEthernet library.

#ifndef OSC_H_
#define OSC_H_

// C++ includes
#include <cstdint>

#include <Print.h>

// Prints an OSC message, either a bundle or not.
void printOSC(Print &out, const uint8_t *b, int len);

#endif  // OSC_H_
