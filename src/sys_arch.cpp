// SPDX-FileCopyrightText: (c) 2021-2024 Shawn Silverman <shawn@pobox.com>
// SPDX-License-Identifier: AGPL-3.0-or-later

// sys_arch.cpp provides system function implementations for lwIP.
// This file is part of the QNEthernet library.

#include "arch/sys_arch.h"

// C++ includes
#include <cstdint>

#include "lwip/opt.h"

// --------------------------------------------------------------------------
//  Time
// --------------------------------------------------------------------------

extern "C" {

uint32_t qnethernet_hal_millis();

u32_t sys_now(void) {
  return qnethernet_hal_millis();
}

}  // extern "C"

// --------------------------------------------------------------------------
//  Core Locking
// --------------------------------------------------------------------------

extern "C" {

#if SYS_LIGHTWEIGHT_PROT
sys_prot_t sys_arch_protect(void) {
  return 0;
}

void sys_arch_unprotect(sys_prot_t pval) {
}
#endif  // SYS_LIGHTWEIGHT_PROT

}  // extern "C"
