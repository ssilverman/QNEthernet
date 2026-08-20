// SPDX-FileCopyrightText: (c) 2023-2026 Shawn Silverman <shawn@pobox.com>
// SPDX-License-Identifier: AGPL-3.0-or-later

// entropy.cpp implements the hardware entropy functions.
// This file is part of the QNEthernet library.

#include "qnethernet/entropy/entropy.h"

#if defined(TEENSYDUINO) && defined(__IMXRT1062__)
#if !QNETHERNET_USE_ENTROPY_LIB

// C++ includes
#include <algorithm>
#include <cerrno>
#include <cstring>

#include "qnethernet/compat/c++11_compat.h"
#include "qnethernet/hardware/imxrt1060/CCM.h"
#include "qnethernet/hardware/imxrt1060/TRNG.h"
#include "qnethernet/platforms/pgmspace.h"

namespace qindesign {
namespace entropy {

using namespace qindesign::hardware::imxrt1060;

namespace {
namespace config {

enum : uint32_t {
  // Clock settings
  kCLOCK_MODE   = 0,  // 0=Ring Oscillator, 1=System Clock (test use only)
  kRING_OSC_DIV =     // Divide by 2^n
#if (F_CPU >= 528000000)
      0,
#else
      2,
#endif  // F_CPU range

  // Sampling
  kSAMPLE_MODE      = TRNG::MCTL::kSAMP_MODE_VonNeuMannShifterRawChecker/*kSAMP_MODE_RawBoth*/,
      // 0:Von Neumann in both, 1:raw in both, 2:VN Entropy and raw in stats
  kSPARSE_BIT_LIMIT = 63,

  // Seed control
  kENTROPY_DELAY = 3200,
  kSAMPLE_SIZE   = 2500/*512*/,

  // Statistical check parameters
  kRETRY_COUNT   = 1,
  kRUN_MAX_LIMIT = 34/*32*/,

  kMONOBIT_MAX       = 1384/*317*/,
  kMONOBIT_RANGE     = 268/*122*/,
  kRUNBIT1_MAX       = 405/*107*/,
  kRUNBIT1_RANGE     = 178/*80*/,
  kRUNBIT2_MAX       = 220/*62*/,
  kRUNBIT2_RANGE     = 122/*55*/,
  kRUNBIT3_MAX       = 125/*39*/,
  kRUNBIT3_RANGE     = 88/*39*/,
  kRUNBIT4_MAX       = 75/*26*/,
  kRUNBIT4_RANGE     = 64/*26*/,
  kRUNBIT5_MAX       = 47/*18*/,
  kRUNBIT5_RANGE     = 46/*18*/,
  kRUNBIT6PLUS_MAX   = 47/*17*/,
  kRUNBIT6PLUS_RANGE = 46/*17*/,

  // Limits for statistical check of "Poker Test"
  kPOKER_MAX   = 26912/*1600*/,
  kPOKER_RANGE = 2467/*570*/,

  // Limits for statistical check of entropy sample frequency count
  kFREQUENCY_MAX = 25600/*30000*/,
  kFREQUENCY_MIN = 1600,

