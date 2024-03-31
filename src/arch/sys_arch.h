// SPDX-FileCopyrightText: (c) 2023-2024 Shawn Silverman <shawn@pobox.com>
// SPDX-License-Identifier: AGPL-3.0-or-later

// sys_arch.h supports the system functions for lwIP.
// This file is part of the QNEthernet library.

// C++ includes
#include <cstdint>

#include "lwip/opt.h"

#if SYS_LIGHTWEIGHT_PROT
typedef uint32_t sys_prot_t;
#endif /* SYS_LIGHTWEIGHT_PROT */
