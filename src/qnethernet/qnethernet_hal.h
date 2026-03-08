// SPDX-FileCopyrightText: (c) 2026 Shawn Silverman <shawn@pobox.com>
// SPDX-License-Identifier: AGPL-3.0-or-later

// qnethernet_hal.h defines some useful HAL utilities.
// This file is part of the QNEthernet library.

// The QNETHERNET_HAL_START_NOINTERRUPTS_BLOCK() and
// QNETHERNET_HAL_END_NOINTERRUPTS_BLOCK() macros either disable and enable
// interrupts, or wrap code in a do-while block that utilizes LDREXW and STREXW.
// In other words, everything between them may or may not be inside a block.
// This means that any variables defined between them need to be declared
// outside the pair of calls.
//
// The 'monitor' parameter must be declared somewhere non-local to the
// macro use.
//
// This is useful for when a platform supports performing a task, but it's
// preferred not to disable interrupts. For example, for lower latency
// or efficiency.
//
// Usage example:
//     uint32_t _nointerrupts_monitor;  // Define this somewhere non-local
//     ...
//     QNETHERNET_HAL_START_NOINTERRUPTS_BLOCK(_nointerrupts_monitor) {
//       perform_task();
//     } QNETHERNET_HAL_END_NOINTERRUPTS_BLOCK(_nointerrupts_monitor);

// TODO: Implemenet LDREX- and STREX-like usage for other platforms

#if defined(TEENSYDUINO) &&                                 \
    (defined(__IMXRT1062__) || defined(ARDUINO_TEENSY36) || \
     defined(ARDUINO_TEENSY35) || defined(ARDUINO_TEENSY32))
#include <arm_math.h>  // For LDREXW and STREXW instructions

// The monitor is a uint32_t that guards the operation. It must be declared
// non-local to the macro use.
#define QNETHERNET_HAL_START_NOINTERRUPTS_BLOCK(monitor) \
  do {                                                   \
    (void)__LDREXW(&(monitor));

#define QNETHERNET_HAL_END_NOINTERRUPTS_BLOCK(monitor) \
  } while (__STREXW(1, &(monitor)) != 0)

#else

extern "C" {
void qnethernet_hal_disable_interrupts();
void qnethernet_hal_enable_interrupts();
}  // extern "C"

#define QNETHERNET_HAL_START_NOINTERRUPTS_BLOCK(monitor) \
  qnethernet_hal_disable_interrupts();

#define QNETHERNET_HAL_END_NOINTERRUPTS_BLOCK(monitor) \
  qnethernet_hal_enable_interrupts()

#endif  // Has xREXW instructions
