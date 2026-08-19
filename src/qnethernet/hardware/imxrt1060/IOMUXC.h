// SPDX-FileCopyrightText: (c) 2026 Shawn Silverman <shawn@pobox.com>
// SPDX-License-Identifier: AGPL-3.0-or-later

// IOMUXC.h defines all the IOMUXC registers.
// This file is part of the imxrt1060-regs library.

#pragma once

#include <cstddef>
#include <cstdint>
#include <type_traits>

#include "qnethernet/hardware/regs/regs.h"

namespace qindesign {
namespace hardware {
namespace imxrt1060 {

// Structure type to access the IOMUX Controller (IOMUXC) registers.
//
// Comments are from BSD-3-licensed NXP SDK.
//
// See:
// * https://github.com/nxp-mcuxpresso/mcux-devices-rt/blob/main/RT1060/periph/PERI_IOMUXC.h
// * https://github.com/nxp-mcuxpresso/legacy-mcux-sdk/blob/main/devices/MIMXRT1062/MIMXRT1062.h

// Size of Registers Arrays
constexpr size_t kIOMUXC_SW_MUX_CTL_PAD_count   = 124;
constexpr size_t kIOMUXC_SW_PAD_CTL_PAD_count   = 124;
constexpr size_t kIOMUXC_SELECT_INPUT_count     = 154;
constexpr size_t kIOMUXC_SW_MUX_CTL_PAD_1_count =  22;
constexpr size_t kIOMUXC_SW_PAD_CTL_PAD_1_count =  22;
constexpr size_t kIOMUXC_SELECT_INPUT_1_count   =  33;

struct IOMUXC_Layout {
  uint32_t HARDWARE_REGS_LAYOUT_MEMBER_RESERVED[5];
  volatile uint32_t SW_MUX_CTL_PAD[kIOMUXC_SW_MUX_CTL_PAD_count];      /**< SW_MUX_CTL_PAD_GPIO_EMC_00 SW MUX Control Register..SW_MUX_CTL_PAD_GPIO_SD_B1_11 SW MUX Control Register, array offset: 0x14, array step: 0x4 */
  volatile uint32_t SW_PAD_CTL_PAD[kIOMUXC_SW_PAD_CTL_PAD_count];      /**< SW_PAD_CTL_PAD_GPIO_EMC_00 SW PAD Control Register..SW_PAD_CTL_PAD_GPIO_SD_B1_11 SW PAD Control Register, array offset: 0x204, array step: 0x4 */
  volatile uint32_t SELECT_INPUT[kIOMUXC_SELECT_INPUT_count];          /**< ANATOP_USB_OTG1_ID_SELECT_INPUT DAISY Register..XBAR1_IN23_SELECT_INPUT DAISY Register, array offset: 0x3F4, array step: 0x4 */
  volatile uint32_t SW_MUX_CTL_PAD_1[kIOMUXC_SW_MUX_CTL_PAD_1_count];  /**< SW_MUX_CTL_PAD_GPIO_SPI_B0_00 SW MUX Control Register..SW_MUX_CTL_PAD_GPIO_SPI_B1_07 SW MUX Control Register, array offset: 0x65C, array step: 0x4 */
  volatile uint32_t SW_PAD_CTL_PAD_1[kIOMUXC_SW_PAD_CTL_PAD_1_count];  /**< SW_PAD_CTL_PAD_GPIO_SPI_B0_00 SW PAD Control Register..SW_PAD_CTL_PAD_GPIO_SPI_B1_07 SW PAD Control Register, array offset: 0x6B4, array step: 0x4 */
  volatile uint32_t SELECT_INPUT_1[kIOMUXC_SELECT_INPUT_1_count];      /**< ENET2_IPG_CLK_RMII_SELECT_INPUT DAISY Register..CANFD_IPP_IND_CANRX_SELECT_INPUT DAISY Register, array offset: 0x70C, array step: 0x4 */
};

constexpr size_t    kIOMUXC_size = 0x790;
constexpr uintptr_t kIOMUXC_base = 0x401F'8000;

namespace IOMUXC {

constexpr regs::RegGroup<IOMUXC_Layout, kIOMUXC_size, kIOMUXC_base> group;

template <size_t Index, size_t Bits, unsigned int Shift,
          auto AssignMask = regs::shiftedMask32<Bits, Shift>(),
          typename = std::enable_if_t<(Index < kIOMUXC_SW_MUX_CTL_PAD_count)>>
using SW_MUX_CTL_PAD_Reg =
    regs::Reg32<kIOMUXC_base, IOMUXC_Layout, &IOMUXC_Layout::SW_MUX_CTL_PAD,
                Index, Bits, Shift, AssignMask>;

template <size_t Index, size_t Bits, unsigned int Shift,
          auto AssignMask = regs::shiftedMask32<Bits, Shift>(),
          typename = std::enable_if_t<(Index < kIOMUXC_SW_PAD_CTL_PAD_count)>>
using SW_PAD_CTL_PAD_Reg =
    regs::Reg32<kIOMUXC_base, IOMUXC_Layout, &IOMUXC_Layout::SW_PAD_CTL_PAD,
                Index, Bits, Shift, AssignMask>;

template <size_t Index, size_t Bits, unsigned int Shift,
          auto AssignMask = regs::shiftedMask32<Bits, Shift>(),
          typename = std::enable_if_t<(Index < kIOMUXC_SELECT_INPUT_count)>>
using SELECT_INPUT_Reg =
    regs::Reg32<kIOMUXC_base, IOMUXC_Layout, &IOMUXC_Layout::SELECT_INPUT,
                Index, Bits, Shift, AssignMask>;

template <size_t Index, size_t Bits, unsigned int Shift,
          auto AssignMask = regs::shiftedMask32<Bits, Shift>(),
          typename = std::enable_if_t<(Index < kIOMUXC_SW_MUX_CTL_PAD_1_count)>>
using SW_MUX_CTL_PAD_1_Reg =
    regs::Reg32<kIOMUXC_base, IOMUXC_Layout, &IOMUXC_Layout::SW_MUX_CTL_PAD_1,
                Index, Bits, Shift, AssignMask>;

template <size_t Index, size_t Bits, unsigned int Shift,
          auto AssignMask = regs::shiftedMask32<Bits, Shift>(),
          typename = std::enable_if_t<(Index < kIOMUXC_SW_PAD_CTL_PAD_1_count)>>
using SW_PAD_CTL_PAD_1_Reg =
    regs::Reg32<kIOMUXC_base, IOMUXC_Layout, &IOMUXC_Layout::SW_PAD_CTL_PAD_1,
                Index, Bits, Shift, AssignMask>;

template <size_t Index, size_t Bits, unsigned int Shift,
          auto AssignMask = regs::shiftedMask32<Bits, Shift>(),
          typename = std::enable_if_t<(Index < kIOMUXC_SELECT_INPUT_1_count)>>
using SELECT_INPUT_1_Reg =
    regs::Reg32<kIOMUXC_base, IOMUXC_Layout, &IOMUXC_Layout::SELECT_INPUT_1,
                Index, Bits, Shift, AssignMask>;

namespace SW_MUX_CTL_PAD {
namespace vals {
constexpr regs::RegValue32<1, 4> SION;      // Software Input On Field.
    // 0b0..Input Path is determined by functionality
    // 0b1..Force input path of pad
constexpr regs::RegValue32<4, 0> MUX_MODE;  // MUX Mode Select Field.
    // Some values are less than 4 bits
    // 0b0000..Select mux mode: ALT0 mux port
    // 0b0001..Select mux mode: ALT1 mux port
    // 0b0010..Select mux mode: ALT2 mux port
    // 0b0011..Select mux mode: ALT3 mux port
    // 0b0100..Select mux mode: ALT4 mux port
    // 0b0101..Select mux mode: ALT5 mux port
    // 0b0110..Select mux mode: ALT6 mux port
    // 0b0111..Select mux mode: ALT7 mux port
    // 0b1000..Select mux mode: ALT8 mux port
    // 0b1001..Select mux mode: ALT9 mux port
}  // namespace vals

template <size_t Index>
constexpr SW_MUX_CTL_PAD_Reg<Index, 1, 4> SION;      // Software Input On Field.
    // 0b0..Input Path is determined by functionality
    // 0b1..Force input path of pad
template <size_t Index>
constexpr SW_MUX_CTL_PAD_Reg<Index, 4, 0> MUX_MODE;  // MUX Mode Select Field.
    // Some values are less than 4 bits
    // 0b0000..Select mux mode: ALT0 mux port
    // 0b0001..Select mux mode: ALT1 mux port
    // 0b0010..Select mux mode: ALT2 mux port
    // 0b0011..Select mux mode: ALT3 mux port
    // 0b0100..Select mux mode: ALT4 mux port
    // 0b0101..Select mux mode: ALT5 mux port
    // 0b0110..Select mux mode: ALT6 mux port
    // 0b0111..Select mux mode: ALT7 mux port
    // 0b1000..Select mux mode: ALT8 mux port
    // 0b1001..Select mux mode: ALT9 mux port
}  // namespace SW_MUX_CTL_PAD

namespace SW_PAD_CTL_PAD {
namespace vals {
constexpr regs::RegValue32<1, 16> HYS;    // Hyst. Enable Field
    // 0b0..Hysteresis Disabled
    // 0b1..Hysteresis Enabled
constexpr regs::RegValue32<2, 14> PUS;    // Pull Up / Down Config. Field
    // 0b00..100K Ohm Pull Down
    // 0b01..47K Ohm Pull Up
    // 0b10..100K Ohm Pull Up
    // 0b11..22K Ohm Pull Up
constexpr regs::RegValue32<1, 13> PUE;    // Pull / Keep Select Field
    // 0b0..Keeper
    // 0b1..Pull
constexpr regs::RegValue32<1, 12> PKE;    // Pull / Keep Enable Field
    // 0b0..Pull/Keeper Disabled
    // 0b1..Pull/Keeper Enabled
constexpr regs::RegValue32<1, 11> ODE;    // Open Drain Enable Field
    // 0b0..Open Drain Disabled
    // 0b1..Open Drain Enabled
constexpr regs::RegValue32<2,  6> SPEED;  // Speed Field
    // 0b00..low(50MHz)
    // 0b01..medium(100MHz)
    // 0b10..fast(150MHz)
    // 0b11..max(200MHz)
constexpr regs::RegValue32<3,  3> DSE;    // Drive Strength Field
    // 0b000..output driver disabled;
    // 0b001..R0(150 Ohm @ 3.3V, 260 Ohm@1.8V)
    // 0b010..R0/2
    // 0b011..R0/3
    // 0b100..R0/4
    // 0b101..R0/5
    // 0b110..R0/6
    // 0b111..R0/7
constexpr regs::RegValue32<1,  0> SRE;    // Slew Rate Field
    // 0b0..Slow Slew Rate
    // 0b1..Fast Slew Rate
}  // namespace vals

template <size_t Index>
constexpr SW_PAD_CTL_PAD_Reg<Index, 1, 16> HYS;    // Hyst. Enable Field
    // 0b0..Hysteresis Disabled
    // 0b1..Hysteresis Enabled
template <size_t Index>
constexpr SW_PAD_CTL_PAD_Reg<Index, 2, 14> PUS;    // Pull Up / Down Config. Field
    // 0b00..100K Ohm Pull Down
    // 0b01..47K Ohm Pull Up
    // 0b10..100K Ohm Pull Up
    // 0b11..22K Ohm Pull Up
template <size_t Index>
constexpr SW_PAD_CTL_PAD_Reg<Index, 1, 13> PUE;    // Pull / Keep Select Field
    // 0b0..Keeper
    // 0b1..Pull
template <size_t Index>
constexpr SW_PAD_CTL_PAD_Reg<Index, 1, 12> PKE;    // Pull / Keep Enable Field
    // 0b0..Pull/Keeper Disabled
    // 0b1..Pull/Keeper Enabled
template <size_t Index>
constexpr SW_PAD_CTL_PAD_Reg<Index, 1, 11> ODE;    // Open Drain Enable Field
    // 0b0..Open Drain Disabled
    // 0b1..Open Drain Enabled
template <size_t Index>
constexpr SW_PAD_CTL_PAD_Reg<Index, 2,  6> SPEED;  // Speed Field
    // 0b00..low(50MHz)
    // 0b01..medium(100MHz)
    // 0b10..fast(150MHz)
    // 0b11..max(200MHz)
template <size_t Index>
constexpr SW_PAD_CTL_PAD_Reg<Index, 3,  3> DSE;    // Drive Strength Field
    // 0b000..output driver disabled;
    // 0b001..R0(150 Ohm @ 3.3V, 260 Ohm@1.8V)
    // 0b010..R0/2
    // 0b011..R0/3
    // 0b100..R0/4
    // 0b101..R0/5
    // 0b110..R0/6
    // 0b111..R0/7
template <size_t Index>
constexpr SW_PAD_CTL_PAD_Reg<Index, 1,  0> SRE;    // Slew Rate Field
    // 0b0..Slow Slew Rate
    // 0b1..Fast Slew Rate
}  // namespace SW_PAD_CTL_PAD

namespace SELECT_INPUT {
namespace vals {
constexpr regs::RegValue32<3, 0> DAISY;  // Selecting Pads Involved in Daisy Chain.
    // Some values are less than 3 bits
}  // namespace vals

template <size_t Index>
constexpr SELECT_INPUT_Reg<Index, 3, 0> DAISY;  // Selecting Pads Involved in Daisy Chain.
    // Some values are less than 3 bits
}  // namespace SELECT_INPUT

namespace SW_MUX_CTL_PAD_1 {
template <size_t Index>
constexpr SW_MUX_CTL_PAD_1_Reg<Index, 1, 4> SION;      // Software Input On Field.
template <size_t Index>
constexpr SW_MUX_CTL_PAD_1_Reg<Index, 4, 0> MUX_MODE;  // MUX Mode Select Field.

namespace vals {
constexpr regs::RegValue32<1, 4> SION;
constexpr regs::RegValue32<4, 0> MUX_MODE;
}  // namespace vals
}  // namespace SW_MUX_CTL_PAD_1

// IOMUXC_SW_PAD_CTL_PAD_1
namespace SW_PAD_CTL_PAD_1 {
template <size_t Index>
constexpr SW_PAD_CTL_PAD_1_Reg<Index, 1, 16> HYS;    // Hyst. Enable Field
    // 0b0..Hysteresis Disabled
    // 0b1..Hysteresis Enabled
template <size_t Index>
constexpr SW_PAD_CTL_PAD_1_Reg<Index, 2, 14> PUS;    // Pull Up / Down Config. Field
    // 0b00..100K Ohm Pull Down
    // 0b01..47K Ohm Pull Up
    // 0b10..100K Ohm Pull Up
    // 0b11..22K Ohm Pull Up
template <size_t Index>
constexpr SW_PAD_CTL_PAD_1_Reg<Index, 1, 13> PUE;    // Pull / Keep Select Field
    // 0b0..Keeper
    // 0b1..Pull
template <size_t Index>
constexpr SW_PAD_CTL_PAD_1_Reg<Index, 1, 12> PKE;    // Pull / Keep Enable Field
    // 0b0..Pull/Keeper Disabled
    // 0b1..Pull/Keeper Enabled
template <size_t Index>
constexpr SW_PAD_CTL_PAD_1_Reg<Index, 1, 11> ODE;    // Open Drain Enable Field
    // 0b0..Open Drain Disabled
    // 0b1..Open Drain Enabled
template <size_t Index>
constexpr SW_PAD_CTL_PAD_1_Reg<Index, 2,  6> SPEED;  // Speed Field
    // 0b00..low(50MHz)
    // 0b01..medium(100MHz)
    // 0b10..fast(150MHz)
    // 0b11..max(200MHz)
template <size_t Index>
constexpr SW_PAD_CTL_PAD_1_Reg<Index, 3,  3> DSE;    // Drive Strength Field
    // 0b000..output driver disabled;
    // 0b001..R0(150 Ohm @ 3.3V, 260 Ohm@1.8V)
    // 0b010..R0/2
    // 0b011..R0/3
    // 0b100..R0/4
    // 0b101..R0/5
    // 0b110..R0/6
    // 0b111..R0/7
template <size_t Index>
constexpr SW_PAD_CTL_PAD_1_Reg<Index, 1,  0> SRE;    // Slew Rate Field
    // 0b0..Slow Slew Rate
    // 0b1..Fast Slew Rate

namespace vals {
constexpr regs::RegValue32<1, 16> HYS;
constexpr regs::RegValue32<2, 14> PUS;
constexpr regs::RegValue32<1, 13> PUE;
constexpr regs::RegValue32<1, 12> PKE;
constexpr regs::RegValue32<1, 11> ODE;
constexpr regs::RegValue32<2,  6> SPEED;
constexpr regs::RegValue32<3,  3> DSE;
constexpr regs::RegValue32<1,  0> SRE;
}  // namespace vals
}  // namespace SW_PAD_CTL_PAD_1

// IOMUXC_SELECT_INPUT_1
namespace SELECT_INPUT_1 {
template <size_t Index>
constexpr SELECT_INPUT_1_Reg<Index, 2, 0> DAISY;  // Selecting Pads Involved in Daisy Chain.

namespace vals {
constexpr regs::RegValue32<2, 0> DAISY;
}  // namespace vals
}  // namespace SELECT_INPUT_1

namespace SW_MUX_CTL_PAD {
namespace GPIO {

namespace EMC_00 {
constexpr SW_MUX_CTL_PAD_Reg<0, 1, 4> SION;      // Software Input On Field.
constexpr SW_MUX_CTL_PAD_Reg<0, 3, 0> MUX_MODE;  // MUX Mode Select Field.
}  // namespace EMC_00
namespace EMC_01 {
constexpr SW_MUX_CTL_PAD_Reg<1, 1, 4> SION;      // Software Input On Field.
constexpr SW_MUX_CTL_PAD_Reg<1, 3, 0> MUX_MODE;  // MUX Mode Select Field.
}  // namespace EMC_01
namespace EMC_02 {
constexpr SW_MUX_CTL_PAD_Reg<2, 1, 4> SION;      // Software Input On Field.
constexpr SW_MUX_CTL_PAD_Reg<2, 3, 0> MUX_MODE;  // MUX Mode Select Field.
}  // namespace EMC_02
namespace EMC_03 {
constexpr SW_MUX_CTL_PAD_Reg<3, 1, 4> SION;      // Software Input On Field.
constexpr SW_MUX_CTL_PAD_Reg<3, 3, 0> MUX_MODE;  // MUX Mode Select Field.
}  // namespace EMC_03
namespace EMC_04 {
constexpr SW_MUX_CTL_PAD_Reg<4, 1, 4> SION;      // Software Input On Field.
constexpr SW_MUX_CTL_PAD_Reg<4, 3, 0> MUX_MODE;  // MUX Mode Select Field.
}  // namespace EMC_04
namespace EMC_05 {
constexpr SW_MUX_CTL_PAD_Reg<5, 1, 4> SION;      // Software Input On Field.
constexpr SW_MUX_CTL_PAD_Reg<5, 3, 0> MUX_MODE;  // MUX Mode Select Field.
}  // namespace EMC_05
namespace EMC_06 {
constexpr SW_MUX_CTL_PAD_Reg<6, 1, 4> SION;      // Software Input On Field.
constexpr SW_MUX_CTL_PAD_Reg<6, 3, 0> MUX_MODE;  // MUX Mode Select Field.
}  // namespace EMC_06
namespace EMC_07 {
constexpr SW_MUX_CTL_PAD_Reg<7, 1, 4> SION;      // Software Input On Field.
constexpr SW_MUX_CTL_PAD_Reg<7, 3, 0> MUX_MODE;  // MUX Mode Select Field.
}  // namespace EMC_07
namespace EMC_08 {
constexpr SW_MUX_CTL_PAD_Reg<8, 1, 4> SION;      // Software Input On Field.
constexpr SW_MUX_CTL_PAD_Reg<8, 3, 0> MUX_MODE;  // MUX Mode Select Field.
}  // namespace EMC_08
namespace EMC_09 {
constexpr SW_MUX_CTL_PAD_Reg<9, 1, 4> SION;      // Software Input On Field.
constexpr SW_MUX_CTL_PAD_Reg<9, 4, 0> MUX_MODE;  // MUX Mode Select Field.
}  // namespace EMC_09
namespace EMC_10 {
constexpr SW_MUX_CTL_PAD_Reg<10, 1, 4> SION;      // Software Input On Field.
constexpr SW_MUX_CTL_PAD_Reg<10, 4, 0> MUX_MODE;  // MUX Mode Select Field.
}  // namespace EMC_10
namespace EMC_11 {
constexpr SW_MUX_CTL_PAD_Reg<11, 1, 4> SION;      // Software Input On Field.
constexpr SW_MUX_CTL_PAD_Reg<11, 4, 0> MUX_MODE;  // MUX Mode Select Field.
}  // namespace EMC_11
namespace EMC_12 {
constexpr SW_MUX_CTL_PAD_Reg<12, 1, 4> SION;      // Software Input On Field.
constexpr SW_MUX_CTL_PAD_Reg<12, 4, 0> MUX_MODE;  // MUX Mode Select Field.
}  // namespace EMC_12
namespace EMC_13 {
constexpr SW_MUX_CTL_PAD_Reg<13, 1, 4> SION;      // Software Input On Field.
constexpr SW_MUX_CTL_PAD_Reg<13, 4, 0> MUX_MODE;  // MUX Mode Select Field.
}  // namespace EMC_13
namespace EMC_14 {
constexpr SW_MUX_CTL_PAD_Reg<14, 1, 4> SION;      // Software Input On Field.
constexpr SW_MUX_CTL_PAD_Reg<14, 4, 0> MUX_MODE;  // MUX Mode Select Field.
}  // namespace EMC_14
namespace EMC_15 {
constexpr SW_MUX_CTL_PAD_Reg<15, 1, 4> SION;      // Software Input On Field.
constexpr SW_MUX_CTL_PAD_Reg<15, 4, 0> MUX_MODE;  // MUX Mode Select Field.
}  // namespace EMC_15
namespace EMC_16 {
constexpr SW_MUX_CTL_PAD_Reg<16, 1, 4> SION;      // Software Input On Field.
constexpr SW_MUX_CTL_PAD_Reg<16, 4, 0> MUX_MODE;  // MUX Mode Select Field.
}  // namespace EMC_16
namespace EMC_17 {
constexpr SW_MUX_CTL_PAD_Reg<17, 1, 4> SION;      // Software Input On Field.
constexpr SW_MUX_CTL_PAD_Reg<17, 3, 0> MUX_MODE;  // MUX Mode Select Field.
}  // namespace EMC_17
namespace EMC_18 {
constexpr SW_MUX_CTL_PAD_Reg<18, 1, 4> SION;      // Software Input On Field.
constexpr SW_MUX_CTL_PAD_Reg<18, 3, 0> MUX_MODE;  // MUX Mode Select Field.
}  // namespace EMC_18
namespace EMC_19 {
constexpr SW_MUX_CTL_PAD_Reg<19, 1, 4> SION;      // Software Input On Field.
constexpr SW_MUX_CTL_PAD_Reg<19, 3, 0> MUX_MODE;  // MUX Mode Select Field.
}  // namespace EMC_19
namespace EMC_20 {
constexpr SW_MUX_CTL_PAD_Reg<20, 1, 4> SION;      // Software Input On Field.
constexpr SW_MUX_CTL_PAD_Reg<20, 3, 0> MUX_MODE;  // MUX Mode Select Field.
}  // namespace EMC_20
namespace EMC_21 {
constexpr SW_MUX_CTL_PAD_Reg<21, 1, 4> SION;      // Software Input On Field.
constexpr SW_MUX_CTL_PAD_Reg<21, 3, 0> MUX_MODE;  // MUX Mode Select Field.
}  // namespace EMC_21
namespace EMC_22 {
constexpr SW_MUX_CTL_PAD_Reg<22, 1, 4> SION;      // Software Input On Field.
constexpr SW_MUX_CTL_PAD_Reg<22, 4, 0> MUX_MODE;  // MUX Mode Select Field.
}  // namespace EMC_22
namespace EMC_23 {
constexpr SW_MUX_CTL_PAD_Reg<23, 1, 4> SION;      // Software Input On Field.
constexpr SW_MUX_CTL_PAD_Reg<23, 4, 0> MUX_MODE;  // MUX Mode Select Field.
}  // namespace EMC_23
namespace EMC_24 {
constexpr SW_MUX_CTL_PAD_Reg<24, 1, 4> SION;      // Software Input On Field.
constexpr SW_MUX_CTL_PAD_Reg<24, 4, 0> MUX_MODE;  // MUX Mode Select Field.
}  // namespace EMC_24
namespace EMC_25 {
constexpr SW_MUX_CTL_PAD_Reg<25, 1, 4> SION;      // Software Input On Field.
constexpr SW_MUX_CTL_PAD_Reg<25, 4, 0> MUX_MODE;  // MUX Mode Select Field.
}  // namespace EMC_25
namespace EMC_26 {
constexpr SW_MUX_CTL_PAD_Reg<26, 1, 4> SION;      // Software Input On Field.
constexpr SW_MUX_CTL_PAD_Reg<26, 4, 0> MUX_MODE;  // MUX Mode Select Field.
}  // namespace EMC_26
namespace EMC_27 {
constexpr SW_MUX_CTL_PAD_Reg<27, 1, 4> SION;      // Software Input On Field.
constexpr SW_MUX_CTL_PAD_Reg<27, 4, 0> MUX_MODE;  // MUX Mode Select Field.
}  // namespace EMC_27
namespace EMC_28 {
constexpr SW_MUX_CTL_PAD_Reg<28, 1, 4> SION;      // Software Input On Field.
constexpr SW_MUX_CTL_PAD_Reg<28, 4, 0> MUX_MODE;  // MUX Mode Select Field.
}  // namespace EMC_28
namespace EMC_29 {
constexpr SW_MUX_CTL_PAD_Reg<29, 1, 4> SION;      // Software Input On Field.
constexpr SW_MUX_CTL_PAD_Reg<29, 4, 0> MUX_MODE;  // MUX Mode Select Field.
}  // namespace EMC_29
namespace EMC_30 {
constexpr SW_MUX_CTL_PAD_Reg<30, 1, 4> SION;      // Software Input On Field.
constexpr SW_MUX_CTL_PAD_Reg<30, 4, 0> MUX_MODE;  // MUX Mode Select Field.
}  // namespace EMC_30
namespace EMC_31 {
constexpr SW_MUX_CTL_PAD_Reg<31, 1, 4> SION;      // Software Input On Field.
constexpr SW_MUX_CTL_PAD_Reg<31, 4, 0> MUX_MODE;  // MUX Mode Select Field.
}  // namespace EMC_31
namespace EMC_32 {
constexpr SW_MUX_CTL_PAD_Reg<32, 1, 4> SION;      // Software Input On Field.
constexpr SW_MUX_CTL_PAD_Reg<32, 4, 0> MUX_MODE;  // MUX Mode Select Field.
}  // namespace EMC_32
namespace EMC_33 {
constexpr SW_MUX_CTL_PAD_Reg<33, 1, 4> SION;      // Software Input On Field.
constexpr SW_MUX_CTL_PAD_Reg<33, 4, 0> MUX_MODE;  // MUX Mode Select Field.
}  // namespace EMC_33
namespace EMC_34 {
constexpr SW_MUX_CTL_PAD_Reg<34, 1, 4> SION;      // Software Input On Field.
constexpr SW_MUX_CTL_PAD_Reg<34, 4, 0> MUX_MODE;  // MUX Mode Select Field.
}  // namespace EMC_34
namespace EMC_35 {
constexpr SW_MUX_CTL_PAD_Reg<35, 1, 4> SION;      // Software Input On Field.
constexpr SW_MUX_CTL_PAD_Reg<35, 4, 0> MUX_MODE;  // MUX Mode Select Field.
}  // namespace EMC_35
namespace EMC_36 {
constexpr SW_MUX_CTL_PAD_Reg<36, 1, 4> SION;      // Software Input On Field.
constexpr SW_MUX_CTL_PAD_Reg<36, 4, 0> MUX_MODE;  // MUX Mode Select Field.
}  // namespace EMC_36
namespace EMC_37 {
constexpr SW_MUX_CTL_PAD_Reg<37, 1, 4> SION;      // Software Input On Field.
constexpr SW_MUX_CTL_PAD_Reg<37, 4, 0> MUX_MODE;  // MUX Mode Select Field.
}  // namespace EMC_37
namespace EMC_38 {
constexpr SW_MUX_CTL_PAD_Reg<38, 1, 4> SION;      // Software Input On Field.
constexpr SW_MUX_CTL_PAD_Reg<38, 4, 0> MUX_MODE;  // MUX Mode Select Field.
}  // namespace EMC_38
namespace EMC_39 {
constexpr SW_MUX_CTL_PAD_Reg<39, 1, 4> SION;      // Software Input On Field.
constexpr SW_MUX_CTL_PAD_Reg<39, 4, 0> MUX_MODE;  // MUX Mode Select Field.
}  // namespace EMC_39
namespace EMC_40 {
constexpr SW_MUX_CTL_PAD_Reg<40, 1, 4> SION;      // Software Input On Field.
constexpr SW_MUX_CTL_PAD_Reg<40, 4, 0> MUX_MODE;  // MUX Mode Select Field.
}  // namespace EMC_40
namespace EMC_41 {
constexpr SW_MUX_CTL_PAD_Reg<41, 1, 4> SION;      // Software Input On Field.
constexpr SW_MUX_CTL_PAD_Reg<41, 3, 0> MUX_MODE;  // MUX Mode Select Field.
}  // namespace EMC_41

namespace AD_B0_00 {
constexpr SW_MUX_CTL_PAD_Reg<42, 1, 4> SION;      // Software Input On Field.
constexpr SW_MUX_CTL_PAD_Reg<42, 3, 0> MUX_MODE;  // MUX Mode Select Field.
}  // namespace AD_B0_00
namespace AD_B0_01 {
constexpr SW_MUX_CTL_PAD_Reg<43, 1, 4> SION;      // Software Input On Field.
constexpr SW_MUX_CTL_PAD_Reg<43, 3, 0> MUX_MODE;  // MUX Mode Select Field.
}  // namespace AD_B0_01
namespace AD_B0_02 {
constexpr SW_MUX_CTL_PAD_Reg<44, 1, 4> SION;      // Software Input On Field.
constexpr SW_MUX_CTL_PAD_Reg<44, 3, 0> MUX_MODE;  // MUX Mode Select Field.
}  // namespace AD_B0_02
namespace AD_B0_03 {
constexpr SW_MUX_CTL_PAD_Reg<45, 1, 4> SION;      // Software Input On Field.
constexpr SW_MUX_CTL_PAD_Reg<45, 3, 0> MUX_MODE;  // MUX Mode Select Field.
}  // namespace AD_B0_03
namespace AD_B0_04 {
constexpr SW_MUX_CTL_PAD_Reg<46, 1, 4> SION;      // Software Input On Field.
constexpr SW_MUX_CTL_PAD_Reg<46, 3, 0> MUX_MODE;  // MUX Mode Select Field.
}  // namespace AD_B0_04
namespace AD_B0_05 {
constexpr SW_MUX_CTL_PAD_Reg<47, 1, 4> SION;      // Software Input On Field.
constexpr SW_MUX_CTL_PAD_Reg<47, 3, 0> MUX_MODE;  // MUX Mode Select Field.
}  // namespace AD_B0_05
namespace AD_B0_06 {
constexpr SW_MUX_CTL_PAD_Reg<48, 1, 4> SION;      // Software Input On Field.
constexpr SW_MUX_CTL_PAD_Reg<48, 3, 0> MUX_MODE;  // MUX Mode Select Field.
}  // namespace AD_B0_06
namespace AD_B0_07 {
constexpr SW_MUX_CTL_PAD_Reg<49, 1, 4> SION;      // Software Input On Field.
constexpr SW_MUX_CTL_PAD_Reg<49, 3, 0> MUX_MODE;  // MUX Mode Select Field.
}  // namespace AD_B0_07
namespace AD_B0_08 {
constexpr SW_MUX_CTL_PAD_Reg<50, 1, 4> SION;      // Software Input On Field.
constexpr SW_MUX_CTL_PAD_Reg<50, 3, 0> MUX_MODE;  // MUX Mode Select Field.
}  // namespace AD_B0_08
namespace AD_B0_09 {
constexpr SW_MUX_CTL_PAD_Reg<51, 1, 4> SION;      // Software Input On Field.
constexpr SW_MUX_CTL_PAD_Reg<51, 4, 0> MUX_MODE;  // MUX Mode Select Field.
}  // namespace AD_B0_09
namespace AD_B0_10 {
constexpr SW_MUX_CTL_PAD_Reg<52, 1, 4> SION;      // Software Input On Field.
constexpr SW_MUX_CTL_PAD_Reg<52, 4, 0> MUX_MODE;  // MUX Mode Select Field.
}  // namespace AD_B0_10
namespace AD_B0_11 {
constexpr SW_MUX_CTL_PAD_Reg<53, 1, 4> SION;      // Software Input On Field.
constexpr SW_MUX_CTL_PAD_Reg<53, 4, 0> MUX_MODE;  // MUX Mode Select Field.
}  // namespace AD_B0_11
namespace AD_B0_12 {
constexpr SW_MUX_CTL_PAD_Reg<54, 1, 4> SION;      // Software Input On Field.
constexpr SW_MUX_CTL_PAD_Reg<54, 3, 0> MUX_MODE;  // MUX Mode Select Field.
}  // namespace AD_B0_12
namespace AD_B0_13 {
constexpr SW_MUX_CTL_PAD_Reg<55, 1, 4> SION;      // Software Input On Field.
constexpr SW_MUX_CTL_PAD_Reg<55, 3, 0> MUX_MODE;  // MUX Mode Select Field.
}  // namespace AD_B0_13
namespace AD_B0_14 {
constexpr SW_MUX_CTL_PAD_Reg<56, 1, 4> SION;      // Software Input On Field.
constexpr SW_MUX_CTL_PAD_Reg<56, 4, 0> MUX_MODE;  // MUX Mode Select Field.
}  // namespace AD_B0_14
namespace AD_B0_15 {
constexpr SW_MUX_CTL_PAD_Reg<57, 1, 4> SION;      // Software Input On Field.
constexpr SW_MUX_CTL_PAD_Reg<57, 4, 0> MUX_MODE;  // MUX Mode Select Field.
}  // namespace AD_B0_15

namespace AD_B1_00 {
constexpr SW_MUX_CTL_PAD_Reg<58, 1, 4> SION;      // Software Input On Field.
constexpr SW_MUX_CTL_PAD_Reg<58, 4, 0> MUX_MODE;  // MUX Mode Select Field.
}  // namespace AD_B1_00
namespace AD_B1_01 {
constexpr SW_MUX_CTL_PAD_Reg<59, 1, 4> SION;      // Software Input On Field.
constexpr SW_MUX_CTL_PAD_Reg<59, 4, 0> MUX_MODE;  // MUX Mode Select Field.
}  // namespace AD_B1_01
namespace AD_B1_02 {
constexpr SW_MUX_CTL_PAD_Reg<60, 1, 4> SION;      // Software Input On Field.
constexpr SW_MUX_CTL_PAD_Reg<60, 4, 0> MUX_MODE;  // MUX Mode Select Field.
}  // namespace AD_B1_02
namespace AD_B1_03 {
constexpr SW_MUX_CTL_PAD_Reg<61, 1, 4> SION;      // Software Input On Field.
constexpr SW_MUX_CTL_PAD_Reg<61, 4, 0> MUX_MODE;  // MUX Mode Select Field.
}  // namespace AD_B1_03
namespace AD_B1_04 {
constexpr SW_MUX_CTL_PAD_Reg<62, 1, 4> SION;      // Software Input On Field.
constexpr SW_MUX_CTL_PAD_Reg<62, 4, 0> MUX_MODE;  // MUX Mode Select Field.
}  // namespace AD_B1_04
namespace AD_B1_05 {
constexpr SW_MUX_CTL_PAD_Reg<63, 1, 4> SION;      // Software Input On Field.
constexpr SW_MUX_CTL_PAD_Reg<63, 4, 0> MUX_MODE;  // MUX Mode Select Field.
}  // namespace AD_B1_05
namespace AD_B1_06 {
constexpr SW_MUX_CTL_PAD_Reg<64, 1, 4> SION;      // Software Input On Field.
constexpr SW_MUX_CTL_PAD_Reg<64, 4, 0> MUX_MODE;  // MUX Mode Select Field.
}  // namespace AD_B1_06
namespace AD_B1_07 {
constexpr SW_MUX_CTL_PAD_Reg<65, 1, 4> SION;      // Software Input On Field.
constexpr SW_MUX_CTL_PAD_Reg<65, 4, 0> MUX_MODE;  // MUX Mode Select Field.
}  // namespace AD_B1_07
namespace AD_B1_08 {
constexpr SW_MUX_CTL_PAD_Reg<66, 1, 4> SION;      // Software Input On Field.
constexpr SW_MUX_CTL_PAD_Reg<66, 4, 0> MUX_MODE;  // MUX Mode Select Field.
}  // namespace AD_B1_08
namespace AD_B1_09 {
constexpr SW_MUX_CTL_PAD_Reg<67, 1, 4> SION;      // Software Input On Field.
constexpr SW_MUX_CTL_PAD_Reg<67, 4, 0> MUX_MODE;  // MUX Mode Select Field.
}  // namespace AD_B1_09
namespace AD_B1_10 {
constexpr SW_MUX_CTL_PAD_Reg<68, 1, 4> SION;      // Software Input On Field.
constexpr SW_MUX_CTL_PAD_Reg<68, 4, 0> MUX_MODE;  // MUX Mode Select Field.
}  // namespace AD_B1_10
namespace AD_B1_11 {
constexpr SW_MUX_CTL_PAD_Reg<69, 1, 4> SION;      // Software Input On Field.
constexpr SW_MUX_CTL_PAD_Reg<69, 4, 0> MUX_MODE;  // MUX Mode Select Field.
}  // namespace AD_B1_11
namespace AD_B1_12 {
constexpr SW_MUX_CTL_PAD_Reg<70, 1, 4> SION;      // Software Input On Field.
constexpr SW_MUX_CTL_PAD_Reg<70, 4, 0> MUX_MODE;  // MUX Mode Select Field.
}  // namespace AD_B1_12
namespace AD_B1_13 {
constexpr SW_MUX_CTL_PAD_Reg<71, 1, 4> SION;      // Software Input On Field.
constexpr SW_MUX_CTL_PAD_Reg<71, 4, 0> MUX_MODE;  // MUX Mode Select Field.
}  // namespace AD_B1_13
namespace AD_B1_14 {
constexpr SW_MUX_CTL_PAD_Reg<72, 1, 4> SION;      // Software Input On Field.
constexpr SW_MUX_CTL_PAD_Reg<72, 4, 0> MUX_MODE;  // MUX Mode Select Field.
}  // namespace AD_B1_14
namespace AD_B1_15 {
constexpr SW_MUX_CTL_PAD_Reg<73, 1, 4> SION;      // Software Input On Field.
constexpr SW_MUX_CTL_PAD_Reg<73, 4, 0> MUX_MODE;  // MUX Mode Select Field.
}  // namespace AD_B1_15

namespace B0_00 {
constexpr SW_MUX_CTL_PAD_Reg<74, 1, 4> SION;      // Software Input On Field.
constexpr SW_MUX_CTL_PAD_Reg<74, 4, 0> MUX_MODE;  // MUX Mode Select Field.
}  // namespace B0_00
namespace B0_01 {
constexpr SW_MUX_CTL_PAD_Reg<75, 1, 4> SION;      // Software Input On Field.
constexpr SW_MUX_CTL_PAD_Reg<75, 4, 0> MUX_MODE;  // MUX Mode Select Field.
}  // namespace B0_01
namespace B0_02 {
constexpr SW_MUX_CTL_PAD_Reg<76, 1, 4> SION;      // Software Input On Field.
constexpr SW_MUX_CTL_PAD_Reg<76, 4, 0> MUX_MODE;  // MUX Mode Select Field.
}  // namespace B0_02
namespace B0_03 {
constexpr SW_MUX_CTL_PAD_Reg<77, 1, 4> SION;      // Software Input On Field.
constexpr SW_MUX_CTL_PAD_Reg<77, 4, 0> MUX_MODE;  // MUX Mode Select Field.
}  // namespace B0_03
namespace B0_04 {
constexpr SW_MUX_CTL_PAD_Reg<78, 1, 4> SION;      // Software Input On Field.
constexpr SW_MUX_CTL_PAD_Reg<78, 4, 0> MUX_MODE;  // MUX Mode Select Field.
}  // namespace B0_04
namespace B0_05 {
constexpr SW_MUX_CTL_PAD_Reg<79, 1, 4> SION;      // Software Input On Field.
constexpr SW_MUX_CTL_PAD_Reg<79, 4, 0> MUX_MODE;  // MUX Mode Select Field.
}  // namespace B0_05
namespace B0_06 {
constexpr SW_MUX_CTL_PAD_Reg<80, 1, 4> SION;      // Software Input On Field.
constexpr SW_MUX_CTL_PAD_Reg<80, 4, 0> MUX_MODE;  // MUX Mode Select Field.
}  // namespace B0_06
namespace B0_07 {
constexpr SW_MUX_CTL_PAD_Reg<81, 1, 4> SION;      // Software Input On Field.
constexpr SW_MUX_CTL_PAD_Reg<81, 4, 0> MUX_MODE;  // MUX Mode Select Field.
}  // namespace B0_07
namespace B0_08 {
constexpr SW_MUX_CTL_PAD_Reg<82, 1, 4> SION;      // Software Input On Field.
constexpr SW_MUX_CTL_PAD_Reg<82, 4, 0> MUX_MODE;  // MUX Mode Select Field.
}  // namespace B0_08
namespace B0_09 {
constexpr SW_MUX_CTL_PAD_Reg<83, 1, 4> SION;      // Software Input On Field.
constexpr SW_MUX_CTL_PAD_Reg<83, 4, 0> MUX_MODE;  // MUX Mode Select Field.
}  // namespace B0_09
namespace B0_10 {
constexpr SW_MUX_CTL_PAD_Reg<84, 1, 4> SION;      // Software Input On Field.
constexpr SW_MUX_CTL_PAD_Reg<84, 4, 0> MUX_MODE;  // MUX Mode Select Field.
}  // namespace B0_10
namespace B0_11 {
constexpr SW_MUX_CTL_PAD_Reg<85, 1, 4> SION;      // Software Input On Field.
constexpr SW_MUX_CTL_PAD_Reg<85, 4, 0> MUX_MODE;  // MUX Mode Select Field.
}  // namespace B0_11
namespace B0_12 {
constexpr SW_MUX_CTL_PAD_Reg<86, 1, 4> SION;      // Software Input On Field.
constexpr SW_MUX_CTL_PAD_Reg<86, 4, 0> MUX_MODE;  // MUX Mode Select Field.
}  // namespace B0_12
namespace B0_13 {
constexpr SW_MUX_CTL_PAD_Reg<87, 1, 4> SION;      // Software Input On Field.
constexpr SW_MUX_CTL_PAD_Reg<87, 4, 0> MUX_MODE;  // MUX Mode Select Field.
}  // namespace B0_13
namespace B0_14 {
constexpr SW_MUX_CTL_PAD_Reg<88, 1, 4> SION;      // Software Input On Field.
constexpr SW_MUX_CTL_PAD_Reg<88, 4, 0> MUX_MODE;  // MUX Mode Select Field.
}  // namespace B0_14
namespace B0_15 {
constexpr SW_MUX_CTL_PAD_Reg<89, 1, 4> SION;      // Software Input On Field.
constexpr SW_MUX_CTL_PAD_Reg<89, 4, 0> MUX_MODE;  // MUX Mode Select Field.
}  // namespace B0_15

namespace B1_00 {
constexpr SW_MUX_CTL_PAD_Reg<90, 1, 4> SION;      // Software Input On Field.
constexpr SW_MUX_CTL_PAD_Reg<90, 4, 0> MUX_MODE;  // MUX Mode Select Field.
}  // namespace B1_00
namespace B1_01 {
constexpr SW_MUX_CTL_PAD_Reg<91, 1, 4> SION;      // Software Input On Field.
constexpr SW_MUX_CTL_PAD_Reg<91, 4, 0> MUX_MODE;  // MUX Mode Select Field.
}  // namespace B1_01
namespace B1_02 {
constexpr SW_MUX_CTL_PAD_Reg<92, 1, 4> SION;      // Software Input On Field.
constexpr SW_MUX_CTL_PAD_Reg<92, 4, 0> MUX_MODE;  // MUX Mode Select Field.
}  // namespace B1_02
namespace B1_03 {
constexpr SW_MUX_CTL_PAD_Reg<93, 1, 4> SION;      // Software Input On Field.
constexpr SW_MUX_CTL_PAD_Reg<93, 4, 0> MUX_MODE;  // MUX Mode Select Field.
}  // namespace B1_03
namespace B1_04 {
constexpr SW_MUX_CTL_PAD_Reg<94, 1, 4> SION;      // Software Input On Field.
constexpr SW_MUX_CTL_PAD_Reg<94, 4, 0> MUX_MODE;  // MUX Mode Select Field.
}  // namespace B1_04
namespace B1_05 {
constexpr SW_MUX_CTL_PAD_Reg<95, 1, 4> SION;      // Software Input On Field.
constexpr SW_MUX_CTL_PAD_Reg<95, 4, 0> MUX_MODE;  // MUX Mode Select Field.
}  // namespace B1_05
namespace B1_06 {
constexpr SW_MUX_CTL_PAD_Reg<96, 1, 4> SION;      // Software Input On Field.
constexpr SW_MUX_CTL_PAD_Reg<96, 4, 0> MUX_MODE;  // MUX Mode Select Field.
}  // namespace B1_06
namespace B1_07 {
constexpr SW_MUX_CTL_PAD_Reg<97, 1, 4> SION;      // Software Input On Field.
constexpr SW_MUX_CTL_PAD_Reg<97, 4, 0> MUX_MODE;  // MUX Mode Select Field.
}  // namespace B1_07
namespace B1_08 {
constexpr SW_MUX_CTL_PAD_Reg<98, 1, 4> SION;      // Software Input On Field.
constexpr SW_MUX_CTL_PAD_Reg<98, 4, 0> MUX_MODE;  // MUX Mode Select Field.
}  // namespace B1_08
namespace B1_09 {
constexpr SW_MUX_CTL_PAD_Reg<99, 1, 4> SION;      // Software Input On Field.
constexpr SW_MUX_CTL_PAD_Reg<99, 4, 0> MUX_MODE;  // MUX Mode Select Field.
}  // namespace B1_09
namespace B1_10 {
constexpr SW_MUX_CTL_PAD_Reg<100, 1, 4> SION;      // Software Input On Field.
constexpr SW_MUX_CTL_PAD_Reg<100, 4, 0> MUX_MODE;  // MUX Mode Select Field.
}  // namespace B1_10
namespace B1_11 {
constexpr SW_MUX_CTL_PAD_Reg<101, 1, 4> SION;      // Software Input On Field.
constexpr SW_MUX_CTL_PAD_Reg<101, 4, 0> MUX_MODE;  // MUX Mode Select Field.
}  // namespace B1_11
namespace B1_12 {
constexpr SW_MUX_CTL_PAD_Reg<102, 1, 4> SION;      // Software Input On Field.
constexpr SW_MUX_CTL_PAD_Reg<102, 4, 0> MUX_MODE;  // MUX Mode Select Field.
}  // namespace B1_12
namespace B1_13 {
constexpr SW_MUX_CTL_PAD_Reg<103, 1, 4> SION;      // Software Input On Field.
constexpr SW_MUX_CTL_PAD_Reg<103, 4, 0> MUX_MODE;  // MUX Mode Select Field.
}  // namespace B1_13
namespace B1_14 {
constexpr SW_MUX_CTL_PAD_Reg<104, 1, 4> SION;      // Software Input On Field.
constexpr SW_MUX_CTL_PAD_Reg<104, 4, 0> MUX_MODE;  // MUX Mode Select Field.
}  // namespace B1_14
namespace B1_15 {
constexpr SW_MUX_CTL_PAD_Reg<105, 1, 4> SION;      // Software Input On Field.
constexpr SW_MUX_CTL_PAD_Reg<105, 4, 0> MUX_MODE;  // MUX Mode Select Field.
}  // namespace B1_15

namespace SD_B0_00 {
constexpr SW_MUX_CTL_PAD_Reg<106, 1, 4> SION;      // Software Input On Field.
constexpr SW_MUX_CTL_PAD_Reg<106, 4, 0> MUX_MODE;  // MUX Mode Select Field.
}  // namespace SD_B0_00
namespace SD_B0_01 {
constexpr SW_MUX_CTL_PAD_Reg<107, 1, 4> SION;      // Software Input On Field.
constexpr SW_MUX_CTL_PAD_Reg<107, 4, 0> MUX_MODE;  // MUX Mode Select Field.
}  // namespace SD_B0_01
namespace SD_B0_02 {
constexpr SW_MUX_CTL_PAD_Reg<108, 1, 4> SION;      // Software Input On Field.
constexpr SW_MUX_CTL_PAD_Reg<108, 4, 0> MUX_MODE;  // MUX Mode Select Field.
}  // namespace SD_B0_02
namespace SD_B0_03 {
constexpr SW_MUX_CTL_PAD_Reg<109, 1, 4> SION;      // Software Input On Field.
constexpr SW_MUX_CTL_PAD_Reg<109, 4, 0> MUX_MODE;  // MUX Mode Select Field.
}  // namespace SD_B0_03
namespace SD_B0_04 {
constexpr SW_MUX_CTL_PAD_Reg<110, 1, 4> SION;      // Software Input On Field.
constexpr SW_MUX_CTL_PAD_Reg<110, 4, 0> MUX_MODE;  // MUX Mode Select Field.
}  // namespace SD_B0_04
namespace SD_B0_05 {
constexpr SW_MUX_CTL_PAD_Reg<111, 1, 4> SION;      // Software Input On Field.
constexpr SW_MUX_CTL_PAD_Reg<111, 4, 0> MUX_MODE;  // MUX Mode Select Field.
}  // namespace SD_B0_05

namespace SD_B1_00 {
constexpr SW_MUX_CTL_PAD_Reg<112, 1, 4> SION;      // Software Input On Field.
constexpr SW_MUX_CTL_PAD_Reg<112, 4, 0> MUX_MODE;  // MUX Mode Select Field.
}  // namespace SD_B1_00
namespace SD_B1_01 {
constexpr SW_MUX_CTL_PAD_Reg<113, 1, 4> SION;      // Software Input On Field.
constexpr SW_MUX_CTL_PAD_Reg<113, 4, 0> MUX_MODE;  // MUX Mode Select Field.
}  // namespace SD_B1_01
namespace SD_B1_02 {
constexpr SW_MUX_CTL_PAD_Reg<114, 1, 4> SION;      // Software Input On Field.
constexpr SW_MUX_CTL_PAD_Reg<114, 4, 0> MUX_MODE;  // MUX Mode Select Field.
}  // namespace SD_B1_02
namespace SD_B1_03 {
constexpr SW_MUX_CTL_PAD_Reg<115, 1, 4> SION;      // Software Input On Field.
constexpr SW_MUX_CTL_PAD_Reg<115, 4, 0> MUX_MODE;  // MUX Mode Select Field.
}  // namespace SD_B1_03
namespace SD_B1_04 {
constexpr SW_MUX_CTL_PAD_Reg<116, 1, 4> SION;      // Software Input On Field.
constexpr SW_MUX_CTL_PAD_Reg<116, 4, 0> MUX_MODE;  // MUX Mode Select Field.
}  // namespace SD_B1_04
namespace SD_B1_05 {
constexpr SW_MUX_CTL_PAD_Reg<117, 1, 4> SION;      // Software Input On Field.
constexpr SW_MUX_CTL_PAD_Reg<117, 4, 0> MUX_MODE;  // MUX Mode Select Field.
}  // namespace SD_B1_05
namespace SD_B1_06 {
constexpr SW_MUX_CTL_PAD_Reg<118, 1, 4> SION;      // Software Input On Field.
constexpr SW_MUX_CTL_PAD_Reg<118, 4, 0> MUX_MODE;  // MUX Mode Select Field.
}  // namespace SD_B1_06
namespace SD_B1_07 {
constexpr SW_MUX_CTL_PAD_Reg<119, 1, 4> SION;      // Software Input On Field.
constexpr SW_MUX_CTL_PAD_Reg<119, 3, 0> MUX_MODE;  // MUX Mode Select Field.
}  // namespace SD_B1_07
namespace SD_B1_08 {
constexpr SW_MUX_CTL_PAD_Reg<120, 1, 4> SION;      // Software Input On Field.
constexpr SW_MUX_CTL_PAD_Reg<120, 3, 0> MUX_MODE;  // MUX Mode Select Field.
}  // namespace SD_B1_08
namespace SD_B1_09 {
constexpr SW_MUX_CTL_PAD_Reg<121, 1, 4> SION;      // Software Input On Field.
constexpr SW_MUX_CTL_PAD_Reg<121, 3, 0> MUX_MODE;  // MUX Mode Select Field.
}  // namespace SD_B1_09
namespace SD_B1_10 {
constexpr SW_MUX_CTL_PAD_Reg<122, 1, 4> SION;      // Software Input On Field.
constexpr SW_MUX_CTL_PAD_Reg<122, 3, 0> MUX_MODE;  // MUX Mode Select Field.
}  // namespace SD_B1_10
namespace SD_B1_11 {
constexpr SW_MUX_CTL_PAD_Reg<123, 1, 4> SION;      // Software Input On Field.
constexpr SW_MUX_CTL_PAD_Reg<123, 3, 0> MUX_MODE;  // MUX Mode Select Field.
}  // namespace SD_B1_11

}  // namespace GPIO
}  // namespace SW_MUX_CTL_PAD

// IOMUXC SW_PAD_CTL_PAD
namespace SW_PAD_CTL_PAD {
namespace GPIO {

namespace EMC_00 {
constexpr SW_PAD_CTL_PAD_Reg<0, 1, 16> HYS;    // Hyst. Enable Field
constexpr SW_PAD_CTL_PAD_Reg<0, 2, 14> PUS;    // Pull Up / Down Config. Field
constexpr SW_PAD_CTL_PAD_Reg<0, 1, 13> PUE;    // Pull / Keep Select Field
constexpr SW_PAD_CTL_PAD_Reg<0, 1, 12> PKE;    // Pull / Keep Enable Field
constexpr SW_PAD_CTL_PAD_Reg<0, 1, 11> ODE;    // Open Drain Enable Field
constexpr SW_PAD_CTL_PAD_Reg<0, 2,  6> SPEED;  // Speed Field
constexpr SW_PAD_CTL_PAD_Reg<0, 3,  3> DSE;    // Drive Strength Field
constexpr SW_PAD_CTL_PAD_Reg<0, 1,  0> SRE;    // Slew Rate Field
}  // namespace EMC_00
namespace EMC_01 {
constexpr SW_PAD_CTL_PAD_Reg<1, 1, 16> HYS;    // Hyst. Enable Field
constexpr SW_PAD_CTL_PAD_Reg<1, 2, 14> PUS;    // Pull Up / Down Config. Field
constexpr SW_PAD_CTL_PAD_Reg<1, 1, 13> PUE;    // Pull / Keep Select Field
constexpr SW_PAD_CTL_PAD_Reg<1, 1, 12> PKE;    // Pull / Keep Enable Field
constexpr SW_PAD_CTL_PAD_Reg<1, 1, 11> ODE;    // Open Drain Enable Field
constexpr SW_PAD_CTL_PAD_Reg<1, 2,  6> SPEED;  // Speed Field
constexpr SW_PAD_CTL_PAD_Reg<1, 3,  3> DSE;    // Drive Strength Field
constexpr SW_PAD_CTL_PAD_Reg<1, 1,  0> SRE;    // Slew Rate Field
}  // namespace EMC_01
namespace EMC_02 {
constexpr SW_PAD_CTL_PAD_Reg<2, 1, 16> HYS;    // Hyst. Enable Field
constexpr SW_PAD_CTL_PAD_Reg<2, 2, 14> PUS;    // Pull Up / Down Config. Field
constexpr SW_PAD_CTL_PAD_Reg<2, 1, 13> PUE;    // Pull / Keep Select Field
constexpr SW_PAD_CTL_PAD_Reg<2, 1, 12> PKE;    // Pull / Keep Enable Field
constexpr SW_PAD_CTL_PAD_Reg<2, 1, 11> ODE;    // Open Drain Enable Field
constexpr SW_PAD_CTL_PAD_Reg<2, 2,  6> SPEED;  // Speed Field
constexpr SW_PAD_CTL_PAD_Reg<2, 3,  3> DSE;    // Drive Strength Field
constexpr SW_PAD_CTL_PAD_Reg<2, 1,  0> SRE;    // Slew Rate Field
}  // namespace EMC_02
namespace EMC_03 {
constexpr SW_PAD_CTL_PAD_Reg<3, 1, 16> HYS;    // Hyst. Enable Field
constexpr SW_PAD_CTL_PAD_Reg<3, 2, 14> PUS;    // Pull Up / Down Config. Field
constexpr SW_PAD_CTL_PAD_Reg<3, 1, 13> PUE;    // Pull / Keep Select Field
constexpr SW_PAD_CTL_PAD_Reg<3, 1, 12> PKE;    // Pull / Keep Enable Field
constexpr SW_PAD_CTL_PAD_Reg<3, 1, 11> ODE;    // Open Drain Enable Field
constexpr SW_PAD_CTL_PAD_Reg<3, 2,  6> SPEED;  // Speed Field
constexpr SW_PAD_CTL_PAD_Reg<3, 3,  3> DSE;    // Drive Strength Field
constexpr SW_PAD_CTL_PAD_Reg<3, 1,  0> SRE;    // Slew Rate Field
}  // namespace EMC_03
namespace EMC_04 {
constexpr SW_PAD_CTL_PAD_Reg<4, 1, 16> HYS;    // Hyst. Enable Field
constexpr SW_PAD_CTL_PAD_Reg<4, 2, 14> PUS;    // Pull Up / Down Config. Field
constexpr SW_PAD_CTL_PAD_Reg<4, 1, 13> PUE;    // Pull / Keep Select Field
constexpr SW_PAD_CTL_PAD_Reg<4, 1, 12> PKE;    // Pull / Keep Enable Field
constexpr SW_PAD_CTL_PAD_Reg<4, 1, 11> ODE;    // Open Drain Enable Field
constexpr SW_PAD_CTL_PAD_Reg<4, 2,  6> SPEED;  // Speed Field
constexpr SW_PAD_CTL_PAD_Reg<4, 3,  3> DSE;    // Drive Strength Field
constexpr SW_PAD_CTL_PAD_Reg<4, 1,  0> SRE;    // Slew Rate Field
}  // namespace EMC_04
namespace EMC_05 {
constexpr SW_PAD_CTL_PAD_Reg<5, 1, 16> HYS;    // Hyst. Enable Field
constexpr SW_PAD_CTL_PAD_Reg<5, 2, 14> PUS;    // Pull Up / Down Config. Field
constexpr SW_PAD_CTL_PAD_Reg<5, 1, 13> PUE;    // Pull / Keep Select Field
constexpr SW_PAD_CTL_PAD_Reg<5, 1, 12> PKE;    // Pull / Keep Enable Field
constexpr SW_PAD_CTL_PAD_Reg<5, 1, 11> ODE;    // Open Drain Enable Field
constexpr SW_PAD_CTL_PAD_Reg<5, 2,  6> SPEED;  // Speed Field
constexpr SW_PAD_CTL_PAD_Reg<5, 3,  3> DSE;    // Drive Strength Field
constexpr SW_PAD_CTL_PAD_Reg<5, 1,  0> SRE;    // Slew Rate Field
}  // namespace EMC_05
namespace EMC_06 {
constexpr SW_PAD_CTL_PAD_Reg<6, 1, 16> HYS;    // Hyst. Enable Field
constexpr SW_PAD_CTL_PAD_Reg<6, 2, 14> PUS;    // Pull Up / Down Config. Field
constexpr SW_PAD_CTL_PAD_Reg<6, 1, 13> PUE;    // Pull / Keep Select Field
constexpr SW_PAD_CTL_PAD_Reg<6, 1, 12> PKE;    // Pull / Keep Enable Field
constexpr SW_PAD_CTL_PAD_Reg<6, 1, 11> ODE;    // Open Drain Enable Field
constexpr SW_PAD_CTL_PAD_Reg<6, 2,  6> SPEED;  // Speed Field
constexpr SW_PAD_CTL_PAD_Reg<6, 3,  3> DSE;    // Drive Strength Field
constexpr SW_PAD_CTL_PAD_Reg<6, 1,  0> SRE;    // Slew Rate Field
}  // namespace EMC_06
namespace EMC_07 {
constexpr SW_PAD_CTL_PAD_Reg<7, 1, 16> HYS;    // Hyst. Enable Field
constexpr SW_PAD_CTL_PAD_Reg<7, 2, 14> PUS;    // Pull Up / Down Config. Field
constexpr SW_PAD_CTL_PAD_Reg<7, 1, 13> PUE;    // Pull / Keep Select Field
constexpr SW_PAD_CTL_PAD_Reg<7, 1, 12> PKE;    // Pull / Keep Enable Field
constexpr SW_PAD_CTL_PAD_Reg<7, 1, 11> ODE;    // Open Drain Enable Field
constexpr SW_PAD_CTL_PAD_Reg<7, 2,  6> SPEED;  // Speed Field
constexpr SW_PAD_CTL_PAD_Reg<7, 3,  3> DSE;    // Drive Strength Field
constexpr SW_PAD_CTL_PAD_Reg<7, 1,  0> SRE;    // Slew Rate Field
}  // namespace EMC_07
namespace EMC_08 {
constexpr SW_PAD_CTL_PAD_Reg<8, 1, 16> HYS;    // Hyst. Enable Field
constexpr SW_PAD_CTL_PAD_Reg<8, 2, 14> PUS;    // Pull Up / Down Config. Field
constexpr SW_PAD_CTL_PAD_Reg<8, 1, 13> PUE;    // Pull / Keep Select Field
constexpr SW_PAD_CTL_PAD_Reg<8, 1, 12> PKE;    // Pull / Keep Enable Field
constexpr SW_PAD_CTL_PAD_Reg<8, 1, 11> ODE;    // Open Drain Enable Field
constexpr SW_PAD_CTL_PAD_Reg<8, 2,  6> SPEED;  // Speed Field
constexpr SW_PAD_CTL_PAD_Reg<8, 3,  3> DSE;    // Drive Strength Field
constexpr SW_PAD_CTL_PAD_Reg<8, 1,  0> SRE;    // Slew Rate Field
}  // namespace EMC_08
namespace EMC_09 {
constexpr SW_PAD_CTL_PAD_Reg<9, 1, 16> HYS;    // Hyst. Enable Field
constexpr SW_PAD_CTL_PAD_Reg<9, 2, 14> PUS;    // Pull Up / Down Config. Field
constexpr SW_PAD_CTL_PAD_Reg<9, 1, 13> PUE;    // Pull / Keep Select Field
constexpr SW_PAD_CTL_PAD_Reg<9, 1, 12> PKE;    // Pull / Keep Enable Field
constexpr SW_PAD_CTL_PAD_Reg<9, 1, 11> ODE;    // Open Drain Enable Field
constexpr SW_PAD_CTL_PAD_Reg<9, 2,  6> SPEED;  // Speed Field
constexpr SW_PAD_CTL_PAD_Reg<9, 3,  3> DSE;    // Drive Strength Field
constexpr SW_PAD_CTL_PAD_Reg<9, 1,  0> SRE;    // Slew Rate Field
}  // namespace EMC_09
namespace EMC_10 {
constexpr SW_PAD_CTL_PAD_Reg<10, 1, 16> HYS;    // Hyst. Enable Field
constexpr SW_PAD_CTL_PAD_Reg<10, 2, 14> PUS;    // Pull Up / Down Config. Field
constexpr SW_PAD_CTL_PAD_Reg<10, 1, 13> PUE;    // Pull / Keep Select Field
constexpr SW_PAD_CTL_PAD_Reg<10, 1, 12> PKE;    // Pull / Keep Enable Field
constexpr SW_PAD_CTL_PAD_Reg<10, 1, 11> ODE;    // Open Drain Enable Field
constexpr SW_PAD_CTL_PAD_Reg<10, 2,  6> SPEED;  // Speed Field
constexpr SW_PAD_CTL_PAD_Reg<10, 3,  3> DSE;    // Drive Strength Field
constexpr SW_PAD_CTL_PAD_Reg<10, 1,  0> SRE;    // Slew Rate Field
}  // namespace EMC_10
namespace EMC_11 {
constexpr SW_PAD_CTL_PAD_Reg<11, 1, 16> HYS;    // Hyst. Enable Field
constexpr SW_PAD_CTL_PAD_Reg<11, 2, 14> PUS;    // Pull Up / Down Config. Field
constexpr SW_PAD_CTL_PAD_Reg<11, 1, 13> PUE;    // Pull / Keep Select Field
constexpr SW_PAD_CTL_PAD_Reg<11, 1, 12> PKE;    // Pull / Keep Enable Field
constexpr SW_PAD_CTL_PAD_Reg<11, 1, 11> ODE;    // Open Drain Enable Field
constexpr SW_PAD_CTL_PAD_Reg<11, 2,  6> SPEED;  // Speed Field
constexpr SW_PAD_CTL_PAD_Reg<11, 3,  3> DSE;    // Drive Strength Field
constexpr SW_PAD_CTL_PAD_Reg<11, 1,  0> SRE;    // Slew Rate Field
}  // namespace EMC_11
namespace EMC_12 {
constexpr SW_PAD_CTL_PAD_Reg<12, 1, 16> HYS;    // Hyst. Enable Field
constexpr SW_PAD_CTL_PAD_Reg<12, 2, 14> PUS;    // Pull Up / Down Config. Field
constexpr SW_PAD_CTL_PAD_Reg<12, 1, 13> PUE;    // Pull / Keep Select Field
constexpr SW_PAD_CTL_PAD_Reg<12, 1, 12> PKE;    // Pull / Keep Enable Field
constexpr SW_PAD_CTL_PAD_Reg<12, 1, 11> ODE;    // Open Drain Enable Field
constexpr SW_PAD_CTL_PAD_Reg<12, 2,  6> SPEED;  // Speed Field
constexpr SW_PAD_CTL_PAD_Reg<12, 3,  3> DSE;    // Drive Strength Field
constexpr SW_PAD_CTL_PAD_Reg<12, 1,  0> SRE;    // Slew Rate Field
}  // namespace EMC_12
namespace EMC_13 {
constexpr SW_PAD_CTL_PAD_Reg<13, 1, 16> HYS;    // Hyst. Enable Field
constexpr SW_PAD_CTL_PAD_Reg<13, 2, 14> PUS;    // Pull Up / Down Config. Field
constexpr SW_PAD_CTL_PAD_Reg<13, 1, 13> PUE;    // Pull / Keep Select Field
constexpr SW_PAD_CTL_PAD_Reg<13, 1, 12> PKE;    // Pull / Keep Enable Field
constexpr SW_PAD_CTL_PAD_Reg<13, 1, 11> ODE;    // Open Drain Enable Field
constexpr SW_PAD_CTL_PAD_Reg<13, 2,  6> SPEED;  // Speed Field
constexpr SW_PAD_CTL_PAD_Reg<13, 3,  3> DSE;    // Drive Strength Field
constexpr SW_PAD_CTL_PAD_Reg<13, 1,  0> SRE;    // Slew Rate Field
}  // namespace EMC_13
namespace EMC_14 {
constexpr SW_PAD_CTL_PAD_Reg<14, 1, 16> HYS;    // Hyst. Enable Field
constexpr SW_PAD_CTL_PAD_Reg<14, 2, 14> PUS;    // Pull Up / Down Config. Field
constexpr SW_PAD_CTL_PAD_Reg<14, 1, 13> PUE;    // Pull / Keep Select Field
constexpr SW_PAD_CTL_PAD_Reg<14, 1, 12> PKE;    // Pull / Keep Enable Field
constexpr SW_PAD_CTL_PAD_Reg<14, 1, 11> ODE;    // Open Drain Enable Field
constexpr SW_PAD_CTL_PAD_Reg<14, 2,  6> SPEED;  // Speed Field
constexpr SW_PAD_CTL_PAD_Reg<14, 3,  3> DSE;    // Drive Strength Field
constexpr SW_PAD_CTL_PAD_Reg<14, 1,  0> SRE;    // Slew Rate Field
}  // namespace EMC_14
namespace EMC_15 {
constexpr SW_PAD_CTL_PAD_Reg<15, 1, 16> HYS;    // Hyst. Enable Field
constexpr SW_PAD_CTL_PAD_Reg<15, 2, 14> PUS;    // Pull Up / Down Config. Field
constexpr SW_PAD_CTL_PAD_Reg<15, 1, 13> PUE;    // Pull / Keep Select Field
constexpr SW_PAD_CTL_PAD_Reg<15, 1, 12> PKE;    // Pull / Keep Enable Field
constexpr SW_PAD_CTL_PAD_Reg<15, 1, 11> ODE;    // Open Drain Enable Field
constexpr SW_PAD_CTL_PAD_Reg<15, 2,  6> SPEED;  // Speed Field
constexpr SW_PAD_CTL_PAD_Reg<15, 3,  3> DSE;    // Drive Strength Field
constexpr SW_PAD_CTL_PAD_Reg<15, 1,  0> SRE;    // Slew Rate Field
}  // namespace EMC_15
namespace EMC_16 {
constexpr SW_PAD_CTL_PAD_Reg<16, 1, 16> HYS;    // Hyst. Enable Field
constexpr SW_PAD_CTL_PAD_Reg<16, 2, 14> PUS;    // Pull Up / Down Config. Field
constexpr SW_PAD_CTL_PAD_Reg<16, 1, 13> PUE;    // Pull / Keep Select Field
constexpr SW_PAD_CTL_PAD_Reg<16, 1, 12> PKE;    // Pull / Keep Enable Field
constexpr SW_PAD_CTL_PAD_Reg<16, 1, 11> ODE;    // Open Drain Enable Field
constexpr SW_PAD_CTL_PAD_Reg<16, 2,  6> SPEED;  // Speed Field
constexpr SW_PAD_CTL_PAD_Reg<16, 3,  3> DSE;    // Drive Strength Field
constexpr SW_PAD_CTL_PAD_Reg<16, 1,  0> SRE;    // Slew Rate Field
}  // namespace EMC_16
namespace EMC_17 {
constexpr SW_PAD_CTL_PAD_Reg<17, 1, 16> HYS;    // Hyst. Enable Field
constexpr SW_PAD_CTL_PAD_Reg<17, 2, 14> PUS;    // Pull Up / Down Config. Field
constexpr SW_PAD_CTL_PAD_Reg<17, 1, 13> PUE;    // Pull / Keep Select Field
constexpr SW_PAD_CTL_PAD_Reg<17, 1, 12> PKE;    // Pull / Keep Enable Field
constexpr SW_PAD_CTL_PAD_Reg<17, 1, 11> ODE;    // Open Drain Enable Field
constexpr SW_PAD_CTL_PAD_Reg<17, 2,  6> SPEED;  // Speed Field
constexpr SW_PAD_CTL_PAD_Reg<17, 3,  3> DSE;    // Drive Strength Field
constexpr SW_PAD_CTL_PAD_Reg<17, 1,  0> SRE;    // Slew Rate Field
}  // namespace EMC_17
namespace EMC_18 {
constexpr SW_PAD_CTL_PAD_Reg<18, 1, 16> HYS;    // Hyst. Enable Field
constexpr SW_PAD_CTL_PAD_Reg<18, 2, 14> PUS;    // Pull Up / Down Config. Field
constexpr SW_PAD_CTL_PAD_Reg<18, 1, 13> PUE;    // Pull / Keep Select Field
constexpr SW_PAD_CTL_PAD_Reg<18, 1, 12> PKE;    // Pull / Keep Enable Field
constexpr SW_PAD_CTL_PAD_Reg<18, 1, 11> ODE;    // Open Drain Enable Field
constexpr SW_PAD_CTL_PAD_Reg<18, 2,  6> SPEED;  // Speed Field
constexpr SW_PAD_CTL_PAD_Reg<18, 3,  3> DSE;    // Drive Strength Field
constexpr SW_PAD_CTL_PAD_Reg<18, 1,  0> SRE;    // Slew Rate Field
}  // namespace EMC_18
namespace EMC_19 {
constexpr SW_PAD_CTL_PAD_Reg<19, 1, 16> HYS;    // Hyst. Enable Field
constexpr SW_PAD_CTL_PAD_Reg<19, 2, 14> PUS;    // Pull Up / Down Config. Field
constexpr SW_PAD_CTL_PAD_Reg<19, 1, 13> PUE;    // Pull / Keep Select Field
constexpr SW_PAD_CTL_PAD_Reg<19, 1, 12> PKE;    // Pull / Keep Enable Field
constexpr SW_PAD_CTL_PAD_Reg<19, 1, 11> ODE;    // Open Drain Enable Field
constexpr SW_PAD_CTL_PAD_Reg<19, 2,  6> SPEED;  // Speed Field
constexpr SW_PAD_CTL_PAD_Reg<19, 3,  3> DSE;    // Drive Strength Field
constexpr SW_PAD_CTL_PAD_Reg<19, 1,  0> SRE;    // Slew Rate Field
}  // namespace EMC_19
namespace EMC_20 {
constexpr SW_PAD_CTL_PAD_Reg<20, 1, 16> HYS;    // Hyst. Enable Field
constexpr SW_PAD_CTL_PAD_Reg<20, 2, 14> PUS;    // Pull Up / Down Config. Field
constexpr SW_PAD_CTL_PAD_Reg<20, 1, 13> PUE;    // Pull / Keep Select Field
constexpr SW_PAD_CTL_PAD_Reg<20, 1, 12> PKE;    // Pull / Keep Enable Field
constexpr SW_PAD_CTL_PAD_Reg<20, 1, 11> ODE;    // Open Drain Enable Field
constexpr SW_PAD_CTL_PAD_Reg<20, 2,  6> SPEED;  // Speed Field
constexpr SW_PAD_CTL_PAD_Reg<20, 3,  3> DSE;    // Drive Strength Field
constexpr SW_PAD_CTL_PAD_Reg<20, 1,  0> SRE;    // Slew Rate Field
}  // namespace EMC_20
namespace EMC_21 {
constexpr SW_PAD_CTL_PAD_Reg<21, 1, 16> HYS;    // Hyst. Enable Field
constexpr SW_PAD_CTL_PAD_Reg<21, 2, 14> PUS;    // Pull Up / Down Config. Field
constexpr SW_PAD_CTL_PAD_Reg<21, 1, 13> PUE;    // Pull / Keep Select Field
constexpr SW_PAD_CTL_PAD_Reg<21, 1, 12> PKE;    // Pull / Keep Enable Field
constexpr SW_PAD_CTL_PAD_Reg<21, 1, 11> ODE;    // Open Drain Enable Field
constexpr SW_PAD_CTL_PAD_Reg<21, 2,  6> SPEED;  // Speed Field
constexpr SW_PAD_CTL_PAD_Reg<21, 3,  3> DSE;    // Drive Strength Field
constexpr SW_PAD_CTL_PAD_Reg<21, 1,  0> SRE;    // Slew Rate Field
}  // namespace EMC_21
namespace EMC_22 {
constexpr SW_PAD_CTL_PAD_Reg<22, 1, 16> HYS;    // Hyst. Enable Field
constexpr SW_PAD_CTL_PAD_Reg<22, 2, 14> PUS;    // Pull Up / Down Config. Field
constexpr SW_PAD_CTL_PAD_Reg<22, 1, 13> PUE;    // Pull / Keep Select Field
constexpr SW_PAD_CTL_PAD_Reg<22, 1, 12> PKE;    // Pull / Keep Enable Field
constexpr SW_PAD_CTL_PAD_Reg<22, 1, 11> ODE;    // Open Drain Enable Field
constexpr SW_PAD_CTL_PAD_Reg<22, 2,  6> SPEED;  // Speed Field
constexpr SW_PAD_CTL_PAD_Reg<22, 3,  3> DSE;    // Drive Strength Field
constexpr SW_PAD_CTL_PAD_Reg<22, 1,  0> SRE;    // Slew Rate Field
}  // namespace EMC_22
namespace EMC_23 {
constexpr SW_PAD_CTL_PAD_Reg<23, 1, 16> HYS;    // Hyst. Enable Field
constexpr SW_PAD_CTL_PAD_Reg<23, 2, 14> PUS;    // Pull Up / Down Config. Field
constexpr SW_PAD_CTL_PAD_Reg<23, 1, 13> PUE;    // Pull / Keep Select Field
constexpr SW_PAD_CTL_PAD_Reg<23, 1, 12> PKE;    // Pull / Keep Enable Field
constexpr SW_PAD_CTL_PAD_Reg<23, 1, 11> ODE;    // Open Drain Enable Field
constexpr SW_PAD_CTL_PAD_Reg<23, 2,  6> SPEED;  // Speed Field
constexpr SW_PAD_CTL_PAD_Reg<23, 3,  3> DSE;    // Drive Strength Field
constexpr SW_PAD_CTL_PAD_Reg<23, 1,  0> SRE;    // Slew Rate Field
}  // namespace EMC_23
namespace EMC_24 {
constexpr SW_PAD_CTL_PAD_Reg<24, 1, 16> HYS;    // Hyst. Enable Field
constexpr SW_PAD_CTL_PAD_Reg<24, 2, 14> PUS;    // Pull Up / Down Config. Field
constexpr SW_PAD_CTL_PAD_Reg<24, 1, 13> PUE;    // Pull / Keep Select Field
constexpr SW_PAD_CTL_PAD_Reg<24, 1, 12> PKE;    // Pull / Keep Enable Field
constexpr SW_PAD_CTL_PAD_Reg<24, 1, 11> ODE;    // Open Drain Enable Field
constexpr SW_PAD_CTL_PAD_Reg<24, 2,  6> SPEED;  // Speed Field
constexpr SW_PAD_CTL_PAD_Reg<24, 3,  3> DSE;    // Drive Strength Field
constexpr SW_PAD_CTL_PAD_Reg<24, 1,  0> SRE;    // Slew Rate Field
}  // namespace EMC_24
namespace EMC_25 {
constexpr SW_PAD_CTL_PAD_Reg<25, 1, 16> HYS;    // Hyst. Enable Field
constexpr SW_PAD_CTL_PAD_Reg<25, 2, 14> PUS;    // Pull Up / Down Config. Field
constexpr SW_PAD_CTL_PAD_Reg<25, 1, 13> PUE;    // Pull / Keep Select Field
constexpr SW_PAD_CTL_PAD_Reg<25, 1, 12> PKE;    // Pull / Keep Enable Field
constexpr SW_PAD_CTL_PAD_Reg<25, 1, 11> ODE;    // Open Drain Enable Field
constexpr SW_PAD_CTL_PAD_Reg<25, 2,  6> SPEED;  // Speed Field
constexpr SW_PAD_CTL_PAD_Reg<25, 3,  3> DSE;    // Drive Strength Field
constexpr SW_PAD_CTL_PAD_Reg<25, 1,  0> SRE;    // Slew Rate Field
}  // namespace EMC_25
namespace EMC_26 {
constexpr SW_PAD_CTL_PAD_Reg<26, 1, 16> HYS;    // Hyst. Enable Field
constexpr SW_PAD_CTL_PAD_Reg<26, 2, 14> PUS;    // Pull Up / Down Config. Field
constexpr SW_PAD_CTL_PAD_Reg<26, 1, 13> PUE;    // Pull / Keep Select Field
constexpr SW_PAD_CTL_PAD_Reg<26, 1, 12> PKE;    // Pull / Keep Enable Field
constexpr SW_PAD_CTL_PAD_Reg<26, 1, 11> ODE;    // Open Drain Enable Field
constexpr SW_PAD_CTL_PAD_Reg<26, 2,  6> SPEED;  // Speed Field
constexpr SW_PAD_CTL_PAD_Reg<26, 3,  3> DSE;    // Drive Strength Field
constexpr SW_PAD_CTL_PAD_Reg<26, 1,  0> SRE;    // Slew Rate Field
}  // namespace EMC_26
namespace EMC_27 {
constexpr SW_PAD_CTL_PAD_Reg<27, 1, 16> HYS;    // Hyst. Enable Field
constexpr SW_PAD_CTL_PAD_Reg<27, 2, 14> PUS;    // Pull Up / Down Config. Field
constexpr SW_PAD_CTL_PAD_Reg<27, 1, 13> PUE;    // Pull / Keep Select Field
constexpr SW_PAD_CTL_PAD_Reg<27, 1, 12> PKE;    // Pull / Keep Enable Field
constexpr SW_PAD_CTL_PAD_Reg<27, 1, 11> ODE;    // Open Drain Enable Field
constexpr SW_PAD_CTL_PAD_Reg<27, 2,  6> SPEED;  // Speed Field
constexpr SW_PAD_CTL_PAD_Reg<27, 3,  3> DSE;    // Drive Strength Field
constexpr SW_PAD_CTL_PAD_Reg<27, 1,  0> SRE;    // Slew Rate Field
}  // namespace EMC_27
namespace EMC_28 {
constexpr SW_PAD_CTL_PAD_Reg<28, 1, 16> HYS;    // Hyst. Enable Field
constexpr SW_PAD_CTL_PAD_Reg<28, 2, 14> PUS;    // Pull Up / Down Config. Field
constexpr SW_PAD_CTL_PAD_Reg<28, 1, 13> PUE;    // Pull / Keep Select Field
constexpr SW_PAD_CTL_PAD_Reg<28, 1, 12> PKE;    // Pull / Keep Enable Field
constexpr SW_PAD_CTL_PAD_Reg<28, 1, 11> ODE;    // Open Drain Enable Field
constexpr SW_PAD_CTL_PAD_Reg<28, 2,  6> SPEED;  // Speed Field
constexpr SW_PAD_CTL_PAD_Reg<28, 3,  3> DSE;    // Drive Strength Field
constexpr SW_PAD_CTL_PAD_Reg<28, 1,  0> SRE;    // Slew Rate Field
}  // namespace EMC_28
namespace EMC_29 {
constexpr SW_PAD_CTL_PAD_Reg<29, 1, 16> HYS;    // Hyst. Enable Field
constexpr SW_PAD_CTL_PAD_Reg<29, 2, 14> PUS;    // Pull Up / Down Config. Field
constexpr SW_PAD_CTL_PAD_Reg<29, 1, 13> PUE;    // Pull / Keep Select Field
constexpr SW_PAD_CTL_PAD_Reg<29, 1, 12> PKE;    // Pull / Keep Enable Field
constexpr SW_PAD_CTL_PAD_Reg<29, 1, 11> ODE;    // Open Drain Enable Field
constexpr SW_PAD_CTL_PAD_Reg<29, 2,  6> SPEED;  // Speed Field
constexpr SW_PAD_CTL_PAD_Reg<29, 3,  3> DSE;    // Drive Strength Field
constexpr SW_PAD_CTL_PAD_Reg<29, 1,  0> SRE;    // Slew Rate Field
}  // namespace EMC_29
namespace EMC_30 {
constexpr SW_PAD_CTL_PAD_Reg<30, 1, 16> HYS;    // Hyst. Enable Field
constexpr SW_PAD_CTL_PAD_Reg<30, 2, 14> PUS;    // Pull Up / Down Config. Field
constexpr SW_PAD_CTL_PAD_Reg<30, 1, 13> PUE;    // Pull / Keep Select Field
constexpr SW_PAD_CTL_PAD_Reg<30, 1, 12> PKE;    // Pull / Keep Enable Field
constexpr SW_PAD_CTL_PAD_Reg<30, 1, 11> ODE;    // Open Drain Enable Field
constexpr SW_PAD_CTL_PAD_Reg<30, 2,  6> SPEED;  // Speed Field
constexpr SW_PAD_CTL_PAD_Reg<30, 3,  3> DSE;    // Drive Strength Field
constexpr SW_PAD_CTL_PAD_Reg<30, 1,  0> SRE;    // Slew Rate Field
}  // namespace EMC_30
namespace EMC_31 {
constexpr SW_PAD_CTL_PAD_Reg<31, 1, 16> HYS;    // Hyst. Enable Field
constexpr SW_PAD_CTL_PAD_Reg<31, 2, 14> PUS;    // Pull Up / Down Config. Field
constexpr SW_PAD_CTL_PAD_Reg<31, 1, 13> PUE;    // Pull / Keep Select Field
constexpr SW_PAD_CTL_PAD_Reg<31, 1, 12> PKE;    // Pull / Keep Enable Field
constexpr SW_PAD_CTL_PAD_Reg<31, 1, 11> ODE;    // Open Drain Enable Field
constexpr SW_PAD_CTL_PAD_Reg<31, 2,  6> SPEED;  // Speed Field
constexpr SW_PAD_CTL_PAD_Reg<31, 3,  3> DSE;    // Drive Strength Field
constexpr SW_PAD_CTL_PAD_Reg<31, 1,  0> SRE;    // Slew Rate Field
}  // namespace EMC_31
namespace EMC_32 {
constexpr SW_PAD_CTL_PAD_Reg<32, 1, 16> HYS;    // Hyst. Enable Field
constexpr SW_PAD_CTL_PAD_Reg<32, 2, 14> PUS;    // Pull Up / Down Config. Field
constexpr SW_PAD_CTL_PAD_Reg<32, 1, 13> PUE;    // Pull / Keep Select Field
constexpr SW_PAD_CTL_PAD_Reg<32, 1, 12> PKE;    // Pull / Keep Enable Field
constexpr SW_PAD_CTL_PAD_Reg<32, 1, 11> ODE;    // Open Drain Enable Field
constexpr SW_PAD_CTL_PAD_Reg<32, 2,  6> SPEED;  // Speed Field
constexpr SW_PAD_CTL_PAD_Reg<32, 3,  3> DSE;    // Drive Strength Field
constexpr SW_PAD_CTL_PAD_Reg<32, 1,  0> SRE;    // Slew Rate Field
}  // namespace EMC_32
namespace EMC_33 {
constexpr SW_PAD_CTL_PAD_Reg<33, 1, 16> HYS;    // Hyst. Enable Field
constexpr SW_PAD_CTL_PAD_Reg<33, 2, 14> PUS;    // Pull Up / Down Config. Field
constexpr SW_PAD_CTL_PAD_Reg<33, 1, 13> PUE;    // Pull / Keep Select Field
constexpr SW_PAD_CTL_PAD_Reg<33, 1, 12> PKE;    // Pull / Keep Enable Field
constexpr SW_PAD_CTL_PAD_Reg<33, 1, 11> ODE;    // Open Drain Enable Field
constexpr SW_PAD_CTL_PAD_Reg<33, 2,  6> SPEED;  // Speed Field
constexpr SW_PAD_CTL_PAD_Reg<33, 3,  3> DSE;    // Drive Strength Field
constexpr SW_PAD_CTL_PAD_Reg<33, 1,  0> SRE;    // Slew Rate Field
}  // namespace EMC_33
namespace EMC_34 {
constexpr SW_PAD_CTL_PAD_Reg<34, 1, 16> HYS;    // Hyst. Enable Field
constexpr SW_PAD_CTL_PAD_Reg<34, 2, 14> PUS;    // Pull Up / Down Config. Field
constexpr SW_PAD_CTL_PAD_Reg<34, 1, 13> PUE;    // Pull / Keep Select Field
constexpr SW_PAD_CTL_PAD_Reg<34, 1, 12> PKE;    // Pull / Keep Enable Field
constexpr SW_PAD_CTL_PAD_Reg<34, 1, 11> ODE;    // Open Drain Enable Field
constexpr SW_PAD_CTL_PAD_Reg<34, 2,  6> SPEED;  // Speed Field
constexpr SW_PAD_CTL_PAD_Reg<34, 3,  3> DSE;    // Drive Strength Field
constexpr SW_PAD_CTL_PAD_Reg<34, 1,  0> SRE;    // Slew Rate Field
}  // namespace EMC_34
namespace EMC_35 {
constexpr SW_PAD_CTL_PAD_Reg<35, 1, 16> HYS;    // Hyst. Enable Field
constexpr SW_PAD_CTL_PAD_Reg<35, 2, 14> PUS;    // Pull Up / Down Config. Field
constexpr SW_PAD_CTL_PAD_Reg<35, 1, 13> PUE;    // Pull / Keep Select Field
constexpr SW_PAD_CTL_PAD_Reg<35, 1, 12> PKE;    // Pull / Keep Enable Field
constexpr SW_PAD_CTL_PAD_Reg<35, 1, 11> ODE;    // Open Drain Enable Field
constexpr SW_PAD_CTL_PAD_Reg<35, 2,  6> SPEED;  // Speed Field
constexpr SW_PAD_CTL_PAD_Reg<35, 3,  3> DSE;    // Drive Strength Field
constexpr SW_PAD_CTL_PAD_Reg<35, 1,  0> SRE;    // Slew Rate Field
}  // namespace EMC_35
namespace EMC_36 {
constexpr SW_PAD_CTL_PAD_Reg<36, 1, 16> HYS;    // Hyst. Enable Field
constexpr SW_PAD_CTL_PAD_Reg<36, 2, 14> PUS;    // Pull Up / Down Config. Field
constexpr SW_PAD_CTL_PAD_Reg<36, 1, 13> PUE;    // Pull / Keep Select Field
constexpr SW_PAD_CTL_PAD_Reg<36, 1, 12> PKE;    // Pull / Keep Enable Field
constexpr SW_PAD_CTL_PAD_Reg<36, 1, 11> ODE;    // Open Drain Enable Field
constexpr SW_PAD_CTL_PAD_Reg<36, 2,  6> SPEED;  // Speed Field
constexpr SW_PAD_CTL_PAD_Reg<36, 3,  3> DSE;    // Drive Strength Field
constexpr SW_PAD_CTL_PAD_Reg<36, 1,  0> SRE;    // Slew Rate Field
}  // namespace EMC_36
namespace EMC_37 {
constexpr SW_PAD_CTL_PAD_Reg<37, 1, 16> HYS;    // Hyst. Enable Field
constexpr SW_PAD_CTL_PAD_Reg<37, 2, 14> PUS;    // Pull Up / Down Config. Field
constexpr SW_PAD_CTL_PAD_Reg<37, 1, 13> PUE;    // Pull / Keep Select Field
constexpr SW_PAD_CTL_PAD_Reg<37, 1, 12> PKE;    // Pull / Keep Enable Field
constexpr SW_PAD_CTL_PAD_Reg<37, 1, 11> ODE;    // Open Drain Enable Field
constexpr SW_PAD_CTL_PAD_Reg<37, 2,  6> SPEED;  // Speed Field
constexpr SW_PAD_CTL_PAD_Reg<37, 3,  3> DSE;    // Drive Strength Field
constexpr SW_PAD_CTL_PAD_Reg<37, 1,  0> SRE;    // Slew Rate Field
}  // namespace EMC_37
namespace EMC_38 {
constexpr SW_PAD_CTL_PAD_Reg<38, 1, 16> HYS;    // Hyst. Enable Field
constexpr SW_PAD_CTL_PAD_Reg<38, 2, 14> PUS;    // Pull Up / Down Config. Field
constexpr SW_PAD_CTL_PAD_Reg<38, 1, 13> PUE;    // Pull / Keep Select Field
constexpr SW_PAD_CTL_PAD_Reg<38, 1, 12> PKE;    // Pull / Keep Enable Field
constexpr SW_PAD_CTL_PAD_Reg<38, 1, 11> ODE;    // Open Drain Enable Field
constexpr SW_PAD_CTL_PAD_Reg<38, 2,  6> SPEED;  // Speed Field
constexpr SW_PAD_CTL_PAD_Reg<38, 3,  3> DSE;    // Drive Strength Field
constexpr SW_PAD_CTL_PAD_Reg<38, 1,  0> SRE;    // Slew Rate Field
}  // namespace EMC_38
namespace EMC_39 {
constexpr SW_PAD_CTL_PAD_Reg<39, 1, 16> HYS;    // Hyst. Enable Field
constexpr SW_PAD_CTL_PAD_Reg<39, 2, 14> PUS;    // Pull Up / Down Config. Field
constexpr SW_PAD_CTL_PAD_Reg<39, 1, 13> PUE;    // Pull / Keep Select Field
constexpr SW_PAD_CTL_PAD_Reg<39, 1, 12> PKE;    // Pull / Keep Enable Field
constexpr SW_PAD_CTL_PAD_Reg<39, 1, 11> ODE;    // Open Drain Enable Field
constexpr SW_PAD_CTL_PAD_Reg<39, 2,  6> SPEED;  // Speed Field
constexpr SW_PAD_CTL_PAD_Reg<39, 3,  3> DSE;    // Drive Strength Field
constexpr SW_PAD_CTL_PAD_Reg<39, 1,  0> SRE;    // Slew Rate Field
}  // namespace EMC_39
namespace EMC_40 {
constexpr SW_PAD_CTL_PAD_Reg<40, 1, 16> HYS;    // Hyst. Enable Field
constexpr SW_PAD_CTL_PAD_Reg<40, 2, 14> PUS;    // Pull Up / Down Config. Field
constexpr SW_PAD_CTL_PAD_Reg<40, 1, 13> PUE;    // Pull / Keep Select Field
constexpr SW_PAD_CTL_PAD_Reg<40, 1, 12> PKE;    // Pull / Keep Enable Field
constexpr SW_PAD_CTL_PAD_Reg<40, 1, 11> ODE;    // Open Drain Enable Field
constexpr SW_PAD_CTL_PAD_Reg<40, 2,  6> SPEED;  // Speed Field
constexpr SW_PAD_CTL_PAD_Reg<40, 3,  3> DSE;    // Drive Strength Field
constexpr SW_PAD_CTL_PAD_Reg<40, 1,  0> SRE;    // Slew Rate Field
}  // namespace EMC_40
namespace EMC_41 {
constexpr SW_PAD_CTL_PAD_Reg<41, 1, 16> HYS;    // Hyst. Enable Field
constexpr SW_PAD_CTL_PAD_Reg<41, 2, 14> PUS;    // Pull Up / Down Config. Field
constexpr SW_PAD_CTL_PAD_Reg<41, 1, 13> PUE;    // Pull / Keep Select Field
constexpr SW_PAD_CTL_PAD_Reg<41, 1, 12> PKE;    // Pull / Keep Enable Field
constexpr SW_PAD_CTL_PAD_Reg<41, 1, 11> ODE;    // Open Drain Enable Field
constexpr SW_PAD_CTL_PAD_Reg<41, 2,  6> SPEED;  // Speed Field
constexpr SW_PAD_CTL_PAD_Reg<41, 3,  3> DSE;    // Drive Strength Field
constexpr SW_PAD_CTL_PAD_Reg<41, 1,  0> SRE;    // Slew Rate Field
}  // namespace EMC_41

namespace AD_B0_00 {
constexpr SW_PAD_CTL_PAD_Reg<42, 1, 16> HYS;    // Hyst. Enable Field
constexpr SW_PAD_CTL_PAD_Reg<42, 2, 14> PUS;    // Pull Up / Down Config. Field
constexpr SW_PAD_CTL_PAD_Reg<42, 1, 13> PUE;    // Pull / Keep Select Field
constexpr SW_PAD_CTL_PAD_Reg<42, 1, 12> PKE;    // Pull / Keep Enable Field
constexpr SW_PAD_CTL_PAD_Reg<42, 1, 11> ODE;    // Open Drain Enable Field
constexpr SW_PAD_CTL_PAD_Reg<42, 2,  6> SPEED;  // Speed Field
constexpr SW_PAD_CTL_PAD_Reg<42, 3,  3> DSE;    // Drive Strength Field
constexpr SW_PAD_CTL_PAD_Reg<42, 1,  0> SRE;    // Slew Rate Field
}  // namespace AD_B0_00
namespace AD_B0_01 {
constexpr SW_PAD_CTL_PAD_Reg<43, 1, 16> HYS;    // Hyst. Enable Field
constexpr SW_PAD_CTL_PAD_Reg<43, 2, 14> PUS;    // Pull Up / Down Config. Field
constexpr SW_PAD_CTL_PAD_Reg<43, 1, 13> PUE;    // Pull / Keep Select Field
constexpr SW_PAD_CTL_PAD_Reg<43, 1, 12> PKE;    // Pull / Keep Enable Field
constexpr SW_PAD_CTL_PAD_Reg<43, 1, 11> ODE;    // Open Drain Enable Field
constexpr SW_PAD_CTL_PAD_Reg<43, 2,  6> SPEED;  // Speed Field
constexpr SW_PAD_CTL_PAD_Reg<43, 3,  3> DSE;    // Drive Strength Field
constexpr SW_PAD_CTL_PAD_Reg<43, 1,  0> SRE;    // Slew Rate Field
}  // namespace AD_B0_01
namespace AD_B0_02 {
constexpr SW_PAD_CTL_PAD_Reg<44, 1, 16> HYS;    // Hyst. Enable Field
constexpr SW_PAD_CTL_PAD_Reg<44, 2, 14> PUS;    // Pull Up / Down Config. Field
constexpr SW_PAD_CTL_PAD_Reg<44, 1, 13> PUE;    // Pull / Keep Select Field
constexpr SW_PAD_CTL_PAD_Reg<44, 1, 12> PKE;    // Pull / Keep Enable Field
constexpr SW_PAD_CTL_PAD_Reg<44, 1, 11> ODE;    // Open Drain Enable Field
constexpr SW_PAD_CTL_PAD_Reg<44, 2,  6> SPEED;  // Speed Field
constexpr SW_PAD_CTL_PAD_Reg<44, 3,  3> DSE;    // Drive Strength Field
constexpr SW_PAD_CTL_PAD_Reg<44, 1,  0> SRE;    // Slew Rate Field
}  // namespace AD_B0_02
namespace AD_B0_03 {
constexpr SW_PAD_CTL_PAD_Reg<45, 1, 16> HYS;    // Hyst. Enable Field
constexpr SW_PAD_CTL_PAD_Reg<45, 2, 14> PUS;    // Pull Up / Down Config. Field
constexpr SW_PAD_CTL_PAD_Reg<45, 1, 13> PUE;    // Pull / Keep Select Field
constexpr SW_PAD_CTL_PAD_Reg<45, 1, 12> PKE;    // Pull / Keep Enable Field
constexpr SW_PAD_CTL_PAD_Reg<45, 1, 11> ODE;    // Open Drain Enable Field
constexpr SW_PAD_CTL_PAD_Reg<45, 2,  6> SPEED;  // Speed Field
constexpr SW_PAD_CTL_PAD_Reg<45, 3,  3> DSE;    // Drive Strength Field
constexpr SW_PAD_CTL_PAD_Reg<45, 1,  0> SRE;    // Slew Rate Field
}  // namespace AD_B0_03
namespace AD_B0_04 {
constexpr SW_PAD_CTL_PAD_Reg<46, 1, 16> HYS;    // Hyst. Enable Field
constexpr SW_PAD_CTL_PAD_Reg<46, 2, 14> PUS;    // Pull Up / Down Config. Field
constexpr SW_PAD_CTL_PAD_Reg<46, 1, 13> PUE;    // Pull / Keep Select Field
constexpr SW_PAD_CTL_PAD_Reg<46, 1, 12> PKE;    // Pull / Keep Enable Field
constexpr SW_PAD_CTL_PAD_Reg<46, 1, 11> ODE;    // Open Drain Enable Field
constexpr SW_PAD_CTL_PAD_Reg<46, 2,  6> SPEED;  // Speed Field
constexpr SW_PAD_CTL_PAD_Reg<46, 3,  3> DSE;    // Drive Strength Field
constexpr SW_PAD_CTL_PAD_Reg<46, 1,  0> SRE;    // Slew Rate Field
}  // namespace AD_B0_04
namespace AD_B0_05 {
constexpr SW_PAD_CTL_PAD_Reg<47, 1, 16> HYS;    // Hyst. Enable Field
constexpr SW_PAD_CTL_PAD_Reg<47, 2, 14> PUS;    // Pull Up / Down Config. Field
constexpr SW_PAD_CTL_PAD_Reg<47, 1, 13> PUE;    // Pull / Keep Select Field
constexpr SW_PAD_CTL_PAD_Reg<47, 1, 12> PKE;    // Pull / Keep Enable Field
constexpr SW_PAD_CTL_PAD_Reg<47, 1, 11> ODE;    // Open Drain Enable Field
constexpr SW_PAD_CTL_PAD_Reg<47, 2,  6> SPEED;  // Speed Field
constexpr SW_PAD_CTL_PAD_Reg<47, 3,  3> DSE;    // Drive Strength Field
constexpr SW_PAD_CTL_PAD_Reg<47, 1,  0> SRE;    // Slew Rate Field
}  // namespace AD_B0_05
namespace AD_B0_06 {
constexpr SW_PAD_CTL_PAD_Reg<48, 1, 16> HYS;    // Hyst. Enable Field
constexpr SW_PAD_CTL_PAD_Reg<48, 2, 14> PUS;    // Pull Up / Down Config. Field
constexpr SW_PAD_CTL_PAD_Reg<48, 1, 13> PUE;    // Pull / Keep Select Field
constexpr SW_PAD_CTL_PAD_Reg<48, 1, 12> PKE;    // Pull / Keep Enable Field
constexpr SW_PAD_CTL_PAD_Reg<48, 1, 11> ODE;    // Open Drain Enable Field
constexpr SW_PAD_CTL_PAD_Reg<48, 2,  6> SPEED;  // Speed Field
constexpr SW_PAD_CTL_PAD_Reg<48, 3,  3> DSE;    // Drive Strength Field
constexpr SW_PAD_CTL_PAD_Reg<48, 1,  0> SRE;    // Slew Rate Field
}  // namespace AD_B0_06
namespace AD_B0_07 {
constexpr SW_PAD_CTL_PAD_Reg<49, 1, 16> HYS;    // Hyst. Enable Field
constexpr SW_PAD_CTL_PAD_Reg<49, 2, 14> PUS;    // Pull Up / Down Config. Field
constexpr SW_PAD_CTL_PAD_Reg<49, 1, 13> PUE;    // Pull / Keep Select Field
constexpr SW_PAD_CTL_PAD_Reg<49, 1, 12> PKE;    // Pull / Keep Enable Field
constexpr SW_PAD_CTL_PAD_Reg<49, 1, 11> ODE;    // Open Drain Enable Field
constexpr SW_PAD_CTL_PAD_Reg<49, 2,  6> SPEED;  // Speed Field
constexpr SW_PAD_CTL_PAD_Reg<49, 3,  3> DSE;    // Drive Strength Field
constexpr SW_PAD_CTL_PAD_Reg<49, 1,  0> SRE;    // Slew Rate Field
}  // namespace AD_B0_07
namespace AD_B0_08 {
constexpr SW_PAD_CTL_PAD_Reg<50, 1, 16> HYS;    // Hyst. Enable Field
constexpr SW_PAD_CTL_PAD_Reg<50, 2, 14> PUS;    // Pull Up / Down Config. Field
constexpr SW_PAD_CTL_PAD_Reg<50, 1, 13> PUE;    // Pull / Keep Select Field
constexpr SW_PAD_CTL_PAD_Reg<50, 1, 12> PKE;    // Pull / Keep Enable Field
constexpr SW_PAD_CTL_PAD_Reg<50, 1, 11> ODE;    // Open Drain Enable Field
constexpr SW_PAD_CTL_PAD_Reg<50, 2,  6> SPEED;  // Speed Field
constexpr SW_PAD_CTL_PAD_Reg<50, 3,  3> DSE;    // Drive Strength Field
constexpr SW_PAD_CTL_PAD_Reg<50, 1,  0> SRE;    // Slew Rate Field
}  // namespace AD_B0_08
namespace AD_B0_09 {
constexpr SW_PAD_CTL_PAD_Reg<51, 1, 16> HYS;    // Hyst. Enable Field
constexpr SW_PAD_CTL_PAD_Reg<51, 2, 14> PUS;    // Pull Up / Down Config. Field
constexpr SW_PAD_CTL_PAD_Reg<51, 1, 13> PUE;    // Pull / Keep Select Field
constexpr SW_PAD_CTL_PAD_Reg<51, 1, 12> PKE;    // Pull / Keep Enable Field
constexpr SW_PAD_CTL_PAD_Reg<51, 1, 11> ODE;    // Open Drain Enable Field
constexpr SW_PAD_CTL_PAD_Reg<51, 2,  6> SPEED;  // Speed Field
constexpr SW_PAD_CTL_PAD_Reg<51, 3,  3> DSE;    // Drive Strength Field
constexpr SW_PAD_CTL_PAD_Reg<51, 1,  0> SRE;    // Slew Rate Field
}  // namespace AD_B0_09
namespace AD_B0_10 {
constexpr SW_PAD_CTL_PAD_Reg<52, 1, 16> HYS;    // Hyst. Enable Field
constexpr SW_PAD_CTL_PAD_Reg<52, 2, 14> PUS;    // Pull Up / Down Config. Field
constexpr SW_PAD_CTL_PAD_Reg<52, 1, 13> PUE;    // Pull / Keep Select Field
constexpr SW_PAD_CTL_PAD_Reg<52, 1, 12> PKE;    // Pull / Keep Enable Field
constexpr SW_PAD_CTL_PAD_Reg<52, 1, 11> ODE;    // Open Drain Enable Field
constexpr SW_PAD_CTL_PAD_Reg<52, 2,  6> SPEED;  // Speed Field
constexpr SW_PAD_CTL_PAD_Reg<52, 3,  3> DSE;    // Drive Strength Field
constexpr SW_PAD_CTL_PAD_Reg<52, 1,  0> SRE;    // Slew Rate Field
}  // namespace AD_B0_10
namespace AD_B0_11 {
constexpr SW_PAD_CTL_PAD_Reg<53, 1, 16> HYS;    // Hyst. Enable Field
constexpr SW_PAD_CTL_PAD_Reg<53, 2, 14> PUS;    // Pull Up / Down Config. Field
constexpr SW_PAD_CTL_PAD_Reg<53, 1, 13> PUE;    // Pull / Keep Select Field
constexpr SW_PAD_CTL_PAD_Reg<53, 1, 12> PKE;    // Pull / Keep Enable Field
constexpr SW_PAD_CTL_PAD_Reg<53, 1, 11> ODE;    // Open Drain Enable Field
constexpr SW_PAD_CTL_PAD_Reg<53, 2,  6> SPEED;  // Speed Field
constexpr SW_PAD_CTL_PAD_Reg<53, 3,  3> DSE;    // Drive Strength Field
constexpr SW_PAD_CTL_PAD_Reg<53, 1,  0> SRE;    // Slew Rate Field
}  // namespace AD_B0_11
namespace AD_B0_12 {
constexpr SW_PAD_CTL_PAD_Reg<54, 1, 16> HYS;    // Hyst. Enable Field
constexpr SW_PAD_CTL_PAD_Reg<54, 2, 14> PUS;    // Pull Up / Down Config. Field
constexpr SW_PAD_CTL_PAD_Reg<54, 1, 13> PUE;    // Pull / Keep Select Field
constexpr SW_PAD_CTL_PAD_Reg<54, 1, 12> PKE;    // Pull / Keep Enable Field
constexpr SW_PAD_CTL_PAD_Reg<54, 1, 11> ODE;    // Open Drain Enable Field
constexpr SW_PAD_CTL_PAD_Reg<54, 2,  6> SPEED;  // Speed Field
constexpr SW_PAD_CTL_PAD_Reg<54, 3,  3> DSE;    // Drive Strength Field
constexpr SW_PAD_CTL_PAD_Reg<54, 1,  0> SRE;    // Slew Rate Field
}  // namespace AD_B0_12
namespace AD_B0_13 {
constexpr SW_PAD_CTL_PAD_Reg<55, 1, 16> HYS;    // Hyst. Enable Field
constexpr SW_PAD_CTL_PAD_Reg<55, 2, 14> PUS;    // Pull Up / Down Config. Field
constexpr SW_PAD_CTL_PAD_Reg<55, 1, 13> PUE;    // Pull / Keep Select Field
constexpr SW_PAD_CTL_PAD_Reg<55, 1, 12> PKE;    // Pull / Keep Enable Field
constexpr SW_PAD_CTL_PAD_Reg<55, 1, 11> ODE;    // Open Drain Enable Field
constexpr SW_PAD_CTL_PAD_Reg<55, 2,  6> SPEED;  // Speed Field
constexpr SW_PAD_CTL_PAD_Reg<55, 3,  3> DSE;    // Drive Strength Field
constexpr SW_PAD_CTL_PAD_Reg<55, 1,  0> SRE;    // Slew Rate Field
}  // namespace AD_B0_13
namespace AD_B0_14 {
constexpr SW_PAD_CTL_PAD_Reg<56, 1, 16> HYS;    // Hyst. Enable Field
constexpr SW_PAD_CTL_PAD_Reg<56, 2, 14> PUS;    // Pull Up / Down Config. Field
constexpr SW_PAD_CTL_PAD_Reg<56, 1, 13> PUE;    // Pull / Keep Select Field
constexpr SW_PAD_CTL_PAD_Reg<56, 1, 12> PKE;    // Pull / Keep Enable Field
constexpr SW_PAD_CTL_PAD_Reg<56, 1, 11> ODE;    // Open Drain Enable Field
constexpr SW_PAD_CTL_PAD_Reg<56, 2,  6> SPEED;  // Speed Field
constexpr SW_PAD_CTL_PAD_Reg<56, 3,  3> DSE;    // Drive Strength Field
constexpr SW_PAD_CTL_PAD_Reg<56, 1,  0> SRE;    // Slew Rate Field
}  // namespace AD_B0_14
namespace AD_B0_15 {
constexpr SW_PAD_CTL_PAD_Reg<57, 1, 16> HYS;    // Hyst. Enable Field
constexpr SW_PAD_CTL_PAD_Reg<57, 2, 14> PUS;    // Pull Up / Down Config. Field
constexpr SW_PAD_CTL_PAD_Reg<57, 1, 13> PUE;    // Pull / Keep Select Field
constexpr SW_PAD_CTL_PAD_Reg<57, 1, 12> PKE;    // Pull / Keep Enable Field
constexpr SW_PAD_CTL_PAD_Reg<57, 1, 11> ODE;    // Open Drain Enable Field
constexpr SW_PAD_CTL_PAD_Reg<57, 2,  6> SPEED;  // Speed Field
constexpr SW_PAD_CTL_PAD_Reg<57, 3,  3> DSE;    // Drive Strength Field
constexpr SW_PAD_CTL_PAD_Reg<57, 1,  0> SRE;    // Slew Rate Field
}  // namespace AD_B0_15

namespace AD_B1_00 {
constexpr SW_PAD_CTL_PAD_Reg<58, 1, 16> HYS;    // Hyst. Enable Field
constexpr SW_PAD_CTL_PAD_Reg<58, 2, 14> PUS;    // Pull Up / Down Config. Field
constexpr SW_PAD_CTL_PAD_Reg<58, 1, 13> PUE;    // Pull / Keep Select Field
constexpr SW_PAD_CTL_PAD_Reg<58, 1, 12> PKE;    // Pull / Keep Enable Field
constexpr SW_PAD_CTL_PAD_Reg<58, 1, 11> ODE;    // Open Drain Enable Field
constexpr SW_PAD_CTL_PAD_Reg<58, 2,  6> SPEED;  // Speed Field
constexpr SW_PAD_CTL_PAD_Reg<58, 3,  3> DSE;    // Drive Strength Field
constexpr SW_PAD_CTL_PAD_Reg<58, 1,  0> SRE;    // Slew Rate Field
}  // namespace AD_B1_00
namespace AD_B1_01 {
constexpr SW_PAD_CTL_PAD_Reg<59, 1, 16> HYS;    // Hyst. Enable Field
constexpr SW_PAD_CTL_PAD_Reg<59, 2, 14> PUS;    // Pull Up / Down Config. Field
constexpr SW_PAD_CTL_PAD_Reg<59, 1, 13> PUE;    // Pull / Keep Select Field
constexpr SW_PAD_CTL_PAD_Reg<59, 1, 12> PKE;    // Pull / Keep Enable Field
constexpr SW_PAD_CTL_PAD_Reg<59, 1, 11> ODE;    // Open Drain Enable Field
constexpr SW_PAD_CTL_PAD_Reg<59, 2,  6> SPEED;  // Speed Field
constexpr SW_PAD_CTL_PAD_Reg<59, 3,  3> DSE;    // Drive Strength Field
constexpr SW_PAD_CTL_PAD_Reg<59, 1,  0> SRE;    // Slew Rate Field
}  // namespace AD_B1_01
namespace AD_B1_02 {
constexpr SW_PAD_CTL_PAD_Reg<60, 1, 16> HYS;    // Hyst. Enable Field
constexpr SW_PAD_CTL_PAD_Reg<60, 2, 14> PUS;    // Pull Up / Down Config. Field
constexpr SW_PAD_CTL_PAD_Reg<60, 1, 13> PUE;    // Pull / Keep Select Field
constexpr SW_PAD_CTL_PAD_Reg<60, 1, 12> PKE;    // Pull / Keep Enable Field
constexpr SW_PAD_CTL_PAD_Reg<60, 1, 11> ODE;    // Open Drain Enable Field
constexpr SW_PAD_CTL_PAD_Reg<60, 2,  6> SPEED;  // Speed Field
constexpr SW_PAD_CTL_PAD_Reg<60, 3,  3> DSE;    // Drive Strength Field
constexpr SW_PAD_CTL_PAD_Reg<60, 1,  0> SRE;    // Slew Rate Field
}  // namespace AD_B1_02
namespace AD_B1_03 {
constexpr SW_PAD_CTL_PAD_Reg<61, 1, 16> HYS;    // Hyst. Enable Field
constexpr SW_PAD_CTL_PAD_Reg<61, 2, 14> PUS;    // Pull Up / Down Config. Field
constexpr SW_PAD_CTL_PAD_Reg<61, 1, 13> PUE;    // Pull / Keep Select Field
constexpr SW_PAD_CTL_PAD_Reg<61, 1, 12> PKE;    // Pull / Keep Enable Field
constexpr SW_PAD_CTL_PAD_Reg<61, 1, 11> ODE;    // Open Drain Enable Field
constexpr SW_PAD_CTL_PAD_Reg<61, 2,  6> SPEED;  // Speed Field
constexpr SW_PAD_CTL_PAD_Reg<61, 3,  3> DSE;    // Drive Strength Field
constexpr SW_PAD_CTL_PAD_Reg<61, 1,  0> SRE;    // Slew Rate Field
}  // namespace AD_B1_03
namespace AD_B1_04 {
constexpr SW_PAD_CTL_PAD_Reg<62, 1, 16> HYS;    // Hyst. Enable Field
constexpr SW_PAD_CTL_PAD_Reg<62, 2, 14> PUS;    // Pull Up / Down Config. Field
constexpr SW_PAD_CTL_PAD_Reg<62, 1, 13> PUE;    // Pull / Keep Select Field
constexpr SW_PAD_CTL_PAD_Reg<62, 1, 12> PKE;    // Pull / Keep Enable Field
constexpr SW_PAD_CTL_PAD_Reg<62, 1, 11> ODE;    // Open Drain Enable Field
constexpr SW_PAD_CTL_PAD_Reg<62, 2,  6> SPEED;  // Speed Field
constexpr SW_PAD_CTL_PAD_Reg<62, 3,  3> DSE;    // Drive Strength Field
constexpr SW_PAD_CTL_PAD_Reg<62, 1,  0> SRE;    // Slew Rate Field
}  // namespace AD_B1_04
namespace AD_B1_05 {
constexpr SW_PAD_CTL_PAD_Reg<63, 1, 16> HYS;    // Hyst. Enable Field
constexpr SW_PAD_CTL_PAD_Reg<63, 2, 14> PUS;    // Pull Up / Down Config. Field
constexpr SW_PAD_CTL_PAD_Reg<63, 1, 13> PUE;    // Pull / Keep Select Field
constexpr SW_PAD_CTL_PAD_Reg<63, 1, 12> PKE;    // Pull / Keep Enable Field
constexpr SW_PAD_CTL_PAD_Reg<63, 1, 11> ODE;    // Open Drain Enable Field
constexpr SW_PAD_CTL_PAD_Reg<63, 2,  6> SPEED;  // Speed Field
constexpr SW_PAD_CTL_PAD_Reg<63, 3,  3> DSE;    // Drive Strength Field
constexpr SW_PAD_CTL_PAD_Reg<63, 1,  0> SRE;    // Slew Rate Field
}  // namespace AD_B1_05
namespace AD_B1_06 {
constexpr SW_PAD_CTL_PAD_Reg<64, 1, 16> HYS;    // Hyst. Enable Field
constexpr SW_PAD_CTL_PAD_Reg<64, 2, 14> PUS;    // Pull Up / Down Config. Field
constexpr SW_PAD_CTL_PAD_Reg<64, 1, 13> PUE;    // Pull / Keep Select Field
constexpr SW_PAD_CTL_PAD_Reg<64, 1, 12> PKE;    // Pull / Keep Enable Field
constexpr SW_PAD_CTL_PAD_Reg<64, 1, 11> ODE;    // Open Drain Enable Field
constexpr SW_PAD_CTL_PAD_Reg<64, 2,  6> SPEED;  // Speed Field
constexpr SW_PAD_CTL_PAD_Reg<64, 3,  3> DSE;    // Drive Strength Field
constexpr SW_PAD_CTL_PAD_Reg<64, 1,  0> SRE;    // Slew Rate Field
}  // namespace AD_B1_06
namespace AD_B1_07 {
constexpr SW_PAD_CTL_PAD_Reg<65, 1, 16> HYS;    // Hyst. Enable Field
constexpr SW_PAD_CTL_PAD_Reg<65, 2, 14> PUS;    // Pull Up / Down Config. Field
constexpr SW_PAD_CTL_PAD_Reg<65, 1, 13> PUE;    // Pull / Keep Select Field
constexpr SW_PAD_CTL_PAD_Reg<65, 1, 12> PKE;    // Pull / Keep Enable Field
constexpr SW_PAD_CTL_PAD_Reg<65, 1, 11> ODE;    // Open Drain Enable Field
constexpr SW_PAD_CTL_PAD_Reg<65, 2,  6> SPEED;  // Speed Field
constexpr SW_PAD_CTL_PAD_Reg<65, 3,  3> DSE;    // Drive Strength Field
constexpr SW_PAD_CTL_PAD_Reg<65, 1,  0> SRE;    // Slew Rate Field
}  // namespace AD_B1_07
namespace AD_B1_08 {
constexpr SW_PAD_CTL_PAD_Reg<66, 1, 16> HYS;    // Hyst. Enable Field
constexpr SW_PAD_CTL_PAD_Reg<66, 2, 14> PUS;    // Pull Up / Down Config. Field
constexpr SW_PAD_CTL_PAD_Reg<66, 1, 13> PUE;    // Pull / Keep Select Field
constexpr SW_PAD_CTL_PAD_Reg<66, 1, 12> PKE;    // Pull / Keep Enable Field
constexpr SW_PAD_CTL_PAD_Reg<66, 1, 11> ODE;    // Open Drain Enable Field
constexpr SW_PAD_CTL_PAD_Reg<66, 2,  6> SPEED;  // Speed Field
constexpr SW_PAD_CTL_PAD_Reg<66, 3,  3> DSE;    // Drive Strength Field
constexpr SW_PAD_CTL_PAD_Reg<66, 1,  0> SRE;    // Slew Rate Field
}  // namespace AD_B1_08
namespace AD_B1_09 {
constexpr SW_PAD_CTL_PAD_Reg<67, 1, 16> HYS;    // Hyst. Enable Field
constexpr SW_PAD_CTL_PAD_Reg<67, 2, 14> PUS;    // Pull Up / Down Config. Field
constexpr SW_PAD_CTL_PAD_Reg<67, 1, 13> PUE;    // Pull / Keep Select Field
constexpr SW_PAD_CTL_PAD_Reg<67, 1, 12> PKE;    // Pull / Keep Enable Field
constexpr SW_PAD_CTL_PAD_Reg<67, 1, 11> ODE;    // Open Drain Enable Field
constexpr SW_PAD_CTL_PAD_Reg<67, 2,  6> SPEED;  // Speed Field
constexpr SW_PAD_CTL_PAD_Reg<67, 3,  3> DSE;    // Drive Strength Field
constexpr SW_PAD_CTL_PAD_Reg<67, 1,  0> SRE;    // Slew Rate Field
}  // namespace AD_B1_09
namespace AD_B1_10 {
constexpr SW_PAD_CTL_PAD_Reg<68, 1, 16> HYS;    // Hyst. Enable Field
constexpr SW_PAD_CTL_PAD_Reg<68, 2, 14> PUS;    // Pull Up / Down Config. Field
constexpr SW_PAD_CTL_PAD_Reg<68, 1, 13> PUE;    // Pull / Keep Select Field
constexpr SW_PAD_CTL_PAD_Reg<68, 1, 12> PKE;    // Pull / Keep Enable Field
constexpr SW_PAD_CTL_PAD_Reg<68, 1, 11> ODE;    // Open Drain Enable Field
constexpr SW_PAD_CTL_PAD_Reg<68, 2,  6> SPEED;  // Speed Field
constexpr SW_PAD_CTL_PAD_Reg<68, 3,  3> DSE;    // Drive Strength Field
constexpr SW_PAD_CTL_PAD_Reg<68, 1,  0> SRE;    // Slew Rate Field
}  // namespace AD_B1_10
namespace AD_B1_11 {
constexpr SW_PAD_CTL_PAD_Reg<69, 1, 16> HYS;    // Hyst. Enable Field
constexpr SW_PAD_CTL_PAD_Reg<69, 2, 14> PUS;    // Pull Up / Down Config. Field
constexpr SW_PAD_CTL_PAD_Reg<69, 1, 13> PUE;    // Pull / Keep Select Field
constexpr SW_PAD_CTL_PAD_Reg<69, 1, 12> PKE;    // Pull / Keep Enable Field
constexpr SW_PAD_CTL_PAD_Reg<69, 1, 11> ODE;    // Open Drain Enable Field
constexpr SW_PAD_CTL_PAD_Reg<69, 2,  6> SPEED;  // Speed Field
constexpr SW_PAD_CTL_PAD_Reg<69, 3,  3> DSE;    // Drive Strength Field
constexpr SW_PAD_CTL_PAD_Reg<69, 1,  0> SRE;    // Slew Rate Field
}  // namespace AD_B1_11
namespace AD_B1_12 {
constexpr SW_PAD_CTL_PAD_Reg<70, 1, 16> HYS;    // Hyst. Enable Field
constexpr SW_PAD_CTL_PAD_Reg<70, 2, 14> PUS;    // Pull Up / Down Config. Field
constexpr SW_PAD_CTL_PAD_Reg<70, 1, 13> PUE;    // Pull / Keep Select Field
constexpr SW_PAD_CTL_PAD_Reg<70, 1, 12> PKE;    // Pull / Keep Enable Field
constexpr SW_PAD_CTL_PAD_Reg<70, 1, 11> ODE;    // Open Drain Enable Field
constexpr SW_PAD_CTL_PAD_Reg<70, 2,  6> SPEED;  // Speed Field
constexpr SW_PAD_CTL_PAD_Reg<70, 3,  3> DSE;    // Drive Strength Field
constexpr SW_PAD_CTL_PAD_Reg<70, 1,  0> SRE;    // Slew Rate Field
}  // namespace AD_B1_12
namespace AD_B1_13 {
constexpr SW_PAD_CTL_PAD_Reg<71, 1, 16> HYS;    // Hyst. Enable Field
constexpr SW_PAD_CTL_PAD_Reg<71, 2, 14> PUS;    // Pull Up / Down Config. Field
constexpr SW_PAD_CTL_PAD_Reg<71, 1, 13> PUE;    // Pull / Keep Select Field
constexpr SW_PAD_CTL_PAD_Reg<71, 1, 12> PKE;    // Pull / Keep Enable Field
constexpr SW_PAD_CTL_PAD_Reg<71, 1, 11> ODE;    // Open Drain Enable Field
constexpr SW_PAD_CTL_PAD_Reg<71, 2,  6> SPEED;  // Speed Field
constexpr SW_PAD_CTL_PAD_Reg<71, 3,  3> DSE;    // Drive Strength Field
constexpr SW_PAD_CTL_PAD_Reg<71, 1,  0> SRE;    // Slew Rate Field
}  // namespace AD_B1_13
namespace AD_B1_14 {
constexpr SW_PAD_CTL_PAD_Reg<72, 1, 16> HYS;    // Hyst. Enable Field
constexpr SW_PAD_CTL_PAD_Reg<72, 2, 14> PUS;    // Pull Up / Down Config. Field
constexpr SW_PAD_CTL_PAD_Reg<72, 1, 13> PUE;    // Pull / Keep Select Field
constexpr SW_PAD_CTL_PAD_Reg<72, 1, 12> PKE;    // Pull / Keep Enable Field
constexpr SW_PAD_CTL_PAD_Reg<72, 1, 11> ODE;    // Open Drain Enable Field
constexpr SW_PAD_CTL_PAD_Reg<72, 2,  6> SPEED;  // Speed Field
constexpr SW_PAD_CTL_PAD_Reg<72, 3,  3> DSE;    // Drive Strength Field
constexpr SW_PAD_CTL_PAD_Reg<72, 1,  0> SRE;    // Slew Rate Field
}  // namespace AD_B1_14
namespace AD_B1_15 {
constexpr SW_PAD_CTL_PAD_Reg<73, 1, 16> HYS;    // Hyst. Enable Field
constexpr SW_PAD_CTL_PAD_Reg<73, 2, 14> PUS;    // Pull Up / Down Config. Field
constexpr SW_PAD_CTL_PAD_Reg<73, 1, 13> PUE;    // Pull / Keep Select Field
constexpr SW_PAD_CTL_PAD_Reg<73, 1, 12> PKE;    // Pull / Keep Enable Field
constexpr SW_PAD_CTL_PAD_Reg<73, 1, 11> ODE;    // Open Drain Enable Field
constexpr SW_PAD_CTL_PAD_Reg<73, 2,  6> SPEED;  // Speed Field
constexpr SW_PAD_CTL_PAD_Reg<73, 3,  3> DSE;    // Drive Strength Field
constexpr SW_PAD_CTL_PAD_Reg<73, 1,  0> SRE;    // Slew Rate Field
}  // namespace AD_B1_15

namespace B0_00 {
constexpr SW_PAD_CTL_PAD_Reg<74, 1, 16> HYS;    // Hyst. Enable Field
constexpr SW_PAD_CTL_PAD_Reg<74, 2, 14> PUS;    // Pull Up / Down Config. Field
constexpr SW_PAD_CTL_PAD_Reg<74, 1, 13> PUE;    // Pull / Keep Select Field
constexpr SW_PAD_CTL_PAD_Reg<74, 1, 12> PKE;    // Pull / Keep Enable Field
constexpr SW_PAD_CTL_PAD_Reg<74, 1, 11> ODE;    // Open Drain Enable Field
constexpr SW_PAD_CTL_PAD_Reg<74, 2,  6> SPEED;  // Speed Field
constexpr SW_PAD_CTL_PAD_Reg<74, 3,  3> DSE;    // Drive Strength Field
constexpr SW_PAD_CTL_PAD_Reg<74, 1,  0> SRE;    // Slew Rate Field
}  // namespace B0_00
namespace B0_01 {
constexpr SW_PAD_CTL_PAD_Reg<75, 1, 16> HYS;    // Hyst. Enable Field
constexpr SW_PAD_CTL_PAD_Reg<75, 2, 14> PUS;    // Pull Up / Down Config. Field
constexpr SW_PAD_CTL_PAD_Reg<75, 1, 13> PUE;    // Pull / Keep Select Field
constexpr SW_PAD_CTL_PAD_Reg<75, 1, 12> PKE;    // Pull / Keep Enable Field
constexpr SW_PAD_CTL_PAD_Reg<75, 1, 11> ODE;    // Open Drain Enable Field
constexpr SW_PAD_CTL_PAD_Reg<75, 2,  6> SPEED;  // Speed Field
constexpr SW_PAD_CTL_PAD_Reg<75, 3,  3> DSE;    // Drive Strength Field
constexpr SW_PAD_CTL_PAD_Reg<75, 1,  0> SRE;    // Slew Rate Field
}  // namespace B0_01
namespace B0_02 {
constexpr SW_PAD_CTL_PAD_Reg<76, 1, 16> HYS;    // Hyst. Enable Field
constexpr SW_PAD_CTL_PAD_Reg<76, 2, 14> PUS;    // Pull Up / Down Config. Field
constexpr SW_PAD_CTL_PAD_Reg<76, 1, 13> PUE;    // Pull / Keep Select Field
constexpr SW_PAD_CTL_PAD_Reg<76, 1, 12> PKE;    // Pull / Keep Enable Field
constexpr SW_PAD_CTL_PAD_Reg<76, 1, 11> ODE;    // Open Drain Enable Field
constexpr SW_PAD_CTL_PAD_Reg<76, 2,  6> SPEED;  // Speed Field
constexpr SW_PAD_CTL_PAD_Reg<76, 3,  3> DSE;    // Drive Strength Field
constexpr SW_PAD_CTL_PAD_Reg<76, 1,  0> SRE;    // Slew Rate Field
}  // namespace B0_02
namespace B0_03 {
constexpr SW_PAD_CTL_PAD_Reg<77, 1, 16> HYS;    // Hyst. Enable Field
constexpr SW_PAD_CTL_PAD_Reg<77, 2, 14> PUS;    // Pull Up / Down Config. Field
constexpr SW_PAD_CTL_PAD_Reg<77, 1, 13> PUE;    // Pull / Keep Select Field
constexpr SW_PAD_CTL_PAD_Reg<77, 1, 12> PKE;    // Pull / Keep Enable Field
constexpr SW_PAD_CTL_PAD_Reg<77, 1, 11> ODE;    // Open Drain Enable Field
constexpr SW_PAD_CTL_PAD_Reg<77, 2,  6> SPEED;  // Speed Field
constexpr SW_PAD_CTL_PAD_Reg<77, 3,  3> DSE;    // Drive Strength Field
constexpr SW_PAD_CTL_PAD_Reg<77, 1,  0> SRE;    // Slew Rate Field
}  // namespace B0_03
namespace B0_04 {
constexpr SW_PAD_CTL_PAD_Reg<78, 1, 16> HYS;    // Hyst. Enable Field
constexpr SW_PAD_CTL_PAD_Reg<78, 2, 14> PUS;    // Pull Up / Down Config. Field
constexpr SW_PAD_CTL_PAD_Reg<78, 1, 13> PUE;    // Pull / Keep Select Field
constexpr SW_PAD_CTL_PAD_Reg<78, 1, 12> PKE;    // Pull / Keep Enable Field
constexpr SW_PAD_CTL_PAD_Reg<78, 1, 11> ODE;    // Open Drain Enable Field
constexpr SW_PAD_CTL_PAD_Reg<78, 2,  6> SPEED;  // Speed Field
constexpr SW_PAD_CTL_PAD_Reg<78, 3,  3> DSE;    // Drive Strength Field
constexpr SW_PAD_CTL_PAD_Reg<78, 1,  0> SRE;    // Slew Rate Field
}  // namespace B0_04
namespace B0_05 {
constexpr SW_PAD_CTL_PAD_Reg<79, 1, 16> HYS;    // Hyst. Enable Field
constexpr SW_PAD_CTL_PAD_Reg<79, 2, 14> PUS;    // Pull Up / Down Config. Field
constexpr SW_PAD_CTL_PAD_Reg<79, 1, 13> PUE;    // Pull / Keep Select Field
constexpr SW_PAD_CTL_PAD_Reg<79, 1, 12> PKE;    // Pull / Keep Enable Field
constexpr SW_PAD_CTL_PAD_Reg<79, 1, 11> ODE;    // Open Drain Enable Field
constexpr SW_PAD_CTL_PAD_Reg<79, 2,  6> SPEED;  // Speed Field
constexpr SW_PAD_CTL_PAD_Reg<79, 3,  3> DSE;    // Drive Strength Field
constexpr SW_PAD_CTL_PAD_Reg<79, 1,  0> SRE;    // Slew Rate Field
}  // namespace B0_05
namespace B0_06 {
constexpr SW_PAD_CTL_PAD_Reg<80, 1, 16> HYS;    // Hyst. Enable Field
constexpr SW_PAD_CTL_PAD_Reg<80, 2, 14> PUS;    // Pull Up / Down Config. Field
constexpr SW_PAD_CTL_PAD_Reg<80, 1, 13> PUE;    // Pull / Keep Select Field
constexpr SW_PAD_CTL_PAD_Reg<80, 1, 12> PKE;    // Pull / Keep Enable Field
constexpr SW_PAD_CTL_PAD_Reg<80, 1, 11> ODE;    // Open Drain Enable Field
constexpr SW_PAD_CTL_PAD_Reg<80, 2,  6> SPEED;  // Speed Field
constexpr SW_PAD_CTL_PAD_Reg<80, 3,  3> DSE;    // Drive Strength Field
constexpr SW_PAD_CTL_PAD_Reg<80, 1,  0> SRE;    // Slew Rate Field
}  // namespace B0_06
namespace B0_07 {
constexpr SW_PAD_CTL_PAD_Reg<81, 1, 16> HYS;    // Hyst. Enable Field
constexpr SW_PAD_CTL_PAD_Reg<81, 2, 14> PUS;    // Pull Up / Down Config. Field
constexpr SW_PAD_CTL_PAD_Reg<81, 1, 13> PUE;    // Pull / Keep Select Field
constexpr SW_PAD_CTL_PAD_Reg<81, 1, 12> PKE;    // Pull / Keep Enable Field
constexpr SW_PAD_CTL_PAD_Reg<81, 1, 11> ODE;    // Open Drain Enable Field
constexpr SW_PAD_CTL_PAD_Reg<81, 2,  6> SPEED;  // Speed Field
constexpr SW_PAD_CTL_PAD_Reg<81, 3,  3> DSE;    // Drive Strength Field
constexpr SW_PAD_CTL_PAD_Reg<81, 1,  0> SRE;    // Slew Rate Field
}  // namespace B0_07
namespace B0_08 {
constexpr SW_PAD_CTL_PAD_Reg<82, 1, 16> HYS;    // Hyst. Enable Field
constexpr SW_PAD_CTL_PAD_Reg<82, 2, 14> PUS;    // Pull Up / Down Config. Field
constexpr SW_PAD_CTL_PAD_Reg<82, 1, 13> PUE;    // Pull / Keep Select Field
constexpr SW_PAD_CTL_PAD_Reg<82, 1, 12> PKE;    // Pull / Keep Enable Field
constexpr SW_PAD_CTL_PAD_Reg<82, 1, 11> ODE;    // Open Drain Enable Field
constexpr SW_PAD_CTL_PAD_Reg<82, 2,  6> SPEED;  // Speed Field
constexpr SW_PAD_CTL_PAD_Reg<82, 3,  3> DSE;    // Drive Strength Field
constexpr SW_PAD_CTL_PAD_Reg<82, 1,  0> SRE;    // Slew Rate Field
}  // namespace B0_08
namespace B0_09 {
constexpr SW_PAD_CTL_PAD_Reg<83, 1, 16> HYS;    // Hyst. Enable Field
constexpr SW_PAD_CTL_PAD_Reg<83, 2, 14> PUS;    // Pull Up / Down Config. Field
constexpr SW_PAD_CTL_PAD_Reg<83, 1, 13> PUE;    // Pull / Keep Select Field
constexpr SW_PAD_CTL_PAD_Reg<83, 1, 12> PKE;    // Pull / Keep Enable Field
constexpr SW_PAD_CTL_PAD_Reg<83, 1, 11> ODE;    // Open Drain Enable Field
constexpr SW_PAD_CTL_PAD_Reg<83, 2,  6> SPEED;  // Speed Field
constexpr SW_PAD_CTL_PAD_Reg<83, 3,  3> DSE;    // Drive Strength Field
constexpr SW_PAD_CTL_PAD_Reg<83, 1,  0> SRE;    // Slew Rate Field
}  // namespace B0_09
namespace B0_10 {
constexpr SW_PAD_CTL_PAD_Reg<84, 1, 16> HYS;    // Hyst. Enable Field
constexpr SW_PAD_CTL_PAD_Reg<84, 2, 14> PUS;    // Pull Up / Down Config. Field
constexpr SW_PAD_CTL_PAD_Reg<84, 1, 13> PUE;    // Pull / Keep Select Field
constexpr SW_PAD_CTL_PAD_Reg<84, 1, 12> PKE;    // Pull / Keep Enable Field
constexpr SW_PAD_CTL_PAD_Reg<84, 1, 11> ODE;    // Open Drain Enable Field
constexpr SW_PAD_CTL_PAD_Reg<84, 2,  6> SPEED;  // Speed Field
constexpr SW_PAD_CTL_PAD_Reg<84, 3,  3> DSE;    // Drive Strength Field
constexpr SW_PAD_CTL_PAD_Reg<84, 1,  0> SRE;    // Slew Rate Field
}  // namespace B0_10
namespace B0_11 {
constexpr SW_PAD_CTL_PAD_Reg<85, 1, 16> HYS;    // Hyst. Enable Field
constexpr SW_PAD_CTL_PAD_Reg<85, 2, 14> PUS;    // Pull Up / Down Config. Field
constexpr SW_PAD_CTL_PAD_Reg<85, 1, 13> PUE;    // Pull / Keep Select Field
constexpr SW_PAD_CTL_PAD_Reg<85, 1, 12> PKE;    // Pull / Keep Enable Field
constexpr SW_PAD_CTL_PAD_Reg<85, 1, 11> ODE;    // Open Drain Enable Field
constexpr SW_PAD_CTL_PAD_Reg<85, 2,  6> SPEED;  // Speed Field
constexpr SW_PAD_CTL_PAD_Reg<85, 3,  3> DSE;    // Drive Strength Field
constexpr SW_PAD_CTL_PAD_Reg<85, 1,  0> SRE;    // Slew Rate Field
}  // namespace B0_11
namespace B0_12 {
constexpr SW_PAD_CTL_PAD_Reg<86, 1, 16> HYS;    // Hyst. Enable Field
constexpr SW_PAD_CTL_PAD_Reg<86, 2, 14> PUS;    // Pull Up / Down Config. Field
constexpr SW_PAD_CTL_PAD_Reg<86, 1, 13> PUE;    // Pull / Keep Select Field
constexpr SW_PAD_CTL_PAD_Reg<86, 1, 12> PKE;    // Pull / Keep Enable Field
constexpr SW_PAD_CTL_PAD_Reg<86, 1, 11> ODE;    // Open Drain Enable Field
constexpr SW_PAD_CTL_PAD_Reg<86, 2,  6> SPEED;  // Speed Field
constexpr SW_PAD_CTL_PAD_Reg<86, 3,  3> DSE;    // Drive Strength Field
constexpr SW_PAD_CTL_PAD_Reg<86, 1,  0> SRE;    // Slew Rate Field
}  // namespace B0_12
namespace B0_13 {
constexpr SW_PAD_CTL_PAD_Reg<87, 1, 16> HYS;    // Hyst. Enable Field
constexpr SW_PAD_CTL_PAD_Reg<87, 2, 14> PUS;    // Pull Up / Down Config. Field
constexpr SW_PAD_CTL_PAD_Reg<87, 1, 13> PUE;    // Pull / Keep Select Field
constexpr SW_PAD_CTL_PAD_Reg<87, 1, 12> PKE;    // Pull / Keep Enable Field
constexpr SW_PAD_CTL_PAD_Reg<87, 1, 11> ODE;    // Open Drain Enable Field
constexpr SW_PAD_CTL_PAD_Reg<87, 2,  6> SPEED;  // Speed Field
constexpr SW_PAD_CTL_PAD_Reg<87, 3,  3> DSE;    // Drive Strength Field
constexpr SW_PAD_CTL_PAD_Reg<87, 1,  0> SRE;    // Slew Rate Field
}  // namespace B0_13
namespace B0_14 {
constexpr SW_PAD_CTL_PAD_Reg<88, 1, 16> HYS;    // Hyst. Enable Field
constexpr SW_PAD_CTL_PAD_Reg<88, 2, 14> PUS;    // Pull Up / Down Config. Field
constexpr SW_PAD_CTL_PAD_Reg<88, 1, 13> PUE;    // Pull / Keep Select Field
constexpr SW_PAD_CTL_PAD_Reg<88, 1, 12> PKE;    // Pull / Keep Enable Field
constexpr SW_PAD_CTL_PAD_Reg<88, 1, 11> ODE;    // Open Drain Enable Field
constexpr SW_PAD_CTL_PAD_Reg<88, 2,  6> SPEED;  // Speed Field
constexpr SW_PAD_CTL_PAD_Reg<88, 3,  3> DSE;    // Drive Strength Field
constexpr SW_PAD_CTL_PAD_Reg<88, 1,  0> SRE;    // Slew Rate Field
}  // namespace B0_14
namespace B0_15 {
constexpr SW_PAD_CTL_PAD_Reg<89, 1, 16> HYS;    // Hyst. Enable Field
constexpr SW_PAD_CTL_PAD_Reg<89, 2, 14> PUS;    // Pull Up / Down Config. Field
constexpr SW_PAD_CTL_PAD_Reg<89, 1, 13> PUE;    // Pull / Keep Select Field
constexpr SW_PAD_CTL_PAD_Reg<89, 1, 12> PKE;    // Pull / Keep Enable Field
constexpr SW_PAD_CTL_PAD_Reg<89, 1, 11> ODE;    // Open Drain Enable Field
constexpr SW_PAD_CTL_PAD_Reg<89, 2,  6> SPEED;  // Speed Field
constexpr SW_PAD_CTL_PAD_Reg<89, 3,  3> DSE;    // Drive Strength Field
constexpr SW_PAD_CTL_PAD_Reg<89, 1,  0> SRE;    // Slew Rate Field
}  // namespace B0_15

namespace B1_00 {
constexpr SW_PAD_CTL_PAD_Reg<90, 1, 16> HYS;    // Hyst. Enable Field
constexpr SW_PAD_CTL_PAD_Reg<90, 2, 14> PUS;    // Pull Up / Down Config. Field
constexpr SW_PAD_CTL_PAD_Reg<90, 1, 13> PUE;    // Pull / Keep Select Field
constexpr SW_PAD_CTL_PAD_Reg<90, 1, 12> PKE;    // Pull / Keep Enable Field
constexpr SW_PAD_CTL_PAD_Reg<90, 1, 11> ODE;    // Open Drain Enable Field
constexpr SW_PAD_CTL_PAD_Reg<90, 2,  6> SPEED;  // Speed Field
constexpr SW_PAD_CTL_PAD_Reg<90, 3,  3> DSE;    // Drive Strength Field
constexpr SW_PAD_CTL_PAD_Reg<90, 1,  0> SRE;    // Slew Rate Field
}  // namespace B1_00
namespace B1_01 {
constexpr SW_PAD_CTL_PAD_Reg<91, 1, 16> HYS;    // Hyst. Enable Field
constexpr SW_PAD_CTL_PAD_Reg<91, 2, 14> PUS;    // Pull Up / Down Config. Field
constexpr SW_PAD_CTL_PAD_Reg<91, 1, 13> PUE;    // Pull / Keep Select Field
constexpr SW_PAD_CTL_PAD_Reg<91, 1, 12> PKE;    // Pull / Keep Enable Field
constexpr SW_PAD_CTL_PAD_Reg<91, 1, 11> ODE;    // Open Drain Enable Field
constexpr SW_PAD_CTL_PAD_Reg<91, 2,  6> SPEED;  // Speed Field
constexpr SW_PAD_CTL_PAD_Reg<91, 3,  3> DSE;    // Drive Strength Field
constexpr SW_PAD_CTL_PAD_Reg<91, 1,  0> SRE;    // Slew Rate Field
}  // namespace B1_01
namespace B1_02 {
constexpr SW_PAD_CTL_PAD_Reg<92, 1, 16> HYS;    // Hyst. Enable Field
constexpr SW_PAD_CTL_PAD_Reg<92, 2, 14> PUS;    // Pull Up / Down Config. Field
constexpr SW_PAD_CTL_PAD_Reg<92, 1, 13> PUE;    // Pull / Keep Select Field
constexpr SW_PAD_CTL_PAD_Reg<92, 1, 12> PKE;    // Pull / Keep Enable Field
constexpr SW_PAD_CTL_PAD_Reg<92, 1, 11> ODE;    // Open Drain Enable Field
constexpr SW_PAD_CTL_PAD_Reg<92, 2,  6> SPEED;  // Speed Field
constexpr SW_PAD_CTL_PAD_Reg<92, 3,  3> DSE;    // Drive Strength Field
constexpr SW_PAD_CTL_PAD_Reg<92, 1,  0> SRE;    // Slew Rate Field
}  // namespace B1_02
namespace B1_03 {
constexpr SW_PAD_CTL_PAD_Reg<93, 1, 16> HYS;    // Hyst. Enable Field
constexpr SW_PAD_CTL_PAD_Reg<93, 2, 14> PUS;    // Pull Up / Down Config. Field
constexpr SW_PAD_CTL_PAD_Reg<93, 1, 13> PUE;    // Pull / Keep Select Field
constexpr SW_PAD_CTL_PAD_Reg<93, 1, 12> PKE;    // Pull / Keep Enable Field
constexpr SW_PAD_CTL_PAD_Reg<93, 1, 11> ODE;    // Open Drain Enable Field
constexpr SW_PAD_CTL_PAD_Reg<93, 2,  6> SPEED;  // Speed Field
constexpr SW_PAD_CTL_PAD_Reg<93, 3,  3> DSE;    // Drive Strength Field
constexpr SW_PAD_CTL_PAD_Reg<93, 1,  0> SRE;    // Slew Rate Field
}  // namespace B1_03
namespace B1_04 {
constexpr SW_PAD_CTL_PAD_Reg<94, 1, 16> HYS;    // Hyst. Enable Field
constexpr SW_PAD_CTL_PAD_Reg<94, 2, 14> PUS;    // Pull Up / Down Config. Field
constexpr SW_PAD_CTL_PAD_Reg<94, 1, 13> PUE;    // Pull / Keep Select Field
constexpr SW_PAD_CTL_PAD_Reg<94, 1, 12> PKE;    // Pull / Keep Enable Field
constexpr SW_PAD_CTL_PAD_Reg<94, 1, 11> ODE;    // Open Drain Enable Field
constexpr SW_PAD_CTL_PAD_Reg<94, 2,  6> SPEED;  // Speed Field
constexpr SW_PAD_CTL_PAD_Reg<94, 3,  3> DSE;    // Drive Strength Field
constexpr SW_PAD_CTL_PAD_Reg<94, 1,  0> SRE;    // Slew Rate Field
}  // namespace B1_04
namespace B1_05 {
constexpr SW_PAD_CTL_PAD_Reg<95, 1, 16> HYS;    // Hyst. Enable Field
constexpr SW_PAD_CTL_PAD_Reg<95, 2, 14> PUS;    // Pull Up / Down Config. Field
constexpr SW_PAD_CTL_PAD_Reg<95, 1, 13> PUE;    // Pull / Keep Select Field
constexpr SW_PAD_CTL_PAD_Reg<95, 1, 12> PKE;    // Pull / Keep Enable Field
constexpr SW_PAD_CTL_PAD_Reg<95, 1, 11> ODE;    // Open Drain Enable Field
constexpr SW_PAD_CTL_PAD_Reg<95, 2,  6> SPEED;  // Speed Field
constexpr SW_PAD_CTL_PAD_Reg<95, 3,  3> DSE;    // Drive Strength Field
constexpr SW_PAD_CTL_PAD_Reg<95, 1,  0> SRE;    // Slew Rate Field
}  // namespace B1_05
namespace B1_06 {
constexpr SW_PAD_CTL_PAD_Reg<96, 1, 16> HYS;    // Hyst. Enable Field
constexpr SW_PAD_CTL_PAD_Reg<96, 2, 14> PUS;    // Pull Up / Down Config. Field
constexpr SW_PAD_CTL_PAD_Reg<96, 1, 13> PUE;    // Pull / Keep Select Field
constexpr SW_PAD_CTL_PAD_Reg<96, 1, 12> PKE;    // Pull / Keep Enable Field
constexpr SW_PAD_CTL_PAD_Reg<96, 1, 11> ODE;    // Open Drain Enable Field
constexpr SW_PAD_CTL_PAD_Reg<96, 2,  6> SPEED;  // Speed Field
constexpr SW_PAD_CTL_PAD_Reg<96, 3,  3> DSE;    // Drive Strength Field
constexpr SW_PAD_CTL_PAD_Reg<96, 1,  0> SRE;    // Slew Rate Field
}  // namespace B1_06
namespace B1_07 {
constexpr SW_PAD_CTL_PAD_Reg<97, 1, 16> HYS;    // Hyst. Enable Field
constexpr SW_PAD_CTL_PAD_Reg<97, 2, 14> PUS;    // Pull Up / Down Config. Field
constexpr SW_PAD_CTL_PAD_Reg<97, 1, 13> PUE;    // Pull / Keep Select Field
constexpr SW_PAD_CTL_PAD_Reg<97, 1, 12> PKE;    // Pull / Keep Enable Field
constexpr SW_PAD_CTL_PAD_Reg<97, 1, 11> ODE;    // Open Drain Enable Field
constexpr SW_PAD_CTL_PAD_Reg<97, 2,  6> SPEED;  // Speed Field
constexpr SW_PAD_CTL_PAD_Reg<97, 3,  3> DSE;    // Drive Strength Field
constexpr SW_PAD_CTL_PAD_Reg<97, 1,  0> SRE;    // Slew Rate Field
}  // namespace B1_07
namespace B1_08 {
constexpr SW_PAD_CTL_PAD_Reg<98, 1, 16> HYS;    // Hyst. Enable Field
constexpr SW_PAD_CTL_PAD_Reg<98, 2, 14> PUS;    // Pull Up / Down Config. Field
constexpr SW_PAD_CTL_PAD_Reg<98, 1, 13> PUE;    // Pull / Keep Select Field
constexpr SW_PAD_CTL_PAD_Reg<98, 1, 12> PKE;    // Pull / Keep Enable Field
constexpr SW_PAD_CTL_PAD_Reg<98, 1, 11> ODE;    // Open Drain Enable Field
constexpr SW_PAD_CTL_PAD_Reg<98, 2,  6> SPEED;  // Speed Field
constexpr SW_PAD_CTL_PAD_Reg<98, 3,  3> DSE;    // Drive Strength Field
constexpr SW_PAD_CTL_PAD_Reg<98, 1,  0> SRE;    // Slew Rate Field
}  // namespace B1_08
namespace B1_09 {
constexpr SW_PAD_CTL_PAD_Reg<99, 1, 16> HYS;    // Hyst. Enable Field
constexpr SW_PAD_CTL_PAD_Reg<99, 2, 14> PUS;    // Pull Up / Down Config. Field
constexpr SW_PAD_CTL_PAD_Reg<99, 1, 13> PUE;    // Pull / Keep Select Field
constexpr SW_PAD_CTL_PAD_Reg<99, 1, 12> PKE;    // Pull / Keep Enable Field
constexpr SW_PAD_CTL_PAD_Reg<99, 1, 11> ODE;    // Open Drain Enable Field
constexpr SW_PAD_CTL_PAD_Reg<99, 2,  6> SPEED;  // Speed Field
constexpr SW_PAD_CTL_PAD_Reg<99, 3,  3> DSE;    // Drive Strength Field
constexpr SW_PAD_CTL_PAD_Reg<99, 1,  0> SRE;    // Slew Rate Field
}  // namespace B1_09
namespace B1_10 {
constexpr SW_PAD_CTL_PAD_Reg<100, 1, 16> HYS;    // Hyst. Enable Field
constexpr SW_PAD_CTL_PAD_Reg<100, 2, 14> PUS;    // Pull Up / Down Config. Field
constexpr SW_PAD_CTL_PAD_Reg<100, 1, 13> PUE;    // Pull / Keep Select Field
constexpr SW_PAD_CTL_PAD_Reg<100, 1, 12> PKE;    // Pull / Keep Enable Field
constexpr SW_PAD_CTL_PAD_Reg<100, 1, 11> ODE;    // Open Drain Enable Field
constexpr SW_PAD_CTL_PAD_Reg<100, 2,  6> SPEED;  // Speed Field
constexpr SW_PAD_CTL_PAD_Reg<100, 3,  3> DSE;    // Drive Strength Field
constexpr SW_PAD_CTL_PAD_Reg<100, 1,  0> SRE;    // Slew Rate Field
}  // namespace B1_10
namespace B1_11 {
constexpr SW_PAD_CTL_PAD_Reg<101, 1, 16> HYS;    // Hyst. Enable Field
constexpr SW_PAD_CTL_PAD_Reg<101, 2, 14> PUS;    // Pull Up / Down Config. Field
constexpr SW_PAD_CTL_PAD_Reg<101, 1, 13> PUE;    // Pull / Keep Select Field
constexpr SW_PAD_CTL_PAD_Reg<101, 1, 12> PKE;    // Pull / Keep Enable Field
constexpr SW_PAD_CTL_PAD_Reg<101, 1, 11> ODE;    // Open Drain Enable Field
constexpr SW_PAD_CTL_PAD_Reg<101, 2,  6> SPEED;  // Speed Field
constexpr SW_PAD_CTL_PAD_Reg<101, 3,  3> DSE;    // Drive Strength Field
constexpr SW_PAD_CTL_PAD_Reg<101, 1,  0> SRE;    // Slew Rate Field
}  // namespace B1_11
namespace B1_12 {
constexpr SW_PAD_CTL_PAD_Reg<102, 1, 16> HYS;    // Hyst. Enable Field
constexpr SW_PAD_CTL_PAD_Reg<102, 2, 14> PUS;    // Pull Up / Down Config. Field
constexpr SW_PAD_CTL_PAD_Reg<102, 1, 13> PUE;    // Pull / Keep Select Field
constexpr SW_PAD_CTL_PAD_Reg<102, 1, 12> PKE;    // Pull / Keep Enable Field
constexpr SW_PAD_CTL_PAD_Reg<102, 1, 11> ODE;    // Open Drain Enable Field
constexpr SW_PAD_CTL_PAD_Reg<102, 2,  6> SPEED;  // Speed Field
constexpr SW_PAD_CTL_PAD_Reg<102, 3,  3> DSE;    // Drive Strength Field
constexpr SW_PAD_CTL_PAD_Reg<102, 1,  0> SRE;    // Slew Rate Field
}  // namespace B1_12
namespace B1_13 {
constexpr SW_PAD_CTL_PAD_Reg<103, 1, 16> HYS;    // Hyst. Enable Field
constexpr SW_PAD_CTL_PAD_Reg<103, 2, 14> PUS;    // Pull Up / Down Config. Field
constexpr SW_PAD_CTL_PAD_Reg<103, 1, 13> PUE;    // Pull / Keep Select Field
constexpr SW_PAD_CTL_PAD_Reg<103, 1, 12> PKE;    // Pull / Keep Enable Field
constexpr SW_PAD_CTL_PAD_Reg<103, 1, 11> ODE;    // Open Drain Enable Field
constexpr SW_PAD_CTL_PAD_Reg<103, 2,  6> SPEED;  // Speed Field
constexpr SW_PAD_CTL_PAD_Reg<103, 3,  3> DSE;    // Drive Strength Field
constexpr SW_PAD_CTL_PAD_Reg<103, 1,  0> SRE;    // Slew Rate Field
}  // namespace B1_13
namespace B1_14 {
constexpr SW_PAD_CTL_PAD_Reg<104, 1, 16> HYS;    // Hyst. Enable Field
constexpr SW_PAD_CTL_PAD_Reg<104, 2, 14> PUS;    // Pull Up / Down Config. Field
constexpr SW_PAD_CTL_PAD_Reg<104, 1, 13> PUE;    // Pull / Keep Select Field
constexpr SW_PAD_CTL_PAD_Reg<104, 1, 12> PKE;    // Pull / Keep Enable Field
constexpr SW_PAD_CTL_PAD_Reg<104, 1, 11> ODE;    // Open Drain Enable Field
constexpr SW_PAD_CTL_PAD_Reg<104, 2,  6> SPEED;  // Speed Field
constexpr SW_PAD_CTL_PAD_Reg<104, 3,  3> DSE;    // Drive Strength Field
constexpr SW_PAD_CTL_PAD_Reg<104, 1,  0> SRE;    // Slew Rate Field
}  // namespace B1_14
namespace B1_15 {
constexpr SW_PAD_CTL_PAD_Reg<105, 1, 16> HYS;    // Hyst. Enable Field
constexpr SW_PAD_CTL_PAD_Reg<105, 2, 14> PUS;    // Pull Up / Down Config. Field
constexpr SW_PAD_CTL_PAD_Reg<105, 1, 13> PUE;    // Pull / Keep Select Field
constexpr SW_PAD_CTL_PAD_Reg<105, 1, 12> PKE;    // Pull / Keep Enable Field
constexpr SW_PAD_CTL_PAD_Reg<105, 1, 11> ODE;    // Open Drain Enable Field
constexpr SW_PAD_CTL_PAD_Reg<105, 2,  6> SPEED;  // Speed Field
constexpr SW_PAD_CTL_PAD_Reg<105, 3,  3> DSE;    // Drive Strength Field
constexpr SW_PAD_CTL_PAD_Reg<105, 1,  0> SRE;    // Slew Rate Field
}  // namespace B1_15

namespace SD_B0_00 {
constexpr SW_PAD_CTL_PAD_Reg<106, 1, 16> HYS;    // Hyst. Enable Field
constexpr SW_PAD_CTL_PAD_Reg<106, 2, 14> PUS;    // Pull Up / Down Config. Field
constexpr SW_PAD_CTL_PAD_Reg<106, 1, 13> PUE;    // Pull / Keep Select Field
constexpr SW_PAD_CTL_PAD_Reg<106, 1, 12> PKE;    // Pull / Keep Enable Field
constexpr SW_PAD_CTL_PAD_Reg<106, 1, 11> ODE;    // Open Drain Enable Field
constexpr SW_PAD_CTL_PAD_Reg<106, 2,  6> SPEED;  // Speed Field
constexpr SW_PAD_CTL_PAD_Reg<106, 3,  3> DSE;    // Drive Strength Field
constexpr SW_PAD_CTL_PAD_Reg<106, 1,  0> SRE;    // Slew Rate Field
}  // namespace SD_B0_00
namespace SD_B0_01 {
constexpr SW_PAD_CTL_PAD_Reg<107, 1, 16> HYS;    // Hyst. Enable Field
constexpr SW_PAD_CTL_PAD_Reg<107, 2, 14> PUS;    // Pull Up / Down Config. Field
constexpr SW_PAD_CTL_PAD_Reg<107, 1, 13> PUE;    // Pull / Keep Select Field
constexpr SW_PAD_CTL_PAD_Reg<107, 1, 12> PKE;    // Pull / Keep Enable Field
constexpr SW_PAD_CTL_PAD_Reg<107, 1, 11> ODE;    // Open Drain Enable Field
constexpr SW_PAD_CTL_PAD_Reg<107, 2,  6> SPEED;  // Speed Field
constexpr SW_PAD_CTL_PAD_Reg<107, 3,  3> DSE;    // Drive Strength Field
constexpr SW_PAD_CTL_PAD_Reg<107, 1,  0> SRE;    // Slew Rate Field
}  // namespace SD_B0_01
namespace SD_B0_02 {
constexpr SW_PAD_CTL_PAD_Reg<108, 1, 16> HYS;    // Hyst. Enable Field
constexpr SW_PAD_CTL_PAD_Reg<108, 2, 14> PUS;    // Pull Up / Down Config. Field
constexpr SW_PAD_CTL_PAD_Reg<108, 1, 13> PUE;    // Pull / Keep Select Field
constexpr SW_PAD_CTL_PAD_Reg<108, 1, 12> PKE;    // Pull / Keep Enable Field
constexpr SW_PAD_CTL_PAD_Reg<108, 1, 11> ODE;    // Open Drain Enable Field
constexpr SW_PAD_CTL_PAD_Reg<108, 2,  6> SPEED;  // Speed Field
constexpr SW_PAD_CTL_PAD_Reg<108, 3,  3> DSE;    // Drive Strength Field
constexpr SW_PAD_CTL_PAD_Reg<108, 1,  0> SRE;    // Slew Rate Field
}  // namespace SD_B0_02
namespace SD_B0_03 {
constexpr SW_PAD_CTL_PAD_Reg<109, 1, 16> HYS;    // Hyst. Enable Field
constexpr SW_PAD_CTL_PAD_Reg<109, 2, 14> PUS;    // Pull Up / Down Config. Field
constexpr SW_PAD_CTL_PAD_Reg<109, 1, 13> PUE;    // Pull / Keep Select Field
constexpr SW_PAD_CTL_PAD_Reg<109, 1, 12> PKE;    // Pull / Keep Enable Field
constexpr SW_PAD_CTL_PAD_Reg<109, 1, 11> ODE;    // Open Drain Enable Field
constexpr SW_PAD_CTL_PAD_Reg<109, 2,  6> SPEED;  // Speed Field
constexpr SW_PAD_CTL_PAD_Reg<109, 3,  3> DSE;    // Drive Strength Field
constexpr SW_PAD_CTL_PAD_Reg<109, 1,  0> SRE;    // Slew Rate Field
}  // namespace SD_B0_03
namespace SD_B0_04 {
constexpr SW_PAD_CTL_PAD_Reg<110, 1, 16> HYS;    // Hyst. Enable Field
constexpr SW_PAD_CTL_PAD_Reg<110, 2, 14> PUS;    // Pull Up / Down Config. Field
constexpr SW_PAD_CTL_PAD_Reg<110, 1, 13> PUE;    // Pull / Keep Select Field
constexpr SW_PAD_CTL_PAD_Reg<110, 1, 12> PKE;    // Pull / Keep Enable Field
constexpr SW_PAD_CTL_PAD_Reg<110, 1, 11> ODE;    // Open Drain Enable Field
constexpr SW_PAD_CTL_PAD_Reg<110, 2,  6> SPEED;  // Speed Field
constexpr SW_PAD_CTL_PAD_Reg<110, 3,  3> DSE;    // Drive Strength Field
constexpr SW_PAD_CTL_PAD_Reg<110, 1,  0> SRE;    // Slew Rate Field
}  // namespace SD_B0_04
namespace SD_B0_05 {
constexpr SW_PAD_CTL_PAD_Reg<111, 1, 16> HYS;    // Hyst. Enable Field
constexpr SW_PAD_CTL_PAD_Reg<111, 2, 14> PUS;    // Pull Up / Down Config. Field
constexpr SW_PAD_CTL_PAD_Reg<111, 1, 13> PUE;    // Pull / Keep Select Field
constexpr SW_PAD_CTL_PAD_Reg<111, 1, 12> PKE;    // Pull / Keep Enable Field
constexpr SW_PAD_CTL_PAD_Reg<111, 1, 11> ODE;    // Open Drain Enable Field
constexpr SW_PAD_CTL_PAD_Reg<111, 2,  6> SPEED;  // Speed Field
constexpr SW_PAD_CTL_PAD_Reg<111, 3,  3> DSE;    // Drive Strength Field
constexpr SW_PAD_CTL_PAD_Reg<111, 1,  0> SRE;    // Slew Rate Field
}  // namespace SD_B0_05

namespace SD_B1_00 {
constexpr SW_PAD_CTL_PAD_Reg<112, 1, 16> HYS;    // Hyst. Enable Field
constexpr SW_PAD_CTL_PAD_Reg<112, 2, 14> PUS;    // Pull Up / Down Config. Field
constexpr SW_PAD_CTL_PAD_Reg<112, 1, 13> PUE;    // Pull / Keep Select Field
constexpr SW_PAD_CTL_PAD_Reg<112, 1, 12> PKE;    // Pull / Keep Enable Field
constexpr SW_PAD_CTL_PAD_Reg<112, 1, 11> ODE;    // Open Drain Enable Field
constexpr SW_PAD_CTL_PAD_Reg<112, 2,  6> SPEED;  // Speed Field
constexpr SW_PAD_CTL_PAD_Reg<112, 3,  3> DSE;    // Drive Strength Field
constexpr SW_PAD_CTL_PAD_Reg<112, 1,  0> SRE;    // Slew Rate Field
}  // namespace SD_B1_00
namespace SD_B1_01 {
constexpr SW_PAD_CTL_PAD_Reg<113, 1, 16> HYS;    // Hyst. Enable Field
constexpr SW_PAD_CTL_PAD_Reg<113, 2, 14> PUS;    // Pull Up / Down Config. Field
constexpr SW_PAD_CTL_PAD_Reg<113, 1, 13> PUE;    // Pull / Keep Select Field
constexpr SW_PAD_CTL_PAD_Reg<113, 1, 12> PKE;    // Pull / Keep Enable Field
constexpr SW_PAD_CTL_PAD_Reg<113, 1, 11> ODE;    // Open Drain Enable Field
constexpr SW_PAD_CTL_PAD_Reg<113, 2,  6> SPEED;  // Speed Field
constexpr SW_PAD_CTL_PAD_Reg<113, 3,  3> DSE;    // Drive Strength Field
constexpr SW_PAD_CTL_PAD_Reg<113, 1,  0> SRE;    // Slew Rate Field
}  // namespace SD_B1_01
namespace SD_B1_02 {
constexpr SW_PAD_CTL_PAD_Reg<114, 1, 16> HYS;    // Hyst. Enable Field
constexpr SW_PAD_CTL_PAD_Reg<114, 2, 14> PUS;    // Pull Up / Down Config. Field
constexpr SW_PAD_CTL_PAD_Reg<114, 1, 13> PUE;    // Pull / Keep Select Field
constexpr SW_PAD_CTL_PAD_Reg<114, 1, 12> PKE;    // Pull / Keep Enable Field
constexpr SW_PAD_CTL_PAD_Reg<114, 1, 11> ODE;    // Open Drain Enable Field
constexpr SW_PAD_CTL_PAD_Reg<114, 2,  6> SPEED;  // Speed Field
constexpr SW_PAD_CTL_PAD_Reg<114, 3,  3> DSE;    // Drive Strength Field
constexpr SW_PAD_CTL_PAD_Reg<114, 1,  0> SRE;    // Slew Rate Field
}  // namespace SD_B1_02
namespace SD_B1_03 {
constexpr SW_PAD_CTL_PAD_Reg<115, 1, 16> HYS;    // Hyst. Enable Field
constexpr SW_PAD_CTL_PAD_Reg<115, 2, 14> PUS;    // Pull Up / Down Config. Field
constexpr SW_PAD_CTL_PAD_Reg<115, 1, 13> PUE;    // Pull / Keep Select Field
constexpr SW_PAD_CTL_PAD_Reg<115, 1, 12> PKE;    // Pull / Keep Enable Field
constexpr SW_PAD_CTL_PAD_Reg<115, 1, 11> ODE;    // Open Drain Enable Field
constexpr SW_PAD_CTL_PAD_Reg<115, 2,  6> SPEED;  // Speed Field
constexpr SW_PAD_CTL_PAD_Reg<115, 3,  3> DSE;    // Drive Strength Field
constexpr SW_PAD_CTL_PAD_Reg<115, 1,  0> SRE;    // Slew Rate Field
}  // namespace SD_B1_03
namespace SD_B1_04 {
constexpr SW_PAD_CTL_PAD_Reg<116, 1, 16> HYS;    // Hyst. Enable Field
constexpr SW_PAD_CTL_PAD_Reg<116, 2, 14> PUS;    // Pull Up / Down Config. Field
constexpr SW_PAD_CTL_PAD_Reg<116, 1, 13> PUE;    // Pull / Keep Select Field
constexpr SW_PAD_CTL_PAD_Reg<116, 1, 12> PKE;    // Pull / Keep Enable Field
constexpr SW_PAD_CTL_PAD_Reg<116, 1, 11> ODE;    // Open Drain Enable Field
constexpr SW_PAD_CTL_PAD_Reg<116, 2,  6> SPEED;  // Speed Field
constexpr SW_PAD_CTL_PAD_Reg<116, 3,  3> DSE;    // Drive Strength Field
constexpr SW_PAD_CTL_PAD_Reg<116, 1,  0> SRE;    // Slew Rate Field
}  // namespace SD_B1_04
namespace SD_B1_05 {
constexpr SW_PAD_CTL_PAD_Reg<117, 1, 16> HYS;    // Hyst. Enable Field
constexpr SW_PAD_CTL_PAD_Reg<117, 2, 14> PUS;    // Pull Up / Down Config. Field
constexpr SW_PAD_CTL_PAD_Reg<117, 1, 13> PUE;    // Pull / Keep Select Field
constexpr SW_PAD_CTL_PAD_Reg<117, 1, 12> PKE;    // Pull / Keep Enable Field
constexpr SW_PAD_CTL_PAD_Reg<117, 1, 11> ODE;    // Open Drain Enable Field
constexpr SW_PAD_CTL_PAD_Reg<117, 2,  6> SPEED;  // Speed Field
constexpr SW_PAD_CTL_PAD_Reg<117, 3,  3> DSE;    // Drive Strength Field
constexpr SW_PAD_CTL_PAD_Reg<117, 1,  0> SRE;    // Slew Rate Field
}  // namespace SD_B1_05
namespace SD_B1_06 {
constexpr SW_PAD_CTL_PAD_Reg<118, 1, 16> HYS;    // Hyst. Enable Field
constexpr SW_PAD_CTL_PAD_Reg<118, 2, 14> PUS;    // Pull Up / Down Config. Field
constexpr SW_PAD_CTL_PAD_Reg<118, 1, 13> PUE;    // Pull / Keep Select Field
constexpr SW_PAD_CTL_PAD_Reg<118, 1, 12> PKE;    // Pull / Keep Enable Field
constexpr SW_PAD_CTL_PAD_Reg<118, 1, 11> ODE;    // Open Drain Enable Field
constexpr SW_PAD_CTL_PAD_Reg<118, 2,  6> SPEED;  // Speed Field
constexpr SW_PAD_CTL_PAD_Reg<118, 3,  3> DSE;    // Drive Strength Field
constexpr SW_PAD_CTL_PAD_Reg<118, 1,  0> SRE;    // Slew Rate Field
}  // namespace SD_B1_06
namespace SD_B1_07 {
constexpr SW_PAD_CTL_PAD_Reg<119, 1, 16> HYS;    // Hyst. Enable Field
constexpr SW_PAD_CTL_PAD_Reg<119, 2, 14> PUS;    // Pull Up / Down Config. Field
constexpr SW_PAD_CTL_PAD_Reg<119, 1, 13> PUE;    // Pull / Keep Select Field
constexpr SW_PAD_CTL_PAD_Reg<119, 1, 12> PKE;    // Pull / Keep Enable Field
constexpr SW_PAD_CTL_PAD_Reg<119, 1, 11> ODE;    // Open Drain Enable Field
constexpr SW_PAD_CTL_PAD_Reg<119, 2,  6> SPEED;  // Speed Field
constexpr SW_PAD_CTL_PAD_Reg<119, 3,  3> DSE;    // Drive Strength Field
constexpr SW_PAD_CTL_PAD_Reg<119, 1,  0> SRE;    // Slew Rate Field
}  // namespace SD_B1_07
namespace SD_B1_08 {
constexpr SW_PAD_CTL_PAD_Reg<120, 1, 16> HYS;    // Hyst. Enable Field
constexpr SW_PAD_CTL_PAD_Reg<120, 2, 14> PUS;    // Pull Up / Down Config. Field
constexpr SW_PAD_CTL_PAD_Reg<120, 1, 13> PUE;    // Pull / Keep Select Field
constexpr SW_PAD_CTL_PAD_Reg<120, 1, 12> PKE;    // Pull / Keep Enable Field
constexpr SW_PAD_CTL_PAD_Reg<120, 1, 11> ODE;    // Open Drain Enable Field
constexpr SW_PAD_CTL_PAD_Reg<120, 2,  6> SPEED;  // Speed Field
constexpr SW_PAD_CTL_PAD_Reg<120, 3,  3> DSE;    // Drive Strength Field
constexpr SW_PAD_CTL_PAD_Reg<120, 1,  0> SRE;    // Slew Rate Field
}  // namespace SD_B1_08
namespace SD_B1_09 {
constexpr SW_PAD_CTL_PAD_Reg<121, 1, 16> HYS;    // Hyst. Enable Field
constexpr SW_PAD_CTL_PAD_Reg<121, 2, 14> PUS;    // Pull Up / Down Config. Field
constexpr SW_PAD_CTL_PAD_Reg<121, 1, 13> PUE;    // Pull / Keep Select Field
constexpr SW_PAD_CTL_PAD_Reg<121, 1, 12> PKE;    // Pull / Keep Enable Field
constexpr SW_PAD_CTL_PAD_Reg<121, 1, 11> ODE;    // Open Drain Enable Field
constexpr SW_PAD_CTL_PAD_Reg<121, 2,  6> SPEED;  // Speed Field
constexpr SW_PAD_CTL_PAD_Reg<121, 3,  3> DSE;    // Drive Strength Field
constexpr SW_PAD_CTL_PAD_Reg<121, 1,  0> SRE;    // Slew Rate Field
}  // namespace SD_B1_09
namespace SD_B1_10 {
constexpr SW_PAD_CTL_PAD_Reg<122, 1, 16> HYS;    // Hyst. Enable Field
constexpr SW_PAD_CTL_PAD_Reg<122, 2, 14> PUS;    // Pull Up / Down Config. Field
constexpr SW_PAD_CTL_PAD_Reg<122, 1, 13> PUE;    // Pull / Keep Select Field
constexpr SW_PAD_CTL_PAD_Reg<122, 1, 12> PKE;    // Pull / Keep Enable Field
constexpr SW_PAD_CTL_PAD_Reg<122, 1, 11> ODE;    // Open Drain Enable Field
constexpr SW_PAD_CTL_PAD_Reg<122, 2,  6> SPEED;  // Speed Field
constexpr SW_PAD_CTL_PAD_Reg<122, 3,  3> DSE;    // Drive Strength Field
constexpr SW_PAD_CTL_PAD_Reg<122, 1,  0> SRE;    // Slew Rate Field
}  // namespace SD_B1_10
namespace SD_B1_11 {
constexpr SW_PAD_CTL_PAD_Reg<123, 1, 16> HYS;    // Hyst. Enable Field
constexpr SW_PAD_CTL_PAD_Reg<123, 2, 14> PUS;    // Pull Up / Down Config. Field
constexpr SW_PAD_CTL_PAD_Reg<123, 1, 13> PUE;    // Pull / Keep Select Field
constexpr SW_PAD_CTL_PAD_Reg<123, 1, 12> PKE;    // Pull / Keep Enable Field
constexpr SW_PAD_CTL_PAD_Reg<123, 1, 11> ODE;    // Open Drain Enable Field
constexpr SW_PAD_CTL_PAD_Reg<123, 2,  6> SPEED;  // Speed Field
constexpr SW_PAD_CTL_PAD_Reg<123, 3,  3> DSE;    // Drive Strength Field
constexpr SW_PAD_CTL_PAD_Reg<123, 1,  0> SRE;    // Slew Rate Field
}  // namespace SD_B1_11

}  // namespace GPIO
}  // namespace SW_PAD_CTL_PAD

namespace SELECT_INPUT {

namespace ANATOP_USB_OTG1_ID {
constexpr SELECT_INPUT_Reg<0, 1, 0> DAISY;  // Selecting Pads Involved in Daisy Chain.
}  // namespace ANATOP_USB_OTG1_ID
namespace ANATOP_USB_OTG2_ID {
constexpr SELECT_INPUT_Reg<1, 1, 0> DAISY;  // Selecting Pads Involved in Daisy Chain.
}  // namespace ANATOP_USB_OTG2_ID
namespace CCM_PMIC_READY {
constexpr SELECT_INPUT_Reg<2, 3, 0> DAISY;  // Selecting Pads Involved in Daisy Chain.
}  // namespace CCM_PMIC_READY
namespace CSI_DATA02 {
constexpr SELECT_INPUT_Reg<3, 1, 0> DAISY;  // Selecting Pads Involved in Daisy Chain.
}  // namespace CSI_DATA02
namespace CSI_DATA03 {
constexpr SELECT_INPUT_Reg<4, 1, 0> DAISY;  // Selecting Pads Involved in Daisy Chain.
}  // namespace CSI_DATA03
namespace CSI_DATA04 {
constexpr SELECT_INPUT_Reg<5, 1, 0> DAISY;  // Selecting Pads Involved in Daisy Chain.
}  // namespace CSI_DATA04
namespace CSI_DATA05 {
constexpr SELECT_INPUT_Reg<6, 1, 0> DAISY;  // Selecting Pads Involved in Daisy Chain.
}  // namespace CSI_DATA05
namespace CSI_DATA06 {
constexpr SELECT_INPUT_Reg<7, 1, 0> DAISY;  // Selecting Pads Involved in Daisy Chain.
}  // namespace CSI_DATA06
namespace CSI_DATA07 {
constexpr SELECT_INPUT_Reg<8, 1, 0> DAISY;  // Selecting Pads Involved in Daisy Chain.
}  // namespace CSI_DATA07
namespace CSI_DATA08 {
constexpr SELECT_INPUT_Reg<9, 1, 0> DAISY;  // Selecting Pads Involved in Daisy Chain.
}  // namespace CSI_DATA08
namespace CSI_DATA09 {
constexpr SELECT_INPUT_Reg<10, 1, 0> DAISY;  // Selecting Pads Involved in Daisy Chain.
}  // namespace CSI_DATA09
namespace CSI_HSYNC {
constexpr SELECT_INPUT_Reg<11, 2, 0> DAISY;  // Selecting Pads Involved in Daisy Chain.
}  // namespace CSI_HSYNC
namespace CSI_PIXCLK {
constexpr SELECT_INPUT_Reg<12, 1, 0> DAISY;  // Selecting Pads Involved in Daisy Chain.
}  // namespace CSI_PIXCLK
namespace CSI_VSYNC {
constexpr SELECT_INPUT_Reg<13, 2, 0> DAISY;  // Selecting Pads Involved in Daisy Chain.
}  // namespace CSI_VSYNC
namespace ENET_IPG_CLK_RMII {
constexpr SELECT_INPUT_Reg<14, 1, 0> DAISY;  // Selecting Pads Involved in Daisy Chain.
}  // namespace ENET_IPG_CLK_RMII
namespace ENET_MDIO {
constexpr SELECT_INPUT_Reg<15, 2, 0> DAISY;  // Selecting Pads Involved in Daisy Chain.
}  // namespace ENET_MDIO
namespace ENET0_RXDATA {
constexpr SELECT_INPUT_Reg<16, 1, 0> DAISY;  // Selecting Pads Involved in Daisy Chain.
}  // namespace ENET0_RXDATA
namespace ENET1_RXDATA {
constexpr SELECT_INPUT_Reg<17, 1, 0> DAISY;  // Selecting Pads Involved in Daisy Chain.
}  // namespace ENET1_RXDATA
namespace ENET_RXEN {
constexpr SELECT_INPUT_Reg<18, 1, 0> DAISY;  // Selecting Pads Involved in Daisy Chain.
}  // namespace ENET_RXEN
namespace ENET_RXERR {
constexpr SELECT_INPUT_Reg<19, 1, 0> DAISY;  // Selecting Pads Involved in Daisy Chain.
}  // namespace ENET_RXERR
namespace ENET0_TIMER {
constexpr SELECT_INPUT_Reg<20, 2, 0> DAISY;  // Selecting Pads Involved in Daisy Chain.
}  // namespace ENET0_TIMER
namespace ENET_TXCLK {
constexpr SELECT_INPUT_Reg<21, 1, 0> DAISY;  // Selecting Pads Involved in Daisy Chain.
}  // namespace ENET_TXCLK
namespace FLEXCAN1_RX {
constexpr SELECT_INPUT_Reg<22, 2, 0> DAISY;  // Selecting Pads Involved in Daisy Chain.
}  // namespace FLEXCAN1_RX
namespace FLEXCAN2_RX {
constexpr SELECT_INPUT_Reg<23, 2, 0> DAISY;  // Selecting Pads Involved in Daisy Chain.
}  // namespace FLEXCAN2_RX
namespace FLEXPWM1_PWMA3 {
constexpr SELECT_INPUT_Reg<24, 3, 0> DAISY;  // Selecting Pads Involved in Daisy Chain.
}  // namespace FLEXPWM1_PWMA3
namespace FLEXPWM1_PWMA0 {
constexpr SELECT_INPUT_Reg<25, 1, 0> DAISY;  // Selecting Pads Involved in Daisy Chain.
}  // namespace FLEXPWM1_PWMA0
namespace FLEXPWM1_PWMA1 {
constexpr SELECT_INPUT_Reg<26, 1, 0> DAISY;  // Selecting Pads Involved in Daisy Chain.
}  // namespace FLEXPWM1_PWMA1
namespace FLEXPWM1_PWMA2 {
constexpr SELECT_INPUT_Reg<27, 1, 0> DAISY;  // Selecting Pads Involved in Daisy Chain.
}  // namespace FLEXPWM1_PWMA2
namespace FLEXPWM1_PWMB3 {
constexpr SELECT_INPUT_Reg<28, 3, 0> DAISY;  // Selecting Pads Involved in Daisy Chain.
}  // namespace FLEXPWM1_PWMB3
namespace FLEXPWM1_PWMB0 {
constexpr SELECT_INPUT_Reg<29, 1, 0> DAISY;  // Selecting Pads Involved in Daisy Chain.
}  // namespace FLEXPWM1_PWMB0
namespace FLEXPWM1_PWMB1 {
constexpr SELECT_INPUT_Reg<30, 1, 0> DAISY;  // Selecting Pads Involved in Daisy Chain.
}  // namespace FLEXPWM1_PWMB1
namespace FLEXPWM1_PWMB2 {
constexpr SELECT_INPUT_Reg<31, 1, 0> DAISY;  // Selecting Pads Involved in Daisy Chain.
}  // namespace FLEXPWM1_PWMB2
namespace FLEXPWM2_PWMA3 {
constexpr SELECT_INPUT_Reg<32, 3, 0> DAISY;  // Selecting Pads Involved in Daisy Chain.
}  // namespace FLEXPWM2_PWMA3
namespace FLEXPWM2_PWMA0 {
constexpr SELECT_INPUT_Reg<33, 1, 0> DAISY;  // Selecting Pads Involved in Daisy Chain.
}  // namespace FLEXPWM2_PWMA0
namespace FLEXPWM2_PWMA1 {
constexpr SELECT_INPUT_Reg<34, 1, 0> DAISY;  // Selecting Pads Involved in Daisy Chain.
}  // namespace FLEXPWM2_PWMA1
namespace FLEXPWM2_PWMA2 {
constexpr SELECT_INPUT_Reg<35, 1, 0> DAISY;  // Selecting Pads Involved in Daisy Chain.
}  // namespace FLEXPWM2_PWMA2
namespace FLEXPWM2_PWMB3 {
constexpr SELECT_INPUT_Reg<36, 2, 0> DAISY;  // Selecting Pads Involved in Daisy Chain.
}  // namespace FLEXPWM2_PWMB3
namespace FLEXPWM2_PWMB0 {
constexpr SELECT_INPUT_Reg<37, 1, 0> DAISY;  // Selecting Pads Involved in Daisy Chain.
}  // namespace FLEXPWM2_PWMB0
namespace FLEXPWM2_PWMB1 {
constexpr SELECT_INPUT_Reg<38, 1, 0> DAISY;  // Selecting Pads Involved in Daisy Chain.
}  // namespace FLEXPWM2_PWMB1
namespace FLEXPWM2_PWMB2 {
constexpr SELECT_INPUT_Reg<39, 1, 0> DAISY;  // Selecting Pads Involved in Daisy Chain.
}  // namespace FLEXPWM2_PWMB2
namespace FLEXPWM4_PWMA0 {
constexpr SELECT_INPUT_Reg<40, 1, 0> DAISY;  // Selecting Pads Involved in Daisy Chain.
}  // namespace FLEXPWM4_PWMA0
namespace FLEXPWM4_PWMA1 {
constexpr SELECT_INPUT_Reg<41, 1, 0> DAISY;  // Selecting Pads Involved in Daisy Chain.
}  // namespace FLEXPWM4_PWMA1
namespace FLEXPWM4_PWMA2 {
constexpr SELECT_INPUT_Reg<42, 1, 0> DAISY;  // Selecting Pads Involved in Daisy Chain.
}  // namespace FLEXPWM4_PWMA2
namespace FLEXPWM4_PWMA3 {
constexpr SELECT_INPUT_Reg<43, 1, 0> DAISY;  // Selecting Pads Involved in Daisy Chain.
}  // namespace FLEXPWM4_PWMA3
namespace FLEXSPIA_DQS {
constexpr SELECT_INPUT_Reg<44, 1, 0> DAISY;  // Selecting Pads Involved in Daisy Chain.
}  // namespace FLEXSPIA_DQS
namespace FLEXSPIA_DATA0 {
constexpr SELECT_INPUT_Reg<45, 1, 0> DAISY;  // Selecting Pads Involved in Daisy Chain.
}  // namespace FLEXSPIA_DATA0
namespace FLEXSPIA_DATA1 {
constexpr SELECT_INPUT_Reg<46, 1, 0> DAISY;  // Selecting Pads Involved in Daisy Chain.
}  // namespace FLEXSPIA_DATA1
namespace FLEXSPIA_DATA2 {
constexpr SELECT_INPUT_Reg<47, 1, 0> DAISY;  // Selecting Pads Involved in Daisy Chain.
}  // namespace FLEXSPIA_DATA2
namespace FLEXSPIA_DATA3 {
constexpr SELECT_INPUT_Reg<48, 1, 0> DAISY;  // Selecting Pads Involved in Daisy Chain.
}  // namespace FLEXSPIA_DATA3
namespace FLEXSPIB_DATA0 {
constexpr SELECT_INPUT_Reg<49, 1, 0> DAISY;  // Selecting Pads Involved in Daisy Chain.
}  // namespace FLEXSPIB_DATA0
namespace FLEXSPIB_DATA1 {
constexpr SELECT_INPUT_Reg<50, 1, 0> DAISY;  // Selecting Pads Involved in Daisy Chain.
}  // namespace FLEXSPIB_DATA1
namespace FLEXSPIB_DATA2 {
constexpr SELECT_INPUT_Reg<51, 1, 0> DAISY;  // Selecting Pads Involved in Daisy Chain.
}  // namespace FLEXSPIB_DATA2
namespace FLEXSPIB_DATA3 {
constexpr SELECT_INPUT_Reg<52, 1, 0> DAISY;  // Selecting Pads Involved in Daisy Chain.
}  // namespace FLEXSPIB_DATA3
namespace FLEXSPIA_SCK {
constexpr SELECT_INPUT_Reg<53, 1, 0> DAISY;  // Selecting Pads Involved in Daisy Chain.
}  // namespace FLEXSPIA_SCK
namespace LPI2C1_SCL {
constexpr SELECT_INPUT_Reg<54, 1, 0> DAISY;  // Selecting Pads Involved in Daisy Chain.
}  // namespace LPI2C1_SCL
namespace LPI2C1_SDA {
constexpr SELECT_INPUT_Reg<55, 1, 0> DAISY;  // Selecting Pads Involved in Daisy Chain.
}  // namespace LPI2C1_SDA
namespace LPI2C2_SCL {
constexpr SELECT_INPUT_Reg<56, 1, 0> DAISY;  // Selecting Pads Involved in Daisy Chain.
}  // namespace LPI2C2_SCL
namespace LPI2C2_SDA {
constexpr SELECT_INPUT_Reg<57, 1, 0> DAISY;  // Selecting Pads Involved in Daisy Chain.
}  // namespace LPI2C2_SDA
namespace LPI2C3_SCL {
constexpr SELECT_INPUT_Reg<58, 2, 0> DAISY;  // Selecting Pads Involved in Daisy Chain.
}  // namespace LPI2C3_SCL
namespace LPI2C3_SDA {
constexpr SELECT_INPUT_Reg<59, 2, 0> DAISY;  // Selecting Pads Involved in Daisy Chain.
}  // namespace LPI2C3_SDA
namespace LPI2C4_SCL {
constexpr SELECT_INPUT_Reg<60, 1, 0> DAISY;  // Selecting Pads Involved in Daisy Chain.
}  // namespace LPI2C4_SCL
namespace LPI2C4_SDA {
constexpr SELECT_INPUT_Reg<61, 1, 0> DAISY;  // Selecting Pads Involved in Daisy Chain.
}  // namespace LPI2C4_SDA
namespace LPSPI1_PCS0 {
constexpr SELECT_INPUT_Reg<62, 1, 0> DAISY;  // Selecting Pads Involved in Daisy Chain.
}  // namespace LPSPI1_PCS0
namespace LPSPI1_SCK {
constexpr SELECT_INPUT_Reg<63, 1, 0> DAISY;  // Selecting Pads Involved in Daisy Chain.
}  // namespace LPSPI1_SCK
namespace LPSPI1_SDI {
constexpr SELECT_INPUT_Reg<64, 1, 0> DAISY;  // Selecting Pads Involved in Daisy Chain.
}  // namespace LPSPI1_SDI
namespace LPSPI1_SDO {
constexpr SELECT_INPUT_Reg<65, 1, 0> DAISY;  // Selecting Pads Involved in Daisy Chain.
}  // namespace LPSPI1_SDO
namespace LPSPI2_PCS0 {
constexpr SELECT_INPUT_Reg<66, 1, 0> DAISY;  // Selecting Pads Involved in Daisy Chain.
}  // namespace LPSPI2_PCS0
namespace LPSPI2_SCK {
constexpr SELECT_INPUT_Reg<67, 1, 0> DAISY;  // Selecting Pads Involved in Daisy Chain.
}  // namespace LPSPI2_SCK
namespace LPSPI2_SDI {
constexpr SELECT_INPUT_Reg<68, 1, 0> DAISY;  // Selecting Pads Involved in Daisy Chain.
}  // namespace LPSPI2_SDI
namespace LPSPI2_SDO {
constexpr SELECT_INPUT_Reg<69, 1, 0> DAISY;  // Selecting Pads Involved in Daisy Chain.
}  // namespace LPSPI2_SDO
namespace LPSPI3_PCS0 {
constexpr SELECT_INPUT_Reg<70, 1, 0> DAISY;  // Selecting Pads Involved in Daisy Chain.
}  // namespace LPSPI3_PCS0
namespace LPSPI3_SCK {
constexpr SELECT_INPUT_Reg<71, 1, 0> DAISY;  // Selecting Pads Involved in Daisy Chain.
}  // namespace LPSPI3_SCK
namespace LPSPI3_SDI {
constexpr SELECT_INPUT_Reg<72, 1, 0> DAISY;  // Selecting Pads Involved in Daisy Chain.
}  // namespace LPSPI3_SDI
namespace LPSPI3_SDO {
constexpr SELECT_INPUT_Reg<73, 1, 0> DAISY;  // Selecting Pads Involved in Daisy Chain.
}  // namespace LPSPI3_SDO
namespace LPSPI4_PCS0 {
constexpr SELECT_INPUT_Reg<74, 1, 0> DAISY;  // Selecting Pads Involved in Daisy Chain.
}  // namespace LPSPI4_PCS0
namespace LPSPI4_SCK {
constexpr SELECT_INPUT_Reg<75, 1, 0> DAISY;  // Selecting Pads Involved in Daisy Chain.
}  // namespace LPSPI4_SCK
namespace LPSPI4_SDI {
constexpr SELECT_INPUT_Reg<76, 1, 0> DAISY;  // Selecting Pads Involved in Daisy Chain.
}  // namespace LPSPI4_SDI
namespace LPSPI4_SDO {
constexpr SELECT_INPUT_Reg<77, 1, 0> DAISY;  // Selecting Pads Involved in Daisy Chain.
}  // namespace LPSPI4_SDO
namespace LPUART2_RX {
constexpr SELECT_INPUT_Reg<78, 1, 0> DAISY;  // Selecting Pads Involved in Daisy Chain.
}  // namespace LPUART2_RX
namespace LPUART2_TX {
constexpr SELECT_INPUT_Reg<79, 1, 0> DAISY;  // Selecting Pads Involved in Daisy Chain.
}  // namespace LPUART2_TX
namespace LPUART3_CTS_B {
constexpr SELECT_INPUT_Reg<80, 1, 0> DAISY;  // Selecting Pads Involved in Daisy Chain.
}  // namespace LPUART3_CTS_B
namespace LPUART3_RX {
constexpr SELECT_INPUT_Reg<81, 2, 0> DAISY;  // Selecting Pads Involved in Daisy Chain.
}  // namespace LPUART3_RX
namespace LPUART3_TX {
constexpr SELECT_INPUT_Reg<82, 2, 0> DAISY;  // Selecting Pads Involved in Daisy Chain.
}  // namespace LPUART3_TX
namespace LPUART4_RX {
constexpr SELECT_INPUT_Reg<83, 2, 0> DAISY;  // Selecting Pads Involved in Daisy Chain.
}  // namespace LPUART4_RX
namespace LPUART4_TX {
constexpr SELECT_INPUT_Reg<84, 2, 0> DAISY;  // Selecting Pads Involved in Daisy Chain.
}  // namespace LPUART4_TX
namespace LPUART5_RX {
constexpr SELECT_INPUT_Reg<85, 1, 0> DAISY;  // Selecting Pads Involved in Daisy Chain.
}  // namespace LPUART5_RX
namespace LPUART5_TX {
constexpr SELECT_INPUT_Reg<86, 1, 0> DAISY;  // Selecting Pads Involved in Daisy Chain.
}  // namespace LPUART5_TX
namespace LPUART6_RX {
constexpr SELECT_INPUT_Reg<87, 1, 0> DAISY;  // Selecting Pads Involved in Daisy Chain.
}  // namespace LPUART6_RX
namespace LPUART6_TX {
constexpr SELECT_INPUT_Reg<88, 1, 0> DAISY;  // Selecting Pads Involved in Daisy Chain.
}  // namespace LPUART6_TX
namespace LPUART7_RX {
constexpr SELECT_INPUT_Reg<89, 1, 0> DAISY;  // Selecting Pads Involved in Daisy Chain.
}  // namespace LPUART7_RX
namespace LPUART7_TX {
constexpr SELECT_INPUT_Reg<90, 1, 0> DAISY;  // Selecting Pads Involved in Daisy Chain.
}  // namespace LPUART7_TX
namespace LPUART8_RX {
constexpr SELECT_INPUT_Reg<91, 2, 0> DAISY;  // Selecting Pads Involved in Daisy Chain.
}  // namespace LPUART8_RX
namespace LPUART8_TX {
constexpr SELECT_INPUT_Reg<92, 2, 0> DAISY;  // Selecting Pads Involved in Daisy Chain.
}  // namespace LPUART8_TX
namespace NMI {
constexpr SELECT_INPUT_Reg<93, 1, 0> DAISY;  // Selecting Pads Involved in Daisy Chain.
}  // namespace NMI
namespace QTIMER2_TIMER0 {
constexpr SELECT_INPUT_Reg<94, 1, 0> DAISY;  // Selecting Pads Involved in Daisy Chain.
}  // namespace QTIMER2_TIMER0
namespace QTIMER2_TIMER1 {
constexpr SELECT_INPUT_Reg<95, 1, 0> DAISY;  // Selecting Pads Involved in Daisy Chain.
}  // namespace QTIMER2_TIMER1
namespace QTIMER2_TIMER2 {
constexpr SELECT_INPUT_Reg<96, 1, 0> DAISY;  // Selecting Pads Involved in Daisy Chain.
}  // namespace QTIMER2_TIMER2
namespace QTIMER2_TIMER3 {
constexpr SELECT_INPUT_Reg<97, 1, 0> DAISY;  // Selecting Pads Involved in Daisy Chain.
}  // namespace QTIMER2_TIMER3
namespace QTIMER3_TIMER0 {
constexpr SELECT_INPUT_Reg<98, 2, 0> DAISY;  // Selecting Pads Involved in Daisy Chain.
}  // namespace QTIMER3_TIMER0
namespace QTIMER3_TIMER1 {
constexpr SELECT_INPUT_Reg<99, 2, 0> DAISY;  // Selecting Pads Involved in Daisy Chain.
}  // namespace QTIMER3_TIMER1
namespace QTIMER3_TIMER2 {
constexpr SELECT_INPUT_Reg<100, 2, 0> DAISY;  // Selecting Pads Involved in Daisy Chain.
}  // namespace QTIMER3_TIMER2
namespace QTIMER3_TIMER3 {
constexpr SELECT_INPUT_Reg<101, 2, 0> DAISY;  // Selecting Pads Involved in Daisy Chain.
}  // namespace QTIMER3_TIMER3
namespace SAI1_MCLK2 {
constexpr SELECT_INPUT_Reg<102, 2, 0> DAISY;  // Selecting Pads Involved in Daisy Chain.
}  // namespace SAI1_MCLK2
namespace SAI1_RX_BCLK {
constexpr SELECT_INPUT_Reg<103, 2, 0> DAISY;  // Selecting Pads Involved in Daisy Chain.
}  // namespace SAI1_RX_BCLK
namespace SAI1_RX_DATA0 {
constexpr SELECT_INPUT_Reg<104, 2, 0> DAISY;  // Selecting Pads Involved in Daisy Chain.
}  // namespace SAI1_RX_DATA0
namespace SAI1_RX_DATA1 {
constexpr SELECT_INPUT_Reg<105, 1, 0> DAISY;  // Selecting Pads Involved in Daisy Chain.
}  // namespace SAI1_RX_DATA1
namespace SAI1_RX_DATA2 {
constexpr SELECT_INPUT_Reg<106, 1, 0> DAISY;  // Selecting Pads Involved in Daisy Chain.
}  // namespace SAI1_RX_DATA2
namespace SAI1_RX_DATA3 {
constexpr SELECT_INPUT_Reg<107, 1, 0> DAISY;  // Selecting Pads Involved in Daisy Chain.
}  // namespace SAI1_RX_DATA3
namespace SAI1_RX_SYNC {
constexpr SELECT_INPUT_Reg<108, 2, 0> DAISY;  // Selecting Pads Involved in Daisy Chain.
}  // namespace SAI1_RX_SYNC
namespace SAI1_TX_BCLK {
constexpr SELECT_INPUT_Reg<109, 2, 0> DAISY;  // Selecting Pads Involved in Daisy Chain.
}  // namespace SAI1_TX_BCLK
namespace SAI1_TX_SYNC {
constexpr SELECT_INPUT_Reg<110, 2, 0> DAISY;  // Selecting Pads Involved in Daisy Chain.
}  // namespace SAI1_TX_SYNC
namespace SAI2_MCLK2 {
constexpr SELECT_INPUT_Reg<111, 1, 0> DAISY;  // Selecting Pads Involved in Daisy Chain.
}  // namespace SAI2_MCLK2
namespace SAI2_RX_BCLK {
constexpr SELECT_INPUT_Reg<112, 1, 0> DAISY;  // Selecting Pads Involved in Daisy Chain.
}  // namespace SAI2_RX_BCLK
namespace SAI2_RX_DATA0 {
constexpr SELECT_INPUT_Reg<113, 1, 0> DAISY;  // Selecting Pads Involved in Daisy Chain.
}  // namespace SAI2_RX_DATA0
namespace SAI2_RX_SYNC {
constexpr SELECT_INPUT_Reg<114, 1, 0> DAISY;  // Selecting Pads Involved in Daisy Chain.
}  // namespace SAI2_RX_SYNC
namespace SAI2_TX_BCLK {
constexpr SELECT_INPUT_Reg<115, 1, 0> DAISY;  // Selecting Pads Involved in Daisy Chain.
}  // namespace SAI2_TX_BCLK
namespace SAI2_TX_SYNC {
constexpr SELECT_INPUT_Reg<116, 1, 0> DAISY;  // Selecting Pads Involved in Daisy Chain.
}  // namespace SAI2_TX_SYNC
namespace SPDIF_IN {
constexpr SELECT_INPUT_Reg<117, 1, 0> DAISY;  // Selecting Pads Involved in Daisy Chain.
}  // namespace SPDIF_IN
namespace USB_OTG2_OC {
constexpr SELECT_INPUT_Reg<118, 1, 0> DAISY;  // Selecting Pads Involved in Daisy Chain.
}  // namespace USB_OTG2_OC
namespace USB_OTG1_OC {
constexpr SELECT_INPUT_Reg<119, 1, 0> DAISY;  // Selecting Pads Involved in Daisy Chain.
}  // namespace USB_OTG1_OC
namespace USDHC1_CD_B {
constexpr SELECT_INPUT_Reg<120, 2, 0> DAISY;  // Selecting Pads Involved in Daisy Chain.
}  // namespace USDHC1_CD_B
namespace USDHC1_WP {
constexpr SELECT_INPUT_Reg<121, 2, 0> DAISY;  // Selecting Pads Involved in Daisy Chain.
}  // namespace USDHC1_WP
namespace USDHC2_CLK {
constexpr SELECT_INPUT_Reg<122, 1, 0> DAISY;  // Selecting Pads Involved in Daisy Chain.
}  // namespace USDHC2_CLK
namespace USDHC2_CD_B {
constexpr SELECT_INPUT_Reg<123, 1, 0> DAISY;  // Selecting Pads Involved in Daisy Chain.
}  // namespace USDHC2_CD_B
namespace USDHC2_CMD {
constexpr SELECT_INPUT_Reg<124, 1, 0> DAISY;  // Selecting Pads Involved in Daisy Chain.
}  // namespace USDHC2_CMD
namespace USDHC2_DATA0 {
constexpr SELECT_INPUT_Reg<125, 1, 0> DAISY;  // Selecting Pads Involved in Daisy Chain.
}  // namespace USDHC2_DATA0
namespace USDHC2_DATA1 {
constexpr SELECT_INPUT_Reg<126, 1, 0> DAISY;  // Selecting Pads Involved in Daisy Chain.
}  // namespace USDHC2_DATA1
namespace USDHC2_DATA2 {
constexpr SELECT_INPUT_Reg<127, 1, 0> DAISY;  // Selecting Pads Involved in Daisy Chain.
}  // namespace USDHC2_DATA2
namespace USDHC2_DATA3 {
constexpr SELECT_INPUT_Reg<128, 1, 0> DAISY;  // Selecting Pads Involved in Daisy Chain.
}  // namespace USDHC2_DATA3
namespace USDHC2_DATA4 {
constexpr SELECT_INPUT_Reg<129, 1, 0> DAISY;  // Selecting Pads Involved in Daisy Chain.
}  // namespace USDHC2_DATA4
namespace USDHC2_DATA5 {
constexpr SELECT_INPUT_Reg<130, 1, 0> DAISY;  // Selecting Pads Involved in Daisy Chain.
}  // namespace USDHC2_DATA5
namespace USDHC2_DATA6 {
constexpr SELECT_INPUT_Reg<131, 1, 0> DAISY;  // Selecting Pads Involved in Daisy Chain.
}  // namespace USDHC2_DATA6
namespace USDHC2_DATA7 {
constexpr SELECT_INPUT_Reg<132, 1, 0> DAISY;  // Selecting Pads Involved in Daisy Chain.
}  // namespace USDHC2_DATA7
namespace USDHC2_WP {
constexpr SELECT_INPUT_Reg<133, 1, 0> DAISY;  // Selecting Pads Involved in Daisy Chain.
}  // namespace USDHC2_WP
namespace XBAR1_IN02 {
constexpr SELECT_INPUT_Reg<134, 1, 0> DAISY;  // Selecting Pads Involved in Daisy Chain.
}  // namespace XBAR1_IN02
namespace XBAR1_IN03 {
constexpr SELECT_INPUT_Reg<135, 1, 0> DAISY;  // Selecting Pads Involved in Daisy Chain.
}  // namespace XBAR1_IN03
namespace XBAR1_IN04 {
constexpr SELECT_INPUT_Reg<136, 1, 0> DAISY;  // Selecting Pads Involved in Daisy Chain.
}  // namespace XBAR1_IN04
namespace XBAR1_IN05 {
constexpr SELECT_INPUT_Reg<137, 1, 0> DAISY;  // Selecting Pads Involved in Daisy Chain.
}  // namespace XBAR1_IN05
namespace XBAR1_IN06 {
constexpr SELECT_INPUT_Reg<138, 1, 0> DAISY;  // Selecting Pads Involved in Daisy Chain.
}  // namespace XBAR1_IN06
namespace XBAR1_IN07 {
constexpr SELECT_INPUT_Reg<139, 1, 0> DAISY;  // Selecting Pads Involved in Daisy Chain.
}  // namespace XBAR1_IN07
namespace XBAR1_IN08 {
constexpr SELECT_INPUT_Reg<140, 1, 0> DAISY;  // Selecting Pads Involved in Daisy Chain.
}  // namespace XBAR1_IN08
namespace XBAR1_IN09 {
constexpr SELECT_INPUT_Reg<141, 1, 0> DAISY;  // Selecting Pads Involved in Daisy Chain.
}  // namespace XBAR1_IN09
namespace XBAR1_IN17 {
constexpr SELECT_INPUT_Reg<142, 2, 0> DAISY;  // Selecting Pads Involved in Daisy Chain.
}  // namespace XBAR1_IN17
namespace XBAR1_IN18 {
constexpr SELECT_INPUT_Reg<143, 1, 0> DAISY;  // Selecting Pads Involved in Daisy Chain.
}  // namespace XBAR1_IN18
namespace XBAR1_IN20 {
constexpr SELECT_INPUT_Reg<144, 1, 0> DAISY;  // Selecting Pads Involved in Daisy Chain.
}  // namespace XBAR1_IN20
namespace XBAR1_IN22 {
constexpr SELECT_INPUT_Reg<145, 1, 0> DAISY;  // Selecting Pads Involved in Daisy Chain.
}  // namespace XBAR1_IN22
namespace XBAR1_IN23 {
constexpr SELECT_INPUT_Reg<146, 1, 0> DAISY;  // Selecting Pads Involved in Daisy Chain.
}  // namespace XBAR1_IN23
namespace XBAR1_IN24 {
constexpr SELECT_INPUT_Reg<147, 1, 0> DAISY;  // Selecting Pads Involved in Daisy Chain.
}  // namespace XBAR1_IN24
namespace XBAR1_IN14 {
constexpr SELECT_INPUT_Reg<148, 1, 0> DAISY;  // Selecting Pads Involved in Daisy Chain.
}  // namespace XBAR1_IN14
namespace XBAR1_IN15 {
constexpr SELECT_INPUT_Reg<149, 1, 0> DAISY;  // Selecting Pads Involved in Daisy Chain.
}  // namespace XBAR1_IN15
namespace XBAR1_IN16 {
constexpr SELECT_INPUT_Reg<150, 1, 0> DAISY;  // Selecting Pads Involved in Daisy Chain.
}  // namespace XBAR1_IN16
namespace XBAR1_IN25 {
constexpr SELECT_INPUT_Reg<151, 1, 0> DAISY;  // Selecting Pads Involved in Daisy Chain.
}  // namespace XBAR1_IN25
namespace XBAR1_IN19 {
constexpr SELECT_INPUT_Reg<152, 1, 0> DAISY;  // Selecting Pads Involved in Daisy Chain.
}  // namespace XBAR1_IN19
namespace XBAR1_IN21 {
constexpr SELECT_INPUT_Reg<153, 1, 0> DAISY;  // Selecting Pads Involved in Daisy Chain.
}  // namespace XBAR1_IN21

// IOMUXC SELECT_INPUT_1

namespace ENET2_IPG_CLK_RMII {
constexpr SELECT_INPUT_1_Reg<0, 2, 0> DAISY;  // Selecting Pads Involved in Daisy Chain.
}  // namespace ENET2_IPG_CLK_RMII
namespace ENET2_IPP_IND_MAC0_MDIO {
constexpr SELECT_INPUT_1_Reg<1, 1, 0> DAISY;  // Selecting Pads Involved in Daisy Chain.
}  // namespace ENET2_IPP_IND_MAC0_MDIO
namespace ENET2_IPP_IND_MAC0_RXDATA_0 {
constexpr SELECT_INPUT_1_Reg<2, 2, 0> DAISY;  // Selecting Pads Involved in Daisy Chain.
}  // namespace ENET2_IPP_IND_MAC0_RXDATA_0
namespace ENET2_IPP_IND_MAC0_RXDATA_1 {
constexpr SELECT_INPUT_1_Reg<3, 2, 0> DAISY;  // Selecting Pads Involved in Daisy Chain.
}  // namespace ENET2_IPP_IND_MAC0_RXDATA_1
namespace ENET2_IPP_IND_MAC0_RXEN {
constexpr SELECT_INPUT_1_Reg<4, 2, 0> DAISY;  // Selecting Pads Involved in Daisy Chain.
}  // namespace ENET2_IPP_IND_MAC0_RXEN
namespace ENET2_IPP_IND_MAC0_RXERR {
constexpr SELECT_INPUT_1_Reg<5, 2, 0> DAISY;  // Selecting Pads Involved in Daisy Chain.
}  // namespace ENET2_IPP_IND_MAC0_RXERR
namespace ENET2_IPP_IND_MAC0_TIMER_0 {
constexpr SELECT_INPUT_1_Reg<6, 1, 0> DAISY;  // Selecting Pads Involved in Daisy Chain.
}  // namespace ENET2_IPP_IND_MAC0_TIMER_0
namespace ENET2_IPP_IND_MAC0_TXCLK {
constexpr SELECT_INPUT_1_Reg<7, 2, 0> DAISY;  // Selecting Pads Involved in Daisy Chain.
}  // namespace ENET2_IPP_IND_MAC0_TXCLK
namespace FLEXSPI2_IPP_IND_DQS_FA {
constexpr SELECT_INPUT_1_Reg<8, 2, 0> DAISY;  // Selecting Pads Involved in Daisy Chain.
}  // namespace FLEXSPI2_IPP_IND_DQS_FA
namespace FLEXSPI2_IPP_IND_IO_FA_BIT0 {
constexpr SELECT_INPUT_1_Reg<9, 2, 0> DAISY;  // Selecting Pads Involved in Daisy Chain.
}  // namespace FLEXSPI2_IPP_IND_IO_FA_BIT0
namespace FLEXSPI2_IPP_IND_IO_FA_BIT1 {
constexpr SELECT_INPUT_1_Reg<10, 2, 0> DAISY;  // Selecting Pads Involved in Daisy Chain.
}  // namespace FLEXSPI2_IPP_IND_IO_FA_BIT1
namespace FLEXSPI2_IPP_IND_IO_FA_BIT2 {
constexpr SELECT_INPUT_1_Reg<11, 2, 0> DAISY;  // Selecting Pads Involved in Daisy Chain.
}  // namespace FLEXSPI2_IPP_IND_IO_FA_BIT2
namespace FLEXSPI2_IPP_IND_IO_FA_BIT3 {
constexpr SELECT_INPUT_1_Reg<12, 2, 0> DAISY;  // Selecting Pads Involved in Daisy Chain.
}  // namespace FLEXSPI2_IPP_IND_IO_FA_BIT3
namespace FLEXSPI2_IPP_IND_IO_FB_BIT0 {
constexpr SELECT_INPUT_1_Reg<13, 1, 0> DAISY;  // Selecting Pads Involved in Daisy Chain.
}  // namespace FLEXSPI2_IPP_IND_IO_FB_BIT0
namespace FLEXSPI2_IPP_IND_IO_FB_BIT1 {
constexpr SELECT_INPUT_1_Reg<14, 1, 0> DAISY;  // Selecting Pads Involved in Daisy Chain.
}  // namespace FLEXSPI2_IPP_IND_IO_FB_BIT1
namespace FLEXSPI2_IPP_IND_IO_FB_BIT2 {
constexpr SELECT_INPUT_1_Reg<15, 1, 0> DAISY;  // Selecting Pads Involved in Daisy Chain.
}  // namespace FLEXSPI2_IPP_IND_IO_FB_BIT2
namespace FLEXSPI2_IPP_IND_IO_FB_BIT3 {
constexpr SELECT_INPUT_1_Reg<16, 1, 0> DAISY;  // Selecting Pads Involved in Daisy Chain.
}  // namespace FLEXSPI2_IPP_IND_IO_FB_BIT3
namespace FLEXSPI2_IPP_IND_SCK_FA {
constexpr SELECT_INPUT_1_Reg<17, 2, 0> DAISY;  // Selecting Pads Involved in Daisy Chain.
}  // namespace FLEXSPI2_IPP_IND_SCK_FA
namespace FLEXSPI2_IPP_IND_SCK_FB {
constexpr SELECT_INPUT_1_Reg<18, 1, 0> DAISY;  // Selecting Pads Involved in Daisy Chain.
}  // namespace FLEXSPI2_IPP_IND_SCK_FB
namespace GPT1_IPP_IND_CAPIN1 {
constexpr SELECT_INPUT_1_Reg<19, 1, 0> DAISY;  // Selecting Pads Involved in Daisy Chain.
}  // namespace GPT1_IPP_IND_CAPIN1
namespace GPT1_IPP_IND_CAPIN2 {
constexpr SELECT_INPUT_1_Reg<20, 1, 0> DAISY;  // Selecting Pads Involved in Daisy Chain.
}  // namespace GPT1_IPP_IND_CAPIN2
namespace GPT1_IPP_IND_CLKIN {
constexpr SELECT_INPUT_1_Reg<21, 1, 0> DAISY;  // Selecting Pads Involved in Daisy Chain.
}  // namespace GPT1_IPP_IND_CLKIN
namespace GPT2_IPP_IND_CAPIN1 {
constexpr SELECT_INPUT_1_Reg<22, 1, 0> DAISY;  // Selecting Pads Involved in Daisy Chain.
}  // namespace GPT2_IPP_IND_CAPIN1
namespace GPT2_IPP_IND_CAPIN2 {
constexpr SELECT_INPUT_1_Reg<23, 1, 0> DAISY;  // Selecting Pads Involved in Daisy Chain.
}  // namespace GPT2_IPP_IND_CAPIN2
namespace GPT2_IPP_IND_CLKIN {
constexpr SELECT_INPUT_1_Reg<24, 1, 0> DAISY;  // Selecting Pads Involved in Daisy Chain.
}  // namespace GPT2_IPP_IND_CLKIN
namespace SAI3_IPG_CLK_SAI_MCLK_2 {
constexpr SELECT_INPUT_1_Reg<25, 1, 0> DAISY;  // Selecting Pads Involved in Daisy Chain.
}  // namespace SAI3_IPG_CLK_SAI_MCLK_2
namespace SAI3_IPP_IND_SAI_RXBCLK {
constexpr SELECT_INPUT_1_Reg<26, 1, 0> DAISY;  // Selecting Pads Involved in Daisy Chain.
}  // namespace SAI3_IPP_IND_SAI_RXBCLK
namespace SAI3_IPP_IND_SAI_RXDATA_0 {
constexpr SELECT_INPUT_1_Reg<27, 1, 0> DAISY;  // Selecting Pads Involved in Daisy Chain.
}  // namespace SAI3_IPP_IND_SAI_RXDATA_0
namespace SAI3_IPP_IND_SAI_RXSYNC {
constexpr SELECT_INPUT_1_Reg<28, 1, 0> DAISY;  // Selecting Pads Involved in Daisy Chain.
}  // namespace SAI3_IPP_IND_SAI_RXSYNC
namespace SAI3_IPP_IND_SAI_TXBCLK {
constexpr SELECT_INPUT_1_Reg<29, 1, 0> DAISY;  // Selecting Pads Involved in Daisy Chain.
}  // namespace SAI3_IPP_IND_SAI_TXBCLK
namespace SAI3_IPP_IND_SAI_TXSYNC {
constexpr SELECT_INPUT_1_Reg<30, 1, 0> DAISY;  // Selecting Pads Involved in Daisy Chain.
}  // namespace SAI3_IPP_IND_SAI_TXSYNC
namespace SEMC_I_IPP_IND_DQS4 {
constexpr SELECT_INPUT_1_Reg<31, 2, 0> DAISY;  // Selecting Pads Involved in Daisy Chain.
}  // namespace SEMC_I_IPP_IND_DQS4
namespace CANFD_IPP_IND_CANRX {
constexpr SELECT_INPUT_1_Reg<32, 2, 0> DAISY;  // Selecting Pads Involved in Daisy Chain.
}  // namespace CANFD_IPP_IND_CANRX

}  // namespace SELECT_INPUT

// SW_MUX_CTL_PAD and SW_PAD_CTL_PAD indexes
namespace SW_CTL_PAD {
constexpr size_t kEMC_00   =   0;
constexpr size_t kEMC_01   =   1;
constexpr size_t kEMC_02   =   2;
constexpr size_t kEMC_03   =   3;
constexpr size_t kEMC_04   =   4;
constexpr size_t kEMC_05   =   5;
constexpr size_t kEMC_06   =   6;
constexpr size_t kEMC_07   =   7;
constexpr size_t kEMC_08   =   8;
constexpr size_t kEMC_09   =   9;
constexpr size_t kEMC_10   =  10;
constexpr size_t kEMC_11   =  11;
constexpr size_t kEMC_12   =  12;
constexpr size_t kEMC_13   =  13;
constexpr size_t kEMC_14   =  14;
constexpr size_t kEMC_15   =  15;
constexpr size_t kEMC_16   =  16;
constexpr size_t kEMC_17   =  17;
constexpr size_t kEMC_18   =  18;
constexpr size_t kEMC_19   =  19;
constexpr size_t kEMC_20   =  20;
constexpr size_t kEMC_21   =  21;
constexpr size_t kEMC_22   =  22;
constexpr size_t kEMC_23   =  23;
constexpr size_t kEMC_24   =  24;
constexpr size_t kEMC_25   =  25;
constexpr size_t kEMC_26   =  26;
constexpr size_t kEMC_27   =  27;
constexpr size_t kEMC_28   =  28;
constexpr size_t kEMC_29   =  29;
constexpr size_t kEMC_30   =  30;
constexpr size_t kEMC_31   =  31;
constexpr size_t kEMC_32   =  32;
constexpr size_t kEMC_33   =  33;
constexpr size_t kEMC_34   =  34;
constexpr size_t kEMC_35   =  35;
constexpr size_t kEMC_36   =  36;
constexpr size_t kEMC_37   =  37;
constexpr size_t kEMC_38   =  38;
constexpr size_t kEMC_39   =  39;
constexpr size_t kEMC_40   =  40;
constexpr size_t kEMC_41   =  41;
constexpr size_t kAD_B0_00 =  42;
constexpr size_t kAD_B0_01 =  43;
constexpr size_t kAD_B0_02 =  44;
constexpr size_t kAD_B0_03 =  45;
constexpr size_t kAD_B0_04 =  46;
constexpr size_t kAD_B0_05 =  47;
constexpr size_t kAD_B0_06 =  48;
constexpr size_t kAD_B0_07 =  49;
constexpr size_t kAD_B0_08 =  50;
constexpr size_t kAD_B0_09 =  51;
constexpr size_t kAD_B0_10 =  52;
constexpr size_t kAD_B0_11 =  53;
constexpr size_t kAD_B0_12 =  54;
constexpr size_t kAD_B0_13 =  55;
constexpr size_t kAD_B0_14 =  56;
constexpr size_t kAD_B0_15 =  57;
constexpr size_t kAD_B1_00 =  58;
constexpr size_t kAD_B1_01 =  59;
constexpr size_t kAD_B1_02 =  60;
constexpr size_t kAD_B1_03 =  61;
constexpr size_t kAD_B1_04 =  62;
constexpr size_t kAD_B1_05 =  63;
constexpr size_t kAD_B1_06 =  64;
constexpr size_t kAD_B1_07 =  65;
constexpr size_t kAD_B1_08 =  66;
constexpr size_t kAD_B1_09 =  67;
constexpr size_t kAD_B1_10 =  68;
constexpr size_t kAD_B1_11 =  69;
constexpr size_t kAD_B1_12 =  70;
constexpr size_t kAD_B1_13 =  71;
constexpr size_t kAD_B1_14 =  72;
constexpr size_t kAD_B1_15 =  73;
constexpr size_t kB0_00    =  74;
constexpr size_t kB0_01    =  75;
constexpr size_t kB0_02    =  76;
constexpr size_t kB0_03    =  77;
constexpr size_t kB0_04    =  78;
constexpr size_t kB0_05    =  79;
constexpr size_t kB0_06    =  80;
constexpr size_t kB0_07    =  81;
constexpr size_t kB0_08    =  82;
constexpr size_t kB0_09    =  83;
constexpr size_t kB0_10    =  84;
constexpr size_t kB0_11    =  85;
constexpr size_t kB0_12    =  86;
constexpr size_t kB0_13    =  87;
constexpr size_t kB0_14    =  88;
constexpr size_t kB0_15    =  89;
constexpr size_t kB1_00    =  90;
constexpr size_t kB1_01    =  91;
constexpr size_t kB1_02    =  92;
constexpr size_t kB1_03    =  93;
constexpr size_t kB1_04    =  94;
constexpr size_t kB1_05    =  95;
constexpr size_t kB1_06    =  96;
constexpr size_t kB1_07    =  97;
constexpr size_t kB1_08    =  98;
constexpr size_t kB1_09    =  99;
constexpr size_t kB1_10    = 100;
constexpr size_t kB1_11    = 101;
constexpr size_t kB1_12    = 102;
constexpr size_t kB1_13    = 103;
constexpr size_t kB1_14    = 104;
constexpr size_t kB1_15    = 105;
constexpr size_t kSD_B0_00 = 106;
constexpr size_t kSD_B0_01 = 107;
constexpr size_t kSD_B0_02 = 108;
constexpr size_t kSD_B0_03 = 109;
constexpr size_t kSD_B0_04 = 110;
constexpr size_t kSD_B0_05 = 111;
constexpr size_t kSD_B1_00 = 112;
constexpr size_t kSD_B1_01 = 113;
constexpr size_t kSD_B1_02 = 114;
constexpr size_t kSD_B1_03 = 115;
constexpr size_t kSD_B1_04 = 116;
constexpr size_t kSD_B1_05 = 117;
constexpr size_t kSD_B1_06 = 118;
constexpr size_t kSD_B1_07 = 119;
constexpr size_t kSD_B1_08 = 120;
constexpr size_t kSD_B1_09 = 121;
constexpr size_t kSD_B1_10 = 122;
constexpr size_t kSD_B1_11 = 123;
}  // namespace SW_CTL_PAD

// SELECT_INPUT indexes
namespace SELECT_INPUT {
constexpr size_t kANATOP_USB_OTG1_ID =   0;
constexpr size_t kANATOP_USB_OTG2_ID =   1;
constexpr size_t kCCM_PMIC_READY     =   2;
constexpr size_t kCSI_DATA02         =   3;
constexpr size_t kCSI_DATA03         =   4;
constexpr size_t kCSI_DATA04         =   5;
constexpr size_t kCSI_DATA05         =   6;
constexpr size_t kCSI_DATA06         =   7;
constexpr size_t kCSI_DATA07         =   8;
constexpr size_t kCSI_DATA08         =   9;
constexpr size_t kCSI_DATA09         =  10;
constexpr size_t kCSI_HSYNC          =  11;
constexpr size_t kCSI_PIXCLK         =  12;
constexpr size_t kCSI_VSYNC          =  13;
constexpr size_t kENET_IPG_CLK_RMII  =  14;
constexpr size_t kENET_MDIO          =  15;
constexpr size_t kENET0_RXDATA       =  16;
constexpr size_t kENET1_RXDATA       =  17;
constexpr size_t kENET_RXEN          =  18;
constexpr size_t kENET_RXERR         =  19;
constexpr size_t kENET0_TIMER        =  20;
constexpr size_t kENET_TXCLK         =  21;
constexpr size_t kFLEXCAN1_RX        =  22;
constexpr size_t kFLEXCAN2_RX        =  23;
constexpr size_t kFLEXPWM1_PWMA3     =  24;
constexpr size_t kFLEXPWM1_PWMA0     =  25;
constexpr size_t kFLEXPWM1_PWMA1     =  26;
constexpr size_t kFLEXPWM1_PWMA2     =  27;
constexpr size_t kFLEXPWM1_PWMB3     =  28;
constexpr size_t kFLEXPWM1_PWMB0     =  29;
constexpr size_t kFLEXPWM1_PWMB1     =  30;
constexpr size_t kFLEXPWM1_PWMB2     =  31;
constexpr size_t kFLEXPWM2_PWMA3     =  32;
constexpr size_t kFLEXPWM2_PWMA0     =  33;
constexpr size_t kFLEXPWM2_PWMA1     =  34;
constexpr size_t kFLEXPWM2_PWMA2     =  35;
constexpr size_t kFLEXPWM2_PWMB3     =  36;
constexpr size_t kFLEXPWM2_PWMB0     =  37;
constexpr size_t kFLEXPWM2_PWMB1     =  38;
constexpr size_t kFLEXPWM2_PWMB2     =  39;
constexpr size_t kFLEXPWM4_PWMA0     =  40;
constexpr size_t kFLEXPWM4_PWMA1     =  41;
constexpr size_t kFLEXPWM4_PWMA2     =  42;
constexpr size_t kFLEXPWM4_PWMA3     =  43;
constexpr size_t kFLEXSPIA_DQS       =  44;
constexpr size_t kFLEXSPIA_DATA0     =  45;
constexpr size_t kFLEXSPIA_DATA1     =  46;
constexpr size_t kFLEXSPIA_DATA2     =  47;
constexpr size_t kFLEXSPIA_DATA3     =  48;
constexpr size_t kFLEXSPIB_DATA0     =  49;
constexpr size_t kFLEXSPIB_DATA1     =  50;
constexpr size_t kFLEXSPIB_DATA2     =  51;
constexpr size_t kFLEXSPIB_DATA3     =  52;
constexpr size_t kFLEXSPIA_SCK       =  53;
constexpr size_t kLPI2C1_SCL         =  54;
constexpr size_t kLPI2C1_SDA         =  55;
constexpr size_t kLPI2C2_SCL         =  56;
constexpr size_t kLPI2C2_SDA         =  57;
constexpr size_t kLPI2C3_SCL         =  58;
constexpr size_t kLPI2C3_SDA         =  59;
constexpr size_t kLPI2C4_SCL         =  60;
constexpr size_t kLPI2C4_SDA         =  61;
constexpr size_t kLPSPI1_PCS0        =  62;
constexpr size_t kLPSPI1_SCK         =  63;
constexpr size_t kLPSPI1_SDI         =  64;
constexpr size_t kLPSPI1_SDO         =  65;
constexpr size_t kLPSPI2_PCS0        =  66;
constexpr size_t kLPSPI2_SCK         =  67;
constexpr size_t kLPSPI2_SDI         =  68;
constexpr size_t kLPSPI2_SDO         =  69;
constexpr size_t kLPSPI3_PCS0        =  70;
constexpr size_t kLPSPI3_SCK         =  71;
constexpr size_t kLPSPI3_SDI         =  72;
constexpr size_t kLPSPI3_SDO         =  73;
constexpr size_t kLPSPI4_PCS0        =  74;
constexpr size_t kLPSPI4_SCK         =  75;
constexpr size_t kLPSPI4_SDI         =  76;
constexpr size_t kLPSPI4_SDO         =  77;
constexpr size_t kLPUART2_RX         =  78;
constexpr size_t kLPUART2_TX         =  79;
constexpr size_t kLPUART3_CTS_B      =  80;
constexpr size_t kLPUART3_RX         =  81;
constexpr size_t kLPUART3_TX         =  82;
constexpr size_t kLPUART4_RX         =  83;
constexpr size_t kLPUART4_TX         =  84;
constexpr size_t kLPUART5_RX         =  85;
constexpr size_t kLPUART5_TX         =  86;
constexpr size_t kLPUART6_RX         =  87;
constexpr size_t kLPUART6_TX         =  88;
constexpr size_t kLPUART7_RX         =  89;
constexpr size_t kLPUART7_TX         =  90;
constexpr size_t kLPUART8_RX         =  91;
constexpr size_t kLPUART8_TX         =  92;
constexpr size_t kNMI                =  93;
constexpr size_t kQTIMER2_TIMER0     =  94;
constexpr size_t kQTIMER2_TIMER1     =  95;
constexpr size_t kQTIMER2_TIMER2     =  96;
constexpr size_t kQTIMER2_TIMER3     =  97;
constexpr size_t kQTIMER3_TIMER0     =  98;
constexpr size_t kQTIMER3_TIMER1     =  99;
constexpr size_t kQTIMER3_TIMER2     = 100;
constexpr size_t kQTIMER3_TIMER3     = 101;
constexpr size_t kSAI1_MCLK2         = 102;
constexpr size_t kSAI1_RX_BCLK       = 103;
constexpr size_t kSAI1_RX_DATA0      = 104;
constexpr size_t kSAI1_RX_DATA1      = 105;
constexpr size_t kSAI1_RX_DATA2      = 106;
constexpr size_t kSAI1_RX_DATA3      = 107;
constexpr size_t kSAI1_RX_SYNC       = 108;
constexpr size_t kSAI1_TX_BCLK       = 109;
constexpr size_t kSAI1_TX_SYNC       = 110;
constexpr size_t kSAI2_MCLK2         = 111;
constexpr size_t kSAI2_RX_BCLK       = 112;
constexpr size_t kSAI2_RX_DATA0      = 113;
constexpr size_t kSAI2_RX_SYNC       = 114;
constexpr size_t kSAI2_TX_BCLK       = 115;
constexpr size_t kSAI2_TX_SYNC       = 116;
constexpr size_t kSPDIF_IN           = 117;
constexpr size_t kUSB_OTG2_OC        = 118;
constexpr size_t kUSB_OTG1_OC        = 119;
constexpr size_t kUSDHC1_CD_B        = 120;
constexpr size_t kUSDHC1_WP          = 121;
constexpr size_t kUSDHC2_CLK         = 122;
constexpr size_t kUSDHC2_CD_B        = 123;
constexpr size_t kUSDHC2_CMD         = 124;
constexpr size_t kUSDHC2_DATA0       = 125;
constexpr size_t kUSDHC2_DATA1       = 126;
constexpr size_t kUSDHC2_DATA2       = 127;
constexpr size_t kUSDHC2_DATA3       = 128;
constexpr size_t kUSDHC2_DATA4       = 129;
constexpr size_t kUSDHC2_DATA5       = 130;
constexpr size_t kUSDHC2_DATA6       = 131;
constexpr size_t kUSDHC2_DATA7       = 132;
constexpr size_t kUSDHC2_WP          = 133;
constexpr size_t kXBAR1_IN02         = 134;
constexpr size_t kXBAR1_IN03         = 135;
constexpr size_t kXBAR1_IN04         = 136;
constexpr size_t kXBAR1_IN05         = 137;
constexpr size_t kXBAR1_IN06         = 138;
constexpr size_t kXBAR1_IN07         = 139;
constexpr size_t kXBAR1_IN08         = 140;
constexpr size_t kXBAR1_IN09         = 141;
constexpr size_t kXBAR1_IN17         = 142;
constexpr size_t kXBAR1_IN18         = 143;
constexpr size_t kXBAR1_IN20         = 144;
constexpr size_t kXBAR1_IN22         = 145;
constexpr size_t kXBAR1_IN23         = 146;
constexpr size_t kXBAR1_IN24         = 147;
constexpr size_t kXBAR1_IN14         = 148;
constexpr size_t kXBAR1_IN15         = 149;
constexpr size_t kXBAR1_IN16         = 150;
constexpr size_t kXBAR1_IN25         = 151;
constexpr size_t kXBAR1_IN19         = 152;
constexpr size_t kXBAR1_IN21         = 153;
}  // namespace SELECT_INPUT

// SELECT_INPUT_1 indexes
namespace SELECT_INPUT_1 {
constexpr size_t kENET2_IPG_CLK_RMII          =  0;
constexpr size_t kENET2_IPP_IND_MAC0_MDIO     =  1;
constexpr size_t kENET2_IPP_IND_MAC0_RXDATA_0 =  2;
constexpr size_t kENET2_IPP_IND_MAC0_RXDATA_1 =  3;
constexpr size_t kENET2_IPP_IND_MAC0_RXEN     =  4;
constexpr size_t kENET2_IPP_IND_MAC0_RXERR    =  5;
constexpr size_t kENET2_IPP_IND_MAC0_TIMER_0  =  6;
constexpr size_t kENET2_IPP_IND_MAC0_TXCLK    =  7;
constexpr size_t kFLEXSPI2_IPP_IND_DQS_FA     =  8;
constexpr size_t kFLEXSPI2_IPP_IND_IO_FA_BIT0 =  9;
constexpr size_t kFLEXSPI2_IPP_IND_IO_FA_BIT1 = 10;
constexpr size_t kFLEXSPI2_IPP_IND_IO_FA_BIT2 = 11;
constexpr size_t kFLEXSPI2_IPP_IND_IO_FA_BIT3 = 12;
constexpr size_t kFLEXSPI2_IPP_IND_IO_FB_BIT0 = 13;
constexpr size_t kFLEXSPI2_IPP_IND_IO_FB_BIT1 = 14;
constexpr size_t kFLEXSPI2_IPP_IND_IO_FB_BIT2 = 15;
constexpr size_t kFLEXSPI2_IPP_IND_IO_FB_BIT3 = 16;
constexpr size_t kFLEXSPI2_IPP_IND_SCK_FA     = 17;
constexpr size_t kFLEXSPI2_IPP_IND_SCK_FB     = 18;
constexpr size_t kGPT1_IPP_IND_CAPIN1         = 19;
constexpr size_t kGPT1_IPP_IND_CAPIN2         = 20;
constexpr size_t kGPT1_IPP_IND_CLKIN          = 21;
constexpr size_t kGPT2_IPP_IND_CAPIN1         = 22;
constexpr size_t kGPT2_IPP_IND_CAPIN2         = 23;
constexpr size_t kGPT2_IPP_IND_CLKIN          = 24;
constexpr size_t kSAI3_IPG_CLK_SAI_MCLK_2     = 25;
constexpr size_t kSAI3_IPP_IND_SAI_RXBCLK     = 26;
constexpr size_t kSAI3_IPP_IND_SAI_RXDATA_0   = 27;
constexpr size_t kSAI3_IPP_IND_SAI_RXSYNC     = 28;
constexpr size_t kSAI3_IPP_IND_SAI_TXBCLK     = 29;
constexpr size_t kSAI3_IPP_IND_SAI_TXSYNC     = 30;
constexpr size_t kSEMC_I_IPP_IND_DQS4         = 31;
constexpr size_t kCANFD_IPP_IND_CANRX         = 32;
}  // namespace SELECT_INPUT_1

}  // namespace IOMUXC

}  // namespace imxrt1060
}  // namespace hardware
}  // namespace qindesign

