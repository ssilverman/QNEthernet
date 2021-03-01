// lwIP on Teensy 4.1
// (c) 2021 Shawn Silverman

#ifndef LWIPTEENSY_LWIPOPTS_H_
#define LWIPTEENSY_LWIPOPTS_H_

#define NO_SYS 1
#define SYS_LIGHTWEIGHT_PROT 0
#define MEM_LIBC_MALLOC 1
// #define MEM_ALIGNMENT 4
// #define MEM_SIZE 10240
// #define LWIP_NETIF_API 0
#define LWIP_NETCONN 0
#define LWIP_SOCKET 0

#endif  // LWIPTEENSY_LWIPOPTS_H_
