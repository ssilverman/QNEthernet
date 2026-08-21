// SPDX-FileCopyrightText: (c) 2021-2026 Shawn Silverman <shawn@pobox.com>
// SPDX-License-Identifier: AGPL-3.0-or-later

// driver_teensymm.cpp contains the Teensy MicroMod Ethernet interface
// implementation for the LAN8720A PHY.
// Based on the Teensy 4.1 driver.
// This file is part of the QNEthernet library.

#include "qnethernet/lwip_driver.h"

#if defined(QNETHERNET_INTERNAL_DRIVER_TEENSYMM)

// C++ includes
#include <atomic>
#include <cstring>

#include <core_pins.h>
#include <util/atomic.h>

#include "lwip/debug.h"
#include "lwip/err.h"
#include "lwip/stats.h"
#include "qnethernet/hardware/imxrt1060/CCM.h"
#include "qnethernet/hardware/imxrt1060/CCM_ANALOG.h"
#include "qnethernet/hardware/imxrt1060/ENET.h"
#include "qnethernet/hardware/imxrt1060/GPIO.h"
#include "qnethernet/hardware/imxrt1060/IOMUXC.h"
#include "qnethernet/hardware/imxrt1060/IOMUXC_GPR.h"
#include "qnethernet/hardware/imxrt1060/NVIC.h"
#include "qnethernet/hardware/imxrt1060/SCB.h"
#include "qnethernet/platforms/pgmspace.h"

// Some test code from the forum:
// https://forum.pjrc.com/index.php?threads/add-ethernet-to-a-micromod-format-teensy.67643/post-356447

// [LAN8720A](https://www.microchip.com/en-us/product/lan8720a)
// [LAN8720A Datasheet](https://ww1.microchip.com/downloads/aemDocuments/documents/UNG/ProductDocuments/DataSheets/LAN8720A-LAN8720Ai-Data-Sheet-DS00002165.pdf)
// [i.MX RT1062 Manual](https://www.pjrc.com/teensy/IMXRT1060RM_rev3.pdf)