// // IOMUXC SW_MUX_CTL_PAD indexes.
// enum IOMUXC_SW_MUX_CTL_PAD : size_t {
//   kIOMUXC_SW_MUX_CTL_PAD_GPIO_EMC_00 =  0,
//   kIOMUXC_SW_MUX_CTL_PAD_GPIO_EMC_01 =  1,
//   kIOMUXC_SW_MUX_CTL_PAD_GPIO_EMC_02 =  2,
//   kIOMUXC_SW_MUX_CTL_PAD_GPIO_EMC_03 =  3,
//   kIOMUXC_SW_MUX_CTL_PAD_GPIO_EMC_04 =  4,
//   kIOMUXC_SW_MUX_CTL_PAD_GPIO_EMC_05 =  5,
//   kIOMUXC_SW_MUX_CTL_PAD_GPIO_EMC_06 =  6,
//   kIOMUXC_SW_MUX_CTL_PAD_GPIO_EMC_07 =  7,
//   kIOMUXC_SW_MUX_CTL_PAD_GPIO_EMC_08 =  8,
//   kIOMUXC_SW_MUX_CTL_PAD_GPIO_EMC_09 =  9,
//   kIOMUXC_SW_MUX_CTL_PAD_GPIO_EMC_10 = 10,
//   kIOMUXC_SW_MUX_CTL_PAD_GPIO_EMC_11 = 11,
//   kIOMUXC_SW_MUX_CTL_PAD_GPIO_EMC_12 = 12,
//   kIOMUXC_SW_MUX_CTL_PAD_GPIO_EMC_13 = 13,
//   kIOMUXC_SW_MUX_CTL_PAD_GPIO_EMC_14 = 14,
//   kIOMUXC_SW_MUX_CTL_PAD_GPIO_EMC_15 = 15,
//   kIOMUXC_SW_MUX_CTL_PAD_GPIO_EMC_16 = 16,
//   kIOMUXC_SW_MUX_CTL_PAD_GPIO_EMC_17 = 17,
//   kIOMUXC_SW_MUX_CTL_PAD_GPIO_EMC_18 = 18,
//   kIOMUXC_SW_MUX_CTL_PAD_GPIO_EMC_19 = 19,
//   kIOMUXC_SW_MUX_CTL_PAD_GPIO_EMC_20 = 20,
//   kIOMUXC_SW_MUX_CTL_PAD_GPIO_EMC_21 = 21,
//   kIOMUXC_SW_MUX_CTL_PAD_GPIO_EMC_22 = 22,
//   kIOMUXC_SW_MUX_CTL_PAD_GPIO_EMC_23 = 23,
//   kIOMUXC_SW_MUX_CTL_PAD_GPIO_EMC_24 = 24,
//   kIOMUXC_SW_MUX_CTL_PAD_GPIO_EMC_25 = 25,
//   kIOMUXC_SW_MUX_CTL_PAD_GPIO_EMC_26 = 26,
//   kIOMUXC_SW_MUX_CTL_PAD_GPIO_EMC_27 = 27,
//   kIOMUXC_SW_MUX_CTL_PAD_GPIO_EMC_28 = 28,
//   kIOMUXC_SW_MUX_CTL_PAD_GPIO_EMC_29 = 29,
//   kIOMUXC_SW_MUX_CTL_PAD_GPIO_EMC_30 = 30,
//   kIOMUXC_SW_MUX_CTL_PAD_GPIO_EMC_31 = 31,
//   kIOMUXC_SW_MUX_CTL_PAD_GPIO_EMC_32 = 32,
//   kIOMUXC_SW_MUX_CTL_PAD_GPIO_EMC_33 = 33,
//   kIOMUXC_SW_MUX_CTL_PAD_GPIO_EMC_34 = 34,
//   kIOMUXC_SW_MUX_CTL_PAD_GPIO_EMC_35 = 35,
//   kIOMUXC_SW_MUX_CTL_PAD_GPIO_EMC_36 = 36,
//   kIOMUXC_SW_MUX_CTL_PAD_GPIO_EMC_37 = 37,
//   kIOMUXC_SW_MUX_CTL_PAD_GPIO_EMC_38 = 38,
//   kIOMUXC_SW_MUX_CTL_PAD_GPIO_EMC_39 = 39,
//   kIOMUXC_SW_MUX_CTL_PAD_GPIO_EMC_40 = 40,
//   kIOMUXC_SW_MUX_CTL_PAD_GPIO_EMC_41 = 41,

