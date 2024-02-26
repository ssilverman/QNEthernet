// SPDX-FileCopyrightText: (c) 2021-2024 Shawn Silverman <shawn@pobox.com>
// SPDX-License-Identifier: AGPL-3.0-or-later

// sys_arch.cpp provides system function implementations for lwIP.
// This file is part of the QNEthernet library.

// C includes
#include <unistd.h>

#include "qnethernet_opts.h"

// C++ includes
#if QNETHERNET_ENABLE_CUSTOM_WRITE
#include <cerrno>
#endif  // QNETHERNET_ENABLE_CUSTOM_WRITE
#include <cstdint>
#include <cstdio>
#include <limits>

#include <Print.h>

#include "security/RandomDevice.h"

extern "C" {

#include "adapters/funcs.h"
#include "adapters/pgmspace.h"
#include "lwip/arch.h"
#include "lwip/debug.h"
#include "lwip/opt.h"
#ifdef LWIP_DEBUG
#include "lwip/err.h"
#endif  // LWIP_DEBUG

u32_t sys_now(void) {
  return millis();
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

  static constexpr size_t kDigits = std::numeric_limits<err_t>::digits10 + 1;  // Add 1 for ceiling
  static constexpr char kPrefix[]{"err "};
  static char buf[sizeof(kPrefix) + kDigits + 1];  // Includes the NUL and sign
  snprintf(buf, sizeof(buf), "%s%d", kPrefix, err);
  return buf;
}
#endif  // LWIP_DEBUG

}  // extern "C"

// --------------------------------------------------------------------------
//  stdio
// --------------------------------------------------------------------------

#if QNETHERNET_ENABLE_CUSTOM_WRITE

// The user program can set these to something initialized. For example,
// `&Serial`, after `Serial.begin(speed)`.
namespace qindesign {
namespace network {

Print *volatile stdoutPrint = nullptr;
Print *volatile stderrPrint = nullptr;

}  // namespace network
}  // namespace qindesign

#else

#include <Arduino.h>  // For Serial

#endif  // QNETHERNET_ENABLE_CUSTOM_WRITE

extern "C" {

// Gets the Print* for the given file descriptor.
static inline Print *getPrint(int file) {
  switch (file) {
#if QNETHERNET_ENABLE_CUSTOM_WRITE
    case STDOUT_FILENO:
      return ::qindesign::network::stdoutPrint;
    case STDERR_FILENO:
      return ::qindesign::network::stderrPrint;
#else
    case STDOUT_FILENO:
    case STDERR_FILENO:
      return &Serial;
#endif  // QNETHERNET_ENABLE_CUSTOM_WRITE
    case STDIN_FILENO:
      return nullptr;
    default:
      return reinterpret_cast<Print *>(file);
  }
}

#if QNETHERNET_ENABLE_CUSTOM_WRITE

// Define this function to provide expanded stdio output behaviour.
// See: https://forum.pjrc.com/threads/28473-Quick-Guide-Using-printf()-on-Teensy-ARM
// Note: Can't define as weak by default because we don't know which `_write`
//       would be chosen by the linker, this one or the one defined
//       in Print.cpp.
int _write(int file, const void *buf, size_t len) {
  Print *out = getPrint(file);
  if (out == nullptr) {
    errno = EBADF;
    return -1;
  }

  return out->write(static_cast<const uint8_t *>(buf), len);
}

#endif  // QNETHERNET_ENABLE_CUSTOM_WRITE

// Ensures the Print object is flushed because fflush() just flushes by writing
// to the FILE*. This doesn't necessarily send all the bytes right away. For
// example, Serial/USB output behaves this way.
void qnethernet_stdio_flush(int file) {
  Print *p = getPrint(file);
  if (p != nullptr) {
    p->flush();
  }
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

void sys_check_core_locking(const char *file, int line, const char *func) {
#if defined(TEENSYDUINO) && defined(__IMXRT1062__)
  uint32_t ipsr;
  __asm__ volatile("mrs %0, ipsr\n" : "=r" (ipsr) ::);
  if (ipsr != 0) {
    printf("%s:%d:%s()\r\n", file, line, func);
    LWIP_PLATFORM_ASSERT("Function called from interrupt context");
  }
#endif  // defined(TEENSYDUINO) && defined(__IMXRT1062__)
}

// --------------------------------------------------------------------------
//  Randomness
// --------------------------------------------------------------------------

// Gets a 32-bit random number for LWIP_RAND().
uint32_t qnethernet_rand() {
  return qindesign::security::RandomDevice::instance()();
}

}  // extern "C"
