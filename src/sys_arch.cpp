// SPDX-FileCopyrightText: (c) 2021 Shawn Silverman <shawn@pobox.com>
// SPDX-License-Identifier: MIT

// sys_arch.cpp provides system function implementations for lwIP.
// This file is part of the QNEthernet library.

#include <Arduino.h>

extern "C" {
#include <stdint.h>
#include <stdio.h>
#include <lwip/arch.h>

extern volatile uint32_t systick_millis_count;

u32_t sys_now(void) {
  return systick_millis_count;
}

// Define this function so that printf works; parts of lwIP may use printf.
// It's marked as "weak" so that programs that define this function
// won't conflict.
__attribute__((weak))
int _write(int file, const char *buf, size_t len) {
  Print *out = nullptr;

  // Send both stdout and stderr to Serial
  if (file == stdout->_file || file == stderr->_file) {
    out = &Serial;
  } else {
    out = reinterpret_cast<Print *>(file);
  }

  if (out != nullptr) {
    return out->write(reinterpret_cast<const uint8_t *>(buf), len);
  } else {
    return len;
  }
}

#if SYS_LIGHTWEIGHT_PROT
sys_prot_t sys_arch_protect(void) {
  return 0;
}

void sys_arch_unprotect(sys_prot_t pval) {
}
#endif  // SYS_LIGHTWEIGHT_PROT
}  // extern "C"