//   kIOMUXC_SW_MUX_CTL_PAD_GPIO_AD_B0_00 = 42,
//   kIOMUXC_SW_MUX_CTL_PAD_GPIO_AD_B0_01 = 43,
//   kIOMUXC_SW_MUX_CTL_PAD_GPIO_AD_B0_02 = 44,
//   kIOMUXC_SW_MUX_CTL_PAD_GPIO_AD_B0_03 = 45,
//   kIOMUXC_SW_MUX_CTL_PAD_GPIO_AD_B0_04 = 46,
//   kIOMUXC_SW_MUX_CTL_PAD_GPIO_AD_B0_05 = 47,
//   kIOMUXC_SW_MUX_CTL_PAD_GPIO_AD_B0_06 = 48,
//   kIOMUXC_SW_MUX_CTL_PAD_GPIO_AD_B0_07 = 49,
//   kIOMUXC_SW_MUX_CTL_PAD_GPIO_AD_B0_08 = 50,
//   kIOMUXC_SW_MUX_CTL_PAD_GPIO_AD_B0_09 = 51,
//   kIOMUXC_SW_MUX_CTL_PAD_GPIO_AD_B0_10 = 52,
//   kIOMUXC_SW_MUX_CTL_PAD_GPIO_AD_B0_11 = 53,
//   kIOMUXC_SW_MUX_CTL_PAD_GPIO_AD_B0_12 = 54,
//   kIOMUXC_SW_MUX_CTL_PAD_GPIO_AD_B0_13 = 55,
//   kIOMUXC_SW_MUX_CTL_PAD_GPIO_AD_B0_14 = 56,
//   kIOMUXC_SW_MUX_CTL_PAD_GPIO_AD_B0_15 = 57,
//   kIOMUXC_SW_MUX_CTL_PAD_GPIO_AD_B1_00 = 58,
//   kIOMUXC_SW_MUX_CTL_PAD_GPIO_AD_B1_01 = 59,
//   kIOMUXC_SW_MUX_CTL_PAD_GPIO_AD_B1_02 = 60,
//   kIOMUXC_SW_MUX_CTL_PAD_GPIO_AD_B1_03 = 61,
//   kIOMUXC_SW_MUX_CTL_PAD_GPIO_AD_B1_04 = 62,
//   kIOMUXC_SW_MUX_CTL_PAD_GPIO_AD_B1_05 = 63,
//   kIOMUXC_SW_MUX_CTL_PAD_GPIO_AD_B1_06 = 64,
//   kIOMUXC_SW_MUX_CTL_PAD_GPIO_AD_B1_07 = 65,
//   kIOMUXC_SW_MUX_CTL_PAD_GPIO_AD_B1_08 = 66,
//   kIOMUXC_SW_MUX_CTL_PAD_GPIO_AD_B1_09 = 67,
//   kIOMUXC_SW_MUX_CTL_PAD_GPIO_AD_B1_10 = 68,
//   kIOMUXC_SW_MUX_CTL_PAD_GPIO_AD_B1_11 = 69,
//   kIOMUXC_SW_MUX_CTL_PAD_GPIO_AD_B1_12 = 70,
//   kIOMUXC_SW_MUX_CTL_PAD_GPIO_AD_B1_13 = 71,
//   kIOMUXC_SW_MUX_CTL_PAD_GPIO_AD_B1_14 = 72,
//   kIOMUXC_SW_MUX_CTL_PAD_GPIO_AD_B1_15 = 73,