namespace qindesign {
namespace network {
namespace driver {

using namespace qindesign::hardware::imxrt1060;

// --------------------------------------------------------------------------
//  Defines
// --------------------------------------------------------------------------

// Stronger pull-up for the straps, but even this might not be strong enough.
static constexpr uint32_t kStrapPadPullup = (0
    // HYS_0_Hysteresis_Disabled
    | IOMUXC::SW_PAD_CTL_PAD::vals::PUS(3)    // PUS_3_22K_Ohm_Pull_Up
    | IOMUXC::SW_PAD_CTL_PAD::vals::PUE(1)    // PUE_1_Pull
    | IOMUXC::SW_PAD_CTL_PAD::vals::PKE(1)    // PKE_1_Pull_Keeper_Enabled
    // ODE_0_Open_Drain_Disabled
    | IOMUXC::SW_PAD_CTL_PAD::vals::SPEED(0)  // SPEED_0_low_50MHz
    | IOMUXC::SW_PAD_CTL_PAD::vals::DSE(7)    // DSE_7_R0_7
    // SRE_0_Slow_Slew_Rate
    );
    // HYS:0 PUS:11 PUE:1 PKE:1 ODE:0 000 SPEED:00 DSE:111 00 SRE:0
    // 0xF038

static constexpr uint32_t kStrapPadPulldown = (0
    // HYS_0_Hysteresis_Disabled
    | IOMUXC::SW_PAD_CTL_PAD::vals::PUS(0)    // PUS_0_100K_Ohm_Pull_Down
    | IOMUXC::SW_PAD_CTL_PAD::vals::PUE(1)    // PUE_1_Pull
    | IOMUXC::SW_PAD_CTL_PAD::vals::PKE(1)    // PKE_1_Pull_Keeper_Enabled
    // ODE_0_Open_Drain_Disabled
    | IOMUXC::SW_PAD_CTL_PAD::vals::SPEED(0)  // SPEED_0_low_50MHz
    | IOMUXC::SW_PAD_CTL_PAD::vals::DSE(7)    // DSE_7_R0_7
    // SRE_0_Slow_Slew_Rate
    );
    // HYS:0 PUS:00 PUE:1 PKE:1 ODE:0 000 SPEED:00 DSE:111 00 SRE:0
    // 0x3038

static constexpr uint32_t kMDIOPadPullup = (0
    // HYS_0_Hysteresis_Disabled
    | IOMUXC::SW_PAD_CTL_PAD::vals::PUS(2)    // PUS_2_100K_Ohm_Pull_Up
    | IOMUXC::SW_PAD_CTL_PAD::vals::PUE(1)    // PUE_1_Pull
    | IOMUXC::SW_PAD_CTL_PAD::vals::PKE(1)    // PKE_1_Pull_Keeper_Enabled
    | IOMUXC::SW_PAD_CTL_PAD::vals::ODE(1)    // ODE_1_Open_Drain_Enabled
    | IOMUXC::SW_PAD_CTL_PAD::vals::SPEED(0)  // SPEED_0_low_50MHz
    | IOMUXC::SW_PAD_CTL_PAD::vals::DSE(5)    // DSE_5_R0_5
    | IOMUXC::SW_PAD_CTL_PAD::vals::SRE(1)    // SRE_1_Fast_Slew_Rate
    );
    // HYS:0 PUS:10 PUE:1 PKE:1 ODE:1 000 SPEED:00 DSE:101 00 SRE:1
    // 0xB829
    // Reference schematic suggests 1.5kohms, but this is what we got. It has
    // some internal resistor. Hopefully these cover what we need.

static constexpr uint32_t kMDIOMux = 8;
    // SION:0 MUX_MODE:1000
    // ALT8

// static const uint32_t kRMIIPadPulldown = (0
//     // HYS_0_Hysteresis_Disabled
//     | IOMUXC::SW_PAD_CTL_PAD::vals::PUS(0)    // PUS_0_100K_Ohm_Pull_Down
//     | IOMUXC::SW_PAD_CTL_PAD::vals::PUE(1)    // PUE_1_Pull
//     | IOMUXC::SW_PAD_CTL_PAD::vals::PKE(1)    // PKE_1_Pull_Keeper_Enabled
//     // ODE_0_Open_Drain_Disabled
//     | IOMUXC::SW_PAD_CTL_PAD::vals::SPEED(3)  // SPEED_3_max_200MHz
//     | IOMUXC::SW_PAD_CTL_PAD::vals::DSE(5)    // DSE_5_R0_5
//     | IOMUXC::SW_PAD_CTL_PAD::vals::SRE(1)    // SRE_1_Fast_Slew_Rate
//     );
//     // HYS:0 PUS:00 PUE:1 PKE:1 ODE:0 000 SPEED:11 DSE:101 00 SRE:1
//     // 0x30E9

static constexpr uint32_t kRMIIPadPullup = (0
    // HYS_0_Hysteresis_Disabled
    | IOMUXC::SW_PAD_CTL_PAD::vals::PUS(2)    // PUS_2_100K_Ohm_Pull_Up
    | IOMUXC::SW_PAD_CTL_PAD::vals::PUE(1)    // PUE_1_Pull
    | IOMUXC::SW_PAD_CTL_PAD::vals::PKE(1)    // PKE_1_Pull_Keeper_Enabled
    // ODE_0_Open_Drain_Disabled
    | IOMUXC::SW_PAD_CTL_PAD::vals::SPEED(3)  // SPEED_3_max_200MHz
    | IOMUXC::SW_PAD_CTL_PAD::vals::DSE(5)    // DSE_5_R0_5
    | IOMUXC::SW_PAD_CTL_PAD::vals::SRE(1)    // SRE_1_Fast_Slew_Rate
    );
    // HYS:0 PUS:10 PUE:1 PKE:1 ODE:0 000 SPEED:11 DSE:101 00 SRE:1
    // 0xB0E9

static constexpr uint32_t kRMIIPadSignal = (0
    // HYS_0_Hysteresis_Disabled
    // PUS_0_100K_Ohm_Pull_Down
    // PUE_0_Keeper
    | IOMUXC::SW_PAD_CTL_PAD::vals::PKE(1)    // PKE_1_Pull_Keeper_Enabled
    // ODE_0_Open_Drain_Disabled
    | IOMUXC::SW_PAD_CTL_PAD::vals::SPEED(2)  // SPEED_2_fast_150MHz
    | IOMUXC::SW_PAD_CTL_PAD::vals::DSE(6)    // DSE_6_R0_6
    // SRE_0_Slow_Slew_Rate
    );
    // HYS:0 PUS:00 PUE:0 PKE:1 ODE:0 000 SPEED:10 DSE:110 00 SRE:0
    // 0x10B0  <-- This is the default


static constexpr uint32_t kRMIIPadClock = (0
    // HYS_0_Hysteresis_Disabled
    // PUS_0_100K_Ohm_Pull_Down
    // PUE_0_Keeper
    // PKE_0_Pull_Keeper_Disabled
    // ODE_0_Open_Drain_Disabled
    | IOMUXC::SW_PAD_CTL_PAD::vals::SPEED(0)  // SPEED_0_low_50MHz
    | IOMUXC::SW_PAD_CTL_PAD::vals::DSE(6)    // DSE_6_R0_6
    | IOMUXC::SW_PAD_CTL_PAD::vals::SRE(1)    // SRE_1_Fast_Slew_Rate
    );
    // HYS:0 PUS:00 PUE:0 PKE:0 ODE:0 000 SPEED:00 DSE:110 00 SRE:1
    // 0x0031

static constexpr uint32_t kRMIIMuxClock = (9 | 0x10);
    // SION:1 MUX_MODE:1001
    // ALT9

static constexpr uint32_t kRMIIMux = 8;
    // SION:0 MUX_MODE:1000
    // ALT8

static constexpr int kMDCPin  = 33;
static constexpr int kMDIOPin = 35;

// Sizes
static constexpr size_t kRxSize = 10;
static constexpr size_t kTxSize = 10;

// Buffer size for transferring to and from the Ethernet MAC. The frame size is
// either 1518 or 1522, assuming a 1500-byte payload, depending on whether VLAN
// support is desired. VLAN support requires an extra 4 bytes. The ARM cache
// management functions require 32-bit alignment, but the ENETx_MRBR max.
// receive buffer size register says that the RX buffer size must be a multiple
// of 64 and >= 256.
//
// [1518 or 1522 made into a multiple of 32 for ARM cache flush sizing and a
// multiple of 64 for ENETx_MRBR.]
// NOTE: kBufSize will be 1536 whether we use 1518 or 1522 (plus ETH_PAD_SIZE)
// * Padding(2)
// * Destination(6) + Source(6) + VLAN tag(2) + VLAN info(2) + Length/Type(2) +
//   Payload(1500) + FCS(4)
static constexpr size_t kBufSize =
    (((ETH_PAD_SIZE + 6 + 6 + 2 + 2 + 2 + 1500 + 4) + 63) & ~63);

// static constexpr int kIRQPriority = 64

// Size checks
static_assert((kRxSize >= 1) && (kRxSize >= 1), "Rx and Tx sizes must be >= 1");

#if !QNETHERNET_BUFFERS_IN_RAM1
ATTRIBUTE_NODISCARD ATTRIBUTE_ALWAYS_INLINE
static inline uint32_t multipleOf32(uint32_t x) {
  return (x + 31u) & ~31u;
}

#define BUFFER_DMAMEM DMAMEM
#else
#define BUFFER_DMAMEM
#endif  // !QNETHERNET_BUFFERS_IN_RAM1

// --------------------------------------------------------------------------
//  Types
// --------------------------------------------------------------------------

namespace {  // Internal linkage

// Defines the control and status region of the receive buffer descriptor.
namespace rx_bd_status {
constexpr uint16_t kEmpty           = 0x8000;  // Empty bit
constexpr uint16_t kRxSoftOwner1    = 0x4000;  // Receive software ownership
constexpr uint16_t kWrap            = 0x2000;  // Wrap buffer descriptor
constexpr uint16_t kRxSoftOwner2    = 0x1000;  // Receive software ownership
constexpr uint16_t kLast            = 0x0800;  // Last BD in the frame (L bit)
constexpr uint16_t kMiss            = 0x0100;  // Miss; in promiscuous mode; needs L
constexpr uint16_t kBroadcast       = 0x0080;  // Broadcast
constexpr uint16_t kMulticast       = 0x0040;  // Multicast
constexpr uint16_t kLengthViolation = 0x0020;  // Receive length violation; needs L
constexpr uint16_t kNonOctet        = 0x0010;  // Receive non-octet aligned frame; needs L
constexpr uint16_t kCrc             = 0x0004;  // Receive CRC or frame error; needs L
constexpr uint16_t kOverrun         = 0x0002;  // Receive FIFO overrun; needs L
constexpr uint16_t kTrunc           = 0x0001;  // Frame is truncated
}  // namespace rx_bd_status

// Defines the control extended region1 of the receive buffer descriptor.
namespace rx_bd_extend0 {
constexpr uint16_t kVlanPriorityCodePoint = 0xE000;  // VLAN priority code point; needs L
constexpr uint16_t kIpHeaderChecksumErr   = 0x0020;  // IP header checksum error; needs L
constexpr uint16_t kProtocolChecksumErr   = 0x0010;  // Protocol checksum error; needs L
constexpr uint16_t kVlan                  = 0x0004;  // VLAN; needs L
constexpr uint16_t kIpv6                  = 0x0002;  // IPv6 frame; needs L
constexpr uint16_t kIpv4Fragment          = 0x0001;  // IPv4 fragment; needs L
}  // namespace rx_bd_extend0

// Defines the control extended region2 of the receive buffer descriptor.
namespace rx_bd_extend1 {
constexpr uint16_t kMacErr    = 0x8000;  // MAC error; needs L
constexpr uint16_t kPhyErr    = 0x0400;  // PHY error; needs L
constexpr uint16_t kCollision = 0x0200;  // Collision; needs L
constexpr uint16_t kUnicast   = 0x0100;  // Unicast frame; valid even if L is not set
constexpr uint16_t kInterrupt = 0x0080;  // Generate RXB/RXF interrupt
}  // namespace rx_bd_extend1

// Defines the control status of the transmit buffer descriptor.
namespace tx_bd_control {
constexpr uint16_t kReady            = 0x8000;  // Ready bit
constexpr uint16_t kTxSoftwareOwner1 = 0x4000;  // Transmit software ownership
constexpr uint16_t kWrap             = 0x2000;  // Wrap buffer descriptor
constexpr uint16_t kTxSoftwareOwner2 = 0x1000;  // Transmit software ownership
constexpr uint16_t kLast             = 0x0800;  // Last BD in the frame (L bit)
constexpr uint16_t kTxCrc            = 0x0400;  // Transmit CRC; needs L
}  // namespace tx_bd_control

// Defines the control extended region1 of the transmit buffer descriptor.
namespace tx_bd_extend0 {
constexpr uint16_t kTxErr              = 0x8000;  // Transmit error; needs L
constexpr uint16_t kUnderflowErr       = 0x2000;  // Underflow error; needs L
constexpr uint16_t kExcessCollisionErr = 0x1000;  // Excess collision error; needs L
constexpr uint16_t kFrameErr           = 0x0800;  // Frame with error; needs L
constexpr uint16_t kLateCollisionErr   = 0x0400;  // Late collision error; needs L
constexpr uint16_t kOverflowErr        = 0x0200;  // Overflow error; needs L
constexpr uint16_t kTimestampErr       = 0x0100;  // Timestamp error; needs L
}  // namespace tx_bd_extend0

// Defines the control extended region2 of the transmit buffer descriptor.
namespace tx_bd_extend1 {
constexpr uint16_t kInterrupt        = 0x4000;  // Generate interrupt flags; all BDs
constexpr uint16_t kTimestamp        = 0x2000;  // Timestamp; all BDs
constexpr uint16_t kProtocolChecksum = 0x1000;  // Insert protocol specific checksum; all BDs
constexpr uint16_t kIpHeaderChecksum = 0x0800;  // Insert IP header checksum; all BDs
}  // namespace tx_bd_extend1

}  // namespace

struct ATTRIBUTE_PACKED BufferDescriptor {
  uint16_t length;
  union {
    uint16_t status;   // Rx
    uint16_t control;  // Tx
  };
  void*    buffer;
  uint16_t extend0;
  uint16_t extend1;
  uint16_t checksum;   // Rx
  uint8_t  protoType;  // Rx
  uint8_t  headerLen;  // Rx
  uint16_t unused0;
  uint16_t extend2;
  uint32_t timestamp;
  uint16_t unused1;
  uint16_t unused2;
  uint16_t unused3;
  uint16_t unused4;
};

enum class InitStates {
  kStart,           // Unknown hardware
  kNoHardware,      // No PHY
  kHasHardware,     // Has PHY
  kPHYInitialized,  // PHY's been initialized
  kInitialized,     // PHY and MAC have been initialized
};

// --------------------------------------------------------------------------
//  Internal Variables
// --------------------------------------------------------------------------

// Ethernet buffers
alignas(64) static BufferDescriptor s_rxRing[kRxSize];
alignas(64) static BufferDescriptor s_txRing[kTxSize];
alignas(64) static uint8_t s_rxBufs[kRxSize * kBufSize] BUFFER_DMAMEM;
alignas(64) static uint8_t s_txBufs[kTxSize * kBufSize] BUFFER_DMAMEM;
static volatile BufferDescriptor* s_pRxBD = &s_rxRing[0];
static volatile BufferDescriptor* s_pTxBD = &s_txRing[0];

// Misc. internal state
static std::atomic_flag s_rxNotAvail = ATOMIC_FLAG_INIT;
static InitStates s_initState = InitStates::kStart;

// PHY status, polled
static int s_checkLinkStatusState = 0;
static LinkInfo s_linkInfo;

// Notification data
static bool s_manualLinkState = false;  // True for sticky

// Incoming MAC address hash-collision bookkeeping
// Don't release bits that have had a collision. Track these here.
static uint32_t s_collisionGALR = 0;
static uint32_t s_collisionGAUR = 0;
static uint32_t s_collisionIALR = 0;
static uint32_t s_collisionIAUR = 0;

SCB::VTOR::Vector s_prevENETVector = nullptr;

// Forward declarations
static void enet_isr();

// --------------------------------------------------------------------------
//  PHY I/O
// --------------------------------------------------------------------------

namespace {  // Internal linkage

// PHY register definitions.
namespace phy_regs {
constexpr uint16_t kBCR      = 0;   // Basic Control Register
constexpr uint16_t kBSR      = 1;   // Basic Status Register
constexpr uint16_t kPHYID1   = 2;   // PHY Identifier 1 Register
constexpr uint16_t kPHYID2   = 3;   // PHY Identifier 2 Register
constexpr uint16_t kANAR     = 4;   // Auto Negotiation Advertisement Register
constexpr uint16_t kPHYSCSR  = 31;  // PHY Special Control/Status Register
}  // namespace phy_regs

namespace phy_vals {
constexpr uint16_t kBCR_DUPLEX_MODE      = (1 <<  8);  // 0: Half duplex, 1: Full duplex
constexpr uint16_t kBCR_RESTART_AUTO_NEG = (1 <<  9);  // 0: Normal, 1: Restart (Self-clearing)
constexpr uint16_t kBCR_AUTO_NEG_ENABLE  = (1 << 12);  // 0: Disable, 1: Enable
constexpr uint16_t kBCR_SPEED_SELECT     = (1 << 13);  // 0: 10Mbps, 1: 100Mbps
constexpr uint16_t kBCR_SOFT_RESET       = (1 << 15);  // 1: Software reset (Self-clearing)

constexpr uint16_t kBSR_LINK_STATUS      = (1 << 2);  // 0: No link, 1: Valid link
}  // namespace phy_vals

}  // namespace

ATTRIBUTE_ALWAYS_INLINE
static inline void writeMDIOBit(const int n) {
  digitalWriteFast(kMDCPin, LOW);
  digitalWriteFast(kMDIOPin, (n == 0) ? LOW : HIGH);
  delayNanoseconds(200);
  digitalWriteFast(kMDCPin, HIGH);
  delayNanoseconds(200);
}

ATTRIBUTE_ALWAYS_INLINE
static inline uint16_t readMDIOBit(const uint16_t r) {
  digitalWriteFast(kMDCPin, LOW);
  delayNanoseconds(200);
  digitalWriteFast(kMDCPin, HIGH);
  const uint16_t v = (r << 1) | ((digitalReadFast(kMDIOPin) == HIGH) ? 1 : 0);
  delayNanoseconds(200);
  return v;
}

ATTRIBUTE_ALWAYS_INLINE
static inline void doClockCycle() {
  digitalWriteFast(kMDCPin, LOW);
  delayNanoseconds(200);
  digitalWriteFast(kMDCPin, HIGH);
  delayNanoseconds(200);
}

// Blocking MDIO read.
uint16_t mdio_read(const uint16_t regaddr) {
  // 32 1's
  digitalWriteFast(kMDIOPin, HIGH);
  for (int i = 0; i < 32; ++i) {
    doClockCycle();
  }

  // Start of Frame (0 1)
  writeMDIOBit(0);
  writeMDIOBit(1);

  // Read opcode (1 0)
  writeMDIOBit(1);
  writeMDIOBit(0);

  // PHY Address (0 0 0 0 0)
  digitalWriteFast(kMDIOPin, LOW);
  doClockCycle();
  doClockCycle();
  doClockCycle();
  doClockCycle();
  doClockCycle();

  // Register Address
  for (int i = 5; i-- > 0; ){
    writeMDIOBit((regaddr >> i) & 0x01);
  }

  // Turnaround (Z 0)
  doClockCycle();
  writeMDIOBit(0);

  // Data
  pinMode(kMDIOPin, INPUT_PULLUP);
  uint16_t r = 0;
  for (int i = 0; i < 16; ++i) {
    r = readMDIOBit(r);
  }
  pinMode(kMDIOPin, OUTPUT);

  digitalWriteFast(kMDCPin, LOW);
  return r;
}

// Blocking MDIO write.
void mdio_write(const uint16_t regaddr, const uint16_t data) {
  // 32 1's
  digitalWriteFast(kMDIOPin, HIGH);
  for (int i = 0; i < 32; ++i) {
    doClockCycle();
  }

  // Start of Frame (0 1)
  writeMDIOBit(0);
  writeMDIOBit(1);

  // Read opcode (0 1)
  writeMDIOBit(0);
  writeMDIOBit(1);

  // PHY Address (0 0 0 0 0)
  digitalWriteFast(kMDIOPin, LOW);
  doClockCycle();
  doClockCycle();
  doClockCycle();
  doClockCycle();
  doClockCycle();

  // Register Address
  for (int i = 5; i-- > 0; ){
    writeMDIOBit((regaddr >> i) & 0x01);
  }

  // Turnaround (1 0)
  writeMDIOBit(1);
  writeMDIOBit(0);

  // Data
  for (int i = 16; i-- > 0; ) {
    writeMDIOBit((data >> i) & 0x01);
  }

  digitalWriteFast(kMDCPin, LOW);
}

// --------------------------------------------------------------------------
//  Low-Level
// --------------------------------------------------------------------------

// Enables the Ethernet-related clocks. See also disable_enet_clocks().
FLASHMEM static void enable_enet_clocks() {
  // Enable the Ethernet clocks
  CCM::CCGR1::ENET  = CCM::CCGR::kOn;
  CCM::CCGR7::ENET2 = CCM::CCGR::kOn;

  // Configure PLL6 for 50 MHz (page 1112)
  CCM_ANALOG::PLL_ENET_SET::BYPASS = 1;
  CCM_ANALOG::group->PLL_ENET_CLR = 0
                                    | CCM_ANALOG::PLL_ENET::BYPASS_CLK_SRC(3)
                                    | CCM_ANALOG::PLL_ENET::ENET2_DIV_SELECT(3)
                                    | CCM_ANALOG::PLL_ENET::DIV_SELECT(3)
                                    ;
  CCM_ANALOG::group->PLL_ENET_SET = 0
                                    // | CCM_ANALOG::PLL_ENET::ENET_25M_REF_EN(1)
                                    | CCM_ANALOG::PLL_ENET::ENET2_REF_EN(1)
                                    // | CCM_ANALOG::PLL_ENET::ENABLE(1)
                                    | CCM_ANALOG::PLL_ENET::ENET2_DIV_SELECT(1)  // 50MHz
                                    // | CCM_ANALOG::PLL_ENET::DIV_SELECT(1)        // 50MHz
                                    ;
  CCM_ANALOG::PLL_ENET_CLR::POWERDOWN = 1;
  while (CCM_ANALOG::PLL_ENET::LOCK == 0) {
    // Wait for PLL lock
  }
  CCM_ANALOG::PLL_ENET_CLR::BYPASS = 1;
  // printf("PLL6 = %08" PRIX32 "h (should be 80202001h)\n", CCM_ANALOG::group->PLL_ENET);

  // Configure REFCLK to be driven as output by PLL6 (page 325)
  IOMUXC_GPR::GPR1::ENET2_CLK_SEL = 0;
  IOMUXC_GPR::group->GPR1 |= IOMUXC_GPR::GPR1::ENET_IPG_CLK_S_EN(1) |
                             IOMUXC_GPR::GPR1::ENET2_TX_CLK_DIR(1);
}

// Disables everything enabled with enable_enet_clocks().
FLASHMEM static void disable_enet_clocks() {
  // Configure REFCLK
  IOMUXC_GPR::GPR1::ENET2_TX_CLK_DIR = 0;

  // Stop the PLL (first bypassing)
  CCM_ANALOG::PLL_ENET_SET::BYPASS = 1;
  CCM_ANALOG::group->PLL_ENET = 0
                                | CCM_ANALOG::PLL_ENET::BYPASS(1)      // Reset to default
                                | CCM_ANALOG::PLL_ENET::POWERDOWN(1)
                                | CCM_ANALOG::PLL_ENET::DIV_SELECT(1)
                                ;

  // Disable the clocks for ENET
  CCM::CCGR7::ENET2 = CCM::CCGR::kOff;
  CCM::CCGR1::ENET  = CCM::CCGR::kOff;
}

// Configures all the pins necessary for communicating with the PHY.
FLASHMEM static void configure_phy_pins() {
  // PHY strap pins:
  // 3.7.1 PHYAD[0]: PHY Address Configuration (page 26) (default 0)
  //     RXER, Pin 10, Teensy pin 8
  // 3.7.2 MODE[2:0]: Mode Configuration (page 27) (default 111, all-capable)
  //     RXD0, Pin 8, Teensy pin 34
  //     RXD1, Pin 7, Teensy pin 38
  //     CRS_DV, Pin 11, Teensy pin 39
  // 3.7.3 REGOFF: Internal +1.2V Regulator Configuration (page 28) (default 0)
  // 3.7.4 nINTSEL: nINT/REFCLKO Configuration (page 28) (default 1)

  // Configure the MDIO and MDC pins for bit-bang
  pinMode(kMDCPin, OUTPUT);
  pinMode(kMDIOPin, OUTPUT);
}

// Configures all the RMII pins. This should be called after initializing
// the PHY.
FLASHMEM static void configure_rmii_pins() {
  // TODO: Figure out what these should be
  // The NXP SDK and original Teensy 4.1 example code use pull-ups
  IOMUXC::group->SW_PAD_CTL_PAD[IOMUXC::SW_CTL_PAD::kSD_B0_03] = kRMIIPadSignal;  // RXD0
  IOMUXC::group->SW_PAD_CTL_PAD[IOMUXC::SW_CTL_PAD::kSD_B0_04] = kRMIIPadSignal;  // RXD1
  IOMUXC::group->SW_PAD_CTL_PAD[IOMUXC::SW_CTL_PAD::kSD_B0_05] = kRMIIPadSignal;  // CRS_DV
  IOMUXC::group->SW_PAD_CTL_PAD[IOMUXC::SW_CTL_PAD::kB1_00]    = kRMIIPadSignal;  // RXER
  IOMUXC::group->SW_PAD_CTL_PAD[IOMUXC::SW_CTL_PAD::kB1_07]    = kRMIIPadSignal;  // TXD0
  IOMUXC::group->SW_PAD_CTL_PAD[IOMUXC::SW_CTL_PAD::kB1_08]    = kRMIIPadSignal;  // TXD1
  IOMUXC::group->SW_PAD_CTL_PAD[IOMUXC::SW_CTL_PAD::kB1_09]    = kRMIIPadSignal;  // TXEN (PHY has internal pull-down)

  IOMUXC::group->SW_MUX_CTL_PAD[IOMUXC::SW_CTL_PAD::kSD_B0_03] = kRMIIMux;  // RXD0 pin 8 (ENET2_RDATA00 of enet, page 539), Teensy pin 34
  IOMUXC::group->SW_MUX_CTL_PAD[IOMUXC::SW_CTL_PAD::kSD_B0_04] = kRMIIMux;  // RXD1 pin 7 (ENET2_RDATA01 of enet, page 540), Teensy pin 38
  IOMUXC::group->SW_MUX_CTL_PAD[IOMUXC::SW_CTL_PAD::kB1_00]    = kRMIIMux;  // RXER pin 10 (ENET2_RX_ER of enet, page 520), Teensy pin 8
  IOMUXC::group->SW_MUX_CTL_PAD[IOMUXC::SW_CTL_PAD::kSD_B0_05] = kRMIIMux;  // CRS_DV pin 11 (ENET2_RX_EN of enet, page 541), Teensy pin 39
  IOMUXC::group->SW_MUX_CTL_PAD[IOMUXC::SW_CTL_PAD::kEMC_32]   = kRMIIMux;  // TXEN pin 16 (ENET2_TX_EN of enet, page 458), Teensy pin 28
  IOMUXC::group->SW_MUX_CTL_PAD[IOMUXC::SW_CTL_PAD::kB0_12]    = kRMIIMux;  // TXD0 pin 17 (ENET2_TDATA00 of enet, page 516), Teensy pin 32
  IOMUXC::group->SW_MUX_CTL_PAD[IOMUXC::SW_CTL_PAD::kEMC_31]   = kRMIIMux;  // TXD1 pin 18 (ENET2_TDATA01 of enet, page 457), Teensy pin 29

  IOMUXC::SELECT_INPUT::ENET2_IPG_CLK_RMII::DAISY = 1;  // GPIO_SD_B0_01_ALT9 (page 914)
      // DAISY:1

  // IOMUXC::SELECT_INPUT::ENET2_IPP_IND_MAC0_MDIO::DAISY     = 1;  // GPIO_B0_01_ALT8 (page 915)
  IOMUXC::SELECT_INPUT::ENET2_IPP_IND_MAC0_RXDATA_0::DAISY = 1;  // GPIO_SD_B0_03_ALT8 (page 916)
  IOMUXC::SELECT_INPUT::ENET2_IPP_IND_MAC0_RXDATA_1::DAISY = 1;  // GPIO_SD_B0_04_ALT8 (page 916)
  IOMUXC::SELECT_INPUT::ENET2_IPP_IND_MAC0_RXEN::DAISY     = 1;  // GPIO_SD_B0_05_ALT8 (page 917)
  IOMUXC::SELECT_INPUT::ENET2_IPP_IND_MAC0_RXERR::DAISY    = 2;  // GPIO_B1_00_ALT8 (page 918)
  IOMUXC::SELECT_INPUT::ENET2_IPP_IND_MAC0_TXCLK::DAISY    = 1;  // GPIO_SD_B0_01_ALT8 (page 920)
}

// Initialization and check for hardware. This does nothing if the init state
// isn't at START or HAS_HARDWARE. After this function returns, the init state
// will either be NO_HARDWARE or PHY_INITIALIZED, unless it wasn't START or
// HAS_HARDWARE when called.
FLASHMEM static void init_phy() {
  if ((s_initState != InitStates::kStart) &&
      (s_initState != InitStates::kHasHardware)) {
    return;
  }

  enable_enet_clocks();

  configure_phy_pins();

  // Note: Ensure the clock is present at the CLKIN at power up
  IOMUXC::group->SW_PAD_CTL_PAD[IOMUXC::SW_CTL_PAD::kSD_B0_01] = kRMIIPadClock;
  IOMUXC::group->SW_MUX_CTL_PAD[IOMUXC::SW_CTL_PAD::kSD_B0_01] = kRMIIMuxClock;  // CLKIN pin 5 (ENET2_REF_CLK2 of enet, page 537), Teensy pin 36

  // Ensure the device is out of reset and the power supply is stable
  delay(50);

  // Check for PHY presence
  // PHYID1: PHY ID Number: OUI bits 18-3: 0x0007
  // PHYID2: PHY ID Number: OUI bits 24-19: 110000b
  //         Model Number:                  001111b
  //         Revision Number: 4 bits
  const uint16_t id1 = mdio_read(phy_regs::kPHYID1);
  const uint16_t id2 = mdio_read(phy_regs::kPHYID2);
  if ((id1 != 0x0007) || ((id2 & 0xfff0) != 0xC0F0)) {
    disable_enet_clocks();

    s_initState = InitStates::kNoHardware;
    return;
  }

  // Configure the PHY registers
  // The strap pulls may not have been strong enough, or they can't be set, so
  // ensure those values are set properly too

  // mdio_write(phy_regs::kBCR, 0x3100);   // 13: Speed_Select: 1=100Mbps
  //                                       // 12: Auto-Negotiation Enable: 1=enabled
  //                                       //  8: Duplex Mode: 1=Full-Duplex
  // mdio_write(phy_regs::kANAR, 0x01E1);  // 8: 100BASE-TX Full Duplex: 1=advertise
  //                                       // 7: 100BASE-TX: 1=advertise
  //                                       // 6: 10BASE-T Full Duplex: 1=advertise
  //                                       // 5: 10BASE-T: 1=advertise
  //                                       // 4-0: Selector Field: IEEE 802.3

  s_initState = InitStates::kPHYInitialized;
}

// Low-level input function that transforms a received frame into an lwIP pbuf.
// This returns a newly-allocated pbuf, or NULL if there was a frame error or
// allocation error.
ATTRIBUTE_NODISCARD
static struct pbuf* low_level_input(volatile BufferDescriptor* const pBD) {
  const uint16_t err_mask = rx_bd_status::kTrunc    |
                            rx_bd_status::kOverrun  |
                            rx_bd_status::kCrc      |
                            rx_bd_status::kNonOctet |
                            rx_bd_status::kLengthViolation;

