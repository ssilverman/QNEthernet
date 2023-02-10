// SPDX-FileCopyrightText: (c) 2021-2023 Shawn Silverman <shawn@pobox.com>
// SPDX-License-Identifier: MIT

// lwIP configuration for QNEthernet library on Teensy 4.1.
// This file is part of the QNEthernet library.

#ifndef QNETHERNET_ARCH_CC_H_
#define QNETHERNET_ARCH_CC_H_

#include <stdlib.h>

#define LWIP_RAND() ((u32_t)rand())

#endif  // QNETHERNET_ARCH_CC_H_
