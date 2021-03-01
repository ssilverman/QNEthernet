// lwIP on Teensy 4.1
// (c) 2021 Shawn Silverman

#ifndef LWIPTEENSY_ARCH_CC_H_
#define LWIPTEENSY_ARCH_CC_H_

#include <stdint.h>

#ifndef BYTE_ORDER  // May be defined by the system
#define BYTE_ORDER LITTLE_ENDIAN
#endif  // BYTE_ORDER
#define LWIP_ERRNO_STDINCLUDE 1

typedef uint32_t sys_prot_t;

#endif  // #ifndef LWIPTEENSY_ARCH_CC_H_