  struct pbuf* p = nullptr;

  // Determine if a frame has been received
  if ((pBD->status & err_mask) != 0) {
#if LINK_STATS
    // Either truncated or others
    if ((pBD->status & rx_bd_status::kTrunc) != 0) {
      LINK_STATS_INC(link.lenerr);
    } else if ((pBD->status & rx_bd_status::kLast) != 0) {
      // The others are only valid if the 'L' bit is set
      if ((pBD->status & rx_bd_status::kOverrun) != 0) {
        LINK_STATS_INC(link.err);
      } else {  // Either overrun and others zero, or others
        if ((pBD->status & rx_bd_status::kNonOctet) != 0) {
          LINK_STATS_INC(link.err);
        } else if ((pBD->status & rx_bd_status::kCrc) != 0) {  // Non-octet or CRC
          LINK_STATS_INC(link.chkerr);
        }
        if ((pBD->status & rx_bd_status::kLengthViolation) != 0) {
          LINK_STATS_INC(link.lenerr);
        }
      }
    }
    LINK_STATS_INC(link.drop);
#endif  // LINK_STATS
  } else {
    LINK_STATS_INC(link.recv);
    p = pbuf_alloc(PBUF_RAW, pBD->length, PBUF_POOL);
    if (p != nullptr) {
#if !QNETHERNET_BUFFERS_IN_RAM1
      arm_dcache_delete(pBD->buffer, multipleOf32(p->tot_len));
#endif  // !QNETHERNET_BUFFERS_IN_RAM1
      const err_t err = pbuf_take(p, pBD->buffer, p->tot_len);
      if (err != ERR_OK) {
          LWIP_PLATFORM_ASSERT("Expected space for pbuf fill");
      }
    } else {
      LINK_STATS_INC(link.drop);
      LINK_STATS_INC(link.memerr);
    }
  }

