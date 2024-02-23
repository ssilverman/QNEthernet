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

#include "adapters/pgmspace.h"
#include "qnethernet_opts.h"

#define LWIP_RAND() qnethernet_rand()
extern uint32_t qnethernet_rand();

#define LWIP_PLATFORM_ASSERT(x)                          \
  do {                                                   \
    printf("Assertion \"%s\" failed at line %d in %s\n", \
           x, __LINE__, __FILE__);                       \
    fflush(NULL);                                        \
    qnethernet_stdio_flush(STDOUT_FILENO);               \
    abort();                                             \
  } while (0)
extern void qnethernet_stdio_flush(int file);

#if !QNETHERNET_LWIP_MEMORY_IN_RAM1
#define LWIP_DECLARE_MEMORY_ALIGNED(variable_name, size) \
  alignas(MEM_ALIGNMENT) u8_t variable_name[(size)] DMAMEM
#else
#define LWIP_DECLARE_MEMORY_ALIGNED(variable_name, size) \
  alignas(MEM_ALIGNMENT) u8_t variable_name[(size)]
#endif  // !QNETHERNET_LWIP_MEMORY_IN_RAM1

#if (__BYTE_ORDER__ == __ORDER_LITTLE_ENDIAN__)
#define lwip_htons(x) __builtin_bswap16(x)
#define lwip_htonl(x) __builtin_bswap32(x)
#endif  // __BYTE_ORDER__ == __ORDER_LITTLE_ENDIAN__
