// SPDX-FileCopyrightText: (c) 2021-2022 Shawn Silverman <shawn@pobox.com>
// SPDX-License-Identifier: MIT

// sys_arch.cpp provides system function implementations for lwIP.
// This file is part of the QNEthernet library.

// C includes
#include <unistd.h>

// C++ includes
#include <cerrno>
#undef errno
extern int errno;
#include <cstdint>
#include <cstdio>

#include <Print.h>
#include <pgmspace.h>

extern "C" {

#include "lwip/arch.h"
#include "lwip/debug.h"
#include "lwip/opt.h"
#ifdef LWIP_DEBUG
#include "lwip/err.h"
#endif  // LWIP_DEBUG

extern volatile uint32_t systick_millis_count;

// Define the heap in RAM2
// See: lwip/mem.c
static DMAMEM LWIP_DECLARE_MEMORY_ALIGNED(the_heap,
                                          LWIP_MEM_ALIGN_SIZE(MEM_SIZE) +
                                              2U*LWIP_MEM_ALIGN_SIZE(8U));
void *ram_heap = the_heap;

u32_t sys_now(void) {
  return systick_millis_count;
}

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

  constexpr double kLog2 = 0.301029995663981;
  // # digits = log_10(2^bits) = bits * log_10(2)
  constexpr size_t kDigits = sizeof(err_t)*8*kLog2 + 1;  // Add 1 for ceiling
  constexpr char kPrefix[]{"err "};
  static char buf[sizeof(kPrefix) + kDigits + 1];  // Includes the NUL and sign
  snprintf(buf, sizeof(buf), "%s%d", kPrefix, err);
  return buf;
}
#endif  // LWIP_DEBUG

}  // extern "C"

// --------------------------------------------------------------------------
//  stdio
// --------------------------------------------------------------------------

// The user program can set this to something initialized. For example,
// `Serial`, after `Serial.begin(speed)`.
namespace qindesign {
namespace network {

Print *volatile stdPrint = nullptr;
Print *volatile stderrPrint = nullptr;

}  // namespace network
}  // namespace qindesign

extern "C" {

// Define this function so that printf works; parts of lwIP may use printf.
// See: https://forum.pjrc.com/threads/28473-Quick-Guide-Using-printf()-on-Teensy-ARM
// Note: Can't define as weak by default because we don't know which `_write`
//       would be chosen by the linker, this one or the one defined
//       in Print.cpp.
#ifdef QNETHERNET_WEAK_WRITE
__attribute__((weak))
#endif
int _write(int file, const void *buf, size_t len) {
  if (len == 0) {
    return 0;
  }

  Print *out;

  // Send both stdout and stderr to stdPrint
  if (file == STDOUT_FILENO) {
    out = ::qindesign::network::stdPrint;
  } else if (file == STDERR_FILENO) {
    if (::qindesign::network::stderrPrint == nullptr) {
      out = ::qindesign::network::stdPrint;
    } else {
      out = ::qindesign::network::stderrPrint;
    }
  } else if (file == STDIN_FILENO) {
    errno = EBADF;
    return -1;
  } else {
    out = (Print *)file;
  }

  if (out == nullptr) {
    return len;
  }
  return out->write((const uint8_t *)buf, len);
}

// --------------------------------------------------------------------------
//  Core Locking
// --------------------------------------------------------------------------

#if SYS_LIGHTWEIGHT_PROT
typedef uint32_t sys_prot_t;

sys_prot_t sys_arch_protect(void) {
  return 0;
}

void sys_arch_unprotect(sys_prot_t pval) {
}
#endif  // SYS_LIGHTWEIGHT_PROT

void sys_check_core_locking(void) {
  uint32_t ipsr;
  __asm__ volatile("mrs %0, ipsr\n" : "=r" (ipsr) ::);
  LWIP_ASSERT("Function called from interrupt context", (ipsr == 0));
}

}  // extern "C"