  // Set rx bd empty
  pBD->status = (pBD->status & rx_bd_status::kWrap) | rx_bd_status::kEmpty;

  ENET2::RDAR::RDAR = 1;

  return p;
}

// Acquires a buffer descriptor. Meant to be used with update_bufdesc().
// This waits until there is a TX buffer available.
ATTRIBUTE_NODISCARD
static inline volatile BufferDescriptor* get_bufdesc() {
  volatile BufferDescriptor* const pBD = s_pTxBD;

  while ((pBD->control & tx_bd_control::kReady) != 0) {
    // Wait until a free buffer is available
    // TODO: Limit count?
  }

  return pBD;
}

// Updates a buffer descriptor. Meant to be used with get_bufdesc().
static inline void update_bufdesc(volatile BufferDescriptor* const pBD,
                                  const uint16_t len) {
  pBD->length  = len;
  pBD->control = (pBD->control & tx_bd_control::kWrap) |
                 tx_bd_control::kTxCrc                 |
                 tx_bd_control::kLast                  |
                 tx_bd_control::kReady;

  ENET2::TDAR::TDAR = 1;

  if ((pBD->control & tx_bd_control::kWrap) != 0) {
    s_pTxBD = &s_txRing[0];
  } else {
    ++s_pTxBD;
  }

  LINK_STATS_INC(link.xmit);
}

// Finds the next non-empty BD.
ATTRIBUTE_NODISCARD
static inline volatile BufferDescriptor* rxbd_next() {
  volatile BufferDescriptor* pBD = s_pRxBD;

  while ((pBD->status & rx_bd_status::kEmpty) != 0) {
    if ((pBD->status & rx_bd_status::kWrap) != 0) {
      pBD = &s_rxRing[0];
    } else {
      ++pBD;
    }
    if (pBD == s_pRxBD) {
      return nullptr;
    }
  }

  if ((s_pRxBD->status & rx_bd_status::kWrap) != 0) {
    s_pRxBD = &s_rxRing[0];
  } else {
    ++s_pRxBD;
  }
  return pBD;
}

// The Ethernet ISR.
static void enet_isr() {
  if (ENET2::EIR::RXF != 0) {
    ENET2::EIR::RXF = 1;
    std::atomic_flag_clear(&s_rxNotAvail);
  }
}

// Checks the link status and returns zero if complete and a state value if
// not complete. The return value should be used in the next call to
// this function.
ATTRIBUTE_NODISCARD
static inline int check_link_status(struct netif* const netif,
                                    const int state) {
  static uint16_t bsr;
  static uint16_t physcsr;
  static uint8_t is_link_up;

  if (s_initState != InitStates::kInitialized) {
    return 0;
  }

  switch (state) {
    case 0:
      ATTRIBUTE_FALLTHROUGH;
    case 1:
      bsr = mdio_read(phy_regs::kBSR);
      is_link_up = ((bsr & phy_vals::kBSR_LINK_STATUS) != 0);
      if (!is_link_up) {
        break;
      }
      ATTRIBUTE_FALLTHROUGH;

    case 2:
      physcsr = mdio_read(phy_regs::kPHYSCSR);
      break;

    default:
      break;
  }

  if (netif_is_link_up(netif) != is_link_up) {
    if (is_link_up) {
      switch ((physcsr >> 2) & 0x07) {
        case 1:
          s_linkInfo.speed = 10;
          s_linkInfo.fullNotHalfDuplex = false;
          break;
        case 5:
          s_linkInfo.speed = 10;
          s_linkInfo.fullNotHalfDuplex = true;
          break;
        case 2:
          s_linkInfo.speed = 100;
          s_linkInfo.fullNotHalfDuplex = false;
          break;
        case 6:
          s_linkInfo.speed = 100;
          s_linkInfo.fullNotHalfDuplex = true;
          break;
        default:
          s_linkInfo.speed = 10;
          s_linkInfo.fullNotHalfDuplex = false;
      }

      netif_set_link_up(netif);
    } else {
      if (!s_manualLinkState) {
        netif_set_link_down(netif);
      }
    }
  }

  return 0;
}

// --------------------------------------------------------------------------
//  Driver Interface
// --------------------------------------------------------------------------

FLASHMEM void get_capabilities(DriverCapabilities* const dc) {
  dc->isMACSettable                = true;
  dc->isLinkStateDetectable        = true;
  dc->isLinkSpeedDetectable        = true;
  dc->isLinkSpeedSettable          = true;
  dc->isLinkFullDuplexDetectable   = true;
  dc->isLinkFullDuplexSettable     = true;
  dc->isAutoNegotiationSettable    = true;
  dc->isLinkCrossoverDetectable    = false;
  dc->isAutoNegotiationRestartable = true;
  dc->isPHYResettable              = true;
}

bool is_unknown() {
  return s_initState == InitStates::kStart;
}

extern "C" {
void qnethernet_hal_get_system_mac_address(uint8_t mac[ETH_HWADDR_LEN]);
}  // extern "C"

void get_system_mac(uint8_t mac[ETH_HWADDR_LEN]) {
  qnethernet_hal_get_system_mac_address(mac);
}

bool get_mac(uint8_t mac[ETH_HWADDR_LEN]) {
  // Don't do anything if the Ethernet clock isn't running because register
  // access will freeze the machine
  if (CCM::CCGR7::ENET2 == CCM::CCGR::kOff) {
    return false;
  }

  const uint32_t rl = *ENET2::PALR::PADDR1;
  const uint32_t ru = *ENET2::PAUR::PADDR2;
  mac[0] = static_cast<uint8_t>(rl >> 24);
  mac[1] = static_cast<uint8_t>(rl >> 16);
  mac[2] = static_cast<uint8_t>(rl >>  8);
  mac[3] = static_cast<uint8_t>(rl >>  0);
  mac[4] = static_cast<uint8_t>(ru >> 24);
  mac[5] = static_cast<uint8_t>(ru >> 16);

  return true;
}

bool set_mac(const uint8_t mac[ETH_HWADDR_LEN]) {
  // Don't do anything if the Ethernet clock isn't running because register
  // access will freeze the machine
  if (CCM::CCGR7::ENET2 == CCM::CCGR::kOff) {
    return false;
  }

  // TODO: Not sure if disabling interrupts is really needed
  ATOMIC_BLOCK(ATOMIC_RESTORESTATE) {
    ENET2::PALR::PADDR1 = (uint32_t{mac[0]} << 24) | (uint32_t{mac[1]} << 16) |
                          (uint32_t{mac[2]} <<  8) | (uint32_t{mac[3]} <<  0);
    ENET2::PAUR::PADDR2 = (uint32_t{mac[4]} << 8) | (uint32_t{mac[5]} << 0);
  }

  return true;
  }

bool has_hardware() {
  switch (s_initState) {
    case InitStates::kHasHardware:
      ATTRIBUTE_FALLTHROUGH;
    case InitStates::kPHYInitialized:
      ATTRIBUTE_FALLTHROUGH;
    case InitStates::kInitialized:
      return true;
    case InitStates::kNoHardware:
      return false;
    default:
      break;
  }
  init_phy();
  return (s_initState != InitStates::kNoHardware);
}

void set_chip_select_pin(const int pin) {
  (void)pin;
}

// Initializes the PHY and Ethernet interface. This sets the init state and
// returns whether the initialization was successful.
FLASHMEM bool init() {
  if (s_initState == InitStates::kInitialized) {
    return true;
  }

  init_phy();
  if (s_initState != InitStates::kPHYInitialized) {
    return false;
  }

  // Configure pins
  // TODO: What should these actually be? Why pull-ups? Note that the reference code uses pull-ups.
  // Note: The original code left RXD0, RXEN, and RXER with PULLDOWN
  configure_rmii_pins();

  (void)std::memset(s_rxRing, 0, sizeof(s_rxRing));
  (void)std::memset(s_txRing, 0, sizeof(s_txRing));
  s_pRxBD = &s_rxRing[0];
  s_pTxBD = &s_txRing[0];

  for (size_t i = 0; i < kRxSize; ++i) {
    s_rxRing[i].buffer  = &s_rxBufs[i * kBufSize];
    s_rxRing[i].status  = rx_bd_status::kEmpty;
    s_rxRing[i].extend1 = rx_bd_extend1::kInterrupt;
  }
  // The last buffer descriptor should be set with the wrap flag
  s_rxRing[kRxSize - 1].status |= rx_bd_status::kWrap;

  for (size_t i = 0; i < kTxSize; ++i) {
    s_txRing[i].buffer  = &s_txBufs[i * kBufSize];
    s_txRing[i].control = tx_bd_control::kTxCrc;
    s_txRing[i].extend1 = 0
                          | tx_bd_extend1::kInterrupt
#if !QNETHERNET_ENABLE_RAW_FRAME_SUPPORT
                          | tx_bd_extend1::kProtocolChecksum
                          | tx_bd_extend1::kIpHeaderChecksum
#endif  // !QNETHERNET_ENABLE_RAW_FRAME_SUPPORT
                          ;
  }
  s_txRing[kTxSize - 1].control |= tx_bd_control::kWrap;

  ENET2::group->EIMR = 0;  // This also deasserts all interrupts

  ENET2::group->RCR = 0
                     | ENET2::RCR::NLC(1)        // Payload length is checked
                     | ENET2::RCR::MAX_FL((MAX_FRAME_LEN) + 4)  // Include the 4-byte CRC
                     | ENET2::RCR::CFEN(1)       // Discard non-pause MAC control frames
                     | ENET2::RCR::CRCFWD(1)     // CRC is stripped (ignored if PADEN)
                     | ENET2::RCR::PADEN(1)      // Padding is removed
                     | ENET2::RCR::RMII_MODE(1)
                     | ENET2::RCR::FCE(1)        // Flow control enable
#if QNETHERNET_ENABLE_PROMISCUOUS_MODE
                     | ENET2::RCR::PROM(1)       // Promiscuous mode
#endif  // QNETHERNET_ENABLE_PROMISCUOUS_MODE
                     | ENET2::RCR::MII_MODE(1)
                     ;
  ENET2::group->TCR = 0
#if !QNETHERNET_ENABLE_RAW_FRAME_SUPPORT
                     | ENET2::TCR::ADDINS(1)     // Overwrite with programmed MAC address
#endif  // !QNETHERNET_ENABLE_RAW_FRAME_SUPPORT
                     | ENET2::TCR::ADDSEL(0)
                     // | ENET2::TCR::RFC_PAUSE(1)
                     // | ENET2::TCR::TFC_PAUSE(1)
                     | ENET2::TCR::FDEN(1)       // Enable full-duplex
                     ;

  ENET2::group->TACC = 0
#if (CHECKSUM_GEN_UDP == 0) || \
    (CHECKSUM_GEN_TCP == 0) || \
    (CHECKSUM_GEN_ICMP == 0)
                       | ENET2::TACC::PROCHK(1)  // Insert protocol checksum
#endif  // not(Generate all checksums)
#if CHECKSUM_GEN_IP == 0
                       | ENET2::TACC::IPCHK(1)   // Insert IP header checksum
#endif  // CHECKSUM_GEN_IP == 0
#if ETH_PAD_SIZE == 2
                       | ENET2::TACC::SHIFT16(1)
#endif  // ETH_PAD_SIZE == 2
                       ;

  ENET2::group->RACC = 0
#if ETH_PAD_SIZE == 2
                       | ENET2::RACC::SHIFT16(1)
#endif  // ETH_PAD_SIZE == 2
                       | ENET2::RACC::LINEDIS(1)  // Discard bad frames
#if (CHECKSUM_CHECK_UDP == 0) && \
    (CHECKSUM_CHECK_TCP == 0) && \
    (CHECKSUM_CHECK_ICMP == 0)
                       | ENET2::RACC::PRODIS(1)   // Discard frames with incorrect protocol checksum
                                                  // Requires RSFL == 0
#endif  // not(Check any checksums)
#if CHECKSUM_CHECK_IP == 0
                       | ENET2::RACC::IPDIS(1)    // Discard frames with incorrect IPv4 header checksum
                                                  // Requires RSFL == 0
#endif  // CHECKSUM_CHECK_IP == 0
                       | ENET2::RACC::PADREM(1)
      ;

  ENET2::TFWR::STRFWD = 1;
  ENET2::RSFL::RX_SECTION_FULL = 0;

  ENET2::group->RDSR = reinterpret_cast<uint32_t>(s_rxRing);
  ENET2::group->TDSR = reinterpret_cast<uint32_t>(s_txRing);
  ENET2::group->MRBR = kBufSize;

  ENET2::group->RXIC[0] = 0;
  ENET2::group->TXIC[0] = 0;
  // ENET2::PALR::PADDR1 = (uint32_t{mac[0]} << 24) | (uint32_t{mac[1]} << 16) |
  //                       (uint32_t{mac[2]} <<  8) | (uint32_t{mac[3]} <<  0);
  // ENET2::PAUR::PADDR2 = (uint32_t{mac[4]} << 8) | (uint32_t{mac[5]} << 0);

  ENET2::OPD::PAUSE_DUR = 0x0014;
  ENET2::group->RSEM    = 0;

  ENET2::MIBC::MIB_DIS = 1;  // Start with MIB logic disabled (RFC 2819)

  ENET2::IAUR::IADDR1 = 0;
  ENET2::IALR::IADDR2 = 0;
  ENET2::GAUR::GADDR1 = 0;
  ENET2::GALR::GADDR2 = 0;

  ENET2::EIMR::RXF = 1;
  s_prevENETVector = SCB::VTOR::getVector(NVIC::IRQ::kENET2);
  SCB::VTOR::setVector(NVIC::IRQ::kENET2, &enet_isr);
  NVIC::IRQ::enable(NVIC::IRQ::kENET2);

  // Last few things to do
  ENET2::group->EIR = 0x7fff'8000;  // Clear any pending interrupts before setting ETHEREN
  (void)std::atomic_flag_test_and_set(&s_rxNotAvail);

  // Last, enable the Ethernet MAC

  ENET2::group->ECR = ENET2::ECR::kWOO |
                      ENET2::ECR::DBSWP(1) |
                      ENET2::ECR::EN1588(1) |
                      ENET2::ECR::ETHEREN(1);

  // Indicate there are empty RX buffers and available ready TX buffers
  ENET2::RDAR::RDAR = 1;
  ENET2::TDAR::TDAR = 1;

  // PHY soft reset
  // mdio_write(phy_regs::kBMCR, 1u << 15);

  s_initState = InitStates::kInitialized;

  return true;
}

FLASHMEM void deinit() {
  // Something about stopping Ethernet and the PHY kills performance if Ethernet
  // is restarted after calling end(), so gate the following two blocks with a
  // macro for now

  // Clear the MAC address hash-collision bookkeeping
  s_collisionGALR = 0;
  s_collisionGAUR = 0;
  s_collisionIALR = 0;
  s_collisionIAUR = 0;

#if QNETHERNET_INTERNAL_END_STOPS_ALL
  if (s_initState == InitStates::kInitialized) {
    NVIC::IRQ::disable(NVIC::IRQ::kENET2);
    SCB::VTOR::setVector(NVIC::IRQ::kENET2, s_prevENETVector);
    s_prevENETVector = nullptr;
    ENET2::group->EIMR = 0;  // Disable interrupts

    // Gracefully stop any transmission before disabling the Ethernet MAC
    ENET2::EIR::GRA = 1;  // Clear status
    ENET2::TCR::GTS = 1;
    while (ENET2::EIR::GRA == 0) {
      // Wait until it's gracefully stopped
    }
    ENET2::EIR::GRA = 1;

    // Disable the Ethernet MAC
    // Note: All interrupts are cleared when Ethernet is reinitialized,
    //       so nothing will be pending
    ENET2::group->ECR = ENET2::ECR::kWOO;

    s_initState = InitStates::kPHYInitialized;
  }

  if (s_initState == InitStates::kPHYInitialized) {
    disable_enet_clocks();

    s_initState = InitStates::kHasHardware;
  }
#endif  // QNETHERNET_INTERNAL_END_STOPS_ALL
}

struct pbuf* proc_input(struct netif* const netif, const int counter) {
  // Finish any pending link status check
  if (s_checkLinkStatusState != 0) {
    s_checkLinkStatusState = check_link_status(netif, s_checkLinkStatusState);
  }

