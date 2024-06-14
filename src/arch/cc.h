// SPDX-FileCopyrightText: (c) 2021-2024 Shawn Silverman <shawn@pobox.com>
// SPDX-License-Identifier: AGPL-3.0-or-later

// cc.h is the lwIP architecture configuration.
// This file is part of the QNEthernet library.

#pragma once

// C includes
#include <stdalign.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

#include <avr/pgmspace.h>

#include "qnethernet_opts.h"

// Define this for err_to_errno()
#define LWIP_ERRNO_STDINCLUDE

#ifdef __cplusplus
extern "C" {
#endif  // __cplusplus

#define LWIP_RAND() qnethernet_hal_rand()
uint32_t qnethernet_hal_rand();

#define LWIP_PLATFORM_ASSERT(x)                          \
  do {                                                   \
    printf("Assertion \"%s\" failed at line %d in %s\n", \
           (x), __LINE__, __FILE__);                     \
    fflush(NULL);                                        \
    qnethernet_hal_stdio_flush(STDOUT_FILENO);           \
    abort();                                             \
  } while (0)
void qnethernet_hal_stdio_flush(int file);

#if !QNETHERNET_LWIP_MEMORY_IN_RAM1 && \
    (defined(TEENSYDUINO) && defined(__IMXRT1062__))
#define LWIP_DECLARE_MEMORY_ALIGNED(variable_name, size) \
  alignas(MEM_ALIGNMENT) u8_t variable_name[(size)] DMAMEM
#else
#define LWIP_DECLARE_MEMORY_ALIGNED(variable_name, size) \
  alignas(MEM_ALIGNMENT) u8_t variable_name[(size)]
#endif  // !QNETHERNET_LWIP_MEMORY_IN_RAM1

// Byte swapping
#if (__BYTE_ORDER__ == __ORDER_LITTLE_ENDIAN__)
#if defined(__has_builtin)

#if __has_builtin(__builtin_bswap16)
#define lwip_htons(x) __builtin_bswap16(x)
#endif  // __has_builtin(__builtin_bswap16)

#if __has_builtin(__builtin_bswap32)
#define lwip_htonl(x) __builtin_bswap32(x)
#endif  // __has_builtin(__builtin_bswap32)

#endif  // defined(__has_builtin)
#endif  // __BYTE_ORDER__ == __ORDER_LITTLE_ENDIAN__

// Other functions
#define lwip_itoa(result, bufsize, number) \
  snprintf((result), (bufsize), "%d", (number))

#ifdef __cplusplus
}  // extern "C"
#endif  // __cplusplus
