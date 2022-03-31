// SPDX-FileCopyrightText: (c) 2021-2022 Shawn Silverman <shawn@pobox.com>
// SPDX-License-Identifier: MIT

// sys_arch.cpp provides system function implementations for lwIP.
// This file is part of the QNEthernet library.

// C includes
#include <unistd.h>

// C++ includes
#include <cstdint>
#include <cstdio>

#include <Print.h>
#include <pgmspace.h>

extern "C" {
#include "lwip/arch.h"
#include "lwip/opt.h"
#ifdef LWIP_DEBUG
#include "lwip/err.h"
#endif  // LWIP_DEBUG

extern volatile uint32_t systick_millis_count;

// Define the heap in RAM2
// See: lwip/mem.c
static DMAMEM LWIP_DECLARE_MEMORY_ALIGNED(the_heap,
                                          LWIP_MEM_ALIGN_SIZE(MEM_SIZE) +
                                              2*LWIP_MEM_ALIGN_SIZE(8));
void *ram_heap = the_heap;

u32_t sys_now(void) {
  return systick_millis_count;
}

#ifdef LWIP_DEBUG
// include\lwip\err.h
const char *lwip_strerr(err_t err) {
  static char buf[16];
  snprintf(buf, sizeof(buf), "err %d", err);
  return buf;
}
#endif  // LWIP_DEBUG
}  // extern "C"

// The user program can set this to something initialized. For example,
// `Serial`, after `Serial.begin(speed)`.
namespace qindesign {
namespace network {

Print *volatile stdPrint = nullptr;

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
  if (file == STDOUT_FILENO || file == STDERR_FILENO) {
    out = ::qindesign::network::stdPrint;
  } else {
    out = (Print *)file;
  }

  if (out == nullptr) {
    return len;
  }
  return out->write((const uint8_t *)buf, len);
}

#if SYS_LIGHTWEIGHT_PROT
sys_prot_t sys_arch_protect(void) {
  return 0;
}

void sys_arch_unprotect(sys_prot_t pval) {
}
#endif  // SYS_LIGHTWEIGHT_PROT
}  // extern "C"
