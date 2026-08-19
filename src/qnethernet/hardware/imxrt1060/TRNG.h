// SPDX-FileCopyrightText: (c) 2026 Shawn Silverman <shawn@pobox.com>
// SPDX-License-Identifier: AGPL-3.0-or-later

// TRNG.h defines all the TRNG registers.
// This file is part of the imxrt1060-regs library.

#pragma once

#include <cstddef>
#include <cstdint>

#include "qnethernet/hardware/regs/regs.h"

namespace qindesign {
namespace hardware {
namespace imxrt1060 {

// Size of Registers Arrays
constexpr size_t kTRNG_ENTA_count = 16;

// Structure type to access the True Random Number Generator (TRNG) registers.
//
// Comments are from BSD-3-licensed NXP SDK.
//
// See:
// * https://github.com/nxp-mcuxpresso/mcux-devices-rt/blob/main/RT1060/periph/PERI_TRNG.h
// * https://github.com/nxp-mcuxpresso/legacy-mcux-sdk/blob/main/devices/MIMXRT1062/MIMXRT1062.h
struct TRNG_Layout {
  volatile uint32_t MCTL;                              /**< Miscellaneous Control Register, offset: 0x0 */
  volatile uint32_t SCMISC;                            /**< Statistical Check Miscellaneous Register, offset: 0x4 */
  volatile uint32_t PKRRNG;                            /**< Poker Range Register, offset: 0x8 */
  union {                                          /* offset: 0xC */
    volatile uint32_t PKRMAX;                          /**< Poker Maximum Limit Register, offset: 0xC */
    const volatile uint32_t PKRSQ;                     /**< Poker Square Calculation Result Register, offset: 0xC */
  };
  volatile uint32_t SDCTL;                             /**< Seed Control Register, offset: 0x10 */
  union {                                          /* offset: 0x14 */
    volatile uint32_t SBLIM;                           /**< Sparse Bit Limit Register, offset: 0x14 */
    const volatile uint32_t TOTSAM;                    /**< Total Samples Register, offset: 0x14 */
  };
  volatile uint32_t FRQMIN;                            /**< Frequency Count Minimum Limit Register, offset: 0x18 */
  union {                                          /* offset: 0x1C */
    const volatile uint32_t FRQCNT;                    /**< Frequency Count Register, offset: 0x1C */
    volatile uint32_t FRQMAX;                          /**< Frequency Count Maximum Limit Register, offset: 0x1C */
  };
  union {                                          /* offset: 0x20 */
    const volatile uint32_t SCMC;                      /**< Statistical Check Monobit Count Register, offset: 0x20 */
    volatile uint32_t SCML;                            /**< Statistical Check Monobit Limit Register, offset: 0x20 */
  };
  union {                                          /* offset: 0x24 */
    const volatile uint32_t SCR1C;                     /**< Statistical Check Run Length 1 Count Register, offset: 0x24 */
    volatile uint32_t SCR1L;                           /**< Statistical Check Run Length 1 Limit Register, offset: 0x24 */
  };
  union {                                          /* offset: 0x28 */
    const volatile uint32_t SCR2C;                     /**< Statistical Check Run Length 2 Count Register, offset: 0x28 */
    volatile uint32_t SCR2L;                           /**< Statistical Check Run Length 2 Limit Register, offset: 0x28 */
  };
  union {                                          /* offset: 0x2C */
    const volatile uint32_t SCR3C;                     /**< Statistical Check Run Length 3 Count Register, offset: 0x2C */
    volatile uint32_t SCR3L;                           /**< Statistical Check Run Length 3 Limit Register, offset: 0x2C */
  };
  union {                                          /* offset: 0x30 */
    const volatile uint32_t SCR4C;                     /**< Statistical Check Run Length 4 Count Register, offset: 0x30 */
    volatile uint32_t SCR4L;                           /**< Statistical Check Run Length 4 Limit Register, offset: 0x30 */
  };
  union {                                          /* offset: 0x34 */
    const volatile uint32_t SCR5C;                     /**< Statistical Check Run Length 5 Count Register, offset: 0x34 */
    volatile uint32_t SCR5L;                           /**< Statistical Check Run Length 5 Limit Register, offset: 0x34 */
  };
  union {                                          /* offset: 0x38 */
    const volatile uint32_t SCR6PC;                    /**< Statistical Check Run Length 6+ Count Register, offset: 0x38 */
    volatile uint32_t SCR6PL;                          /**< Statistical Check Run Length 6+ Limit Register, offset: 0x38 */
  };
  const volatile uint32_t STATUS;                      /**< Status Register, offset: 0x3C */
  const volatile uint32_t ENT[kTRNG_ENTA_count];       /**< Entropy Read Register, array offset: 0x40, array step: 0x4 */
  const volatile uint32_t PKRCNT10;                    /**< Statistical Check Poker Count 1 and 0 Register, offset: 0x80 */
  const volatile uint32_t PKRCNT32;                    /**< Statistical Check Poker Count 3 and 2 Register, offset: 0x84 */
  const volatile uint32_t PKRCNT54;                    /**< Statistical Check Poker Count 5 and 4 Register, offset: 0x88 */
  const volatile uint32_t PKRCNT76;                    /**< Statistical Check Poker Count 7 and 6 Register, offset: 0x8C */
  const volatile uint32_t PKRCNT98;                    /**< Statistical Check Poker Count 9 and 8 Register, offset: 0x90 */
  const volatile uint32_t PKRCNTBA;                    /**< Statistical Check Poker Count B and A Register, offset: 0x94 */
  const volatile uint32_t PKRCNTDC;                    /**< Statistical Check Poker Count D and C Register, offset: 0x98 */
  const volatile uint32_t PKRCNTFE;                    /**< Statistical Check Poker Count F and E Register, offset: 0x9C */
  volatile uint32_t SEC_CFG;                           /**< Security Configuration Register, offset: 0xA0 */
  volatile uint32_t INT_CTRL;                          /**< Interrupt Control Register, offset: 0xA4 */
  volatile uint32_t INT_MASK;                          /**< Mask Register, offset: 0xA8 */
  const volatile uint32_t INT_STATUS;                  /**< Interrupt Status Register, offset: 0xAC */
  uint32_t HARDWARE_REGS_LAYOUT_MEMBER_RESERVED[16];
  const volatile uint32_t VID1;                        /**< Version ID Register (MS), offset: 0xF0 */
  const volatile uint32_t VID2;                        /**< Version ID Register (LS), offset: 0xF4 */
};

constexpr size_t    kTRNG_size = 0xF8;
constexpr uintptr_t kTRNG_base = 0x400C'C000;

namespace TRNG {

constexpr regs::RegGroup<TRNG_Layout, kTRNG_size, kTRNG_base> group;

template <auto Member, size_t Bits, unsigned int Shift,
          auto AssignMask = regs::shiftedMask32<Bits, Shift>(),
          bool WriteOnly = false>
using TRNG_Reg = regs::Reg32<kTRNG_base, TRNG_Layout, Member, 0, Bits, Shift,
                             AssignMask, 0, WriteOnly>;

// Miscellaneous Control Register
namespace MCTL {
// TODO: Is this the correct way?
constexpr uint32_t kW1C = regs::shiftedMask32<1, 12>();
constexpr uint32_t kWO  = regs::shiftedMask32<1, 6>();

constexpr TRNG_Reg<&TRNG_Layout::MCTL, 1, 16, regs::shiftedMask32<1, 16>() | kW1C | kWO> PRGM;       // Programming Mode Select
constexpr TRNG_Reg<&TRNG_Layout::MCTL, 1, 14, regs::shiftedMask32<1, 14>() | kW1C | kWO> LRUN_CONT;  // Long run count continues between entropy generations
constexpr TRNG_Reg<regs::constify(&TRNG_Layout::MCTL), 1, 13> TSTOP_OK;                              // TRNG_OK_TO_STOP
constexpr TRNG_Reg<&TRNG_Layout::MCTL, 1, 12, kW1C | kWO> ERR;                                       // Read: Error status
constexpr TRNG_Reg<regs::constify(&TRNG_Layout::MCTL), 1, 11> TST_OUT;                               // Read only: Test point inside ring oscillator.
constexpr TRNG_Reg<regs::constify(&TRNG_Layout::MCTL), 1, 10> ENT_VAL;                               // Read only: Entropy Valid
constexpr TRNG_Reg<regs::constify(&TRNG_Layout::MCTL), 1,  9> FCT_VAL;                               // Read only: Frequency Count Valid.
    // Indicates that a valid frequency count may be read from FRQCNT.
constexpr TRNG_Reg<regs::constify(&TRNG_Layout::MCTL), 1,  8> FCT_FAIL;                              // Read only: Frequency Count Fail
constexpr TRNG_Reg<&TRNG_Layout::MCTL, 1,  7, regs::shiftedMask32<1,  7>() | kW1C | kWO> FOR_SCLK;   // Force System Clock
constexpr TRNG_Reg<&TRNG_Layout::MCTL, 1,  6, kW1C | kWO, true> RST_DEF;                             // Reset Defaults
constexpr TRNG_Reg<regs::constify(&TRNG_Layout::MCTL), 1,  5> UNUSED5;                               // This bit is unused.
    // Always reads zero.
constexpr TRNG_Reg<regs::constify(&TRNG_Layout::MCTL), 1,  4> UNUSED4;                               // This bit is unused.
    // Always reads zero.
constexpr TRNG_Reg<&TRNG_Layout::MCTL, 2,  2, regs::shiftedMask32<2,  2>() | kW1C | kWO> OSC_DIV;    // Oscillator Divide
    // 1/2^'value':
    // 0b00..use ring oscillator with no divide
    // 0b01..use ring oscillator divided-by-2
    // 0b10..use ring oscillator divided-by-4
    // 0b11..use ring oscillator divided-by-8
constexpr TRNG_Reg<&TRNG_Layout::MCTL, 2,  0, regs::shiftedMask32<2,  0>() | kW1C | kWO> SAMP_MODE;  // Sample Mode
    // 0b00..use Von Neumann data into both Entropy shifter and Statistical Checker
    // 0b01..use raw data into both Entropy shifter and Statistical Checker
    // 0b10..use Von Neumann data into Entropy shifter. Use raw data into Statistical Checker
    // 0b11..undefined/reserved.

// Sample Mode values
constexpr uint32_t kSAMP_MODE_VonNeumannBoth              = 0;
constexpr uint32_t kSAMP_MODE_RawBoth                     = 1;
constexpr uint32_t kSAMP_MODE_VonNeuMannShifterRawChecker = 2;
}  // namespace MCTL

// Statistical Check Miscellaneous Register
namespace SCMISC {
constexpr TRNG_Reg<&TRNG_Layout::SCMISC, 4, 16> RTY_CT;    // RETRY COUNT
constexpr TRNG_Reg<&TRNG_Layout::SCMISC, 8,  0> LRUN_MAX;  // LONG RUN MAX LIMIT
}  // namespace SCMISC

// Poker Range Register
namespace PKRRNG {
constexpr TRNG_Reg<&TRNG_Layout::PKRRNG, 16, 0> PKR_RNG;  // Poker Range
}  // namespace PKRRNG

// Poker Maximum Limit Register
namespace PKRMAX {
constexpr TRNG_Reg<&TRNG_Layout::PKRMAX, 24, 0> PKR_MAX;  // Poker Maximum Limit.
}  // namespace PKRMAX

// Poker Square Calculation Result Register
namespace PKRSQ {
constexpr TRNG_Reg<&TRNG_Layout::PKRSQ, 24, 0> PKR_SQ;  // Poker Square Calculation Result.
}  // namespace PKRSQ

// Seed Control Register
namespace SDCTL {
constexpr TRNG_Reg<&TRNG_Layout::SDCTL, 16, 16> ENT_DLY;    // Entropy Delay
constexpr TRNG_Reg<&TRNG_Layout::SDCTL, 16,  0> SAMP_SIZE;  // Sample Size
}  // namespace SDCTL

// Sparse Bit Limit Register
namespace SBLIM {
constexpr TRNG_Reg<&TRNG_Layout::SBLIM, 10, 0> SB_LIM;  // Sparse Bit Limit
}  // namespace SBLIM

// Total Samples Register
namespace TOTSAM {
constexpr TRNG_Reg<&TRNG_Layout::TOTSAM, 20, 0> TOT_SAM;  // Total Samples
}  // namespace TOTSAM

// Frequency Count Minimum Limit Register
namespace FRQMIN {
constexpr TRNG_Reg<&TRNG_Layout::FRQMIN, 22, 0> FRQ_MIN;  // Frequency Count Minimum Limit
}  // namespace FRQMIN

// Frequency Count Register
namespace FRQCNT {
constexpr TRNG_Reg<&TRNG_Layout::FRQCNT, 22, 0> FRQ_CT;  // Frequency Count
}  // namespace FRQCNT

// Frequency Count Maximum Limit Register
namespace FRQMAX {
constexpr TRNG_Reg<&TRNG_Layout::FRQMAX, 22, 0> FRQ_MAX;  // Frequency Counter Maximum Limit
}  // namespace FRQMAX

// Statistical Check Monobit Count Register
namespace SCMC {
constexpr TRNG_Reg<&TRNG_Layout::SCMC, 16, 0> MONO_CT;  // Monobit Count
}  // namespace SCMC

// Statistical Check Monobit Limit Register
namespace SCML {
constexpr TRNG_Reg<&TRNG_Layout::SCML, 16, 16> MONO_RNG;  // Monobit Range
constexpr TRNG_Reg<&TRNG_Layout::SCML, 16,  0> MONO_MAX;  // Monobit Maximum Limit
}  // namespace SCML

// Statistical Check Run Length 1 Count Register
namespace SCR1C {
constexpr TRNG_Reg<&TRNG_Layout::SCR1C, 15, 16> R1_1_CT;  // Runs of One, Length 1 Count
constexpr TRNG_Reg<&TRNG_Layout::SCR1C, 15,  0> R1_0_CT;  // Runs of Zero, Length 1 Count
}  // namespace SCR1C

// Statistical Check Run Length 1 Limit Register
namespace SCR1L {
constexpr TRNG_Reg<&TRNG_Layout::SCR1L, 15, 16> RUN1_RNG;  // Run Length 1 Range
constexpr TRNG_Reg<&TRNG_Layout::SCR1L, 15,  0> RUN1_MAX;  // Run Length 1 Maximum Limit
}  // namespace SCR1L

// Statistical Check Run Length 2 Count Register
namespace SCR2C {
constexpr TRNG_Reg<&TRNG_Layout::SCR2C, 14, 16> R2_1_CT;  // Runs of One, Length 2 Count
constexpr TRNG_Reg<&TRNG_Layout::SCR2C, 14,  0> R2_0_CT;  // Runs of Zero, Length 2 Count
}  // namespace SCR2C

// Statistical Check Run Length 2 Limit Register
namespace SCR2L {
constexpr TRNG_Reg<&TRNG_Layout::SCR2L, 14, 16> RUN2_RNG;  // Run Length 2 Range
constexpr TRNG_Reg<&TRNG_Layout::SCR2L, 14,  0> RUN2_MAX;  // Run Length 2 Maximum Limit
}  // namespace SCR2L

// Statistical Check Run Length 3 Count Register
namespace SCR3C {
constexpr TRNG_Reg<&TRNG_Layout::SCR3C, 13, 16> R3_1_CT;  // Runs of Ones, Length 3 Count
constexpr TRNG_Reg<&TRNG_Layout::SCR3C, 13,  0> R3_0_CT;  // Runs of Zeroes, Length 3 Count
}  // namespace SCR3C

// Statistical Check Run Length 3 Limit Register
namespace SCR3L {
constexpr TRNG_Reg<&TRNG_Layout::SCR3L, 13, 16> RUN3_RNG;  // Run Length 3 Range
constexpr TRNG_Reg<&TRNG_Layout::SCR3L, 13,  0> RUN3_MAX;  // Run Length 3 Maximum Limit
}  // namespace SCR3L

// Statistical Check Run Length 4 Count Register
namespace SCR4C {
constexpr TRNG_Reg<&TRNG_Layout::SCR4C, 12, 16> R4_1_CT;  // Runs of One, Length 4 Count
constexpr TRNG_Reg<&TRNG_Layout::SCR4C, 12,  0> R4_0_CT;  // Runs of Zero, Length 4 Count
}  // namespace SCR4C

// Statistical Check Run Length 4 Limit Register
namespace SCR4L {
constexpr TRNG_Reg<&TRNG_Layout::SCR4L, 12, 16> RUN4_RNG;  // Run Length 4 Range
constexpr TRNG_Reg<&TRNG_Layout::SCR4L, 12,  0> RUN4_MAX;  // Run Length 4 Maximum Limit
}  // namespace SCR4L

// Statistical Check Run Length 5 Count Register
namespace SCR5C {
constexpr TRNG_Reg<&TRNG_Layout::SCR5C, 11, 16> R5_1_CT;  // Runs of One, Length 5 Count
constexpr TRNG_Reg<&TRNG_Layout::SCR5C, 11,  0> R5_0_CT;  // Runs of Zero, Length 5 Count
}  // namespace SCR5C

// Statistical Check Run Length 5 Limit Register
namespace SCR5L {
constexpr TRNG_Reg<&TRNG_Layout::SCR5L, 11, 16> RUN5_RNG;  // Run Length 5 Range
constexpr TRNG_Reg<&TRNG_Layout::SCR5L, 11,  0> RUN5_MAX;  // Run Length 5 Maximum Limit
}  // namespace SCR5L

// Statistical Check Run Length 6+ Count Register
namespace SCR6PC {
constexpr TRNG_Reg<&TRNG_Layout::SCR6PC, 11, 16> R6P_1_CT;  // Runs of One, Length 6+ Count
constexpr TRNG_Reg<&TRNG_Layout::SCR6PC, 11,  0> R6P_0_CT;  // Runs of Zero, Length 6+ Count
}  // namespace SCR6PC

// Statistical Check Run Length 6+ Limit Register
namespace SCR6PL {
constexpr TRNG_Reg<&TRNG_Layout::SCR6PL, 11, 16> RUN6P_RNG;  // Run Length 6+ Range
constexpr TRNG_Reg<&TRNG_Layout::SCR6PL, 11,  0> RUN6P_MAX;  // Run Length 6+ Maximum Limit
}  // namespace SCR6PL

// Status Register
namespace STATUS {
constexpr TRNG_Reg<&TRNG_Layout::STATUS, 4, 16> RETRY_CT;  // RETRY COUNT
constexpr TRNG_Reg<&TRNG_Layout::STATUS, 1, 15> TFMB;      // Test Fail, Mono Bit.
    // If TFMB=1, the Mono Bit Test has failed.
constexpr TRNG_Reg<&TRNG_Layout::STATUS, 1, 14> TFP;       // Test Fail, Poker.
    // If TFP=1, the Poker Test has failed.
constexpr TRNG_Reg<&TRNG_Layout::STATUS, 1, 13> TFLR;      // Test Fail, Long Run.
    // If TFLR=1, the Long Run Test has failed.
constexpr TRNG_Reg<&TRNG_Layout::STATUS, 1, 12> TFSB;      // Test Fail, Sparse Bit.
    // If TFSB=1, the Sparse Bit Test has failed.
constexpr TRNG_Reg<&TRNG_Layout::STATUS, 1, 11> TF6PBR1;   // Test Fail, 6 Plus Bit Run, Sampling 1s
constexpr TRNG_Reg<&TRNG_Layout::STATUS, 1, 10> TF6PBR0;   // Test Fail, 6 Plus Bit Run, Sampling 0s
constexpr TRNG_Reg<&TRNG_Layout::STATUS, 1,  9> TF5BR1;    // Test Fail, 5-Bit Run, Sampling 1s.
    // If TF5BR1=1, the 5-Bit Run, Sampling 1s Test has failed.
constexpr TRNG_Reg<&TRNG_Layout::STATUS, 1,  8> TF5BR0;    // Test Fail, 5-Bit Run, Sampling 0s.
    // If TF5BR0=1, the 5-Bit Run, Sampling 0s Test has failed.
constexpr TRNG_Reg<&TRNG_Layout::STATUS, 1,  7> TF4BR1;    // Test Fail, 4-Bit Run, Sampling 1s.
    // If TF4BR1=1, the 4-Bit Run, Sampling 1s Test has failed.
constexpr TRNG_Reg<&TRNG_Layout::STATUS, 1,  6> TF4BR0;    // Test Fail, 4-Bit Run, Sampling 0s.
    // If TF4BR0=1, the 4-Bit Run, Sampling 0s Test has failed.
constexpr TRNG_Reg<&TRNG_Layout::STATUS, 1,  5> TF3BR1;    // Test Fail, 3-Bit Run, Sampling 1s.
    // If TF3BR1=1, the 3-Bit Run, Sampling 1s Test has failed.
constexpr TRNG_Reg<&TRNG_Layout::STATUS, 1,  4> TF3BR0;    // Test Fail, 3-Bit Run, Sampling 0s.
    // If TF3BR0=1, the 3-Bit Run, Sampling 0s Test has failed.
constexpr TRNG_Reg<&TRNG_Layout::STATUS, 1,  3> TF2BR1;    // Test Fail, 2-Bit Run, Sampling 1s.
    // If TF2BR1=1, the 2-Bit Run, Sampling 1s Test has failed.
constexpr TRNG_Reg<&TRNG_Layout::STATUS, 1,  2> TF2BR0;    // Test Fail, 2-Bit Run, Sampling 0s.
    // If TF2BR0=1, the 2-Bit Run, Sampling 0s Test has failed.
constexpr TRNG_Reg<&TRNG_Layout::STATUS, 1,  1> TF1BR1;    // Test Fail, 1-Bit Run, Sampling 1s.
    // If TF1BR1=1, the 1-Bit Run, Sampling 1s Test has failed.
constexpr TRNG_Reg<&TRNG_Layout::STATUS, 1,  0> TF1BR0;    // Test Fail, 1-Bit Run, Sampling 0s.
    // If TF1BR0=1, the 1-Bit Run, Sampling 0s Test has failed.
}  // namespace STATUS

// Statistical Check Poker Count 1 and 0 Register
namespace PKRCNT10 {
constexpr TRNG_Reg<&TRNG_Layout::PKRCNT10, 16, 16> PKR_1_CT;  // Poker 1h Count
constexpr TRNG_Reg<&TRNG_Layout::PKRCNT10, 16,  0> PKR_0_CT;  // Poker 0h Count
}  // namespace PKRCNT10

// Statistical Check Poker Count 3 and 2 Register
namespace PKRCNT32 {
constexpr TRNG_Reg<&TRNG_Layout::PKRCNT32, 16, 16> PKR_3_CT;  // Poker 3h Count
constexpr TRNG_Reg<&TRNG_Layout::PKRCNT32, 16,  0> PKR_2_CT;  // Poker 2h Count
}  // namespace PKRCNT32

// Statistical Check Poker Count 5 and 4 Register
namespace PKRCNT54 {
constexpr TRNG_Reg<&TRNG_Layout::PKRCNT54, 16, 16> PKR_5_CT;  // Poker 5h Count
constexpr TRNG_Reg<&TRNG_Layout::PKRCNT54, 16,  0> PKR_4_CT;  // Poker 4h Count
}  // namespace PKRCNT54

// Statistical Check Poker Count 7 and 6 Register
namespace PKRCNT76 {
constexpr TRNG_Reg<&TRNG_Layout::PKRCNT76, 16, 16> PKR_7_CT;  // Poker 7h Count
constexpr TRNG_Reg<&TRNG_Layout::PKRCNT76, 16,  0> PKR_6_CT;  // Poker 6h Count
}  // namespace PKRCNT76

// Statistical Check Poker Count 9 and 8 Register
namespace PKRCNT98 {
constexpr TRNG_Reg<&TRNG_Layout::PKRCNT98, 16, 16> PKR_9_CT;  // Poker 9h Count
constexpr TRNG_Reg<&TRNG_Layout::PKRCNT98, 16,  0> PKR_8_CT;  // Poker 8h Count
}  // namespace PKRCNT98

// Statistical Check Poker Count B and A Register
namespace PKRCNTBA {
constexpr TRNG_Reg<&TRNG_Layout::PKRCNTBA, 16, 16> PKR_B_CT;  // Poker Bh Count
constexpr TRNG_Reg<&TRNG_Layout::PKRCNTBA, 16,  0> PKR_A_CT;  // Poker Ah Count
}  // namespace PKRCNTBA

// Statistical Check Poker Count D and C Register
namespace PKRCNTDC {
constexpr TRNG_Reg<&TRNG_Layout::PKRCNTDC, 16, 16> PKR_D_CT;  // Poker Dh Count
constexpr TRNG_Reg<&TRNG_Layout::PKRCNTDC, 16,  0> PKR_C_CT;  // Poker Ch Count
}  // namespace PKRCNTDC

// Statistical Check Poker Count F and E Register
namespace PKRCNTFE {
constexpr TRNG_Reg<&TRNG_Layout::PKRCNTFE, 16, 16> PKR_F_CT;  // Poker Fh Count
constexpr TRNG_Reg<&TRNG_Layout::PKRCNTFE, 16,  0> PKR_E_CT;  // Poker Eh Count
}  // namespace PKRCNTFE

namespace SEC_CFG {
constexpr TRNG_Reg<&TRNG_Layout::SEC_CFG, 1, 2> UNUSED2;  // This bit is unused.
    // Ignore.
constexpr TRNG_Reg<&TRNG_Layout::SEC_CFG, 1, 1> NO_PRGM;  // If set, the TRNG registers cannot be programmed
    // 0b0..Programability of registers controlled only by the Miscellaneous Control Register's access mode bit.
    // 0b1..Overides Miscellaneous Control Register access mode and prevents TRNG register programming.
constexpr TRNG_Reg<&TRNG_Layout::SEC_CFG, 1, 0> UNUSED0;  // This bit is unused.
    // Ignore.
}  // namespace SEC_CFG

// Interrupt Control Register
namespace INT_CTRL {
// 0b0..Corresponding bit of INT_STATUS register cleared.
// 0b1..Corresponding bit of INT_STATUS register active.
constexpr TRNG_Reg<&TRNG_Layout::INT_CTRL, 29, 3> UNUSED;
constexpr TRNG_Reg<&TRNG_Layout::INT_CTRL, 1,  2> FRQ_CT_FAIL;  // Same behavior as bit 0 of this register.
constexpr TRNG_Reg<&TRNG_Layout::INT_CTRL, 1,  1> ENT_VAL;      // Same behavior as bit 0 of this register.
constexpr TRNG_Reg<&TRNG_Layout::INT_CTRL, 1,  0> HW_ERR;       // Bit position that can be cleared if corresponding bit of INT_STATUS register has been asserted.
}  // namespace INT_CTRL

// Mask Register
namespace INT_MASK {
// 0b0..Corresponding interrupt of INT_STATUS is masked.
// 0b1..Corresponding bit of INT_STATUS is active.
constexpr TRNG_Reg<&TRNG_Layout::INT_MASK, 1, 2> FRQ_CT_FAIL;  // Same behavior as bit 0 of this register.
    // 0b0..Same behavior as bit 0 of this register.
    // 0b1..Same behavior as bit 0 of this register.
constexpr TRNG_Reg<&TRNG_Layout::INT_MASK, 1, 1> ENT_VAL;      // Same behavior as bit 0 of this register.
    // 0b0..Same behavior as bit 0 of this register.
    // 0b1..Same behavior as bit 0 of this register.
constexpr TRNG_Reg<&TRNG_Layout::INT_MASK, 1, 0> HW_ERR;       // Bit position that can be cleared or set to enable the corresponding bit of INT_STATUS to show interupt status
    // 0b0..Corresponding interrupt of INT_STATUS is masked.
    // 0b1..Corresponding bit of INT_STATUS is active.
}  // namespace INT_MASK

// Interrupt Status Register
namespace INT_STATUS {
constexpr TRNG_Reg<&TRNG_Layout::INT_STATUS, 1, 2> FRQ_CT_FAIL;  // Read only: Frequency Count Fail
    // 0b0..No hardware nor self test frequency errors.
    // 0b1..The frequency counter has detected a failure.
constexpr TRNG_Reg<&TRNG_Layout::INT_STATUS, 1, 1> ENT_VAL;      // Read only: Entropy Valid
    // 0b0..Busy generation entropy. Any value read is invalid.
    // 0b1..TRNG can be stopped and entropy is valid if read.
constexpr TRNG_Reg<&TRNG_Layout::INT_STATUS, 1, 0> HW_ERR;       // Read: Error status
    // 0b0..no error
    // 0b1..error detected.
}  // namespace INT_STATUS

// Version ID Register (MS)
namespace VID1 {
constexpr TRNG_Reg<&TRNG_Layout::VID1, 16, 16> IP_ID;    // Shows the IP ID.
    // 0b0000000000110000..ID for TRNG.
constexpr TRNG_Reg<&TRNG_Layout::VID1,  8,  8> MAJ_REV;  // Shows the IP's Major revision of the TRNG.
    // 0b00000001..Major revision number for TRNG.
constexpr TRNG_Reg<&TRNG_Layout::VID1,  8,  0> MIN_REV;  // Shows the IP's Minor revision of the TRNG.
    // 0b00000000..Minor revision number for TRNG.
}  // namespace VID1

// Version ID Register (LS)
namespace VID2 {
constexpr TRNG_Reg<&TRNG_Layout::VID2, 8, 24> ERA;         // Shows the compile options for the TRNG.
    // 0b00000000..COMPILE_OPT for TRNG.
constexpr TRNG_Reg<&TRNG_Layout::VID2, 8, 16> INTG_OPT;    // Shows the integration options for the TRNG
    // 0b00000000..INTG_OPT for TRNG.
constexpr TRNG_Reg<&TRNG_Layout::VID2, 8,  8> ECO_REV;     // Shows the IP's ECO revision of the TRNG.
    // 0b00000000..TRNG_ECO_REV for TRNG.
constexpr TRNG_Reg<&TRNG_Layout::VID2, 8,  0> CONFIG_OPT;  // Shows the IP's Configuaration options for the TRNG.
    // 0b00000000..TRNG_CONFIG_OPT for TRNG.
}  // namespace VID2

}  // namespace TRNG

}  // namespace imxrt1060
}  // namespace hardware
}  // namespace qindesign
