// SPDX-FileCopyrightText: (c) 2026 Shawn Silverman <shawn@pobox.com>
// SPDX-License-Identifier: AGPL-3.0-or-later

// CCM.h defines all the CCM registers.
// This file is part of the imxrt1060-regs library.

#pragma once

#include <cstddef>
#include <cstdint>

#include "qnethernet/hardware/regs/regs.h"

namespace qindesign {
namespace hardware {
namespace imxrt1060 {

// Structure type to access the Clock Controller Module (CCM) registers.
//
// Comments are from BSD-3-licensed NXP SDK.
//
// See:
// * https://github.com/nxp-mcuxpresso/mcux-devices-rt/blob/main/RT1060/periph/PERI_CCM.h
// * https://github.com/nxp-mcuxpresso/legacy-mcux-sdk/blob/main/devices/MIMXRT1062/MIMXRT1062.h
struct CCM_Layout {
  volatile uint32_t CCR;                               /**< CCM Control Register, offset: 0x0 */
  uint32_t HARDWARE_REGS_LAYOUT_MEMBER_RESERVED[1];
  const volatile uint32_t CSR;                         /**< CCM Status Register, offset: 0x8 */
  volatile uint32_t CCSR;                              /**< CCM Clock Switcher Register, offset: 0xC */
  volatile uint32_t CACRR;                             /**< CCM Arm Clock Root Register, offset: 0x10 */
  volatile uint32_t CBCDR;                             /**< CCM Bus Clock Divider Register, offset: 0x14 */
  volatile uint32_t CBCMR;                             /**< CCM Bus Clock Multiplexer Register, offset: 0x18 */
  volatile uint32_t CSCMR1;                            /**< CCM Serial Clock Multiplexer Register 1, offset: 0x1C */
  volatile uint32_t CSCMR2;                            /**< CCM Serial Clock Multiplexer Register 2, offset: 0x20 */
  volatile uint32_t CSCDR1;                            /**< CCM Serial Clock Divider Register 1, offset: 0x24 */
  volatile uint32_t CS1CDR;                            /**< CCM Clock Divider Register, offset: 0x28 */
  volatile uint32_t CS2CDR;                            /**< CCM Clock Divider Register, offset: 0x2C */
  volatile uint32_t CDCDR;                             /**< CCM D1 Clock Divider Register, offset: 0x30 */
  uint32_t HARDWARE_REGS_LAYOUT_MEMBER_RESERVED[1];
  volatile uint32_t CSCDR2;                            /**< CCM Serial Clock Divider Register 2, offset: 0x38 */
  volatile uint32_t CSCDR3;                            /**< CCM Serial Clock Divider Register 3, offset: 0x3C */
  uint32_t HARDWARE_REGS_LAYOUT_MEMBER_RESERVED[2];
  const volatile uint32_t CDHIPR;                      /**< CCM Divider Handshake In-Process Register, offset: 0x48 */
  uint32_t HARDWARE_REGS_LAYOUT_MEMBER_RESERVED[2];
  volatile uint32_t CLPCR;                             /**< CCM Low Power Control Register, offset: 0x54 */
  volatile uint32_t CISR;                              /**< CCM Interrupt Status Register, offset: 0x58 */
  volatile uint32_t CIMR;                              /**< CCM Interrupt Mask Register, offset: 0x5C */
  volatile uint32_t CCOSR;                             /**< CCM Clock Output Source Register, offset: 0x60 */
  volatile uint32_t CGPR;                              /**< CCM General Purpose Register, offset: 0x64 */
  volatile uint32_t CCGR0;                             /**< CCM Clock Gating Register 0, offset: 0x68 */
  volatile uint32_t CCGR1;                             /**< CCM Clock Gating Register 1, offset: 0x6C */
  volatile uint32_t CCGR2;                             /**< CCM Clock Gating Register 2, offset: 0x70 */
  volatile uint32_t CCGR3;                             /**< CCM Clock Gating Register 3, offset: 0x74 */
  volatile uint32_t CCGR4;                             /**< CCM Clock Gating Register 4, offset: 0x78 */
  volatile uint32_t CCGR5;                             /**< CCM Clock Gating Register 5, offset: 0x7C */
  volatile uint32_t CCGR6;                             /**< CCM Clock Gating Register 6, offset: 0x80 */
  volatile uint32_t CCGR7;                             /**< CCM Clock Gating Register 7, offset: 0x84 */
  volatile uint32_t CMEOR;                             /**< CCM Module Enable Overide Register, offset: 0x88 */
};

constexpr size_t    kCCM_size = 0x8C;
constexpr uintptr_t kCCM_base = 0x400F'C000;

namespace CCM {

constexpr regs::RegGroup<CCM_Layout, kCCM_size, kCCM_base> group;

template <auto Member, size_t Bits, unsigned int Shift,
          auto AssignMask = regs::shiftedMask32<Bits, Shift>()>
using CCM_Reg =
    regs::Reg32<kCCM_base, CCM_Layout, Member, 0, Bits, Shift, AssignMask>;

template <auto Member, size_t Bits, unsigned int Shift, uint32_t AssignSet>
using CGPR_Reg =
    regs::Reg32<kCCM_base, CCM_Layout, Member, 0, Bits, Shift,
                regs::shiftedMask32<Bits, Shift>(), AssignSet>;

// CCM Control Register
namespace CCR {
constexpr CCM_Reg<&CCM_Layout::CCR, 1, 27> RBC_EN;            // Enable for REG_BYPASS_COUNTER
    // 0b0..REG_BYPASS_COUNTER disabled
    // 0b1..REG_BYPASS_COUNTER enabled.
constexpr CCM_Reg<&CCM_Layout::CCR, 6, 21> REG_BYPASS_COUNT;  // Counter for analog_reg_bypass signal assertion after standby voltage request by PMIC_STBY_REQ
    // 'value' CKIL clock period delay:
    // 0b000000..no delay
    // 0b000001..1 CKIL clock period delay
    // 0b111111..63 CKIL clock periods delay
constexpr CCM_Reg<&CCM_Layout::CCR, 1, 12> COSC_EN;           // On chip oscillator enable bit - this bit value is reflected on the output cosc_en
    // 0b0..disable on chip oscillator
    // 0b1..enable on chip oscillator
constexpr CCM_Reg<&CCM_Layout::CCR, 8,  0> OSCNT;             // Oscillator ready counter value.
    // These bits define value of 32KHz counter, that serve as
    // counter for oscillator lock time (count to n+1 ckil's). This is used for oscillator lock time.
    // Current estimation is ~5ms. This counter will be used in ignition sequence and in wake from
    // stop sequence if sbyos bit was defined, to notify that on chip oscillator output is ready for
    // the dpll_ip to use and only then the gate in dpll_ip can be opened.
}  // namespace CCR

// CCM Status Register
namespace CSR {
constexpr CCM_Reg<&CCM_Layout::CSR, 1, 5> COSC_READY;   // Status indication of on board oscillator
    // 0b0..on board oscillator is not ready.
    // 0b1..on board oscillator is ready.
constexpr CCM_Reg<&CCM_Layout::CSR, 1, 3> CAMP2_READY;  // Status indication of CAMP2.
    // 0b0..CAMP2 is not ready.
    // 0b1..CAMP2 is ready.
constexpr CCM_Reg<&CCM_Layout::CSR, 1, 0> REF_EN_B;     // Status of the value of CCM_REF_EN_B output of ccm
    // 0b0..value of CCM_REF_EN_B is '0'
    // 0b1..value of CCM_REF_EN_B is '1'
}  // namespace CSR

// CCM Clock Switcher Register
namespace CCSR {
constexpr CCM_Reg<&CCM_Layout::CCSR, 1, 0> PLL3_SW_CLK_SEL;  // Selects source to generate pll3_sw_clk.
    // This bit should only be used for testing purposes.
    // 0b0..pll3_main_clk
    // 0b1..pll3 bypass clock

// CCM Clock Switcher Register values
constexpr uint32_t kPLL3_SW_CLK_SEL_pll3_main_clk   = 0;
constexpr uint32_t kPLL3_SW_CLK_SEL_pll3_bypass_clk = 1;
}  // namespace CCSR

// CCM Arm Clock Root Register
namespace CACRR {
constexpr CCM_Reg<&CCM_Layout::CACRR, 3, 0> ARM_PODF;  // Divider for Arm clock root
    // Divide by 'value' + 1:
    // 0b000..divide by 1
    // 0b001..divide by 2
    // 0b010..divide by 3
    // 0b011..divide by 4
    // 0b100..divide by 5
    // 0b101..divide by 6
    // 0b110..divide by 7
    // 0b111..divide by 8
}  // namespace CACRR

// CCM Bus Clock Divider Register
namespace CBCDR {
constexpr CCM_Reg<&CCM_Layout::CBCDR, 3, 27> PERIPH_CLK2_PODF;  // Divider for periph_clk2_podf.
    // Divide by 'value' + 1:
    // 0b000..divide by 1
    // 0b001..divide by 2
    // 0b010..divide by 3
    // 0b011..divide by 4
    // 0b100..divide by 5
    // 0b101..divide by 6
    // 0b110..divide by 7
    // 0b111..divide by 8
constexpr CCM_Reg<&CCM_Layout::CBCDR, 1, 25> PERIPH_CLK_SEL;    // Selector for peripheral main clock
    // 0b0..derive clock from pre_periph_clk_sel
    // 0b1..derive clock from periph_clk2_clk_divided
constexpr CCM_Reg<&CCM_Layout::CBCDR, 3, 16> SEMC_PODF;         // Post divider for SEMC clock
    // Divide by 'value' + 1:
    // 0b000..divide by 1
    // 0b001..divide by 2
    // 0b010..divide by 3
    // 0b011..divide by 4
    // 0b100..divide by 5
    // 0b101..divide by 6
    // 0b110..divide by 7
    // 0b111..divide by 8
constexpr CCM_Reg<&CCM_Layout::CBCDR, 3, 10> AHB_PODF;          // Divider for AHB PODF
    // Divide by 'value' + 1:
    // 0b000..divide by 1
    // 0b001..divide by 2
    // 0b010..divide by 3
    // 0b011..divide by 4
    // 0b100..divide by 5
    // 0b101..divide by 6
    // 0b110..divide by 7
    // 0b111..divide by 8
constexpr CCM_Reg<&CCM_Layout::CBCDR, 2,  8> IPG_PODF;          // Divider for ipg podf.
    // Divide by 'value' + 1:
    // 0b00..divide by 1
    // 0b01..divide by 2
    // 0b10..divide by 3
    // 0b11..divide by 4
constexpr CCM_Reg<&CCM_Layout::CBCDR, 1,  7> SEMC_ALT_CLK_SEL;  // SEMC alternative clock select
    // 0b0..PLL2 PFD2 will be selected as alternative clock for SEMC root clock
    // 0b1..PLL3 PFD1 will be selected as alternative clock for SEMC root clock
constexpr CCM_Reg<&CCM_Layout::CBCDR, 1,  6> SEMC_CLK_SEL;      // SEMC clock source select
    // 0b0..Periph_clk output will be used as SEMC clock root
    // 0b1..SEMC alternative clock will be used as SEMC clock root

// CCM Bus Clock Divider Register values
constexpr uint32_t kSEMC_CLK_SEL_Periph_clk                = 0;
constexpr uint32_t kSEMC_CLK_SEL_Alt                       = 1;
constexpr uint32_t kSEMC_ALT_CLK_SEL_PLL2_PFD2             = 0;
constexpr uint32_t kSEMC_ALT_CLK_SEL_PLL3_PFD1             = 1;
constexpr uint32_t kPERIPH_CLK_SEL_pre_periph_clk_sel      = 0;
constexpr uint32_t kPERIPH_CLK_SEL_periph_clk2_clk_divided = 1;
}  // namespace CBCDR

// CCM Bus Clock Multiplexer Register
namespace CBCMR {
constexpr CCM_Reg<&CCM_Layout::CBCMR, 3, 29> FLEXSPI2_PODF;       // Divider for flexspi2 clock root.
    // Divide by 'value' + 1:
    // 0b000..divide by 1
    // 0b001..divide by 2
    // 0b010..divide by 3
    // 0b011..divide by 4
    // 0b100..divide by 5
    // 0b101..divide by 6
    // 0b110..divide by 7
    // 0b111..divide by 8
constexpr CCM_Reg<&CCM_Layout::CBCMR, 3, 26> LPSPI_PODF;          // Divider for LPSPI.
    // Divider should be updated when output clock is gated.
    // Divide by 'value' + 1:
    // 0b000..divide by 1
    // 0b001..divide by 2
    // 0b010..divide by 3
    // 0b011..divide by 4
    // 0b100..divide by 5
    // 0b101..divide by 6
    // 0b110..divide by 7
    // 0b111..divide by 8
constexpr CCM_Reg<&CCM_Layout::CBCMR, 3, 23> LCDIF_PODF;          // Post-divider for LCDIF clock.
    // Divide by 'value' + 1:
    // 0b000..divide by 1
    // 0b001..divide by 2
    // 0b010..divide by 3
    // 0b011..divide by 4
    // 0b100..divide by 5
    // 0b101..divide by 6
    // 0b110..divide by 7
    // 0b111..divide by 8
constexpr CCM_Reg<&CCM_Layout::CBCMR, 2, 18> PRE_PERIPH_CLK_SEL;  // Selector for pre_periph clock multiplexer
    // 0b00..derive clock from PLL2
    // 0b01..derive clock from PLL2 PFD2
    // 0b10..derive clock from PLL2 PFD0
    // 0b11..derive clock from divided PLL1
constexpr CCM_Reg<&CCM_Layout::CBCMR, 2, 14> TRACE_CLK_SEL;       // Selector for Trace clock multiplexer
    // 0b00..derive clock from PLL2
    // 0b01..derive clock from PLL2 PFD2
    // 0b10..derive clock from PLL2 PFD0
    // 0b11..derive clock from PLL2 PFD1
constexpr CCM_Reg<&CCM_Layout::CBCMR, 2, 12> PERIPH_CLK2_SEL;     // Selector for peripheral clk2 clock multiplexer
    // 0b00..derive clock from pll3_sw_clk
    // 0b01..derive clock from osc_clk (pll1_ref_clk)
    // 0b10..derive clock from pll2_bypass_clk
    // 0b11..reserved
constexpr CCM_Reg<&CCM_Layout::CBCMR, 2,  8> FLEXSPI2_CLK_SEL;    // Selector for flexspi2 clock multiplexer
    // 0b00..derive clock from PLL2 PFD2
    // 0b01..derive clock from PLL3 PFD0
    // 0b10..derive clock from PLL3 PFD1
    // 0b11..derive clock from PLL2 (pll2_main_clk)
constexpr CCM_Reg<&CCM_Layout::CBCMR, 2,  4> LPSPI_CLK_SEL;       // Selector for lpspi clock multiplexer
    // 0b00..derive clock from PLL3 PFD1 clk
    // 0b01..derive clock from PLL3 PFD0
    // 0b10..derive clock from PLL2
    // 0b11..derive clock from PLL2 PFD2
}  // namespace CBCMR

// CCM Serial Clock Multiplexer Register 1
namespace CSCMR1 {
constexpr CCM_Reg<&CCM_Layout::CSCMR1, 2, 29> FLEXSPI_CLK_SEL;  // Selector for flexspi clock multiplexer
    // 0b00..derive clock from semc_clk_root_pre
    // 0b01..derive clock from pll3_sw_clk
    // 0b10..derive clock from PLL2 PFD2
    // 0b11..derive clock from PLL3 PFD0
constexpr CCM_Reg<&CCM_Layout::CSCMR1, 3, 23> FLEXSPI_PODF;     // Divider for flexspi clock root.
    // Divide by 'value' + 1:
    // 0b000..divide by 1
    // 0b001..divide by 2
    // 0b010..divide by 3
    // 0b011..divide by 4
    // 0b100..divide by 5
    // 0b101..divide by 6
    // 0b110..divide by 7
    // 0b111..divide by 8
constexpr CCM_Reg<&CCM_Layout::CSCMR1, 1, 17> USDHC2_CLK_SEL;   // Selector for usdhc2 clock multiplexer
    // 0b0..derive clock from PLL2 PFD2
    // 0b1..derive clock from PLL2 PFD0
constexpr CCM_Reg<&CCM_Layout::CSCMR1, 1, 16> USDHC1_CLK_SEL;   // Selector for usdhc1 clock multiplexer
    // 0b0..derive clock from PLL2 PFD2
    // 0b1..derive clock from PLL2 PFD0
constexpr CCM_Reg<&CCM_Layout::CSCMR1, 2, 14> SAI3_CLK_SEL;     // Selector for sai3/adc1/adc2 clock multiplexer
    // 0b00..derive clock from PLL3 PFD2
    // 0b01..derive clock from PLL5
    // 0b10..derive clock from PLL4
    // 0b11..Reserved
constexpr CCM_Reg<&CCM_Layout::CSCMR1, 2, 12> SAI2_CLK_SEL;     // Selector for sai2 clock multiplexer
    // 0b00..derive clock from PLL3 PFD2
    // 0b01..derive clock from PLL5
    // 0b10..derive clock from PLL4
    // 0b11..Reserved
constexpr CCM_Reg<&CCM_Layout::CSCMR1, 2, 10> SAI1_CLK_SEL;     // Selector for sai1 clock multiplexer
    // 0b00..derive clock from PLL3 PFD2
    // 0b01..derive clock from PLL5
    // 0b10..derive clock from PLL4
    // 0b11..Reserved
constexpr CCM_Reg<&CCM_Layout::CSCMR1, 1,  6> PERCLK_CLK_SEL;   // Selector for the perclk clock multiplexor
    // 0b0..derive clock from ipg clk root
    // 0b1..derive clock from osc_clk
constexpr CCM_Reg<&CCM_Layout::CSCMR1, 6,  0> PERCLK_PODF;      // Divider for perclk podf.
    // Divide by 'value' + 1
}  // namespace CSCMR1

// CCM Serial Clock Multiplexer Register 2
namespace CSCMR2 {
constexpr CCM_Reg<&CCM_Layout::CSCMR2, 2, 19> FLEXIO2_CLK_SEL;  // Selector for flexio2/flexio3 clock multiplexer
    // 0b00..derive clock from PLL4 divided clock
    // 0b01..derive clock from PLL3 PFD2 clock
    // 0b10..derive clock from PLL5 clock
    // 0b11..derive clock from pll3_sw_clk
constexpr CCM_Reg<&CCM_Layout::CSCMR2, 2,  8> CAN_CLK_SEL;      // Selector for CAN/CANFD clock multiplexer
    // 0b00..derive clock from pll3_sw_clk divided clock (60M)
    // 0b01..derive clock from osc_clk (24M)
    // 0b10..derive clock from pll3_sw_clk divided clock (80M)
    // 0b11..Disable FlexCAN clock
constexpr CCM_Reg<&CCM_Layout::CSCMR2, 6,  2> CAN_CLK_PODF;     // Divider for CAN/CANFD clock podf.
    // Divide by 'value' + 1
}  // namespace CSCMR2

// CCM Serial Clock Divider Register 1
namespace CSCDR1 {
constexpr CCM_Reg<&CCM_Layout::CSCDR1, 2, 25> TRACE_PODF;     // Divider for trace clock.
    // Divider should be updated when output clock is gated.
    // Divide by 'value' + 1:
    // 0b00..divide by 1
    // 0b01..divide by 2
    // 0b10..divide by 3
    // 0b11..divide by 4
constexpr CCM_Reg<&CCM_Layout::CSCDR1, 3, 16> USDHC2_PODF;    // Divider for usdhc2 clock.
    // Divider should be updated when output clock is gated.
    // Divide by 'value' + 1:
    // 0b000..divide by 1
    // 0b001..divide by 2
    // 0b010..divide by 3
    // 0b011..divide by 4
    // 0b100..divide by 5
    // 0b101..divide by 6
    // 0b110..divide by 7
    // 0b111..divide by 8
constexpr CCM_Reg<&CCM_Layout::CSCDR1, 3, 11> USDHC1_PODF;    // Divider for usdhc1 clock podf.
    // Divider should be updated when output clock is gated.
    // Divide by 'value' + 1:
    // 0b000..divide by 1
    // 0b001..divide by 2
    // 0b010..divide by 3
    // 0b011..divide by 4
    // 0b100..divide by 5
    // 0b101..divide by 6
    // 0b110..divide by 7
    // 0b111..divide by 8
constexpr CCM_Reg<&CCM_Layout::CSCDR1, 1,  6> UART_CLK_SEL;   // Selector for the UART clock multiplexor
    // 0b0..derive clock from pll3_80m
    // 0b1..derive clock from osc_clk
constexpr CCM_Reg<&CCM_Layout::CSCDR1, 6,  0> UART_CLK_PODF;  // Divider for uart clock podf.
    // Divide by 'value' + 1
}  // namespace CSCDR1

// CCM Clock Divider Register
namespace CS1CDR {
constexpr CCM_Reg<&CCM_Layout::CS1CDR, 3, 25> FLEXIO2_CLK_PODF;  //  Divider for flexio2/flexio3 clock.
    // Divider should be updated when output clock is gated.
    // Divide by 'value' + 1:
    // 0b000..Divide by 1
    // 0b001..Divide by 2
    // 0b010..Divide by 3
    // 0b011..Divide by 4
    // 0b100..Divide by 5
    // 0b101..Divide by 6
    // 0b110..Divide by 7
    // 0b111..Divide by 8
constexpr CCM_Reg<&CCM_Layout::CS1CDR, 3, 22> SAI3_CLK_PRED;     // Divider for sai3/adc1/adc2 clock pred.
    // Divide by 'value' + 1:
    // 0b000..divide by 1
    // 0b001..divide by 2
    // 0b010..divide by 3
    // 0b011..divide by 4
    // 0b100..divide by 5
    // 0b101..divide by 6
    // 0b110..divide by 7
    // 0b111..divide by 8
constexpr CCM_Reg<&CCM_Layout::CS1CDR, 6, 16> SAI3_CLK_PODF;     // Divider for sai3 clock podf.
    // The input clock to this divider should be lower
    //   than 300Mhz, the predivider can be used to achieve this.
    // Divide by 'value' + 1
constexpr CCM_Reg<&CCM_Layout::CS1CDR, 3,  9> FLEXIO2_CLK_PRED;  // Divider for flexio2/flexio3 clock.
    // Divide by 'value' + 1:
    // 0b000..divide by 1
    // 0b001..divide by 2
    // 0b010..divide by 3
    // 0b011..divide by 4
    // 0b100..divide by 5
    // 0b101..divide by 6
    // 0b110..divide by 7
    // 0b111..divide by 8
constexpr CCM_Reg<&CCM_Layout::CS1CDR, 3,  6> SAI1_CLK_PRED;     // Divider for sai1 clock pred.
    // Divide by 'value' + 1:
    // 0b000..divide by 1
    // 0b001..divide by 2
    // 0b010..divide by 3
    // 0b011..divide by 4
    // 0b100..divide by 5
    // 0b101..divide by 6
    // 0b110..divide by 7
    // 0b111..divide by 8
constexpr CCM_Reg<&CCM_Layout::CS1CDR, 6,  0> SAI1_CLK_PODF;     // Divider for sai1 clock podf.
    // The input clock to this divider should be lower
    //   than 300Mhz, the predivider can be used to achieve this.
    // Divide by 'value' + 1
}  // namespace CS1CDR

// CCM Clock Divider Register
namespace CS2CDR {
constexpr CCM_Reg<&CCM_Layout::CS2CDR, 3, 6> SAI2_CLK_PRED;  // Divider for sai2 clock pred.
    // Divider should be updated when output clock is gated.
    // Divide by 'value' + 1:
    // 0b000..divide by 1
    // 0b001..divide by 2
    // 0b010..divide by 3
    // 0b011..divide by 4
    // 0b100..divide by 5
    // 0b101..divide by 6
    // 0b110..divide by 7
    // 0b111..divide by 8
constexpr CCM_Reg<&CCM_Layout::CS2CDR, 6, 0> SAI2_CLK_PODF;  // Divider for sai2 clock podf.
    // The input clock to this divider should be lower
    //   than 300Mhz, the predivider can be used to achieve this.
    // Divide by 'value' + 1
}  // namespace CS2CDR

// CCM D1 Clock Divider Register
namespace CDCDR {
constexpr CCM_Reg<&CCM_Layout::CDCDR, 3, 25> SPDIF0_CLK_PRED;   // Divider for spdif0 clock pred.
    // Divider should be updated when output clock is gated.
    // Divide by 'value' + 1:
    // 0b000..Divide by 1
    // 0b001..Divide by 2
    // 0b010..Divide by 3
    // 0b011..Divide by 4
    // 0b100..Divide by 5
    // 0b101..Divide by 6
    // 0b110..Divide by 7
    // 0b111..Divide by 8
constexpr CCM_Reg<&CCM_Layout::CDCDR, 3, 22> SPDIF0_CLK_PODF;   // Divider for spdif0 clock podf.
    // Divider should be updated when output clock is gated.
    // Divide by 'value' + 1:
    // 0b000..Divide by 1
    // 0b001..Divide by 2
    // 0b010..Divide by 3
    // 0b011..Divide by 4
    // 0b100..Divide by 5
    // 0b101..Divide by 6
    // 0b110..Divide by 7
    // 0b111..Divide by 8
constexpr CCM_Reg<&CCM_Layout::CDCDR, 2, 20> SPDIF0_CLK_SEL;    // Selector for spdif0 clock multiplexer
    // 0b00..derive clock from PLL4
    // 0b01..derive clock from PLL3 PFD2
    // 0b10..derive clock from PLL5
    // 0b11..derive clock from pll3_sw_clk
constexpr CCM_Reg<&CCM_Layout::CDCDR, 3, 12> FLEXIO1_CLK_PRED;  // Divider for flexio1 clock pred.
    // Divider should be updated when output clock is gated.
    // Divide by 'value' + 1:
    // 0b000..Divide by 1
    // 0b001..Divide by 2
    // 0b010..Divide by 3
    // 0b011..Divide by 4
    // 0b100..Divide by 5
    // 0b101..Divide by 6
    // 0b110..Divide by 7
    // 0b111..Divide by 8
constexpr CCM_Reg<&CCM_Layout::CDCDR, 3,  9> FLEXIO1_CLK_PODF;  // Divider for flexio1 clock podf.
    // Divider should be updated when output clock is gated.
    // Divide by 'value' + 1:
    // 0b000..Divide by 1
    // 0b001..Divide by 2
    // 0b010..Divide by 3
    // 0b011..Divide by 4
    // 0b100..Divide by 5
    // 0b101..Divide by 6
    // 0b110..Divide by 7
    // 0b111..Divide by 8
constexpr CCM_Reg<&CCM_Layout::CDCDR, 2,  7> FLEXIO1_CLK_SEL;   // Selector for flexio1 clock multiplexer
    // 0b00..derive clock from PLL4
    // 0b01..derive clock from PLL3 PFD2
    // 0b10..derive clock from PLL5
    // 0b11..derive clock from pll3_sw_clk
}  // namespace CDCDR

// CCM Serial Clock Divider Register 2
namespace CSCDR2 {
constexpr CCM_Reg<&CCM_Layout::CSCDR2, 6, 19> LPI2C_CLK_PODF;     // Divider for lpi2c clock podf.
    // Divider should be updated when output clock is
    //   gated. The input clock to this divider should be lower than 300Mhz, the predivider can be used
    //   to achieve this.
    // Divide by 'value' + 1
constexpr CCM_Reg<&CCM_Layout::CSCDR2, 1, 18> LPI2C_CLK_SEL;      // Selector for the LPI2C clock multiplexor
    // 0b0..derive clock from pll3_60m
    // 0b1..derive clock from osc_clk
constexpr CCM_Reg<&CCM_Layout::CSCDR2, 3, 15> LCDIF_PRE_CLK_SEL;  // Selector for lcdif root clock pre-multiplexer
    // 0b000..derive clock from PLL2
    // 0b001..derive clock from PLL3 PFD3
    // 0b010..derive clock from PLL5
    // 0b011..derive clock from PLL2 PFD0
    // 0b100..derive clock from PLL2 PFD1
    // 0b101..derive clock from PLL3 PFD1
    // 0b110-0b111..Reserved
constexpr CCM_Reg<&CCM_Layout::CSCDR2, 3, 12> LCDIF_PRED;         // Pre-divider for lcdif clock.
    // Divider should be updated when output clock is gated.
    // Divide by 'value' + 1:
    // 0b000..divide by 1
    // 0b001..divide by 2
    // 0b010..divide by 3
    // 0b011..divide by 4
    // 0b100..divide by 5
    // 0b101..divide by 6
    // 0b110..divide by 7
    // 0b111..divide by 8
}  // namespace CSCDR2

// CCM Serial Clock Divider Register 3
namespace CSCDR3 {
constexpr CCM_Reg<&CCM_Layout::CSCDR3, 3, 11> CSI_PODF;     // Post divider for csi_mclk.
    // Divider should be updated when output clock is gated.
    // Divide by 'value' + 1:
    // 0b000..divide by 1
    // 0b001..divide by 2
    // 0b010..divide by 3
    // 0b011..divide by 4
    // 0b100..divide by 5
    // 0b101..divide by 6
    // 0b110..divide by 7
    // 0b111..divide by 8
constexpr CCM_Reg<&CCM_Layout::CSCDR3, 2,  9> CSI_CLK_SEL;  // Selector for csi_mclk multiplexer
    // 0b00..derive clock from osc_clk (24M)
    // 0b01..derive clock from PLL2 PFD2
    // 0b10..derive clock from pll3_120M
    // 0b11..derive clock from PLL3 PFD1
}  // namespace CSCDR3

// CCM Divider Handshake In-Process Register
namespace CDHIPR {
constexpr CCM_Reg<&CCM_Layout::CDHIPR, 1, 16> ARM_PODF_BUSY;         // Busy indicator for arm_podf.
    // 0b0..divider is not busy and its value represents the actual division.
    // 0b1..divider is busy with handshake process with module. The value read in the divider represents the previous
    //      value of the division factor, and after the handshake the written value of the arm_podf will be applied.
constexpr CCM_Reg<&CCM_Layout::CDHIPR, 1,  5> PERIPH_CLK_SEL_BUSY;   // Busy indicator for periph_clk_sel mux control.
    // 0b0..mux is not busy and its value represents the actual division.
    // 0b1..mux is busy with handshake process with module. The value read in the periph_clk_sel represents the
    //      previous value of select, and after the handshake periph_clk_sel value will be applied.
constexpr CCM_Reg<&CCM_Layout::CDHIPR, 1,  3> PERIPH2_CLK_SEL_BUSY;  // Busy indicator for periph2_clk_sel mux control.
    // 0b0..mux is not busy and its value represents the actual division.
    // 0b1..mux is busy with handshake process with module. The value read in the periph2_clk_sel represents the
    //      previous value of select, and after the handshake periph2_clk_sel value will be applied.
constexpr CCM_Reg<&CCM_Layout::CDHIPR, 1,  1> AHB_PODF_BUSY;         // Busy indicator for ahb_podf.
    // 0b0..divider is not busy and its value represents the actual division.
    // 0b1..divider is busy with handshake process with module. The value read in the divider represents the previous
    //      value of the division factor, and after the handshake the written value of the ahb_podf will be applied.
constexpr CCM_Reg<&CCM_Layout::CDHIPR, 1,  0> SEMC_PODF_BUSY;        // Busy indicator for semc_podf.
    // 0b0..divider is not busy and its value represents the actual division.
    // 0b1..divider is busy with handshake process with module. The value read in the divider represents the previous
    //      value of the division factor, and after the handshake the written value of the semc_podf will be applied.
}  // namespace CDHIPR

// CCM Low Power Control Register
namespace CLPCR {
constexpr CCM_Reg<&CCM_Layout::CLPCR, 1, 27> MASK_L2CC_IDLE;      // Mask L2CC IDLE for entering low power mode
    // 0b0..L2CC IDLE is not masked
    // 0b1..L2CC IDLE is masked
constexpr CCM_Reg<&CCM_Layout::CLPCR, 1, 26> MASK_SCU_IDLE;       // Mask SCU IDLE for entering low power mode
    // Assertion of all bits[27:22] will generate low power mode request
    // 0b0..SCU IDLE is not masked
    // 0b1..SCU IDLE is masked
constexpr CCM_Reg<&CCM_Layout::CLPCR, 1, 22> MASK_CORE0_WFI;      // Mask WFI of core0 for entering low power mode
    // Assertion of all bits[27:22] will generate low power mode request
    // 0b0..WFI of core0 is not masked
    // 0b1..WFI of core0 is masked
constexpr CCM_Reg<&CCM_Layout::CLPCR, 1, 21> BYPASS_LPM_HS0;      // Bypass low power mode handshake.
    // This bit should always be set to 1'b1 by software.
constexpr CCM_Reg<&CCM_Layout::CLPCR, 1, 19> BYPASS_LPM_HS1;      // Bypass low power mode handshake.
    // This bit should always be set to 1'b1 by software.
constexpr CCM_Reg<&CCM_Layout::CLPCR, 1, 11> COSC_PWRDOWN;        // In run mode, software can manually control powering down of on chip oscillator, i
    // 0b0..On chip oscillator will not be powered down, i.e. cosc_pwrdown = '0'.
    // 0b1..On chip oscillator will be powered down, i.e. cosc_pwrdown = '1'.
constexpr CCM_Reg<&CCM_Layout::CLPCR, 2,  9> STBY_COUNT;          // Standby counter definition
    // 0b00..CCM will wait (1*pmic_delay_scaler)+1 ckil clock cycles
    // 0b01..CCM will wait (3*pmic_delay_scaler)+1 ckil clock cycles
    // 0b10..CCM will wait (7*pmic_delay_scaler)+1 ckil clock cycles
    // 0b11..CCM will wait (15*pmic_delay_scaler)+1 ckil clock cycles
constexpr CCM_Reg<&CCM_Layout::CLPCR, 1,  8> VSTBY;               // Voltage standby request bit
    // 0b0..Voltage will not be changed to standby voltage after next entrance to STOP mode. ( PMIC_STBY_REQ will remain negated - '0')
    // 0b1..Voltage will be requested to change to standby voltage after next entrance to stop mode. ( PMIC_STBY_REQ will be asserted - '1').
constexpr CCM_Reg<&CCM_Layout::CLPCR, 1,  7> DIS_REF_OSC;         // dis_ref_osc - in run mode, software can manually control closing of external reference oscillator clock, i
    // 0b0..external high frequency oscillator will be enabled, i.e. CCM_REF_EN_B = '0'.
    // 0b1..external high frequency oscillator will be disabled, i.e. CCM_REF_EN_B = '1'
constexpr CCM_Reg<&CCM_Layout::CLPCR, 1,  6> SBYOS;               // Standby clock oscillator bit
    // 0b0..On-chip oscillator will not be powered down, after next entrance to STOP mode. (CCM_REF_EN_B will remain
    //      asserted - '0' and cosc_pwrdown will remain de asserted - '0')
    // 0b1..On-chip oscillator will be powered down, after next entrance to STOP mode. (CCM_REF_EN_B will be
    //      deasserted - '1' and cosc_pwrdown will be asserted - '1'). When returning from STOP mode, external oscillator will
    //      be enabled again, on-chip oscillator will return to oscillator mode, and after oscnt count, CCM will
    //      continue with the exit from the STOP mode process.
constexpr CCM_Reg<&CCM_Layout::CLPCR, 1,  5> ARM_CLK_DIS_ON_LPM;  // Define if Arm clocks (arm_clk, soc_mxclk, soc_pclk, soc_dbg_pclk, vl_wrck) will be disabled on wait mode
    // 0b0..Arm clock enabled on wait mode.
    // 0b1..Arm clock disabled on wait mode. .
constexpr CCM_Reg<&CCM_Layout::CLPCR, 2,  0> LPM;                 // Setting the low power mode that system will enter on next assertion of dsm_request signal.
    // 0b00..Remain in run mode
    // 0b01..Transfer to wait mode
    // 0b10..Transfer to stop mode
    // 0b11..Reserved
}  // namespace CLPCR

// CCM Interrupt Status Register
namespace CISR {
constexpr CCM_Reg<&CCM_Layout::CISR, 1, 26, 0x0> ARM_PODF_LOADED;         // CCM interrupt request 1 generated due to frequency change of arm_podf
    // 0b0..interrupt is not generated due to frequency change of arm_podf
    // 0b1..interrupt generated due to frequency change of arm_podf
constexpr CCM_Reg<&CCM_Layout::CISR, 1, 22, 0x0> PERIPH_CLK_SEL_LOADED;   // CCM interrupt request 1 generated due to update of periph_clk_sel.
    // 0b0..interrupt is not generated due to update of periph_clk_sel.
    // 0b1..interrupt generated due to update of periph_clk_sel.
constexpr CCM_Reg<&CCM_Layout::CISR, 1, 20, 0x0> AHB_PODF_LOADED;         // CCM interrupt request 1 generated due to frequency change of ahb_podf
    // 0b0..interrupt is not generated due to frequency change of ahb_podf
    // 0b1..interrupt generated due to frequency change of ahb_podf
constexpr CCM_Reg<&CCM_Layout::CISR, 1, 19, 0x0> PERIPH2_CLK_SEL_LOADED;  // CCM interrupt request 1 generated due to frequency change of periph2_clk_sel
    // 0b0..interrupt is not generated due to frequency change of periph2_clk_sel
    // 0b1..interrupt generated due to frequency change of periph2_clk_sel
constexpr CCM_Reg<&CCM_Layout::CISR, 1, 17, 0x0> SEMC_PODF_LOADED;        // CCM interrupt request 1 generated due to frequency change of semc_podf
    // 0b0..interrupt is not generated due to frequency change of semc_podf
    // 0b1..interrupt generated due to frequency change of semc_podf
constexpr CCM_Reg<&CCM_Layout::CISR, 1,  6, 0x0> COSC_READY;              // CCM interrupt request 2 generated due to on board oscillator ready, i
    // 0b0..interrupt is not generated due to on board oscillator ready
    // 0b1..interrupt generated due to on board oscillator ready
constexpr CCM_Reg<&CCM_Layout::CISR, 1,  0, 0x0> LRF_PLL;                 // CCM interrupt request 2 generated due to lock of all enabled and not bypaseed PLLs
    // 0b0..interrupt is not generated due to lock ready of all enabled and not bypaseed PLLs
    // 0b1..interrupt generated due to lock ready of all enabled and not bypaseed PLLs
}  // namespace CISR

// CCM Interrupt Mask Register
namespace CIMR {
constexpr CCM_Reg<&CCM_Layout::CIMR, 1, 26> ARM_PODF_LOADED;              // mask interrupt generation due to frequency change of arm_podf
    // 0b0..don't mask interrupt due to frequency change of arm_podf - interrupt will be created
    // 0b1..mask interrupt due to frequency change of arm_podf
constexpr CCM_Reg<&CCM_Layout::CIMR, 1, 22> MASK_PERIPH_CLK_SEL_LOADED;   // mask interrupt generation due to update of periph_clk_sel.
    // 0b0..don't mask interrupt due to update of periph_clk_sel - interrupt will be created
    // 0b1..mask interrupt due to update of periph_clk_sel
constexpr CCM_Reg<&CCM_Layout::CIMR, 1, 20> MASK_AHB_PODF_LOADED;         // mask interrupt generation due to frequency change of ahb_podf
    // 0b0..don't mask interrupt due to frequency change of ahb_podf - interrupt will be created
    // 0b1..mask interrupt due to frequency change of ahb_podf
constexpr CCM_Reg<&CCM_Layout::CIMR, 1, 19> MASK_PERIPH2_CLK_SEL_LOADED;  // mask interrupt generation due to update of periph2_clk_sel.
    // 0b0..don't mask interrupt due to update of periph2_clk_sel - interrupt will be created
    // 0b1..mask interrupt due to update of periph2_clk_sel
constexpr CCM_Reg<&CCM_Layout::CIMR, 1, 17> MASK_SEMC_PODF_LOADED;        // mask interrupt generation due to frequency change of semc_podf
    // 0b0..don't mask interrupt due to frequency change of semc_podf - interrupt will be created
    // 0b1..mask interrupt due to frequency change of semc_podf
constexpr CCM_Reg<&CCM_Layout::CIMR, 1,  6> MASK_COSC_READY;              // mask interrupt generation due to on board oscillator ready
    // 0b0..don't mask interrupt due to on board oscillator ready - interrupt will be created
    // 0b1..mask interrupt due to on board oscillator ready
constexpr CCM_Reg<&CCM_Layout::CIMR, 1,  0> MASK_LRF_PLL;                 // mask interrupt generation due to lrf of PLLs
    // 0b0..don't mask interrupt due to lrf of PLLs - interrupt will be created
    // 0b1..mask interrupt due to lrf of PLLs
}  // namespace CIMR

// CCM Clock Output Source Register
namespace CCOSR {
constexpr CCM_Reg<&CCM_Layout::CCOSR, 1, 24> CLKO2_EN;     // Enable of CCM_CLKO2 clock
    // 0b0..CCM_CLKO2 disabled.
    // 0b1..CCM_CLKO2 enabled.
constexpr CCM_Reg<&CCM_Layout::CCOSR, 3, 21> CLKO2_DIV;    // Setting the divider of CCM_CLKO2
    // Divide by 'value' + 1:
    // 0b000..divide by 1
    // 0b001..divide by 2
    // 0b010..divide by 3
    // 0b011..divide by 4
    // 0b100..divide by 5
    // 0b101..divide by 6
    // 0b110..divide by 7
    // 0b111..divide by 8
constexpr CCM_Reg<&CCM_Layout::CCOSR, 5, 16> CLKO2_SEL;    // Selection of the clock to be generated on CCM_CLKO2
    // 0b00011..usdhc1_clk_root
    // 0b00110..lpi2c_clk_root
    // 0b01011..csi_clk_root
    // 0b01110..osc_clk
    // 0b10001..usdhc2_clk_root
    // 0b10010..sai1_clk_root
    // 0b10011..sai2_clk_root
    // 0b10100..sai3_clk_root (shared with ADC1 and ADC2 alt_clk root)
    // 0b10111..can_clk_root (FlexCAN, shared with CANFD)
    // 0b11011..flexspi_clk_root
    // 0b11100..uart_clk_root
    // 0b11101..spdif0_clk_root
    // 0b11111..Reserved
constexpr CCM_Reg<&CCM_Layout::CCOSR, 1,  8> CLK_OUT_SEL;  // CCM_CLKO1 output to reflect CCM_CLKO1 or CCM_CLKO2 clocks
    // 0b0..CCM_CLKO1 output drives CCM_CLKO1 clock
    // 0b1..CCM_CLKO1 output drives CCM_CLKO2 clock
constexpr CCM_Reg<&CCM_Layout::CCOSR, 1,  7> CLKO1_EN;     // Enable of CCM_CLKO1 clock
    // 0b0..CCM_CLKO1 disabled.
    // 0b1..CCM_CLKO1 enabled.
constexpr CCM_Reg<&CCM_Layout::CCOSR, 3,  4> CLKO1_DIV;    // Setting the divider of CCM_CLKO1
    // Divide by 'value' + 1:
    // 0b000..divide by 1
    // 0b001..divide by 2
    // 0b010..divide by 3
    // 0b011..divide by 4
    // 0b100..divide by 5
    // 0b101..divide by 6
    // 0b110..divide by 7
    // 0b111..divide by 8
constexpr CCM_Reg<&CCM_Layout::CCOSR, 4,  0> CLKO1_SEL;    // Selection of the clock to be generated on CCM_CLKO1
    // 0b0000..USB1 PLL clock (divided by 2)
    // 0b0001..SYS PLL clock (divided by 2)
    // 0b0011..VIDEO PLL clock (divided by 2)
    // 0b0101..semc_clk_root
    // 0b0110..Reserved
    // 0b1010..lcdif_pix_clk_root
    // 0b1011..ahb_clk_root
    // 0b1100..ipg_clk_root
    // 0b1101..perclk_root
    // 0b1110..ckil_sync_clk_root
    // 0b1111..pll4_main_clk
}  // namespace CCOSR

// CCM General Purpose Register
namespace CGPR {
constexpr uint32_t kWOO = 0x0000'0002;

constexpr CGPR_Reg<&CCM_Layout::CGPR, 1, 17, kWOO> INT_MEM_CLK_LPM;         // Control for the Deep Sleep signal to the Arm Platform memories with additional control logic based on the Arm WFI signal
    // 0b0..Disable the clock to the Arm platform memories when entering Low Power Mode
    // 0b1..Keep the clocks to the Arm platform memories enabled only if an interrupt is pending when entering Low
    //      Power Modes (WAIT and STOP without power gating)
constexpr CGPR_Reg<&CCM_Layout::CGPR, 1, 16, kWOO> FPL;                     // Fast PLL enable.
    // 0b0..Engage PLL enable default way.
    // 0b1..Engage PLL enable 3 CKIL clocks earlier at exiting low power mode (STOP). Should be used only if 24MHz OSC was active in low power mode.
constexpr CGPR_Reg<&CCM_Layout::CGPR, 2, 14, kWOO> SYS_MEM_DS_CTRL;         // System memory DS control
    // 0b00..Disable memory DS mode always
    // 0b01..Enable memory (outside Arm platform) DS mode when system STOP and PLL are disabled
    // 0b1x..enable memory (outside Arm platform) DS mode when system is in STOP mode
constexpr CGPR_Reg<&CCM_Layout::CGPR, 1,  4, kWOO> EFUSE_PROG_SUPPLY_GATE;  // Defines the value of the output signal cgpr_dout[4].
    // Gate of program supply for efuse programing
    // 0b0..fuse programing supply voltage is gated off to the efuse module
    // 0b1..allow fuse programing.
constexpr CGPR_Reg<&CCM_Layout::CGPR, 1,  0, kWOO> PMIC_DELAY_SCALER;       // Defines clock dividion of clock for stby_count (pmic delay counter)
    // 0b0..clock is not divided
    // 0b1..clock is divided /8
}  // namespace CGPR

// CCM Clock Gating Register 0
namespace CCGR0 {
constexpr CCM_Reg<&CCM_Layout::CCGR0, 2, 30> GPIO2;        // CG15 - gpio2_clocks (gpio2_clk_enable)
constexpr CCM_Reg<&CCM_Layout::CCGR0, 2, 28> LPUART2;      // CG14 - lpuart2 clock (lpuart2_clk_enable)
constexpr CCM_Reg<&CCM_Layout::CCGR0, 2, 26> GPT2_SERIAL;  // CG13 - gpt2 serial clocks (gpt2_serial_clk_enable)
constexpr CCM_Reg<&CCM_Layout::CCGR0, 2, 24> GPT2_BUS;     // CG12 - gpt2 bus clocks (gpt2_bus_clk_enable)
constexpr CCM_Reg<&CCM_Layout::CCGR0, 2, 22> TRACE;        // CG11 - trace clock (trace_clk_enable)
constexpr CCM_Reg<&CCM_Layout::CCGR0, 2, 20> CAN2_SERIAL;  // CG10 - can2_serial clock (can2_serial_clk_enable)
constexpr CCM_Reg<&CCM_Layout::CCGR0, 2, 18> CAN2_CLOCK;   // CG9 - can2 clock (can2_clk_enable)
constexpr CCM_Reg<&CCM_Layout::CCGR0, 2, 16> CAN1_SERIAL;  // CG8 - can1_serial clock (can1_serial_clk_enable)
constexpr CCM_Reg<&CCM_Layout::CCGR0, 2, 14> CAN1_CLOCK;   // CG7 - can1 clock (can1_clk_enable)
constexpr CCM_Reg<&CCM_Layout::CCGR0, 2, 12> LPUART3;      // CG6 - lpuart3 clock (lpuart3_clk_enable)
constexpr CCM_Reg<&CCM_Layout::CCGR0, 2, 10> DCP;          // CG5 - dcp clock (dcp_clk_enable)
constexpr CCM_Reg<&CCM_Layout::CCGR0, 2,  8> SIM_M;        // CG4 - sim_m or sim_main register access clock (sim_m_mainclk_r_enable)
constexpr CCM_Reg<&CCM_Layout::CCGR0, 2,  6> CG3;          // CG3 - Reserved
constexpr CCM_Reg<&CCM_Layout::CCGR0, 2,  4> MQS;          // CG2 - mqs clock ( mqs_hmclk_clock_enable)
constexpr CCM_Reg<&CCM_Layout::CCGR0, 2,  2> AIPS_TZ2;     // CG1 - aips_tz2 clocks (aips_tz2_clk_enable)
constexpr CCM_Reg<&CCM_Layout::CCGR0, 2,  0> AIPS_TZ1;     // CG0 - aips_tz1 clocks (aips_tz1_clk_enable)
}  // namespace CCGR0

// CCM Clock Gating Register 1
namespace CCGR1 {
constexpr CCM_Reg<&CCM_Layout::CCGR1, 2, 30> GPIO5;        // CG15 - gpio5 clock (gpio5_clk_enable)
constexpr CCM_Reg<&CCM_Layout::CCGR1, 2, 28> CSU;          // CG14 - csu clock (csu_clk_enable)
constexpr CCM_Reg<&CCM_Layout::CCGR1, 2, 26> GPIO1;        // CG13 - gpio1 clock (gpio1_clk_enable)
constexpr CCM_Reg<&CCM_Layout::CCGR1, 2, 24> LPUART4;      // CG12 - lpuart4 clock (lpuart4_clk_enable)
constexpr CCM_Reg<&CCM_Layout::CCGR1, 2, 22> GPT1_SERIAL;  // CG11 - gpt1 serial clock (gpt_serial_clk_enable)
constexpr CCM_Reg<&CCM_Layout::CCGR1, 2, 20> GPT1_BUS;     // CG10 - gpt1 bus clock (gpt_clk_enable)
constexpr CCM_Reg<&CCM_Layout::CCGR1, 2, 18> SEMC_EXSC;    // CG9 - semc_exsc clock (semc_exsc_clk_enable)
constexpr CCM_Reg<&CCM_Layout::CCGR1, 2, 16> ADC1;         // CG8 - adc1 clock (adc1_clk_enable)
constexpr CCM_Reg<&CCM_Layout::CCGR1, 2, 14> AOI2;         // CG7 - aoi2 clocks (aoi2_clk_enable)
constexpr CCM_Reg<&CCM_Layout::CCGR1, 2, 12> PIT;          // CG6 - pit clocks (pit_clk_enable)
constexpr CCM_Reg<&CCM_Layout::CCGR1, 2, 10> ENET;         // CG5 - enet clock (enet_clk_enable)
constexpr CCM_Reg<&CCM_Layout::CCGR1, 2,  8> ADC2;         // CG4 - adc2 clock (adc2_clk_enable)
constexpr CCM_Reg<&CCM_Layout::CCGR1, 2,  6> LPSPI4;       // CG3 - lpspi4 clocks (lpspi4_clk_enable)
constexpr CCM_Reg<&CCM_Layout::CCGR1, 2,  4> LPSPI3;       // CG2 - lpspi3 clocks (lpspi3_clk_enable)
constexpr CCM_Reg<&CCM_Layout::CCGR1, 2,  2> LPSPI2;       // CG1 - lpspi2 clocks (lpspi2_clk_enable)
constexpr CCM_Reg<&CCM_Layout::CCGR1, 2,  0> LPSPI1;       // CG0 - lpspi1 clocks (lpspi1_clk_enable)
}  // namespace CCGR1

// CCM Clock Gating Register 2
namespace CCGR2 {
constexpr CCM_Reg<&CCM_Layout::CCGR2, 2, 30> PXP;          // CG15 - pxp clocks (pxp_clk_enable)
constexpr CCM_Reg<&CCM_Layout::CCGR2, 2, 28> LCD;          // CG14 - lcd clocks (lcd_clk_enable)
constexpr CCM_Reg<&CCM_Layout::CCGR2, 2, 26> GPIO3;        // CG13 - gpio3 clock (gpio3_clk_enable)
constexpr CCM_Reg<&CCM_Layout::CCGR2, 2, 24> XBAR2;        // CG12 - xbar2 clock (xbar2_clk_enable)
constexpr CCM_Reg<&CCM_Layout::CCGR2, 2, 22> XBAR1;        // CG11 - xbar1 clock (xbar1_clk_enable)
constexpr CCM_Reg<&CCM_Layout::CCGR2, 2, 20> IPMUX3;       // CG10 - ipmux3 clock (ipmux3_clk_enable)
constexpr CCM_Reg<&CCM_Layout::CCGR2, 2, 18> IPMUX2;       // CG9 - ipmux2 clock (ipmux2_clk_enable)
constexpr CCM_Reg<&CCM_Layout::CCGR2, 2, 16> IPMUX1;       // CG8 - ipmux1 clock (ipmux1_clk_enable)
constexpr CCM_Reg<&CCM_Layout::CCGR2, 2, 14> XBAR3;        // CG7 - xbar3 clock (xbar3_clk_enable)
constexpr CCM_Reg<&CCM_Layout::CCGR2, 2, 12> OCOTP_CTRL;   // CG6 - OCOTP_CTRL clock (ocotp_clk_enable)
constexpr CCM_Reg<&CCM_Layout::CCGR2, 2, 10> LPI2C3;       // CG5 - lpi2c3 clock (lpi2c3_clk_enable)
constexpr CCM_Reg<&CCM_Layout::CCGR2, 2,  8> LPI2C2;       // CG4 - lpi2c2 clock (lpi2c2_clk_enable)
constexpr CCM_Reg<&CCM_Layout::CCGR2, 2,  6> LPI2C1;       // CG3 - lpi2c1 clock (lpi2c1_clk_enable)
constexpr CCM_Reg<&CCM_Layout::CCGR2, 2,  4> IOMUXC_SNVS;  // CG2 - iomuxc_snvs clock (iomuxc_snvs_clk_enable)
constexpr CCM_Reg<&CCM_Layout::CCGR2, 2,  2> CSI;          // CG1 - csi clock (csi_clk_enable)
constexpr CCM_Reg<&CCM_Layout::CCGR2, 2,  0> OCRAM_EXSC;   // CG0 - ocram_exsc clock (ocram_exsc_clk_enable)
}  // namespace CCGR2

// CCM Clock Gating Register 3
namespace CCGR3 {
constexpr CCM_Reg<&CCM_Layout::CCGR3, 2, 30> IOMUXC_SNVS_GPR;  // CG15 - iomuxc_snvs_gpr clock (iomuxc_snvs_gpr_clk_enable)
constexpr CCM_Reg<&CCM_Layout::CCGR3, 2, 28> OCRAM;            // CG14 - The OCRAM clock cannot be turned off when the CM cache is running on this device.
constexpr CCM_Reg<&CCM_Layout::CCGR3, 2, 26> ACMP4;            // CG13 - acmp4 clocks (acmp4_clk_enable)
constexpr CCM_Reg<&CCM_Layout::CCGR3, 2, 24> ACMP3;            // CG12 - acmp3 clocks (acmp3_clk_enable)
constexpr CCM_Reg<&CCM_Layout::CCGR3, 2, 22> ACMP2;            // CG11 - acmp2 clocks (acmp2_clk_enable)
constexpr CCM_Reg<&CCM_Layout::CCGR3, 2, 20> ACMP1;            // CG10 - acmp1 clocks (acmp1_clk_enable)
constexpr CCM_Reg<&CCM_Layout::CCGR3, 2, 18> FLEXRAM;          // CG9 - flexram clock (flexram_clk_enable)
constexpr CCM_Reg<&CCM_Layout::CCGR3, 2, 16> WDOG1;            // CG8 - wdog1 clock (wdog1_clk_enable)
constexpr CCM_Reg<&CCM_Layout::CCGR3, 2, 14> EWM;              // CG7 - ewm clocks (ewm_clk_enable)
constexpr CCM_Reg<&CCM_Layout::CCGR3, 2, 12> GPIO4;            // CG6 - gpio4 clock (gpio4_clk_enable)
constexpr CCM_Reg<&CCM_Layout::CCGR3, 2, 10> LCDIF_PIX;        // CG5 - lcdif pix clock (lcdif_pix_clk_enable)
constexpr CCM_Reg<&CCM_Layout::CCGR3, 2,  8> AOI1;             // CG4 - aoi1 clock (aoi1_clk_enable)
constexpr CCM_Reg<&CCM_Layout::CCGR3, 2,  6> LPUART6;          // CG3 - lpuart6 clock (lpuart6_clk_enable)
constexpr CCM_Reg<&CCM_Layout::CCGR3, 2,  4> SEMC;             // CG2 - semc clocks (semc_clk_enable)
constexpr CCM_Reg<&CCM_Layout::CCGR3, 2,  2> LPUART5;          // CG1 - lpuart5 clock (lpuart5_clk_enable)
constexpr CCM_Reg<&CCM_Layout::CCGR3, 2,  0> FLEXIO2;          // CG0 - flexio2 clocks (flexio2_clk_enable)
}  // namespace CCGR3

// CCM Clock Gating Register 4
namespace CCGR4 {
constexpr CCM_Reg<&CCM_Layout::CCGR4, 2, 30> QDC4;        // CG15 - qdc4 clocks (qdc4_clk_enable)
constexpr CCM_Reg<&CCM_Layout::CCGR4, 2, 28> QDC3;        // CG14 - qdc3 clocks (qdc3_clk_enable)
constexpr CCM_Reg<&CCM_Layout::CCGR4, 2, 26> QDC2;        // CG13 - qdc2 clocks (qdc2_clk_enable)
constexpr CCM_Reg<&CCM_Layout::CCGR4, 2, 24> QDC1;        // CG12 - qdc1 clocks (qdc1_clk_enable)
constexpr CCM_Reg<&CCM_Layout::CCGR4, 2, 22> PWM4;        // CG11 - pwm4 clocks (pwm4_clk_enable)
constexpr CCM_Reg<&CCM_Layout::CCGR4, 2, 20> PWM3;        // CG10 - pwm3 clocks (pwm3_clk_enable)
constexpr CCM_Reg<&CCM_Layout::CCGR4, 2, 18> PWM2;        // CG9 - pwm2 clocks (pwm2_clk_enable)
constexpr CCM_Reg<&CCM_Layout::CCGR4, 2, 16> PWM1;        // CG8 - pwm1 clocks (pwm1_clk_enable)
constexpr CCM_Reg<&CCM_Layout::CCGR4, 2, 14> SIM_EMS;     // CG7 - sim_ems clocks (sim_ems_clk_enable)
constexpr CCM_Reg<&CCM_Layout::CCGR4, 2, 12> SIM_M;       // CG6 - sim_m clocks (sim_m_clk_enable)
constexpr CCM_Reg<&CCM_Layout::CCGR4, 2, 10> TSC_DIG;     // CG5 - tsc_dig clock (tsc_clk_enable)
constexpr CCM_Reg<&CCM_Layout::CCGR4, 2,  8> SIM_M7;      // CG4 - sim_m7 clock (sim_m7_clk_enable)
constexpr CCM_Reg<&CCM_Layout::CCGR4, 2,  6> BEE;         // CG3 - bee clock(bee_clk_enable)
constexpr CCM_Reg<&CCM_Layout::CCGR4, 2,  4> IOMUXC_GPR;  // CG2 - iomuxc gpr clock (iomuxc_gpr_clk_enable)
constexpr CCM_Reg<&CCM_Layout::CCGR4, 2,  2> IOMUXC;      // CG1 - iomuxc clock (iomuxc_clk_enable)
constexpr CCM_Reg<&CCM_Layout::CCGR4, 2,  0> SIM_M7_REG;  // CG0 - sim_m7 register access clock (sim_m7_mainclk_r_enable)
}  // namespace CCGR4

// CCM Clock Gating Register 5
namespace CCGR5 {
constexpr CCM_Reg<&CCM_Layout::CCGR5, 2, 30> SNVS_LP;   // CG15 - snvs_lp clock (snvs_lp_clk_enable)
constexpr CCM_Reg<&CCM_Layout::CCGR5, 2, 28> SNVS_HP;   // CG14 - snvs_hp clock (snvs_hp_clk_enable)
constexpr CCM_Reg<&CCM_Layout::CCGR5, 2, 26> LPUART7;   // CG13 - lpuart7 clock (lpuart7_clk_enable)
constexpr CCM_Reg<&CCM_Layout::CCGR5, 2, 24> LPUART1;   // CG12 - lpuart1 clock (lpuart1_clk_enable)
constexpr CCM_Reg<&CCM_Layout::CCGR5, 2, 22> SAI3;      // CG11 - sai3 clock (sai3_clk_enable)
constexpr CCM_Reg<&CCM_Layout::CCGR5, 2, 20> SAI2;      // CG10 - sai2 clock (sai2_clk_enable)
constexpr CCM_Reg<&CCM_Layout::CCGR5, 2, 18> SAI1;      // CG9 - sai1 clock (sai1_clk_enable)
constexpr CCM_Reg<&CCM_Layout::CCGR5, 2, 16> SIM_MAIN;  // CG8 - sim_main clock (sim_main_clk_enable)
constexpr CCM_Reg<&CCM_Layout::CCGR5, 2, 14> SPDIF;     // CG7 - spdif clock (spdif_clk_enable)
constexpr CCM_Reg<&CCM_Layout::CCGR5, 2, 12> AIPSTZ4;   // CG6 - aipstz4 clocks (aips_tz4_clk_enable)
constexpr CCM_Reg<&CCM_Layout::CCGR5, 2, 10> WDOG2;     // CG5 - wdog2 clock (wdog2_clk_enable)
constexpr CCM_Reg<&CCM_Layout::CCGR5, 2,  8> KPP;       // CG4 - kpp clock (kpp_clk_enable)
constexpr CCM_Reg<&CCM_Layout::CCGR5, 2,  6> DMA;       // CG3 - dma clock (dma_clk_enable)
constexpr CCM_Reg<&CCM_Layout::CCGR5, 2,  4> WDOG3;     // CG2 - wdog3 clock (wdog3_clk_enable)
constexpr CCM_Reg<&CCM_Layout::CCGR5, 2,  2> FLEXIO1;   // CG1 - flexio1 clock (flexio1_clk_enable)
constexpr CCM_Reg<&CCM_Layout::CCGR5, 2,  0> ROM;       // CG0 - rom clock (rom_clk_enable)
}  // namespace CCGR5

// CCM Clock Gating Register 6
namespace CCGR6 {
constexpr CCM_Reg<&CCM_Layout::CCGR6, 2, 30> TIMER3;    // CG15 - timer3 clocks (timer3_clk_enable)
constexpr CCM_Reg<&CCM_Layout::CCGR6, 2, 28> TIMER2;    // CG14 - timer2 clocks (timer2_clk_enable)
constexpr CCM_Reg<&CCM_Layout::CCGR6, 2, 26> TIMER1;    // CG13 - timer1 clocks (timer1_clk_enable)
constexpr CCM_Reg<&CCM_Layout::CCGR6, 2, 24> LPI2C4;    // CG12 - lpi2c4 serial clock (lpi2c4_serial_clk_enable)
constexpr CCM_Reg<&CCM_Layout::CCGR6, 2, 22> ANADIG;    // CG11 - anadig clocks (anadig_clk_enable)
constexpr CCM_Reg<&CCM_Layout::CCGR6, 2, 20> SIM_PER;   // CG10 - sim_per clock (sim_per_clk_enable) sim_axbs_p_clk_enable
constexpr CCM_Reg<&CCM_Layout::CCGR6, 2, 18> AIPS_TZ3;  // CG9 - aips_tz3 clock (aips_tz3_clk_enable)
constexpr CCM_Reg<&CCM_Layout::CCGR6, 2, 16> TIMER4;    // CG8 - timer4 clocks (timer4_clk_enable)
constexpr CCM_Reg<&CCM_Layout::CCGR6, 2, 14> LPUART8;   // CG7 - lpuart8 clocks (lpuart8_clk_enable)
constexpr CCM_Reg<&CCM_Layout::CCGR6, 2, 12> TRNG;      // CG6 - trng clock (trng_clk_enable)
constexpr CCM_Reg<&CCM_Layout::CCGR6, 2, 10> FLEXSPI;   // CG5 - flexspi clocks (flexspi_clk_enable) sim_ems_clk_enable must also be cleared, when flexspi_clk_enable is cleared
constexpr CCM_Reg<&CCM_Layout::CCGR6, 2,  8> IPMUX4;    // CG4 - ipmux4 clock (ipmux4_clk_enable)
constexpr CCM_Reg<&CCM_Layout::CCGR6, 2,  6> DCDC;      // CG3 - dcdc clocks (dcdc_clk_enable)
constexpr CCM_Reg<&CCM_Layout::CCGR6, 2,  4> USDHC2;    // CG2 - usdhc2 clocks (usdhc2_clk_enable)
constexpr CCM_Reg<&CCM_Layout::CCGR6, 2,  2> USDHC1;    // CG1 - usdhc1 clocks (usdhc1_clk_enable)
constexpr CCM_Reg<&CCM_Layout::CCGR6, 2,  0> USBOH3;    // CG0 - usboh3 clock (usboh3_clk_enable)
}  // namespace CCGR6

// CCM Clock Gating Register 7
namespace CCGR7 {
constexpr CCM_Reg<&CCM_Layout::CCGR7, 2, 12> FLEXIO3;      // CG6 - flexio3_clk_enable
constexpr CCM_Reg<&CCM_Layout::CCGR7, 2, 10> AIPS_LITE;    // CG5 - aips_lite_clk_enable
constexpr CCM_Reg<&CCM_Layout::CCGR7, 2,  8> CAN3_SERIAL;  // CG4 - can3_serial_clk_enable
constexpr CCM_Reg<&CCM_Layout::CCGR7, 2,  6> CAN3_CLK;     // CG3 - can3_clk_enable
constexpr CCM_Reg<&CCM_Layout::CCGR7, 2,  4> AXBS_L;       // CG2 - axbs_l_clk_enable
constexpr CCM_Reg<&CCM_Layout::CCGR7, 2,  2> FLEXSPI2;     // CG1 - flexspi2_clk_enable
constexpr CCM_Reg<&CCM_Layout::CCGR7, 2,  0> ENET2;        // CG0 - enet2_clk_enable
}  // namespace CCGR7

// CCM Module Enable Overide Register
namespace CMEOR {
constexpr CCM_Reg<&CCM_Layout::CMEOR, 1, 30> MOD_EN_OV_CAN1_CPI;   // Overide clock enable signal from CAN1 - clock will not be gated based on CAN's signal 'enable_clk_cpi'
    // 0b0..don't overide module enable signal
    // 0b1..overide module enable signal
constexpr CCM_Reg<&CCM_Layout::CMEOR, 1, 28> MOD_EN_OV_CAN2_CPI;   // Overide clock enable signal from CAN2 - clock will not be gated based on CAN's signal 'enable_clk_cpi'
    // 0b0..don't override module enable signal
    // 0b1..override module enable signal
constexpr CCM_Reg<&CCM_Layout::CMEOR, 1, 10> MOD_EN_OV_CANFD_CPI;  // Overide clock enable signal from FlexCAN3(CANFD) - clock will not be gated based on CAN's signal 'enable_clk_cpi'
    // 0b0..don't override module enable signal
    // 0b1..override module enable signal
constexpr CCM_Reg<&CCM_Layout::CMEOR, 1,  9> MOD_EN_OV_TRNG;       // Overide clock enable signal from TRNG
    // 0b0..don't override module enable signal
    // 0b1..override module enable signal
constexpr CCM_Reg<&CCM_Layout::CMEOR, 1,  7> MOD_EN_USDHC;         // overide clock enable signal from USDHC.
    // 0b0..don't override module enable signal
    // 0b1..override module enable signal
constexpr CCM_Reg<&CCM_Layout::CMEOR, 1,  6> MOD_EN_OV_PIT;        // Overide clock enable signal from PIT - clock will not be gated based on PIT's signal 'ipg_enable_clk'
    // 0b0..don't override module enable signal
    // 0b1..override module enable signal
constexpr CCM_Reg<&CCM_Layout::CMEOR, 1,  5> MOD_EN_OV_GPT;        // Overide clock enable signal from GPT - clock will not be gated based on GPT's signal 'ipg_enable_clk'
    // 0b0..don't override module enable signal
    // 0b1..override module enable signal
}  // namespace CMEOR

// Clock gating values
namespace CCGR {
constexpr uint32_t kOff      = 0x00;
constexpr uint32_t kRunOnly  = 0x01;
constexpr uint32_t kOn       = 0x03;
}  // namespace CCGR

}  // namespace CCM

}  // namespace imxrt1060
}  // namespace hardware
}  // namespace qindesign