  // Security configuration
  kLOCK = 0,
};

}  // namespace config
}  // namespace

// Entropy storage
static constexpr size_t kEntropyCount      = 16;  // In dwords
static constexpr size_t kEntropyCountBytes = (kEntropyCount << 2);  // In bytes

static uint32_t s_entropy[kEntropyCount] DMAMEM;
static size_t s_entropySizeBytes = 0;  // Size in bytes

bool trng_is_started() {
  // Two checks:
  // 1. Clock is running
  // 2. "OK to stop" bit: asserted if the ring oscillator isn't running
  return (CCM::CCGR6::TRNG != 0) && (TRNG::MCTL::TSTOP_OK == 0);
}

// Restarts entropy generation.
static void restartEntropy() {
  (void)TRNG::group->ENT[15];
  (void)TRNG::group->ENT[0];  // Dummy read for defect workaround
  s_entropySizeBytes = 0;
}

FLASHMEM void trng_init() {
  // Enable the clock
  CCM::CCGR6::TRNG = CCM::CCGR::kOn;

  // Set program mode, clear pending errors, reset registers to default
  TRNG::group->MCTL =
      TRNG::MCTL::PRGM(1) | TRNG::MCTL::ERR(1) | TRNG::MCTL::RST_DEF(1);

  // Apply configuration
  TRNG::group->SCMISC = TRNG::SCMISC::RTY_CT(config::kRETRY_COUNT) |
                        TRNG::SCMISC::LRUN_MAX(config::kRUN_MAX_LIMIT);
  TRNG::group->SCML   = TRNG::SCML::MONO_RNG(config::kMONOBIT_RANGE) |
                        TRNG::SCML::MONO_MAX(config::kMONOBIT_MAX);
  TRNG::group->SCR1L  = TRNG::SCR1L::RUN1_RNG(config::kRUNBIT1_RANGE) |
                        TRNG::SCR1L::RUN1_MAX(config::kRUNBIT1_MAX);
  TRNG::group->SCR2L  = TRNG::SCR2L::RUN2_RNG(config::kRUNBIT2_RANGE) |
                        TRNG::SCR2L::RUN2_MAX(config::kRUNBIT2_MAX);
  TRNG::group->SCR3L  = TRNG::SCR3L::RUN3_RNG(config::kRUNBIT3_RANGE) |
                        TRNG::SCR3L::RUN3_MAX(config::kRUNBIT3_MAX);
  TRNG::group->SCR4L  = TRNG::SCR4L::RUN4_RNG(config::kRUNBIT4_RANGE) |
                        TRNG::SCR4L::RUN4_MAX(config::kRUNBIT4_MAX);
  TRNG::group->SCR5L  = TRNG::SCR5L::RUN5_RNG(config::kRUNBIT5_RANGE) |
                        TRNG::SCR5L::RUN5_MAX(config::kRUNBIT5_MAX);
  TRNG::group->SCR6PL = TRNG::SCR6PL::RUN6P_RNG(config::kRUNBIT6PLUS_RANGE) |
                        TRNG::SCR6PL::RUN6P_MAX(config::kRUNBIT6PLUS_MAX);
  TRNG::PKRMAX::PKR_MAX = config::kPOKER_MAX;
  TRNG::PKRRNG::PKR_RNG = config::kPOKER_RANGE;
  TRNG::FRQMAX::FRQ_MAX = config::kFREQUENCY_MAX;
  TRNG::FRQMIN::FRQ_MIN = config::kFREQUENCY_MIN;

  // Clock settings
  TRNG::MCTL::FOR_SCLK = config::kCLOCK_MODE;
  TRNG::MCTL::OSC_DIV  = config::kRING_OSC_DIV;

  // Sampling
  TRNG::MCTL::SAMP_MODE = config::kSAMPLE_MODE;
  TRNG::SBLIM::SB_LIM   = config::kSPARSE_BIT_LIMIT;

  // Seed control
  TRNG::SDCTL::ENT_DLY   = config::kENTROPY_DELAY;
  TRNG::SDCTL::SAMP_SIZE = config::kSAMPLE_SIZE;

  // Security configuration
  TRNG::SEC_CFG::NO_PRGM = config::kLOCK;

  TRNG::MCTL::PRGM = 0;

  // Discard stale data
  restartEntropy();
}

FLASHMEM void trng_deinit() {
  TRNG::MCTL::PRGM = 1;  // Move to program mode; stop entropy generation

  // Check this bit before stopping the clock
  while (TRNG::MCTL::TSTOP_OK == 0) {
    // Wait
  }

  CCM::CCGR6::TRNG = CCM::CCGR::kOff;  // Disable the clock
}

// Copies entropy into the local entropy buffer. It is assumed there's entropy
// available. This checks for an error, and if there is one, returns false.
ATTRIBUTE_NODISCARD
static bool fillEntropyBuf() {
  // Check for an error
  if (TRNG::MCTL::ERR != 0) {
    TRNG::MCTL::ERR = 1;  // Clear error
    return false;
  }

  // Fill the array
  std::copy_n(&TRNG::group->ENT[0], kEntropyCount, &s_entropy[0]);
  (void)TRNG::group->ENT[0];  // Dummy read after TRNG_ENT15 for defect workaround (according to SDK)
  s_entropySizeBytes = kEntropyCountBytes;

  return true;
}

// Fills the entropy pool if empty. This waits for entropy to be available or
// an error.
ATTRIBUTE_NODISCARD
static bool fillEntropy() {
  if (s_entropySizeBytes != 0) {
    return true;
  }

  // Wait for either Valid or Error flag
  while ((TRNG::group->MCTL & (TRNG::MCTL::ENT_VAL(1) | TRNG::MCTL::ERR(1))) ==
         0) {
  }

  return fillEntropyBuf();
}

// // Reads a single entropy byte.
// ATTRIBUTE_NODISCARD
// static bool readEntropy(uint8_t* const b) {
//   if (!fillEntropy()) {
//     return false;
//   }
//   *b = (reinterpret_cast<uint8_t*>(
//       s_entropy))[kEntropyCountBytes - (s_entropySizeBytes--)];
//   return true;
// }

size_t trng_available() {
  if (s_entropySizeBytes == 0) {
    // Check for Valid
    if (TRNG::MCTL::ENT_VAL == 0) {
      return 0;
    }
    if (!fillEntropyBuf()) {
      return 0;
    }
  }

  return s_entropySizeBytes;
}

size_t trng_data(void* const data, const size_t size) {
  // After a deep sleep exit, some error bits are set in MCTL and must be
  // cleared before continuing. Also, trigger new entropy generation to be sure
  // there's fresh bits.
  if (TRNG::MCTL::ERR != 0) {
    TRNG::MCTL::ERR = 1;  // Clear error

    // Restart entropy generation
    restartEntropy();
  }

  uint8_t* p = static_cast<uint8_t*>(data);
  size_t rem = size;

  while (rem != 0) {
    if (!fillEntropy()) {
      return size - rem;
    }
    const size_t toCopy = std::min(rem, s_entropySizeBytes);
    (void)std::memcpy(p,
                      &(reinterpret_cast<uint8_t*>(
                          s_entropy))[kEntropyCountBytes - s_entropySizeBytes],
                      toCopy);
    p += toCopy;
    s_entropySizeBytes -= toCopy;
    rem -= toCopy;
  }

  return size;
}

// Gathers a 32-bit random number and returns whether successful. This assumes
// that the argument is not NULL.
ATTRIBUTE_NODISCARD
static inline bool random32(uint32_t* const r) {
  if (trng_data(r, sizeof(*r)) == sizeof(*r)) {
    return true;
  } else {
    errno = EIO;
    return false;
  }
}

bool entropy_random(uint32_t* const out) {
  uint32_t r;
  if (!random32(&r)) {
    return false;
  }
  *out = r;
  return true;
}

bool entropy_random_range(const uint32_t range, uint32_t* const out) {
  if (range == 0) {
    errno = EDOM;
    return false;
  }

  uint32_t r;
  if (!random32(&r)) {
    return false;
  }

  // Is power of 2?
  if ((range & (range - 1)) == 0) {
    *out = r & (range - 1);
    return true;
  }

  // Daniel Lemire's nearly-divisionless algorithm
  // https://lemire.me/blog/2019/09/28/doubling-the-speed-of-stduniform_int_distribution-in-the-gnu-c-library/
  // Note: There's not much impact if entropy generation takes much longer
  //       than division.
  uint64_t product = uint64_t{r} * uint64_t{range};
  uint32_t low = static_cast<uint32_t>(product);
  if (low < range) {  // Application of the rejection method
    const uint32_t threshold = -range % range;  // 2^L mod s = (2^L − s) mod s
    while (low < threshold) {
      if (!random32(&r)) {
        return false;
      }
      product = uint64_t{r} * uint64_t{range};
      low = static_cast<uint32_t>(product);
    }
  }
  *out = static_cast<uint32_t>(product >> 32);
  return true;
}

}  // namespace entropy
}  // namespace qindesign

