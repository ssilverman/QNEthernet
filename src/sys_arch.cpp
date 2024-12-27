// SPDX-FileCopyrightText: (c) 2021-2024 Shawn Silverman <shawn@pobox.com>
// SPDX-License-Identifier: AGPL-3.0-or-later

// sys_arch.cpp provides system function implementations for lwIP.
// This file is part of the QNEthernet library.

#include "arch/sys_arch.h"

#include "lwip/sys.h"

// --------------------------------------------------------------------------
//  Time
// --------------------------------------------------------------------------

extern "C" {

uint32_t qnethernet_hal_millis(void);

u32_t sys_now(void) {
#ifndef LWIP_FUZZ_SYS_NOW
  return qnethernet_hal_millis();
#else
  return qnethernet_hal_millis() + sys_now_offset;
#endif  // !LWIP_FUZZ_SYS_NOW
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

void sys_arch_unprotect(const sys_prot_t pval) {
}
#endif  // SYS_LIGHTWEIGHT_PROT

}  // extern "C"