//   kIOMUXC_SW_MUX_CTL_PAD_GPIO_B0_00 =  74,
//   kIOMUXC_SW_MUX_CTL_PAD_GPIO_B0_01 =  75,
//   kIOMUXC_SW_MUX_CTL_PAD_GPIO_B0_02 =  76,
//   kIOMUXC_SW_MUX_CTL_PAD_GPIO_B0_03 =  77,
//   kIOMUXC_SW_MUX_CTL_PAD_GPIO_B0_04 =  78,
//   kIOMUXC_SW_MUX_CTL_PAD_GPIO_B0_05 =  79,
//   kIOMUXC_SW_MUX_CTL_PAD_GPIO_B0_06 =  80,
//   kIOMUXC_SW_MUX_CTL_PAD_GPIO_B0_07 =  81,
//   kIOMUXC_SW_MUX_CTL_PAD_GPIO_B0_08 =  82,
//   kIOMUXC_SW_MUX_CTL_PAD_GPIO_B0_09 =  83,
//   kIOMUXC_SW_MUX_CTL_PAD_GPIO_B0_10 =  84,
//   kIOMUXC_SW_MUX_CTL_PAD_GPIO_B0_11 =  85,
//   kIOMUXC_SW_MUX_CTL_PAD_GPIO_B0_12 =  86,
//   kIOMUXC_SW_MUX_CTL_PAD_GPIO_B0_13 =  87,
//   kIOMUXC_SW_MUX_CTL_PAD_GPIO_B0_14 =  88,
//   kIOMUXC_SW_MUX_CTL_PAD_GPIO_B0_15 =  89,
//   kIOMUXC_SW_MUX_CTL_PAD_GPIO_B1_00 =  90,
//   kIOMUXC_SW_MUX_CTL_PAD_GPIO_B1_01 =  91,
//   kIOMUXC_SW_MUX_CTL_PAD_GPIO_B1_02 =  92,
//   kIOMUXC_SW_MUX_CTL_PAD_GPIO_B1_03 =  93,
//   kIOMUXC_SW_MUX_CTL_PAD_GPIO_B1_04 =  94,
//   kIOMUXC_SW_MUX_CTL_PAD_GPIO_B1_05 =  95,
//   kIOMUXC_SW_MUX_CTL_PAD_GPIO_B1_06 =  96,
//   kIOMUXC_SW_MUX_CTL_PAD_GPIO_B1_07 =  97,
//   kIOMUXC_SW_MUX_CTL_PAD_GPIO_B1_08 =  98,
//   kIOMUXC_SW_MUX_CTL_PAD_GPIO_B1_09 =  99,
//   kIOMUXC_SW_MUX_CTL_PAD_GPIO_B1_10 = 100,
//   kIOMUXC_SW_MUX_CTL_PAD_GPIO_B1_11 = 101,
//   kIOMUXC_SW_MUX_CTL_PAD_GPIO_B1_12 = 102,
//   kIOMUXC_SW_MUX_CTL_PAD_GPIO_B1_13 = 103,
//   kIOMUXC_SW_MUX_CTL_PAD_GPIO_B1_14 = 104,
//   kIOMUXC_SW_MUX_CTL_PAD_GPIO_B1_15 = 105,

