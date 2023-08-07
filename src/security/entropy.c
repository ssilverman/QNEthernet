// SPDX-FileCopyrightText: (c) 2023 Shawn Silverman <shawn@pobox.com>
// SPDX-License-Identifier: MIT

// entropy.cpp implements the TRNG functions.
// This file is part of the QNEthernet library.

#if defined(__IMXRT1062__)
#if !defined(QNETHERNET_USE_ENTROPY_LIB)

#include "entropy.h"

// C includes
#include <errno.h>
#include <string.h>

#include <imxrt.h>
#include <pgmspace.h>

// Clock settings
#define TRNG_CONFIG_CLOCK_MODE   0  /* 0=Ring Oscillator, 1=System Clock (test use only) */
#define TRNG_CONFIG_RING_OSC_DIV 0  /* Divide by 2^n */

// Sampling
#define TRNG_CONFIG_SAMPLE_MODE      2  /* 0:Von Neumann in both, 1:raw in both, 2:VN Entropy and raw in stats */
#define TRNG_CONFIG_SPARSE_BIT_LIMIT 63

// Seed control
#define TRNG_CONFIG_ENTROPY_DELAY 3200
#define TRNG_CONFIG_SAMPLE_SIZE   2500/*512*/

// Statistical check parameters
#define TRNG_CONFIG_RETRY_COUNT   1
#define TRNG_CONFIG_RUN_MAX_LIMIT 34/*32*/

#define TRNG_CONFIG_MONOBIT_MAX       1384/*317*/
#define TRNG_CONFIG_MONOBIT_RANGE     268/*122*/
#define TRNG_CONFIG_RUNBIT1_MAX       405/*107*/
#define TRNG_CONFIG_RUNBIT1_RANGE     178/*80*/
#define TRNG_CONFIG_RUNBIT2_MAX       220/*62*/
#define TRNG_CONFIG_RUNBIT2_RANGE     122/*55*/
#define TRNG_CONFIG_RUNBIT3_MAX       125/*39*/
#define TRNG_CONFIG_RUNBIT3_RANGE     88/*39*/
#define TRNG_CONFIG_RUNBIT4_MAX       75/*26*/
#define TRNG_CONFIG_RUNBIT4_RANGE     64/*26*/
#define TRNG_CONFIG_RUNBIT5_MAX       47/*18*/
#define TRNG_CONFIG_RUNBIT5_RANGE     46/*18*/
#define TRNG_CONFIG_RUNBIT6PLUS_MAX   47/*17*/
#define TRNG_CONFIG_RUNBIT6PLUS_RANGE 46/*17*/

// Limits for statistical check of "Poker Test"
#define TRNG_CONFIG_POKER_MAX   26912/*1600*/
#define TRNG_CONFIG_POKER_RANGE 2467/*570*/

// Limits for statistical check of entropy sample frequency count
#define TRNG_CONFIG_FREQUENCY_MAX 25600/*30000*/
#define TRNG_CONFIG_FREQUENCY_MIN 1600

// Security configuration
#define TRNG_CONFIG_LOCK 0

// Additional bit-setting defines
#define TRNG_SBLIM_SB_LIM(n)    ((uint32_t)(((n) & 0x000003ff) << 0))
#define TRNG_PKRMAX_PKR_MAX(n)  ((uint32_t)(((n) & 0x00ffffff) << 0))
#define TRNG_PKRRNG_PKR_RNG(n)  ((uint32_t)(((n) & 0x0000ffff) << 0))
#define TRNG_FRQMAX_FRQ_MAX(n)  ((uint32_t)(((n) & 0x003fffff) << 0))
#define TRNG_FRQMIN_FRQ_MIN(n)  ((uint32_t)(((n) & 0x003fffff) << 0))
#define TRNG_SEC_CFG_NO_PROG(n) ((uint32_t)(((n) & 0x01) << 1))

