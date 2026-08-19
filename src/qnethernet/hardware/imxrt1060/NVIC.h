// SPDX-FileCopyrightText: (c) 2026 Shawn Silverman <shawn@pobox.com>
// SPDX-License-Identifier: AGPL-3.0-or-later

// NVIC.h defines all the NVIC registers.
// This file is part of the imxrt1060-regs library.

#pragma once

#include <cstddef>
#include <cstdint>

#include "qnethernet/hardware/regs/regs.h"

namespace qindesign {
namespace hardware {
namespace imxrt1060 {

// Structure type to access the Nested Vectored Interrupt Controller
// (NVIC) registers.
//
// Comments are from BSD-3-licensed NXP SDK.
//
// See:
// * https://github.com/nxp-mcuxpresso/mcu-sdk-cmsis/blob/mcux_main/Core/Include/core_cm7.h
struct NVIC_Layout {
  volatile uint32_t ISER[8];               /*!< w1s, Offset: 0x000 (R/W)  Interrupt Set Enable Register */
  uint32_t HARDWARE_REGS_LAYOUT_MEMBER_RESERVED[24];
  volatile uint32_t ICER[8];               /*!< w1c, Offset: 0x080 (R/W)  Interrupt Clear Enable Register */
  uint32_t HARDWARE_REGS_LAYOUT_MEMBER_RESERVED[24];
  volatile uint32_t ISPR[8];               /*!< w1s, Offset: 0x100 (R/W)  Interrupt Set Pending Register */
  uint32_t HARDWARE_REGS_LAYOUT_MEMBER_RESERVED[24];
  volatile uint32_t ICPR[8];               /*!< w1c, Offset: 0x180 (R/W)  Interrupt Clear Pending Register */
  uint32_t HARDWARE_REGS_LAYOUT_MEMBER_RESERVED[24];
  volatile uint32_t IABR[8];               /*!< Offset: 0x200 (R/W)  Interrupt Active bit Register */
  uint32_t HARDWARE_REGS_LAYOUT_MEMBER_RESERVED[56];
  volatile uint8_t  IPR[240];              /*!< Offset: 0x300 (R/W)  Interrupt Priority Register (8Bit wide) */
  uint32_t HARDWARE_REGS_LAYOUT_MEMBER_RESERVED[644];
  volatile uint32_t STIR;                  /*!< Offset: 0xE00 ( /W)  Software Trigger Interrupt Register */
};

constexpr size_t    kNVIC_size = 0xE04;
namespace NVIC {
constexpr uintptr_t kSCS_base = 0xE000'E000;           /*!< System Control Space Base Address */
}  // namespace NVIC
constexpr uintptr_t kNVIC_base = NVIC::kSCS_base + 0x0100;  /*!< NVIC Base Address */

namespace NVIC {

constexpr regs::RegGroup<NVIC_Layout, kNVIC_size, kNVIC_base> group;

template <auto Member, size_t Bits, unsigned int Shift,
          auto AssignMask = regs::shiftedMask32<Bits, Shift>(),
          bool WriteOnly = false>
using NVIC_Reg = regs::Reg32<kNVIC_base, NVIC_Layout, Member, 0, Bits, Shift,
                             AssignMask, 0, WriteOnly>;

// Software Trigger Interrupt Register
namespace STIR {
constexpr NVIC_Reg<&NVIC_Layout::STIR, 9, 0, 0x0, true> INTID;  // Interrupt ID of the interrupt to trigger, in the range 0-239.
    // For example, a value of 0x03 specifies interrupt IRQ3.
}  // namespace STIR

// Interrupt Priority Register
namespace IPR {
namespace vals {
constexpr regs::RegValue8<4, 4> PRI;  // Priority of the Nth interrupt
}  // namespace vals
}  // namespace IPR

namespace IRQ {

// Enables the specified IRQ.
inline void enable(const uint8_t irq) {
  asm volatile ("" ::: "memory");
  group->ISER[irq >> 5] = (uint32_t{1} << (irq & 0x1f));
  asm volatile ("" ::: "memory");
}

// Disables the specified IRQ.
inline void disable(const uint8_t irq) {
  group->ICER[irq >> 5] = (uint32_t{1} << (irq & 0x1f));
  asm volatile ("dsb sy" ::: "memory");
  asm volatile ("isb sy" ::: "memory");
}

// Checks if the specified IRQ is enabled.
inline bool isEnabled(const uint8_t irq) {
  return (group->ISER[irq >> 5] & (uint32_t{1} << (irq & 0x1f))) != 0;
}

// Sets the specified IRQ to be pending.
inline void setPending(const uint8_t irq) {
  group->ISPR[irq >> 5] = (uint32_t{1} << (irq & 0x1f));
}

// Clears pending state of the specified IRQ.
inline void clearPending(const uint8_t irq) {
  group->ICPR[irq >> 5] = (uint32_t{1} << (irq & 0x1f));
}

// Checks if the specified IRQ is pending.
inline bool isPending(const uint8_t irq) {
  return (group->ISPR[irq >> 5] & (uint32_t{1} << (irq & 0x1f))) != 0;
}

// Sets the priority of the given IRQ. The IRQ must be in range.
inline void setPriority(const uint8_t irq, const uint8_t pri) {
  group->IPR[irq] = pri;
}

// Gets the priority of the given IRQ. The IRQ must be in range.
inline uint8_t getPriority(const uint8_t irq) {
  return group->IPR[irq];
}

// Checks if the specified IRQ is active.
inline bool isActive(const uint8_t irq) {
  return (group->IABR[irq >> 5] & (uint32_t{1} << (irq & 0x1f))) != 0;
}

constexpr uint8_t kDMA0_DMA16           =   0;
constexpr uint8_t kDMA1_DMA17           =   1;
constexpr uint8_t kDMA2_DMA18           =   2;
constexpr uint8_t kDMA3_DMA19           =   3;
constexpr uint8_t kDMA4_DMA20           =   4;
constexpr uint8_t kDMA5_DMA21           =   5;
constexpr uint8_t kDMA6_DMA22           =   6;
constexpr uint8_t kDMA7_DMA23           =   7;
constexpr uint8_t kDMA8_DMA24           =   8;
constexpr uint8_t kDMA9_DMA25           =   9;
constexpr uint8_t kDMA10_DMA26          =  10;
constexpr uint8_t kDMA11_DMA27          =  11;
constexpr uint8_t kDMA12_DMA28          =  12;
constexpr uint8_t kDMA13_DMA29          =  13;
constexpr uint8_t kDMA14_DMA30          =  14;
constexpr uint8_t kDMA15_DMA31          =  15;
constexpr uint8_t kDMA_ERROR            =  16;
constexpr uint8_t kCTI0_ERROR           =  17;
constexpr uint8_t kCTI1_ERROR           =  18;
constexpr uint8_t kCORE                 =  19;
constexpr uint8_t kLPUART1              =  20;
constexpr uint8_t kLPUART2              =  21;
constexpr uint8_t kLPUART3              =  22;
constexpr uint8_t kLPUART4              =  23;
constexpr uint8_t kLPUART5              =  24;
constexpr uint8_t kLPUART6              =  25;
constexpr uint8_t kLPUART7              =  26;
constexpr uint8_t kLPUART8              =  27;
constexpr uint8_t kLPI2C1               =  28;
constexpr uint8_t kLPI2C2               =  29;
constexpr uint8_t kLPI2C3               =  30;
constexpr uint8_t kLPI2C4               =  31;
constexpr uint8_t kLPSPI1               =  32;
constexpr uint8_t kLPSPI2               =  33;
constexpr uint8_t kLPSPI3               =  34;
constexpr uint8_t kLPSPI4               =  35;
constexpr uint8_t kCAN1                 =  36;  // FLEXCAN1
constexpr uint8_t kCAN2                 =  37;  // FLEXCAN2
constexpr uint8_t kFLEXRAM              =  38;  // FlexRAM address out of range Or access hit IRQ
constexpr uint8_t kKPP                  =  39;
constexpr uint8_t kTSC_DIG              =  40;
constexpr uint8_t kGPR_IRQ              =  41;
constexpr uint8_t kLCDIF                =  42;
constexpr uint8_t kCSI                  =  43;
constexpr uint8_t kPXP                  =  44;
constexpr uint8_t kWDOG2                =  45;
constexpr uint8_t kSNVS_HP_WRAPPER      =  46;
constexpr uint8_t kSNVS_HP_WRAPPER_TZ   =  47;
constexpr uint8_t kSNVS_LP_WRAPPER      =  48;  // SNVS_ONOFF, ON-OFF button press shorter than 5 secs (pulse event)
constexpr uint8_t kCSU                  =  49;
constexpr uint8_t kDCP                  =  50;
constexpr uint8_t kDCP_VMI              =  51;
// constexpr uint8_t kReserved1            =  52;
constexpr uint8_t kTRNG                 =  53;
constexpr uint8_t kSJC                  =  54;  // Reserved in reference manual
constexpr uint8_t kBEE                  =  55;
constexpr uint8_t kSAI1                 =  56;
constexpr uint8_t kSAI2                 =  57;
constexpr uint8_t kSAI3_RX              =  58;
constexpr uint8_t kSAI3_TX              =  59;
constexpr uint8_t kSPDIF                =  60;
constexpr uint8_t kPMU_EVENT            =  61;  // Brown-out event on either the 1.1, 2.5 or 3.0 regulators.
// constexpr uint8_t kReserved2            =  62;
constexpr uint8_t kTEMP_LOW_HIGH        =  63;
constexpr uint8_t kTEMP_PANIC           =  64;
constexpr uint8_t kUSB_PHY1             =  65;
constexpr uint8_t kUSB_PHY2             =  66;
constexpr uint8_t kADC1                 =  67;
constexpr uint8_t kADC2                 =  68;
constexpr uint8_t kDCDC                 =  69;
constexpr uint8_t kSOFTWARE             =  70;  // Teensy 4 Audio Library, Reserved in reference manual
constexpr uint8_t kGPIO10_Combined_0_31 =  71;  // Reserved in reference manual
constexpr uint8_t kGPIO1_INT0           =  72;
constexpr uint8_t kGPIO1_INT1           =  73;
constexpr uint8_t kGPIO1_INT2           =  74;
constexpr uint8_t kGPIO1_INT3           =  75;
constexpr uint8_t kGPIO1_INT4           =  76;
constexpr uint8_t kGPIO1_INT5           =  77;
constexpr uint8_t kGPIO1_INT6           =  78;
constexpr uint8_t kGPIO1_INT7           =  79;
constexpr uint8_t kGPIO1_Combined_0_15  =  80;
constexpr uint8_t kGPIO1_Combined_16_31 =  81;
constexpr uint8_t kGPIO2_Combined_0_15  =  82;
constexpr uint8_t kGPIO2_Combined_16_31 =  83;
constexpr uint8_t kGPIO3_Combined_0_15  =  84;
constexpr uint8_t kGPIO3_Combined_16_31 =  85;
constexpr uint8_t kGPIO4_Combined_0_15  =  86;
constexpr uint8_t kGPIO4_Combined_16_31 =  87;
constexpr uint8_t kGPIO5_Combined_0_15  =  88;
constexpr uint8_t kGPIO5_Combined_16_31 =  89;
constexpr uint8_t kFLEXIO1              =  90;
constexpr uint8_t kFLEXIO2              =  91;
constexpr uint8_t kWDOG1                =  92;
constexpr uint8_t kRTWDOG               =  93;  // WDOG3
constexpr uint8_t kEWM                  =  94;
constexpr uint8_t kCCM_1                =  95;
constexpr uint8_t kCCM_2                =  96;
constexpr uint8_t kGPC                  =  97;
constexpr uint8_t kSRC                  =  98;
// constexpr uint8_t kReserved3            =  99;
constexpr uint8_t kGPT1                 = 100;
constexpr uint8_t kGPT2                 = 101;
constexpr uint8_t kPWM1_0               = 102;  // FLEXPWM1_0
constexpr uint8_t kPWM1_1               = 103;  // FLEXPWM1_1
constexpr uint8_t kPWM1_2               = 104;  // FLEXPWM1_2
constexpr uint8_t kPWM1_3               = 105;  // FLEXPWM1_3
constexpr uint8_t kPWM1_FAULT           = 106;  // Fault or reload error - FLEXPWM1_FAULT
constexpr uint8_t kFLEXSPI2             = 107;
constexpr uint8_t kFLEXSPI              = 108;
constexpr uint8_t kSEMC                 = 109;
constexpr uint8_t kUSDHC1               = 110;
constexpr uint8_t kUSDHC2               = 111;
constexpr uint8_t kUSB_OTG2             = 112;  // Reserved in reference manual
constexpr uint8_t kUSB_OTG1             = 113;  // Reserved in reference manual
constexpr uint8_t kENET                 = 114;
constexpr uint8_t kENET_1588_Timer      = 115;
constexpr uint8_t kXBAR1_IRQ_0_1        = 116;
constexpr uint8_t kXBAR1_IRQ_2_3        = 117;
constexpr uint8_t kADC_ETC_IRQ0         = 118;
constexpr uint8_t kADC_ETC_IRQ1         = 119;
constexpr uint8_t kADC_ETC_IRQ2         = 120;
constexpr uint8_t kADC_ETC_ERROR_IRQ    = 121;
constexpr uint8_t kPIT                  = 122;  // Timers 0-3
constexpr uint8_t kACMP1                = 123;
constexpr uint8_t kACMP2                = 124;
constexpr uint8_t kACMP3                = 125;
constexpr uint8_t kACMP4                = 126;
// constexpr uint8_t kReserved4            = 127;
// constexpr uint8_t kReserved5            = 128;
constexpr uint8_t kENC1                 = 129;  // QDC1
constexpr uint8_t kENC2                 = 130;  // QDC2
constexpr uint8_t kENC3                 = 131;  // QDC3
constexpr uint8_t kENC4                 = 132;  // QDC4
constexpr uint8_t kTMR1                 = 133;  // QTIMER1
constexpr uint8_t kTMR2                 = 134;  // QTIMER2
constexpr uint8_t kTMR3                 = 135;  // QTIMER3
constexpr uint8_t kTMR4                 = 136;  // QTIMER4
constexpr uint8_t kPWM2_0               = 137;  // FLEXPWM2_0
constexpr uint8_t kPWM2_1               = 138;  // FLEXPWM2_1
constexpr uint8_t kPWM2_2               = 139;  // FLEXPWM2_2
constexpr uint8_t kPWM2_3               = 140;  // FLEXPWM2_3
constexpr uint8_t kPWM2_FAULT           = 141;  // fault or reload error - FLEXPWM2_FAULT
constexpr uint8_t kPWM3_0               = 142;  // FLEXPWM3_0
constexpr uint8_t kPWM3_1               = 143;  // FLEXPWM3_1
constexpr uint8_t kPWM3_2               = 144;  // FLEXPWM3_2
constexpr uint8_t kPWM3_3               = 145;  // FLEXPWM3_3
constexpr uint8_t kPWM3_FAULT           = 146;  // fault or reload error - FLEXPWM3_FAULT
constexpr uint8_t kPWM4_0               = 147;  // FLEXPWM4_0
constexpr uint8_t kPWM4_1               = 148;  // FLEXPWM4_1
constexpr uint8_t kPWM4_2               = 149;  // FLEXPWM4_2
constexpr uint8_t kPWM4_3               = 150;  // FLEXPWM4_3
constexpr uint8_t kPWM4_FAULT           = 151;  // fault or reload error - FLEXPWM4_FAULT
constexpr uint8_t kENET2                = 152;
constexpr uint8_t kENET2_1588_Timer     = 153;
constexpr uint8_t kCAN3                 = 154;  // FLEXCAN3 (CANFD)
// constexpr uint8_t kReserved6            = 155;
constexpr uint8_t kFLEXIO3              = 156;
constexpr uint8_t kGPIO6_7_8_9          = 157;
// constexpr uint8_t kSJC_DEBUG            = 158;  // Reserved in reference manual
// constexpr uint8_t kNMI_WAKEUP           = 159;  // Reserved in reference manual

}  // namespace IRQ

}  // namespace NVIC

}  // namespace imxrt1060
}  // namespace hardware
}  // namespace qindesign