//   kIOMUXC_SW_MUX_CTL_PAD_GPIO_SD_B0_00 = 106,
//   kIOMUXC_SW_MUX_CTL_PAD_GPIO_SD_B0_01 = 107,
//   kIOMUXC_SW_MUX_CTL_PAD_GPIO_SD_B0_02 = 108,
//   kIOMUXC_SW_MUX_CTL_PAD_GPIO_SD_B0_03 = 109,
//   kIOMUXC_SW_MUX_CTL_PAD_GPIO_SD_B0_04 = 110,
//   kIOMUXC_SW_MUX_CTL_PAD_GPIO_SD_B0_05 = 111,
//   kIOMUXC_SW_MUX_CTL_PAD_GPIO_SD_B1_00 = 112,
//   kIOMUXC_SW_MUX_CTL_PAD_GPIO_SD_B1_01 = 113,
//   kIOMUXC_SW_MUX_CTL_PAD_GPIO_SD_B1_02 = 114,
//   kIOMUXC_SW_MUX_CTL_PAD_GPIO_SD_B1_03 = 115,
//   kIOMUXC_SW_MUX_CTL_PAD_GPIO_SD_B1_04 = 116,
//   kIOMUXC_SW_MUX_CTL_PAD_GPIO_SD_B1_05 = 117,
//   kIOMUXC_SW_MUX_CTL_PAD_GPIO_SD_B1_06 = 118,
//   kIOMUXC_SW_MUX_CTL_PAD_GPIO_SD_B1_07 = 119,
//   kIOMUXC_SW_MUX_CTL_PAD_GPIO_SD_B1_08 = 120,
//   kIOMUXC_SW_MUX_CTL_PAD_GPIO_SD_B1_09 = 121,
//   kIOMUXC_SW_MUX_CTL_PAD_GPIO_SD_B1_10 = 122,
//   kIOMUXC_SW_MUX_CTL_PAD_GPIO_SD_B1_11 = 123,
// };