#define CLRSET(reg, clear, set) ((reg) = ((reg) & ~(clear)) | (set))

// Entropy storage
#define ENTROPY_COUNT       16                      /* In dwords */
#define ENTROPY_COUNT_BYTES ((ENTROPY_COUNT) << 2)  /* In bytes */
static uint32_t s_entropy[ENTROPY_COUNT] DMAMEM;
static size_t s_entropySizeBytes = 0;  // Size in bytes

bool trng_is_started() {
  // Two checks:
  // 1. Clock: check only the run-only bit because that's always set when running
  // 2. "OK to stop" bit: asserted if the ring oscillator isn't running
  return ((CCM_CCGR6 & CCM_CCGR6_TRNG(CCM_CCGR_ON_RUNONLY)) ==
          CCM_CCGR6_TRNG(CCM_CCGR_ON_RUNONLY)) &&
         ((TRNG_MCTL & TRNG_MCTL_TSTOP_OK) == 0);
}

// Restarts entropy generation.
static void restartEntropy() {
  TRNG_ENT15;
  TRNG_ENT0;  // Dummy read for defect workaround
  s_entropySizeBytes = 0;
}

FLASHMEM void trng_init() {
  // Enable the clock
  CCM_CCGR6 |= CCM_CCGR6_TRNG(CCM_CCGR_ON);

  // Set program mode, clear pending errors, reset registers to default
  TRNG_MCTL = TRNG_MCTL_PRGM | TRNG_MCTL_ERR | TRNG_MCTL_RST_DEF;

  // Apply configuration
  TRNG_SCMISC = TRNG_SCMISC_RTY_CT(TRNG_CONFIG_RETRY_COUNT) |
                TRNG_SCMISC_LRUN_MAX(TRNG_CONFIG_RUN_MAX_LIMIT);
  TRNG_SCML   = TRNG_SCML_MONO_RNG(TRNG_CONFIG_MONOBIT_RANGE) |
                TRNG_SCML_MONO_MAX(TRNG_CONFIG_MONOBIT_MAX);
  TRNG_SCR1L  = TRNG_SCR1L_RUN1_RNG(TRNG_CONFIG_RUNBIT1_RANGE) |
                TRNG_SCR1L_RUN1_MAX(TRNG_CONFIG_RUNBIT1_MAX);
  TRNG_SCR2L  = TRNG_SCR1L_RUN1_RNG(TRNG_CONFIG_RUNBIT2_RANGE) |
                TRNG_SCR1L_RUN1_MAX(TRNG_CONFIG_RUNBIT2_MAX);
  TRNG_SCR3L  = TRNG_SCR1L_RUN1_RNG(TRNG_CONFIG_RUNBIT3_RANGE) |
                TRNG_SCR1L_RUN1_MAX(TRNG_CONFIG_RUNBIT3_MAX);
  TRNG_SCR4L  = TRNG_SCR1L_RUN1_RNG(TRNG_CONFIG_RUNBIT4_RANGE) |
                TRNG_SCR1L_RUN1_MAX(TRNG_CONFIG_RUNBIT4_MAX);
  TRNG_SCR5L  = TRNG_SCR1L_RUN1_RNG(TRNG_CONFIG_RUNBIT5_RANGE) |
                TRNG_SCR1L_RUN1_MAX(TRNG_CONFIG_RUNBIT5_MAX);
  TRNG_SCR5L  = TRNG_SCR1L_RUN1_RNG(TRNG_CONFIG_RUNBIT6PLUS_RANGE) |
                TRNG_SCR1L_RUN1_MAX(TRNG_CONFIG_RUNBIT6PLUS_MAX);
  TRNG_PKRMAX = TRNG_PKRMAX_PKR_MAX(TRNG_CONFIG_POKER_MAX);
  TRNG_PKRRNG = TRNG_PKRRNG_PKR_RNG(TRNG_CONFIG_POKER_RANGE);
  TRNG_FRQMAX = TRNG_FRQMAX_FRQ_MAX(TRNG_CONFIG_FREQUENCY_MAX);
  TRNG_FRQMIN = TRNG_FRQMIN_FRQ_MIN(TRNG_CONFIG_FREQUENCY_MIN);

  // Clock settings
  CLRSET(TRNG_MCTL, TRNG_MCTL_FOR_SCLK,
         TRNG_CONFIG_CLOCK_MODE ? TRNG_MCTL_FOR_SCLK : 0);
  CLRSET(TRNG_MCTL, TRNG_MCTL_OSC_DIV(3),
         TRNG_MCTL_OSC_DIV(TRNG_CONFIG_RING_OSC_DIV));

  // Sampling
  CLRSET(TRNG_MCTL, TRNG_MCTL_SAMP_MODE(3),
         TRNG_MCTL_SAMP_MODE(TRNG_CONFIG_SAMPLE_MODE));
  TRNG_SBLIM = TRNG_SBLIM_SB_LIM(TRNG_CONFIG_SPARSE_BIT_LIMIT);

  // Seed control
  TRNG_SDCTL = TRNG_SDCTL_ENT_DLY(TRNG_CONFIG_ENTROPY_DELAY) |
               TRNG_SDCTL_SAMP_SIZE(TRNG_CONFIG_SAMPLE_SIZE);

  // CLRSET(TRNG_SCMISC, TRNG_SCMISC_LRUN_MAX, TRNG_CONFIG_RUN_MAX_LIMIT);

  // Security configuration
  CLRSET(TRNG_SEC_CFG, TRNG_SEC_CFG_NO_PROG(1),
         TRNG_SEC_CFG_NO_PROG(TRNG_CONFIG_LOCK));

  CLRSET(TRNG_MCTL, TRNG_MCTL_PRGM, 0);

  // Discard stale data
  restartEntropy();
}

