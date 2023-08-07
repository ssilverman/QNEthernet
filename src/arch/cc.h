// SPDX-FileCopyrightText: (c) 2021-2023 Shawn Silverman <shawn@pobox.com>
// SPDX-License-Identifier: MIT

// cc.h is the lwIP architecture configuration.
// This file is part of the QNEthernet library.

#ifndef QNETHERNET_ARCH_CC_H_
#define QNETHERNET_ARCH_CC_H_

// C includes
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

#include <pgmspace.h>

#define LWIP_RAND() ((u32_t)rand())

#define LWIP_PLATFORM_ASSERT(x)                          \
  do {                                                   \
    printf("Assertion \"%s\" failed at line %d in %s\n", \
           x, __LINE__, __FILE__);                       \
    fflush(NULL);                                        \
    qnethernet_stdio_flush(STDOUT_FILENO);               \
    abort();                                             \
  } while (0)
extern void qnethernet_stdio_flush(int file);

#ifndef QNETHERNET_MEMORY_IN_RAM1
#define LWIP_DECLARE_MEMORY_ALIGNED(variable_name, size) \
  u8_t variable_name[(size)] DMAMEM __attribute__((aligned(MEM_ALIGNMENT)))
#else
#define LWIP_DECLARE_MEMORY_ALIGNED(variable_name, size) \
  u8_t variable_name[(size)] __attribute__((aligned(MEM_ALIGNMENT)))
#endif  // !QNETHERNET_MEMORY_IN_RAM1

#if (__BYTE_ORDER__ == __ORDER_LITTLE_ENDIAN__)
#define lwip_htons(x) __builtin_bswap16(x)
#define lwip_htonl(x) __builtin_bswap32(x)
#endif  // __BYTE_ORDER__ == __ORDER_LITTLE_ENDIAN__

#endif  // QNETHERNET_ARCH_CC_H_
