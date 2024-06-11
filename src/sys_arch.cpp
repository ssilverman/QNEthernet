// SPDX-FileCopyrightText: (c) 2021-2024 Shawn Silverman <shawn@pobox.com>
// SPDX-License-Identifier: AGPL-3.0-or-later

// sys_arch.cpp provides system function implementations for lwIP.
// This file is part of the QNEthernet library.

#include "arch/sys_arch.h"

// C includes
#include <unistd.h>

// C++ includes
#include <cstdint>
#include <cstdio>
#include <limits>

#include "lwip/arch.h"
#include "lwip/debug.h"
#include "lwip/opt.h"
#ifdef LWIP_DEBUG
#include "lwip/err.h"
#endif  // LWIP_DEBUG

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
//  Error-to-String
// --------------------------------------------------------------------------

// Returns the size of the given array.
template <typename T, size_t N>
constexpr size_t countof(const T (&)[N]) {
  return N;
}

extern "C" {

#ifdef LWIP_DEBUG
// include\lwip\err.h

static const char *err_strerr[]{
    "Ok",
    "Out of memory error",
    "Buffer error",
    "Timeout",
    "Routing problem",
    "Operation in progress",
    "Illegal value",
    "Operation would block",
    "Address in use",
    "Already connecting",
    "Conn already established",
    "Not connected",
    "Low-level netif error",
    "Connection aborted",
    "Connection reset",
    "Connection closed",
    "Illegal argument",
};

const char *lwip_strerr(err_t err) {
  if (0 <= -err && -err < static_cast<err_t>(countof(err_strerr))) {
    return err_strerr[-err];
  }

  static constexpr size_t kDigits = std::numeric_limits<err_t>::digits10 + 1;  // Add 1 for ceiling
  static constexpr char kPrefix[]{"err "};
  static char buf[sizeof(kPrefix) + kDigits + 1];  // Includes the NUL and sign
  std::snprintf(buf, sizeof(buf), "%s%d", kPrefix, err);
  return buf;
}

#endif  // LWIP_DEBUG

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