FLASHMEM void trng_deinit() {
  TRNG_MCTL |= TRNG_MCTL_PRGM;  // Move to program mode; stop entropy generation

  // Check this bit before stopping the clock
  while ((TRNG_MCTL & TRNG_MCTL_TSTOP_OK) == 0) {
    // Wait
  }

  CCM_CCGR6 &= ~CCM_CCGR6_TRNG(CCM_CCGR_ON);  // Disable the clock
}

// Copies entropy into the local entropy buffer. It is assumed there's entropy
// available. This checks for an error, and if there is one, returns false.
static bool fillEntropyBuf() {
  // Check for an error
  if ((TRNG_MCTL & TRNG_MCTL_ERR) != 0) {
    TRNG_MCTL = TRNG_MCTL_ERR;  // Clear error
    return false;
  }

  // Fill the array
  volatile uint32_t *addr = &TRNG_ENT0;
  for (size_t i = 0; i < ENTROPY_COUNT; i++) {
    s_entropy[i] = *(addr++);
  }
  TRNG_ENT0;  // Dummy read after TRNG_ENT15 for defect workaround (according to SDK)
  s_entropySizeBytes = ENTROPY_COUNT_BYTES;

  return true;
}

// Fills the entropy pool if empty. This waits for entropy to be available or
// an error.
static bool fillEntropy() {
  if (s_entropySizeBytes > 0) {
    return true;
  }

  // Wait for either Valid or Error flag
  while ((TRNG_MCTL & (TRNG_MCTL_ENT_VAL | TRNG_MCTL_ERR)) == 0) {
  }

  return fillEntropyBuf();
}

// // Reads a single entropy byte.
// static bool readEntropy(uint8_t *b) {
//   if (!fillEntropy()) {
//     return false;
//   }
//   *b = ((uint8_t *)s_entropy)[ENTROPY_COUNT_BYTES - (s_entropySizeBytes--)];
//   return true;
// }