// // IOMUXC SW_PAD_CTL_PAD indexes.
// enum IOMUXC_SW_PAD_CTL_PAD : size_t {
//     kIOMUXC_SW_PAD_CTL_PAD_GPIO_EMC_00 =  0,
//     kIOMUXC_SW_PAD_CTL_PAD_GPIO_EMC_01 =  1,
//     kIOMUXC_SW_PAD_CTL_PAD_GPIO_EMC_02 =  2,
//     kIOMUXC_SW_PAD_CTL_PAD_GPIO_EMC_03 =  3,
//     kIOMUXC_SW_PAD_CTL_PAD_GPIO_EMC_04 =  4,
//     kIOMUXC_SW_PAD_CTL_PAD_GPIO_EMC_05 =  5,
//     kIOMUXC_SW_PAD_CTL_PAD_GPIO_EMC_06 =  6,
//     kIOMUXC_SW_PAD_CTL_PAD_GPIO_EMC_07 =  7,
//     kIOMUXC_SW_PAD_CTL_PAD_GPIO_EMC_08 =  8,
//     kIOMUXC_SW_PAD_CTL_PAD_GPIO_EMC_09 =  9,
//     kIOMUXC_SW_PAD_CTL_PAD_GPIO_EMC_10 = 10,
//     kIOMUXC_SW_PAD_CTL_PAD_GPIO_EMC_11 = 11,
//     kIOMUXC_SW_PAD_CTL_PAD_GPIO_EMC_12 = 12,
//     kIOMUXC_SW_PAD_CTL_PAD_GPIO_EMC_13 = 13,
//     kIOMUXC_SW_PAD_CTL_PAD_GPIO_EMC_14 = 14,
//     kIOMUXC_SW_PAD_CTL_PAD_GPIO_EMC_15 = 15,
//     kIOMUXC_SW_PAD_CTL_PAD_GPIO_EMC_16 = 16,
//     kIOMUXC_SW_PAD_CTL_PAD_GPIO_EMC_17 = 17,
//     kIOMUXC_SW_PAD_CTL_PAD_GPIO_EMC_18 = 18,
//     kIOMUXC_SW_PAD_CTL_PAD_GPIO_EMC_19 = 19,
//     kIOMUXC_SW_PAD_CTL_PAD_GPIO_EMC_20 = 20,
//     kIOMUXC_SW_PAD_CTL_PAD_GPIO_EMC_21 = 21,
//     kIOMUXC_SW_PAD_CTL_PAD_GPIO_EMC_22 = 22,
//     kIOMUXC_SW_PAD_CTL_PAD_GPIO_EMC_23 = 23,
//     kIOMUXC_SW_PAD_CTL_PAD_GPIO_EMC_24 = 24,
//     kIOMUXC_SW_PAD_CTL_PAD_GPIO_EMC_25 = 25,
//     kIOMUXC_SW_PAD_CTL_PAD_GPIO_EMC_26 = 26,
//     kIOMUXC_SW_PAD_CTL_PAD_GPIO_EMC_27 = 27,
//     kIOMUXC_SW_PAD_CTL_PAD_GPIO_EMC_28 = 28,
//     kIOMUXC_SW_PAD_CTL_PAD_GPIO_EMC_29 = 29,
//     kIOMUXC_SW_PAD_CTL_PAD_GPIO_EMC_30 = 30,
//     kIOMUXC_SW_PAD_CTL_PAD_GPIO_EMC_31 = 31,
//     kIOMUXC_SW_PAD_CTL_PAD_GPIO_EMC_32 = 32,
//     kIOMUXC_SW_PAD_CTL_PAD_GPIO_EMC_33 = 33,
//     kIOMUXC_SW_PAD_CTL_PAD_GPIO_EMC_34 = 34,
//     kIOMUXC_SW_PAD_CTL_PAD_GPIO_EMC_35 = 35,
//     kIOMUXC_SW_PAD_CTL_PAD_GPIO_EMC_36 = 36,
//     kIOMUXC_SW_PAD_CTL_PAD_GPIO_EMC_37 = 37,
//     kIOMUXC_SW_PAD_CTL_PAD_GPIO_EMC_38 = 38,
//     kIOMUXC_SW_PAD_CTL_PAD_GPIO_EMC_39 = 39,
//     kIOMUXC_SW_PAD_CTL_PAD_GPIO_EMC_40 = 40,
//     kIOMUXC_SW_PAD_CTL_PAD_GPIO_EMC_41 = 41,

