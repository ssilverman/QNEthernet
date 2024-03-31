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

#if defined(TEENSYDUINO)
extern volatile uint32_t systick_millis_count;
u32_t sys_now(void) {
  return systick_millis_count;
}
#else
unsigned long millis();
u32_t sys_now(void) {
  return millis();
}
#endif  // defined(TEENSYDUINO)

}  // extern "C"

// --------------------------------------------------------------------------
//  Error-to-String
// --------------------------------------------------------------------------

extern "C" {

#ifdef LWIP_DEBUG
// include\lwip\err.h
const char *lwip_strerr(err_t err) {
  switch (err) {
    case ERR_OK:         return "err Ok";
    case ERR_MEM:        return "err Out of Memory";
    case ERR_BUF:        return "err Buffer";
    case ERR_TIMEOUT:    return "err Timeout";
    case ERR_RTE:        return "err Routing Problem";
    case ERR_INPROGRESS: return "err Operation in Progress";
    case ERR_VAL:        return "err Illegal Value";
    case ERR_WOULDBLOCK: return "err Operation Would Block";
    case ERR_USE:        return "err Address in Use";
    case ERR_ALREADY:    return "err Already Connecting";
    case ERR_ISCONN:     return "err Connection Already Established";
    case ERR_CONN:       return "err Not Connected";
    case ERR_IF:         return "err Low-Level netif";
    case ERR_ABRT:       return "err Connection Aborted";
    case ERR_RST:        return "err Connection Reset";
    case ERR_CLSD:       return "err Connection Closed";
    case ERR_ARG:        return "err Illegal Argument";
    default:
      break;
  }

  static constexpr size_t kDigits = std::numeric_limits<err_t>::digits10 + 1;  // Add 1 for ceiling
  static constexpr char kPrefix[]{"err "};
  static char buf[sizeof(kPrefix) + kDigits + 1];  // Includes the NUL and sign
  snprintf(buf, sizeof(buf), "%s%d", kPrefix, err);
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

void sys_check_core_locking(const char *file, int line, const char *func) {
#if defined(TEENSYDUINO) && defined(__IMXRT1062__)
  uint32_t ipsr;
  __asm__ volatile("mrs %0, ipsr\n" : "=r" (ipsr) ::);
  if (ipsr != 0) {
    printf("%s:%d:%s()\r\n", file, line, func);
    LWIP_PLATFORM_ASSERT("Function called from interrupt context");
  }
#else
  LWIP_UNUSED_ARG(file);
  LWIP_UNUSED_ARG(line);
  LWIP_UNUSED_ARG(func);
#endif  // defined(TEENSYDUINO) && defined(__IMXRT1062__)
}

}  // extern "C"
