// SPDX-FileCopyrightText: (c) 2026 Shawn Silverman <shawn@pobox.com>
// SPDX-License-Identifier: AGPL-3.0-or-later

// CCM_ANALOG.h defines all the CCM_ANALOG registers.
// This file is part of the imxrt1060-regs library.

#pragma once

#include <cstddef>
#include <cstdint>

#include "qnethernet/hardware/regs/regs.h"

namespace qindesign {
namespace hardware {
namespace imxrt1060 {

// Structure type to access the Clock Controller Module analog part
// (CCM_ANALOG) registers.
//
// Comments are from BSD-3-licensed NXP SDK.
//
// See:
// * https://github.com/nxp-mcuxpresso/mcux-devices-rt/blob/main/RT1060/periph/PERI_CCM_ANALOG.h
// * https://github.com/nxp-mcuxpresso/legacy-mcux-sdk/blob/main/devices/MIMXRT1062/MIMXRT1062.h
struct CCM_ANALOG_Layout {
  volatile uint32_t PLL_ARM;                           /**< Analog ARM PLL control Register, offset: 0x0 */
  volatile uint32_t PLL_ARM_SET;                       /**< Analog ARM PLL control Register, offset: 0x4 */
  volatile uint32_t PLL_ARM_CLR;                       /**< Analog ARM PLL control Register, offset: 0x8 */
  volatile uint32_t PLL_ARM_TOG;                       /**< Analog ARM PLL control Register, offset: 0xC */
  volatile uint32_t PLL_USB1;                          /**< Analog USB1 480MHz PLL Control Register, offset: 0x10 */
  volatile uint32_t PLL_USB1_SET;                      /**< Analog USB1 480MHz PLL Control Register, offset: 0x14 */
  volatile uint32_t PLL_USB1_CLR;                      /**< Analog USB1 480MHz PLL Control Register, offset: 0x18 */
  volatile uint32_t PLL_USB1_TOG;                      /**< Analog USB1 480MHz PLL Control Register, offset: 0x1C */
  volatile uint32_t PLL_USB2;                          /**< Analog USB2 480MHz PLL Control Register, offset: 0x20 */
  volatile uint32_t PLL_USB2_SET;                      /**< Analog USB2 480MHz PLL Control Register, offset: 0x24 */
  volatile uint32_t PLL_USB2_CLR;                      /**< Analog USB2 480MHz PLL Control Register, offset: 0x28 */
  volatile uint32_t PLL_USB2_TOG;                      /**< Analog USB2 480MHz PLL Control Register, offset: 0x2C */
  volatile uint32_t PLL_SYS;                           /**< Analog System PLL Control Register, offset: 0x30 */
  volatile uint32_t PLL_SYS_SET;                       /**< Analog System PLL Control Register, offset: 0x34 */
  volatile uint32_t PLL_SYS_CLR;                       /**< Analog System PLL Control Register, offset: 0x38 */
  volatile uint32_t PLL_SYS_TOG;                       /**< Analog System PLL Control Register, offset: 0x3C */
  volatile uint32_t PLL_SYS_SS;                        /**< 528MHz System PLL Spread Spectrum Register, offset: 0x40 */
  uint32_t HARDWARE_REGS_LAYOUT_MEMBER_RESERVED[3];
  volatile uint32_t PLL_SYS_NUM;                       /**< Numerator of 528MHz System PLL Fractional Loop Divider Register, offset: 0x50 */
  uint32_t HARDWARE_REGS_LAYOUT_MEMBER_RESERVED[3];
  volatile uint32_t PLL_SYS_DENOM;                     /**< Denominator of 528MHz System PLL Fractional Loop Divider Register, offset: 0x60 */
  uint32_t HARDWARE_REGS_LAYOUT_MEMBER_RESERVED[3];
  volatile uint32_t PLL_AUDIO;                         /**< Analog Audio PLL control Register, offset: 0x70 */
  volatile uint32_t PLL_AUDIO_SET;                     /**< Analog Audio PLL control Register, offset: 0x74 */
  volatile uint32_t PLL_AUDIO_CLR;                     /**< Analog Audio PLL control Register, offset: 0x78 */
  volatile uint32_t PLL_AUDIO_TOG;                     /**< Analog Audio PLL control Register, offset: 0x7C */
  volatile uint32_t PLL_AUDIO_NUM;                     /**< Numerator of Audio PLL Fractional Loop Divider Register, offset: 0x80 */
  uint32_t HARDWARE_REGS_LAYOUT_MEMBER_RESERVED[3];
  volatile uint32_t PLL_AUDIO_DENOM;                   /**< Denominator of Audio PLL Fractional Loop Divider Register, offset: 0x90 */
  uint32_t HARDWARE_REGS_LAYOUT_MEMBER_RESERVED[3];
  volatile uint32_t PLL_VIDEO;                         /**< Analog Video PLL control Register, offset: 0xA0 */
  volatile uint32_t PLL_VIDEO_SET;                     /**< Analog Video PLL control Register, offset: 0xA4 */
  volatile uint32_t PLL_VIDEO_CLR;                     /**< Analog Video PLL control Register, offset: 0xA8 */
  volatile uint32_t PLL_VIDEO_TOG;                     /**< Analog Video PLL control Register, offset: 0xAC */
  volatile uint32_t PLL_VIDEO_NUM;                     /**< Numerator of Video PLL Fractional Loop Divider Register, offset: 0xB0 */
  uint32_t HARDWARE_REGS_LAYOUT_MEMBER_RESERVED[3];
  volatile uint32_t PLL_VIDEO_DENOM;                   /**< Denominator of Video PLL Fractional Loop Divider Register, offset: 0xC0 */
  uint32_t HARDWARE_REGS_LAYOUT_MEMBER_RESERVED[7];
  volatile uint32_t PLL_ENET;                          /**< Analog ENET PLL Control Register, offset: 0xE0 */
  volatile uint32_t PLL_ENET_SET;                      /**< Analog ENET PLL Control Register, offset: 0xE4 */
  volatile uint32_t PLL_ENET_CLR;                      /**< Analog ENET PLL Control Register, offset: 0xE8 */
  volatile uint32_t PLL_ENET_TOG;                      /**< Analog ENET PLL Control Register, offset: 0xEC */
  volatile uint32_t PFD_480;                           /**< 480MHz Clock (PLL3) Phase Fractional Divider Control Register, offset: 0xF0 */
  volatile uint32_t PFD_480_SET;                       /**< 480MHz Clock (PLL3) Phase Fractional Divider Control Register, offset: 0xF4 */
  volatile uint32_t PFD_480_CLR;                       /**< 480MHz Clock (PLL3) Phase Fractional Divider Control Register, offset: 0xF8 */
  volatile uint32_t PFD_480_TOG;                       /**< 480MHz Clock (PLL3) Phase Fractional Divider Control Register, offset: 0xFC */
  volatile uint32_t PFD_528;                           /**< 528MHz Clock (PLL2) Phase Fractional Divider Control Register, offset: 0x100 */
  volatile uint32_t PFD_528_SET;                       /**< 528MHz Clock (PLL2) Phase Fractional Divider Control Register, offset: 0x104 */
  volatile uint32_t PFD_528_CLR;                       /**< 528MHz Clock (PLL2) Phase Fractional Divider Control Register, offset: 0x108 */
  volatile uint32_t PFD_528_TOG;                       /**< 528MHz Clock (PLL2) Phase Fractional Divider Control Register, offset: 0x10C */
  uint32_t HARDWARE_REGS_LAYOUT_MEMBER_RESERVED[16];
  volatile uint32_t MISC0;                             /**< Miscellaneous Register 0, offset: 0x150 */
  volatile uint32_t MISC0_SET;                         /**< Miscellaneous Register 0, offset: 0x154 */
  volatile uint32_t MISC0_CLR;                         /**< Miscellaneous Register 0, offset: 0x158 */
  volatile uint32_t MISC0_TOG;                         /**< Miscellaneous Register 0, offset: 0x15C */
  volatile uint32_t MISC1;                             /**< Miscellaneous Register 1, offset: 0x160 */
  volatile uint32_t MISC1_SET;                         /**< Miscellaneous Register 1, offset: 0x164 */
  volatile uint32_t MISC1_CLR;                         /**< Miscellaneous Register 1, offset: 0x168 */
  volatile uint32_t MISC1_TOG;                         /**< Miscellaneous Register 1, offset: 0x16C */
  volatile uint32_t MISC2;                             /**< Miscellaneous Register 2, offset: 0x170 */
  volatile uint32_t MISC2_SET;                         /**< Miscellaneous Register 2, offset: 0x174 */
  volatile uint32_t MISC2_CLR;                         /**< Miscellaneous Register 2, offset: 0x178 */
  volatile uint32_t MISC2_TOG;                         /**< Miscellaneous Register 2, offset: 0x17C */
};

constexpr size_t    kCCM_ANALOG_size = 0x180;
constexpr uintptr_t kCCM_ANALOG_base = 0x400D'8000;

namespace CCM_ANALOG {

constexpr regs::RegGroup<CCM_ANALOG_Layout, kCCM_ANALOG_size, kCCM_ANALOG_base>
    group;

template <auto Member, size_t Bits, unsigned int Shift,
          auto AssignMask = regs::shiftedMask32<Bits, Shift>()>
using CCM_ANALOG_Reg = regs::Reg32<kCCM_ANALOG_base, CCM_ANALOG_Layout, Member,
                                   0, Bits, Shift, AssignMask>;

// Analog ARM PLL control Register
namespace PLL_ARM {
constexpr CCM_ANALOG_Reg<regs::constify(&CCM_ANALOG_Layout::PLL_ARM), 1, 31> LOCK;
    // 1 - PLL is currently locked.
    // 0 - PLL is not currently locked.
constexpr CCM_ANALOG_Reg<&CCM_ANALOG_Layout::PLL_ARM, 1, 19> PLL_SEL;               // Reserved
constexpr CCM_ANALOG_Reg<&CCM_ANALOG_Layout::PLL_ARM, 1, 16> BYPASS;                // Bypass the PLL.
constexpr CCM_ANALOG_Reg<&CCM_ANALOG_Layout::PLL_ARM, 2, 14> BYPASS_CLK_SRC;        // Determines the bypass source
    // 0b00..Select the 24MHz oscillator as source.
    // 0b01..Select the CLK1_N / CLK1_P as source.
    // 0b10..Reserved1
    // 0b11..Reserved2
constexpr CCM_ANALOG_Reg<&CCM_ANALOG_Layout::PLL_ARM, 1, 13> ENABLE;                // Enable the clock output.
constexpr CCM_ANALOG_Reg<&CCM_ANALOG_Layout::PLL_ARM, 1, 12> POWERDOWN;             // Powers down the PLL.
constexpr CCM_ANALOG_Reg<&CCM_ANALOG_Layout::PLL_ARM, 7,  0> DIV_SELECT;            // This field controls the PLL loop divider
}  // namespace PLL_ARM

// Analog ARM PLL control Register
namespace PLL_ARM_SET {
constexpr CCM_ANALOG_Reg<regs::constify(&CCM_ANALOG_Layout::PLL_ARM_SET), 1, 31> LOCK;
    // 1 - PLL is currently locked.
    // 0 - PLL is not currently locked.
constexpr CCM_ANALOG_Reg<&CCM_ANALOG_Layout::PLL_ARM_SET, 1, 19, 0x0> PLL_SEL;          // Reserved
constexpr CCM_ANALOG_Reg<&CCM_ANALOG_Layout::PLL_ARM_SET, 1, 16, 0x0> BYPASS;           // Bypass the PLL.
constexpr CCM_ANALOG_Reg<&CCM_ANALOG_Layout::PLL_ARM_SET, 2, 14, 0x0> BYPASS_CLK_SRC;   // Determines the bypass source
constexpr CCM_ANALOG_Reg<&CCM_ANALOG_Layout::PLL_ARM_SET, 1, 13, 0x0> ENABLE;           // Enable the clock output.
constexpr CCM_ANALOG_Reg<&CCM_ANALOG_Layout::PLL_ARM_SET, 1, 12, 0x0> POWERDOWN;        // Powers down the PLL.
constexpr CCM_ANALOG_Reg<&CCM_ANALOG_Layout::PLL_ARM_SET, 7,  0, 0x0> DIV_SELECT;       // This field controls the PLL loop divider
}  // namespace PLL_ARM_SET

// Analog ARM PLL control Register
namespace PLL_ARM_CLR {
constexpr CCM_ANALOG_Reg<regs::constify(&CCM_ANALOG_Layout::PLL_ARM_CLR), 1, 31> LOCK;
    // 1 - PLL is currently locked.
    // 0 - PLL is not currently locked.
constexpr CCM_ANALOG_Reg<&CCM_ANALOG_Layout::PLL_ARM_CLR, 1, 19, 0x0> PLL_SEL;          // Reserved
constexpr CCM_ANALOG_Reg<&CCM_ANALOG_Layout::PLL_ARM_CLR, 1, 16, 0x0> BYPASS;           // Bypass the PLL.
constexpr CCM_ANALOG_Reg<&CCM_ANALOG_Layout::PLL_ARM_CLR, 2, 14, 0x0> BYPASS_CLK_SRC;   // Determines the bypass source
constexpr CCM_ANALOG_Reg<&CCM_ANALOG_Layout::PLL_ARM_CLR, 1, 13, 0x0> ENABLE;           // Enable the clock output.
constexpr CCM_ANALOG_Reg<&CCM_ANALOG_Layout::PLL_ARM_CLR, 1, 12, 0x0> POWERDOWN;        // Powers down the PLL.
constexpr CCM_ANALOG_Reg<&CCM_ANALOG_Layout::PLL_ARM_CLR, 7,  0, 0x0> DIV_SELECT;       // This field controls the PLL loop divider
}  // namespace PLL_ARM_CLR

// Analog ARM PLL control Register
namespace PLL_ARM_TOG {
constexpr CCM_ANALOG_Reg<regs::constify(&CCM_ANALOG_Layout::PLL_ARM_TOG), 1, 31> LOCK;
    // 1 - PLL is currently locked.
    // 0 - PLL is not currently locked.
constexpr CCM_ANALOG_Reg<&CCM_ANALOG_Layout::PLL_ARM_TOG, 1, 19, 0x0> PLL_SEL;          // Reserved
constexpr CCM_ANALOG_Reg<&CCM_ANALOG_Layout::PLL_ARM_TOG, 1, 16, 0x0> BYPASS;           // Bypass the PLL.
constexpr CCM_ANALOG_Reg<&CCM_ANALOG_Layout::PLL_ARM_TOG, 2, 14, 0x0> BYPASS_CLK_SRC;   // Determines the bypass source
constexpr CCM_ANALOG_Reg<&CCM_ANALOG_Layout::PLL_ARM_TOG, 1, 13, 0x0> ENABLE;           // Enable the clock output.
constexpr CCM_ANALOG_Reg<&CCM_ANALOG_Layout::PLL_ARM_TOG, 1, 12, 0x0> POWERDOWN;        // Powers down the PLL.
constexpr CCM_ANALOG_Reg<&CCM_ANALOG_Layout::PLL_ARM_TOG, 7,  0, 0x0> DIV_SELECT;       // This field controls the PLL loop divider
}  // namespace PLL_ARM_TOG

// Analog USB1 480MHz PLL Control Register
namespace PLL_USB1 {
constexpr CCM_ANALOG_Reg<regs::constify(&CCM_ANALOG_Layout::PLL_USB1), 1, 31> LOCK;
    // 1 - PLL is currently locked.
    // 0 - PLL is not currently locked.
constexpr CCM_ANALOG_Reg<&CCM_ANALOG_Layout::PLL_USB1, 1, 16> BYPASS;                // Bypass the PLL.
constexpr CCM_ANALOG_Reg<&CCM_ANALOG_Layout::PLL_USB1, 2, 14> BYPASS_CLK_SRC;        // Determines the bypass source.
    // 0b00..Select the 24MHz oscillator as source.
    // 0b01..Select the CLK1_N / CLK1_P as source.
constexpr CCM_ANALOG_Reg<&CCM_ANALOG_Layout::PLL_USB1, 1, 13> ENABLE;                // Enable the PLL clock output.
constexpr CCM_ANALOG_Reg<&CCM_ANALOG_Layout::PLL_USB1, 1, 12> POWER;                 // Powers up the PLL.
    // This bit will be set automatically when USBPHY0 remote wakeup event happens.
constexpr CCM_ANALOG_Reg<&CCM_ANALOG_Layout::PLL_USB1, 1,  6> EN_USB_CLKS;           // Powers the 9-phase PLL outputs for USBPHYn
    // 0b0..PLL outputs for USBPHYn off.
    // 0b1..PLL outputs for USBPHYn on.
constexpr CCM_ANALOG_Reg<&CCM_ANALOG_Layout::PLL_USB1, 1,  1> DIV_SELECT;            // This field controls the PLL loop divider.
    // 0 - Fout=Fref*20;
    // 1 - Fout=Fref*22.
}  // namespace PLL_USB1

// Analog USB1 480MHz PLL Control Register
namespace PLL_USB1_SET {
constexpr CCM_ANALOG_Reg<regs::constify(&CCM_ANALOG_Layout::PLL_USB1_SET), 1, 31> LOCK;
    // 1 - PLL is currently locked.
    // 0 - PLL is not currently locked.
constexpr CCM_ANALOG_Reg<&CCM_ANALOG_Layout::PLL_USB1_SET, 1, 16, 0x0> BYPASS;           // Bypass the PLL.
constexpr CCM_ANALOG_Reg<&CCM_ANALOG_Layout::PLL_USB1_SET, 2, 14, 0x0> BYPASS_CLK_SRC;   // Determines the bypass source.
constexpr CCM_ANALOG_Reg<&CCM_ANALOG_Layout::PLL_USB1_SET, 1, 13, 0x0> ENABLE;           // Enable the PLL clock output.
constexpr CCM_ANALOG_Reg<&CCM_ANALOG_Layout::PLL_USB1_SET, 1, 12, 0x0> POWER;            // Powers up the PLL.
    // This bit will be set automatically when USBPHY0 remote wakeup event happens.
constexpr CCM_ANALOG_Reg<&CCM_ANALOG_Layout::PLL_USB1_SET, 1,  6, 0x0> EN_USB_CLKS;      // Powers the 9-phase PLL outputs for USBPHYn
constexpr CCM_ANALOG_Reg<&CCM_ANALOG_Layout::PLL_USB1_SET, 1,  1, 0x0> DIV_SELECT;       // This field controls the PLL loop divider.
    // 0 - Fout=Fref*20;
    // 1 - Fout=Fref*22.
}  // namespace PLL_USB1_SET

// Analog USB1 480MHz PLL Control Register
namespace PLL_USB1_CLR {
constexpr CCM_ANALOG_Reg<regs::constify(&CCM_ANALOG_Layout::PLL_USB1_CLR), 1, 31> LOCK;
    // 1 - PLL is currently locked.
    // 0 - PLL is not currently locked.
constexpr CCM_ANALOG_Reg<&CCM_ANALOG_Layout::PLL_USB1_CLR, 1, 16, 0x0> BYPASS;           // Bypass the PLL.
constexpr CCM_ANALOG_Reg<&CCM_ANALOG_Layout::PLL_USB1_CLR, 2, 14, 0x0> BYPASS_CLK_SRC;   // Determines the bypass source.
constexpr CCM_ANALOG_Reg<&CCM_ANALOG_Layout::PLL_USB1_CLR, 1, 13, 0x0> ENABLE;           // Enable the PLL clock output.
constexpr CCM_ANALOG_Reg<&CCM_ANALOG_Layout::PLL_USB1_CLR, 1, 12, 0x0> POWER;            // Powers up the PLL.
    // This bit will be set automatically when USBPHY0 remote wakeup event happens.
constexpr CCM_ANALOG_Reg<&CCM_ANALOG_Layout::PLL_USB1_CLR, 1,  6, 0x0> EN_USB_CLKS;      // Powers the 9-phase PLL outputs for USBPHYn
constexpr CCM_ANALOG_Reg<&CCM_ANALOG_Layout::PLL_USB1_CLR, 1,  1, 0x0> DIV_SELECT;       // This field controls the PLL loop divider.
    // 0 - Fout=Fref*20;
    // 1 - Fout=Fref*22.
}  // namespace PLL_USB1_CLR

// Analog USB1 480MHz PLL Control Register
namespace PLL_USB1_TOG {
constexpr CCM_ANALOG_Reg<regs::constify(&CCM_ANALOG_Layout::PLL_USB1_TOG), 1, 31> LOCK;
    // 1 - PLL is currently locked.
    // 0 - PLL is not currently locked.
constexpr CCM_ANALOG_Reg<&CCM_ANALOG_Layout::PLL_USB1_TOG, 1, 16, 0x0> BYPASS;           // Bypass the PLL.
constexpr CCM_ANALOG_Reg<&CCM_ANALOG_Layout::PLL_USB1_TOG, 2, 14, 0x0> BYPASS_CLK_SRC;   // Determines the bypass source.
constexpr CCM_ANALOG_Reg<&CCM_ANALOG_Layout::PLL_USB1_TOG, 1, 13, 0x0> ENABLE;           // Enable the PLL clock output.
constexpr CCM_ANALOG_Reg<&CCM_ANALOG_Layout::PLL_USB1_TOG, 1, 12, 0x0> POWER;            // Powers up the PLL.
    // This bit will be set automatically when USBPHY0 remote wakeup event happens.
constexpr CCM_ANALOG_Reg<&CCM_ANALOG_Layout::PLL_USB1_TOG, 1,  6, 0x0> EN_USB_CLKS;      // Powers the 9-phase PLL outputs for USBPHYn
constexpr CCM_ANALOG_Reg<&CCM_ANALOG_Layout::PLL_USB1_TOG, 1,  1, 0x0> DIV_SELECT;       // This field controls the PLL loop divider.
    // 0 - Fout=Fref*20;
    // 1 - Fout=Fref*22.
}  // namespace PLL_USB1_TOG

// Analog USB2 480MHz PLL Control Register
namespace PLL_USB2 {
constexpr CCM_ANALOG_Reg<regs::constify(&CCM_ANALOG_Layout::PLL_USB2), 1, 31> LOCK;
    // 1 - PLL is currently locked.
    // 0 - PLL is not currently locked.
constexpr CCM_ANALOG_Reg<&CCM_ANALOG_Layout::PLL_USB2, 1, 16> BYPASS;                // Bypass the PLL.
constexpr CCM_ANALOG_Reg<&CCM_ANALOG_Layout::PLL_USB2, 2, 14> BYPASS_CLK_SRC;        // Determines the bypass source.
    // 0b00..Select the 24MHz oscillator as source.
    // 0b01..Select the CLK1_N / CLK1_P as source.
    // 0b10..Reserved1
    // 0b11..Reserved2
constexpr CCM_ANALOG_Reg<&CCM_ANALOG_Layout::PLL_USB2, 1, 13> ENABLE;                // Enable the PLL clock output.
constexpr CCM_ANALOG_Reg<&CCM_ANALOG_Layout::PLL_USB2, 1, 12> POWER;                 // Powers up the PLL.
    // This bit will be set automatically when USBPHY1 remote wakeup event happens.
constexpr CCM_ANALOG_Reg<&CCM_ANALOG_Layout::PLL_USB2, 1,  6> EN_USB_CLKS;           // 0: 8-phase PLL outputs for USBPHY1 are powered down
constexpr CCM_ANALOG_Reg<&CCM_ANALOG_Layout::PLL_USB2, 1,  1> DIV_SELECT;            // This field controls the PLL loop divider.
    // 0 - Fout=Fref*20;
    // 1 - Fout=Fref*22.
}  // namespace PLL_USB2

// Analog USB2 480MHz PLL Control Register
namespace PLL_USB2_SET {
constexpr CCM_ANALOG_Reg<regs::constify(&CCM_ANALOG_Layout::PLL_USB2_SET), 1, 31> LOCK;
    // 1 - PLL is currently locked.
    // 0 - PLL is not currently locked.
constexpr CCM_ANALOG_Reg<&CCM_ANALOG_Layout::PLL_USB2_SET, 1, 16, 0x0> BYPASS;           // Bypass the PLL.
constexpr CCM_ANALOG_Reg<&CCM_ANALOG_Layout::PLL_USB2_SET, 2, 14, 0x0> BYPASS_CLK_SRC;   // Determines the bypass source.
constexpr CCM_ANALOG_Reg<&CCM_ANALOG_Layout::PLL_USB2_SET, 1, 13, 0x0> ENABLE;           // Enable the PLL clock output.
constexpr CCM_ANALOG_Reg<&CCM_ANALOG_Layout::PLL_USB2_SET, 1, 12, 0x0> POWER;            // Powers up the PLL.
    // This bit will be set automatically when USBPHY1 remote wakeup event happens.
constexpr CCM_ANALOG_Reg<&CCM_ANALOG_Layout::PLL_USB2_SET, 1,  6, 0x0> EN_USB_CLKS;      // 0: 8-phase PLL outputs for USBPHY1 are powered down
constexpr CCM_ANALOG_Reg<&CCM_ANALOG_Layout::PLL_USB2_SET, 1,  1, 0x0> DIV_SELECT;       // This field controls the PLL loop divider.
    // 0 - Fout=Fref*20;
    // 1 - Fout=Fref*22.
}  // namespace PLL_USB2_SET

// Analog USB2 480MHz PLL Control Register
namespace PLL_USB2_CLR {
constexpr CCM_ANALOG_Reg<regs::constify(&CCM_ANALOG_Layout::PLL_USB2_CLR), 1, 31> LOCK;
    // 1 - PLL is currently locked.
    // 0 - PLL is not currently locked.
constexpr CCM_ANALOG_Reg<&CCM_ANALOG_Layout::PLL_USB2_CLR, 1, 16, 0x0> BYPASS;           // Bypass the PLL.
constexpr CCM_ANALOG_Reg<&CCM_ANALOG_Layout::PLL_USB2_CLR, 2, 14, 0x0> BYPASS_CLK_SRC;   // Determines the bypass source.
constexpr CCM_ANALOG_Reg<&CCM_ANALOG_Layout::PLL_USB2_CLR, 1, 13, 0x0> ENABLE;           // Enable the PLL clock output.
constexpr CCM_ANALOG_Reg<&CCM_ANALOG_Layout::PLL_USB2_CLR, 1, 12, 0x0> POWER;            // Powers up the PLL.
    // This bit will be set automatically when USBPHY1 remote wakeup event happens.
constexpr CCM_ANALOG_Reg<&CCM_ANALOG_Layout::PLL_USB2_CLR, 1,  6, 0x0> EN_USB_CLKS;      // 0: 8-phase PLL outputs for USBPHY1 are powered down
constexpr CCM_ANALOG_Reg<&CCM_ANALOG_Layout::PLL_USB2_CLR, 1,  1, 0x0> DIV_SELECT;       // This field controls the PLL loop divider.
    // 0 - Fout=Fref*20;
    // 1 - Fout=Fref*22.
}  // namespace PLL_USB2_CLR

// Analog USB2 480MHz PLL Control Register
namespace PLL_USB2_TOG {
constexpr CCM_ANALOG_Reg<regs::constify(&CCM_ANALOG_Layout::PLL_USB2_TOG), 1, 31> LOCK;
    // 1 - PLL is currently locked.
    // 0 - PLL is not currently locked.
constexpr CCM_ANALOG_Reg<&CCM_ANALOG_Layout::PLL_USB2_TOG, 1, 16, 0x0> BYPASS;           // Bypass the PLL.
constexpr CCM_ANALOG_Reg<&CCM_ANALOG_Layout::PLL_USB2_TOG, 2, 14, 0x0> BYPASS_CLK_SRC;   // Determines the bypass source.
constexpr CCM_ANALOG_Reg<&CCM_ANALOG_Layout::PLL_USB2_TOG, 1, 13, 0x0> ENABLE;           // Enable the PLL clock output.
constexpr CCM_ANALOG_Reg<&CCM_ANALOG_Layout::PLL_USB2_TOG, 1, 12, 0x0> POWER;            // Powers up the PLL.
    // This bit will be set automatically when USBPHY1 remote wakeup event happens.
constexpr CCM_ANALOG_Reg<&CCM_ANALOG_Layout::PLL_USB2_TOG, 1,  6, 0x0> EN_USB_CLKS;      // 0: 8-phase PLL outputs for USBPHY1 are powered down
constexpr CCM_ANALOG_Reg<&CCM_ANALOG_Layout::PLL_USB2_TOG, 1,  1, 0x0> DIV_SELECT;       // This field controls the PLL loop divider.
    // 0 - Fout=Fref*20;
    // 1 - Fout=Fref*22.
}  // namespace PLL_USB2_TOG

// Analog System PLL Control Register
namespace PLL_SYS {
constexpr CCM_ANALOG_Reg<regs::constify(&CCM_ANALOG_Layout::PLL_SYS), 1, 31> LOCK;
    // 1 - PLL is currently locked;
    // 0 - PLL is not currently locked.
constexpr CCM_ANALOG_Reg<&CCM_ANALOG_Layout::PLL_SYS, 1, 16> BYPASS;                // Bypass the PLL.
constexpr CCM_ANALOG_Reg<&CCM_ANALOG_Layout::PLL_SYS, 2, 14> BYPASS_CLK_SRC;        // Determines the bypass source.
    // 0b00..Select the 24MHz oscillator as source.
    // 0b01..Select the CLK1_N / CLK1_P as source.
constexpr CCM_ANALOG_Reg<&CCM_ANALOG_Layout::PLL_SYS, 1, 13> ENABLE;                // Enable PLL output
constexpr CCM_ANALOG_Reg<&CCM_ANALOG_Layout::PLL_SYS, 1, 12> POWERDOWN;             // Powers down the PLL.
constexpr CCM_ANALOG_Reg<&CCM_ANALOG_Layout::PLL_SYS, 1,  0> DIV_SELECT;            // This field controls the PLL loop divider.
    // 0 - Fout=Fref*20;
    // 1 - Fout=Fref*22.
}  // namespace PLL_SYS

// Analog System PLL Control Register
namespace PLL_SYS_SET {
constexpr CCM_ANALOG_Reg<regs::constify(&CCM_ANALOG_Layout::PLL_SYS_SET), 1, 31> LOCK;
    // 1 - PLL is currently locked;
    // 0 - PLL is not currently locked.
constexpr CCM_ANALOG_Reg<&CCM_ANALOG_Layout::PLL_SYS_SET, 1, 16, 0x0> BYPASS;           // Bypass the PLL.
constexpr CCM_ANALOG_Reg<&CCM_ANALOG_Layout::PLL_SYS_SET, 2, 14, 0x0> BYPASS_CLK_SRC;   // Determines the bypass source.
constexpr CCM_ANALOG_Reg<&CCM_ANALOG_Layout::PLL_SYS_SET, 1, 13, 0x0> ENABLE;           // Enable PLL output
constexpr CCM_ANALOG_Reg<&CCM_ANALOG_Layout::PLL_SYS_SET, 1, 12, 0x0> POWERDOWN;        // Powers down the PLL.
constexpr CCM_ANALOG_Reg<&CCM_ANALOG_Layout::PLL_SYS_SET, 1,  0, 0x0> DIV_SELECT;       // This field controls the PLL loop divider.
    // 0 - Fout=Fref*20;
    // 1 - Fout=Fref*22.
}  // namespace PLL_SYS_SET

// Analog System PLL Control Register
namespace PLL_SYS_CLR {
constexpr CCM_ANALOG_Reg<regs::constify(&CCM_ANALOG_Layout::PLL_SYS_CLR), 1, 31> LOCK;
    // 1 - PLL is currently locked;
    // 0 - PLL is not currently locked.
constexpr CCM_ANALOG_Reg<&CCM_ANALOG_Layout::PLL_SYS_CLR, 1, 16, 0x0> BYPASS;           // Bypass the PLL.
constexpr CCM_ANALOG_Reg<&CCM_ANALOG_Layout::PLL_SYS_CLR, 2, 14, 0x0> BYPASS_CLK_SRC;   // Determines the bypass source.
constexpr CCM_ANALOG_Reg<&CCM_ANALOG_Layout::PLL_SYS_CLR, 1, 13, 0x0> ENABLE;           // Enable PLL output
constexpr CCM_ANALOG_Reg<&CCM_ANALOG_Layout::PLL_SYS_CLR, 1, 12, 0x0> POWERDOWN;        // Powers down the PLL.
constexpr CCM_ANALOG_Reg<&CCM_ANALOG_Layout::PLL_SYS_CLR, 1,  0, 0x0> DIV_SELECT;       // This field controls the PLL loop divider.
    // 0 - Fout=Fref*20;
    // 1 - Fout=Fref*22.
}  // namespace PLL_SYS_CLR

// Analog System PLL Control Register
namespace PLL_SYS_TOG {
constexpr CCM_ANALOG_Reg<regs::constify(&CCM_ANALOG_Layout::PLL_SYS_TOG), 1, 31> LOCK;
    // 1 - PLL is currently locked;
    // 0 - PLL is not currently locked.
constexpr CCM_ANALOG_Reg<&CCM_ANALOG_Layout::PLL_SYS_TOG, 1, 16, 0x0> BYPASS;           // Bypass the PLL.
constexpr CCM_ANALOG_Reg<&CCM_ANALOG_Layout::PLL_SYS_TOG, 2, 14, 0x0> BYPASS_CLK_SRC;   // Determines the bypass source.
constexpr CCM_ANALOG_Reg<&CCM_ANALOG_Layout::PLL_SYS_TOG, 1, 13, 0x0> ENABLE;           // Enable PLL output
constexpr CCM_ANALOG_Reg<&CCM_ANALOG_Layout::PLL_SYS_TOG, 1, 12, 0x0> POWERDOWN;        // Powers down the PLL.
constexpr CCM_ANALOG_Reg<&CCM_ANALOG_Layout::PLL_SYS_TOG, 1,  0, 0x0> DIV_SELECT;       // This field controls the PLL loop divider.
    // 0 - Fout=Fref*20;
    // 1 - Fout=Fref*22.
}  // namespace PLL_SYS_TOG

// 528MHz System PLL Spread Spectrum Register
namespace PLL_SYS_SS {
constexpr CCM_ANALOG_Reg<&CCM_ANALOG_Layout::PLL_SYS_SS, 16, 16> STOP;    // Frequency change = stop/CCM_ANALOG_PLL_SYS_DENOM[B]*24MHz.
constexpr CCM_ANALOG_Reg<&CCM_ANALOG_Layout::PLL_SYS_SS,  1, 15> ENABLE;  // Enable bit
    // 0b0..Spread spectrum modulation disabled
    // 0b1..Spread spectrum modulation enabled
constexpr CCM_ANALOG_Reg<&CCM_ANALOG_Layout::PLL_SYS_SS, 15,  0> STEP;    // Frequency change step = step/CCM_ANALOG_PLL_SYS_DENOM[B]*24MHz.
}  // namespace PLL_SYS_SS

// Numerator of 528MHz System PLL Fractional Loop Divider Register
namespace PLL_SYS_NUM {
constexpr CCM_ANALOG_Reg<&CCM_ANALOG_Layout::PLL_SYS_NUM, 30, 0> A;  // 30 bit numerator (A) of fractional loop divider (signed integer).
}  // namespace PLL_SYS_NUM

// Denominator of 528MHz System PLL Fractional Loop Divider Register
namespace PLL_SYS_DENOM {
constexpr CCM_ANALOG_Reg<&CCM_ANALOG_Layout::PLL_SYS_DENOM, 30, 0> B;  // 30 bit denominator (B) of fractional loop divider (unsigned integer).
}  // namespace PLL_SYS_DENOM

// Analog Audio PLL control Register
namespace PLL_AUDIO {
constexpr CCM_ANALOG_Reg<regs::constify(&CCM_ANALOG_Layout::PLL_AUDIO), 1, 31> LOCK;
    // 1 - PLL is currently locked.
    // 0 - PLL is not currently locked.
constexpr CCM_ANALOG_Reg<&CCM_ANALOG_Layout::PLL_AUDIO, 2, 19> POST_DIV_SELECT;       // These bits implement a divider after the PLL, but before the enable and bypass mux.
    // 0b00..Divide by 4.
    // 0b01..Divide by 2.
    // 0b10..Divide by 1.
    // 0b11..Reserved
constexpr CCM_ANALOG_Reg<&CCM_ANALOG_Layout::PLL_AUDIO, 1, 16> BYPASS;                // Bypass the PLL.
constexpr CCM_ANALOG_Reg<&CCM_ANALOG_Layout::PLL_AUDIO, 2, 14> BYPASS_CLK_SRC;        // Determines the bypass source.
    // 0b00..Select the 24MHz oscillator as source.
    // 0b01..Select the CLK1_N / CLK1_P as source.
    // 0b10..Reserved1
    // 0b11..Reserved2
constexpr CCM_ANALOG_Reg<&CCM_ANALOG_Layout::PLL_AUDIO, 1, 13> ENABLE;                // Enable PLL output
constexpr CCM_ANALOG_Reg<&CCM_ANALOG_Layout::PLL_AUDIO, 1, 12> POWERDOWN;             // Powers down the PLL.
constexpr CCM_ANALOG_Reg<&CCM_ANALOG_Layout::PLL_AUDIO, 7,  0> DIV_SELECT;            // This field controls the PLL loop divider.
    // Valid range for DIV_SELECT divider value: 27~54.
}  // namespace PLL_AUDIO

// Analog Audio PLL control Register
namespace PLL_AUDIO_SET {
constexpr CCM_ANALOG_Reg<regs::constify(&CCM_ANALOG_Layout::PLL_AUDIO_SET), 1, 31> LOCK;
    // 1 - PLL is currently locked.
    // 0 - PLL is not currently locked.
constexpr CCM_ANALOG_Reg<&CCM_ANALOG_Layout::PLL_AUDIO_SET, 2, 19, 0x0> POST_DIV_SELECT;  // These bits implement a divider after the PLL, but before the enable and bypass mux.
constexpr CCM_ANALOG_Reg<&CCM_ANALOG_Layout::PLL_AUDIO_SET, 1, 16, 0x0> BYPASS;           // Bypass the PLL.
constexpr CCM_ANALOG_Reg<&CCM_ANALOG_Layout::PLL_AUDIO_SET, 2, 14, 0x0> BYPASS_CLK_SRC;   // Determines the bypass source.
constexpr CCM_ANALOG_Reg<&CCM_ANALOG_Layout::PLL_AUDIO_SET, 1, 13, 0x0> ENABLE;           // Enable PLL output
constexpr CCM_ANALOG_Reg<&CCM_ANALOG_Layout::PLL_AUDIO_SET, 1, 12, 0x0> POWERDOWN;        // Powers down the PLL.
constexpr CCM_ANALOG_Reg<&CCM_ANALOG_Layout::PLL_AUDIO_SET, 7,  0, 0x0> DIV_SELECT;       // This field controls the PLL loop divider.
    // Valid range for DIV_SELECT divider value: 27~54.
}  // namespace PLL_AUDIO_SET

// Analog Audio PLL control Register
namespace PLL_AUDIO_CLR {
constexpr CCM_ANALOG_Reg<regs::constify(&CCM_ANALOG_Layout::PLL_AUDIO_CLR), 1, 31> LOCK;
    // 1 - PLL is currently locked.
    // 0 - PLL is not currently locked.
constexpr CCM_ANALOG_Reg<&CCM_ANALOG_Layout::PLL_AUDIO_CLR, 2, 19, 0x0> POST_DIV_SELECT;  // These bits implement a divider after the PLL, but before the enable and bypass mux.
constexpr CCM_ANALOG_Reg<&CCM_ANALOG_Layout::PLL_AUDIO_CLR, 1, 16, 0x0> BYPASS;           // Bypass the PLL.
constexpr CCM_ANALOG_Reg<&CCM_ANALOG_Layout::PLL_AUDIO_CLR, 2, 14, 0x0> BYPASS_CLK_SRC;   // Determines the bypass source.
constexpr CCM_ANALOG_Reg<&CCM_ANALOG_Layout::PLL_AUDIO_CLR, 1, 13, 0x0> ENABLE;           // Enable PLL output
constexpr CCM_ANALOG_Reg<&CCM_ANALOG_Layout::PLL_AUDIO_CLR, 1, 12, 0x0> POWERDOWN;        // Powers down the PLL.
constexpr CCM_ANALOG_Reg<&CCM_ANALOG_Layout::PLL_AUDIO_CLR, 7,  0, 0x0> DIV_SELECT;       // This field controls the PLL loop divider.
    // Valid range for DIV_SELECT divider value: 27~54.
}  // namespace PLL_AUDIO_CLR

// Analog Audio PLL control Register
namespace PLL_AUDIO_TOG {
constexpr CCM_ANALOG_Reg<regs::constify(&CCM_ANALOG_Layout::PLL_AUDIO_TOG), 1, 31> LOCK;
    // 1 - PLL is currently locked.
    // 0 - PLL is not currently locked.
constexpr CCM_ANALOG_Reg<&CCM_ANALOG_Layout::PLL_AUDIO_TOG, 2, 19, 0x0> POST_DIV_SELECT;  // These bits implement a divider after the PLL, but before the enable and bypass mux.
constexpr CCM_ANALOG_Reg<&CCM_ANALOG_Layout::PLL_AUDIO_TOG, 1, 16, 0x0> BYPASS;           // Bypass the PLL.
constexpr CCM_ANALOG_Reg<&CCM_ANALOG_Layout::PLL_AUDIO_TOG, 2, 14, 0x0> BYPASS_CLK_SRC;   // Determines the bypass source.
constexpr CCM_ANALOG_Reg<&CCM_ANALOG_Layout::PLL_AUDIO_TOG, 1, 13, 0x0> ENABLE;           // Enable PLL output
constexpr CCM_ANALOG_Reg<&CCM_ANALOG_Layout::PLL_AUDIO_TOG, 1, 12, 0x0> POWERDOWN;        // Powers down the PLL.
constexpr CCM_ANALOG_Reg<&CCM_ANALOG_Layout::PLL_AUDIO_TOG, 7,  0, 0x0> DIV_SELECT;       // This field controls the PLL loop divider.
    // Valid range for DIV_SELECT divider value: 27~54.
}  // namespace PLL_AUDIO_TOG

// Numerator of Audio PLL Fractional Loop Divider Register
namespace PLL_AUDIO_NUM {
constexpr CCM_ANALOG_Reg<&CCM_ANALOG_Layout::PLL_AUDIO_NUM, 30, 0> A;  // 30 bit numerator of fractional loop divider.
}  // namespace PLL_AUDIO_NUM

// Denominator of Audio PLL Fractional Loop Divider Register
namespace PLL_AUDIO_DENOM {
constexpr CCM_ANALOG_Reg<&CCM_ANALOG_Layout::PLL_AUDIO_DENOM, 30, 0> B;  // 30 bit denominator of fractional loop divider.
}  // namespace PLL_AUDIO_DENOM

// Analog Video PLL control Register
namespace PLL_VIDEO {
constexpr CCM_ANALOG_Reg<regs::constify(&CCM_ANALOG_Layout::PLL_VIDEO), 1, 31> LOCK;
    // 1 - PLL is currently locked;
    // 0 - PLL is not currently locked.
constexpr CCM_ANALOG_Reg<&CCM_ANALOG_Layout::PLL_VIDEO, 2, 19> POST_DIV_SELECT;       // These bits implement a divider after the PLL, but before the enable and bypass mux.
    // 0b00..Divide by 4.
    // 0b01..Divide by 2.
    // 0b10..Divide by 1.
    // 0b11..Reserved
constexpr CCM_ANALOG_Reg<&CCM_ANALOG_Layout::PLL_VIDEO, 1, 16> BYPASS;                // Bypass the PLL.
constexpr CCM_ANALOG_Reg<&CCM_ANALOG_Layout::PLL_VIDEO, 2, 14> BYPASS_CLK_SRC;        // Determines the bypass source.
    // 0b00..Select the 24MHz oscillator as source.
    // 0b01..Select the CLK1_N / CLK1_P as source.
    // 0b10..Reserved1
    // 0b11..Reserved2
constexpr CCM_ANALOG_Reg<&CCM_ANALOG_Layout::PLL_VIDEO, 1, 13> ENABLE;                // Enable PLL output
constexpr CCM_ANALOG_Reg<&CCM_ANALOG_Layout::PLL_VIDEO, 1, 12> POWERDOWN;             // Powers down the PLL.
constexpr CCM_ANALOG_Reg<&CCM_ANALOG_Layout::PLL_VIDEO, 7,  0> DIV_SELECT;            // This field controls the PLL loop divider.
    // Valid range for DIV_SELECT divider value: 27~54.
}  // namespace PLL_VIDEO

// Analog Video PLL control Register
namespace PLL_VIDEO_SET {
constexpr CCM_ANALOG_Reg<regs::constify(&CCM_ANALOG_Layout::PLL_VIDEO_SET), 1, 31> LOCK;
    // 1 - PLL is currently locked;
    // 0 - PLL is not currently locked.
constexpr CCM_ANALOG_Reg<&CCM_ANALOG_Layout::PLL_VIDEO_SET, 2, 19, 0x0> POST_DIV_SELECT;  // These bits implement a divider after the PLL, but before the enable and bypass mux.
constexpr CCM_ANALOG_Reg<&CCM_ANALOG_Layout::PLL_VIDEO_SET, 1, 16, 0x0> BYPASS;           // Bypass the PLL.
constexpr CCM_ANALOG_Reg<&CCM_ANALOG_Layout::PLL_VIDEO_SET, 2, 14, 0x0> BYPASS_CLK_SRC;   // Determines the bypass source.
constexpr CCM_ANALOG_Reg<&CCM_ANALOG_Layout::PLL_VIDEO_SET, 1, 13, 0x0> ENABLE;           // Enable PLL output
constexpr CCM_ANALOG_Reg<&CCM_ANALOG_Layout::PLL_VIDEO_SET, 1, 12, 0x0> POWERDOWN;        // Powers down the PLL.
constexpr CCM_ANALOG_Reg<&CCM_ANALOG_Layout::PLL_VIDEO_SET, 7,  0, 0x0> DIV_SELECT;       // This field controls the PLL loop divider.
    // Valid range for DIV_SELECT divider value: 27~54.
}  // namespace PLL_VIDEO_SET

// Analog Video PLL control Register
namespace PLL_VIDEO_CLR {
constexpr CCM_ANALOG_Reg<regs::constify(&CCM_ANALOG_Layout::PLL_VIDEO_CLR), 1, 31> LOCK;
    // 1 - PLL is currently locked;
    // 0 - PLL is not currently locked.
constexpr CCM_ANALOG_Reg<&CCM_ANALOG_Layout::PLL_VIDEO_CLR, 2, 19, 0x0> POST_DIV_SELECT;  // These bits implement a divider after the PLL, but before the enable and bypass mux.
constexpr CCM_ANALOG_Reg<&CCM_ANALOG_Layout::PLL_VIDEO_CLR, 1, 16, 0x0> BYPASS;           // Bypass the PLL.
constexpr CCM_ANALOG_Reg<&CCM_ANALOG_Layout::PLL_VIDEO_CLR, 2, 14, 0x0> BYPASS_CLK_SRC;   // Determines the bypass source.
constexpr CCM_ANALOG_Reg<&CCM_ANALOG_Layout::PLL_VIDEO_CLR, 1, 13, 0x0> ENABLE;           // Enable PLL output
constexpr CCM_ANALOG_Reg<&CCM_ANALOG_Layout::PLL_VIDEO_CLR, 1, 12, 0x0> POWERDOWN;        // Powers down the PLL.
constexpr CCM_ANALOG_Reg<&CCM_ANALOG_Layout::PLL_VIDEO_CLR, 7,  0, 0x0> DIV_SELECT;       // This field controls the PLL loop divider.
    // Valid range for DIV_SELECT divider value: 27~54.
}  // namespace PLL_VIDEO_CLR

// Analog Video PLL control Register
namespace PLL_VIDEO_TOG {
constexpr CCM_ANALOG_Reg<regs::constify(&CCM_ANALOG_Layout::PLL_VIDEO_TOG), 1, 31> LOCK;
    // 1 - PLL is currently locked;
    // 0 - PLL is not currently locked.
constexpr CCM_ANALOG_Reg<&CCM_ANALOG_Layout::PLL_VIDEO_TOG, 2, 19, 0x0> POST_DIV_SELECT;  // These bits implement a divider after the PLL, but before the enable and bypass mux.
constexpr CCM_ANALOG_Reg<&CCM_ANALOG_Layout::PLL_VIDEO_TOG, 1, 16, 0x0> BYPASS;           // Bypass the PLL.
constexpr CCM_ANALOG_Reg<&CCM_ANALOG_Layout::PLL_VIDEO_TOG, 2, 14, 0x0> BYPASS_CLK_SRC;   // Determines the bypass source.
constexpr CCM_ANALOG_Reg<&CCM_ANALOG_Layout::PLL_VIDEO_TOG, 1, 13, 0x0> ENABLE;           // Enable PLL output
constexpr CCM_ANALOG_Reg<&CCM_ANALOG_Layout::PLL_VIDEO_TOG, 1, 12, 0x0> POWERDOWN;        // Powers down the PLL.
constexpr CCM_ANALOG_Reg<&CCM_ANALOG_Layout::PLL_VIDEO_TOG, 7,  0, 0x0> DIV_SELECT;       // This field controls the PLL loop divider.
    // Valid range for DIV_SELECT divider value: 27~54.
}  // namespace PLL_VIDEO_TOG

// Numerator of Video PLL Fractional Loop Divider Register
namespace PLL_VIDEO_NUM {
constexpr CCM_ANALOG_Reg<&CCM_ANALOG_Layout::PLL_VIDEO_NUM, 30, 0> A;  // 30 bit numerator of fractional loop divider (signed number)
}  // namespace PLL_VIDEO_NUM

// Denominator of Video PLL Fractional Loop Divider Register
namespace PLL_VIDEO_DENOM {
constexpr CCM_ANALOG_Reg<&CCM_ANALOG_Layout::PLL_VIDEO_DENOM, 30, 0> B;  // 30 bit denominator of fractional loop divider.
}  // namespace PLL_VIDEO_DENOM

// Analog ENET PLL Control Register
namespace PLL_ENET {
constexpr CCM_ANALOG_Reg<regs::constify(&CCM_ANALOG_Layout::PLL_ENET), 1, 31> LOCK;
    // 1 - PLL is currently locked;
    // 0 - PLL is not currently locked.
constexpr CCM_ANALOG_Reg<&CCM_ANALOG_Layout::PLL_ENET, 1, 21> ENET_25M_REF_EN;       // Enable the PLL providing ENET 25 MHz reference clock
constexpr CCM_ANALOG_Reg<&CCM_ANALOG_Layout::PLL_ENET, 1, 20> ENET2_REF_EN;          // Enable the PLL providing the ENET2 reference clock
constexpr CCM_ANALOG_Reg<&CCM_ANALOG_Layout::PLL_ENET, 1, 16> BYPASS;                // Bypass the PLL.
constexpr CCM_ANALOG_Reg<&CCM_ANALOG_Layout::PLL_ENET, 2, 14> BYPASS_CLK_SRC;        // Determines the bypass source.
    // 0b00..Select the 24MHz oscillator as source.
    // 0b01..Select the CLK1_N / CLK1_P as source.
    // 0b10..Reserved1
    // 0b11..Reserved2
constexpr CCM_ANALOG_Reg<&CCM_ANALOG_Layout::PLL_ENET, 1, 13> ENABLE;                // Enable the PLL providing the ENET reference clock.
constexpr CCM_ANALOG_Reg<&CCM_ANALOG_Layout::PLL_ENET, 1, 12> POWERDOWN;             // Powers down the PLL.
constexpr CCM_ANALOG_Reg<&CCM_ANALOG_Layout::PLL_ENET, 2,  2> ENET2_DIV_SELECT;      // Controls the frequency of the ENET2 reference clock.
    // 0b00..25MHz
    // 0b01..50MHz
    // 0b10..100MHz (not 50% duty cycle)
    // 0b11..125MHz
constexpr CCM_ANALOG_Reg<&CCM_ANALOG_Layout::PLL_ENET, 2, 0> DIV_SELECT;             // Controls the frequency of the ethernet reference clock

// Analog ENET PLL Control Register values
constexpr uint32_t kENET2_DIV_SELECT_25MHz  = 0;
constexpr uint32_t kENET2_DIV_SELECT_50MHz  = 1;
constexpr uint32_t kENET2_DIV_SELECT_100MHz = 2;
constexpr uint32_t kENET2_DIV_SELECT_125MHz = 3;
}  // namespace PLL_ENET

// Analog ENET PLL Control Register
namespace PLL_ENET_SET {
constexpr CCM_ANALOG_Reg<regs::constify(&CCM_ANALOG_Layout::PLL_ENET_SET), 1, 31> LOCK;
    // 1 - PLL is currently locked;
    // 0 - PLL is not currently locked.
constexpr CCM_ANALOG_Reg<&CCM_ANALOG_Layout::PLL_ENET_SET, 1, 21, 0x0> ENET_25M_REF_EN;   // Enable the PLL providing ENET 25 MHz reference clock
constexpr CCM_ANALOG_Reg<&CCM_ANALOG_Layout::PLL_ENET_SET, 1, 20, 0x0> ENET2_REF_EN;      // Enable the PLL providing the ENET2 reference clock
constexpr CCM_ANALOG_Reg<&CCM_ANALOG_Layout::PLL_ENET_SET, 1, 16, 0x0> BYPASS;            // Bypass the PLL.
constexpr CCM_ANALOG_Reg<&CCM_ANALOG_Layout::PLL_ENET_SET, 2, 14, 0x0> BYPASS_CLK_SRC;    // Determines the bypass source.
constexpr CCM_ANALOG_Reg<&CCM_ANALOG_Layout::PLL_ENET_SET, 1, 13, 0x0> ENABLE;            // Enable the PLL providing the ENET reference clock.
constexpr CCM_ANALOG_Reg<&CCM_ANALOG_Layout::PLL_ENET_SET, 1, 12, 0x0> POWERDOWN;         // Powers down the PLL.
constexpr CCM_ANALOG_Reg<&CCM_ANALOG_Layout::PLL_ENET_SET, 2,  2, 0x0> ENET2_DIV_SELECT;  // Controls the frequency of the ENET2 reference clock.
constexpr CCM_ANALOG_Reg<&CCM_ANALOG_Layout::PLL_ENET_SET, 2,  0, 0x0> DIV_SELECT;        // Controls the frequency of the ethernet reference clock
}  // namespace PLL_ENET_SET

// Analog ENET PLL Control Register
namespace PLL_ENET_CLR {
constexpr CCM_ANALOG_Reg<regs::constify(&CCM_ANALOG_Layout::PLL_ENET_CLR), 1, 31> LOCK;
    // 1 - PLL is currently locked;
    // 0 - PLL is not currently locked.
constexpr CCM_ANALOG_Reg<&CCM_ANALOG_Layout::PLL_ENET_CLR, 1, 21, 0x0> ENET_25M_REF_EN;   // Enable the PLL providing ENET 25 MHz reference clock
constexpr CCM_ANALOG_Reg<&CCM_ANALOG_Layout::PLL_ENET_CLR, 1, 20, 0x0> ENET2_REF_EN;      // Enable the PLL providing the ENET2 reference clock
constexpr CCM_ANALOG_Reg<&CCM_ANALOG_Layout::PLL_ENET_CLR, 1, 16, 0x0> BYPASS;            // Bypass the PLL.
constexpr CCM_ANALOG_Reg<&CCM_ANALOG_Layout::PLL_ENET_CLR, 2, 14, 0x0> BYPASS_CLK_SRC;    // Determines the bypass source.
constexpr CCM_ANALOG_Reg<&CCM_ANALOG_Layout::PLL_ENET_CLR, 1, 13, 0x0> ENABLE;            // Enable the PLL providing the ENET reference clock.
constexpr CCM_ANALOG_Reg<&CCM_ANALOG_Layout::PLL_ENET_CLR, 1, 12, 0x0> POWERDOWN;         // Powers down the PLL.
constexpr CCM_ANALOG_Reg<&CCM_ANALOG_Layout::PLL_ENET_CLR, 2,  2, 0x0> ENET2_DIV_SELECT;  // Controls the frequency of the ENET2 reference clock.
constexpr CCM_ANALOG_Reg<&CCM_ANALOG_Layout::PLL_ENET_CLR, 2,  0, 0x0> DIV_SELECT;        // Controls the frequency of the ethernet reference clock
}  // namespace PLL_ENET_CLR

// Analog ENET PLL Control Register
namespace PLL_ENET_TOG {
constexpr CCM_ANALOG_Reg<regs::constify(&CCM_ANALOG_Layout::PLL_ENET_TOG), 1, 31> LOCK;
    // 1 - PLL is currently locked;
    // 0 - PLL is not currently locked.
constexpr CCM_ANALOG_Reg<&CCM_ANALOG_Layout::PLL_ENET_TOG, 1, 21, 0x0> ENET_25M_REF_EN;   // Enable the PLL providing ENET 25 MHz reference clock
constexpr CCM_ANALOG_Reg<&CCM_ANALOG_Layout::PLL_ENET_TOG, 1, 20, 0x0> ENET2_REF_EN;      // Enable the PLL providing the ENET2 reference clock
constexpr CCM_ANALOG_Reg<&CCM_ANALOG_Layout::PLL_ENET_TOG, 1, 16, 0x0> BYPASS;            // Bypass the PLL.
constexpr CCM_ANALOG_Reg<&CCM_ANALOG_Layout::PLL_ENET_TOG, 2, 14, 0x0> BYPASS_CLK_SRC;    // Determines the bypass source.
constexpr CCM_ANALOG_Reg<&CCM_ANALOG_Layout::PLL_ENET_TOG, 1, 13, 0x0> ENABLE;            // Enable the PLL providing the ENET reference clock.
constexpr CCM_ANALOG_Reg<&CCM_ANALOG_Layout::PLL_ENET_TOG, 1, 12, 0x0> POWERDOWN;         // Powers down the PLL.
constexpr CCM_ANALOG_Reg<&CCM_ANALOG_Layout::PLL_ENET_TOG, 2,  2, 0x0> ENET2_DIV_SELECT;  // Controls the frequency of the ENET2 reference clock.
constexpr CCM_ANALOG_Reg<&CCM_ANALOG_Layout::PLL_ENET_TOG, 2,  0, 0x0> DIV_SELECT;        // Controls the frequency of the ethernet reference clock
}  // namespace PLL_ENET_TOG

// 480MHz Clock (PLL3) Phase Fractional Divider Control Register
namespace PFD_480 {
constexpr CCM_ANALOG_Reg<&CCM_ANALOG_Layout::PFD_480, 1, 31> PFD3_CLKGATE;                 // IO Clock Gate
constexpr CCM_ANALOG_Reg<regs::constify(&CCM_ANALOG_Layout::PFD_480), 1, 30> PFD3_STABLE;  // This read-only bitfield is for DIAGNOSTIC PURPOSES ONLY since the fractional divider should become stable quickly enough that this field will never need to be used by either device driver or application code
constexpr CCM_ANALOG_Reg<&CCM_ANALOG_Layout::PFD_480, 6, 24> PFD3_FRAC;                    // This field controls the fractional divide value
constexpr CCM_ANALOG_Reg<&CCM_ANALOG_Layout::PFD_480, 1, 23> PFD2_CLKGATE;                 // IO Clock Gate
constexpr CCM_ANALOG_Reg<regs::constify(&CCM_ANALOG_Layout::PFD_480), 1, 22> PFD2_STABLE;  // This read-only bitfield is for DIAGNOSTIC PURPOSES ONLY since the fractional divider should become stable quickly enough that this field will never need to be used by either device driver or application code
constexpr CCM_ANALOG_Reg<&CCM_ANALOG_Layout::PFD_480, 6, 16> PFD2_FRAC;                    // This field controls the fractional divide value
constexpr CCM_ANALOG_Reg<&CCM_ANALOG_Layout::PFD_480, 1, 15> PFD1_CLKGATE;                 // IO Clock Gate
constexpr CCM_ANALOG_Reg<regs::constify(&CCM_ANALOG_Layout::PFD_480), 1, 14> PFD1_STABLE;  // This read-only bitfield is for DIAGNOSTIC PURPOSES ONLY since the fractional divider should become stable quickly enough that this field will never need to be used by either device driver or application code
constexpr CCM_ANALOG_Reg<&CCM_ANALOG_Layout::PFD_480, 6,  8> PFD1_FRAC;                    // This field controls the fractional divide value
constexpr CCM_ANALOG_Reg<&CCM_ANALOG_Layout::PFD_480, 1,  7> PFD0_CLKGATE;                 // If set to 1, the IO fractional divider clock (reference ref_pfd0) is off (power savings)
constexpr CCM_ANALOG_Reg<regs::constify(&CCM_ANALOG_Layout::PFD_480), 1,  6> PFD0_STABLE;  // This read-only bitfield is for DIAGNOSTIC PURPOSES ONLY since the fractional divider should become stable quickly enough that this field will never need to be used by either device driver or application code
constexpr CCM_ANALOG_Reg<&CCM_ANALOG_Layout::PFD_480, 6,  0> PFD0_FRAC;                    // This field controls the fractional divide value
}  // namespace PFD_480

// 480MHz Clock (PLL3) Phase Fractional Divider Control Register
namespace PFD_480_SET {
constexpr CCM_ANALOG_Reg<&CCM_ANALOG_Layout::PFD_480_SET, 1, 31, 0x0> PFD3_CLKGATE;            // IO Clock Gate
constexpr CCM_ANALOG_Reg<regs::constify(&CCM_ANALOG_Layout::PFD_480_SET), 1, 30> PFD3_STABLE;  // This read-only bitfield is for DIAGNOSTIC PURPOSES ONLY since the fractional divider should become stable quickly enough that this field will never need to be used by either device driver or application code
constexpr CCM_ANALOG_Reg<&CCM_ANALOG_Layout::PFD_480_SET, 6, 24, 0x0> PFD3_FRAC;               // This field controls the fractional divide value
constexpr CCM_ANALOG_Reg<&CCM_ANALOG_Layout::PFD_480_SET, 1, 23, 0x0> PFD2_CLKGATE;            // IO Clock Gate
constexpr CCM_ANALOG_Reg<regs::constify(&CCM_ANALOG_Layout::PFD_480_SET), 1, 22> PFD2_STABLE;  // This read-only bitfield is for DIAGNOSTIC PURPOSES ONLY since the fractional divider should become stable quickly enough that this field will never need to be used by either device driver or application code
constexpr CCM_ANALOG_Reg<&CCM_ANALOG_Layout::PFD_480_SET, 6, 16, 0x0> PFD2_FRAC;               // This field controls the fractional divide value
constexpr CCM_ANALOG_Reg<&CCM_ANALOG_Layout::PFD_480_SET, 1, 15, 0x0> PFD1_CLKGATE;            // IO Clock Gate
constexpr CCM_ANALOG_Reg<regs::constify(&CCM_ANALOG_Layout::PFD_480_SET), 1, 14> PFD1_STABLE;  // This read-only bitfield is for DIAGNOSTIC PURPOSES ONLY since the fractional divider should become stable quickly enough that this field will never need to be used by either device driver or application code
constexpr CCM_ANALOG_Reg<&CCM_ANALOG_Layout::PFD_480_SET, 6,  8, 0x0> PFD1_FRAC;               // This field controls the fractional divide value
constexpr CCM_ANALOG_Reg<&CCM_ANALOG_Layout::PFD_480_SET, 1,  7, 0x0> PFD0_CLKGATE;            // If set to 1, the IO fractional divider clock (reference ref_pfd0) is off (power savings)
constexpr CCM_ANALOG_Reg<regs::constify(&CCM_ANALOG_Layout::PFD_480_SET), 1,  6> PFD0_STABLE;  // This read-only bitfield is for DIAGNOSTIC PURPOSES ONLY since the fractional divider should become stable quickly enough that this field will never need to be used by either device driver or application code
constexpr CCM_ANALOG_Reg<&CCM_ANALOG_Layout::PFD_480_SET, 6,  0, 0x0> PFD0_FRAC;               // This field controls the fractional divide value
}  // namespace PFD_480_SET

// 480MHz Clock (PLL3) Phase Fractional Divider Control Register
namespace PFD_480_CLR {
constexpr CCM_ANALOG_Reg<&CCM_ANALOG_Layout::PFD_480_CLR, 1, 31, 0x0> PFD3_CLKGATE;            // IO Clock Gate
constexpr CCM_ANALOG_Reg<regs::constify(&CCM_ANALOG_Layout::PFD_480_CLR), 1, 30> PFD3_STABLE;  // This read-only bitfield is for DIAGNOSTIC PURPOSES ONLY since the fractional divider should become stable quickly enough that this field will never need to be used by either device driver or application code
constexpr CCM_ANALOG_Reg<&CCM_ANALOG_Layout::PFD_480_CLR, 6, 24, 0x0> PFD3_FRAC;               // This field controls the fractional divide value
constexpr CCM_ANALOG_Reg<&CCM_ANALOG_Layout::PFD_480_CLR, 1, 23, 0x0> PFD2_CLKGATE;            // IO Clock Gate
constexpr CCM_ANALOG_Reg<regs::constify(&CCM_ANALOG_Layout::PFD_480_CLR), 1, 22> PFD2_STABLE;  // This read-only bitfield is for DIAGNOSTIC PURPOSES ONLY since the fractional divider should become stable quickly enough that this field will never need to be used by either device driver or application code
constexpr CCM_ANALOG_Reg<&CCM_ANALOG_Layout::PFD_480_CLR, 6, 16, 0x0> PFD2_FRAC;               // This field controls the fractional divide value
constexpr CCM_ANALOG_Reg<&CCM_ANALOG_Layout::PFD_480_CLR, 1, 15, 0x0> PFD1_CLKGATE;            // IO Clock Gate
constexpr CCM_ANALOG_Reg<regs::constify(&CCM_ANALOG_Layout::PFD_480_CLR), 1, 14> PFD1_STABLE;  // This read-only bitfield is for DIAGNOSTIC PURPOSES ONLY since the fractional divider should become stable quickly enough that this field will never need to be used by either device driver or application code
constexpr CCM_ANALOG_Reg<&CCM_ANALOG_Layout::PFD_480_CLR, 6,  8, 0x0> PFD1_FRAC;               // This field controls the fractional divide value
constexpr CCM_ANALOG_Reg<&CCM_ANALOG_Layout::PFD_480_CLR, 1,  7, 0x0> PFD0_CLKGATE;            // If set to 1, the IO fractional divider clock (reference ref_pfd0) is off (power savings)
constexpr CCM_ANALOG_Reg<regs::constify(&CCM_ANALOG_Layout::PFD_480_CLR), 1,  6> PFD0_STABLE;  // This read-only bitfield is for DIAGNOSTIC PURPOSES ONLY since the fractional divider should become stable quickly enough that this field will never need to be used by either device driver or application code
constexpr CCM_ANALOG_Reg<&CCM_ANALOG_Layout::PFD_480_CLR, 6,  0, 0x0> PFD0_FRAC;               // This field controls the fractional divide value
}  // namespace PFD_480_CLR

// 480MHz Clock (PLL3) Phase Fractional Divider Control Register
namespace PFD_480_TOG {
constexpr CCM_ANALOG_Reg<&CCM_ANALOG_Layout::PFD_480_TOG, 1, 31, 0x0> PFD3_CLKGATE;            // IO Clock Gate
constexpr CCM_ANALOG_Reg<regs::constify(&CCM_ANALOG_Layout::PFD_480_TOG), 1, 30> PFD3_STABLE;  // This read-only bitfield is for DIAGNOSTIC PURPOSES ONLY since the fractional divider should become stable quickly enough that this field will never need to be used by either device driver or application code
constexpr CCM_ANALOG_Reg<&CCM_ANALOG_Layout::PFD_480_TOG, 6, 24, 0x0> PFD3_FRAC;               // This field controls the fractional divide value
constexpr CCM_ANALOG_Reg<&CCM_ANALOG_Layout::PFD_480_TOG, 1, 23, 0x0> PFD2_CLKGATE;            // IO Clock Gate
constexpr CCM_ANALOG_Reg<regs::constify(&CCM_ANALOG_Layout::PFD_480_TOG), 1, 22> PFD2_STABLE;  // This read-only bitfield is for DIAGNOSTIC PURPOSES ONLY since the fractional divider should become stable quickly enough that this field will never need to be used by either device driver or application code
constexpr CCM_ANALOG_Reg<&CCM_ANALOG_Layout::PFD_480_TOG, 6, 16, 0x0> PFD2_FRAC;               // This field controls the fractional divide value
constexpr CCM_ANALOG_Reg<&CCM_ANALOG_Layout::PFD_480_TOG, 1, 15, 0x0> PFD1_CLKGATE;            // IO Clock Gate
constexpr CCM_ANALOG_Reg<regs::constify(&CCM_ANALOG_Layout::PFD_480_TOG), 1, 14> PFD1_STABLE;  // This read-only bitfield is for DIAGNOSTIC PURPOSES ONLY since the fractional divider should become stable quickly enough that this field will never need to be used by either device driver or application code
constexpr CCM_ANALOG_Reg<&CCM_ANALOG_Layout::PFD_480_TOG, 6,  8, 0x0> PFD1_FRAC;               // This field controls the fractional divide value
constexpr CCM_ANALOG_Reg<&CCM_ANALOG_Layout::PFD_480_TOG, 1,  7, 0x0> PFD0_CLKGATE;            // If set to 1, the IO fractional divider clock (reference ref_pfd0) is off (power savings)
constexpr CCM_ANALOG_Reg<regs::constify(&CCM_ANALOG_Layout::PFD_480_TOG), 1,  6> PFD0_STABLE;  // This read-only bitfield is for DIAGNOSTIC PURPOSES ONLY since the fractional divider should become stable quickly enough that this field will never need to be used by either device driver or application code
constexpr CCM_ANALOG_Reg<&CCM_ANALOG_Layout::PFD_480_TOG, 6,  0, 0x0> PFD0_FRAC;               // This field controls the fractional divide value
}  // namespace PFD_480_TOG

// 528MHz Clock (PLL2) Phase Fractional Divider Control Register
namespace PFD_528 {
constexpr CCM_ANALOG_Reg<&CCM_ANALOG_Layout::PFD_528, 1, 31> PFD3_CLKGATE;                 // IO Clock Gate
constexpr CCM_ANALOG_Reg<regs::constify(&CCM_ANALOG_Layout::PFD_528), 1, 30> PFD3_STABLE;  // This read-only bitfield is for DIAGNOSTIC PURPOSES ONLY since the fractional divider should become stable quickly enough that this field will never need to be used by either device driver or application code
constexpr CCM_ANALOG_Reg<&CCM_ANALOG_Layout::PFD_528, 6, 24> PFD3_FRAC;                    // This field controls the fractional divide value
constexpr CCM_ANALOG_Reg<&CCM_ANALOG_Layout::PFD_528, 1, 23> PFD2_CLKGATE;                 // IO Clock Gate
constexpr CCM_ANALOG_Reg<regs::constify(&CCM_ANALOG_Layout::PFD_528), 1, 22> PFD2_STABLE;  // This read-only bitfield is for DIAGNOSTIC PURPOSES ONLY since the fractional divider should become stable quickly enough that this field will never need to be used by either device driver or application code
constexpr CCM_ANALOG_Reg<&CCM_ANALOG_Layout::PFD_528, 6, 16> PFD2_FRAC;                    // This field controls the fractional divide value
constexpr CCM_ANALOG_Reg<&CCM_ANALOG_Layout::PFD_528, 1, 15> PFD1_CLKGATE;                 // IO Clock Gate
constexpr CCM_ANALOG_Reg<regs::constify(&CCM_ANALOG_Layout::PFD_528), 1, 14> PFD1_STABLE;  // This read-only bitfield is for DIAGNOSTIC PURPOSES ONLY since the fractional divider should become stable quickly enough that this field will never need to be used by either device driver or application code
constexpr CCM_ANALOG_Reg<&CCM_ANALOG_Layout::PFD_528, 6,  8> PFD1_FRAC;                    // This field controls the fractional divide value
constexpr CCM_ANALOG_Reg<&CCM_ANALOG_Layout::PFD_528, 1,  7> PFD0_CLKGATE;                 // If set to 1, the IO fractional divider clock (reference ref_pfd0) is off (power savings)
constexpr CCM_ANALOG_Reg<regs::constify(&CCM_ANALOG_Layout::PFD_528), 1,  6> PFD0_STABLE;  // This read-only bitfield is for DIAGNOSTIC PURPOSES ONLY since the fractional divider should become stable quickly enough that this field will never need to be used by either device driver or application code
constexpr CCM_ANALOG_Reg<&CCM_ANALOG_Layout::PFD_528, 6,  0> PFD0_FRAC;                    // This field controls the fractional divide value
}  // namespace PFD_528

// 528MHz Clock (PLL2) Phase Fractional Divider Control Register
namespace PFD_528_SET {
constexpr CCM_ANALOG_Reg<&CCM_ANALOG_Layout::PFD_528_SET, 1, 31, 0x0> PFD3_CLKGATE;            // IO Clock Gate
constexpr CCM_ANALOG_Reg<regs::constify(&CCM_ANALOG_Layout::PFD_528_SET), 1, 30> PFD3_STABLE;  // This read-only bitfield is for DIAGNOSTIC PURPOSES ONLY since the fractional divider should become stable quickly enough that this field will never need to be used by either device driver or application code
constexpr CCM_ANALOG_Reg<&CCM_ANALOG_Layout::PFD_528_SET, 6, 24, 0x0> PFD3_FRAC;               // This field controls the fractional divide value
constexpr CCM_ANALOG_Reg<&CCM_ANALOG_Layout::PFD_528_SET, 1, 23, 0x0> PFD2_CLKGATE;            // IO Clock Gate
constexpr CCM_ANALOG_Reg<regs::constify(&CCM_ANALOG_Layout::PFD_528_SET), 1, 22> PFD2_STABLE;  // This read-only bitfield is for DIAGNOSTIC PURPOSES ONLY since the fractional divider should become stable quickly enough that this field will never need to be used by either device driver or application code
constexpr CCM_ANALOG_Reg<&CCM_ANALOG_Layout::PFD_528_SET, 6, 16, 0x0> PFD2_FRAC;               // This field controls the fractional divide value
constexpr CCM_ANALOG_Reg<&CCM_ANALOG_Layout::PFD_528_SET, 1, 15, 0x0> PFD1_CLKGATE;            // IO Clock Gate
constexpr CCM_ANALOG_Reg<regs::constify(&CCM_ANALOG_Layout::PFD_528_SET), 1, 14> PFD1_STABLE;  // This read-only bitfield is for DIAGNOSTIC PURPOSES ONLY since the fractional divider should become stable quickly enough that this field will never need to be used by either device driver or application code
constexpr CCM_ANALOG_Reg<&CCM_ANALOG_Layout::PFD_528_SET, 6,  8, 0x0> PFD1_FRAC;               // This field controls the fractional divide value
constexpr CCM_ANALOG_Reg<&CCM_ANALOG_Layout::PFD_528_SET, 1,  7, 0x0> PFD0_CLKGATE;            // If set to 1, the IO fractional divider clock (reference ref_pfd0) is off (power savings)
constexpr CCM_ANALOG_Reg<regs::constify(&CCM_ANALOG_Layout::PFD_528_SET), 1,  6> PFD0_STABLE;  // This read-only bitfield is for DIAGNOSTIC PURPOSES ONLY since the fractional divider should become stable quickly enough that this field will never need to be used by either device driver or application code
constexpr CCM_ANALOG_Reg<&CCM_ANALOG_Layout::PFD_528_SET, 6,  0, 0x0> PFD0_FRAC;               // This field controls the fractional divide value
}  // namespace PFD_528_SET

// 528MHz Clock (PLL2) Phase Fractional Divider Control Register
namespace PFD_528_CLR {
constexpr CCM_ANALOG_Reg<&CCM_ANALOG_Layout::PFD_528_CLR, 1, 31, 0x0> PFD3_CLKGATE;            // IO Clock Gate
constexpr CCM_ANALOG_Reg<regs::constify(&CCM_ANALOG_Layout::PFD_528_CLR), 1, 30> PFD3_STABLE;  // This read-only bitfield is for DIAGNOSTIC PURPOSES ONLY since the fractional divider should become stable quickly enough that this field will never need to be used by either device driver or application code
constexpr CCM_ANALOG_Reg<&CCM_ANALOG_Layout::PFD_528_CLR, 6, 24, 0x0> PFD3_FRAC;               // This field controls the fractional divide value
constexpr CCM_ANALOG_Reg<&CCM_ANALOG_Layout::PFD_528_CLR, 1, 23, 0x0> PFD2_CLKGATE;            // IO Clock Gate
constexpr CCM_ANALOG_Reg<regs::constify(&CCM_ANALOG_Layout::PFD_528_CLR), 1, 22> PFD2_STABLE;  // This read-only bitfield is for DIAGNOSTIC PURPOSES ONLY since the fractional divider should become stable quickly enough that this field will never need to be used by either device driver or application code
constexpr CCM_ANALOG_Reg<&CCM_ANALOG_Layout::PFD_528_CLR, 6, 16, 0x0> PFD2_FRAC;               // This field controls the fractional divide value
constexpr CCM_ANALOG_Reg<&CCM_ANALOG_Layout::PFD_528_CLR, 1, 15, 0x0> PFD1_CLKGATE;            // IO Clock Gate
constexpr CCM_ANALOG_Reg<regs::constify(&CCM_ANALOG_Layout::PFD_528_CLR), 1, 14> PFD1_STABLE;  // This read-only bitfield is for DIAGNOSTIC PURPOSES ONLY since the fractional divider should become stable quickly enough that this field will never need to be used by either device driver or application code
constexpr CCM_ANALOG_Reg<&CCM_ANALOG_Layout::PFD_528_CLR, 6,  8, 0x0> PFD1_FRAC;               // This field controls the fractional divide value
constexpr CCM_ANALOG_Reg<&CCM_ANALOG_Layout::PFD_528_CLR, 1,  7, 0x0> PFD0_CLKGATE;            // If set to 1, the IO fractional divider clock (reference ref_pfd0) is off (power savings)
constexpr CCM_ANALOG_Reg<regs::constify(&CCM_ANALOG_Layout::PFD_528_CLR), 1,  6> PFD0_STABLE;  // This read-only bitfield is for DIAGNOSTIC PURPOSES ONLY since the fractional divider should become stable quickly enough that this field will never need to be used by either device driver or application code
constexpr CCM_ANALOG_Reg<&CCM_ANALOG_Layout::PFD_528_CLR, 6,  0, 0x0> PFD0_FRAC;               // This field controls the fractional divide value
}  // namespace PFD_528_CLR

// 528MHz Clock (PLL2) Phase Fractional Divider Control Register
namespace PFD_528_TOG {
constexpr CCM_ANALOG_Reg<&CCM_ANALOG_Layout::PFD_528_TOG, 1, 31, 0x0> PFD3_CLKGATE;            // IO Clock Gate
constexpr CCM_ANALOG_Reg<regs::constify(&CCM_ANALOG_Layout::PFD_528_TOG), 1, 30> PFD3_STABLE;  // This read-only bitfield is for DIAGNOSTIC PURPOSES ONLY since the fractional divider should become stable quickly enough that this field will never need to be used by either device driver or application code
constexpr CCM_ANALOG_Reg<&CCM_ANALOG_Layout::PFD_528_TOG, 6, 24, 0x0> PFD3_FRAC;               // This field controls the fractional divide value
constexpr CCM_ANALOG_Reg<&CCM_ANALOG_Layout::PFD_528_TOG, 1, 23, 0x0> PFD2_CLKGATE;            // IO Clock Gate
constexpr CCM_ANALOG_Reg<regs::constify(&CCM_ANALOG_Layout::PFD_528_TOG), 1, 22> PFD2_STABLE;  // This read-only bitfield is for DIAGNOSTIC PURPOSES ONLY since the fractional divider should become stable quickly enough that this field will never need to be used by either device driver or application code
constexpr CCM_ANALOG_Reg<&CCM_ANALOG_Layout::PFD_528_TOG, 6, 16, 0x0> PFD2_FRAC;               // This field controls the fractional divide value
constexpr CCM_ANALOG_Reg<&CCM_ANALOG_Layout::PFD_528_TOG, 1, 15, 0x0> PFD1_CLKGATE;            // IO Clock Gate
constexpr CCM_ANALOG_Reg<regs::constify(&CCM_ANALOG_Layout::PFD_528_TOG), 1, 14> PFD1_STABLE;  // This read-only bitfield is for DIAGNOSTIC PURPOSES ONLY since the fractional divider should become stable quickly enough that this field will never need to be used by either device driver or application code
constexpr CCM_ANALOG_Reg<&CCM_ANALOG_Layout::PFD_528_TOG, 6,  8, 0x0> PFD1_FRAC;               // This field controls the fractional divide value
constexpr CCM_ANALOG_Reg<&CCM_ANALOG_Layout::PFD_528_TOG, 1,  7, 0x0> PFD0_CLKGATE;            // If set to 1, the IO fractional divider clock (reference ref_pfd0) is off (power savings)
constexpr CCM_ANALOG_Reg<regs::constify(&CCM_ANALOG_Layout::PFD_528_TOG), 1,  6> PFD0_STABLE;  // This read-only bitfield is for DIAGNOSTIC PURPOSES ONLY since the fractional divider should become stable quickly enough that this field will never need to be used by either device driver or application code
constexpr CCM_ANALOG_Reg<&CCM_ANALOG_Layout::PFD_528_TOG, 6,  0, 0x0> PFD0_FRAC;               // This field controls the fractional divide value
}  // namespace PFD_528_TOG

// Miscellaneous Register 0
namespace MISC0 {
constexpr CCM_ANALOG_Reg<&CCM_ANALOG_Layout::MISC0, 1, 30> XTAL_24M_PWD;                     // This field powers down the 24M crystal oscillator if set true
constexpr CCM_ANALOG_Reg<regs::constify(&CCM_ANALOG_Layout::MISC0), 1, 29> RTC_XTAL_SOURCE;  // This field indicates which chip source is being used for the rtc clock
    // 0b0..Internal ring oscillator
    // 0b1..RTC_XTAL
constexpr CCM_ANALOG_Reg<&CCM_ANALOG_Layout::MISC0, 3, 26> CLKGATE_DELAY;                    // This field specifies the delay between powering up the XTAL 24MHz clock and releasing the clock to the digital logic inside the analog block
    // 0b000..0.5ms
    // 0b001..1.0ms
    // 0b010..2.0ms
    // 0b011..3.0ms
    // 0b100..4.0ms
    // 0b101..5.0ms
    // 0b110..6.0ms
    // 0b111..7.0ms
constexpr CCM_ANALOG_Reg<&CCM_ANALOG_Layout::MISC0, 1, 25> CLKGATE_CTRL;                     // This bit allows disabling the clock gate (always ungated) for the xtal 24MHz clock that clocks the digital logic in the analog block
    // 0b0..Allow the logic to automatically gate the clock when the XTAL is powered down.
    // 0b1..Prevent the logic from ever gating off the clock.
constexpr CCM_ANALOG_Reg<&CCM_ANALOG_Layout::MISC0, 1, 16> OSC_XTALOK_EN;                    // This bit enables the detector that signals when the 24MHz crystal oscillator is stable
constexpr CCM_ANALOG_Reg<regs::constify(&CCM_ANALOG_Layout::MISC0), 1, 15> OSC_XTALOK;       // Status bit that signals that the output of the 24-MHz crystal oscillator is stable
constexpr CCM_ANALOG_Reg<&CCM_ANALOG_Layout::MISC0, 2, 13> OSC_I;                            // This field determines the bias current in the 24MHz oscillator
    // Decrease current by 'value'*12.5%:
    // 0b00..Nominal
    // 0b01..Decrease current by 12.5%
    // 0b10..Decrease current by 25.0%
    // 0b11..Decrease current by 37.5%
constexpr CCM_ANALOG_Reg<&CCM_ANALOG_Layout::MISC0, 1, 12> DISCON_HIGH_SNVS;                 // This bit controls a switch from VDD_HIGH_IN to VDD_SNVS_IN.
    // 0b0..Turn on the switch
    // 0b1..Turn off the switch
constexpr CCM_ANALOG_Reg<&CCM_ANALOG_Layout::MISC0, 2, 10> STOP_MODE_CONFIG;                 // Configure the analog behavior in stop mode.
    // 0b00..All analog except RTC powered down on stop mode assertion.
    // 0b01..Beside RTC, analog bandgap, 1p1 and 2p5 regulators are also on.
    // 0b10..Beside RTC, 1p1 and 2p5 regulators are also on, low-power bandgap is selected so that the normal analog
    //       bandgap together with the rest analog is powered down.
    // 0b11..Beside RTC, low-power bandgap is selected and the rest analog is powered down.
constexpr CCM_ANALOG_Reg<&CCM_ANALOG_Layout::MISC0, 1, 7> REFTOP_VBGUP;                      // Status bit that signals the analog bandgap voltage is up and stable
constexpr CCM_ANALOG_Reg<&CCM_ANALOG_Layout::MISC0, 3, 4> REFTOP_VBGADJ;
    // Not related to CCM. See Power Management Unit (PMU)
    // 0b000..Nominal VBG
    // 0b001..VBG+0.78%
    // 0b010..VBG+1.56%
    // 0b011..VBG+2.34%
    // 0b100..VBG-0.78%
    // 0b101..VBG-1.56%
    // 0b110..VBG-2.34%
    // 0b111..VBG-3.12%
constexpr CCM_ANALOG_Reg<&CCM_ANALOG_Layout::MISC0, 1, 3> REFTOP_SELFBIASOFF;                // Control bit to disable the self-bias circuit in the analog bandgap
    // 0b0..Uses coarse bias currents for startup
    // 0b1..Uses bandgap-based bias currents for best performance.
constexpr CCM_ANALOG_Reg<&CCM_ANALOG_Layout::MISC0, 1, 0> REFTOP_PWD;                        // Control bit to power-down the analog bandgap reference circuitry

// Miscellaneous Register 0 values
constexpr uint32_t kREFTOP_VBGADJ_Nominal    = 0;
constexpr uint32_t kREFTOP_VBGADJ_Plus_0p78  = 1;
constexpr uint32_t kREFTOP_VBGADJ_Plus_1p56  = 2;
constexpr uint32_t kREFTOP_VBGADJ_Plus_2p34  = 3;
constexpr uint32_t kREFTOP_VBGADJ_Minus_0p78 = 4;
constexpr uint32_t kREFTOP_VBGADJ_Minus_1p56 = 5;
constexpr uint32_t kREFTOP_VBGADJ_Minus_2p34 = 6;
constexpr uint32_t kREFTOP_VBGADJ_Minus_3p12 = 7;
constexpr uint32_t kRTC_XTAL_SOURCE_Internal = 0;
constexpr uint32_t kRTC_XTAL_SOURCE_XTAL     = 1;
}  // namespace MISC0

// Miscellaneous Register 0
namespace MISC0_SET {
constexpr CCM_ANALOG_Reg<&CCM_ANALOG_Layout::MISC0_SET, 1, 30, 0x0> XTAL_24M_PWD;                // This field powers down the 24M crystal oscillator if set true
constexpr CCM_ANALOG_Reg<regs::constify(&CCM_ANALOG_Layout::MISC0_SET), 1, 29> RTC_XTAL_SOURCE;  // This field indicates which chip source is being used for the rtc clock
constexpr CCM_ANALOG_Reg<&CCM_ANALOG_Layout::MISC0_SET, 3, 26, 0x0> CLKGATE_DELAY;               // This field specifies the delay between powering up the XTAL 24MHz clock and releasing the clock to the digital logic inside the analog block
constexpr CCM_ANALOG_Reg<&CCM_ANALOG_Layout::MISC0_SET, 1, 25, 0x0> CLKGATE_CTRL;                // This bit allows disabling the clock gate (always ungated) for the xtal 24MHz clock that clocks the digital logic in the analog block
constexpr CCM_ANALOG_Reg<&CCM_ANALOG_Layout::MISC0_SET, 1, 16, 0x0> OSC_XTALOK_EN;               // This bit enables the detector that signals when the 24MHz crystal oscillator is stable
constexpr CCM_ANALOG_Reg<regs::constify(&CCM_ANALOG_Layout::MISC0_SET), 1, 15> OSC_XTALOK;       // Status bit that signals that the output of the 24-MHz crystal oscillator is stable
constexpr CCM_ANALOG_Reg<&CCM_ANALOG_Layout::MISC0_SET, 2, 13, 0x0> OSC_I;                       // This field determines the bias current in the 24MHz oscillator
constexpr CCM_ANALOG_Reg<&CCM_ANALOG_Layout::MISC0_SET, 1, 12, 0x0> DISCON_HIGH_SNVS;            // This bit controls a switch from VDD_HIGH_IN to VDD_SNVS_IN.
constexpr CCM_ANALOG_Reg<&CCM_ANALOG_Layout::MISC0_SET, 2, 10, 0x0> STOP_MODE_CONFIG;            // Configure the analog behavior in stop mode.
constexpr CCM_ANALOG_Reg<&CCM_ANALOG_Layout::MISC0_SET, 1,  7, 0x0> REFTOP_VBGUP;                // Status bit that signals the analog bandgap voltage is up and stable
constexpr CCM_ANALOG_Reg<&CCM_ANALOG_Layout::MISC0_SET, 3,  4, 0x0> REFTOP_VBGADJ;
    // Not related to CCM. See Power Management Unit (PMU)
constexpr CCM_ANALOG_Reg<&CCM_ANALOG_Layout::MISC0_SET, 1,  3, 0x0> REFTOP_SELFBIASOFF;          // Control bit to disable the self-bias circuit in the analog bandgap
constexpr CCM_ANALOG_Reg<&CCM_ANALOG_Layout::MISC0_SET, 1,  0, 0x0> REFTOP_PWD;                  // Control bit to power-down the analog bandgap reference circuitry
}  // namespace MISC0_SET

// Miscellaneous Register 0
namespace MISC0_CLR {
constexpr CCM_ANALOG_Reg<&CCM_ANALOG_Layout::MISC0_CLR, 1, 30, 0x0> XTAL_24M_PWD;                // This field powers down the 24M crystal oscillator if set true
constexpr CCM_ANALOG_Reg<regs::constify(&CCM_ANALOG_Layout::MISC0_CLR), 1, 29> RTC_XTAL_SOURCE;  // This field indicates which chip source is being used for the rtc clock
constexpr CCM_ANALOG_Reg<&CCM_ANALOG_Layout::MISC0_CLR, 3, 26, 0x0> CLKGATE_DELAY;               // This field specifies the delay between powering up the XTAL 24MHz clock and releasing the clock to the digital logic inside the analog block
constexpr CCM_ANALOG_Reg<&CCM_ANALOG_Layout::MISC0_CLR, 1, 25, 0x0> CLKGATE_CTRL;                // This bit allows disabling the clock gate (always ungated) for the xtal 24MHz clock that clocks the digital logic in the analog block
constexpr CCM_ANALOG_Reg<&CCM_ANALOG_Layout::MISC0_CLR, 1, 16, 0x0> OSC_XTALOK_EN;               // This bit enables the detector that signals when the 24MHz crystal oscillator is stable
constexpr CCM_ANALOG_Reg<regs::constify(&CCM_ANALOG_Layout::MISC0_CLR), 1, 15> OSC_XTALOK;       // Status bit that signals that the output of the 24-MHz crystal oscillator is stable
constexpr CCM_ANALOG_Reg<&CCM_ANALOG_Layout::MISC0_CLR, 2, 13, 0x0> OSC_I;                       // This field determines the bias current in the 24MHz oscillator
constexpr CCM_ANALOG_Reg<&CCM_ANALOG_Layout::MISC0_CLR, 1, 12, 0x0> DISCON_HIGH_SNVS;            // This bit controls a switch from VDD_HIGH_IN to VDD_SNVS_IN.
constexpr CCM_ANALOG_Reg<&CCM_ANALOG_Layout::MISC0_CLR, 2, 10, 0x0> STOP_MODE_CONFIG;            // Configure the analog behavior in stop mode.
constexpr CCM_ANALOG_Reg<&CCM_ANALOG_Layout::MISC0_CLR, 1,  7, 0x0> REFTOP_VBGUP;                // Status bit that signals the analog bandgap voltage is up and stable
constexpr CCM_ANALOG_Reg<&CCM_ANALOG_Layout::MISC0_CLR, 3,  4, 0x0> REFTOP_VBGADJ;
    // Not related to CCM. See Power Management Unit (PMU)
constexpr CCM_ANALOG_Reg<&CCM_ANALOG_Layout::MISC0_CLR, 1,  3, 0x0> REFTOP_SELFBIASOFF;          // Control bit to disable the self-bias circuit in the analog bandgap
constexpr CCM_ANALOG_Reg<&CCM_ANALOG_Layout::MISC0_CLR, 1,  0, 0x0> REFTOP_PWD;                  // Control bit to power-down the analog bandgap reference circuitry
}  // namespace MISC0_CLR

// Miscellaneous Register 0
namespace MISC0_TOG {
constexpr CCM_ANALOG_Reg<&CCM_ANALOG_Layout::MISC0_TOG, 1, 30, 0x0> XTAL_24M_PWD;                // This field powers down the 24M crystal oscillator if set true
constexpr CCM_ANALOG_Reg<regs::constify(&CCM_ANALOG_Layout::MISC0_TOG), 1, 29> RTC_XTAL_SOURCE;  // This field indicates which chip source is being used for the rtc clock
constexpr CCM_ANALOG_Reg<&CCM_ANALOG_Layout::MISC0_TOG, 3, 26, 0x0> CLKGATE_DELAY;               // This field specifies the delay between powering up the XTAL 24MHz clock and releasing the clock to the digital logic inside the analog block
constexpr CCM_ANALOG_Reg<&CCM_ANALOG_Layout::MISC0_TOG, 1, 25, 0x0> CLKGATE_CTRL;                // This bit allows disabling the clock gate (always ungated) for the xtal 24MHz clock that clocks the digital logic in the analog block
constexpr CCM_ANALOG_Reg<&CCM_ANALOG_Layout::MISC0_TOG, 1, 16, 0x0> OSC_XTALOK_EN;               // This bit enables the detector that signals when the 24MHz crystal oscillator is stable
constexpr CCM_ANALOG_Reg<regs::constify(&CCM_ANALOG_Layout::MISC0_TOG), 1, 15> OSC_XTALOK;       // Status bit that signals that the output of the 24-MHz crystal oscillator is stable
constexpr CCM_ANALOG_Reg<&CCM_ANALOG_Layout::MISC0_TOG, 2, 13, 0x0> OSC_I;                       // This field determines the bias current in the 24MHz oscillator
constexpr CCM_ANALOG_Reg<&CCM_ANALOG_Layout::MISC0_TOG, 1, 12, 0x0> DISCON_HIGH_SNVS;            // This bit controls a switch from VDD_HIGH_IN to VDD_SNVS_IN.
constexpr CCM_ANALOG_Reg<&CCM_ANALOG_Layout::MISC0_TOG, 2, 10, 0x0> STOP_MODE_CONFIG;            // Configure the analog behavior in stop mode.
constexpr CCM_ANALOG_Reg<&CCM_ANALOG_Layout::MISC0_TOG, 1,  7, 0x0> REFTOP_VBGUP;                // Status bit that signals the analog bandgap voltage is up and stable
constexpr CCM_ANALOG_Reg<&CCM_ANALOG_Layout::MISC0_TOG, 3,  4, 0x0> REFTOP_VBGADJ;
    // Not related to CCM. See Power Management Unit (PMU)
constexpr CCM_ANALOG_Reg<&CCM_ANALOG_Layout::MISC0_TOG, 1,  3, 0x0> REFTOP_SELFBIASOFF;          // Control bit to disable the self-bias circuit in the analog bandgap
constexpr CCM_ANALOG_Reg<&CCM_ANALOG_Layout::MISC0_TOG, 1,  0, 0x0> REFTOP_PWD;                  // Control bit to power-down the analog bandgap reference circuitry
}  // namespace MISC0_TOG

// Miscellaneous Register 1
namespace MISC1 {
constexpr uint32_t kW1C = 0xf800'0000;

constexpr CCM_ANALOG_Reg<&CCM_ANALOG_Layout::MISC1, 1, 31, kW1C> IRQ_DIG_BO;                                          // This status bit is set to one when when any of the digital regulator brownout interrupts assert
constexpr CCM_ANALOG_Reg<&CCM_ANALOG_Layout::MISC1, 1, 30, kW1C> IRQ_ANA_BO;                                          // This status bit is set to one when when any of the analog regulator brownout interrupts assert
constexpr CCM_ANALOG_Reg<&CCM_ANALOG_Layout::MISC1, 1, 29, kW1C> IRQ_TEMPHIGH;                                        // This status bit is set to one when the temperature sensor high interrupt asserts for high temperature
constexpr CCM_ANALOG_Reg<&CCM_ANALOG_Layout::MISC1, 1, 28, kW1C> IRQ_TEMPLOW;                                         // This status bit is set to one when the temperature sensor low interrupt asserts for low temperature
constexpr CCM_ANALOG_Reg<&CCM_ANALOG_Layout::MISC1, 1, 27, kW1C> IRQ_TEMPPANIC;                                       // This status bit is set to one when the temperature sensor panic interrupt asserts for a panic high temperature
constexpr CCM_ANALOG_Reg<&CCM_ANALOG_Layout::MISC1, 1, 17, regs::shiftedMask32<1, 17>() | kW1C> PFD_528_AUTOGATE_EN;  // This enables a feature that will clkgate (reset) all PFD_528 clocks anytime the PLL_528 is unlocked or powered off
constexpr CCM_ANALOG_Reg<&CCM_ANALOG_Layout::MISC1, 1, 16, regs::shiftedMask32<1, 16>() | kW1C> PFD_480_AUTOGATE_EN;  // This enables a feature that will clkgate (reset) all PFD_480 clocks anytime the USB1_PLL_480 is unlocked or powered off
constexpr CCM_ANALOG_Reg<&CCM_ANALOG_Layout::MISC1, 1, 12, regs::shiftedMask32<1, 12>() | kW1C> LVDSCLK1_IBEN;        // This enables the LVDS input buffer for anaclk1/1b
constexpr CCM_ANALOG_Reg<&CCM_ANALOG_Layout::MISC1, 1, 10, regs::shiftedMask32<1, 10>() | kW1C> LVDSCLK1_OBEN;        // This enables the LVDS output buffer for anaclk1/1b
constexpr CCM_ANALOG_Reg<&CCM_ANALOG_Layout::MISC1, 5,  0, regs::shiftedMask32<5,  0>() | kW1C> LVDS1_CLK_SEL;        // This field selects the clk to be routed to anaclk1/1b.
    // 0b00000..Arm PLL
    // 0b00001..System PLL
    // 0b00010..ref_pfd4_clk == pll2_pfd0_clk
    // 0b00011..ref_pfd5_clk == pll2_pfd1_clk
    // 0b00100..ref_pfd6_clk == pll2_pfd2_clk
    // 0b00101..ref_pfd7_clk == pll2_pfd3_clk
    // 0b00110..Audio PLL
    // 0b00111..Video PLL
    // 0b01001..ethernet ref clock (ENET_PLL)
    // 0b01100..USB1 PLL clock
    // 0b01101..USB2 PLL clock
    // 0b01110..ref_pfd0_clk == pll3_pfd0_clk
    // 0b01111..ref_pfd1_clk == pll3_pfd1_clk
    // 0b10000..ref_pfd2_clk == pll3_pfd2_clk
    // 0b10001..ref_pfd3_clk == pll3_pfd3_clk
    // 0b10010..xtal (24M)
}  // namespace MISC1

// Miscellaneous Register 1
namespace MISC1_SET {
constexpr CCM_ANALOG_Reg<&CCM_ANALOG_Layout::MISC1_SET, 1, 31, 0x0> IRQ_DIG_BO;           // This status bit is set to one when when any of the digital regulator brownout interrupts assert
constexpr CCM_ANALOG_Reg<&CCM_ANALOG_Layout::MISC1_SET, 1, 30, 0x0> IRQ_ANA_BO;           // This status bit is set to one when when any of the analog regulator brownout interrupts assert
constexpr CCM_ANALOG_Reg<&CCM_ANALOG_Layout::MISC1_SET, 1, 29, 0x0> IRQ_TEMPHIGH;         // This status bit is set to one when the temperature sensor high interrupt asserts for high temperature
constexpr CCM_ANALOG_Reg<&CCM_ANALOG_Layout::MISC1_SET, 1, 28, 0x0> IRQ_TEMPLOW;          // This status bit is set to one when the temperature sensor low interrupt asserts for low temperature
constexpr CCM_ANALOG_Reg<&CCM_ANALOG_Layout::MISC1_SET, 1, 27, 0x0> IRQ_TEMPPANIC;        // This status bit is set to one when the temperature sensor panic interrupt asserts for a panic high temperature
constexpr CCM_ANALOG_Reg<&CCM_ANALOG_Layout::MISC1_SET, 1, 17, 0x0> PFD_528_AUTOGATE_EN;  // This enables a feature that will clkgate (reset) all PFD_528 clocks anytime the PLL_528 is unlocked or powered off
constexpr CCM_ANALOG_Reg<&CCM_ANALOG_Layout::MISC1_SET, 1, 16, 0x0> PFD_480_AUTOGATE_EN;  // This enables a feature that will clkgate (reset) all PFD_480 clocks anytime the USB1_PLL_480 is unlocked or powered off
constexpr CCM_ANALOG_Reg<&CCM_ANALOG_Layout::MISC1_SET, 1, 12, 0x0> LVDSCLK1_IBEN;        // This enables the LVDS input buffer for anaclk1/1b
constexpr CCM_ANALOG_Reg<&CCM_ANALOG_Layout::MISC1_SET, 1, 10, 0x0> LVDSCLK1_OBEN;        // This enables the LVDS output buffer for anaclk1/1b
constexpr CCM_ANALOG_Reg<&CCM_ANALOG_Layout::MISC1_SET, 5,  0, 0x0> LVDS1_CLK_SEL;        // This field selects the clk to be routed to anaclk1/1b.
}  // namespace MISC1_SET

// Miscellaneous Register 1
namespace MISC1_CLR {
constexpr CCM_ANALOG_Reg<&CCM_ANALOG_Layout::MISC1_CLR, 1, 31, 0x0> IRQ_DIG_BO;           // This status bit is set to one when when any of the digital regulator brownout interrupts assert
constexpr CCM_ANALOG_Reg<&CCM_ANALOG_Layout::MISC1_CLR, 1, 30, 0x0> IRQ_ANA_BO;           // This status bit is set to one when when any of the analog regulator brownout interrupts assert
constexpr CCM_ANALOG_Reg<&CCM_ANALOG_Layout::MISC1_CLR, 1, 29, 0x0> IRQ_TEMPHIGH;         // This status bit is set to one when the temperature sensor high interrupt asserts for high temperature
constexpr CCM_ANALOG_Reg<&CCM_ANALOG_Layout::MISC1_CLR, 1, 28, 0x0> IRQ_TEMPLOW;          // This status bit is set to one when the temperature sensor low interrupt asserts for low temperature
constexpr CCM_ANALOG_Reg<&CCM_ANALOG_Layout::MISC1_CLR, 1, 27, 0x0> IRQ_TEMPPANIC;        // This status bit is set to one when the temperature sensor panic interrupt asserts for a panic high temperature
constexpr CCM_ANALOG_Reg<&CCM_ANALOG_Layout::MISC1_CLR, 1, 17, 0x0> PFD_528_AUTOGATE_EN;  // This enables a feature that will clkgate (reset) all PFD_528 clocks anytime the PLL_528 is unlocked or powered off
constexpr CCM_ANALOG_Reg<&CCM_ANALOG_Layout::MISC1_CLR, 1, 16, 0x0> PFD_480_AUTOGATE_EN;  // This enables a feature that will clkgate (reset) all PFD_480 clocks anytime the USB1_PLL_480 is unlocked or powered off
constexpr CCM_ANALOG_Reg<&CCM_ANALOG_Layout::MISC1_CLR, 1, 12, 0x0> LVDSCLK1_IBEN;        // This enables the LVDS input buffer for anaclk1/1b
constexpr CCM_ANALOG_Reg<&CCM_ANALOG_Layout::MISC1_CLR, 1, 10, 0x0> LVDSCLK1_OBEN;        // This enables the LVDS output buffer for anaclk1/1b
constexpr CCM_ANALOG_Reg<&CCM_ANALOG_Layout::MISC1_CLR, 5,  0, 0x0> LVDS1_CLK_SEL;        // This field selects the clk to be routed to anaclk1/1b.
}  // namespace MISC1_CLR

// Miscellaneous Register 1
namespace MISC1_TOG {
constexpr CCM_ANALOG_Reg<&CCM_ANALOG_Layout::MISC1_TOG, 1, 31, 0x0> IRQ_DIG_BO;           // This status bit is set to one when when any of the digital regulator brownout interrupts assert
constexpr CCM_ANALOG_Reg<&CCM_ANALOG_Layout::MISC1_TOG, 1, 30, 0x0> IRQ_ANA_BO;           // This status bit is set to one when when any of the analog regulator brownout interrupts assert
constexpr CCM_ANALOG_Reg<&CCM_ANALOG_Layout::MISC1_TOG, 1, 29, 0x0> IRQ_TEMPHIGH;         // This status bit is set to one when the temperature sensor high interrupt asserts for high temperature
constexpr CCM_ANALOG_Reg<&CCM_ANALOG_Layout::MISC1_TOG, 1, 28, 0x0> IRQ_TEMPLOW;          // This status bit is set to one when the temperature sensor low interrupt asserts for low temperature
constexpr CCM_ANALOG_Reg<&CCM_ANALOG_Layout::MISC1_TOG, 1, 27, 0x0> IRQ_TEMPPANIC;        // This status bit is set to one when the temperature sensor panic interrupt asserts for a panic high temperature
constexpr CCM_ANALOG_Reg<&CCM_ANALOG_Layout::MISC1_TOG, 1, 17, 0x0> PFD_528_AUTOGATE_EN;  // This enables a feature that will clkgate (reset) all PFD_528 clocks anytime the PLL_528 is unlocked or powered off
constexpr CCM_ANALOG_Reg<&CCM_ANALOG_Layout::MISC1_TOG, 1, 16, 0x0> PFD_480_AUTOGATE_EN;  // This enables a feature that will clkgate (reset) all PFD_480 clocks anytime the USB1_PLL_480 is unlocked or powered off
constexpr CCM_ANALOG_Reg<&CCM_ANALOG_Layout::MISC1_TOG, 1, 12, 0x0> LVDSCLK1_IBEN;        // This enables the LVDS input buffer for anaclk1/1b
constexpr CCM_ANALOG_Reg<&CCM_ANALOG_Layout::MISC1_TOG, 1, 10, 0x0> LVDSCLK1_OBEN;        // This enables the LVDS output buffer for anaclk1/1b
constexpr CCM_ANALOG_Reg<&CCM_ANALOG_Layout::MISC1_TOG, 5,  0, 0x0> LVDS1_CLK_SEL;        // This field selects the clk to be routed to anaclk1/1b.
}  // namespace MISC1_TOG

// Miscellaneous Register 2
namespace MISC2 {
constexpr CCM_ANALOG_Reg<&CCM_ANALOG_Layout::MISC2, 2, 30> VIDEO_DIV;                       // Post-divider for video
    // 0b00..divide by 1 (Default)
    // 0b01..divide by 2
    // 0b10..divide by 1
    // 0b11..divide by 4
constexpr CCM_ANALOG_Reg<&CCM_ANALOG_Layout::MISC2, 2, 28> REG2_STEP_TIME;                  // Number of clock periods (24MHz clock).
    // Not related to CCM. See Power Management Unit (PMU)
    // 64 * 2^'value':
    // 0b00..64
    // 0b01..128
    // 0b10..256
    // 0b11..512
constexpr CCM_ANALOG_Reg<&CCM_ANALOG_Layout::MISC2, 2, 26> REG1_STEP_TIME;                  // Number of clock periods (24MHz clock).
    // Not related to CCM. See Power Management Unit (PMU)
    // 64 * 2^'value':
    // 0b00..64
    // 0b01..128
    // 0b10..256
    // 0b11..512
constexpr CCM_ANALOG_Reg<&CCM_ANALOG_Layout::MISC2, 2, 24> REG0_STEP_TIME;                  // Number of clock periods (24MHz clock).
    // Not related to CCM. See Power Management Unit (PMU)
    // 64 * 2^'value':
    // 0b00..64
    // 0b01..128
    // 0b10..256
    // 0b11..512
constexpr CCM_ANALOG_Reg<&CCM_ANALOG_Layout::MISC2, 1, 23> AUDIO_DIV_MSB;                   // MSB of Post-divider for Audio PLL
    // 1/2^'value':
    // 0b0..divide by 1 (Default)
    // 0b1..divide by 2
constexpr CCM_ANALOG_Reg<regs::constify(&CCM_ANALOG_Layout::MISC2), 1, 22> REG2_OK;         // Signals that the voltage is above the brownout level for the SOC supply
constexpr CCM_ANALOG_Reg<&CCM_ANALOG_Layout::MISC2, 1, 21> REG2_ENABLE_BO;                  // Enables the brownout detection.
    // Not related to CCM. See Power Management Unit (PMU)
constexpr CCM_ANALOG_Reg<regs::constify(&CCM_ANALOG_Layout::MISC2), 1, 19> REG2_BO_STATUS;  // Reg2 brownout status bit.
    // Not related to CCM. See Power Management Unit (PMU)
constexpr CCM_ANALOG_Reg<regs::constify(&CCM_ANALOG_Layout::MISC2), 3, 16> REG2_BO_OFFSET;  // This field defines the brown out voltage offset for the xPU power domain
    // 0b100..Brownout offset = 0.100V
    // 0b111..Brownout offset = 0.175V
constexpr CCM_ANALOG_Reg<&CCM_ANALOG_Layout::MISC2, 1, 15> AUDIO_DIV_LSB;                   // LSB of Post-divider for Audio PLL
    // 1/2^'value':
    // 0b0..divide by 1 (Default)
    // 0b1..divide by 2
constexpr CCM_ANALOG_Reg<regs::constify(&CCM_ANALOG_Layout::MISC2), 1, 14> REG1_OK;         // GPU/VPU supply
    // Not related to CCM. See Power Management Unit (PMU)
constexpr CCM_ANALOG_Reg<&CCM_ANALOG_Layout::MISC2, 1, 13> REG1_ENABLE_BO;                  // Enables the brownout detection.
    // Not related to CCM. See Power Management Unit (PMU)
constexpr CCM_ANALOG_Reg<regs::constify(&CCM_ANALOG_Layout::MISC2), 1, 11> REG1_BO_STATUS;  // Reg1 brownout status bit.
    // Not related to CCM. See Power Management Unit (PMU)
    // 0b1..Brownout, supply is below target minus brownout offset.
constexpr CCM_ANALOG_Reg<regs::constify(&CCM_ANALOG_Layout::MISC2), 3,  8> REG1_BO_OFFSET;  // This field defines the brown out voltage offset for the xPU power domain
    // 0b100..Brownout offset = 0.100V
    // 0b111..Brownout offset = 0.175V
constexpr CCM_ANALOG_Reg<&CCM_ANALOG_Layout::MISC2, 1,  7> PLL3_DISABLE;                    // When USB is in low power suspend mode this Control bit is used to indicate if other system peripherals require the USB PLL3 clock when the SoC is not in low power mode
    // 0b0..PLL3 is being used by peripherals and is enabled when SoC is not in any low power mode
    // 0b1..PLL3 can be disabled when the SoC is not in any low power mode
constexpr CCM_ANALOG_Reg<regs::constify(&CCM_ANALOG_Layout::MISC2), 1,  6> REG0_OK;         // Arm supply
    // Not related to CCM. See Power Management Unit (PMU)
constexpr CCM_ANALOG_Reg<&CCM_ANALOG_Layout::MISC2, 1,  5> REG0_ENABLE_BO;                  // Enables the brownout detection.
    // Not related to CCM. See Power Management Unit (PMU)
constexpr CCM_ANALOG_Reg<regs::constify(&CCM_ANALOG_Layout::MISC2), 1,  3> REG0_BO_STATUS;  // Reg0 brownout status bit.
    // Not related to CCM. See Power Management Unit (PMU)
    // 0b1..Brownout, supply is below target minus brownout offset.
constexpr CCM_ANALOG_Reg<regs::constify(&CCM_ANALOG_Layout::MISC2), 3,  0> REG0_BO_OFFSET;  // This field defines the brown out voltage offset for the CORE power domain
    // 0b100..Brownout offset = 0.100V
    // 0b111..Brownout offset = 0.175V
}  // namespace MISC2

// Miscellaneous Register 2
namespace MISC2_SET {
constexpr CCM_ANALOG_Reg<&CCM_ANALOG_Layout::MISC2_SET, 2, 30, 0x0> VIDEO_DIV;                  // Post-divider for video
constexpr CCM_ANALOG_Reg<&CCM_ANALOG_Layout::MISC2_SET, 2, 28, 0x0> REG2_STEP_TIME;             // Number of clock periods (24MHz clock).
    // Not related to CCM. See Power Management Unit (PMU)
constexpr CCM_ANALOG_Reg<&CCM_ANALOG_Layout::MISC2_SET, 2, 26, 0x0> REG1_STEP_TIME;             // Number of clock periods (24MHz clock).
    // Not related to CCM. See Power Management Unit (PMU)
constexpr CCM_ANALOG_Reg<&CCM_ANALOG_Layout::MISC2_SET, 2, 24, 0x0> REG0_STEP_TIME;             // Number of clock periods (24MHz clock).
    // Not related to CCM. See Power Management Unit (PMU)
constexpr CCM_ANALOG_Reg<&CCM_ANALOG_Layout::MISC2_SET, 1, 23, 0x0> AUDIO_DIV_MSB;              // MSB of Post-divider for Audio PLL
constexpr CCM_ANALOG_Reg<regs::constify(&CCM_ANALOG_Layout::MISC2_SET), 1, 22> REG2_OK;         // Signals that the voltage is above the brownout level for the SOC supply
constexpr CCM_ANALOG_Reg<&CCM_ANALOG_Layout::MISC2_SET, 1, 21, 0x0> REG2_ENABLE_BO;             // Enables the brownout detection.
    // Not related to CCM. See Power Management Unit (PMU)
constexpr CCM_ANALOG_Reg<regs::constify(&CCM_ANALOG_Layout::MISC2_SET), 1, 19> REG2_BO_STATUS;  // Reg2 brownout status bit.
    // Not related to CCM. See Power Management Unit (PMU)
constexpr CCM_ANALOG_Reg<regs::constify(&CCM_ANALOG_Layout::MISC2_SET), 3, 16> REG2_BO_OFFSET;  // This field defines the brown out voltage offset for the xPU power domain
constexpr CCM_ANALOG_Reg<&CCM_ANALOG_Layout::MISC2_SET, 1, 15, 0x0> AUDIO_DIV_LSB;              // LSB of Post-divider for Audio PLL
constexpr CCM_ANALOG_Reg<regs::constify(&CCM_ANALOG_Layout::MISC2_SET), 1, 14> REG1_OK;         // GPU/VPU supply
    // Not related to CCM. See Power Management Unit (PMU)
constexpr CCM_ANALOG_Reg<&CCM_ANALOG_Layout::MISC2_SET, 1, 13, 0x0> REG1_ENABLE_BO;             // Enables the brownout detection.
    // Not related to CCM. See Power Management Unit (PMU)
constexpr CCM_ANALOG_Reg<regs::constify(&CCM_ANALOG_Layout::MISC2_SET), 1, 11> REG1_BO_STATUS;  // Reg1 brownout status bit.
    // Not related to CCM. See Power Management Unit (PMU)
constexpr CCM_ANALOG_Reg<regs::constify(&CCM_ANALOG_Layout::MISC2_SET), 3,  8> REG1_BO_OFFSET;  // This field defines the brown out voltage offset for the xPU power domain
constexpr CCM_ANALOG_Reg<&CCM_ANALOG_Layout::MISC2_SET, 1,  7, 0x0> PLL3_DISABLE;               // When USB is in low power suspend mode this Control bit is used to indicate if other system peripherals require the USB PLL3 clock when the SoC is not in low power mode
constexpr CCM_ANALOG_Reg<regs::constify(&CCM_ANALOG_Layout::MISC2_SET), 1,  6> REG0_OK;         // Arm supply
    // Not related to CCM. See Power Management Unit (PMU)
constexpr CCM_ANALOG_Reg<&CCM_ANALOG_Layout::MISC2_SET, 1,  5, 0x0> REG0_ENABLE_BO;             // Enables the brownout detection.
    // Not related to CCM. See Power Management Unit (PMU)
constexpr CCM_ANALOG_Reg<regs::constify(&CCM_ANALOG_Layout::MISC2_SET), 1,  3> REG0_BO_STATUS;  // Reg0 brownout status bit.
    // Not related to CCM. See Power Management Unit (PMU)
constexpr CCM_ANALOG_Reg<regs::constify(&CCM_ANALOG_Layout::MISC2_SET), 3,  0> REG0_BO_OFFSET;  // This field defines the brown out voltage offset for the CORE power domain
}  // namespace MISC2_SET

// Miscellaneous Register 2
namespace MISC2_CLR {
constexpr CCM_ANALOG_Reg<&CCM_ANALOG_Layout::MISC2_CLR, 2, 30, 0x0> VIDEO_DIV;                  // Post-divider for video
constexpr CCM_ANALOG_Reg<&CCM_ANALOG_Layout::MISC2_CLR, 2, 28, 0x0> REG2_STEP_TIME;             // Number of clock periods (24MHz clock).
    // Not related to CCM. See Power Management Unit (PMU)
constexpr CCM_ANALOG_Reg<&CCM_ANALOG_Layout::MISC2_CLR, 2, 26, 0x0> REG1_STEP_TIME;             // Number of clock periods (24MHz clock).
    // Not related to CCM. See Power Management Unit (PMU)
constexpr CCM_ANALOG_Reg<&CCM_ANALOG_Layout::MISC2_CLR, 2, 24, 0x0> REG0_STEP_TIME;             // Number of clock periods (24MHz clock).
    // Not related to CCM. See Power Management Unit (PMU)
constexpr CCM_ANALOG_Reg<&CCM_ANALOG_Layout::MISC2_CLR, 1, 23, 0x0> AUDIO_DIV_MSB;              // MSB of Post-divider for Audio PLL
constexpr CCM_ANALOG_Reg<regs::constify(&CCM_ANALOG_Layout::MISC2_CLR), 1, 22> REG2_OK;         // Signals that the voltage is above the brownout level for the SOC supply
constexpr CCM_ANALOG_Reg<&CCM_ANALOG_Layout::MISC2_CLR, 1, 21, 0x0> REG2_ENABLE_BO;             // Enables the brownout detection.
    // Not related to CCM. See Power Management Unit (PMU)
constexpr CCM_ANALOG_Reg<regs::constify(&CCM_ANALOG_Layout::MISC2_CLR), 1, 19> REG2_BO_STATUS;  // Reg2 brownout status bit.
    // Not related to CCM. See Power Management Unit (PMU)
constexpr CCM_ANALOG_Reg<regs::constify(&CCM_ANALOG_Layout::MISC2_CLR), 3, 16> REG2_BO_OFFSET;  // This field defines the brown out voltage offset for the xPU power domain
constexpr CCM_ANALOG_Reg<&CCM_ANALOG_Layout::MISC2_CLR, 1, 15, 0x0> AUDIO_DIV_LSB;              // LSB of Post-divider for Audio PLL
constexpr CCM_ANALOG_Reg<regs::constify(&CCM_ANALOG_Layout::MISC2_CLR), 1, 14> REG1_OK;         // GPU/VPU supply
    // Not related to CCM. See Power Management Unit (PMU)
constexpr CCM_ANALOG_Reg<&CCM_ANALOG_Layout::MISC2_CLR, 1, 13, 0x0> REG1_ENABLE_BO;             // Enables the brownout detection.
    // Not related to CCM. See Power Management Unit (PMU)
constexpr CCM_ANALOG_Reg<regs::constify(&CCM_ANALOG_Layout::MISC2_CLR), 1, 11> REG1_BO_STATUS;  // Reg1 brownout status bit.
    // Not related to CCM. See Power Management Unit (PMU)
constexpr CCM_ANALOG_Reg<regs::constify(&CCM_ANALOG_Layout::MISC2_CLR), 3,  8> REG1_BO_OFFSET;  // This field defines the brown out voltage offset for the xPU power domain
constexpr CCM_ANALOG_Reg<&CCM_ANALOG_Layout::MISC2_CLR, 1,  7, 0x0> PLL3_DISABLE;               // When USB is in low power suspend mode this Control bit is used to indicate if other system peripherals require the USB PLL3 clock when the SoC is not in low power mode
constexpr CCM_ANALOG_Reg<regs::constify(&CCM_ANALOG_Layout::MISC2_CLR), 1,  6> REG0_OK;         // Arm supply
    // Not related to CCM. See Power Management Unit (PMU)
constexpr CCM_ANALOG_Reg<&CCM_ANALOG_Layout::MISC2_CLR, 1,  5, 0x0> REG0_ENABLE_BO;             // Enables the brownout detection.
    // Not related to CCM. See Power Management Unit (PMU)
constexpr CCM_ANALOG_Reg<regs::constify(&CCM_ANALOG_Layout::MISC2_CLR), 1,  3> REG0_BO_STATUS;  // Reg0 brownout status bit.
    // Not related to CCM. See Power Management Unit (PMU)
constexpr CCM_ANALOG_Reg<regs::constify(&CCM_ANALOG_Layout::MISC2_CLR), 3,  0> REG0_BO_OFFSET;  // This field defines the brown out voltage offset for the CORE power domain
}  // namespace MISC2_CLR

// Miscellaneous Register 2
namespace MISC2_TOG {
constexpr CCM_ANALOG_Reg<&CCM_ANALOG_Layout::MISC2_TOG, 2, 30, 0x0> VIDEO_DIV;                  // Post-divider for video
constexpr CCM_ANALOG_Reg<&CCM_ANALOG_Layout::MISC2_TOG, 2, 28, 0x0> REG2_STEP_TIME;             // Number of clock periods (24MHz clock).
    // Not related to CCM. See Power Management Unit (PMU)
constexpr CCM_ANALOG_Reg<&CCM_ANALOG_Layout::MISC2_TOG, 2, 26, 0x0> REG1_STEP_TIME;             // Number of clock periods (24MHz clock).
    // Not related to CCM. See Power Management Unit (PMU)
constexpr CCM_ANALOG_Reg<&CCM_ANALOG_Layout::MISC2_TOG, 2, 24, 0x0> REG0_STEP_TIME;             // Number of clock periods (24MHz clock).
    // Not related to CCM. See Power Management Unit (PMU)
constexpr CCM_ANALOG_Reg<&CCM_ANALOG_Layout::MISC2_TOG, 1, 23, 0x0> AUDIO_DIV_MSB;              // MSB of Post-divider for Audio PLL
constexpr CCM_ANALOG_Reg<regs::constify(&CCM_ANALOG_Layout::MISC2_TOG), 1, 22> REG2_OK;         // Signals that the voltage is above the brownout level for the SOC supply
constexpr CCM_ANALOG_Reg<&CCM_ANALOG_Layout::MISC2_TOG, 1, 21, 0x0> REG2_ENABLE_BO;             // Enables the brownout detection.
    // Not related to CCM. See Power Management Unit (PMU)
constexpr CCM_ANALOG_Reg<regs::constify(&CCM_ANALOG_Layout::MISC2_TOG), 1, 19> REG2_BO_STATUS;  // Reg2 brownout status bit.
    // Not related to CCM. See Power Management Unit (PMU)
constexpr CCM_ANALOG_Reg<regs::constify(&CCM_ANALOG_Layout::MISC2_TOG), 3, 16> REG2_BO_OFFSET;  // This field defines the brown out voltage offset for the xPU power domain
constexpr CCM_ANALOG_Reg<&CCM_ANALOG_Layout::MISC2_TOG, 1, 15, 0x0> AUDIO_DIV_LSB;              // LSB of Post-divider for Audio PLL
constexpr CCM_ANALOG_Reg<regs::constify(&CCM_ANALOG_Layout::MISC2_TOG), 1, 14> REG1_OK;         // GPU/VPU supply
    // Not related to CCM. See Power Management Unit (PMU)
constexpr CCM_ANALOG_Reg<&CCM_ANALOG_Layout::MISC2_TOG, 1, 13, 0x0> REG1_ENABLE_BO;             // Enables the brownout detection.
    // Not related to CCM. See Power Management Unit (PMU)
constexpr CCM_ANALOG_Reg<regs::constify(&CCM_ANALOG_Layout::MISC2_TOG), 1, 11> REG1_BO_STATUS;  // Reg1 brownout status bit.
    // Not related to CCM. See Power Management Unit (PMU)
constexpr CCM_ANALOG_Reg<regs::constify(&CCM_ANALOG_Layout::MISC2_TOG), 3,  8> REG1_BO_OFFSET;  // This field defines the brown out voltage offset for the xPU power domain
constexpr CCM_ANALOG_Reg<&CCM_ANALOG_Layout::MISC2_TOG, 1,  7, 0x0> PLL3_DISABLE;               // When USB is in low power suspend mode this Control bit is used to indicate if other system peripherals require the USB PLL3 clock when the SoC is not in low power mode
constexpr CCM_ANALOG_Reg<regs::constify(&CCM_ANALOG_Layout::MISC2_TOG), 1,  6> REG0_OK;         // Arm supply
    // Not related to CCM. See Power Management Unit (PMU)
constexpr CCM_ANALOG_Reg<&CCM_ANALOG_Layout::MISC2_TOG, 1,  5, 0x0> REG0_ENABLE_BO;             // Enables the brownout detection.
    // Not related to CCM. See Power Management Unit (PMU)
constexpr CCM_ANALOG_Reg<regs::constify(&CCM_ANALOG_Layout::MISC2_TOG), 1,  3> REG0_BO_STATUS;  // Reg0 brownout status bit.
    // Not related to CCM. See Power Management Unit (PMU)
constexpr CCM_ANALOG_Reg<regs::constify(&CCM_ANALOG_Layout::MISC2_TOG), 3,  0> REG0_BO_OFFSET;  // This field defines the brown out voltage offset for the CORE power domain
}  // namespace MISC2_TOG

// CCM_ANALOG_PLL_xxx_BYPASS_CLK_SRC values
namespace PLL {
constexpr uint32_t kBYPASS_CLK_SRC_24MHz = 0;
constexpr uint32_t kBYPASS_CLK_SRC_CLK1  = 1;
}  // namespace PLL

}  // namespace CCM_ANALOG

}  // namespace imxrt1060
}  // namespace hardware
}  // namespace qindesign
