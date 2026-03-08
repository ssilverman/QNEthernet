// SPDX-FileCopyrightText: (c) 2026 Shawn Silverman <shawn@pobox.com>
// SPDX-License-Identifier: AGPL-3.0-or-later

// qnethernet_hal.h defines some useful HAL utilities.
// This file is part of the QNEthernet library.

// The QNETHERNET_HAL_START_NOINTERRUPTS_BLOCK(lock) and
// QNETHERNET_HAL_END_NOINTERRUPTS_BLOCK(lock) macros either disable and enable
// interrupts, or wrap code in a do-while block that utilizes LDREXW and STREXW.
// In other words, everything between them may or may not be inside a block.
// This means that any variables defined between them need to be declared
// outside the pair of calls.
//
// The 'lock' parameter must be declared somewhere non-local to the macro use.
// It is ignored on non-LDREXW/STREXW targets. It should be volatile.
//
// This is useful for when a platform supports performing a task, but it's
// preferred not to disable interrupts. For example, for lower latency
// or efficiency.
//
// Usage example:
//     uint32_t _nointerrupts_lock;  // Define this somewhere non-local
//     ...
//     QNETHERNET_HAL_START_NOINTERRUPTS_BLOCK(_nointerrupts_lock) {
//       perform_task();
//     } QNETHERNET_HAL_END_NOINTERRUPTS_BLOCK(_nointerrupts_lock);

// TODO: Implemenet LDREX- and STREX-like usage for other platforms

#if defined(TEENSYDUINO) &&                                 \
    (defined(__IMXRT1062__) || defined(ARDUINO_TEENSY36) || \
     defined(ARDUINO_TEENSY35) || defined(ARDUINO_TEENSY32))
#include <arm_math.h>  // For LDREXW and STREXW instructions

// The lock is a uint32_t that guards the operation. It must be declared
// non-local to the macro use.
#define QNETHERNET_HAL_START_NOINTERRUPTS_BLOCK(lock) \
  do {                                                \
    (void)__LDREXW(&(lock));

#define QNETHERNET_HAL_END_NOINTERRUPTS_BLOCK(lock) \
  } while (__STREXW(1, &(lock)) != 0)

#else

extern "C" {
void qnethernet_hal_disable_interrupts();
void qnethernet_hal_enable_interrupts();
}  // extern "C"

// This disables interrupts. 'lock' is ignored.
#define QNETHERNET_HAL_START_NOINTERRUPTS_BLOCK(lock) \
  qnethernet_hal_disable_interrupts();

// This enables interrupts. 'lock' is ignored.
#define QNETHERNET_HAL_END_NOINTERRUPTS_BLOCK(lock) \
  qnethernet_hal_enable_interrupts()

#endif  // Has xREXW instructions