  if (counter == 0) {
    if (std::atomic_flag_test_and_set(&s_rxNotAvail)) {
      return nullptr;
    }
  } else if (counter >= int{kRxSize} * 2) {
    return nullptr;
  }

  // Get the next chunk of input data
  volatile BufferDescriptor* const pBD = rxbd_next();
  if (pBD == nullptr) {
    return nullptr;
  }
  return low_level_input(pBD);
}

void poll(struct netif* const netif) {
  s_checkLinkStatusState = check_link_status(netif, s_checkLinkStatusState);
}

void get_link_info(LinkInfo* const li) {
  *li = s_linkInfo;
}

// Invalid:
// * Speed not 10 or 100
//
// Note that the speed and duplex mode can't be changed if auto-negotiation
// is enabled.
bool set_link(const LinkSettings* const ls) {
  switch (s_initState) {
    case InitStates::kPHYInitialized:
      ATTRIBUTE_FALLTHROUGH;
    case InitStates::kInitialized:
      break;
    default:
      return false;
  }

  if ((ls->speed != 10) && (ls->speed != 100)) {
    return false;
  }

  const uint16_t r = mdio_read(phy_regs::kBCR);
  uint16_t newR = static_cast<uint16_t>(r & ~(phy_vals::kBCR_SPEED_SELECT |
                                              phy_vals::kBCR_AUTO_NEG_ENABLE |
                                              phy_vals::kBCR_DUPLEX_MODE));

  if (ls->speed == 100) {
    newR |= phy_vals::kBCR_SPEED_SELECT;
  }
  if (ls->fullNotHalfDuplex) {
    newR |= phy_vals::kBCR_DUPLEX_MODE;
  }
  if (ls->autoNegotiation) {
    newR |= phy_vals::kBCR_AUTO_NEG_ENABLE;
  }

  if (newR != r) {
    mdio_write(phy_regs::kBCR, newR);
  }
  return true;
}

// Outputs data from the MAC.
err_t output(struct pbuf* const p) {
  // Note: The pbuf already contains the padding (ETH_PAD_SIZE)
  volatile BufferDescriptor* const pBD = get_bufdesc();

  // No need to check for NULL:
  // if (pBD == nullptr) {
  //   LINK_STATS_INC(link.memerr);
  //   LINK_STATS_INC(link.drop);
  //   return ERR_WOULDBLOCK;  // Could also use ERR_MEM, but this lets things like
  //                           // UDP senders know to retry
  // }
  const uint16_t copied = pbuf_copy_partial(p, pBD->buffer, p->tot_len, 0);
  if (copied != p->tot_len) {
    LINK_STATS_INC(link.err);
    LINK_STATS_INC(link.drop);
    return ERR_BUF;
  }
#if !QNETHERNET_BUFFERS_IN_RAM1
  arm_dcache_flush_delete(pBD->buffer, multipleOf32(copied));
#endif  // !QNETHERNET_BUFFERS_IN_RAM1
  update_bufdesc(pBD, copied);
  return ERR_OK;
}

#if QNETHERNET_ENABLE_RAW_FRAME_SUPPORT
bool output_frame(const void* const frame, const size_t len) {
  if (s_initState != InitStates::kInitialized) {
    return false;
  }
  if (len > (kBufSize - size_t{ETH_PAD_SIZE})) {
    return false;
  }

  volatile BufferDescriptor* const pBD = get_bufdesc();

  // No need to check for NULL:
  // if (pBD == nullptr) {
  //   return false;
  // }

  (void)std::memcpy(static_cast<uint8_t*>(pBD->buffer) + ETH_PAD_SIZE, frame,
                    len);
#if !QNETHERNET_BUFFERS_IN_RAM1
  arm_dcache_flush_delete(pBD->buffer, multipleOf32(len + ETH_PAD_SIZE));
#endif  // !QNETHERNET_BUFFERS_IN_RAM1
  update_bufdesc(pBD, static_cast<uint16_t>(len + ETH_PAD_SIZE));

  return true;
}
#endif  // QNETHERNET_ENABLE_RAW_FRAME_SUPPORT

// --------------------------------------------------------------------------
//  MAC Address Filtering
// --------------------------------------------------------------------------

#if !QNETHERNET_ENABLE_PROMISCUOUS_MODE

// CRC-32 routine for computing the 4-byte FCS for multicast lookup. The initial
// value will be zero.
ATTRIBUTE_NODISCARD
static uint32_t crc32(const void* const data, const size_t len) {
  // https://create.stephan-brumme.com/crc32/#fastest-bitwise-crc32

  uint32_t crc = 0;  // Initial value
  const uint8_t* pData = static_cast<const uint8_t*>(data);
  size_t lenRem = len;

  crc = ~crc;
  while ((lenRem--) != 0) {
    crc ^= *(pData++);
    for (int j = 0; j < 8; ++j) {
      crc = (crc >> 1) ^ (-(crc & 0x01) & 0xEDB88320);
    }
  }
  return crc;
}

bool set_incoming_mac_address_allowed(const uint8_t mac[ETH_HWADDR_LEN],
                                      const bool allow) {
  if (mac == nullptr) {
    return false;
  }

  const uint32_t crc = (crc32(mac, ETH_HWADDR_LEN) >> 26) & 0x3f;
  const uint32_t value = uint32_t{1} << (crc & 0x1f);

  // Choose which locations

  const bool isGroup = (mac[0] & 0x01) != 0;
  volatile uint32_t* reg;
  uint32_t* collision;

  if (crc < 0x20) {
    if (isGroup) {
      reg = &ENET2::group->GALR;
      collision = &s_collisionGALR;
    } else {
      reg = &ENET2::group->IALR;
      collision = &s_collisionIALR;
    }
  } else {
    if (isGroup) {  // Group
      reg = &ENET2::group->GAUR;
      collision = &s_collisionGAUR;
    } else {
      reg = &ENET2::group->IAUR;
      collision = &s_collisionIAUR;
    }
  }

  if (allow) {
    if ((*reg & value) != 0) {
      *collision |= value;
    } else {
      *reg |= value;
    }
  } else {
    // Keep collided bits set
    *reg &= ~value | *collision;
    return ((*collision & value) == 0);  // False if can't remove
  }

  return true;
}

#endif  // !QNETHERNET_ENABLE_PROMISCUOUS_MODE

// --------------------------------------------------------------------------
//  Notifications from Upper Layers
// --------------------------------------------------------------------------

void notify_manual_link_state(const bool flag) {
  s_manualLinkState = flag;
}

// --------------------------------------------------------------------------
//  Link Functions
// --------------------------------------------------------------------------

void restart_auto_negotiation() {
  mdio_write(phy_regs::kBCR,
             mdio_read(phy_regs::kBCR) | phy_vals::kBCR_RESTART_AUTO_NEG);
}

void reset_phy() {
  switch (s_initState) {
    case InitStates::kPHYInitialized:
      ATTRIBUTE_FALLTHROUGH;
    case InitStates::kInitialized:
      break;
    default:
      return;
  }

  mdio_write(phy_regs::kBCR,
             mdio_read(phy_regs::kBCR) | phy_vals::kBCR_SOFT_RESET);
  while (true) {
    if ((mdio_read(phy_regs::kBCR) & phy_vals::kBCR_SOFT_RESET) == 0) {
      break;
    }
  }
}

}  // namespace driver
}  // namespace network
}  // namespace qindesign

#endif  // QNETHERNET_INTERNAL_DRIVER_TEENSYMM