/*

Reset values:
TRNG_SCML: 010C0568h
TRNG_SCML_MONO_RNG[31:16](268) | [sdk_2.14.0: 122] [lib:  268]
TRNG_SCML_MONO_MAX[15:0](1384)   [sdk_2.14.0: 317] [lib: 1384]

TRNG_SCR1L: 00B20195h
TRNG_SCR1L_RUN1_RNG[30:16](178) | [sdk_2.14.0:  80] [lib: 178]
TRNG_SCR1L_RUN1_MAX[14:0](405)    [sdk_2.14.0: 107] [lib: 405]

TRNG_SCR2L: 007A00DCh
TRNG_SCR2L_RUN2_RNG[29:16](122) | [sdk_2.14.0: 55] [lib: 122]
TRNG_SCR2L_RUN2_MAX[13:0](220)    [sdk_2.14.0: 62] [lib: 220]

TRNG_SCR3L: 0058007Dh
TRNG_SCR3L_RUN3_RNG[28:16](88) | [sdk_2.14.0: 39] [lib:  88]
TRNG_SCR3L_RUN3_MAX[12:0](125)   [sdk_2.14.0: 39] [lib: 125]

TRNG_SCR4L: 0040004Bh
TRNG_SCR4L_RUN4_RNG[27:16](64) | [sdk_2.14.0: 26] [lib: 64]
TRNG_SCR4L_RUN4_MAX[11:0](75)    [sdk_2.14.0: 26] [lib: 75]

TRNG_SCR5L: 002E002Fh
TRNG_SCR5L_RUN5_RNG[26:16](46) | [sdk_2.14.0: 18] [lib: 46]
TRNG_SCR5L_RUN5_MAX[10:0](47)    [sdk_2.14.0: 18] [lib: 47]

TRNG_SCR6PL: 002E002Fh
TRNG_SCR6PL_RUN6P_RNG[26:16](46) | [sdk_2.14.0: 17] [lib: 46]
TRNG_SCR6PL_RUN6P_MAX[10:0](47)    [sdk_2.14.0: 17] [lib: 47]

TRNG_PKRMAX[23:0]: 00006920h
26912 [sdk_2.14.0: 1600] [lib: 26912]

TRNG_PKRRNG[15:0]: 000009A3h
2467 [sdk_2.14.0: 570] [lib: 2467]

TRNG_FRQMAX[21:0]: 00006400h
25600 [sdk_2.14.0: 30000] [lib: 25600]

TRNG_FRQMIN[21:0]: 00000640h [? 00000000h in test]
1600 [0] [sdk_2.14.0: 1600] [lib: 1600]
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
TRNG_MCTL_SAMP_MODE[1:0](1) | [sdk_2.14.0: raw(1) in examples, 0 in HAL] [lib: 2]
TRNG_MCTL_OSC_DIV[3:2](0) |
[TRNG_MCTL_ENT_VAL | TRNG_MCTL_FCT_VAL]

TRNG_SDCTL: 0C8009C4h [? 032009c4h in test]
TRNG_SDCTL_ENT_DLY[31:16](3200) | [800] [sdk_2.14.0: 3200] [lib: 3200]
TRNG_SDCTL_SAMP_SIZE[15:0](2500)        [sdk_2.14.0:  512] [lib: 2500]

TRNG_SBLIM[9:0]: 0000003F
63 [sdk_2.14.0: 63] [lib: 63]

TRNG_SCMISC: 0001001Fh [? 00010022h in test]
TRNG_SCMISC_RTY_CT[19:16](1) |     [sdk_2.14.0:  1] [lib:  1]
TRNG_SCMISC_LRUN_MAX[7:0](31) [34] [sdk_2.14.0: 32] [lib: 34]

TRNG_ENT: 0

TRNG_SEC_CFG: 0
TRNG_SEC_CFG_NO_PRGM[1](0) [sdk_2.14.0: 0]

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
#endif  // defined(TEENSYDUINO) && defined(__IMXRT1062__)
