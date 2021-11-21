// SPDX-FileCopyrightText: (c) 2021 Shawn Silverman <shawn@pobox.com>
// SPDX-License-Identifier: MIT

// sys_arch.cpp provides system function implementations for lwIP.
// This file is part of the QNEthernet library.

#include <Arduino.h>

extern "C" {
#include <stdint.h>
#include <stdio.h>

#include "lwip/arch.h"

extern volatile uint32_t systick_millis_count;

u32_t sys_now(void) {
  return systick_millis_count;
}
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
// Note: Can't define as weak because we don't know which `_write` would be
//       chosen by the linker, this one or the one defined in Print.cpp.
// TODO: May have to revisit this if user code wants to define `_write`.
int _write(int file, const void *buf, size_t len) {
  Print *out;

  // Send both stdout and stderr to stdPrint
  if (file == stdout->_file || file == stderr->_file) {
    out = ::qindesign::network::stdPrint;
  } else {
    out = (Print *)file;
  }

  if (out == nullptr) {
    return len;
  }
  // Don't check for len == 0 and return early in case there's side effects
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
