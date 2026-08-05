// SPDX-FileCopyrightText: (c) 2026 Shawn Silverman <shawn@pobox.com>
// SPDX-License-Identifier: AGPL-3.0-or-later

// DWT.h defines all the DWT registers.
// This file is part of the imxrt1060-regs library.

#pragma once

#include <cstddef>
#include <cstdint>

#include "qnethernet/hardware/regs/regs.h"

namespace qindesign {
namespace hardware {
namespace imxrt1060 {

// Structure type to access the Data Watchpoint and Trace (DWT) registers.
//
// Comments are from BSD-3-licensed NXP SDK.
//
// See:
// * https://github.com/nxp-mcuxpresso/mcu-sdk-cmsis/blob/mcux_main/Core/Include/core_cm7.h
struct DWT_Layout {
  volatile uint32_t CTRL;                   /*!< Offset: 0x000 (R/W)  Control Register */
  volatile uint32_t CYCCNT;                 /*!< Offset: 0x004 (R/W)  Cycle Count Register */
  volatile uint32_t CPICNT;                 /*!< Offset: 0x008 (R/W)  CPI Count Register */
  volatile uint32_t EXCCNT;                 /*!< Offset: 0x00C (R/W)  Exception Overhead Count Register */
  volatile uint32_t SLEEPCNT;               /*!< Offset: 0x010 (R/W)  Sleep Count Register */
  volatile uint32_t LSUCNT;                 /*!< Offset: 0x014 (R/W)  LSU Count Register */
  volatile uint32_t FOLDCNT;                /*!< Offset: 0x018 (R/W)  Folded-instruction Count Register */
  const volatile uint32_t PCSR;             /*!< Offset: 0x01C (R/ )  Program Counter Sample Register */
  volatile uint32_t COMP0;                  /*!< Offset: 0x020 (R/W)  Comparator Register 0 */
  volatile uint32_t MASK0;                  /*!< Offset: 0x024 (R/W)  Mask Register 0 */
  volatile uint32_t FUNCTION0;              /*!< Offset: 0x028 (R/W)  Function Register 0 */
  uint32_t HARDWARE_REGS_LAYOUT_MEMBER_RESERVED[1];
  volatile uint32_t COMP1;                  /*!< Offset: 0x030 (R/W)  Comparator Register 1 */
  volatile uint32_t MASK1;                  /*!< Offset: 0x034 (R/W)  Mask Register 1 */
  volatile uint32_t FUNCTION1;              /*!< Offset: 0x038 (R/W)  Function Register 1 */
  uint32_t HARDWARE_REGS_LAYOUT_MEMBER_RESERVED[1];
  volatile uint32_t COMP2;                  /*!< Offset: 0x040 (R/W)  Comparator Register 2 */
  volatile uint32_t MASK2;                  /*!< Offset: 0x044 (R/W)  Mask Register 2 */
  volatile uint32_t FUNCTION2;              /*!< Offset: 0x048 (R/W)  Function Register 2 */
  uint32_t HARDWARE_REGS_LAYOUT_MEMBER_RESERVED[1];
  volatile uint32_t COMP3;                  /*!< Offset: 0x050 (R/W)  Comparator Register 3 */
  volatile uint32_t MASK3;                  /*!< Offset: 0x054 (R/W)  Mask Register 3 */
  volatile uint32_t FUNCTION3;              /*!< Offset: 0x058 (R/W)  Function Register 3 */
};

constexpr size_t    kDWT_size = 0x5C;
constexpr uintptr_t kDWT_base = 0xE000'1000;  /*!< DWT Base Address */

namespace DWT {

constexpr regs::RegGroup<DWT_Layout, kDWT_size, kDWT_base> group;

template <auto Member, size_t Bits, unsigned int Shift,
          auto AssignMask = regs::shiftedMask32<Bits, Shift>()>
using DWT_Reg =
    regs::Reg32<kDWT_base, DWT_Layout, Member, 0, Bits, Shift, AssignMask>;

// DWT Control Register Definitions
namespace CTRL {
constexpr DWT_Reg<regs::constify(&DWT_Layout::CTRL), 4, 28> NUMCOMP;
constexpr DWT_Reg<regs::constify(&DWT_Layout::CTRL), 1, 27> NOTRCPKT;
constexpr DWT_Reg<regs::constify(&DWT_Layout::CTRL), 1, 26> NOEXTTRIG;
constexpr DWT_Reg<regs::constify(&DWT_Layout::CTRL), 1, 25> NOCYCCNT;
constexpr DWT_Reg<regs::constify(&DWT_Layout::CTRL), 1, 24> NOPRFCNT;
constexpr DWT_Reg<&DWT_Layout::CTRL, 1, 22> CYCEVTENA;
constexpr DWT_Reg<&DWT_Layout::CTRL, 1, 21> FOLDEVTENA;
constexpr DWT_Reg<&DWT_Layout::CTRL, 1, 20> LSUEVTENA;
constexpr DWT_Reg<&DWT_Layout::CTRL, 1, 19> SLEEPEVTENA;
constexpr DWT_Reg<&DWT_Layout::CTRL, 1, 18> EXCEVTENA;
constexpr DWT_Reg<&DWT_Layout::CTRL, 1, 17> CPIEVTENA;
constexpr DWT_Reg<&DWT_Layout::CTRL, 1, 16> EXCTRCENA;
constexpr DWT_Reg<&DWT_Layout::CTRL, 1, 12> PCSAMPLENA;
constexpr DWT_Reg<&DWT_Layout::CTRL, 2, 10> SYNCTAP;
constexpr DWT_Reg<&DWT_Layout::CTRL, 1,  9> CYCTAP;
constexpr DWT_Reg<&DWT_Layout::CTRL, 4,  5> POSTINIT;
constexpr DWT_Reg<&DWT_Layout::CTRL, 4,  1> POSTPRESET;
constexpr DWT_Reg<&DWT_Layout::CTRL, 1,  0> CYCCNTENA;
}  // namespace CTRL

// DWT Cycle Count Register Definitions
namespace CYCCNT {
constexpr DWT_Reg<&DWT_Layout::CYCCNT, 32, 0> CYCCNT;
}  // namespace CYCCNT

// DWT CPI Count Register Definitions
namespace CPICNT {
constexpr DWT_Reg<&DWT_Layout::CPICNT, 8, 0> CPICNT;
}  // namespace CPICNT

// DWT Exception Overhead Count Register Definitions
namespace EXCCNT {
constexpr DWT_Reg<&DWT_Layout::EXCCNT, 8, 0> EXCCNT;
}  // namespace EXCCNT

// DWT Sleep Count Register Definitions
namespace SLEEPCNT {
constexpr DWT_Reg<&DWT_Layout::SLEEPCNT, 8, 0> SLEEPCNT;
}  // namespace SLEEPCNT

// DWT LSU Count Register Definitions
namespace LSUCNT {
constexpr DWT_Reg<&DWT_Layout::LSUCNT, 8, 0> LSUCNT;
}  // namespace LSUCNT

// DWT Folded-instruction Count Register Definitions
namespace FOLDCNT {
constexpr DWT_Reg<&DWT_Layout::FOLDCNT, 8, 0> FOLDCNT;
}  // namespace FOLDCNT

// DWT Program Counter Sample Register Definitions
namespace PCSR {
constexpr DWT_Reg<&DWT_Layout::PCSR, 32, 0> EIASAMPLE;
}  // namespace PCSR

// DWT Comparator Register Definitions
namespace COMP0 {
constexpr DWT_Reg<&DWT_Layout::COMP0, 32, 0> COMP;
}  // namespace COMP0

// DWT Comparator Mask Register Definitions
namespace MASK0 {
constexpr DWT_Reg<&DWT_Layout::MASK0, 5, 0> MASK;
}  // namespace MASK0

// DWT Comparator Function Register Definitions
namespace FUNCTION0 {
constexpr DWT_Reg<regs::constify(&DWT_Layout::FUNCTION0), 1, 24> MATCHED;
constexpr DWT_Reg<&DWT_Layout::FUNCTION0, 4, 16> DATAVADDR1;
constexpr DWT_Reg<&DWT_Layout::FUNCTION0, 4, 12> DATAVADDR0;
constexpr DWT_Reg<&DWT_Layout::FUNCTION0, 2, 10> DATAVSIZE;
constexpr DWT_Reg<regs::constify(&DWT_Layout::FUNCTION0), 1,  9> LNK1ENA;
constexpr DWT_Reg<&DWT_Layout::FUNCTION0, 1,  8> DATAVMATCH;
constexpr DWT_Reg<&DWT_Layout::FUNCTION0, 1,  7> CYCMATCH;
constexpr DWT_Reg<&DWT_Layout::FUNCTION0, 1,  5> EMITRANGE;
constexpr DWT_Reg<&DWT_Layout::FUNCTION0, 4,  0> FUNCTION;
}  // namespace FUNCTION0

// DWT Comparator Register Definitions
namespace COMP1 {
constexpr DWT_Reg<&DWT_Layout::COMP1, 32, 0> COMP;
}  // namespace COMP1

// DWT Comparator Mask Register Definitions
namespace MASK1 {
constexpr DWT_Reg<&DWT_Layout::MASK1, 5, 0> MASK;
}  // namespace MASK1

// DWT Comparator Function Register Definitions
namespace FUNCTION1 {
constexpr DWT_Reg<regs::constify(&DWT_Layout::FUNCTION1), 1, 24> MATCHED;
constexpr DWT_Reg<&DWT_Layout::FUNCTION1, 4, 16> DATAVADDR1;
constexpr DWT_Reg<&DWT_Layout::FUNCTION1, 4, 12> DATAVADDR0;
constexpr DWT_Reg<&DWT_Layout::FUNCTION1, 2, 10> DATAVSIZE;
constexpr DWT_Reg<regs::constify(&DWT_Layout::FUNCTION1), 1,  9> LNK1ENA;
constexpr DWT_Reg<&DWT_Layout::FUNCTION1, 1,  8> DATAVMATCH;
constexpr DWT_Reg<&DWT_Layout::FUNCTION1, 1,  5> EMITRANGE;
constexpr DWT_Reg<&DWT_Layout::FUNCTION1, 4,  0> FUNCTION;
}  // namespace FUNCTION1

// DWT Comparator Register Definitions
namespace COMP2 {
constexpr DWT_Reg<&DWT_Layout::COMP2, 32, 0> COMP;
}  // namespace COMP2

// DWT Comparator Mask Register Definitions
namespace MASK2 {
constexpr DWT_Reg<&DWT_Layout::MASK2, 5, 0> MASK;
}  // namespace MASK2

// DWT Comparator Function Register Definitions
namespace FUNCTION2 {
constexpr DWT_Reg<regs::constify(&DWT_Layout::FUNCTION2), 1, 24> MATCHED;
constexpr DWT_Reg<&DWT_Layout::FUNCTION2, 4, 16> DATAVADDR1;
constexpr DWT_Reg<&DWT_Layout::FUNCTION2, 4, 12> DATAVADDR0;
constexpr DWT_Reg<&DWT_Layout::FUNCTION2, 2, 10> DATAVSIZE;
constexpr DWT_Reg<regs::constify(&DWT_Layout::FUNCTION2), 1,  9> LNK1ENA;
constexpr DWT_Reg<&DWT_Layout::FUNCTION2, 1,  8> DATAVMATCH;
constexpr DWT_Reg<&DWT_Layout::FUNCTION2, 1,  5> EMITRANGE;
constexpr DWT_Reg<&DWT_Layout::FUNCTION2, 4,  0> FUNCTION;
}  // namespace FUNCTION2

// DWT Comparator Register Definitions
namespace COMP3 {
constexpr DWT_Reg<&DWT_Layout::COMP3, 32, 0> COMP;
}  // namespace COMP3

// DWT Comparator Mask Register Definitions
namespace MASK3 {
constexpr DWT_Reg<&DWT_Layout::MASK3, 5, 0> MASK;
}  // namespace MASK3

// DWT Comparator Function Register Definitions
namespace FUNCTION3 {
constexpr DWT_Reg<regs::constify(&DWT_Layout::FUNCTION3), 1, 24> MATCHED;
constexpr DWT_Reg<&DWT_Layout::FUNCTION3, 4, 16> DATAVADDR1;
constexpr DWT_Reg<&DWT_Layout::FUNCTION3, 4, 12> DATAVADDR0;
constexpr DWT_Reg<&DWT_Layout::FUNCTION3, 2, 10> DATAVSIZE;
constexpr DWT_Reg<regs::constify(&DWT_Layout::FUNCTION3), 1,  9> LNK1ENA;
constexpr DWT_Reg<&DWT_Layout::FUNCTION3, 1,  8> DATAVMATCH;
constexpr DWT_Reg<&DWT_Layout::FUNCTION3, 1,  5> EMITRANGE;
constexpr DWT_Reg<&DWT_Layout::FUNCTION3, 4,  0> FUNCTION;
}  // namespace FUNCTION3

}  // namespace DWT

}  // namespace imxrt1060
}  // namespace hardware
}  // namespace qindesign