//     kIOMUXC_SW_PAD_CTL_PAD_GPIO_AD_B0_00 = 42,
//     kIOMUXC_SW_PAD_CTL_PAD_GPIO_AD_B0_01 = 43,
//     kIOMUXC_SW_PAD_CTL_PAD_GPIO_AD_B0_02 = 44,
//     kIOMUXC_SW_PAD_CTL_PAD_GPIO_AD_B0_03 = 45,
//     kIOMUXC_SW_PAD_CTL_PAD_GPIO_AD_B0_04 = 46,
//     kIOMUXC_SW_PAD_CTL_PAD_GPIO_AD_B0_05 = 47,
//     kIOMUXC_SW_PAD_CTL_PAD_GPIO_AD_B0_06 = 48,
//     kIOMUXC_SW_PAD_CTL_PAD_GPIO_AD_B0_07 = 49,
//     kIOMUXC_SW_PAD_CTL_PAD_GPIO_AD_B0_08 = 50,
//     kIOMUXC_SW_PAD_CTL_PAD_GPIO_AD_B0_09 = 51,
//     kIOMUXC_SW_PAD_CTL_PAD_GPIO_AD_B0_10 = 52,
//     kIOMUXC_SW_PAD_CTL_PAD_GPIO_AD_B0_11 = 53,
//     kIOMUXC_SW_PAD_CTL_PAD_GPIO_AD_B0_12 = 54,
//     kIOMUXC_SW_PAD_CTL_PAD_GPIO_AD_B0_13 = 55,
//     kIOMUXC_SW_PAD_CTL_PAD_GPIO_AD_B0_14 = 56,
//     kIOMUXC_SW_PAD_CTL_PAD_GPIO_AD_B0_15 = 57,
//     kIOMUXC_SW_PAD_CTL_PAD_GPIO_AD_B1_00 = 58,
//     kIOMUXC_SW_PAD_CTL_PAD_GPIO_AD_B1_01 = 59,
//     kIOMUXC_SW_PAD_CTL_PAD_GPIO_AD_B1_02 = 60,
//     kIOMUXC_SW_PAD_CTL_PAD_GPIO_AD_B1_03 = 61,
//     kIOMUXC_SW_PAD_CTL_PAD_GPIO_AD_B1_04 = 62,
//     kIOMUXC_SW_PAD_CTL_PAD_GPIO_AD_B1_05 = 63,
//     kIOMUXC_SW_PAD_CTL_PAD_GPIO_AD_B1_06 = 64,
//     kIOMUXC_SW_PAD_CTL_PAD_GPIO_AD_B1_07 = 65,
//     kIOMUXC_SW_PAD_CTL_PAD_GPIO_AD_B1_08 = 66,
//     kIOMUXC_SW_PAD_CTL_PAD_GPIO_AD_B1_09 = 67,
//     kIOMUXC_SW_PAD_CTL_PAD_GPIO_AD_B1_10 = 68,
//     kIOMUXC_SW_PAD_CTL_PAD_GPIO_AD_B1_11 = 69,
//     kIOMUXC_SW_PAD_CTL_PAD_GPIO_AD_B1_12 = 70,
//     kIOMUXC_SW_PAD_CTL_PAD_GPIO_AD_B1_13 = 71,
//     kIOMUXC_SW_PAD_CTL_PAD_GPIO_AD_B1_14 = 72,
//     kIOMUXC_SW_PAD_CTL_PAD_GPIO_AD_B1_15 = 73,

