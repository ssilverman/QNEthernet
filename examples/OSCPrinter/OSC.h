// SPDX-FileCopyrightText: (c) 2021-2022,2024 Shawn Silverman <shawn@pobox.com>
// SPDX-License-Identifier: AGPL-3.0-or-later

// OSC.h contains OSC function declarations.
// This file is part of the QNEthernet library.

#pragma once

// C++ includes
#include <cstdint>

#include <Print.h>

// Prints an OSC message, either a bundle or not.
void printOSC(Print &out, const uint8_t *b, int len);
