// SPDX-FileCopyrightText: (c) 2025 Shawn Silverman <shawn@pobox.com>
// SPDX-License-Identifier: AGPL-3.0-or-later

// pgmspace.h provides platform-specific <avr/pgmspace.h> contents
// This file is part of the QNEthernet library.

#if defined(__has_include)
#if __has_include(<avr/pgmspace.h>)
#include <avr/pgmspace.h>
#elif __has_include(<pgmspace.h>)
#include <pgmspace.h>
#endif  // __has_include(<avr/pgmspace.h> || <pgmspace.h>)
#endif  // __has_include

#ifndef FLASHMEM
#define FLASHMEM
#endif  // !FLASHMEM