size_t trng_available() {
  if (s_entropySizeBytes == 0) {
    // Check for Valid
    if ((TRNG_MCTL & TRNG_MCTL_ENT_VAL) == 0) {
      return 0;
    }
    if (!fillEntropyBuf()) {
      return 0;
    }
  }

  return s_entropySizeBytes;
}

// See: https://gcc.gnu.org/onlinedocs/gcc/Typeof.html
#define min(a, b) __extension__ ({   \
  typeof(a) _a = (a);  \
  typeof(b) _b = (b);  \
  (_a < _b) ? _a : _b; \
})

size_t trng_data(uint8_t *data, size_t size) {
  // After a deep sleep exit, some error bits are set in MCTL and must be
  // cleared before continuing. Also, trigger new entropy generation to be sure
  // there's fresh bits.
  if ((TRNG_MCTL & TRNG_MCTL_ERR) != 0) {
    TRNG_MCTL = TRNG_MCTL_ERR;  // Clear error

    // Restart entropy generation
    restartEntropy();
  }

  size_t origSize = size;

  while (size > 0) {
    if (!fillEntropy()) {
      return origSize - size;
    }
    size_t toCopy = min(size, s_entropySizeBytes);
    memcpy(data,
           &((uint8_t *)s_entropy)[ENTROPY_COUNT_BYTES - s_entropySizeBytes],
           toCopy);
    data += toCopy;
    s_entropySizeBytes -= toCopy;
    size -= toCopy;
  }

  return origSize;
}

#undef min

uint32_t entropy_random() {
  uint32_t r;
  if (trng_data((uint8_t *)&r, sizeof(r)) < sizeof(r)) {
    errno = EAGAIN;
  }
  return r;
}

uint32_t entropy_random_range(uint32_t range) {
  if (range == 0) {
    errno = EDOM;
    return 0;
  }

  uint32_t r = entropy_random();
  if (errno == EAGAIN) {
    return 0;
  }

  // Is power of 2?
  if ((range & (range - 1)) == 0) {
    return r & (range - 1);
  }

  // Daniel Lemire's nearly-divisionless algorithm
  // https://lemire.me/blog/2019/09/28/doubling-the-speed-of-stduniform_int_distribution-in-the-gnu-c-library/
  // Note: There's not much impact if entropy generation takes much longer
  //       than division.
  uint64_t product = (uint64_t)r * (uint64_t)range;
  uint32_t low = (uint32_t)product;
  if (low < range) {  // Application of the rejection method
    uint32_t threshold = -range % range;  // 2^L mod s = (2^L − s) mod s
    while (low < threshold) {
      r = entropy_random();
      if (errno == EAGAIN) {
        return 0;
      }
      product = (uint64_t)r * (uint64_t)range;
      low = (uint32_t)product;
    }
  }
  return product >> 32;
}

