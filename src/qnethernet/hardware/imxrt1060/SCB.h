// SPDX-FileCopyrightText: (c) 2026 Shawn Silverman <shawn@pobox.com>
// SPDX-License-Identifier: AGPL-3.0-or-later

// SCB.h defines all the SCB registers.
// This file is part of the imxrt1060-regs library.

#pragma once

#include <cstddef>
#include <cstdint>
#include <utility>

#include "qnethernet/hardware/regs/regs.h"

namespace qindesign {
namespace hardware {
namespace imxrt1060 {

// Structure type to access the System Control Block (SCB).
//
// Comments are from BSD-3-licensed NXP SDK.
//
// See:
// * https://github.com/nxp-mcuxpresso/mcu-sdk-cmsis/blob/mcux_main/Core/Include/core_cm7.h
struct SCB_Layout {
  const volatile uint32_t CPUID;            /*!< Offset: 0x000 (R/ )  CPUID Base Register */
  volatile uint32_t ICSR;                   /*!< Offset: 0x004 (R/W)  Interrupt Control and State Register */
  volatile uint32_t VTOR;                   /*!< Offset: 0x008 (R/W)  Vector Table Offset Register */
  volatile uint32_t AIRCR;                  /*!< Offset: 0x00C (R/W)  Application Interrupt and Reset Control Register */
  volatile uint32_t SCR;                    /*!< Offset: 0x010 (R/W)  System Control Register */
  volatile uint32_t CCR;                    /*!< Offset: 0x014 (R/W)  Configuration Control Register */
  volatile uint8_t  SHPR[12];               /*!< Offset: 0x018 (R/W)  System Handlers Priority Registers (4-7, 8-11, 12-15) */
  volatile uint32_t SHCSR;                  /*!< Offset: 0x024 (R/W)  System Handler Control and State Register */
  volatile uint32_t CFSR;                   /*!< Offset: 0x028 (R/W)  Configurable Fault Status Register */
  volatile uint32_t HFSR;                   /*!< Offset: 0x02C (R/W)  HardFault Status Register */
  volatile uint32_t DFSR;                   /*!< Offset: 0x030 (R/W)  Debug Fault Status Register */
  volatile uint32_t MMFAR;                  /*!< Offset: 0x034 (R/W)  MemManage Fault Address Register */
  volatile uint32_t BFAR;                   /*!< Offset: 0x038 (R/W)  BusFault Address Register */
  volatile uint32_t AFSR;                   /*!< Offset: 0x03C (R/W)  Auxiliary Fault Status Register */
  const volatile uint32_t ID_PFR[2];        /*!< Offset: 0x040 (R/ )  Processor Feature Register */
  const volatile uint32_t ID_DFR;           /*!< Offset: 0x048 (R/ )  Debug Feature Register */
  const volatile uint32_t ID_AFR;           /*!< Offset: 0x04C (R/ )  Auxiliary Feature Register */
  const volatile uint32_t ID_MMFR[4];       /*!< Offset: 0x050 (R/ )  Memory Model Feature Register */
  const volatile uint32_t ID_ISAR[5];       /*!< Offset: 0x060 (R/ )  Instruction Set Attributes Register */
  uint32_t HARDWARE_REGS_LAYOUT_MEMBER_RESERVED[1];
  const volatile uint32_t CLIDR;            /*!< Offset: 0x078 (R/ )  Cache Level ID register */
  const volatile uint32_t CTR;              /*!< Offset: 0x07C (R/ )  Cache Type register */
  const volatile uint32_t CCSIDR;           /*!< Offset: 0x080 (R/ )  Cache Size ID Register */
  volatile uint32_t CSSELR;                 /*!< Offset: 0x084 (R/W)  Cache Size Selection Register */
  volatile uint32_t CPACR;                  /*!< Offset: 0x088 (R/W)  Coprocessor Access Control Register */
  uint32_t HARDWARE_REGS_LAYOUT_MEMBER_RESERVED[93];
  volatile uint32_t STIR;                   /*!< Offset: 0x200 ( /W)  Software Triggered Interrupt Register */
  uint32_t HARDWARE_REGS_LAYOUT_MEMBER_RESERVED[15];
  const volatile uint32_t MVFR0;            /*!< Offset: 0x240 (R/ )  Media and VFP Feature Register 0 */
  const volatile uint32_t MVFR1;            /*!< Offset: 0x244 (R/ )  Media and VFP Feature Register 1 */
  const volatile uint32_t MVFR2;            /*!< Offset: 0x248 (R/ )  Media and VFP Feature Register 2 */
  uint32_t HARDWARE_REGS_LAYOUT_MEMBER_RESERVED[1];
  volatile uint32_t ICIALLU;                /*!< Offset: 0x250 ( /W)  I-Cache Invalidate All to PoU */
  uint32_t HARDWARE_REGS_LAYOUT_MEMBER_RESERVED[1];
  volatile uint32_t ICIMVAU;                /*!< Offset: 0x258 ( /W)  I-Cache Invalidate by MVA to PoU */
  volatile uint32_t DCIMVAC;                /*!< Offset: 0x25C ( /W)  D-Cache Invalidate by MVA to PoC */
  volatile uint32_t DCISW;                  /*!< Offset: 0x260 ( /W)  D-Cache Invalidate by Set-way */
  volatile uint32_t DCCMVAU;                /*!< Offset: 0x264 ( /W)  D-Cache Clean by MVA to PoU */
  volatile uint32_t DCCMVAC;                /*!< Offset: 0x268 ( /W)  D-Cache Clean by MVA to PoC */
  volatile uint32_t DCCSW;                  /*!< Offset: 0x26C ( /W)  D-Cache Clean by Set-way */
  volatile uint32_t DCCIMVAC;               /*!< Offset: 0x270 ( /W)  D-Cache Clean and Invalidate by MVA to PoC */
  volatile uint32_t DCCISW;                 /*!< Offset: 0x274 ( /W)  D-Cache Clean and Invalidate by Set-way */
  volatile uint32_t BPIALL;                 /*!< Offset: 0x278 ( /W)  Branch Predictor Invalidate All */
  uint32_t HARDWARE_REGS_LAYOUT_MEMBER_RESERVED[5];
  volatile uint32_t ITCMCR;                 /*!< Offset: 0x290 (R/W)  Instruction Tightly-Coupled Memory Control Register */
  volatile uint32_t DTCMCR;                 /*!< Offset: 0x294 (R/W)  Data Tightly-Coupled Memory Control Registers */
  volatile uint32_t AHBPCR;                 /*!< Offset: 0x298 (R/W)  AHBP Control Register */
  volatile uint32_t CACR;                   /*!< Offset: 0x29C (R/W)  L1 Cache Control Register */
  volatile uint32_t AHBSCR;                 /*!< Offset: 0x2A0 (R/W)  AHB Slave Control Register */
  uint32_t HARDWARE_REGS_LAYOUT_MEMBER_RESERVED[1];
  volatile uint32_t ABFSR;                  /*!< Offset: 0x2A8 (R/W)  Auxiliary Bus Fault Status Register */
};

constexpr size_t    kSCB_size = 0x2AC;
namespace SCB {
constexpr uintptr_t kSCS_base = 0xE000'E000;          /*!< System Control Space Base Address */
}  // namespace SCB
constexpr uintptr_t kSCB_base = SCB::kSCS_base + 0x0D00;  /*!< System Control Block Base Address */

namespace SCB {

constexpr regs::RegGroup<SCB_Layout, kSCB_size, kSCB_base> group;

template <auto Member, size_t Bits, unsigned int Shift,
          auto AssignMask = regs::shiftedMask32<Bits, Shift>(),
          uint32_t AssignSet = 0,
          bool WriteOnly = false>
using SCB_Reg = regs::Reg32<kSCB_base, SCB_Layout, Member, 0, Bits, Shift,
                            AssignMask, AssignSet, WriteOnly>;

template <auto Member, size_t MemberOffset, size_t Bits, unsigned int Shift>
using SCB_ArrayReg32 =
    regs::Reg32<kSCB_base, SCB_Layout, Member, MemberOffset, Bits, Shift>;

template <auto Member, size_t MemberOffset, size_t Bits, unsigned int Shift>
using SCB_ArrayReg8 =
    regs::Reg8<kSCB_base, SCB_Layout, Member, MemberOffset, Bits, Shift>;

// CPUID Base Register
namespace CPUID {
constexpr SCB_Reg<&SCB_Layout::CPUID,  8, 24> IMPLEMENTER;   // Implementer code
constexpr SCB_Reg<&SCB_Layout::CPUID,  4, 20> VARIANT;       // Indicates processor revision: 0x2 = Revision 2
constexpr SCB_Reg<&SCB_Layout::CPUID,  4, 16> ARCHITECTURE;  // ARCHITECTURE
constexpr SCB_Reg<&SCB_Layout::CPUID, 12,  4> PARTNO;        // Indicates part number
constexpr SCB_Reg<&SCB_Layout::CPUID,  4,  0> REVISION;      // Indicates patch release: 0x0 = Patch 0
}  // namespace CPUID

// Interrupt Control and State Register
namespace ICSR {
constexpr SCB_Reg<&SCB_Layout::ICSR, 1, 31, 0x0> NMIPENDSET;              // NMI set-pending bit
constexpr SCB_Reg<&SCB_Layout::ICSR, 1, 28, 0x0> PENDSVSET;               // PendSV set-pending bit
constexpr SCB_Reg<&SCB_Layout::ICSR, 1, 27, 0x0, 0x0, true> PENDSVCLR;    // PendSV clear-pending bit
constexpr SCB_Reg<&SCB_Layout::ICSR, 1, 26, 0x0> PENDSTSET;               // SysTick exception set-pending bit
constexpr SCB_Reg<&SCB_Layout::ICSR, 1, 25, 0x0, 0x0, true> PENDSTCLR;    // SysTick exception clear-pending bit
constexpr SCB_Reg<regs::constify(&SCB_Layout::ICSR), 1, 23> ISRPREEMPT;
constexpr SCB_Reg<regs::constify(&SCB_Layout::ICSR), 1, 22> ISRPENDING;   // Interrupt pending flag, excluding NMI and Faults
constexpr SCB_Reg<regs::constify(&SCB_Layout::ICSR), 9, 12> VECTPENDING;  // Exception number of the highest priority pending enabled exception
constexpr SCB_Reg<regs::constify(&SCB_Layout::ICSR), 1, 11> RETTOBASE;    // Indicates whether there are preempted active exceptions
constexpr SCB_Reg<regs::constify(&SCB_Layout::ICSR), 9,  0> VECTACTIVE;   // Active exception number
}  // namespace ICSR

// Vector Table Offset Register
namespace VTOR {

static_assert(sizeof(void (*)()) == sizeof(uint32_t),
              "Function pointer size must be 4 bytes");

using Vector = void (*)();

// Sets an interrupt vector. VTOR must point to a vector table in writable RAM.
inline void setVector(const uint8_t irq, const Vector f) {
  const auto table = reinterpret_cast<uint32_t*>(group->VTOR);
  table[irq + 16] = reinterpret_cast<uint32_t>(f);
  asm volatile ("dsb sy" ::: "memory");
}

inline Vector getVector(const uint8_t irq) {
  const auto table = reinterpret_cast<uint32_t*>(group->VTOR);
  return reinterpret_cast<Vector>(table[irq + 16]);
}

constexpr SCB_Reg<&SCB_Layout::VTOR, 25, 7> TBLOFF;  // Vector table base offset
    // The vector table address actually uses all the bits, not just these

}  // namespace VTOR

// Application Interrupt and Reset Control Register
// Exercise caution when setting or assigning fields in this register.
namespace AIRCR {

// TODO: Is this the correct way?
constexpr uint32_t kWO = 0xffff'0007;

constexpr SCB_Reg<&SCB_Layout::AIRCR, 16, 16, kWO, 0x0, true> VECTKEY;                                      // Register key
constexpr SCB_Reg<regs::constify(&SCB_Layout::AIRCR), 16, 16> VECTKEYSTAT;
constexpr SCB_Reg<regs::constify(&SCB_Layout::AIRCR),  1, 15> ENDIANNESS;                                   // Data endianness
constexpr SCB_Reg<regs::constify(&SCB_Layout::AIRCR),  3,  8, regs::shiftedMask32<3, 8>() | kWO> PRIGROUP;  // Interrupt priority grouping field.
    // This field determines the split of group priority from subpriority.
// constexpr SCB_Reg<&SCB_Layout::AIRCR,  1,  2, kWO, 0x0, true> SYSRESETREQ;                                  // System reset request
// constexpr SCB_Reg<&SCB_Layout::AIRCR,  1,  1, kWO, 0x0, true> VECTCLRACTIVE;                                // Writing 1 to this bit clears all active state information for fixed and configurable exceptions.
// constexpr SCB_Reg<&SCB_Layout::AIRCR,  1,  0, kWO, 0x0, true> VECTRESET;                                    // Writing 1 to this bit causes a local system reset

// Versions where VECTKEY also gets set
namespace keyed {
constexpr uint32_t kVECTKEY = VECTKEY(0x05FA);

constexpr SCB_Reg<&SCB_Layout::AIRCR, 3, 8, regs::shiftedMask32<3, 8>() | kWO, kVECTKEY> PRIGROUP;
constexpr SCB_Reg<&SCB_Layout::AIRCR, 1, 2, kWO, kVECTKEY, true> SYSRESETREQ;
constexpr SCB_Reg<&SCB_Layout::AIRCR, 1, 1, kWO, kVECTKEY, true> VECTCLRACTIVE;
constexpr SCB_Reg<&SCB_Layout::AIRCR, 1, 0, kWO, kVECTKEY, true> VECTRESET;
}  // namespace keyed

[[noreturn]]
inline void systemReset() {
  // Ensure all outstanding memory accesses including buffered writes are
  // completed before reset
  asm volatile("dsb sy" ::: "memory");

  // Keep priority group unchanged
  group->AIRCR =
      VECTKEY(0x05fa) | (group->AIRCR & PRIGROUP.kMask) | keyed::SYSRESETREQ(1);

  // Ensure completion of memory access
  asm volatile("dsb sy" ::: "memory");

  // Wait until reset
  while (true) {
    asm volatile ("nop");
  }

#if (__cplusplus < 202302L)
  __builtin_unreachable();
#else
  std::unreachable();
#endif  // C++ < 23
}

}  // namespace AIRCR

// System Control Register
namespace SCR {
constexpr SCB_Reg<&SCB_Layout::SCR, 1, 4> SEVONPEND;    // Send Event on Pending bit
constexpr SCB_Reg<&SCB_Layout::SCR, 1, 2> SLEEPDEEP;    // Controls whether the processor uses sleep or deep sleep as its low power mode
constexpr SCB_Reg<&SCB_Layout::SCR, 1, 1> SLEEPONEXIT;  // Indicates sleep-on-exit when returning from Handler mode to Thread mode
}  // namespace SCR

// Configuration and Control Register
namespace CCR {
constexpr SCB_Reg<regs::constify(&SCB_Layout::CCR), 1, 18> BP;  // Always reads-as-one.
    // It indicates branch prediction is enabled.
constexpr SCB_Reg<&SCB_Layout::CCR, 1, 17> IC;                  // Enables L1 instruction cache.
constexpr SCB_Reg<&SCB_Layout::CCR, 1, 16> DC;                  // Enables L1 data cache.
constexpr SCB_Reg<&SCB_Layout::CCR, 1,  9> STKALIGN;            // Indicates stack alignment on exception entry
constexpr SCB_Reg<&SCB_Layout::CCR, 1,  8> BFHFNMIGN;           // Enables handlers with priority -1 or -2 to ignore data BusFaults caused by load and store instructions.
constexpr SCB_Reg<&SCB_Layout::CCR, 1,  4> DIV_0_TRP;           // Enables faulting or halting when the processor executes an SDIV or UDIV instruction with a divisor of 0
constexpr SCB_Reg<&SCB_Layout::CCR, 1,  3> UNALIGN_TRP;         // Enables unaligned access traps
constexpr SCB_Reg<&SCB_Layout::CCR, 1,  1> USERSETMPEND;        // Enables unprivileged software access to the STIR
constexpr SCB_Reg<&SCB_Layout::CCR, 1,  0> NONBASETHRDENA;      // Indicates how the processor enters Thread mode
}  // namespace CCR

// The following SHPR fields expose only the four implemented priority bits.
// They occupy bits [7:4] of each architectural 8-bit priority field. These
// wrappers accept and return unshifted priority values in the range 0-15.
// For example, assigning 0xA writes 0xA0 to the corresponding priority byte.

// System Handler Priority Register 1
namespace SHPR1 {
constexpr SCB_ArrayReg8<&SCB_Layout::SHPR, 0, 4, 4> PRI_4;  // Priority of system handler 4, MemManage
constexpr SCB_ArrayReg8<&SCB_Layout::SHPR, 1, 4, 4> PRI_5;  // Priority of system handler 5, BusFault
constexpr SCB_ArrayReg8<&SCB_Layout::SHPR, 2, 4, 4> PRI_6;  // Priority of system handler 6, UsageFault
}  // namespace SHPR1

// System Handler Priority Register 2
namespace SHPR2 {
constexpr SCB_ArrayReg8<&SCB_Layout::SHPR, 7, 4, 4> PRI_11;  // Priority of system handler 11, SVCall
}  // namespace SHPR2

// System Handler Priority Register 3
namespace SHPR3 {
constexpr SCB_ArrayReg8<&SCB_Layout::SHPR, 10, 4, 4> PRI_14;  // Priority of system handler 14, PendSV
constexpr SCB_ArrayReg8<&SCB_Layout::SHPR, 11, 4, 4> PRI_15;  // Priority of system handler 15, SysTick exception
}  // namespace SHPR3

// System Handler Control and State Registers
namespace SHCSR {
constexpr SCB_Reg<&SCB_Layout::SHCSR, 1, 18> USGFAULTENA;     // UsageFault enable bit
constexpr SCB_Reg<&SCB_Layout::SHCSR, 1, 17> BUSFAULTENA;     // BusFault enable bit
constexpr SCB_Reg<&SCB_Layout::SHCSR, 1, 16> MEMFAULTENA;     // MemManage enable bit
constexpr SCB_Reg<&SCB_Layout::SHCSR, 1, 15> SVCALLPENDED;    // SVCall pending bit
constexpr SCB_Reg<&SCB_Layout::SHCSR, 1, 14> BUSFAULTPENDED;  // BusFault exception pending bit
constexpr SCB_Reg<&SCB_Layout::SHCSR, 1, 13> MEMFAULTPENDED;  // MemManage exception pending bit
constexpr SCB_Reg<&SCB_Layout::SHCSR, 1, 12> USGFAULTPENDED;  // UsageFault exception pending bit
constexpr SCB_Reg<&SCB_Layout::SHCSR, 1, 11> SYSTICKACT;      // SysTick exception active bit
constexpr SCB_Reg<&SCB_Layout::SHCSR, 1, 10> PENDSVACT;       // PendSV exception active bit
constexpr SCB_Reg<&SCB_Layout::SHCSR, 1,  8> MONITORACT;      // Debug monitor active bit
constexpr SCB_Reg<&SCB_Layout::SHCSR, 1,  7> SVCALLACT;       // SVCall active bit
constexpr SCB_Reg<&SCB_Layout::SHCSR, 1,  3> USGFAULTACT;     // UsageFault exception active bit
constexpr SCB_Reg<&SCB_Layout::SHCSR, 1,  1> BUSFAULTACT;     // BusFault exception active bit
constexpr SCB_Reg<&SCB_Layout::SHCSR, 1,  0> MEMFAULTACT;     // MemManage exception active bit
}  // namespace SHCSR

// Configurable Fault Status Register
namespace CFSR {
constexpr SCB_Reg<&SCB_Layout::CFSR, 16, 16, 0x0> USGFAULTSR;
constexpr SCB_Reg<&SCB_Layout::CFSR,  8,  8, 0x0> BUSFAULTSR;
constexpr SCB_Reg<&SCB_Layout::CFSR,  8,  0, 0x0> MEMFAULTSR;

// MemManage Fault Status Register Definitions
constexpr SCB_Reg<&SCB_Layout::CFSR, 1, 7, 0x0> MMARVALID;     // MemManage Fault Address Register (MMFAR) valid flag
constexpr SCB_Reg<&SCB_Layout::CFSR, 1, 5, 0x0> MLSPERR;       // MemManage fault occurred during floating-point lazy state preservation
constexpr SCB_Reg<&SCB_Layout::CFSR, 1, 4, 0x0> MSTKERR;       // MemManage fault on stacking for exception entry
constexpr SCB_Reg<&SCB_Layout::CFSR, 1, 3, 0x0> MUNSTKERR;     // MemManage fault on unstacking for a return from exception
constexpr SCB_Reg<&SCB_Layout::CFSR, 1, 1, 0x0> DACCVIOL;      // Data access violation flag
constexpr SCB_Reg<&SCB_Layout::CFSR, 1, 0, 0x0> IACCVIOL;      // Instruction access violation flag

// BusFault Status Register Definitions
constexpr SCB_Reg<&SCB_Layout::CFSR, 1, 15, 0x0> BFARVALID;    // BusFault Address Register (BFAR) valid flag
constexpr SCB_Reg<&SCB_Layout::CFSR, 1, 13, 0x0> LSPERR;       // Bus fault occurred during floating-point lazy state preservation
constexpr SCB_Reg<&SCB_Layout::CFSR, 1, 12, 0x0> STKERR;       // BusFault on stacking for exception entry
constexpr SCB_Reg<&SCB_Layout::CFSR, 1, 11, 0x0> UNSTKERR;     // BusFault on unstacking for a return from exception
constexpr SCB_Reg<&SCB_Layout::CFSR, 1, 10, 0x0> IMPRECISERR;  // Imprecise data bus error
constexpr SCB_Reg<&SCB_Layout::CFSR, 1,  9, 0x0> PRECISERR;    // Precise data bus error
constexpr SCB_Reg<&SCB_Layout::CFSR, 1,  8, 0x0> IBUSERR;      // Instruction bus error

// UsageFault Status Register Definitions
constexpr SCB_Reg<&SCB_Layout::CFSR, 1, 25, 0x0> DIVBYZERO;    // Divide by zero UsageFault
constexpr SCB_Reg<&SCB_Layout::CFSR, 1, 24, 0x0> UNALIGNED;    // Unaligned access UsageFault
constexpr SCB_Reg<&SCB_Layout::CFSR, 1, 19, 0x0> NOCP;         // No coprocessor UsageFault
constexpr SCB_Reg<&SCB_Layout::CFSR, 1, 18, 0x0> INVPC;        // Invalid PC load UsageFault, caused by an invalid PC load by EXC_RETURN
constexpr SCB_Reg<&SCB_Layout::CFSR, 1, 17, 0x0> INVSTATE;     // Invalid state UsageFault
constexpr SCB_Reg<&SCB_Layout::CFSR, 1, 16, 0x0> UNDEFINSTR;   // Undefined instruction UsageFault
}  // namespace CFSR

// HardFault Status register
namespace HFSR {
constexpr SCB_Reg<&SCB_Layout::HFSR, 1, 31, 0x0> DEBUGEVT;  // Reserved for Debug use.
    // When writing to the register you must write 0 to this bit, otherwise behavior is Unpredictable.
constexpr SCB_Reg<&SCB_Layout::HFSR, 1, 30, 0x0> FORCED;    // Indicates a forced hard fault, generated by escalation of a fault with configurable priority that cannot be handles, either because of priority or because it is disabled.
constexpr SCB_Reg<&SCB_Layout::HFSR, 1,  1, 0x0> VECTTBL;   // Indicates a BusFault on a vector table read during exception processing.
}  // namespace HFSR

// Debug Fault Status Register
namespace DFSR {
constexpr SCB_Reg<&SCB_Layout::DFSR, 1, 4, 0x0> EXTERNAL;  // Debug event generated because of the assertion of an external debug request
constexpr SCB_Reg<&SCB_Layout::DFSR, 1, 3, 0x0> VCATCH;    // Indicates triggering of a Vector catch
constexpr SCB_Reg<&SCB_Layout::DFSR, 1, 2, 0x0> DWTTRAP;   // Debug event generated by the DWT
constexpr SCB_Reg<&SCB_Layout::DFSR, 1, 1, 0x0> BKPT;      // Debug event generated by BKPT instruction execution or a breakpoint match in FPB
constexpr SCB_Reg<&SCB_Layout::DFSR, 1, 0, 0x0> HALTED;    // Indicates a debug event generated by either a C_HALT or C_STEP request, triggered by a write to the DHCSR or a step request triggered by setting DEMCR.MON_STEP to 1.
}  // namespace DFSR

// MemManage Fault Address Register
namespace MMFAR {
constexpr SCB_Reg<&SCB_Layout::MMFAR, 32, 0> ADDRESS;  // Address of MemManage fault location
}  // namespace MMFAR

// BusFault Address Register
namespace BFAR {
constexpr SCB_Reg<&SCB_Layout::BFAR, 32, 0> ADDRESS;  // Address of the BusFault location
}  // namespace BFAR

// Auxiliary Fault Status Register
namespace AFSR {
constexpr SCB_Reg<&SCB_Layout::AFSR, 32, 0> IMPDEF;  // Implementation defined
}  // namespace AFSR

// Processor Feature Register 0
namespace ID_PFR0 {
constexpr SCB_ArrayReg32<&SCB_Layout::ID_PFR, 0, 4, 12> STATE3;  // ARMv7-M unused
constexpr SCB_ArrayReg32<&SCB_Layout::ID_PFR, 0, 4,  8> STATE2;  // ARMv7-M unused
constexpr SCB_ArrayReg32<&SCB_Layout::ID_PFR, 0, 4,  4> STATE1;  // Thumb instruction set support
constexpr SCB_ArrayReg32<&SCB_Layout::ID_PFR, 0, 4,  0> STATE0;  // ARM instruction set support
}  // namespace ID_PFR0

// Processor Feature Register 1
namespace ID_PFR1 {
constexpr SCB_ArrayReg32<&SCB_Layout::ID_PFR, 1, 4, 8> PROGMODEL;  // M profile programmers' model
}  // namespace ID_PFR1

// Debug Feature Register
namespace ID_DFR {
constexpr SCB_Reg<&SCB_Layout::ID_DFR, 4, 20> DEBUGMODEL;  // Support for memory-mapped debug model for M profile processors
}  // namespace ID_DFR

// Auxiliary Feature Register
namespace ID_AFR {
constexpr SCB_Reg<&SCB_Layout::ID_AFR, 4, 12> IMPLEMENTATION_DEFINED3;  // Gives information about the IMPLEMENTATION DEFINED features of a processor implementation.
constexpr SCB_Reg<&SCB_Layout::ID_AFR, 4,  8> IMPLEMENTATION_DEFINED2;  // Gives information about the IMPLEMENTATION DEFINED features of a processor implementation.
constexpr SCB_Reg<&SCB_Layout::ID_AFR, 4,  4> IMPLEMENTATION_DEFINED1;  // Gives information about the IMPLEMENTATION DEFINED features of a processor implementation.
constexpr SCB_Reg<&SCB_Layout::ID_AFR, 4,  0> IMPLEMENTATION_DEFINED0;  // Gives information about the IMPLEMENTATION DEFINED features of a processor implementation.
}  // namespace ID_AFR

// Memory Model Feature Register 0
namespace ID_MMFR0 {
constexpr SCB_ArrayReg32<&SCB_Layout::ID_MMFR, 0, 4, 20> AUXILIARY_REGISTERS;     // Indicates the support for Auxiliary registers
constexpr SCB_ArrayReg32<&SCB_Layout::ID_MMFR, 0, 4, 16> TCM_SUPPORT;             // Indicates the support for Tightly Coupled Memory
constexpr SCB_ArrayReg32<&SCB_Layout::ID_MMFR, 0, 4, 12> SHAREABILITY_LEVELS;     // Indicates the number of shareability levels implemented
constexpr SCB_ArrayReg32<&SCB_Layout::ID_MMFR, 0, 4,  8> OUTERMOST_SHAREABILITY;  // Indicates the outermost shareability domain implemented
constexpr SCB_ArrayReg32<&SCB_Layout::ID_MMFR, 0, 4,  4> PMSASUPPORT;             // Indicates support for a PMSA
}  // namespace ID_MMFR0

// Memory Model Feature Register 2
namespace ID_MMFR2 {
constexpr SCB_ArrayReg32<&SCB_Layout::ID_MMFR, 2, 4, 24> WFI_STALL;  // Indicates the support for Wait For Interrupt (WFI) stalling
}  // namespace ID_MMFR2

// Instruction Set Attributes Register 0
namespace ID_ISAR0 {
constexpr SCB_ArrayReg32<&SCB_Layout::ID_ISAR, 0, 4, 24> DIVIDE_INSTRS;     // Indicates the supported Divide instructions
constexpr SCB_ArrayReg32<&SCB_Layout::ID_ISAR, 0, 4, 20> DEBUG_INSTRS;      // Indicates the supported Debug instructions
constexpr SCB_ArrayReg32<&SCB_Layout::ID_ISAR, 0, 4, 16> COPROC_INSTRS;     // Indicates the supported Coprocessor instructions
constexpr SCB_ArrayReg32<&SCB_Layout::ID_ISAR, 0, 4, 12> CMPBRANCH_INSTRS;  // Indicates the supported combined Compare and Branch instructions
constexpr SCB_ArrayReg32<&SCB_Layout::ID_ISAR, 0, 4,  8> BITFIELD_INSTRS;   // Indicates the supported BitField instructions
constexpr SCB_ArrayReg32<&SCB_Layout::ID_ISAR, 0, 4,  4> BITCOUNT_INSTRS;   // Indicates the supported Bit Counting instructions
}  // namespace ID_ISAR0

// Instruction Set Attributes Register 1
namespace ID_ISAR1 {
constexpr SCB_ArrayReg32<&SCB_Layout::ID_ISAR, 1, 4, 24> INTERWORK_INSTRS;  // Indicates the supported Interworking instructions
constexpr SCB_ArrayReg32<&SCB_Layout::ID_ISAR, 1, 4, 20> IMMEDIATE_INSTRS;  // Indicates the support for data-processing instructions with long immediate
constexpr SCB_ArrayReg32<&SCB_Layout::ID_ISAR, 1, 4, 16> IFTHEN_INSTRS;     // Indicates the supported IfThen instructions
constexpr SCB_ArrayReg32<&SCB_Layout::ID_ISAR, 1, 4, 12> EXTEND_INSTRS;     // Indicates the supported Extend instructions
}  // namespace ID_ISAR1

// Instruction Set Attributes Register 2
namespace ID_ISAR2 {
constexpr SCB_ArrayReg32<&SCB_Layout::ID_ISAR, 2, 4, 28> REVERSAL_INSTRS;        // Indicates the supported Reversal instructions
constexpr SCB_ArrayReg32<&SCB_Layout::ID_ISAR, 2, 4, 20> MULTU_INSTRS;           // Indicates the supported advanced unsigned Multiply instructions
constexpr SCB_ArrayReg32<&SCB_Layout::ID_ISAR, 2, 4, 16> MULTS_INSTRS;           // Indicates the supported advanced signed Multiply instructions
constexpr SCB_ArrayReg32<&SCB_Layout::ID_ISAR, 2, 4, 12> MULT_INSTRS;            // Indicates the supported additional Multiply instructions
constexpr SCB_ArrayReg32<&SCB_Layout::ID_ISAR, 2, 4,  8> MULTIACCESSINT_INSTRS;  // Indicates the support for multi-access interruptible instructions
constexpr SCB_ArrayReg32<&SCB_Layout::ID_ISAR, 2, 4,  4> MEMHINT_INSTRS;         // Indicates the supported Memory Hint instructions
constexpr SCB_ArrayReg32<&SCB_Layout::ID_ISAR, 2, 4,  0> LOADSTORE_INSTRS;       // Indicates the supported additional load and store instructions
}  // namespace ID_ISAR2

// Instruction Set Attributes Register 3
namespace ID_ISAR3 {
constexpr SCB_ArrayReg32<&SCB_Layout::ID_ISAR, 3, 4, 24> TRUENOP_INSTRS;    // Indicates the supported non flag-setting MOV instructions
constexpr SCB_ArrayReg32<&SCB_Layout::ID_ISAR, 3, 4, 20> THUMBCOPY_INSTRS;  // Indicates the supported non flag-setting MOV instructions
constexpr SCB_ArrayReg32<&SCB_Layout::ID_ISAR, 3, 4, 16> TABBRANCH_INSTRS;  // Indicates the supported Table Branch instructions
constexpr SCB_ArrayReg32<&SCB_Layout::ID_ISAR, 3, 4, 12> SYNCHPRIM_INSTRS;  // Together with the ID_ISAR4[SYNCHPRIM_INSTRS_FRAC] indicates the supported Synchronization Primitives
constexpr SCB_ArrayReg32<&SCB_Layout::ID_ISAR, 3, 4,  8> SVC_INSTRS;        // Indicates the supported SVC instructions
constexpr SCB_ArrayReg32<&SCB_Layout::ID_ISAR, 3, 4,  4> SIMD_INSTRS;       // Indicates the supported SIMD instructions
constexpr SCB_ArrayReg32<&SCB_Layout::ID_ISAR, 3, 4,  0> SATURATE_INSTRS;   // Indicates the supported Saturate instructions
}  // namespace ID_ISAR3

// Instruction Set Attributes Register 4
namespace ID_ISAR4 {
constexpr SCB_ArrayReg32<&SCB_Layout::ID_ISAR, 4, 4, 24> PSR_M_INSTRS;           // Indicates the supported M profile instructions to modify the PSRs
constexpr SCB_ArrayReg32<&SCB_Layout::ID_ISAR, 4, 4, 20> SYNCHPRIM_INSTRS_FRAC;  // Together with the ID_ISAR3[SYNCHPRIM_INSTRS] indicates the supported Synchronization Primitives
constexpr SCB_ArrayReg32<&SCB_Layout::ID_ISAR, 4, 4, 16> BARRIER_INSTRS;         // Indicates the supported Barrier instructions
constexpr SCB_ArrayReg32<&SCB_Layout::ID_ISAR, 4, 4,  8> WRITEBACK_INSTRS;       // Indicates the support for Writeback addressing modes
constexpr SCB_ArrayReg32<&SCB_Layout::ID_ISAR, 4, 4,  4> WITHSHIFTS_INSTRS;      // Indicates the support for instructions with shifts
constexpr SCB_ArrayReg32<&SCB_Layout::ID_ISAR, 4, 4,  0> UNPRIV_INSTRS;          // Indicates the supported unprivileged instructions.
    // These are the instruction variants indicated by a T suffix.
}  // namespace ID_ISAR4

// Cache Level ID Register
namespace CLIDR {
constexpr SCB_Reg<&SCB_Layout::CLIDR, 3, 27> LOUU;   // Level of Unification Uniprocessor for the cache hierarchy
constexpr SCB_Reg<&SCB_Layout::CLIDR, 3, 24> LOC;    // Level of Coherency for the cache hierarchy
constexpr SCB_Reg<&SCB_Layout::CLIDR, 3, 21> LOUIS;  // Level of Unification Inner Shareable for the cache hierarchy.
    // This field is RAZ.
constexpr SCB_Reg<&SCB_Layout::CLIDR, 3, 18> CL7;    // Indicate the type of cache implemented at level 7.
constexpr SCB_Reg<&SCB_Layout::CLIDR, 3, 15> CL6;    // Indicate the type of cache implemented at level 6.
constexpr SCB_Reg<&SCB_Layout::CLIDR, 3, 12> CL5;    // Indicate the type of cache implemented at level 5.
constexpr SCB_Reg<&SCB_Layout::CLIDR, 3,  9> CL4;    // Indicate the type of cache implemented at level 4.
constexpr SCB_Reg<&SCB_Layout::CLIDR, 3,  6> CL3;    // Indicate the type of cache implemented at level 3.
constexpr SCB_Reg<&SCB_Layout::CLIDR, 3,  3> CL2;    // Indicate the type of cache implemented at level 2.
constexpr SCB_Reg<&SCB_Layout::CLIDR, 3,  0> CL1;    // Indicate the type of cache implemented at level 1.
}  // namespace CLIDR

// Cache Type Register
namespace CTR {
constexpr SCB_Reg<&SCB_Layout::CTR, 3, 29> FORMAT;    // Indicates the implemented CTR format.
constexpr SCB_Reg<&SCB_Layout::CTR, 4, 24> CWG;       // Cache Write-back Granule.
    // The maximum size of memory that can be overwritten as a result of the eviction of a cache entry that has had a memory location in it modified, encoded as Log2 of the number of words.
constexpr SCB_Reg<&SCB_Layout::CTR, 4, 20> ERG;       // Exclusives Reservation Granule.
    // The maximum size of the reservation granule that has been implemented for the Load-Exclusive and Store-Exclusive instructions, encoded as Log2 of the number of words.
constexpr SCB_Reg<&SCB_Layout::CTR, 4, 16> DMINLINE;  // Log2 of the number of words in the smallest cache line of all the data caches and unified caches that are controlled by the processor.
constexpr SCB_Reg<&SCB_Layout::CTR, 4,  0> IMINLINE;  // Log2 of the number of words in the smallest cache line of all the instruction caches that are controlled by the processor.
}  // namespace CTR

// Cache Size ID Register
namespace CCSIDR {
constexpr SCB_Reg<&SCB_Layout::CCSIDR,  1, 31> WT;             // Indicates whether the cache level supports write-through
constexpr SCB_Reg<&SCB_Layout::CCSIDR,  1, 30> WB;             // Indicates whether the cache level supports write-back
constexpr SCB_Reg<&SCB_Layout::CCSIDR,  1, 29> RA;             // Indicates whether the cache level supports read-allocation
constexpr SCB_Reg<&SCB_Layout::CCSIDR,  1, 28> WA;             // Indicates whether the cache level supports write-allocation
constexpr SCB_Reg<&SCB_Layout::CCSIDR, 15, 13> NUMSETS;        // (Number of sets in cache) - 1, therefore a value of 0 indicates 1 set in the cache.
    // The number of sets does not have to be a power of 2.
constexpr SCB_Reg<&SCB_Layout::CCSIDR, 10,  3> ASSOCIATIVITY;  // (Associativity of cache) - 1, therefore a value of 0 indicates an associativity of 1.
    // The associativity does not have to be a power of 2.
constexpr SCB_Reg<&SCB_Layout::CCSIDR,  3,  0> LINESIZE;       // (Log2(Number of words in cache line)) - 2.
}  // namespace CCSIDR

// Cache Size Selection Register
namespace CSSELR {
constexpr SCB_Reg<&SCB_Layout::CSSELR, 3, 1> LEVEL;  // Cache level of required cache
constexpr SCB_Reg<&SCB_Layout::CSSELR, 1, 0> IND;    // Instruction not data bit
}  // namespace CSSELR

// Coprocessor Access Control Register
namespace CPACR {
constexpr SCB_Reg<&SCB_Layout::CPACR, 2, 22> CP11;  // Access privileges for coprocessor 11.
constexpr SCB_Reg<&SCB_Layout::CPACR, 2, 20> CP10;  // Access privileges for coprocessor 10.
constexpr SCB_Reg<&SCB_Layout::CPACR, 2, 14> CP7;   // Access privileges for coprocessor 7.
constexpr SCB_Reg<&SCB_Layout::CPACR, 2, 12> CP6;   // Access privileges for coprocessor 6.
constexpr SCB_Reg<&SCB_Layout::CPACR, 2, 10> CP5;   // Access privileges for coprocessor 5.
constexpr SCB_Reg<&SCB_Layout::CPACR, 2,  8> CP4;   // Access privileges for coprocessor 4.
constexpr SCB_Reg<&SCB_Layout::CPACR, 2,  6> CP3;   // Access privileges for coprocessor 3.
constexpr SCB_Reg<&SCB_Layout::CPACR, 2,  4> CP2;   // Access privileges for coprocessor 2.
constexpr SCB_Reg<&SCB_Layout::CPACR, 2,  2> CP1;   // Access privileges for coprocessor 1.
constexpr SCB_Reg<&SCB_Layout::CPACR, 2,  0> CP0;   // Access privileges for coprocessor 0.
}  // namespace CPACR

// Software Triggered Interrupt Register
namespace STIR {
constexpr SCB_Reg<&SCB_Layout::STIR, 9, 0, 0x0, 0x0, true> INTID;  // Indicates the interrupt to be triggered
}  // namespace STIR

// Media and VFP Feature Register 0
namespace MVFR0 {
constexpr SCB_Reg<&SCB_Layout::MVFR0, 4, 28> FPRound;      // Rounding modes bits
constexpr SCB_Reg<&SCB_Layout::MVFR0, 4, 24> FPShortvec;   // Short vectors bits
constexpr SCB_Reg<&SCB_Layout::MVFR0, 4, 20> FPSqrt;       // Square root bits
constexpr SCB_Reg<&SCB_Layout::MVFR0, 4, 16> FPDivide;     // Divide bits
constexpr SCB_Reg<&SCB_Layout::MVFR0, 4, 12> FPExceptrap;  // Exception trapping bits
constexpr SCB_Reg<&SCB_Layout::MVFR0, 4,  8> FPDP;         // Double-precision bits
constexpr SCB_Reg<&SCB_Layout::MVFR0, 4,  4> FPSP;         // Single-precision bits
constexpr SCB_Reg<&SCB_Layout::MVFR0, 4,  0> SIMDReg;      // SIMD registers bits
}  // namespace MVFR0

// Media and VFP Feature Register 1
namespace MVFR1 {
constexpr SCB_Reg<&SCB_Layout::MVFR1, 4, 28> FMAC;    // Fused MAC bits
constexpr SCB_Reg<&SCB_Layout::MVFR1, 4, 24> FPHP;    // FP HPFP bits
constexpr SCB_Reg<&SCB_Layout::MVFR1, 4,  4> FPDNaN;  // D_NaN mode bits
constexpr SCB_Reg<&SCB_Layout::MVFR1, 4,  0> FPFtZ;   // FtZ mode bits
}  // namespace MVFR1

// Media and VFP Feature Register 2
namespace MVFR2 {
constexpr SCB_Reg<&SCB_Layout::MVFR2, 4, 4> FPMisc;  // VFP Misc bits
}  // namespace MVFR2

// Instruction cache invalidate all to Point of Unification (PoU)
namespace ICIALLU {
constexpr SCB_Reg<&SCB_Layout::ICIALLU, 32, 0, 0x0, 0x0, true> ICIALLU;  // I-cache invalidate all to PoU
}  // namespace ICIALLU

// Instruction cache invalidate by address to PoU
namespace ICIMVAU {
constexpr SCB_Reg<&SCB_Layout::ICIMVAU, 32, 0, 0x0, 0x0, true> ICIMVAU;  // I-cache invalidate by MVA to PoU
}  // namespace ICIMVAU

// Data cache invalidate by address to Point of Coherency (PoC)
namespace DCIMVAC {
constexpr SCB_Reg<&SCB_Layout::DCIMVAC, 32, 0, 0x0, 0x0, true> DCIMVAC;  // D-cache invalidate by MVA to PoC
}  // namespace DCIMVAC

// Data cache invalidate by set/way
// Exercise caution when setting or assigning fields in this register.
namespace DCISW {
constexpr SCB_Reg<&SCB_Layout::DCISW, 2, 30, 0x0, 0x0, true> WAY;
constexpr SCB_Reg<&SCB_Layout::DCISW, 9,  5, 0x0, 0x0, true> SET;
}  // namespace DCISW

// Data cache by address to PoU
namespace DCCMVAU {
constexpr SCB_Reg<&SCB_Layout::DCCMVAU, 32, 0, 0x0, 0x0, true> DCCMVAU;  // D-cache clean by MVA to PoU
}  // namespace DCCMVAU

// Data cache clean by address to PoC
namespace DCCMVAC {
constexpr SCB_Reg<&SCB_Layout::DCCMVAC, 32, 0, 0x0, 0x0, true> DCCMVAC;  // D-cache clean by MVA to PoC
}  // namespace DCCMVAC

// Data cache clean by set/way
// Exercise caution when setting or assigning fields in this register.
namespace DCCSW {
constexpr SCB_Reg<&SCB_Layout::DCCSW, 2, 30, 0x0, 0x0, true> WAY;
constexpr SCB_Reg<&SCB_Layout::DCCSW, 9,  5, 0x0, 0x0, true> SET;
}  // namespace DCCSW

// Data cache clean and invalidate by address to PoC
namespace DCCIMVAC {
constexpr SCB_Reg<&SCB_Layout::DCCIMVAC, 32, 0, 0x0, 0x0, true> DCCIMVAC;  // D-cache clean and invalidate by MVA to PoC
}  // namespace DCCIMVAC

// Data cache clean and invalidate by set/way
// Exercise caution when setting or assigning fields in this register.
namespace DCCISW {
constexpr SCB_Reg<&SCB_Layout::DCCISW, 2, 30, 0x0, 0x0, true> WAY;
constexpr SCB_Reg<&SCB_Layout::DCCISW, 9,  5, 0x0, 0x0, true> SET;
}  // namespace DCCISW

// Branch Predictor Invalidate All
namespace BPIALL {
constexpr SCB_Reg<&SCB_Layout::BPIALL, 32, 0, 0x0, 0x0, true> BPIALL;
}  // namespace BPIALL

// Instruction Tightly-Coupled Memory Control Register
namespace ITCMCR {
constexpr SCB_Reg<regs::constify(&SCB_Layout::ITCMCR), 4, 3> SZ;  // TCM size.
    // Indicates the size of the relevant TCM.
constexpr SCB_Reg<&SCB_Layout::ITCMCR, 1, 2> RETEN;               // Retry phase enable.
    // When enabled the processor guarantees to honor the retry output on the corresponding TCM interface, re-executing the instruction which carried out the TCM access.
constexpr SCB_Reg<&SCB_Layout::ITCMCR, 1, 1> RMW;                 // Read-Modify-Write (RMW) enable.
    // Indicates that all writes to TCM, that are not the full width of the TCM RAM, use a RMW sequence.
constexpr SCB_Reg<&SCB_Layout::ITCMCR, 1, 0> EN;                  // TCM enable.
    // When a TCM is disabled all accesses are made to the AXIM interface.
}  // namespace ITCMCR

// Data Tightly-Coupled Memory Control Register
namespace DTCMCR {
constexpr SCB_Reg<regs::constify(&SCB_Layout::DTCMCR), 4, 3> SZ;  // TCM size.
    // Indicates the size of the relevant TCM.
constexpr SCB_Reg<&SCB_Layout::DTCMCR, 1, 2> RETEN;               // Retry phase enable.
    // When enabled the processor guarantees to honor the retry output on the corresponding TCM interface, re-executing the instruction which carried out the TCM access.
constexpr SCB_Reg<&SCB_Layout::DTCMCR, 1, 1> RMW;                 // Read-Modify-Write (RMW) enable.
    // Indicates that all writes to TCM, that are not the full width of the TCM RAM, use a RMW sequence.
constexpr SCB_Reg<&SCB_Layout::DTCMCR, 1, 0> EN;                  // TCM enable.
    // When a TCM is disabled all accesses are made to the AXIM interface.
}  // namespace DTCMCR

// AHBP Control Register
namespace AHBPCR {
constexpr SCB_Reg<regs::constify(&SCB_Layout::AHBPCR), 3, 1> SZ;  // AHBP size.
constexpr SCB_Reg<&SCB_Layout::AHBPCR, 1, 0> EN;                  // AHBP enable.
}  // namespace AHBPCR

// L1 Cache Control Register
namespace CACR {
constexpr SCB_Reg<&SCB_Layout::CACR, 1, 2> FORCEWT;  // Enables Force Write-Through in the data cache.
constexpr SCB_Reg<&SCB_Layout::CACR, 1, 1> ECCDIS;   // Enables ECC in the instruction and data cache.
constexpr SCB_Reg<&SCB_Layout::CACR, 1, 0> SIWT;     // Shared cacheable-is-WT for data cache.
    // Enables limited cache coherency usage.
}  // namespace CACR

// AHB Slave Control Register
namespace AHBSCR {
constexpr SCB_Reg<&SCB_Layout::AHBSCR, 5, 11> INITCOUNT;  // Fairness counter initialization value.
constexpr SCB_Reg<&SCB_Layout::AHBSCR, 9,  2> TPRI;       // Threshold execution priority for AHBS traffic demotion.
constexpr SCB_Reg<&SCB_Layout::AHBSCR, 2,  0> CTL;        // AHBS prioritization control.
}  // namespace AHBSCR

// Auxiliary Bus Fault Status Register
// Note: Cleared to 0x0 on writes of any value
namespace ABFSR {
constexpr SCB_Reg<regs::constify(&SCB_Layout::ABFSR), 2, 8> AXIMTYPE;  // Indicates the type of fault on the AXIM interface.
    // Only valid when AXIM is 1.
constexpr SCB_Reg<regs::constify(&SCB_Layout::ABFSR), 1, 4> EPPB;      // Asynchronous fault on EPPB interface.
constexpr SCB_Reg<regs::constify(&SCB_Layout::ABFSR), 1, 3> AXIM;      // Asynchronous fault on AXIM interface.
constexpr SCB_Reg<regs::constify(&SCB_Layout::ABFSR), 1, 2> AHBP;      // Asynchronous fault on AHBP interface.
constexpr SCB_Reg<regs::constify(&SCB_Layout::ABFSR), 1, 1> DTCM;      // Asynchronous fault on DTCM interface.
constexpr SCB_Reg<regs::constify(&SCB_Layout::ABFSR), 1, 0> ITCM;      // Asynchronous fault on ITCM interface.
}  // namespace ABFSR

}  // namespace SCB

}  // namespace imxrt1060
}  // namespace hardware
}  // namespace qindesign
