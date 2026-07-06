// SPDX-FileCopyrightText: (c) 2021-2026 Shawn Silverman <shawn@pobox.com>
// SPDX-License-Identifier: AGPL-3.0-or-later

// driver_teensy41.cpp contains the Teensy 4.1 Ethernet interface implementation.
// Based on code from manitou48 and others:
// https://github.com/PaulStoffregen/teensy41_ethernet
// This file is part of the QNEthernet library.

#include "qnethernet/lwip_driver.h"

#if defined(QNETHERNET_INTERNAL_DRIVER_TEENSY41)

// C++ includes
#include <atomic>
#include <cstring>

#include <core_pins.h>
#include <imxrt.h>
#include <util/atomic.h>

#include "lwip/debug.h"
#include "lwip/err.h"
#include "lwip/stats.h"
#include "qnethernet/internal/macro_funcs.h"
#include "qnethernet/platforms/pgmspace.h"

// https://forum.pjrc.com/threads/60532-Teensy-4-1-Beta-Test?p=237096&viewfull=1#post237096
// https://github.com/PaulStoffregen/teensy41_ethernet/blob/master/teensy41_ethernet.ino

// [PHY Datasheet](https://www.pjrc.com/teensy/dp83825i.pdf)
// [i.MX RT1062 Manual](https://www.pjrc.com/teensy/IMXRT1060RM_rev3.pdf)