/*

Reset values:
TRNG_SCML: 010C0568h
TRNG_SCML_MONO_RNG[31:16](268) | [sdk_2.13.0: 122] [lib:  268]
TRNG_SCML_MONO_MAX[15:0](1384)   [sdk_2.13.0: 317] [lib: 1384]

TRNG_SCR1L: 00B20195h
TRNG_SCR1L_RUN1_RNG[30:16](178) | [sdk_2.13.0:  80] [lib: 178]
TRNG_SCR1L_RUN1_MAX[14:0](405)    [sdk_2.13.0: 107] [lib: 405]

TRNG_SCR2L: 007A00DCh
TRNG_SCR2L_RUN2_RNG[29:16](122) | [sdk_2.13.0: 55] [lib: 122]
TRNG_SCR2L_RUN2_MAX[13:0](220)    [sdk_2.13.0: 62] [lib: 220]

TRNG_SCR3L: 0058007Dh
TRNG_SCR3L_RUN3_RNG[28:16](88) | [sdk_2.13.0: 39] [lib:  88]
TRNG_SCR3L_RUN3_MAX[12:0](125)   [sdk_2.13.0: 39] [lib: 125]

TRNG_SCR4L: 0040004Bh
TRNG_SCR4L_RUN4_RNG[27:16](64) | [sdk_2.13.0: 26] [lib: 64]
TRNG_SCR4L_RUN4_MAX[11:0](75)    [sdk_2.13.0: 26] [lib: 75]

TRNG_SCR5L: 002E002Fh
TRNG_SCR5L_RUN5_RNG[26:16](46) | [sdk_2.13.0: 18] [lib: 46]
TRNG_SCR5L_RUN5_MAX[10:0](47)    [sdk_2.13.0: 18] [lib: 47]

TRNG_SCR6PL: 002E002Fh
TRNG_SCR6PL_RUN6P_RNG[26:16](46) | [sdk_2.13.0: 17] [lib: 46]
TRNG_SCR6PL_RUN6P_MAX[10:0](47)    [sdk_2.13.0: 17] [lib: 47]

TRNG_PKRMAX[23:0]: 00006920h
26912 [sdk_2.13.0: 1600] [lib: 26912]

TRNG_PKRRNG[15:0]: 000009A3h
2467 [sdk_2.13.0: 570] [lib: 2467]

TRNG_FRQMAX[21:0]: 00006400h
25600 [sdk_2.13.0: 30000] [lib: 25600]

TRNG_FRQMIN[21:0]: 00000640h [? 00000000h in test]
1600 [0] [sdk_2.13.0: 1600] [lib: 1600]
bits

TRNG_MCTL: 00012001h [? 00012601h in test]
TRNG_MCTL_PRGM[16] |
TRNG_MCTL_LRUN_CONT[14](0) |
TRNG_MCTL_TSTOP_OK[13] |
TRNG_MCTL_ERR[12](0) |
TRNG_MCTL_TST_OUT[11](0) |
TRNG_MCTL_FCT_FAIL[8](0) |
TRNG_MCTL_FOR_SCLK[7](0) |
TRNG_MCTL_RST_DEF[6](0) |
TRNG_MCTL_SAMP_MODE[1:0](1) | [sdk_2.13.0: raw(1) in examples, 0 in HAL] [lib: 2]
TRNG_MCTL_OSC_DIV[3:2](0) |
[TRNG_MCTL_ENT_VAL | TRNG_MCTL_FCT_VAL]

TRNG_SDCTL: 0C8009C4h [? 032009c4h in test]
TRNG_SDCTL_ENT_DLY[31:16](3200) | [800] [sdk_2.13.0: 3200] [lib: 3200]
TRNG_SDCTL_SAMP_SIZE[15:0](2500)        [sdk_2.13.0:  512] [lib: 2500]

TRNG_SBLIM[9:0]: 0000003F
63 [sdk_2.13.0: 63] [lib: 63]

TRNG_SCMISC: 0001001Fh [? 00010022h in test]
TRNG_SCMISC_RTY_CT[19:16](1) |     [sdk_2.13.0:  1] [lib:  1]
TRNG_SCMISC_LRUN_MAX[7:0](31) [34] [sdk_2.13.0: 32] [lib: 34]

TRNG_ENT: 0

TRNG_SEC_CFG: 0
TRNG_SEC_CFG_NO_PRGM[1](0) [sdk_2.13.0: 0]

-----
TRNG_MCTL values only writable when PRGM is set:
FOR_SCLK[7] (RW)
RST_DEF[6] (WO)
OSC_DIV[3:2] (RW)
SAMP_MODE[1:0] (RW)

Other MCTL bits:
PRGM[16] (RW)
TSTOP_OK[13] (RO)
ERR[12] (W1C)
ENT_VAL[10] (RO)
TRNG_ACC[5] (RW) <-- Doesn't exist on this chip

*/

#endif  // !QNETHERNET_USE_ENTROPY_LIB
#endif  // __IMXRT1062__
