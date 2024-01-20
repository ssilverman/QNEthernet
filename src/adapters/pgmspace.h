// SPDX-FileCopyrightText: (c) 2024 Shawn Silverman <shawn@pobox.com>
// SPDX-License-Identifier: AGPL-3.0-or-later

// pgmspace.h includes the system pgmspace.h and ensures certain variables
// are defined.
// This file is part of the QNEthernet library.

#pragma once

#include <pgmspace.h>

#ifndef DMAMEM
#define DMAMEM
#endif  // !defined(DMAMEM)

#ifndef FLASHMEM
#define FLASHMEM
#endif  // !defined(FLASHMEM)