namespace qindesign {
namespace network {
namespace driver {

// --------------------------------------------------------------------------
//  Defines
// --------------------------------------------------------------------------

static constexpr uint32_t GPIO_PAD_OUTPUT = (0
    /* HYS_0_Hysteresis_Disabled */
    /* PUS_0_100K_Ohm_Pull_Down */
    /* PUE_0_Keeper */
    /* PKE_0_Pull_Keeper_Disabled */
    /* ODE_0_Open_Drain_Disabled */
    | IOMUXC_PAD_SPEED(0)  /* SPEED_0_low_50MHz */
    | IOMUXC_PAD_DSE(7)    /* DSE_7_R0_7 */
    /* SRE_0_Slow_Slew_Rate */);
    // HYS:0 PUS:00 PUE:0 PKE:0 ODE:0 000 SPEED:00 DSE:111 00 SRE:0
    // 0x0038

static constexpr uint32_t GPIO_MUX = 5;
    // SION:0 MUX_MODE:0101
    // ALT5 (GPIO)

// Stronger pull-up for the straps, but even this might not be strong enough.
static constexpr uint32_t STRAP_PAD_PULLUP = (0
    /* HYS_0_Hysteresis_Disabled */
    | IOMUXC_PAD_PUS(3)    /* PUS_3_22K_Ohm_Pull_Up */
    | IOMUXC_PAD_PUE       /* PUE_1_Pull */
    | IOMUXC_PAD_PKE       /* PKE_1_Pull_Keeper_Enabled */
    /* ODE_0_Open_Drain_Disabled */
    | IOMUXC_PAD_SPEED(0)  /* SPEED_0_low_50MHz */
    | IOMUXC_PAD_DSE(7)    /* DSE_7_R0_7 */
    /* SRE_0_Slow_Slew_Rate */
    );
    // HYS:0 PUS:11 PUE:1 PKE:1 ODE:0 000 SPEED:00 DSE:101 00 SRE:0
    // 0xF028

static constexpr uint32_t STRAP_PAD_PULLDOWN = (0
    /* HYS_0_Hysteresis_Disabled */
    | IOMUXC_PAD_PUS(0)    /* PUS_0_100K_Ohm_Pull_Down */
    | IOMUXC_PAD_PUE       /* PUE_1_Pull */
    | IOMUXC_PAD_PKE       /* PKE_1_Pull_Keeper_Enabled */
    /* ODE_0_Open_Drain_Disabled */
    | IOMUXC_PAD_SPEED(0)  /* SPEED_0_low_50MHz */
    | IOMUXC_PAD_DSE(7)    /* DSE_7_R0_7 */
    /* SRE_0_Slow_Slew_Rate */
    );
    // HYS:0 PUS:00 PUE:1 PKE:1 ODE:0 000 SPEED:00 DSE:111 00 SRE:0
    // 0x3038

static constexpr uint32_t MDIO_PAD_PULLUP = (0
    /* HYS_0_Hysteresis_Disabled */
    | IOMUXC_PAD_PUS(3)    /* PUS_3_22K_Ohm_Pull_Up */
    | IOMUXC_PAD_PUE       /* PUE_1_Pull */
    | IOMUXC_PAD_PKE       /* PKE_1_Pull_Keeper_Enabled */
    | IOMUXC_PAD_ODE       /* ODE_1_Open_Drain_Enabled */
    | IOMUXC_PAD_SPEED(0)  /* SPEED_0_low_50MHz */
    | IOMUXC_PAD_DSE(5)    /* DSE_5_R0_5 */
    | IOMUXC_PAD_SRE       /* SRE_1_Fast_Slew_Rate */
    );
    // HYS:0 PUS:11 PUE:1 PKE:1 ODE:1 000 SPEED:00 DSE:101 00 SRE:1
    // 0xF829
    // PHY docs suggest up to 2.2kohms, but this is what we got. It has an
    // internal 10k. It should cover what we need, including 20% error.
    // MDIO requires a 1.5k to 10k pull-up.
    // TODO: Why does the Teensy 4.1 board not have an MDIO pull-up?

static constexpr uint32_t MDIO_MUX = 0;
    // SION:0 MUX_MODE:0000
    // ALT0

// static const uint32_t RMII_PAD_PULLDOWN = (0
//     /* HYS_0_Hysteresis_Disabled */
//     | IOMUXC_PAD_PUS(0)    /* PUS_0_100K_Ohm_Pull_Down */
//     | IOMUXC_PAD_PUE       /* PUE_1_Pull */
//     | IOMUXC_PAD_PKE       /* PKE_1_Pull_Keeper_Enabled */
//     /* ODE_0_Open_Drain_Disabled */
//     | IOMUXC_PAD_SPEED(3)  /* SPEED_3_max_200MHz */
//     | IOMUXC_PAD_DSE(5)    /* DSE_5_R0_5 */
//     | IOMUXC_PAD_SRE       /* SRE_1_Fast_Slew_Rate */
//     );
//     // HYS:0 PUS:00 PUE:1 PKE:1 ODE:0 000 SPEED:11 DSE:101 00 SRE:1
//     // 0x30E9

static constexpr uint32_t RMII_PAD_PULLUP = (0
    /* HYS_0_Hysteresis_Disabled */
    | IOMUXC_PAD_PUS(2)    /* PUS_2_100K_Ohm_Pull_Up */
    | IOMUXC_PAD_PUE       /* PUE_1_Pull */
    | IOMUXC_PAD_PKE       /* PKE_1_Pull_Keeper_Enabled */
    /* ODE_0_Open_Drain_Disabled */
    | IOMUXC_PAD_SPEED(3)  /* SPEED_3_max_200MHz */
    | IOMUXC_PAD_DSE(5)    /* DSE_5_R0_5 */
    | IOMUXC_PAD_SRE       /* SRE_1_Fast_Slew_Rate */
    );
    // HYS:0 PUS:10 PUE:1 PKE:1 ODE:0 000 SPEED:11 DSE:101 00 SRE:1
    // 0xB0E9

// static constexpr uint32_t RMII_PAD_SIGNAL = (0
//     /* HYS_0_Hysteresis_Disabled */
//     /* PUS_0_100K_Ohm_Pull_Down */
//     /* PUE_0_Keeper */
//     /* PKE_0_Pull_Keeper_Disabled */
//     /* ODE_0_Open_Drain_Disabled */
//     | IOMUXC_PAD_SPEED(3)  /* SPEED_3_max_200MHz */
//     | IOMUXC_PAD_DSE(6)    /* DSE_6_R0_6 */
//     | IOMUXC_PAD_SRE       /* SRE_1_Fast_Slew_Rate */
//     );
//     // HYS:0 PUS:00 PUE:0 PKE:0 ODE:0 000 SPEED:11 DSE:101 00 SRE:1
//     // 0x00E9

static constexpr uint32_t RMII_PAD_CLOCK = (0
    /* HYS_0_Hysteresis_Disabled */
    /* PUS_0_100K_Ohm_Pull_Down */
    /* PUE_0_Keeper */
    /* PKE_0_Pull_Keeper_Disabled */
    /* ODE_0_Open_Drain_Disabled */
    | IOMUXC_PAD_SPEED(0)  /* SPEED_0_low_50MHz */
    | IOMUXC_PAD_DSE(6)    /* DSE_6_R0_6 */
    | IOMUXC_PAD_SRE       /* SRE_1_Fast_Slew_Rate */
    );
    // HYS:0 PUS:00 PUE:0 PKE:0 ODE:0 000 SPEED:00 DSE:110 00 SRE:1
    // 0x0031

static constexpr uint32_t RMII_MUX_CLOCK = (6 | 0x10);
      // SION:1 MUX_MODE:0110
      // ALT6

static constexpr uint32_t RMII_MUX = 3;
    // SION:0 MUX_MODE:0011
    // ALT3

// Sizes
static constexpr size_t kRxSize = 5;
static constexpr size_t kTxSize = 5;

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
static std::atomic_flag s_rxNotAvail  = ATOMIC_FLAG_INIT;
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

// Forward declarations
static void enet_isr();

// --------------------------------------------------------------------------
//  PHY I/O
// --------------------------------------------------------------------------

namespace {  // Internal linkage

// PHY register definitions.
namespace phy_regs {
constexpr uint16_t kREGCR   = 0x0D;
constexpr uint16_t kADDAR   = 0x0E;
constexpr uint16_t kLEDCR   = 0x18;
constexpr uint16_t kRCSR    = 0x17;
constexpr uint16_t kBMSR    = 0x01;
constexpr uint16_t kPHYSTS  = 0x10;
constexpr uint16_t kBMCR    = 0x00;
constexpr uint16_t kANAR    = 0x04;
constexpr uint16_t kPHYCR   = 0x19;
constexpr uint16_t kPHYIDR1 = 0x02;
constexpr uint16_t kPHYIDR2 = 0x03;
}  // namespace phy_regs

// PHY register values definitions.
namespace phy_vals {
constexpr uint16_t kLEDCR_BLINK_RATE_20Hz = (0u << 9);
constexpr uint16_t kLEDCR_BLINK_RATE_10Hz = (1u << 9);
constexpr uint16_t kLEDCR_BLINK_RATE_5Hz  = (2u << 9);
constexpr uint16_t kLEDCR_BLINK_RATE_2Hz  = (3u << 9);
constexpr uint16_t kLEDCR_LED_LINK_POLARITY_ACTIVE_HIGH = (1u << 7);

// LEDCR offset 0x18, set LED_Link_Polarity and Blink_rate, pg 62
// LED shows link status, active high, 10Hz
constexpr uint16_t kLEDCR_VALUE =
    (kLEDCR_BLINK_RATE_10Hz | kLEDCR_LED_LINK_POLARITY_ACTIVE_HIGH);
// 10-9: LED Blinking Rate (ON/OFF duration)
// 7: LED Link Polarity: 1=Active High, 0=Active Low

constexpr uint16_t kRCSR_RMII_CLOCK_SELECT_50MHz               = (1u << 7);
constexpr uint16_t kRCSR_RECEIVE_ELASTICITY_BUFFER_SIZE_14_BIT = (0u << 0);
constexpr uint16_t kRCSR_RECEIVE_ELASTICITY_BUFFER_SIZE_2_BIT  = (1u << 0);
constexpr uint16_t kRCSR_RECEIVE_ELASTICITY_BUFFER_SIZE_6_BIT  = (2u << 0);
constexpr uint16_t kRCSR_RECEIVE_ELASTICITY_BUFFER_SIZE_10_BIT = (3u << 0);

constexpr uint16_t kRCSR_VALUE = (kRCSR_RMII_CLOCK_SELECT_50MHz |
                                  kRCSR_RECEIVE_ELASTICITY_BUFFER_SIZE_2_BIT);
// 7: RMII_Clock_Select: 1=50MHz (non-default)
// 1-0: Receive_Elasticity_Buffer_Size: 1=2 bit tolerance (up to 2400 byte packets)

constexpr uint16_t kBMSR_LINK_STATUS = (1u << 2);  // 0: No link, 1: Valid link

constexpr uint16_t kPHYSTS_LINK_STATUS   = (1u <<  0);  // 0: No link, 1: Valid link
constexpr uint16_t kPHYSTS_SPEED_STATUS  = (1u <<  1);  // 0: 100Mbps, 1: 10Mbps
constexpr uint16_t kPHYSTS_DUPLEX_STATUS = (1u <<  2);  // 0: Half-Duplex, 1: Full-Duplex
constexpr uint16_t kPHYSTS_MDI_MDIX_MODE = (1u << 14);  // 0: Normal, 1: Swapped

constexpr uint16_t kBMCR_SPEED_SELECTION  = (1u << 13);  // 0: 10 Mbps, 1: 100 Mbps
constexpr uint16_t kBMCR_AUTO_NEG         = (1u << 12);  // 0: Disable, 1: Enable
constexpr uint16_t kBMCR_RESTART_AUTO_NEG = (1u <<  9);  // 0: Normal, 1: Restart (Self-clearing)
constexpr uint16_t kBMCR_DUPLEX_MODE      = (1u <<  8);  // 0: Half, 1: Full
}  // namespace phy_vals

}  // namespace

// Reads a PHY register (using MDIO & MDC signals) and returns whether
// continuation is needed (not complete). If continuation is needed, then this
// should be called again with 'cont' set to true. If this is the first call,
// then 'cont' should be set to false.
ATTRIBUTE_NODISCARD
static bool mdio_read_nonblocking(const uint16_t regaddr,
                                  uint16_t data[1],
                                  const bool cont) {
  if (!cont) {
    ENET_EIR = ENET_EIR_MII;  // Clear status

    ENET_MMFR = ENET_MMFR_ST(1) | ENET_MMFR_OP(2) | ENET_MMFR_PA(0/*phyaddr*/) |
                ENET_MMFR_RA(regaddr) | ENET_MMFR_TA(2);
  }

  if ((ENET_EIR & ENET_EIR_MII) == 0) {  // Waiting takes on the order of 8.8-8.9us
    return true;
  }

  *data = ENET_MMFR_DATA(ENET_MMFR);
  ENET_EIR = ENET_EIR_MII;
  // printf("mdio read (%04xh): %04xh\r\n", regaddr, data);
  return false;
}

// Blocking MDIO read.
uint16_t mdio_read(const uint16_t regaddr) {
  uint16_t data;
  bool doCont = false;
  do {
    doCont = mdio_read_nonblocking(regaddr, &data, doCont);
  } while (doCont);
  return data;
}

// Writes a PHY register (using MDIO & MDC signals) and returns whether
// continuation is needed (not complete). If continuation is needed, then this
// should be called again with 'cont' set to true. If this is the first call,
// then 'cont' should be set to false.
ATTRIBUTE_NODISCARD
static bool mdio_write_nonblocking(const uint16_t regaddr, const uint16_t data,
                                   const bool cont) {
  if (!cont) {
    ENET_EIR = ENET_EIR_MII;  // Clear status

    ENET_MMFR = ENET_MMFR_ST(1) | ENET_MMFR_OP(1) | ENET_MMFR_PA(0/*phyaddr*/) |
                ENET_MMFR_RA(regaddr) | ENET_MMFR_TA(2) |
                ENET_MMFR_DATA(data);
  }

  if ((ENET_EIR & ENET_EIR_MII) == 0) {  // Waiting takes on the order of 8.8-8.9us
    return true;
  }

  ENET_EIR = ENET_EIR_MII;
  // printhex("mdio write (%04xh): %04xh\r\n", regaddr, data);
  return false;
}

// Blocking MDIO write.
void mdio_write(const uint16_t regaddr, const uint16_t data) {
  bool doCont = false;
  do {
    doCont = mdio_write_nonblocking(regaddr, data, doCont);
  } while (doCont);
}

// --------------------------------------------------------------------------
//  Low-Level
// --------------------------------------------------------------------------

// Enables the Ethernet-related clocks. See also disable_enet_clocks().
FLASHMEM static void enable_enet_clocks() {
  // Enable the Ethernet clock
  CCM_CCGR1 |= CCM_CCGR1_ENET(CCM_CCGR_ON);

  // Configure PLL6 for 50 MHz (page 1112)
  CCM_ANALOG_PLL_ENET_SET = CCM_ANALOG_PLL_ENET_BYPASS;
  CCM_ANALOG_PLL_ENET_CLR = 0
                            | CCM_ANALOG_PLL_ENET_BYPASS_CLK_SRC(3)
                            | CCM_ANALOG_PLL_ENET_ENET2_DIV_SELECT(3)
                            | CCM_ANALOG_PLL_ENET_DIV_SELECT(3)
                            ;
  CCM_ANALOG_PLL_ENET_SET = 0
                            | CCM_ANALOG_PLL_ENET_ENET_25M_REF_EN
                            // | CCM_ANALOG_PLL_ENET_ENET2_REF_EN
                            | CCM_ANALOG_PLL_ENET_ENABLE
                            // | CCM_ANALOG_PLL_ENET_ENET2_DIV_SELECT(1)
                            | CCM_ANALOG_PLL_ENET_DIV_SELECT(1)
                            ;
  CCM_ANALOG_PLL_ENET_CLR = CCM_ANALOG_PLL_ENET_POWERDOWN;
  while ((CCM_ANALOG_PLL_ENET & CCM_ANALOG_PLL_ENET_LOCK) == 0) {
    // Wait for PLL lock
  }
  CCM_ANALOG_PLL_ENET_CLR = CCM_ANALOG_PLL_ENET_BYPASS;
  // printf("PLL6 = %08" PRIX32 "h (should be 80202001h)\n", CCM_ANALOG_PLL_ENET);

  // Configure REFCLK to be driven as output by PLL6 (page 325)
  clearAndSet32(
      &IOMUXC_GPR_GPR1,
      IOMUXC_GPR_GPR1_ENET1_CLK_SEL,
      IOMUXC_GPR_GPR1_ENET_IPG_CLK_S_EN | IOMUXC_GPR_GPR1_ENET1_TX_CLK_DIR);
}

// Disables everything enabled with enable_enet_clocks().
FLASHMEM static void disable_enet_clocks() {
  // Configure REFCLK
  clearAndSet32(&IOMUXC_GPR_GPR1, IOMUXC_GPR_GPR1_ENET1_TX_CLK_DIR, 0);

  // Stop the PLL (first bypassing)
  CCM_ANALOG_PLL_ENET_SET = CCM_ANALOG_PLL_ENET_BYPASS;
  CCM_ANALOG_PLL_ENET = 0
                        | CCM_ANALOG_PLL_ENET_BYPASS         // Reset to default
                        | CCM_ANALOG_PLL_ENET_POWERDOWN
                        | CCM_ANALOG_PLL_ENET_DIV_SELECT(1)
                        ;

  // Disable the clock for ENET
  CCM_CCGR1 &= ~CCM_CCGR1_ENET(CCM_CCGR_ON);
}

// Configures all the pins necessary for communicating with the PHY.
FLASHMEM static void configure_phy_pins() {
  // Configure strap pins
  // Note: The pull-up may not be strong enough
  // Note: All the strap pins have an internal pull-down of 9kohm +/-25%
  // Table 8. PHY Address Strap Table (page 39)
  IOMUXC_SW_PAD_CTL_PAD_GPIO_B1_04 = STRAP_PAD_PULLDOWN;  // PhyAdd[0] = 0 (RX_D0, pin 18) (page 723)
  IOMUXC_SW_PAD_CTL_PAD_GPIO_B1_06 = STRAP_PAD_PULLDOWN;  // PhyAdd[1] = 0 (CRS_DV, pin 20) (page 726)
  // Table 9. RMII MAC Mode Strap Table (page 39)
  IOMUXC_SW_PAD_CTL_PAD_GPIO_B1_05 = STRAP_PAD_PULLUP;    // UP; Master/Slave = RMII Slave Mode (RX_D1, pin 17) (page 724)
  // Not connected: 50MHzOut/LED2 (pin 2, pull-down): RX_DV_En: Pin 20 is configured as CRS_DV
  // Table 10. Auto_Neg Strap Table (page 39)
  IOMUXC_SW_PAD_CTL_PAD_GPIO_B1_11 = STRAP_PAD_PULLDOWN;  // Auto MDIX Enable (RX_ER, pin 22) (page 734)
  // Not connected to a processor pin: LED0 (pin 4, pull-down): ANeg_Dis: Auto Negotiation Enable

  // Configure PHY-connected Reset and Power pins as outputs
  // PHY spec. page 3
  // Note: Teensyduino already configures GPIO2 as its fast counterpart, GPIO7
  IOMUXC_SW_PAD_CTL_PAD_GPIO_B0_15 = GPIO_PAD_OUTPUT;  // INTR/PWRDN, pin 3 (page 714)
  IOMUXC_SW_PAD_CTL_PAD_GPIO_B0_14 = GPIO_PAD_OUTPUT;  // RST_N, pin 5 (page 713)

  IOMUXC_SW_MUX_CTL_PAD_GPIO_B0_15 = GPIO_MUX;  // Power (INT, pin 3) (GPIO2_IO15, page 519)
  IOMUXC_SW_MUX_CTL_PAD_GPIO_B0_14 = GPIO_MUX;  // Reset (RST, pin 5) (GPIO2_IO14, page 518)

  GPIO7_GDIR |= (1u << 15) | (1u << 14);
  GPIO7_DR_CLEAR = (1u << 15);  // Power down
  GPIO7_DR_SET   = (1u << 14);  // Start with reset de-asserted so that it can be
                                // asserted for a specific duration

  // Configure the MDIO and MDC pins
  IOMUXC_SW_PAD_CTL_PAD_GPIO_B1_15 = MDIO_PAD_PULLUP;  // MDIO
  IOMUXC_SW_PAD_CTL_PAD_GPIO_B1_14 = RMII_PAD_PULLUP;  // MDC

  IOMUXC_SW_MUX_CTL_PAD_GPIO_B1_15 = MDIO_MUX;  // MDIO pin 15 (ENET_MDIO of enet, page 535)
  IOMUXC_SW_MUX_CTL_PAD_GPIO_B1_14 = MDIO_MUX;  // MDC pin 16 (ENET_MDC of enet, page 534)

  IOMUXC_ENET_MDIO_SELECT_INPUT = 2;  // GPIO_B1_15_ALT0 (page 791)
      // DAISY:10
}

// Configures all the RMII pins. This should be called after initializing
// the PHY.
FLASHMEM static void configure_rmii_pins() {
  // The NXP SDK and original Teensy 4.1 example code use pull-ups
  IOMUXC_SW_PAD_CTL_PAD_GPIO_B1_04 = RMII_PAD_PULLUP;  // Reset this (RXD0)
  IOMUXC_SW_PAD_CTL_PAD_GPIO_B1_05 = RMII_PAD_PULLUP;  // Reset this (RXD1)
  IOMUXC_SW_PAD_CTL_PAD_GPIO_B1_06 = RMII_PAD_PULLUP;  // Reset this (RXEN)
  IOMUXC_SW_PAD_CTL_PAD_GPIO_B1_11 = RMII_PAD_PULLUP;  // Reset this (RXER)
  IOMUXC_SW_PAD_CTL_PAD_GPIO_B1_07 = RMII_PAD_PULLUP;  // TXD0 (PHY has internal pull-down)
  IOMUXC_SW_PAD_CTL_PAD_GPIO_B1_08 = RMII_PAD_PULLUP;  // TXD1 (PHY has internal pull-down)
  IOMUXC_SW_PAD_CTL_PAD_GPIO_B1_09 = RMII_PAD_PULLUP;  // TXEN (PHY has internal pull-down)

  IOMUXC_SW_MUX_CTL_PAD_GPIO_B1_04 = RMII_MUX;  // RXD0 pin 18 (ENET_RX_DATA00 of enet, page 524)
  IOMUXC_SW_MUX_CTL_PAD_GPIO_B1_05 = RMII_MUX;  // RXD1 pin 17 (ENET_RX_DATA01 of enet, page 525)
  IOMUXC_SW_MUX_CTL_PAD_GPIO_B1_11 = RMII_MUX;  // RXER pin 22 (ENET_RX_ER of enet, page 531)
  IOMUXC_SW_MUX_CTL_PAD_GPIO_B1_06 = RMII_MUX;  // RXEN pin 20 (ENET_RX_EN of enet, page 526)
  IOMUXC_SW_MUX_CTL_PAD_GPIO_B1_09 = RMII_MUX;  // TXEN pin  1 (ENET_TX_EN of enet, page 529)
  IOMUXC_SW_MUX_CTL_PAD_GPIO_B1_07 = RMII_MUX;  // TXD0 pin 23 (ENET_TX_DATA00 of enet, page 527)
  IOMUXC_SW_MUX_CTL_PAD_GPIO_B1_08 = RMII_MUX;  // TXD1 pin 24 (ENET_TX_DATA01 of enet, page 528)

  IOMUXC_ENET_IPG_CLK_RMII_SELECT_INPUT = 1;  // GPIO_B1_10_ALT6 (page 791)
      // DAISY:1

  IOMUXC_ENET0_RXDATA_SELECT_INPUT = 1;  // GPIO_B1_04_ALT3 (page 792)
  IOMUXC_ENET1_RXDATA_SELECT_INPUT = 1;  // GPIO_B1_05_ALT3 (page 793)
  IOMUXC_ENET_RXEN_SELECT_INPUT    = 1;  // GPIO_B1_06_ALT3 (page 794)
  IOMUXC_ENET_RXERR_SELECT_INPUT   = 1;  // GPIO_B1_11_ALT3 (page 795)
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

  // Note: Ensure the clock is present at the PHY (XI) at power up
  IOMUXC_SW_PAD_CTL_PAD_GPIO_B1_10 = RMII_PAD_CLOCK;
  IOMUXC_SW_MUX_CTL_PAD_GPIO_B1_10 = RMII_MUX_CLOCK;  // REFCLK (XI) pin 13 (ENET_REF_CLK of enet, page 530)
  ENET_MSCR = ENET_MSCR_MII_SPEED(9);  // Internal module clock frequency = 50MHz

  GPIO7_DR_SET   = (1u << 15);  // Power on
  delay(50);                    // Just in case; unsure if needed
  GPIO7_DR_CLEAR = (1u << 14);  // Reset
  delayMicroseconds(25) ;       // T1: RESET PULSE Width: Miminum Reset pulse width to be able to reset (w/o debouncing caps)
  GPIO7_DR_SET   = (1u << 14);  // Take out of reset
  delay(2);                     // T2: Reset to SMI ready: Post reset stabilization time prior to MDC preamble for register access

  // PHYIDR1: OUI bits 21-6: 0x2000
  // PHYIDR2: OUI bits 5-0:  0x28: 101000b
  //         Model Number:   0x14: 010100b
  //         Revision Number: 4 bits
  // Check for PHY presence
  if ((mdio_read(phy_regs::kPHYIDR1) != 0x2000) ||
      ((mdio_read(phy_regs::kPHYIDR2) & 0xfff0) != 0xA140)) {
    // Undo some pin configuration, for posterity
    GPIO7_GDIR &= ~((1u << 15) | (1u << 14));

    disable_enet_clocks();

    s_initState = InitStates::kNoHardware;
    return;
  }

  // LEDCR offset 0x18, set LED_Link_Polarity and Blink_rate, pg 62
  // LED shows link status, active high, 10Hz
  mdio_write(phy_regs::kLEDCR, phy_vals::kLEDCR_VALUE);

  // Configure the PHY registers
  // The strap pull-ups may not have been strong enough, so ensure those values
  // are set properly too
  // Right now, it's just the 50MHz clock select for RMII slave mode

  // mdio_write(phy_regs::kBMCR, 0x3100);  // 13: Speed_Selection: 1=100Mbps
  //                                       // 12: Negotiation_Enable: 1=enabled
  //                                       //  8: Duplex_Mode: 1=Full-Duplex
  // mdio_write(phy_regs::kANAR, 0x01E1);  // 8: 100Base-TX_Full-Duplex: 1=advertise
  //                                       // 7: 100Base-TX_Half-Duplex: 1=advertise
  //                                       // 6: 10Base-T_Full-Duplex: 1=advertise
  //                                       // 5: 10Base-T_Half-Duplex: 1=advertise
  //                                       // 4-0: Selector_Field: IEEE802.3u
  // printf("RCSR = %04" PRIx16 "h\r\n", mdio_read(phy_regs::kRCSR));
  mdio_write(phy_regs::kRCSR, phy_vals::kRCSR_VALUE);
  // printf("RCSR = %04" PRIx16 "h\r\n", mdio_read(phy_regs::kRCSR));
  // mdio_write(phy_regs::kPHYCR, 0x8000);  // 15: Auto_MDI/X_Enable: 1=enable

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

  struct pbuf* p = NULL;

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
    if (p != NULL) {
#if !QNETHERNET_BUFFERS_IN_RAM1
      arm_dcache_delete(pBD->buffer, multipleOf32(p->tot_len));
#endif  // !QNETHERNET_BUFFERS_IN_RAM1
      LWIP_ASSERT("Expected space for pbuf fill",
                  pbuf_take(p, pBD->buffer, p->tot_len) == ERR_OK);
    } else {
      LINK_STATS_INC(link.drop);
      LINK_STATS_INC(link.memerr);
    }
  }