//     kIOMUXC_SW_PAD_CTL_PAD_GPIO_B0_00 =  74,
//     kIOMUXC_SW_PAD_CTL_PAD_GPIO_B0_01 =  75,
//     kIOMUXC_SW_PAD_CTL_PAD_GPIO_B0_02 =  76,
//     kIOMUXC_SW_PAD_CTL_PAD_GPIO_B0_03 =  77,
//     kIOMUXC_SW_PAD_CTL_PAD_GPIO_B0_04 =  78,
//     kIOMUXC_SW_PAD_CTL_PAD_GPIO_B0_05 =  79,
//     kIOMUXC_SW_PAD_CTL_PAD_GPIO_B0_06 =  80,
//     kIOMUXC_SW_PAD_CTL_PAD_GPIO_B0_07 =  81,
//     kIOMUXC_SW_PAD_CTL_PAD_GPIO_B0_08 =  82,
//     kIOMUXC_SW_PAD_CTL_PAD_GPIO_B0_09 =  83,
//     kIOMUXC_SW_PAD_CTL_PAD_GPIO_B0_10 =  84,
//     kIOMUXC_SW_PAD_CTL_PAD_GPIO_B0_11 =  85,
//     kIOMUXC_SW_PAD_CTL_PAD_GPIO_B0_12 =  86,
//     kIOMUXC_SW_PAD_CTL_PAD_GPIO_B0_13 =  87,
//     kIOMUXC_SW_PAD_CTL_PAD_GPIO_B0_14 =  88,
//     kIOMUXC_SW_PAD_CTL_PAD_GPIO_B0_15 =  89,
//     kIOMUXC_SW_PAD_CTL_PAD_GPIO_B1_00 =  90,
//     kIOMUXC_SW_PAD_CTL_PAD_GPIO_B1_01 =  91,
//     kIOMUXC_SW_PAD_CTL_PAD_GPIO_B1_02 =  92,
//     kIOMUXC_SW_PAD_CTL_PAD_GPIO_B1_03 =  93,
//     kIOMUXC_SW_PAD_CTL_PAD_GPIO_B1_04 =  94,
//     kIOMUXC_SW_PAD_CTL_PAD_GPIO_B1_05 =  95,
//     kIOMUXC_SW_PAD_CTL_PAD_GPIO_B1_06 =  96,
//     kIOMUXC_SW_PAD_CTL_PAD_GPIO_B1_07 =  97,
//     kIOMUXC_SW_PAD_CTL_PAD_GPIO_B1_08 =  98,
//     kIOMUXC_SW_PAD_CTL_PAD_GPIO_B1_09 =  99,
//     kIOMUXC_SW_PAD_CTL_PAD_GPIO_B1_10 = 100,
//     kIOMUXC_SW_PAD_CTL_PAD_GPIO_B1_11 = 101,
//     kIOMUXC_SW_PAD_CTL_PAD_GPIO_B1_12 = 102,
//     kIOMUXC_SW_PAD_CTL_PAD_GPIO_B1_13 = 103,
//     kIOMUXC_SW_PAD_CTL_PAD_GPIO_B1_14 = 104,
//     kIOMUXC_SW_PAD_CTL_PAD_GPIO_B1_15 = 105,

//     kIOMUXC_SW_PAD_CTL_PAD_GPIO_SD_B0_00 = 106,
//     kIOMUXC_SW_PAD_CTL_PAD_GPIO_SD_B0_01 = 107,
//     kIOMUXC_SW_PAD_CTL_PAD_GPIO_SD_B0_02 = 108,
//     kIOMUXC_SW_PAD_CTL_PAD_GPIO_SD_B0_03 = 109,
//     kIOMUXC_SW_PAD_CTL_PAD_GPIO_SD_B0_04 = 110,
//     kIOMUXC_SW_PAD_CTL_PAD_GPIO_SD_B0_05 = 111,
//     kIOMUXC_SW_PAD_CTL_PAD_GPIO_SD_B1_00 = 112,
//     kIOMUXC_SW_PAD_CTL_PAD_GPIO_SD_B1_01 = 113,
//     kIOMUXC_SW_PAD_CTL_PAD_GPIO_SD_B1_02 = 114,
//     kIOMUXC_SW_PAD_CTL_PAD_GPIO_SD_B1_03 = 115,
//     kIOMUXC_SW_PAD_CTL_PAD_GPIO_SD_B1_04 = 116,
//     kIOMUXC_SW_PAD_CTL_PAD_GPIO_SD_B1_05 = 117,
//     kIOMUXC_SW_PAD_CTL_PAD_GPIO_SD_B1_06 = 118,
//     kIOMUXC_SW_PAD_CTL_PAD_GPIO_SD_B1_07 = 119,
//     kIOMUXC_SW_PAD_CTL_PAD_GPIO_SD_B1_08 = 120,
//     kIOMUXC_SW_PAD_CTL_PAD_GPIO_SD_B1_09 = 121,
//     kIOMUXC_SW_PAD_CTL_PAD_GPIO_SD_B1_10 = 122,
//     kIOMUXC_SW_PAD_CTL_PAD_GPIO_SD_B1_11 = 123,
// };

// // IOMUXC SELECT_INPUT indexes.
// enum IOMUXC_SELECT_INPUT {
//   kIOMUXC_ANATOP_USB_OTG1_ID_SELECT_INPUT = 0,
//   kIOMUXC_ANATOP_USB_OTG2_ID_SELECT_INPUT = 1,

//   kIOMUXC_CCM_PMIC_READY_SELECT_INPUT = 2,

//   kIOMUXC_CSI_DATA02_SELECT_INPUT =  3,
//   kIOMUXC_CSI_DATA03_SELECT_INPUT =  4,
//   kIOMUXC_CSI_DATA04_SELECT_INPUT =  5,
//   kIOMUXC_CSI_DATA05_SELECT_INPUT =  6,
//   kIOMUXC_CSI_DATA06_SELECT_INPUT =  7,
//   kIOMUXC_CSI_DATA07_SELECT_INPUT =  8,
//   kIOMUXC_CSI_DATA08_SELECT_INPUT =  9,
//   kIOMUXC_CSI_DATA09_SELECT_INPUT = 10,
//   kIOMUXC_CSI_HSYNC_SELECT_INPUT  = 11,
//   kIOMUXC_CSI_PIXCLK_SELECT_INPUT = 12,
//   kIOMUXC_CSI_VSYNC_SELECT_INPUT  = 13,

//   kIOMUXC_ENET_IPG_CLK_RMII_SELECT_INPUT = 14,
//   kIOMUXC_ENET_MDIO_SELECT_INPUT         = 15,
//   kIOMUXC_ENET0_RXDATA_SELECT_INPUT      = 16,
//   kIOMUXC_ENET1_RXDATA_SELECT_INPUT      = 17,
//   kIOMUXC_ENET_RXEN_SELECT_INPUT         = 18,
//   kIOMUXC_ENET_RXERR_SELECT_INPUT        = 19,
//   kIOMUXC_ENET0_TIMER_SELECT_INPUT       = 20,
//   kIOMUXC_ENET_TXCLK_SELECT_INPUT        = 21,

//   kIOMUXC_FLEXCAN1_RX_SELECT_INPUT = 22,
//   kIOMUXC_FLEXCAN2_RX_SELECT_INPUT = 23,

//   kIOMUXC_FLEXPWM1_PWMA3_SELECT_INPUT = 24,
//   kIOMUXC_FLEXPWM1_PWMA0_SELECT_INPUT = 25,
//   kIOMUXC_FLEXPWM1_PWMA1_SELECT_INPUT = 26,
//   kIOMUXC_FLEXPWM1_PWMA2_SELECT_INPUT = 27,
//   kIOMUXC_FLEXPWM1_PWMB3_SELECT_INPUT = 28,
//   kIOMUXC_FLEXPWM1_PWMB0_SELECT_INPUT = 29,
//   kIOMUXC_FLEXPWM1_PWMB1_SELECT_INPUT = 30,
//   kIOMUXC_FLEXPWM1_PWMB2_SELECT_INPUT = 31,
//   kIOMUXC_FLEXPWM2_PWMA3_SELECT_INPUT = 32,
//   kIOMUXC_FLEXPWM2_PWMA0_SELECT_INPUT = 33,
//   kIOMUXC_FLEXPWM2_PWMA1_SELECT_INPUT = 34,
//   kIOMUXC_FLEXPWM2_PWMA2_SELECT_INPUT = 35,
//   kIOMUXC_FLEXPWM2_PWMB3_SELECT_INPUT = 36,
//   kIOMUXC_FLEXPWM2_PWMB0_SELECT_INPUT = 37,
//   kIOMUXC_FLEXPWM2_PWMB1_SELECT_INPUT = 38,
//   kIOMUXC_FLEXPWM2_PWMB2_SELECT_INPUT = 39,
//   kIOMUXC_FLEXPWM4_PWMA0_SELECT_INPUT = 40,
//   kIOMUXC_FLEXPWM4_PWMA1_SELECT_INPUT = 41,
//   kIOMUXC_FLEXPWM4_PWMA2_SELECT_INPUT = 42,
//   kIOMUXC_FLEXPWM4_PWMA3_SELECT_INPUT = 43,

//   kIOMUXC_FLEXSPIA_DQS_SELECT_INPUT   = 44,
//   kIOMUXC_FLEXSPIA_DATA0_SELECT_INPUT = 45,
//   kIOMUXC_FLEXSPIA_DATA1_SELECT_INPUT = 46,
//   kIOMUXC_FLEXSPIA_DATA2_SELECT_INPUT = 47,
//   kIOMUXC_FLEXSPIA_DATA3_SELECT_INPUT = 48,
//   kIOMUXC_FLEXSPIB_DATA0_SELECT_INPUT = 49,
//   kIOMUXC_FLEXSPIB_DATA1_SELECT_INPUT = 50,
//   kIOMUXC_FLEXSPIB_DATA2_SELECT_INPUT = 51,
//   kIOMUXC_FLEXSPIB_DATA3_SELECT_INPUT = 52,
//   kIOMUXC_FLEXSPIA_SCK_SELECT_INPUT   = 53,

//   kIOMUXC_LPI2C1_SCL_SELECT_INPUT = 54,
//   kIOMUXC_LPI2C1_SDA_SELECT_INPUT = 55,
//   kIOMUXC_LPI2C2_SCL_SELECT_INPUT = 56,
//   kIOMUXC_LPI2C2_SDA_SELECT_INPUT = 57,
//   kIOMUXC_LPI2C3_SCL_SELECT_INPUT = 58,
//   kIOMUXC_LPI2C3_SDA_SELECT_INPUT = 59,
//   kIOMUXC_LPI2C4_SCL_SELECT_INPUT = 60,
//   kIOMUXC_LPI2C4_SDA_SELECT_INPUT = 61,

//   kIOMUXC_LPSPI1_PCS0_SELECT_INPUT = 62,
//   kIOMUXC_LPSPI1_SCK_SELECT_INPUT  = 63,
//   kIOMUXC_LPSPI1_SDI_SELECT_INPUT  = 64,
//   kIOMUXC_LPSPI1_SDO_SELECT_INPUT  = 65,
//   kIOMUXC_LPSPI2_PCS0_SELECT_INPUT = 66,
//   kIOMUXC_LPSPI2_SCK_SELECT_INPUT  = 67,
//   kIOMUXC_LPSPI2_SDI_SELECT_INPUT  = 68,
//   kIOMUXC_LPSPI2_SDO_SELECT_INPUT  = 69,
//   kIOMUXC_LPSPI3_PCS0_SELECT_INPUT = 70,
//   kIOMUXC_LPSPI3_SCK_SELECT_INPUT  = 71,
//   kIOMUXC_LPSPI3_SDI_SELECT_INPUT  = 72,
//   kIOMUXC_LPSPI3_SDO_SELECT_INPUT  = 73,
//   kIOMUXC_LPSPI4_PCS0_SELECT_INPUT = 74,
//   kIOMUXC_LPSPI4_SCK_SELECT_INPUT  = 75,
//   kIOMUXC_LPSPI4_SDI_SELECT_INPUT  = 76,
//   kIOMUXC_LPSPI4_SDO_SELECT_INPUT  = 77,

//   kIOMUXC_LPUART2_RX_SELECT_INPUT    = 78,
//   kIOMUXC_LPUART2_TX_SELECT_INPUT    = 79,
//   kIOMUXC_LPUART3_CTS_B_SELECT_INPUT = 80,
//   kIOMUXC_LPUART3_RX_SELECT_INPUT    = 81,
//   kIOMUXC_LPUART3_TX_SELECT_INPUT    = 82,
//   kIOMUXC_LPUART4_RX_SELECT_INPUT    = 83,
//   kIOMUXC_LPUART4_TX_SELECT_INPUT    = 84,
//   kIOMUXC_LPUART5_RX_SELECT_INPUT    = 85,
//   kIOMUXC_LPUART5_TX_SELECT_INPUT    = 86,
//   kIOMUXC_LPUART6_RX_SELECT_INPUT    = 87,
//   kIOMUXC_LPUART6_TX_SELECT_INPUT    = 88,
//   kIOMUXC_LPUART7_RX_SELECT_INPUT    = 89,
//   kIOMUXC_LPUART7_TX_SELECT_INPUT    = 90,
//   kIOMUXC_LPUART8_RX_SELECT_INPUT    = 91,
//   kIOMUXC_LPUART8_TX_SELECT_INPUT    = 92,

//   kIOMUXC_NMI_SELECT_INPUT = 93,

//   kIOMUXC_QTIMER2_TIMER0_SELECT_INPUT =  94,
//   kIOMUXC_QTIMER2_TIMER1_SELECT_INPUT =  95,
//   kIOMUXC_QTIMER2_TIMER2_SELECT_INPUT =  96,
//   kIOMUXC_QTIMER2_TIMER3_SELECT_INPUT =  97,
//   kIOMUXC_QTIMER3_TIMER0_SELECT_INPUT =  98,
//   kIOMUXC_QTIMER3_TIMER1_SELECT_INPUT =  99,
//   kIOMUXC_QTIMER3_TIMER2_SELECT_INPUT = 100,
//   kIOMUXC_QTIMER3_TIMER3_SELECT_INPUT = 101,

//   kIOMUXC_SAI1_MCLK2_SELECT_INPUT    = 102,
//   kIOMUXC_SAI1_RX_BCLK_SELECT_INPUT  = 103,
//   kIOMUXC_SAI1_RX_DATA0_SELECT_INPUT = 104,
//   kIOMUXC_SAI1_RX_DATA1_SELECT_INPUT = 105,
//   kIOMUXC_SAI1_RX_DATA2_SELECT_INPUT = 106,
//   kIOMUXC_SAI1_RX_DATA3_SELECT_INPUT = 107,
//   kIOMUXC_SAI1_RX_SYNC_SELECT_INPUT  = 108,
//   kIOMUXC_SAI1_TX_BCLK_SELECT_INPUT  = 109,
//   kIOMUXC_SAI1_TX_SYNC_SELECT_INPUT  = 110,
//   kIOMUXC_SAI2_MCLK2_SELECT_INPUT    = 111,
//   kIOMUXC_SAI2_RX_BCLK_SELECT_INPUT  = 112,
//   kIOMUXC_SAI2_RX_DATA0_SELECT_INPUT = 113,
//   kIOMUXC_SAI2_RX_SYNC_SELECT_INPUT  = 114,
//   kIOMUXC_SAI2_TX_BCLK_SELECT_INPUT  = 115,
//   kIOMUXC_SAI2_TX_SYNC_SELECT_INPUT  = 116,

//   kIOMUXC_SPDIF_IN_SELECT_INPUT = 117,

//   kIOMUXC_USB_OTG2_OC_SELECT_INPUT = 118,
//   kIOMUXC_USB_OTG1_OC_SELECT_INPUT = 119,

//   kIOMUXC_USDHC1_CD_B_SELECT_INPUT = 120,
//   kIOMUXC_USDHC1_WP_SELECT_INPUT   = 121,

//   kIOMUXC_USDHC2_CLK_SELECT_INPUT   = 122,
//   kIOMUXC_USDHC2_CD_B_SELECT_INPUT  = 123,
//   kIOMUXC_USDHC2_CMD_SELECT_INPUT   = 124,
//   kIOMUXC_USDHC2_DATA0_SELECT_INPUT = 125,
//   kIOMUXC_USDHC2_DATA1_SELECT_INPUT = 126,
//   kIOMUXC_USDHC2_DATA2_SELECT_INPUT = 127,
//   kIOMUXC_USDHC2_DATA3_SELECT_INPUT = 128,
//   kIOMUXC_USDHC2_DATA4_SELECT_INPUT = 129,
//   kIOMUXC_USDHC2_DATA5_SELECT_INPUT = 130,
//   kIOMUXC_USDHC2_DATA6_SELECT_INPUT = 131,
//   kIOMUXC_USDHC2_DATA7_SELECT_INPUT = 132,
//   kIOMUXC_USDHC2_WP_SELECT_INPUT    = 133,

//   kIOMUXC_XBAR1_IN02_SELECT_INPUT = 134,
//   kIOMUXC_XBAR1_IN03_SELECT_INPUT = 135,
//   kIOMUXC_XBAR1_IN04_SELECT_INPUT = 136,
//   kIOMUXC_XBAR1_IN05_SELECT_INPUT = 137,
//   kIOMUXC_XBAR1_IN06_SELECT_INPUT = 138,
//   kIOMUXC_XBAR1_IN07_SELECT_INPUT = 139,
//   kIOMUXC_XBAR1_IN08_SELECT_INPUT = 140,
//   kIOMUXC_XBAR1_IN09_SELECT_INPUT = 141,
//   kIOMUXC_XBAR1_IN17_SELECT_INPUT = 142,
//   kIOMUXC_XBAR1_IN18_SELECT_INPUT = 143,
//   kIOMUXC_XBAR1_IN20_SELECT_INPUT = 144,
//   kIOMUXC_XBAR1_IN22_SELECT_INPUT = 145,
//   kIOMUXC_XBAR1_IN23_SELECT_INPUT = 146,
//   kIOMUXC_XBAR1_IN24_SELECT_INPUT = 147,
//   kIOMUXC_XBAR1_IN14_SELECT_INPUT = 148,
//   kIOMUXC_XBAR1_IN15_SELECT_INPUT = 149,
//   kIOMUXC_XBAR1_IN16_SELECT_INPUT = 150,
//   kIOMUXC_XBAR1_IN25_SELECT_INPUT = 151,
//   kIOMUXC_XBAR1_IN19_SELECT_INPUT = 152,
//   kIOMUXC_XBAR1_IN21_SELECT_INPUT = 153,
// };

// // IOMUXC SELECT_INPUT_1 indexes.
// enum IOMUXC_SELECT_INPUT_1 {
//     kIOMUXC_ENET2_IPG_CLK_RMII_SELECT_INPUT          = 0,
//     kIOMUXC_ENET2_IPP_IND_MAC0_MDIO_SELECT_INPUT     = 1,
//     kIOMUXC_ENET2_IPP_IND_MAC0_RXDATA_SELECT_INPUT_0 = 2,
//     kIOMUXC_ENET2_IPP_IND_MAC0_RXDATA_SELECT_INPUT_1 = 3,
//     kIOMUXC_ENET2_IPP_IND_MAC0_RXEN_SELECT_INPUT     = 4,
//     kIOMUXC_ENET2_IPP_IND_MAC0_RXERR_SELECT_INPUT    = 5,
//     kIOMUXC_ENET2_IPP_IND_MAC0_TIMER_SELECT_INPUT_0  = 6,
//     kIOMUXC_ENET2_IPP_IND_MAC0_TXCLK_SELECT_INPUT    = 7,

//     kIOMUXC_FLEXSPI2_IPP_IND_DQS_FA_SELECT_INPUT     =  8,
//     kIOMUXC_FLEXSPI2_IPP_IND_IO_FA_BIT0_SELECT_INPUT =  9,
//     kIOMUXC_FLEXSPI2_IPP_IND_IO_FA_BIT1_SELECT_INPUT = 10,
//     kIOMUXC_FLEXSPI2_IPP_IND_IO_FA_BIT2_SELECT_INPUT = 11,
//     kIOMUXC_FLEXSPI2_IPP_IND_IO_FA_BIT3_SELECT_INPUT = 12,
//     kIOMUXC_FLEXSPI2_IPP_IND_IO_FB_BIT0_SELECT_INPUT = 13,
//     kIOMUXC_FLEXSPI2_IPP_IND_IO_FB_BIT1_SELECT_INPUT = 14,
//     kIOMUXC_FLEXSPI2_IPP_IND_IO_FB_BIT2_SELECT_INPUT = 15,
//     kIOMUXC_FLEXSPI2_IPP_IND_IO_FB_BIT3_SELECT_INPUT = 16,
//     kIOMUXC_FLEXSPI2_IPP_IND_SCK_FA_SELECT_INPUT     = 17,
//     kIOMUXC_FLEXSPI2_IPP_IND_SCK_FB_SELECT_INPUT     = 18,

//     kIOMUXC_GPT1_IPP_IND_CAPIN1_SELECT_INPUT = 19,
//     kIOMUXC_GPT1_IPP_IND_CAPIN2_SELECT_INPUT = 20,
//     kIOMUXC_GPT1_IPP_IND_CLKIN_SELECT_INPUT  = 21,
//     kIOMUXC_GPT2_IPP_IND_CAPIN1_SELECT_INPUT = 22,
//     kIOMUXC_GPT2_IPP_IND_CAPIN2_SELECT_INPUT = 23,
//     kIOMUXC_GPT2_IPP_IND_CLKIN_SELECT_INPUT  = 24,

//     kIOMUXC_SAI3_IPG_CLK_SAI_MCLK_SELECT_INPUT_2   = 25,
//     kIOMUXC_SAI3_IPP_IND_SAI_RXBCLK_SELECT_INPUT   = 26,
//     kIOMUXC_SAI3_IPP_IND_SAI_RXDATA_SELECT_INPUT_0 = 27,
//     kIOMUXC_SAI3_IPP_IND_SAI_RXSYNC_SELECT_INPUT   = 28,
//     kIOMUXC_SAI3_IPP_IND_SAI_TXBCLK_SELECT_INPUT   = 29,
//     kIOMUXC_SAI3_IPP_IND_SAI_TXSYNC_SELECT_INPUT   = 30,

//     kIOMUXC_SEMC_I_IPP_IND_DQS4_SELECT_INPUT = 31,

//     kIOMUXC_CANFD_IPP_IND_CANRX_SELECT_INPUT = 32,
// };
