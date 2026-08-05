// SPDX-FileCopyrightText: (c) 2026 Shawn Silverman <shawn@pobox.com>
// SPDX-License-Identifier: AGPL-3.0-or-later

// DCB.h defines all the DCB registers.
// This file is part of the imxrt1060-regs library.

#pragma once

#include <cstddef>
#include <cstdint>

#include "qnethernet/hardware/regs/regs.h"

namespace qindesign {
namespace hardware {
namespace imxrt1060 {

// Structure type to access the Debug Control Block (DCB) registers.
//
// Comments are from BSD-3-licensed NXP SDK.
//
// See:
// * https://github.com/nxp-mcuxpresso/mcu-sdk-cmsis/blob/mcux_main/Core/Include/core_cm7.h
struct DCB_Layout {
  volatile uint32_t DHCSR;                  /*!< Offset: 0x000 (R/W)  Debug Halting Control and Status Register */
  volatile uint32_t DCRSR;                  /*!< Offset: 0x004 ( /W)  Debug Core Register Selector Register */
  volatile uint32_t DCRDR;                  /*!< Offset: 0x008 (R/W)  Debug Core Register Data Register */
  volatile uint32_t DEMCR;                  /*!< Offset: 0x00C (R/W)  Debug Exception and Monitor Control Register */
};

constexpr size_t    kDCB_size = 0x10;
constexpr uintptr_t kDCB_base = 0xE000'EDF0;  /*!< Core Debug Base Address */

namespace DCB {

constexpr regs::RegGroup<DCB_Layout, kDCB_size, kDCB_base> group;

template <auto Member, size_t Bits, unsigned int Shift,
          auto AssignMask = regs::shiftedMask32<Bits, Shift>(),
          uint32_t AssignSet = 0,
          bool WriteOnly = false>
using DCB_Reg = regs::Reg32<kDCB_base, DCB_Layout, Member, 0, Bits, Shift,
                            AssignMask, AssignSet, WriteOnly>;

// DCB Debug Halting Control and Status Register Definitions
// Exercise caution when setting or assigning fields in this register.
namespace DHCSR {
// TODO: Is this the correct way?
constexpr uint32_t kWO = regs::shiftedMask32<16, 16>();

constexpr DCB_Reg<&DCB_Layout::DHCSR, 16, 16, kWO, 0x0, true> DBGKEY;                                          // Debug key
constexpr DCB_Reg<regs::constify(&DCB_Layout::DHCSR),  1, 25> S_RESET_ST;                                      // Reset sticky status
constexpr DCB_Reg<regs::constify(&DCB_Layout::DHCSR),  1, 24> S_RETIRE_ST;                                     // Retire sticky status
constexpr DCB_Reg<regs::constify(&DCB_Layout::DHCSR),  1, 19> S_LOCKUP;                                        // Lockup status
constexpr DCB_Reg<regs::constify(&DCB_Layout::DHCSR),  1, 18> S_SLEEP;                                         // Sleeping status
constexpr DCB_Reg<regs::constify(&DCB_Layout::DHCSR),  1, 17> S_HALT;                                          // Halted status
constexpr DCB_Reg<regs::constify(&DCB_Layout::DHCSR),  1, 16> S_REGRDY;                                        // Register ready status
constexpr DCB_Reg<regs::constify(&DCB_Layout::DHCSR),  1,  5> C_SNAPSTALL;  // Snap stall control
constexpr DCB_Reg<regs::constify(&DCB_Layout::DHCSR),  1,  3> C_MASKINTS;   // Mask interrupts control
constexpr DCB_Reg<regs::constify(&DCB_Layout::DHCSR),  1,  2> C_STEP;       // Step control
constexpr DCB_Reg<regs::constify(&DCB_Layout::DHCSR),  1,  1> C_HALT;       // Halt control
constexpr DCB_Reg<regs::constify(&DCB_Layout::DHCSR),  1,  0> C_DEBUGEN;    // Debug enable control

// Versions where DBGKEY also gets set
namespace keyed {
constexpr uint32_t kDBGKEY = DBGKEY(0xA05F);

constexpr DCB_Reg<&DCB_Layout::DHCSR, 1, 5, regs::shiftedMask32<1, 5>() | kWO, kDBGKEY> C_SNAPSTALL;
constexpr DCB_Reg<&DCB_Layout::DHCSR, 1, 3, regs::shiftedMask32<1, 3>() | kWO, kDBGKEY> C_MASKINTS;
constexpr DCB_Reg<&DCB_Layout::DHCSR, 1, 2, regs::shiftedMask32<1, 2>() | kWO, kDBGKEY> C_STEP;
constexpr DCB_Reg<&DCB_Layout::DHCSR, 1, 1, regs::shiftedMask32<1, 1>() | kWO, kDBGKEY> C_HALT;
constexpr DCB_Reg<&DCB_Layout::DHCSR, 1, 0, regs::shiftedMask32<1, 0>() | kWO, kDBGKEY> C_DEBUGEN;
}  // namespace keyed
}  // namespace DHCSR

// DCB Debug Core Register Selector Register Definitions
// Exercise caution when setting or assigning fields in this register.
namespace DCRSR {
constexpr DCB_Reg<&DCB_Layout::DCRSR, 1, 16, 0x0, 0x0, true> REGWnR;  // Register write/not-read
constexpr DCB_Reg<&DCB_Layout::DCRSR, 7,  0, 0x0, 0x0, true> REGSEL;  // Register selector
}  // namespace DCRSR

// DCB Debug Core Register Data Register Definitions
namespace DCRDR {
constexpr DCB_Reg<&DCB_Layout::DCRDR, 32, 0> DBGTMP;  // Data temporary buffer
}  // namespace DCRDR

// DCB Debug Exception and Monitor Control Register Definitions
namespace DEMCR {
constexpr DCB_Reg<&DCB_Layout::DEMCR, 1, 24> TRCENA;        // Trace enable
constexpr DCB_Reg<&DCB_Layout::DEMCR, 1, 19> MON_REQ;       // Monitor request
constexpr DCB_Reg<&DCB_Layout::DEMCR, 1, 18> MON_STEP;      // Monitor step
constexpr DCB_Reg<&DCB_Layout::DEMCR, 1, 17> MON_PEND;      // Monitor pend
constexpr DCB_Reg<&DCB_Layout::DEMCR, 1, 16> MON_EN;        // Monitor enable
constexpr DCB_Reg<&DCB_Layout::DEMCR, 1, 10> VC_HARDERR;    // Vector Catch HardFault errors
constexpr DCB_Reg<&DCB_Layout::DEMCR, 1,  9> VC_INTERR;     // Vector Catch interrupt errors
constexpr DCB_Reg<&DCB_Layout::DEMCR, 1,  8> VC_BUSERR;     // Vector Catch BusFault errors
constexpr DCB_Reg<&DCB_Layout::DEMCR, 1,  7> VC_STATERR;    // Vector Catch state errors
constexpr DCB_Reg<&DCB_Layout::DEMCR, 1,  6> VC_CHKERR;     // Vector Catch check errors
constexpr DCB_Reg<&DCB_Layout::DEMCR, 1,  5> VC_NOCPERR;    // Vector Catch NOCP errors
constexpr DCB_Reg<&DCB_Layout::DEMCR, 1,  4> VC_MMERR;      // Vector Catch MemManage errors
constexpr DCB_Reg<&DCB_Layout::DEMCR, 1,  0> VC_CORERESET;  // Vector Catch Core reset
}  // namespace DEMCR

}  // namespace DCB

}  // namespace imxrt1060
}  // namespace hardware
}  // namespace qindesign