  // Set rx bd empty
  pBD->status = (pBD->status & rx_bd_status::kWrap) | rx_bd_status::kEmpty;

  ENET_RDAR = ENET_RDAR_RDAR;

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
                  tx_bd_control::kTxCrc                |
                  tx_bd_control::kLast                 |
                  tx_bd_control::kReady;

  ENET_TDAR = ENET_TDAR_TDAR;

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
      return NULL;
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
  if ((ENET_EIR & ENET_EIR_RXF) != 0) {
    ENET_EIR = ENET_EIR_RXF;
    std::atomic_flag_clear(&s_rxNotAvail);
  }
}

// Checks the link status and returns zero if complete and a state value if
// not complete. The return value should be used in the next call to
// this function.
ATTRIBUTE_NODISCARD
static inline int check_link_status(struct netif* const netif,
                                    const int state) {
  static uint16_t bmsr;
  static uint16_t physts;
  static uint8_t is_link_up;

  if (s_initState != InitStates::kInitialized) {
    return 0;
  }

  // Note: phy_regs::kPHYSTS doesn't seem to contain the live link information
  //       unless BMSR is read too

  switch (state) {
    case 0:
      ATTRIBUTE_FALLTHROUGH;
    case 1:
      if (mdio_read_nonblocking(phy_regs::kBMSR, &bmsr, state == 1)) {
        return 1;
      }
      is_link_up = ((bmsr & phy_vals::kBMSR_LINK_STATUS) != 0);
      if (!is_link_up) {
        break;
      }
      ATTRIBUTE_FALLTHROUGH;

    case 2:
      if (mdio_read_nonblocking(phy_regs::kPHYSTS, &physts, state == 2)) {
        return 2;
      }
      break;

    default:
      break;
  }

  if (netif_is_link_up(netif) != is_link_up) {
    if (is_link_up) {
      s_linkInfo.speed =
          ((physts & phy_vals::kPHYSTS_SPEED_STATUS) != 0) ? 10 : 100;
      s_linkInfo.fullNotHalfDuplex =
          ((physts & phy_vals::kPHYSTS_DUPLEX_STATUS) != 0);
      s_linkInfo.isCrossover =
          ((physts & phy_vals::kPHYSTS_MDI_MDIX_MODE) != 0);

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
  dc->isLinkCrossoverDetectable    = true;
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
  if ((CCM_CCGR1 & CCM_CCGR1_ENET(CCM_CCGR_ON)) == 0) {
    return false;
  }

  const uint32_t rl = ENET_PALR;
  const uint32_t ru = ENET_PAUR;
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
  if ((CCM_CCGR1 & CCM_CCGR1_ENET(CCM_CCGR_ON)) == 0) {
    return false;
  }

  // TODO: Not sure if disabling interrupts is really needed
  ATOMIC_BLOCK(ATOMIC_RESTORESTATE) {
    ENET_PALR = (uint32_t{mac[0]} << 24) | (uint32_t{mac[1]} << 16) |
                (uint32_t{mac[2]} <<  8) | (uint32_t{mac[3]} <<  0);
    ENET_PAUR = (uint32_t{mac[4]} << 24) | (uint32_t{mac[5]} << 16) | 0x8808u;
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

  ENET_EIMR = 0;  // This also deasserts all interrupts

  ENET_RCR = 0
             | ENET_RCR_NLC        // Payload length is checked
             | ENET_RCR_MAX_FL((MAX_FRAME_LEN) + 4)
             | ENET_RCR_CFEN       // Discard non-pause MAC control frames
             | ENET_RCR_CRCFWD     // CRC is stripped (ignored if PADEN)
             | ENET_RCR_PADEN      // Padding is removed
             | ENET_RCR_RMII_MODE
             | ENET_RCR_FCE        // Flow control enable
#if QNETHERNET_ENABLE_PROMISCUOUS_MODE
             | ENET_RCR_PROM       // Promiscuous mode
#endif  // QNETHERNET_ENABLE_PROMISCUOUS_MODE
             | ENET_RCR_MII_MODE;
  ENET_TCR = 0
#if !QNETHERNET_ENABLE_RAW_FRAME_SUPPORT
             | ENET_TCR_ADDINS     // Overwrite with programmed MAC address
#endif  // !QNETHERNET_ENABLE_RAW_FRAME_SUPPORT
             | ENET_TCR_ADDSEL(0)
             // | ENET_TCR_RFC_PAUSE
             // | ENET_TCR_TFC_PAUSE
             | ENET_TCR_FDEN         // Enable full-duplex
             ;

  ENET_TACC = 0
#if (CHECKSUM_GEN_UDP == 0) || \
    (CHECKSUM_GEN_TCP == 0) || \
    (CHECKSUM_GEN_ICMP == 0)
      | ENET_TACC_PROCHK  // Insert protocol checksum
#endif  // not(Generate all checksums)
#if CHECKSUM_GEN_IP == 0
      | ENET_TACC_IPCHK   // Insert IP header checksum
#endif  // CHECKSUM_GEN_IP == 0
#if ETH_PAD_SIZE == 2
      | ENET_TACC_SHIFT16
#endif  // ETH_PAD_SIZE == 2
      ;

  ENET_RACC = 0
#if ETH_PAD_SIZE == 2
      | ENET_RACC_SHIFT16
#endif  // ETH_PAD_SIZE == 2
      | ENET_RACC_LINEDIS  // Discard bad frames
#if (CHECKSUM_CHECK_UDP == 0) && \
    (CHECKSUM_CHECK_TCP == 0) && \
    (CHECKSUM_CHECK_ICMP == 0)
      | ENET_RACC_PRODIS   // Discard frames with incorrect protocol checksum
                           // Requires RSFL == 0
#endif  // not(Check any checksums)
#if CHECKSUM_CHECK_IP == 0
      | ENET_RACC_IPDIS    // Discard frames with incorrect IPv4 header checksum
                           // Requires RSFL == 0
#endif  // CHECKSUM_CHECK_IP == 0
      | ENET_RACC_PADREM
      ;

  ENET_TFWR = ENET_TFWR_STRFWD;
  ENET_RSFL = 0;

  ENET_RDSR = reinterpret_cast<uint32_t>(s_rxRing);
  ENET_TDSR = reinterpret_cast<uint32_t>(s_txRing);
  ENET_MRBR = kBufSize;

  ENET_RXIC = 0;
  ENET_TXIC = 0;
  // ENET_PALR = (uint32_t{mac[0]} << 24) | (uint32_t{mac[1]} << 16) |
  //             (uint32_t{mac[2]} <<  8) | (uint32_t{mac[3]} <<  0);
  // ENET_PAUR = (uint32_t{mac[4]} << 24) | (uint32_t{mac[5]} << 16) | 0x8808u;

  ENET_OPD = 0x10014;
  ENET_RSEM = 0;
  ENET_MIBC = 0;

  ENET_IAUR = 0;
  ENET_IALR = 0;
  ENET_GAUR = 0;
  ENET_GALR = 0;

  ENET_EIMR = ENET_EIMR_RXF;
  attachInterruptVector(IRQ_ENET, &enet_isr);
  NVIC_ENABLE_IRQ(IRQ_ENET);

  // Last few things to do
  ENET_EIR = 0x7fff8000;  // Clear any pending interrupts before setting ETHEREN
  (void)std::atomic_flag_test_and_set(&s_rxNotAvail);

  // Last, enable the Ethernet MAC
  ENET_ECR = 0x70000000 | ENET_ECR_DBSWP | ENET_ECR_EN1588 | ENET_ECR_ETHEREN;

  // Indicate there are empty RX buffers and available ready TX buffers
  ENET_RDAR = ENET_RDAR_RDAR;
  ENET_TDAR = ENET_TDAR_TDAR;

  // PHY soft reset
  // mdio_write(phy_regs::kBMCR, 1u << 15);

  s_initState = InitStates::kInitialized;

  return true;
}

void unused_interrupt_vector();  // startup.c

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
    NVIC_DISABLE_IRQ(IRQ_ENET);
    attachInterruptVector(IRQ_ENET, &unused_interrupt_vector);
    ENET_EIMR = 0;  // Disable interrupts

    // Gracefully stop any transmission before disabling the Ethernet MAC
    ENET_EIR = ENET_EIR_GRA;  // Clear status
    ENET_TCR |= ENET_TCR_GTS;
    while ((ENET_EIR & ENET_EIR_GRA) == 0) {
      // Wait until it's gracefully stopped
    }
    ENET_EIR = ENET_EIR_GRA;

    // Disable the Ethernet MAC
    // Note: All interrupts are cleared when Ethernet is reinitialized,
    //       so nothing will be pending
    ENET_ECR = 0x70000000;

    s_initState = InitStates::kPHYInitialized;
  }

  if (s_initState == InitStates::kPHYInitialized) {
    // Power down the PHY and enable reset
    GPIO7_DR_CLEAR = (1u << 15) | (1u << 14);

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
      return NULL;
    }
  } else if (counter >= int{kRxSize} * 2) {
    return NULL;
  }

  // Get the next chunk of input data
  volatile BufferDescriptor* const pBD = rxbd_next();
  if (pBD == NULL) {
    return NULL;
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

  const uint16_t r = mdio_read(phy_regs::kBMCR);
  uint16_t newR = static_cast<uint16_t>(r & ~(phy_vals::kBMCR_SPEED_SELECTION |
                                              phy_vals::kBMCR_AUTO_NEG |
                                              phy_vals::kBMCR_DUPLEX_MODE));

  if (ls->speed == 100) {
    newR |= phy_vals::kBMCR_SPEED_SELECTION;
  }
  if (ls->fullNotHalfDuplex) {
    newR |= phy_vals::kBMCR_DUPLEX_MODE;
  }
  if (ls->autoNegotiation) {
    newR |= phy_vals::kBMCR_AUTO_NEG;
  }

  if (newR != r) {
    mdio_write(phy_regs::kBMCR, newR);
  }
  return true;
}

// Outputs data from the MAC.
err_t output(struct pbuf* const p) {
  // Note: The pbuf already contains the padding (ETH_PAD_SIZE)
  volatile BufferDescriptor* const pBD = get_bufdesc();

  // No need to check for NULL:
  // if (pBD == NULL) {
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
  // if (pBD == NULL) {
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
  if (mac == NULL) {
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
      reg = &ENET_GALR;
      collision = &s_collisionGALR;
    } else {
      reg = &ENET_IALR;
      collision = &s_collisionIALR;
    }
  } else {
    if (isGroup) {  // Group
      reg = &ENET_GAUR;
      collision = &s_collisionGAUR;
    } else {
      reg = &ENET_IAUR;
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
  mdio_write(phy_regs::kBMCR,
             mdio_read(phy_regs::kBMCR) | phy_vals::kBMCR_RESTART_AUTO_NEG);
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

  GPIO7_DR_CLEAR = (1u << 14);  // Reset
  delayMicroseconds(25);        // T1: RESET PULSE Width: Miminum Reset pulse width to be able to reset (w/o debouncing caps)
  GPIO7_DR_SET   = (1u << 14);  // Take out of reset
  delay(2);                     // T2: Reset to SMI ready: Post reset stabilization time prior to MDC preamble for register access

  mdio_write(phy_regs::kLEDCR, phy_vals::kLEDCR_VALUE);
  mdio_write(phy_regs::kRCSR, phy_vals::kRCSR_VALUE);
}

}  // namespace driver
}  // namespace network
}  // namespace qindesign

#endif  // QNETHERNET_INTERNAL_DRIVER_TEENSY41
