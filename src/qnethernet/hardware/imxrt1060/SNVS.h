// SPDX-FileCopyrightText: (c) 2026 Shawn Silverman <shawn@pobox.com>
// SPDX-License-Identifier: AGPL-3.0-or-later

// SNVS.h defines all the SNVS registers.
// This file is part of the imxrt1060-regs library.

#pragma once

#include <cstddef>
#include <cstdint>

#include "qnethernet/hardware/regs/regs.h"

// Undefine some Arduino nonsense because of field name conflicts
#undef PI

namespace qindesign {
namespace hardware {
namespace imxrt1060 {

// Structure type to access the SNVS_HP and SNVS_LP (SNVS) registers.
//
// Comments are from BSD-3-licensed NXP SDK.
//
// See:
// * https://github.com/nxp-mcuxpresso/mcux-devices-rt/blob/main/RT1060/periph/PERI_SNVS.h
// * https://github.com/nxp-mcuxpresso/legacy-mcux-sdk/blob/main/devices/MIMXRT1062/MIMXRT1062.h

// Size of Registers Arrays
constexpr size_t kSNVS_LPZMKR_count        = 8;
constexpr size_t kSNVS_LPGPR0_LPGPR3_count = 4;
constexpr size_t kSNVS_LPGPR0_LPGPR7_count = 8;

struct SNVS_Layout {
  volatile uint32_t HPLR;                              /**< SNVS_HP Lock Register, offset: 0x0 */
  volatile uint32_t HPCOMR;                            /**< SNVS_HP Command Register, offset: 0x4 */
  volatile uint32_t HPCR;                              /**< SNVS_HP Control Register, offset: 0x8 */
  volatile uint32_t HPSICR;                            /**< SNVS_HP Security Interrupt Control Register, offset: 0xC */
  volatile uint32_t HPSVCR;                            /**< SNVS_HP Security Violation Control Register, offset: 0x10 */
  volatile uint32_t HPSR;                              /**< SNVS_HP Status Register, offset: 0x14 */
  volatile uint32_t HPSVSR;                            /**< SNVS_HP Security Violation Status Register, offset: 0x18 */
  volatile uint32_t HPHACIVR;                          /**< SNVS_HP High Assurance Counter IV Register, offset: 0x1C */
  const volatile uint32_t HPHACR;                      /**< SNVS_HP High Assurance Counter Register, offset: 0x20 */
  volatile uint32_t HPRTCMR;                           /**< SNVS_HP Real Time Counter MSB Register, offset: 0x24 */
  volatile uint32_t HPRTCLR;                           /**< SNVS_HP Real Time Counter LSB Register, offset: 0x28 */
  volatile uint32_t HPTAMR;                            /**< SNVS_HP Time Alarm MSB Register, offset: 0x2C */
  volatile uint32_t HPTALR;                            /**< SNVS_HP Time Alarm LSB Register, offset: 0x30 */
  volatile uint32_t LPLR;                              /**< SNVS_LP Lock Register, offset: 0x34 */
  volatile uint32_t LPCR;                              /**< SNVS_LP Control Register, offset: 0x38 */
  volatile uint32_t LPMKCR;                            /**< SNVS_LP Master Key Control Register, offset: 0x3C */
  volatile uint32_t LPSVCR;                            /**< SNVS_LP Security Violation Control Register, offset: 0x40 */
  uint32_t HARDWARE_REGS_LAYOUT_MEMBER_RESERVED[1];
  volatile uint32_t LPSECR;                            /**< SNVS_LP Security Events Configuration Register, offset: 0x48 */
  volatile uint32_t LPSR;                              /**< SNVS_LP Status Register, offset: 0x4C */
  volatile uint32_t LPSRTCMR;                          /**< SNVS_LP Secure Real Time Counter MSB Register, offset: 0x50 */
  volatile uint32_t LPSRTCLR;                          /**< SNVS_LP Secure Real Time Counter LSB Register, offset: 0x54 */
  volatile uint32_t LPTAR;                             /**< SNVS_LP Time Alarm Register, offset: 0x58 */
  volatile uint32_t LPSMCMR;                           /**< SNVS_LP Secure Monotonic Counter MSB Register, offset: 0x5C */
  volatile uint32_t LPSMCLR;                           /**< SNVS_LP Secure Monotonic Counter LSB Register, offset: 0x60 */
  volatile uint32_t LPLVDR;                            /**< SNVS_LP Digital Low-Voltage Detector Register, offset: 0x64 */
  volatile uint32_t LPGPR0_LEGACY_ALIAS;               /**< SNVS_LP General Purpose Register 0 (legacy alias), offset: 0x68 */
  volatile uint32_t LPZMKR[kSNVS_LPZMKR_count];        /**< SNVS_LP Zeroizable Master Key Register, array offset: 0x6C, array step: 0x4 */
  uint32_t HARDWARE_REGS_LAYOUT_MEMBER_RESERVED[1];
  volatile uint32_t LPGPR_ALIAS[kSNVS_LPGPR0_LPGPR3_count]; /**< SNVS_LP General Purpose Registers 0 .. 3, array offset: 0x90, array step: 0x4 */
  uint32_t HARDWARE_REGS_LAYOUT_MEMBER_RESERVED[24];
  volatile uint32_t LPGPR[kSNVS_LPGPR0_LPGPR7_count];  /**< SNVS_LP General Purpose Registers 0 .. 7, array offset: 0x100, array step: 0x4 */
  uint32_t HARDWARE_REGS_LAYOUT_MEMBER_RESERVED[694];
  const volatile uint32_t HPVIDR1;                     /**< SNVS_HP Version ID Register 1, offset: 0xBF8 */
  const volatile uint32_t HPVIDR2;                     /**< SNVS_HP Version ID Register 2, offset: 0xBFC */
};

constexpr size_t    kSNVS_size = 0xC00;
constexpr uintptr_t kSNVS_base = 0x400D'4000;

namespace SNVS {

constexpr regs::RegGroup<SNVS_Layout, kSNVS_size, kSNVS_base> group;

template <auto Member, size_t Bits, unsigned int Shift,
          auto AssignMask = regs::shiftedMask32<Bits, Shift>(),
          bool WriteOnly = false>
using SNVS_Reg = regs::Reg32<kSNVS_base, SNVS_Layout, Member, 0, Bits, Shift,
                             AssignMask, 0, WriteOnly>;

template <auto Member, size_t MemberOffset, size_t Bits, unsigned int Shift,
          auto AssignMask = regs::shiftedMask32<Bits, Shift>()>
using SNVS_ArrayReg =
    regs::Reg32<kSNVS_base, SNVS_Layout, Member, MemberOffset, Bits, Shift,
                AssignMask>;

// SNVS_HP Lock Register
namespace HPLR {
constexpr SNVS_Reg<&SNVS_Layout::HPLR, 1, 18> HAC_L;      // High Assurance Counter Lock
    // When set, prevents any writes to HPHACIVR, HPHACR, and HAC_EN bit of HPCOMR
    // 0b0..Write access is allowed
    // 0b1..Write access is not allowed
constexpr SNVS_Reg<&SNVS_Layout::HPLR, 1, 17> HPSICR_L;   // HP Security Interrupt Control Register Lock
    // When set, prevents any writes to the HPSICR
    // 0b0..Write access is allowed
    // 0b1..Write access is not allowed
constexpr SNVS_Reg<&SNVS_Layout::HPLR, 1, 16> HPSVCR_L;   // HP Security Violation Control Register Lock
    // When set, prevents any writes to the HPSVCR
    // 0b0..Write access is allowed
    // 0b1..Write access is not allowed
constexpr SNVS_Reg<&SNVS_Layout::HPLR, 1,  9> MKS_SL;     // Master Key Select Soft Lock
    // When set, prevents any writes to the MASTER_KEY_SEL field of the LPMKCR
    // 0b0..Write access is allowed
    // 0b1..Write access is not allowed
constexpr SNVS_Reg<&SNVS_Layout::HPLR, 1,  8> LPSECR_SL;  // LP Security Events Configuration Register Soft Lock
    // When set, prevents any writes to the LPSECR
    // 0b0..Write access is allowed
    // 0b1..Write access is not allowed
constexpr SNVS_Reg<&SNVS_Layout::HPLR, 1,  6> LPSVCR_SL;  // LP Security Violation Control Register Soft Lock
    // When set, prevents any writes to the LPSVCR
    // 0b0..Write access is allowed
    // 0b1..Write access is not allowed
constexpr SNVS_Reg<&SNVS_Layout::HPLR, 1,  5> GPR_SL;     // General Purpose Register Soft Lock
    // When set, prevents any writes to the GPR
    // 0b0..Write access is allowed
    // 0b1..Write access is not allowed
constexpr SNVS_Reg<&SNVS_Layout::HPLR, 1,  4> MC_SL;      // Monotonic Counter Soft Lock
    // When set, prevents any writes (increments) to the MC Registers and MC_ENV bit
    // 0b0..Write access (increment) is allowed
    // 0b1..Write access (increment) is not allowed
constexpr SNVS_Reg<&SNVS_Layout::HPLR, 1,  3> LPCALB_SL;  // LP Calibration Soft Lock
    // When set, prevents any writes to the LP Calibration Value (LPCALB_VAL) and LP Calibration Enable (LPCALB_EN)
    // 0b0..Write access is allowed
    // 0b1..Write access is not allowed
constexpr SNVS_Reg<&SNVS_Layout::HPLR, 1,  2> SRTC_SL;    // Secure Real Time Counter Soft Lock
    // When set, prevents any writes to the SRTC Registers, SRTC_ENV, and SRTC_INV_EN bits
    // 0b0..Write access is allowed
    // 0b1..Write access is not allowed
constexpr SNVS_Reg<&SNVS_Layout::HPLR, 1,  1> ZMK_RSL;    // Zeroizable Master Key Read Soft Lock
    // When set, prevents any software reads to the ZMK Registers and ZMK_ECC_VALUE field of the LPMKCR
    // 0b0..Read access is allowed (only in software Programming mode)
    // 0b1..Read access is not allowed
constexpr SNVS_Reg<&SNVS_Layout::HPLR, 1,  0> ZMK_WSL;    // Zeroizable Master Key Write Soft Lock
    // When set, prevents any writes (software and hardware) to the ZMK registers and the ZMK_HWP, ZMK_VAL, and ZMK_ECC_EN fields of the LPMKCR
    // 0b0..Write access is allowed
    // 0b1..Write access is not allowed
}  // namespace HPLR

// SNVS_HP Command Register
// Exercise caution when setting or assigning fields in this register.
namespace HPCOMR {
// TODO: Is this the correct way?
constexpr uint32_t kWO = 0x0006'1011;

constexpr SNVS_Reg<&SNVS_Layout::HPCOMR, 1, 31, regs::shiftedMask32<1, 31>() | kWO> NPSWA_EN;      // Non-Privileged Software Access Enable
    // When set, allows non-privileged software to access all SNVS registers, including those that are privileged software read/write access only
constexpr SNVS_Reg<&SNVS_Layout::HPCOMR, 1, 19, regs::shiftedMask32<1, 19>() | kWO> HAC_STOP;      // High Assurance Counter Stop
    // This bit can be set only when SSM is in soft fail state
constexpr SNVS_Reg<&SNVS_Layout::HPCOMR, 1, 18, kWO, true> HAC_CLEAR;                              // High Assurance Counter Clear
    // When set, it clears the High Assurance Counter Register
    // 0b0..No Action
    // 0b1..Clear the HAC
constexpr SNVS_Reg<&SNVS_Layout::HPCOMR, 1, 17, kWO, true> HAC_LOAD;                               // High Assurance Counter Load
    // When set, it loads the High Assurance Counter Register with the value of the High Assurance Counter Load Register
    // 0b0..No Action
    // 0b1..Load the HAC
constexpr SNVS_Reg<&SNVS_Layout::HPCOMR, 1, 16, regs::shiftedMask32<1, 16>() | kWO> HAC_EN;        // High Assurance Counter Enable
    // This bit controls the SSM transition from the soft fail to the hard fail state
    // 0b0..High Assurance Counter is disabled
    // 0b1..High Assurance Counter is enable
constexpr SNVS_Reg<&SNVS_Layout::HPCOMR, 1, 13, regs::shiftedMask32<1, 13>() | kWO> MKS_EN;        // Master Key Select Enable
    // When not set, the one time programmable (OTP) master key is selected by default
    // 0b0..OTP master key is selected as an SNVS master key
    // 0b1..SNVS master key is selected according to the setting of the MASTER_KEY_SEL field of LPMKCR
constexpr SNVS_Reg<&SNVS_Layout::HPCOMR, 1, 12, kWO, true> PROG_ZMK;                               // Program Zeroizable Master Key
    // This bit activates ZMK hardware programming mechanism
    // 0b0..No Action
    // 0b1..Activate hardware key programming mechanism
constexpr SNVS_Reg<&SNVS_Layout::HPCOMR, 1, 10, regs::shiftedMask32<1, 10>() | kWO> SW_LPSV;       // LP Software Security Violation
    // When set, SNVS_LP treats this bit as a security violation
constexpr SNVS_Reg<&SNVS_Layout::HPCOMR, 1,  9, regs::shiftedMask32<1,  9>() | kWO> SW_FSV;        // Software Fatal Security Violation
    // When set, the system security monitor treats this bit as a fatal security violation
constexpr SNVS_Reg<&SNVS_Layout::HPCOMR, 1,  8, regs::shiftedMask32<1,  8>() | kWO> SW_SV;         // Software Security Violation
    // When set, the system security monitor treats this bit as a non-fatal security violation
constexpr SNVS_Reg<&SNVS_Layout::HPCOMR, 1,  5, regs::shiftedMask32<1,  5>() | kWO> LP_SWR_DIS;    // LP Software Reset Disable
    // When set, disables the LP software reset
    // 0b0..LP software reset is enabled
    // 0b1..LP software reset is disabled
constexpr SNVS_Reg<&SNVS_Layout::HPCOMR, 1,  4, kWO, true> LP_SWR;                                 // LP Software Reset
    // When set to 1, most registers in the SNVS_LP section are reset, but the following registers are not reset by an LP software reset: Monotonic Counter Secure Real Time Counter Time Alarm Register This bit cannot be set when the LP_SWR_DIS bit is set
    // 0b0..No Action
    // 0b1..Reset LP section
constexpr SNVS_Reg<&SNVS_Layout::HPCOMR, 1,  2, regs::shiftedMask32<1,  2>() | kWO> SSM_SFNS_DIS;  // SSM Soft Fail to Non-Secure State Transition Disable
    // When set, it disables the SSM transition from soft fail to non-secure state
    // 0b0..Soft Fail to Non-Secure State transition is enabled
    // 0b1..Soft Fail to Non-Secure State transition is disabled
constexpr SNVS_Reg<&SNVS_Layout::HPCOMR, 1,  1, regs::shiftedMask32<1,  1>() | kWO> SSM_ST_DIS;    // SSM Secure to Trusted State Transition Disable
    // When set, disables the SSM transition from secure to trusted state
    // 0b0..Secure to Trusted State transition is enabled
    // 0b1..Secure to Trusted State transition is disabled
constexpr SNVS_Reg<&SNVS_Layout::HPCOMR, 1,  0, kWO, true> SSM_ST;                                 // SSM State Transition
    // Transition state of the system security monitor
}  // namespace HPCOMR

// SNVS_HP Control Register
namespace HPCR {
constexpr SNVS_Reg<&SNVS_Layout::HPCR, 1, 27> BTN_MASK;    // Button interrupt mask
constexpr SNVS_Reg<&SNVS_Layout::HPCR, 3, 24> BTN_CONFIG;  // Button Configuration
constexpr SNVS_Reg<&SNVS_Layout::HPCR, 1, 16> HP_TS;       // HP Time Synchronize
    // 0b0..No Action
    // 0b1..Synchronize the HP Time Counter to the LP Time Counter
constexpr SNVS_Reg<&SNVS_Layout::HPCR, 5, 10> HPCALB_VAL;  // HP Calibration Value
    // Defines signed calibration value for the HP Real Time Counter
    // Signed 5-bit value: counts per each 32768 ticks of the counter:
    // 0b00000..+0 counts per each 32768 ticks of the counter
    // 0b00001..+1 counts per each 32768 ticks of the counter
    // 0b00010..+2 counts per each 32768 ticks of the counter
    // 0b01111..+15 counts per each 32768 ticks of the counter
    // 0b10000..-16 counts per each 32768 ticks of the counter
    // 0b10001..-15 counts per each 32768 ticks of the counter
    // 0b11110..-2 counts per each 32768 ticks of the counter
    // 0b11111..-1 counts per each 32768 ticks of the counter
constexpr SNVS_Reg<&SNVS_Layout::HPCR, 1,  8> HPCALB_EN;   // HP Real Time Counter Calibration Enabled
    // Indicates that the time calibration mechanism is enabled.
    // 0b0..HP Timer calibration disabled
    // 0b1..HP Timer calibration enabled
constexpr SNVS_Reg<&SNVS_Layout::HPCR, 4,  4> PI_FREQ;     // Periodic Interrupt Frequency
    // Defines frequency of the periodic interrupt
    // Bit 'value'  of the HPRTCLR is selected as a source of the periodic interrupt
constexpr SNVS_Reg<&SNVS_Layout::HPCR, 1,  3> PI_EN;       // HP Periodic Interrupt Enable
    // The periodic interrupt can be generated only if the HP Real Time Counter is enabled
    // 0b0..HP Periodic Interrupt is disabled
    // 0b1..HP Periodic Interrupt is enabled
constexpr SNVS_Reg<&SNVS_Layout::HPCR, 1,  2> DIS_PI;      // Disable periodic interrupt in the functional interrupt
    // 0b0..Periodic interrupt will trigger a functional interrupt
    // 0b1..Disable periodic interrupt in the function interrupt
constexpr SNVS_Reg<&SNVS_Layout::HPCR, 1,  1> HPTA_EN;     // HP Time Alarm Enable
    // When set, the time alarm interrupt is generated if the value in the HP Time Alarm Registers is equal to the value of the HP Real Time Counter
    // 0b0..HP Time Alarm Interrupt is disabled
    // 0b1..HP Time Alarm Interrupt is enabled
constexpr SNVS_Reg<&SNVS_Layout::HPCR, 1,  0> RTC_EN;      // HP Real Time Counter Enable
    // 0b0..RTC is disabled
    // 0b1..RTC is enabled
}  // namespace HPCR

// SNVS_HP Security Interrupt Control Register
namespace HPSICR {
constexpr SNVS_Reg<&SNVS_Layout::HPSICR, 1, 31> LPSVI_EN;  // LP Security Violation Interrupt Enable
    // This bit enables generating of the security interrupt to the host processor upon security violation signal from the LP section
    // 0b0..LP Security Violation Interrupt is Disabled
    // 0b1..LP Security Violation Interrupt is Enabled
constexpr SNVS_Reg<&SNVS_Layout::HPSICR, 1,  5> SV5_EN;    // Security Violation 5 Interrupt Enable
    // Setting this bit to 1 enables generation of the security interrupt to the host processor upon detection of the Security Violation 5 security violation
    // 0b0..Security Violation 5 Interrupt is Disabled
    // 0b1..Security Violation 5 Interrupt is Enabled
constexpr SNVS_Reg<&SNVS_Layout::HPSICR, 1,  4> SV4_EN;    // Security Violation 4 Interrupt Enable
    // Setting this bit to 1 enables generation of the security interrupt to the host processor upon detection of the Security Violation 4 security violation
    // 0b0..Security Violation 4 Interrupt is Disabled
    // 0b1..Security Violation 4 Interrupt is Enabled
constexpr SNVS_Reg<&SNVS_Layout::HPSICR, 1,  3> SV3_EN;    // Security Violation 3 Interrupt Enable
    // Setting this bit to 1 enables generation of the security interrupt to the host processor upon detection of the Security Violation 3 security violation
    // 0b0..Security Violation 3 Interrupt is Disabled
    // 0b1..Security Violation 3 Interrupt is Enabled
constexpr SNVS_Reg<&SNVS_Layout::HPSICR, 1,  2> SV2_EN;    // Security Violation 2 Interrupt Enable
    // Setting this bit to 1 enables generation of the security interrupt to the host processor upon detection of the Security Violation 2 security violation
    // 0b0..Security Violation 2 Interrupt is Disabled
    // 0b1..Security Violation 2 Interrupt is Enabled
constexpr SNVS_Reg<&SNVS_Layout::HPSICR, 1,  1> SV1_EN;    // Security Violation 1 Interrupt Enable
    // Setting this bit to 1 enables generation of the security interrupt to the host processor upon detection of the Security Violation 1 security violation
    // 0b0..Security Violation 1 Interrupt is Disabled
    // 0b1..Security Violation 1 Interrupt is Enabled
constexpr SNVS_Reg<&SNVS_Layout::HPSICR, 1,  0> SV0_EN;    // Security Violation 0 Interrupt Enable
    // Setting this bit to 1 enables generation of the security interrupt to the host processor upon detection of the Security Violation 0 security violation
    // 0b0..Security Violation 0 Interrupt is Disabled
    // 0b1..Security Violation 0 Interrupt is Enabled
}  // namespace HPSICR

// SNVS_HP Security Violation Control Register
namespace HPSVCR {
constexpr SNVS_Reg<&SNVS_Layout::HPSVCR, 2, 30> LPSV_CFG;  // LP Security Violation Configuration
    // This field configures the LP security violation source.
    // 0b00..LP security violation is disabled
    // 0b01..LP security violation is a non-fatal violation
    // 0b1x..LP security violation is a fatal violation
constexpr SNVS_Reg<&SNVS_Layout::HPSVCR, 2,  5> SV5_CFG;   // Security Violation 5 Security Violation Configuration
    // This field configures the Security Violation 5 Security Violation Input
    // 0b00..Security Violation 5 is disabled
    // 0b01..Security Violation 5 is a non-fatal violation
    // 0b1x..Security Violation 5 is a fatal violation
constexpr SNVS_Reg<&SNVS_Layout::HPSVCR, 1,  4> SV4_CFG;   // Security Violation 4 Security Violation Configuration
    // This field configures the Security Violation 4 Security Violation Input
    // 0b0..Security Violation 4 is a non-fatal violation
    // 0b1..Security Violation 4 is a fatal violation
constexpr SNVS_Reg<&SNVS_Layout::HPSVCR, 1,  3> SV3_CFG;   // Security Violation 3 Security Violation Configuration
    // This field configures the Security Violation 3 Security Violation Input
    // 0b0..Security Violation 3 is a non-fatal violation
    // 0b1..Security Violation 3 is a fatal violation
constexpr SNVS_Reg<&SNVS_Layout::HPSVCR, 1,  2> SV2_CFG;   // Security Violation 2 Security Violation Configuration
    // This field configures the Security Violation 2 Security Violation Input
    // 0b0..Security Violation 2 is a non-fatal violation
    // 0b1..Security Violation 2 is a fatal violation
constexpr SNVS_Reg<&SNVS_Layout::HPSVCR, 1,  1> SV1_CFG;   // Security Violation 1 Security Violation Configuration
    // This field configures the Security Violation 1 Security Violation Input
    // 0b0..Security Violation 1 is a non-fatal violation
    // 0b1..Security Violation 1 is a fatal violation
constexpr SNVS_Reg<&SNVS_Layout::HPSVCR, 1,  0> SV0_CFG;   // Security Violation 0 Security Violation Configuration
    // This field configures the Security Violation 0 Security Violation Input
    // 0b0..Security Violation 0 is a non-fatal violation
    // 0b1..Security Violation 0 is a fatal violation
}  // namespace HPSVCR

// SNVS_HP Status Register
namespace HPSR {
constexpr SNVS_Reg<regs::constify(&SNVS_Layout::HPSR), 1, 31> ZMK_ZERO;          // Zeroizable Master Key is Equal to Zero
    // 0b0..The ZMK is not zero.
    // 0b1..The ZMK is zero.
constexpr SNVS_Reg<regs::constify(&SNVS_Layout::HPSR), 1, 27> OTPMK_ZERO;        // One Time Programmable Master Key is Equal to Zero
    // 0b0..The OTPMK is not zero.
    // 0b1..The OTPMK is zero.
constexpr SNVS_Reg<regs::constify(&SNVS_Layout::HPSR), 9, 16> OTPMK_SYNDROME;    // One Time Programmable Master Key Syndrome
    // In the case of a single-bit error, the eight lower bits of this value indicate the bit number of error location
constexpr SNVS_Reg<regs::constify(&SNVS_Layout::HPSR), 1, 15> SYS_SECURE_BOOT;   // System Secure Boot
    // If SYS_SECURE_BOOT is 1, the chip boots from internal ROM
constexpr SNVS_Reg<regs::constify(&SNVS_Layout::HPSR), 3, 12> SYS_SECURITY_CFG;  // System Security Configuration
    // This field reflects the three security configuration inputs to SNVS
    // 0b000..Fab Configuration - the default configuration of newly fabricated chips
    // 0b001..Open Configuration - the configuration after NXP-programmable fuses have been blown
    // 0b011..Closed Configuration - the configuration after OEM-programmable fuses have been blown
    // 0b111..Field Return Configuration - the configuration of chips that are returned to NXP for analysis
constexpr SNVS_Reg<regs::constify(&SNVS_Layout::HPSR), 4,  8> SSM_STATE;         // System Security Monitor State
    // This field contains the encoded state of the SSM's state machine
    // 0b0000..Init
    // 0b0001..Hard Fail
    // 0b0011..Soft Fail
    // 0b1000..Init Intermediate (transition state between Init and Check - SSM stays in this state only one clock cycle)
    // 0b1001..Check
    // 0b1011..Non-Secure
    // 0b1101..Trusted
    // 0b1111..Secure
constexpr SNVS_Reg<&SNVS_Layout::HPSR, 1, 7, 0x0> BI;                            // Button Interrupt Signal ipi_snvs_btn_int_b was asserted.
constexpr SNVS_Reg<regs::constify(&SNVS_Layout::HPSR), 1,  6> BTN;               // Button Value of the BTN input
constexpr SNVS_Reg<regs::constify(&SNVS_Layout::HPSR), 1,  4> LPDIS;             // Low Power Disable
    // If 1, the low power section has been disabled by means of an input signal to SNVS
constexpr SNVS_Reg<&SNVS_Layout::HPSR, 1, 1, 0x0> PI;                            // Periodic Interrupt
    // Indicates that periodic interrupt has occurred since this bit was last cleared.
    // 0b0..No periodic interrupt occurred.
    // 0b1..A periodic interrupt occurred.
constexpr SNVS_Reg<&SNVS_Layout::HPSR, 1, 0, 0x0> HPTA;                          // HP Time Alarm
    // Indicates that the HP Time Alarm has occurred since this bit was last cleared.
    // 0b0..No time alarm interrupt occurred.
    // 0b1..A time alarm interrupt occurred.

// SNVS_HP Status Register values
constexpr uint32_t kSSM_STATE_Init               = 0;
constexpr uint32_t kSSM_STATE_HardFail           = 1;
constexpr uint32_t kSSM_STATE_SoftFail           = 3;
constexpr uint32_t kSSM_STATE_InitIntermerdiate  = 8;
constexpr uint32_t kSSM_STATE_Check              = 9;
constexpr uint32_t kSSM_STATE_Nonsecure          = 11;
constexpr uint32_t kSSM_STATE_Trusted            = 13;
constexpr uint32_t kSSM_STATE_Secure             = 15;
constexpr uint32_t kSYS_SECURITY_CFG_Fab         = 0;
constexpr uint32_t kSYS_SECURITY_CFG_Open        = 1;
constexpr uint32_t kSYS_SECURITY_CFG_Closed      = 3;
constexpr uint32_t kSYS_SECURITY_CFG_FieldReturn = 7;
}  // namespace HPSR

// SNVS_HP Security Violation Status Register
namespace HPSVSR {
constexpr SNVS_Reg<regs::constify(&SNVS_Layout::HPSVSR), 1, 31> LP_SEC_VIO;    // LP Security Violation
    // A security volation was detected in the SNVS low power section
constexpr SNVS_Reg<&SNVS_Layout::HPSVSR, 1, 27, 0x0> ZMK_ECC_FAIL;             // Zeroizable Master Key Error Correcting Code Check Failure
    // When set, this bit triggers a bad key violation to the SSM and a security violation to the SNVS_LP section, which clears security sensitive data
    // 0b0..ZMK ECC Failure was not detected.
    // 0b1..ZMK ECC Failure was detected.
constexpr SNVS_Reg<regs::constify(&SNVS_Layout::HPSVSR), 9, 16> ZMK_SYNDROME;  // Zeroizable Master Key Syndrome
    // The ZMK syndrome indicates the single-bit error location and parity for the ZMK register
constexpr SNVS_Reg<regs::constify(&SNVS_Layout::HPSVSR), 1, 15> SW_LPSV;       // LP Software Security Violation
    // This bit is a read-only copy of the SW_LPSV bit in the HP Command Register
constexpr SNVS_Reg<regs::constify(&SNVS_Layout::HPSVSR), 1, 14> SW_FSV;        // Software Fatal Security Violation
    // This bit is a read-only copy of the SW_FSV bit in the HP Command Register
constexpr SNVS_Reg<regs::constify(&SNVS_Layout::HPSVSR), 1, 13> SW_SV;         // Software Security Violation
    // This bit is a read-only copy of the SW_SV bit in the HP Command Register
constexpr SNVS_Reg<&SNVS_Layout::HPSVSR, 1,  5, 0x0> SV5;                      // Security Violation 5 security violation was detected.
    // 0b0..No Security Violation 5 security violation was detected.
    // 0b1..Security Violation 5 security violation was detected.
constexpr SNVS_Reg<&SNVS_Layout::HPSVSR, 1,  4, 0x0> SV4;                      // Security Violation 4 security violation was detected.
    // 0b0..No Security Violation 4 security violation was detected.
    // 0b1..Security Violation 4 security violation was detected.
constexpr SNVS_Reg<&SNVS_Layout::HPSVSR, 1,  3, 0x0> SV3;                      // Security Violation 3 security violation was detected.
    // 0b0..No Security Violation 3 security violation was detected.
    // 0b1..Security Violation 3 security violation was detected.
constexpr SNVS_Reg<&SNVS_Layout::HPSVSR, 1,  2, 0x0> SV2;                      // Security Violation 2 security violation was detected.
    // 0b0..No Security Violation 2 security violation was detected.
    // 0b1..Security Violation 2 security violation was detected.
constexpr SNVS_Reg<&SNVS_Layout::HPSVSR, 1,  1, 0x0> SV1;                      // Security Violation 1 security violation was detected.
    // 0b0..No Security Violation 1 security violation was detected.
    // 0b1..Security Violation 1 security violation was detected.
constexpr SNVS_Reg<&SNVS_Layout::HPSVSR, 1,  0, 0x0> SV0;                      // Security Violation 0 security violation was detected.
    // 0b0..No Security Violation 0 security violation was detected.
    // 0b1..Security Violation 0 security violation was detected.
}  // namespace HPSVSR

// SNVS_HP High Assurance Counter IV Register
namespace HPHACIVR {
constexpr SNVS_Reg<&SNVS_Layout::HPHACIVR, 32, 0> HAC_COUNTER_IV;  // High Assurance Counter Initial Value
    // This register is used to set the starting count value to the high
    // assurance counter
}  // namespace HPHACIVR

// SNVS_HP High Assurance Counter Register
namespace HPHACR {
constexpr SNVS_Reg<&SNVS_Layout::HPHACR, 32, 0> HAC_COUNTER;  // High Assurance Counter
    // When the HAC_EN bit is set and the SSM is in the soft fail state, this
    // counter starts to count down with the system clock
}  // namespace HPHACR

// SNVS_HP Real Time Counter MSB Register
namespace HPRTCMR {
constexpr SNVS_Reg<&SNVS_Layout::HPRTCMR, 15, 0> RTC;  // HP Real Time Counter
    // The most-significant 15 bits of the RTC
}  // namespace HPRTCMR

// SNVS_HP Real Time Counter LSB Register
namespace HPRTCLR {
constexpr SNVS_Reg<&SNVS_Layout::HPRTCLR, 32, 0> RTC;  // HP Real Time Counter least-significant 32 bits
}  // namespace HPRTCLR

// SNVS_HP Time Alarm MSB Register
namespace HPTAMR {
constexpr SNVS_Reg<&SNVS_Layout::HPTAMR, 15, 0> HPTA_MS;  // HP Time Alarm, most-significant 15 bits
}  // namespace HPTAMR

// SNVS_HP Time Alarm LSB Register
namespace HPTALR {
constexpr SNVS_Reg<&SNVS_Layout::HPTALR, 32, 0> HPTA_LS;  // HP Time Alarm, 32 least-significant bits
}  // namespace HPTALR

// SNVS_LP Lock Register
namespace LPLR {
constexpr SNVS_Reg<&SNVS_Layout::LPLR, 1, 9> MKS_HL;     // Master Key Select Hard Lock
    // When set, prevents any writes to the MASTER_KEY_SEL field of the LP Master Key Control Register
    // 0b0..Write access is allowed.
    // 0b1..Write access is not allowed.
constexpr SNVS_Reg<&SNVS_Layout::LPLR, 1, 8> LPSECR_HL;  // LP Security Events Configuration Register Hard Lock
    // When set, prevents any writes to the LPSECR
    // 0b0..Write access is allowed.
    // 0b1..Write access is not allowed.
constexpr SNVS_Reg<&SNVS_Layout::LPLR, 1, 6> LPSVCR_HL;  // LP Security Violation Control Register Hard Lock
    // When set, prevents any writes to the LPSVCR
    // 0b0..Write access is allowed.
    // 0b1..Write access is not allowed.
constexpr SNVS_Reg<&SNVS_Layout::LPLR, 1, 5> GPR_HL;     // General Purpose Register Hard Lock
    // When set, prevents any writes to the GPR
    // 0b0..Write access is allowed.
    // 0b1..Write access is not allowed.
constexpr SNVS_Reg<&SNVS_Layout::LPLR, 1, 4> MC_HL;      // Monotonic Counter Hard Lock
    // When set, prevents any writes (increments) to the MC Registers and MC_ENV bit
    // 0b0..Write access (increment) is allowed.
    // 0b1..Write access (increment) is not allowed.
constexpr SNVS_Reg<&SNVS_Layout::LPLR, 1, 3> LPCALB_HL;  // LP Calibration Hard Lock
    // When set, prevents any writes to the LP Calibration Value (LPCALB_VAL) and LP Calibration Enable (LPCALB_EN)
    // 0b0..Write access is allowed.
    // 0b1..Write access is not allowed.
constexpr SNVS_Reg<&SNVS_Layout::LPLR, 1, 2> SRTC_HL;    // Secure Real Time Counter Hard Lock
    // When set, prevents any writes to the SRTC registers, SRTC_ENV, and SRTC_INV_EN bits
    // 0b0..Write access is allowed.
    // 0b1..Write access is not allowed.
constexpr SNVS_Reg<&SNVS_Layout::LPLR, 1, 1> ZMK_RHL;    // Zeroizable Master Key Read Hard Lock
    // When set, prevents any software reads to the ZMK registers and ZMK_ECC_VALUE field of the LPMKCR
    // 0b0..Read access is allowed (only in software programming mode).
    // 0b1..Read access is not allowed.
constexpr SNVS_Reg<&SNVS_Layout::LPLR, 1, 0> ZMK_WHL;    // Zeroizable Master Key Write Hard Lock
    // When set, prevents any writes (software and hardware) to the ZMK registers and ZMK_HWP, ZMK_VAL, and ZMK_ECC_EN fields of the LPMKCR
    // 0b0..Write access is allowed.
    // 0b1..Write access is not allowed.
}  // namespace LPLR

// SNVS_LP Control Register
namespace LPCR {
constexpr SNVS_Reg<&SNVS_Layout::LPCR, 1, 24> GPR_Z_DIS;       // General Purpose Registers Zeroization Disable
constexpr SNVS_Reg<&SNVS_Layout::LPCR, 1, 23> PK_OVERRIDE;     // PMIC On Request Override
    // The value written to PK_OVERRIDE will be asserted on output signal snvs_lp_pk_override
constexpr SNVS_Reg<&SNVS_Layout::LPCR, 1, 22> PK_EN;           // PMIC On Request Enable
    // The value written to PK_EN will be asserted on output signal snvs_lp_pk_en
constexpr SNVS_Reg<&SNVS_Layout::LPCR, 2, 20> ON_TIME;         // The ON_TIME field is used to configure the period of time after BTN is asserted before pmic_en_b is asserted to turn on the SoC power
constexpr SNVS_Reg<&SNVS_Layout::LPCR, 2, 18> DEBOUNCE;        // This field configures the amount of debounce time for the BTN input signal
constexpr SNVS_Reg<&SNVS_Layout::LPCR, 2, 16> BTN_PRESS_TIME;  // This field configures the button press time out values for the PMIC Logic
constexpr SNVS_Reg<&SNVS_Layout::LPCR, 5, 10> LPCALB_VAL;      // LP Calibration Value
    // Defines signed calibration value for SRTC
    // Signed 5-bit value: counts per each 32768 ticks of the counter:
    // 0b00000..+0 counts per each 32768 ticks of the counter clock
    // 0b00001..+1 counts per each 32768 ticks of the counter clock
    // 0b00010..+2 counts per each 32768 ticks of the counter clock
    // 0b01111..+15 counts per each 32768 ticks of the counter clock
    // 0b10000..-16 counts per each 32768 ticks of the counter clock
    // 0b10001..-15 counts per each 32768 ticks of the counter clock
    // 0b11110..-2 counts per each 32768 ticks of the counter clock
    // 0b11111..-1 counts per each 32768 ticks of the counter clock
constexpr SNVS_Reg<&SNVS_Layout::LPCR, 1,  8> LPCALB_EN;       // LP Calibration Enable
    // When set, enables the SRTC calibration mechanism
    // 0b0..SRTC Time calibration is disabled.
    // 0b1..SRTC Time calibration is enabled.
constexpr SNVS_Reg<&SNVS_Layout::LPCR, 1,  7> LVD_EN;          // Digital Low-Voltage Event Enable
    // By default the detection of a low-voltage event does not cause the pmic_en_b signal to be asserted
constexpr SNVS_Reg<&SNVS_Layout::LPCR, 1,  6> TOP;             // Turn off System Power
    // Asserting this bit causes a signal to be sent to the Power Management IC to turn off the system power
    // 0b0..Leave system power on.
    // 0b1..Turn off system power.
constexpr SNVS_Reg<&SNVS_Layout::LPCR, 1,  5> DP_EN;           // Dumb PMIC Enabled
    // When set, software can control the system power
    // 0b0..Smart PMIC enabled.
    // 0b1..Dumb PMIC enabled.
constexpr SNVS_Reg<&SNVS_Layout::LPCR, 1,  4> SRTC_INV_EN;     // If this bit is 1, in the case of a security violation the SRTC stops counting and the SRTC is invalidated (SRTC_ENV bit is cleared)
    // 0b0..SRTC stays valid in the case of security violation (other than a software violation (HPSVSR[SW_LPSV] = 1 or HPCOMR[SW_LPSV] = 1)).
    // 0b1..SRTC is invalidated in the case of security violation.
constexpr SNVS_Reg<&SNVS_Layout::LPCR, 1,  3> LPWUI_EN;        // LP Wake-Up Interrupt Enable
    // This interrupt line should be connected to the external pin and is intended to inform the external chip about an SNVS_LP event (MC rollover, SRTC rollover, or time alarm )
constexpr SNVS_Reg<&SNVS_Layout::LPCR, 1,  2> MC_ENV;          // Monotonic Counter Enabled and Valid
    // When set, the MC can be incremented (by write transaction to the LPSMCMR or LPSMCLR)
    // 0b0..MC is disabled or invalid.
    // 0b1..MC is enabled and valid.
constexpr SNVS_Reg<&SNVS_Layout::LPCR, 1,  1> LPTA_EN;         // LP Time Alarm Enable
    // When set, the SNVS functional interrupt is asserted if the LP Time Alarm Register is equal to the 32 MSBs of the secure real time counter
    // 0b0..LP time alarm interrupt is disabled.
    // 0b1..LP time alarm interrupt is enabled.
constexpr SNVS_Reg<&SNVS_Layout::LPCR, 1,  0> SRTC_ENV;        // Secure Real Time Counter Enabled and Valid
    // When set, the SRTC becomes operational
    // 0b0..SRTC is disabled or invalid.
    // 0b1..SRTC is enabled and valid.
}  // namespace LPCR

// SNVS_LP Master Key Control Register
namespace LPMKCR {
constexpr SNVS_Reg<regs::constify(&SNVS_Layout::LPMKCR), 9, 7> ZMK_ECC_VALUE;  // Zeroizable Master Key Error Correcting Code Value
    // This field is automatically calculated and set when one is written into ZMK_ECC_EN bit of this register
constexpr SNVS_Reg<&SNVS_Layout::LPMKCR, 1, 4> ZMK_ECC_EN;                     // Zeroizable Master Key Error Correcting Code Check Enable
    // Writing one to this field automatically calculates and sets the ZMK ECC value in the ZMK_ECC_VALUE field of this register
    // 0b0..ZMK ECC check is disabled.
    // 0b1..ZMK ECC check is enabled.
constexpr SNVS_Reg<&SNVS_Layout::LPMKCR, 1, 3> ZMK_VAL;                        // Zeroizable Master Key Valid
    // When set, the ZMK value can be selected by the master key control block for use by cryptographic modules
    // 0b0..ZMK is not valid.
    // 0b1..ZMK is valid.
constexpr SNVS_Reg<&SNVS_Layout::LPMKCR, 1, 2> ZMK_HWP;                        // Zeroizable Master Key hardware Programming mode
    // When set, only the hardware key programming mechanism can set the ZMK and software cannot read it
    // 0b0..ZMK is in the software programming mode.
    // 0b1..ZMK is in the hardware programming mode.
constexpr SNVS_Reg<&SNVS_Layout::LPMKCR, 2, 0> MASTER_KEY_SEL;                 // Master Key Select
    // These bits select the SNVS Master Key output when Master Key Select bits are enabled by MKS_EN bit in the HPCOMR
    // 0b0x..Select one time programmable master key.
    // 0b10..Select zeroizable master key when MKS_EN bit is set .
    // 0b11..Select combined master key when MKS_EN bit is set .

// SNVS_LP Master Key Control Register values
constexpr uint32_t kMASTER_KEY_SEL_OTPMK = 0;  /*!< One Time Programmable Master Key. */
constexpr uint32_t kMASTER_KEY_SEL_ZMK   = 2;  /*!< Zeroizable Master Key. */
constexpr uint32_t kMASTER_KEY_SEL_CMK   = 3;  /*!< Combined Master Key, it is XOR of OPTMK and ZMK. */
constexpr uint32_t kZMK_HWP_Software     = 0;
constexpr uint32_t kZMK_HWP_Hardware     = 1;
}  // namespace LPMKCR

// SNVS_LP Security Violation Control Register
namespace LPSVCR {
constexpr SNVS_Reg<&SNVS_Layout::LPSVCR, 1, 5> SV5_EN;  // Security Violation 5 Enable
    // This bit enables Security Violation 5 Input
    // 0b0..Security Violation 5 is disabled in the LP domain.
    // 0b1..Security Violation 5 is enabled in the LP domain.
constexpr SNVS_Reg<&SNVS_Layout::LPSVCR, 1, 4> SV4_EN;  // Security Violation 4 Enable
    // This bit enables Security Violation 4 Input
    // 0b0..Security Violation 4 is disabled in the LP domain.
    // 0b1..Security Violation 4 is enabled in the LP domain.
constexpr SNVS_Reg<&SNVS_Layout::LPSVCR, 1, 3> SV3_EN;  // Security Violation 3 Enable
    // This bit enables Security Violation 3 Input
    // 0b0..Security Violation 3 is disabled in the LP domain.
    // 0b1..Security Violation 3 is enabled in the LP domain.
constexpr SNVS_Reg<&SNVS_Layout::LPSVCR, 1, 2> SV2_EN;  // Security Violation 2 Enable
    // This bit enables Security Violation 2 Input
    // 0b0..Security Violation 2 is disabled in the LP domain.
    // 0b1..Security Violation 2 is enabled in the LP domain.
constexpr SNVS_Reg<&SNVS_Layout::LPSVCR, 1, 1> SV1_EN;  // Security Violation 1 Enable
    // This bit enables Security Violation 1 Input
    // 0b0..Security Violation 1 is disabled in the LP domain.
    // 0b1..Security Violation 1 is enabled in the LP domain.
constexpr SNVS_Reg<&SNVS_Layout::LPSVCR, 1, 0> SV0_EN;  // Security Violation 0 Enable
    // This bit enables Security Violation 0 Input
    // 0b0..Security Violation 0 is disabled in the LP domain.
    // 0b1..Security Violation 0 is enabled in the LP domain.
}  // namespace LPSVCR

// SNVS_LP Security Events Configuration Register
namespace LPSECR {
constexpr SNVS_Reg<&SNVS_Layout::LPSECR, 1, 28> OSCB;        // Oscillator Bypass
    // When OSCB=1 the osc_bypass signal is asserted
    // 0b0..Normal SRTC clock oscillator not bypassed.
    // 0b1..Normal SRTC clock oscillator bypassed. Alternate clock can drive the SRTC clock source.
constexpr SNVS_Reg<&SNVS_Layout::LPSECR, 3, 24> VRC;         // Voltage Reference Configuration
    // These configuration bits are wired as an output of the module.
constexpr SNVS_Reg<&SNVS_Layout::LPSECR, 3, 20> HTDC;        // High Temperature Detect Configuration
    // These configuration bits are wired as an output of the module
constexpr SNVS_Reg<&SNVS_Layout::LPSECR, 3, 16> LTDC;        // Low Temp Detect Configuration
    // These configuration bits are wired as an output of the module.
constexpr SNVS_Reg<&SNVS_Layout::LPSECR, 1, 15> POR_OBSERV;  // Power On Reset (POR) Observability Flop
    // The asynchronous reset input of this flop is connected directly to the output of the POR analog circuitry (external to the SNVS
constexpr SNVS_Reg<&SNVS_Layout::LPSECR, 1, 14> PFD_OBSERV;  // System Power Fail Detector (PFD) Observability Flop
    // The asynchronous reset input of this flop is connected directly to the inverted output of the PFD analog circuitry (external to the SNVS block)
constexpr SNVS_Reg<&SNVS_Layout::LPSECR, 1,  2> MCR_EN;      // MC Rollover Enable
    // When set, an MC Rollover event generates an LP security violation.
    // 0b0..MC rollover is disabled.
    // 0b1..MC rollover is enabled.
constexpr SNVS_Reg<&SNVS_Layout::LPSECR, 1,  1> SRTCR_EN;    // SRTC Rollover Enable
    // When set, an SRTC rollover event generates an LP security violation.
    // 0b0..SRTC rollover is disabled.
    // 0b1..SRTC rollover is enabled.
}  // namespace LPSECR

// SNVS_LP Status Register
namespace LPSR {
constexpr SNVS_Reg<regs::constify(&SNVS_Layout::LPSR), 1, 31> LPS;   // LP Section is Secured
    // Indicates that the LP section is provisioned/programmed in the secure or trusted state
    // 0b0..LP section was not programmed in secure or trusted state.
    // 0b1..LP section was programmed in secure or trusted state.
constexpr SNVS_Reg<regs::constify(&SNVS_Layout::LPSR), 1, 30> LPNS;  // LP Section is Non-Secured
    // Indicates that LP section was provisioned/programmed in the non-secure state
    // 0b0..LP section was not programmed in the non-secure state.
    // 0b1..LP section was programmed in the non-secure state.
constexpr SNVS_Reg<&SNVS_Layout::LPSR, 1, 19, 0x0> SPON;             // Set Power On
    // The SPON bit is set when the set_pwr_on_irq interrupt is triggered, which happens when the power button is pressed longer than the configured debounce time
    // 0b0..Set Power On Interrupt was not detected.
    // 0b1..Set Power On Interrupt was detected.
constexpr SNVS_Reg<&SNVS_Layout::LPSR, 1, 18, 0x0> SPOF;             // Set Power Off
    // The SPO bit is set when the power button is pressed longer than the configured debounce time
    // 0b0..Set Power Off was not detected.
    // 0b1..Set Power Off was detected.
constexpr SNVS_Reg<&SNVS_Layout::LPSR, 1, 17, 0x0> EO;               // Emergency Off
    // This bit is set when a power off is requested.
    // 0b0..Emergency off was not detected.
    // 0b1..Emergency off was detected.
constexpr SNVS_Reg<&SNVS_Layout::LPSR, 1, 16, 0x0> ESVD;             // External Security Violation Detected
    // Indicates that a security violation is detected on one of the HP security violation ports
    // 0b0..No external security violation.
    // 0b1..External security violation is detected.
constexpr SNVS_Reg<&SNVS_Layout::LPSR, 1,  3, 0x0> LVD;              // Digital Low Voltage Event Detected
    // 0b0..No low voltage event detected.
    // 0b1..Low voltage event is detected.
constexpr SNVS_Reg<&SNVS_Layout::LPSR, 1,  2, 0x0> MCR;              // Monotonic Counter Rollover
    // 0b0..MC has not reached its maximum value.
    // 0b1..MC has reached its maximum value.
constexpr SNVS_Reg<&SNVS_Layout::LPSR, 1,  1, 0x0> SRTCR;            // Secure Real Time Counter Rollover
    // 0b0..SRTC has not reached its maximum value.
    // 0b1..SRTC has reached its maximum value.
constexpr SNVS_Reg<&SNVS_Layout::LPSR, 1,  0, 0x0> LPTA;             // LP Time Alarm
    // 0b0..No time alarm interrupt occurred.
    // 0b1..A time alarm interrupt occurred.
}  // namespace LPSR

// SNVS_LP Secure Real Time Counter MSB Register
namespace LPSRTCMR {
constexpr SNVS_Reg<&SNVS_Layout::LPSRTCMR, 15, 0> SRTC;  // LP Secure Real Time Counter
    // The most-significant 15 bits of the SRTC
}  // namespace LPSRTCMR

// SNVS_LP Secure Real Time Counter LSB Register
namespace LPSRTCLR {
constexpr SNVS_Reg<&SNVS_Layout::LPSRTCLR, 32, 0> SRTC;  // LP Secure Real Time Counter least-significant 32 bits
    // This register can be programmed only when SRTC is not active and not
    // locked, meaning the SRTC_ENV, SRTC_SL, and SRTC_HL bits are not set
}  // namespace LPSRTCLR

// SNVS_LP Time Alarm Register
namespace LPTAR {
constexpr SNVS_Reg<&SNVS_Layout::LPTAR, 32, 0> LPTA;  // LP Time Alarm
    // This register can be programmed only when the LP time alarm is disabled
    // (LPTA_EN bit is not set)
}  // namespace LPTAR

// SNVS_LP Secure Monotonic Counter MSB Register
namespace LPSMCMR {
constexpr SNVS_Reg<regs::constify(&SNVS_Layout::LPSMCMR), 16, 16> MC_ERA_BITS;  // Monotonic Counter Era Bits
    // These bits are inputs to the module and typically connect to fuses
constexpr SNVS_Reg<&SNVS_Layout::LPSMCMR, 16,  0> MON_COUNTER;                  // Monotonic Counter most-significant 16 Bits
    // Note that writing to this register does not change the value of this field to the value that was written
}  // namespace LPSMCMR

// SNVS_LP Secure Monotonic Counter LSB Register
namespace LPSMCLR {
constexpr SNVS_Reg<&SNVS_Layout::LPSMCLR, 32, 0> MON_COUNTER;  // Monotonic Counter bits
    // Note that writing to this register does not change the value of this
    // field to the value that was written
}  // namespace LPSMCLR

// SNVS_LP Digital Low-Voltage Detector Register
namespace LPLVDR {
constexpr SNVS_Reg<&SNVS_Layout::LPLVDR, 32, 0> LVD;  // Low-Voltage Detector Value
}  // namespace LPLVDR

// SNVS_LP General Purpose Register 0 (legacy alias)
namespace LPGPR0_LEGACY_ALIAS {
constexpr SNVS_Reg<&SNVS_Layout::LPGPR0_LEGACY_ALIAS, 32, 0> GPR;  // General Purpose Register
    // When GPR_SL or GPR_HL bit is set, the register cannot be programmed.
}  // namespace LPGPR0_LEGACY_ALIAS

// SNVS_LP Zeroizable Master Key Register
namespace LPZMKR {
template <size_t Index>
constexpr SNVS_ArrayReg<&SNVS_Layout::LPZMKR, Index, 32, 0> ZMK;  // Zeroizable Master Key
    // Each of these registers contains 32 bits of the 256-bit ZMK value

namespace vals {
constexpr regs::RegValue32<32, 0> ZMK;
}  // namespace vals
}  // namespace LPZMKR

// SNVS_LP General Purpose Registers 0 .. 3
namespace LPGPR_ALIAS {
template <size_t Index>
constexpr SNVS_ArrayReg<&SNVS_Layout::LPGPR_ALIAS, Index, 32, 0> GPR;  // General Purpose Register
    // When GPR_SL or GPR_HL bit is set, the register cannot be programmed.

namespace vals {
constexpr regs::RegValue32<32, 0> GPR;
}  // namespace vals
}  // namespace LPGPR_ALIAS

// SNVS_LP General Purpose Registers 0 .. 7
namespace LPGPR {
template <size_t Index>
constexpr SNVS_ArrayReg<&SNVS_Layout::LPGPR, Index, 32, 0> GPR;  // General Purpose Register
    // When GPR_SL or GPR_HL bit is set, the register cannot be programmed.

namespace vals {
constexpr regs::RegValue32<32, 0> GPR;
}  // namespace vals
}  // namespace LPGPR

// SNVS_HP Version ID Register 1
namespace HPVIDR1 {
constexpr SNVS_Reg<&SNVS_Layout::HPVIDR1, 16, 16> IP_ID;      // SNVS block ID
constexpr SNVS_Reg<&SNVS_Layout::HPVIDR1,  8,  8> MAJOR_REV;  // SNVS block major version number
constexpr SNVS_Reg<&SNVS_Layout::HPVIDR1,  8,  0> MINOR_REV;  // SNVS block minor version number
}  // namespace HPVIDR1

// SNVS_HP Version ID Register 2
namespace HPVIDR2 {
constexpr SNVS_Reg<&SNVS_Layout::HPVIDR2, 8, 24> IP_ERA;      // IP Era 00h - Era 1 or 2 03h - Era 3 04h - Era 4 05h - Era 5 06h - Era 6
constexpr SNVS_Reg<&SNVS_Layout::HPVIDR2, 8, 16> INTG_OPT;    // SNVS Integration Options
constexpr SNVS_Reg<&SNVS_Layout::HPVIDR2, 8,  8> ECO_REV;     // SNVS ECO Revision
constexpr SNVS_Reg<&SNVS_Layout::HPVIDR2, 8,  0> CONFIG_OPT;  // SNVS Configuration Options
}  // namespace HPVIDR2

}  // namespace SNVS

}  // namespace imxrt1060
}  // namespace hardware
}  // namespace qindesign
