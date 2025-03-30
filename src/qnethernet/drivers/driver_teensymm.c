// SPDX-FileCopyrightText: (c) 2021-2025 Shawn Silverman <shawn@pobox.com>
// SPDX-License-Identifier: AGPL-3.0-or-later

// driver_teensymm.c contains the Teensy MicroMod Ethernet
// interface implementation.
// Based on the Teensy 4.1 driver and here:
// https://forum.pjrc.com/index.php?threads/add-ethernet-to-a-micromod-format-teensy.67643/post-356447
// This file is part of the QNEthernet library.

#include "qnethernet/lwip_driver.h"

#if defined(QNETHERNET_INTERNAL_DRIVER_TEENSYMM)

// C includes
#include <stdalign.h>
#include <stdatomic.h>
#include <string.h>

#include <avr/pgmspace.h>
#include <core_pins.h>
#include <imxrt.h>

#include "lwip/arch.h"
#include "lwip/err.h"
#include "lwip/stats.h"

// https://forum.pjrc.com/threads/60532-Teensy-4-1-Beta-Test?p=237096&viewfull=1#post237096
// https://github.com/PaulStoffregen/teensy41_ethernet/blob/master/teensy41_ethernet.ino

// [PHY Datasheet](https://www.pjrc.com/teensy/dp83825i.pdf)
// [i.MX RT1062 Manual](https://www.pjrc.com/teensy/IMXRT1060RM_rev3.pdf)

// --------------------------------------------------------------------------
//  Defines
// --------------------------------------------------------------------------

#define CLRSET(reg, clear, set) ((reg) = ((reg) & ~(clear)) | (set))

// Stronger pull-up for the straps, but even this might not be strong enough.
#define STRAP_PAD_PULLUP (0                                \
    /* HYS_0_Hysteresis_Disabled */                        \
    | IOMUXC_PAD_PUS(3)    /* PUS_3_22K_Ohm_Pull_Up */     \
    | IOMUXC_PAD_PUE       /* PUE_1_Pull */                \
    | IOMUXC_PAD_PKE       /* PKE_1_Pull_Keeper_Enabled */ \
    /* ODE_0_Open_Drain_Disabled */                        \
    | IOMUXC_PAD_SPEED(0)  /* SPEED_0_low_50MHz */         \
    | IOMUXC_PAD_DSE(5)    /* DSE_5_R0_5 */                \
    /* SRE_0_Slow_Slew_Rate */                             \
    )
    // HYS:0 PUS:11 PUE:1 PKE:1 ODE:0 000 SPEED:00 DSE:101 00 SRE:0
    // 0xF028

#define MDIO_PAD_PULLUP (0                                 \
    /* HYS_0_Hysteresis_Disabled */                        \
    | IOMUXC_PAD_PUS(3)    /* PUS_3_22K_Ohm_Pull_Up */     \
    | IOMUXC_PAD_PUE       /* PUE_1_Pull */                \
    | IOMUXC_PAD_PKE       /* PKE_1_Pull_Keeper_Enabled */ \
    | IOMUXC_PAD_ODE       /* ODE_1_Open_Drain_Enabled */  \
    | IOMUXC_PAD_SPEED(0)  /* SPEED_0_low_50MHz */         \
    | IOMUXC_PAD_DSE(5)    /* DSE_5_R0_5 */                \
    | IOMUXC_PAD_SRE       /* SRE_1_Fast_Slew_Rate */      \
    )
    // HYS:0 PUS:11 PUE:1 PKE:1 ODE:1 000 SPEED:00 DSE:101 00 SRE:1
    // 0xF829
    // Reference schematic suggests 1.5kohms, but this is what we got. It has
    // some internal resistor. Hopefully these cover what we need.

#define MDIO_MUX 8
    // SION:0 MUX_MODE:1000
    // ALT8

#define RMII_PAD_PULLDOWN (0                               \
    /* HYS_0_Hysteresis_Disabled */                        \
    | IOMUXC_PAD_PUS(0)    /* PUS_0_100K_Ohm_Pull_Down */  \
    | IOMUXC_PAD_PUE       /* PUE_1_Pull */                \
    | IOMUXC_PAD_PKE       /* PKE_1_Pull_Keeper_Enabled */ \
    /* ODE_0_Open_Drain_Disabled */                        \
    | IOMUXC_PAD_SPEED(3)  /* SPEED_3_max_200MHz */        \
    | IOMUXC_PAD_DSE(5)    /* DSE_5_R0_5 */                \
    | IOMUXC_PAD_SRE       /* SRE_1_Fast_Slew_Rate */      \
    )
    // HYS:0 PUS:00 PUE:1 PKE:1 ODE:0 000 SPEED:11 DSE:101 00 SRE:1
    // 0x30E9

#define RMII_PAD_PULLUP (0                                 \
    /* HYS_0_Hysteresis_Disabled */                        \
    | IOMUXC_PAD_PUS(2)    /* PUS_2_100K_Ohm_Pull_Up */    \
    | IOMUXC_PAD_PUE       /* PUE_1_Pull */                \
    | IOMUXC_PAD_PKE       /* PKE_1_Pull_Keeper_Enabled */ \
    /* ODE_0_Open_Drain_Disabled */                        \
    | IOMUXC_PAD_SPEED(3)  /* SPEED_3_max_200MHz */        \
    | IOMUXC_PAD_DSE(5)    /* DSE_5_R0_5 */                \
    | IOMUXC_PAD_SRE       /* SRE_1_Fast_Slew_Rate */      \
    )
    // HYS:0 PUS:10 PUE:1 PKE:1 ODE:0 000 SPEED:11 DSE:101 00 SRE:1
    // 0xB0E9

#define RMII_PAD_CLOCK (0                             \
    /* HYS_0_Hysteresis_Disabled */                   \
    /* PUS_0_100K_Ohm_Pull_Down */                    \
    /* PUE_0_Keeper */                                \
    /* PKE_0_Pull_Keeper_Disabled */                  \
    /* ODE_0_Open_Drain_Disabled */                   \
    | IOMUXC_PAD_SPEED(0)  /* SPEED_0_low_50MHz */    \
    | IOMUXC_PAD_DSE(6)    /* DSE_6_R0_6 */           \
    | IOMUXC_PAD_SRE       /* SRE_1_Fast_Slew_Rate */ \
    )
    // HYS:0 PUS:00 PUE:0 PKE:0 ODE:0 000 SPEED:00 DSE:110 00 SRE:1
    // 0x0031

#define RMII_MUX_CLOCK (9 | 0x10)
      // SION:1 MUX_MODE:1001
      // ALT9

#define RMII_MUX 8
    // SION:0 MUX_MODE:1000
    // ALT8

#define RMII_PAD_NOPULL (0                                \
    /* HYS_0_Hysteresis_Disabled */                       \
    | IOMUXC_PAD_PUS(0)    /* PUS_0_100K_Ohm_Pull_Down */ \
    /* PUE_0_Keeper  */                                   \
    /* PKE_0_Pull_Keeper_Disabled  */                     \
    /* ODE_0_Open_Drain_Disabled */                       \
    | IOMUXC_PAD_SPEED(2)  /* SPEED_2_fast_150MHz */      \
    | IOMUXC_PAD_DSE(6)    /* DSE_6_R0_6 */               \
    /* SRE_0_Slow_Slew_Rate */                            \
    )
    // HYS:0 PUS:00 PUE:0 PKE:0 ODE:0 000 SPEED:10 DSE:110 00 SRE:0
    // 0xB0E9

#define RX_SIZE 10
#define TX_SIZE 10
#define IRQ_PRIORITY 64

// Size checks
#if (RX_SIZE < 1) || (TX_SIZE < 1)
#error "Rx and Tx sizes mist be >= 1"
#endif  // (RX_SIZE < 1) || (TX_SIZE < 1)

// Buffer size for transferring to and from the Ethernet MAC. The frame size is
// either 1518 or 1522, assuming a 1500-byte payload, depending on whether VLAN
// support is desired. VLAN support requires an extra 4 bytes. The ARM cache
// management functions require 32-bit alignment, but the ENETx_MRBR max.
// receive buffer size register says that the RX buffer size must be a multiple
// of 64 and >= 256.
//
// [1518 or 1522 made into a multiple of 32 for ARM cache flush sizing and a
// multiple of 64 for ENETx_MRBR.]
// NOTE: BUF_SIZE will be 1536 whether we use 1518 or 1522 (plus ETH_PAD_SIZE)
// * Padding(2)
// * Destination(6) + Source(6) + VLAN tag(2) + VLAN info(2) + Length/Type(2) +
//   Payload(1500) + FCS(4)
#define BUF_SIZE (((ETH_PAD_SIZE + 6 + 6 + 2 + 2 + 2 + 1500 + 4) + 63) & ~63)

#if !QNETHERNET_BUFFERS_IN_RAM1
#define MULTIPLE_OF_32(x) (((x) + 31) & ~31)
#define BUFFER_DMAMEM DMAMEM
#else
#define BUFFER_DMAMEM
#endif  // !QNETHERNET_BUFFERS_IN_RAM1

// --------------------------------------------------------------------------
//  Types
// --------------------------------------------------------------------------

// Defines the control and status region of the receive buffer descriptor.
typedef enum _enet_rx_bd_control_status {
  kEnetRxBdEmpty           = 0x8000U,  // Empty bit
  kEnetRxBdRxSoftOwner1    = 0x4000U,  // Receive software ownership
  kEnetRxBdWrap            = 0x2000U,  // Wrap buffer descriptor
  kEnetRxBdRxSoftOwner2    = 0x1000U,  // Receive software ownership
  kEnetRxBdLast            = 0x0800U,  // Last BD in the frame (L bit)
  kEnetRxBdMiss            = 0x0100U,  // Miss; in promiscuous mode; needs L
  kEnetRxBdBroadcast       = 0x0080U,  // Broadcast
  kEnetRxBdMulticast       = 0x0040U,  // Multicast
  kEnetRxBdLengthViolation = 0x0020U,  // Receive length violation; needs L
  kEnetRxBdNonOctet        = 0x0010U,  // Receive non-octet aligned frame; needs L
  kEnetRxBdCrc             = 0x0004U,  // Receive CRC or frame error; needs L
  kEnetRxBdOverrun         = 0x0002U,  // Receive FIFO overrun; needs L
  kEnetRxBdTrunc           = 0x0001U   // Frame is truncated
} enet_rx_bd_control_status_t;

// Defines the control extended region1 of the receive buffer descriptor.
typedef enum _enet_rx_bd_control_extend0 {
  kEnetRxBdIpHeaderChecksumErr = 0x0020U,  // IP header checksum error; needs L
  kEnetRxBdProtocolChecksumErr = 0x0010U,  // Protocol checksum error; needs L
  kEnetRxBdVlan                = 0x0004U,  // VLAN; needs L
  kEnetRxBdIpv6                = 0x0002U,  // Ipv6 frame; needs L
  kEnetRxBdIpv4Fragment        = 0x0001U,  // Ipv4 fragment; needs L
} enet_rx_bd_control_extend0_t;

// Defines the control extended region2 of the receive buffer descriptor.
typedef enum _enet_rx_bd_control_extend1 {
  kEnetRxBdMacErr    = 0x8000U,  // MAC error; needs L
  kEnetRxBdPhyErr    = 0x0400U,  // PHY error; needs L
  kEnetRxBdCollision = 0x0200U,  // Collision; needs L
  kEnetRxBdUnicast   = 0x0100U,  // Unicast frame; valid even if L is not set
  kEnetRxBdInterrupt = 0x0080U,  // Generate RXB/RXF interrupt
} enet_rx_bd_control_extend1_t;

// Defines the control status of the transmit buffer descriptor.
typedef enum _enet_tx_bd_control_status {
  kEnetTxBdReady        = 0x8000U,  // Ready bit
  kEnetTxBdTxSoftOwner1 = 0x4000U,  // Transmit software ownership
  kEnetTxBdWrap         = 0x2000U,  // Wrap buffer descriptor
  kEnetTxBdTxSoftOwner2 = 0x1000U,  // Transmit software ownership
  kEnetTxBdLast         = 0x0800U,  // Last BD in the frame (L bit)
  kEnetTxBdTransmitCrc  = 0x0400U,  // Transmit CRC; needs L
} enet_tx_bd_control_status_t;

// Defines the control extended region1 of the transmit buffer descriptor.
typedef enum _enet_tx_bd_control_extend0 {
  kEnetTxBdTxErr              = 0x8000U,  // Transmit error; needs L
  kEnetTxBdTxUnderflowErr     = 0x2000U,  // Underflow error; needs L
  kEnetTxBdExcessCollisionErr = 0x1000U,  // Excess collision error; needs L
  kEnetTxBdTxFrameErr         = 0x0800U,  // Frame with error; needs L
  kEnetTxBdLatecollisionErr   = 0x0400U,  // Late collision error; needs L
  kEnetTxBdOverflowErr        = 0x0200U,  // Overflow error; needs L
  kEnetTxTimestampErr         = 0x0100U,  // Timestamp error; needs L
} enet_tx_bd_control_extend0_t;

// Defines the control extended region2 of the transmit buffer descriptor.
typedef enum _enet_tx_bd_control_extend1 {
  kEnetTxBdTxInterrupt   = 0x4000U,  // Transmit interrupt; all BDs
  kEnetTxBdTimestamp     = 0x2000U,  // Transmit timestamp flag; all BDs
  kEnetTxBdProtChecksum  = 0x1000U,  // Insert protocol specific checksum; all BDs
  kEnetTxBdIpHdrChecksum = 0x0800U,  // Insert IP header checksum; all BDs
} enet_tx_bd_control_extend1_t;

typedef struct {
  uint16_t length;
  uint16_t status;
  void     *buffer;
  uint16_t extend0;
  uint16_t extend1;
  uint16_t checksum;
  uint8_t  prototype;
  uint8_t  headerlen;
  uint16_t unused0;
  uint16_t extend2;
  uint32_t timestamp;
  uint16_t unused1;
  uint16_t unused2;
  uint16_t unused3;
  uint16_t unused4;
} enetbufferdesc_t;

typedef enum _enet_init_states {
  kInitStateStart,           // Unknown hardware
  kInitStateNoHardware,      // No PHY
  kInitStateHasHardware,     // Has PHY
  kInitStatePHYInitialized,  // PHY's been initialized
  kInitStateInitialized,     // PHY and MAC have been initialized
} enet_init_states_t;

// --------------------------------------------------------------------------
//  Internal Variables
// --------------------------------------------------------------------------

// Ethernet buffers
alignas(64) static enetbufferdesc_t s_rxRing[RX_SIZE];
alignas(64) static enetbufferdesc_t s_txRing[TX_SIZE];
alignas(64) static uint8_t s_rxBufs[RX_SIZE * BUF_SIZE] BUFFER_DMAMEM;
alignas(64) static uint8_t s_txBufs[TX_SIZE * BUF_SIZE] BUFFER_DMAMEM;
static volatile enetbufferdesc_t *s_pRxBD = &s_rxRing[0];
static volatile enetbufferdesc_t *s_pTxBD = &s_txRing[0];

// Misc. internal state
static atomic_flag s_rxNotAvail       = ATOMIC_FLAG_INIT;
static enet_init_states_t s_initState = kInitStateStart;

// PHY status, polled
static int s_checkLinkStatusState = 0;
static bool s_linkSpeed10Not100 = false;
static bool s_linkIsFullDuplex  = false;
static bool s_linkIsCrossover   = false;

// Forward declarations
static void enet_isr(void);

// --------------------------------------------------------------------------
//  PHY I/O
// --------------------------------------------------------------------------

// PHY register definitions
#define PHY_BCR      0  /* Basic Control Register */
#define PHY_BSR      1  /* Basic Status Register */
#define PHY_PHYID1   2  /* PHY Identifier 1 Register */
#define PHY_PHYID2   3  /* PHY Identifier 2 Register */
#define PHY_ANAR     4  /* Auto Negotiation Advertisement Register */
#define PHY_PHYSCSR 31  /* PHY Special Control/Status Register */

#define PHY_BSR_LINK_STATUS (1 << 2)  /* 0: No link, 1: Valid link */

// Reads a PHY register (using MDIO & MDC signals) and returns whether
// continuation is needed (not complete). If continuation is needed, then this
// should be called again with 'cont' set to true. If this is the first call,
// then 'cont' should be set to false.
static bool mdio_read_nonblocking(const uint16_t regaddr,
                                  uint16_t data[static 1],
                                  const bool cont) {
  if (!cont) {
    ENET2_EIR = ENET_EIR_MII;  // Clear status

    ENET2_MMFR = ENET_MMFR_ST(1) | ENET_MMFR_OP(2) | ENET_MMFR_PA(0/*phyaddr*/) |
                 ENET_MMFR_RA(regaddr) | ENET_MMFR_TA(2);
  }

  if ((ENET2_EIR & ENET_EIR_MII) == 0) {  // Waiting takes on the order of 8.8-8.9us
    return true;
  }

  *data = ENET_MMFR_DATA(ENET2_MMFR);
  ENET2_EIR = ENET_EIR_MII;
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
static bool mdio_write_nonblocking(const uint16_t regaddr, const uint16_t data,
                                   const bool cont) {
  if (!cont) {
    ENET2_EIR = ENET_EIR_MII;  // Clear status

    ENET2_MMFR = ENET_MMFR_ST(1) | ENET_MMFR_OP(1) | ENET_MMFR_PA(0/*phyaddr*/) |
                 ENET_MMFR_RA(regaddr) | ENET_MMFR_TA(2) |
                 ENET_MMFR_DATA(data);
  }

  if ((ENET2_EIR & ENET_EIR_MII) == 0) {  // Waiting takes on the order of 8.8-8.9us
    return true;
  }

  ENET2_EIR = ENET_EIR_MII;
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
FLASHMEM static void enable_enet_clocks(void) {
  // Enable the Ethernet clocks
  CCM_CCGR1 |= CCM_CCGR1_ENET(CCM_CCGR_ON);
  CCM_CCGR7 |= CCM_CCGR7_ENET2(CCM_CCGR_ON);

  // Configure PLL6 for 50 MHz (page 1112)
  CCM_ANALOG_PLL_ENET_SET = CCM_ANALOG_PLL_ENET_BYPASS;
  CCM_ANALOG_PLL_ENET_CLR = 0
                            | CCM_ANALOG_PLL_ENET_BYPASS_CLK_SRC(3)
                            | CCM_ANALOG_PLL_ENET_ENET2_DIV_SELECT(3)
                            | CCM_ANALOG_PLL_ENET_DIV_SELECT(3)
                            ;
  CCM_ANALOG_PLL_ENET_SET = 0
                            // | CCM_ANALOG_PLL_ENET_ENET_25M_REF_EN
                            | CCM_ANALOG_PLL_ENET_ENET2_REF_EN
                            // | CCM_ANALOG_PLL_ENET_ENABLE
                            | CCM_ANALOG_PLL_ENET_ENET2_DIV_SELECT(1)  // 50MHz
                            // | CCM_ANALOG_PLL_ENET_DIV_SELECT(1)        // 50MHz
                            ;
  CCM_ANALOG_PLL_ENET_CLR = CCM_ANALOG_PLL_ENET_POWERDOWN;
  while ((CCM_ANALOG_PLL_ENET & CCM_ANALOG_PLL_ENET_LOCK) == 0) {
    // Wait for PLL lock
  }
  CCM_ANALOG_PLL_ENET_CLR = CCM_ANALOG_PLL_ENET_BYPASS;
  // printf("PLL6 = %08" PRIX32 "h (should be 80202001h)\n", CCM_ANALOG_PLL_ENET);

  // Configure REFCLK to be driven as output by PLL6 (page 325)
  CLRSET(IOMUXC_GPR_GPR1,
         IOMUXC_GPR_GPR1_ENET_IPG_CLK_S_EN | IOMUXC_GPR_GPR1_ENET2_CLK_SEL,
         IOMUXC_GPR_GPR1_ENET2_TX_CLK_DIR);
}

// Disables everything enabled with enable_enet_clocks().
FLASHMEM static void disable_enet_clocks(void) {
  // Configure REFCLK
  CLRSET(IOMUXC_GPR_GPR1, IOMUXC_GPR_GPR1_ENET2_TX_CLK_DIR, 0);

  // Stop the PLL (first bypassing)
  CCM_ANALOG_PLL_ENET_SET = CCM_ANALOG_PLL_ENET_BYPASS;
  CCM_ANALOG_PLL_ENET = 0
                        | CCM_ANALOG_PLL_ENET_BYPASS         // Reset to default
                        | CCM_ANALOG_PLL_ENET_POWERDOWN
                        | CCM_ANALOG_PLL_ENET_DIV_SELECT(1)
                        ;

  // Disable the clocks for ENET
  CCM_CCGR7 &= ~CCM_CCGR7_ENET2(CCM_CCGR_ON);
  CCM_CCGR1 &= ~CCM_CCGR1_ENET(CCM_CCGR_ON);
}

// Configures all the pins necessary for communicating with the PHY.
FLASHMEM static void configure_phy_pins(void) {
  // Configure strap pins
  // Note: The pulls may not be strong enough
  // Note: All the strap pins have internal resistors
  // 3.7.1 PHYAD[0]: PHY Address Configuration (page 26)
  IOMUXC_SW_PAD_CTL_PAD_GPIO_B1_00 = RMII_PAD_PULLDOWN;  // PHYAD0 = 0 (RXER, pin 10) (page 716)
  // 3.7.2 MODE[2:0]: Mode Configuration (all capabilities) (page 27)
  IOMUXC_SW_PAD_CTL_PAD_GPIO_SD_B0_03 = STRAP_PAD_PULLUP;  // RXD0/MODE0 pin 8 (page 749)
  IOMUXC_SW_PAD_CTL_PAD_GPIO_SD_B0_04 = STRAP_PAD_PULLUP;  // RXD1/MODE1 pin 7 (page 751)
  IOMUXC_SW_PAD_CTL_PAD_GPIO_SD_B0_05 = STRAP_PAD_PULLUP;  // CRS_DV/MODE2 pin 11 (page 753)

  // Configure the MDIO and MDC pins
  IOMUXC_SW_PAD_CTL_PAD_GPIO_B1_15 = MDIO_PAD_PULLUP;  // MDIO
  IOMUXC_SW_PAD_CTL_PAD_GPIO_B1_14 = RMII_PAD_PULLUP;  // MDC

  IOMUXC_SW_MUX_CTL_PAD_GPIO_B0_01 = MDIO_MUX;  // MDIO pin 12 (ENET2_MDIO of enet, page 505), Teensy pin 12
  IOMUXC_SW_MUX_CTL_PAD_GPIO_B0_00 = MDIO_MUX;  // MDC pin 13 (ENET2_MDC of enet, page 504), Teensy pin 10

  IOMUXC_ENET_MDIO_SELECT_INPUT = 2;  // GPIO_B1_15_ALT0 (page 791)
      // DAISY:10
}

// Configures all the RMII pins. This should be called after initializing
// the PHY.
FLASHMEM static void configure_rmii_pins(void) {
  // The NXP SDK and original Teensy 4.1 example code use pull-ups
  IOMUXC_SW_PAD_CTL_PAD_GPIO_SD_B0_03 = RMII_PAD_PULLUP;  // Reset this (RXD0)
  IOMUXC_SW_PAD_CTL_PAD_GPIO_SD_B0_04 = RMII_PAD_PULLUP;  // Reset this (RXD1)
  IOMUXC_SW_PAD_CTL_PAD_GPIO_SD_B0_05 = RMII_PAD_PULLUP;  // Reset this (RXEN)
  IOMUXC_SW_PAD_CTL_PAD_GPIO_B1_00 = RMII_PAD_PULLUP;     // Reset this (RXER)
  IOMUXC_SW_PAD_CTL_PAD_GPIO_B1_07 = RMII_PAD_PULLUP;  // TXD0 (PHY has internal pull-down)
  IOMUXC_SW_PAD_CTL_PAD_GPIO_B1_08 = RMII_PAD_PULLUP;  // TXD1 (PHY has internal pull-down)
  IOMUXC_SW_PAD_CTL_PAD_GPIO_B1_09 = RMII_PAD_PULLUP;  // TXEN (PHY has internal pull-down)

  IOMUXC_SW_MUX_CTL_PAD_GPIO_SD_B0_03 = RMII_MUX;  // RXD0 pin 8 (ENET2_RDATA00 of enet, page 539), Teensy pin 34
  IOMUXC_SW_MUX_CTL_PAD_GPIO_SD_B0_04 = RMII_MUX;  // RXD1 pin 7 (ENET2_RDATA01 of enet, page 540), Teensy pin 38
  IOMUXC_SW_MUX_CTL_PAD_GPIO_B1_00    = RMII_MUX;  // RXER pin 10 (ENET2_RX_ER of enet, page 520), Teensy pin 8
  IOMUXC_SW_MUX_CTL_PAD_GPIO_SD_B0_05 = RMII_MUX;  // CRS_DV pin 11 (ENET2_RX_EN of enet, page 541), Teensy pin 39
  IOMUXC_SW_MUX_CTL_PAD_GPIO_EMC_32   = RMII_MUX;  // TXEN pin 16 (ENET2_TX_EN of enet, page 458), Teensy pin 28
  IOMUXC_SW_MUX_CTL_PAD_GPIO_B0_12    = RMII_MUX;  // TXD0 pin 17 (ENET2_TDATA00 of enet, page 516), Teensy pin 32
  IOMUXC_SW_MUX_CTL_PAD_GPIO_EMC_31   = RMII_MUX;  // TXD1 pin 18 (ENET2_TDATA01 of enet, page 457), Teensy pin 29

  IOMUXC_ENET2_IPG_CLK_RMII_SELECT_INPUT = 1;  // GPIO_SD_B0_01_ALT9 (page 914)
      // DAISY:1

  IOMUXC_ENET2_IPP_IND_MAC0_MDIO_SELECT_INPUT     = 1;  // GPIO_B0_01_ALT8 (page 915)
  IOMUXC_ENET2_IPP_IND_MAC0_RXDATA_SELECT_INPUT_0 = 1;  // GPIO_SD_B0_03_ALT8 (page 916)
  IOMUXC_ENET2_IPP_IND_MAC0_RXDATA_SELECT_INPUT_1 = 1;  // GPIO_SD_B0_04_ALT8 (page 916)
  IOMUXC_ENET2_IPP_IND_MAC0_RXEN_SELECT_INPUT     = 1;  // GPIO_SD_B0_05_ALT8 (page 917)
  IOMUXC_ENET2_IPP_IND_MAC0_RXERR_SELECT_INPUT    = 2;  // GPIO_B1_00_ALT8 (page 918)
  IOMUXC_ENET2_IPP_IND_MAC0_TXCLK_SELECT_INPUT    = 1;  // GPIO_SD_B0_01_ALT8 (page 920)
}

// Initialization and check for hardware. This does nothing if the init state
// isn't at START or HAS_HARDWARE. After this function returns, the init state
// will either be NO_HARDWARE or PHY_INITIALIZED, unless it wasn't START or
// HAS_HARDWARE when called.
FLASHMEM static void init_phy(void) {
  if (s_initState != kInitStateStart && s_initState != kInitStateHasHardware) {
    return;
  }

  enable_enet_clocks();

  configure_phy_pins();

  // Note: Ensure the clock is present at the CLKIN at power up
  IOMUXC_SW_PAD_CTL_PAD_GPIO_SD_B0_01 = RMII_PAD_CLOCK;
  IOMUXC_SW_MUX_CTL_PAD_GPIO_SD_B0_01 = RMII_MUX_CLOCK;  // CLKIN pin 5 (ENET2_REF_CLK2 of enet, page 537), Teensy pin 36
  ENET2_MSCR = ENET_MSCR_MII_SPEED(9);  // Internal module clock frequency = 50MHz

  // Check for PHY presence
  if ((mdio_read(PHY_PHYID1) != 0x0007) ||
      ((mdio_read(PHY_PHYID2) & 0xfff0) != 0xC0F0)) {
    disable_enet_clocks();

    s_initState = kInitStateNoHardware;
    return;
  }

  // Configure the PHY registers
  // The strap pull-ups may not have been strong enough, so ensure those values
  // are set properly too
  // Right now, it's just the 50MHz clock select for RMII slave mode

  // mdio_write(PHY_BCR, 0x3100);   // 13: Speed_Select: 1=100Mbps
  //                                // 12: Auto-Negotiation Enable: 1=enabled
  //                                //  8: Duplex Mode: 1=Full-Duplex
  // mdio_write(PHY_ANAR, 0x01E1);  // 8: 100BASE-TX Full Duplex: 1=advertise
  //                                // 7: 100BASE-TX: 1=advertise
  //                                // 6: 10BASE-T Full Duplex: 1=advertise
  //                                // 5: 10BASE-T: 1=advertise
  //                                // 4-0: Selector Field: IEEE 802.3

  s_initState = kInitStatePHYInitialized;
}

// Low-level input function that transforms a received frame into an lwIP pbuf.
// This returns a newly-allocated pbuf, or NULL if there was a frame error or
// allocation error.
static struct pbuf *low_level_input(volatile enetbufferdesc_t *const pBD) {
  const u16_t err_mask = kEnetRxBdTrunc    |
                         kEnetRxBdOverrun  |
                         kEnetRxBdCrc      |
                         kEnetRxBdNonOctet |
                         kEnetRxBdLengthViolation;

  struct pbuf *p = NULL;

  // Determine if a frame has been received
  if (pBD->status & err_mask) {
#if LINK_STATS
    // Either truncated or others
    if (pBD->status & kEnetRxBdTrunc) {
      LINK_STATS_INC(link.lenerr);
    } else if (pBD->status & kEnetRxBdLast) {
      // The others are only valid if the 'L' bit is set
      if (pBD->status & kEnetRxBdOverrun) {
        LINK_STATS_INC(link.err);
      } else {  // Either overrun and others zero, or others
        if (pBD->status & kEnetRxBdNonOctet) {
          LINK_STATS_INC(link.err);
        } else if (pBD->status & kEnetRxBdCrc) {  // Non-octet or CRC
          LINK_STATS_INC(link.chkerr);
        }
        if (pBD->status & kEnetRxBdLengthViolation) {
          LINK_STATS_INC(link.lenerr);
        }
      }
    }
    LINK_STATS_INC(link.drop);
#endif  // LINK_STATS
  } else {
    LINK_STATS_INC(link.recv);
    p = pbuf_alloc(PBUF_RAW, pBD->length, PBUF_POOL);
    if (p) {
#if !QNETHERNET_BUFFERS_IN_RAM1
      arm_dcache_delete(pBD->buffer, MULTIPLE_OF_32(p->tot_len));
#endif  // !QNETHERNET_BUFFERS_IN_RAM1
      pbuf_take(p, pBD->buffer, p->tot_len);
    } else {
      LINK_STATS_INC(link.drop);
      LINK_STATS_INC(link.memerr);
    }
  }

  // Set rx bd empty
  pBD->status = (pBD->status & kEnetRxBdWrap) | kEnetRxBdEmpty;

  ENET2_RDAR = ENET_RDAR_RDAR;

  return p;
}

// Acquires a buffer descriptor. Meant to be used with update_bufdesc().
// This returns NULL if there is no TX buffer available.
static inline volatile enetbufferdesc_t *get_bufdesc(void) {
  volatile enetbufferdesc_t *const pBD = s_pTxBD;

  if ((pBD->status & kEnetTxBdReady) != 0) {
    return NULL;
  }

  return pBD;
}

// Updates a buffer descriptor. Meant to be used with get_bufdesc().
static inline void update_bufdesc(volatile enetbufferdesc_t *const pBD,
                                  const uint16_t len) {
  pBD->length = len;
  pBD->status = (pBD->status & kEnetTxBdWrap) |
                kEnetTxBdTransmitCrc          |
                kEnetTxBdLast                 |
                kEnetTxBdReady;

  ENET2_TDAR = ENET_TDAR_TDAR;

  if (pBD->status & kEnetTxBdWrap) {
    s_pTxBD = &s_txRing[0];
  } else {
    s_pTxBD++;
  }

  LINK_STATS_INC(link.xmit);
}

// Finds the next non-empty BD.
static inline volatile enetbufferdesc_t *rxbd_next(void) {
  volatile enetbufferdesc_t *pBD = s_pRxBD;

  while (pBD->status & kEnetRxBdEmpty) {
    if (pBD->status & kEnetRxBdWrap) {
      pBD = &s_rxRing[0];
    } else {
      pBD++;
    }
    if (pBD == s_pRxBD) {
      return NULL;
    }
  }

  if (s_pRxBD->status & kEnetRxBdWrap) {
    s_pRxBD = &s_rxRing[0];
  } else {
    s_pRxBD++;
  }
  return pBD;
}

// The Ethernet ISR.
static void enet_isr(void) {
  if ((ENET2_EIR & ENET_EIR_RXF) != 0) {
    ENET2_EIR = ENET_EIR_RXF;
    atomic_flag_clear(&s_rxNotAvail);
  }
}

// Checks the link status and returns zero if complete and a state value if
// not complete. The return value should be used in the next call to
// this function.
static inline int check_link_status(struct netif *const netif,
                                    const int state) {
  static uint16_t bsr;
  static uint16_t physcsr;
  static uint8_t is_link_up;

  if (s_initState != kInitStateInitialized) {
    return 0;
  }

  switch (state) {
    case 0:
      // Fallthrough
    case 1:
      if (mdio_read_nonblocking(PHY_BSR, &bsr, state == 1)) {
        return 1;
      }
      is_link_up = ((bsr & PHY_BSR_LINK_STATUS) != 0);
      if (!is_link_up) {
        break;
      }
      // Fallthrough

    case 2:
      if (mdio_read_nonblocking(PHY_PHYSCSR, &physcsr, state == 2)) {
        return 2;
      }
      break;

    default:
      break;
  }

  if (netif_is_link_up(netif) != is_link_up) {
    if (is_link_up) {
      switch ((physcsr >> 2) & 0x07) {
        case 1:
          s_linkSpeed10Not100 = true;
          s_linkIsFullDuplex = false;
          break;
        case 5:
          s_linkSpeed10Not100 = true;
          s_linkIsFullDuplex = true;
          break;
        case 2:
          s_linkSpeed10Not100 = false;
          s_linkIsFullDuplex = false;
          break;
        case 6:
          s_linkSpeed10Not100 = false;
          s_linkIsFullDuplex = true;
          break;
        default:
          s_linkSpeed10Not100 = true;
          s_linkIsFullDuplex = false;
      }

      netif_set_link_up(netif);
    } else {
      netif_set_link_down(netif);
    }
  }

  return 0;
}

// --------------------------------------------------------------------------
//  Driver Interface
// --------------------------------------------------------------------------

FLASHMEM void driver_get_capabilities(struct DriverCapabilities *const dc) {
  dc->isMACSettable              = true;
  dc->isLinkStateDetectable      = true;
  dc->isLinkSpeedDetectable      = true;
  dc->isLinkSpeedSettable        = false;
  dc->isLinkFullDuplexDetectable = true;
  dc->isLinkFullDuplexSettable   = false;
  dc->isLinkCrossoverDetectable  = false;
}

bool driver_is_unknown(void) {
  return s_initState == kInitStateStart;
}

void qnethernet_hal_get_system_mac_address(uint8_t mac[ETH_HWADDR_LEN]);

void driver_get_system_mac(uint8_t mac[ETH_HWADDR_LEN]) {
  qnethernet_hal_get_system_mac_address(mac);
}

bool driver_get_mac(uint8_t mac[ETH_HWADDR_LEN]) {
  // Don't do anything if the Ethernet clock isn't running because register
  // access will freeze the machine
  if ((CCM_CCGR7 & CCM_CCGR7_ENET2(CCM_CCGR_ON)) == 0) {
    return false;
  }

  uint32_t r = ENET2_PALR;
  mac[0] = r >> 24;
  mac[1] = r >> 16;
  mac[2] = r >> 8;
  mac[3] = r;
  r = ENET2_PAUR;
  mac[4] = r >> 24;
  mac[5] = r >> 16;

  return true;
}

bool driver_set_mac(const uint8_t mac[ETH_HWADDR_LEN]) {
  // Don't do anything if the Ethernet clock isn't running because register
  // access will freeze the machine
  if ((CCM_CCGR7 & CCM_CCGR7_ENET2(CCM_CCGR_ON)) == 0) {
    return false;
  }

  __disable_irq();  // TODO: Not sure if disabling interrupts is really needed
  ENET2_PALR = (mac[0] << 24) | (mac[1] << 16) | (mac[2] << 8) | mac[3];
  ENET2_PAUR = (mac[4] << 24) | (mac[5] << 16) | 0x8808;
  __enable_irq();

  return true;
}

bool driver_has_hardware(void) {
  switch (s_initState) {
    case kInitStateHasHardware:
    case kInitStatePHYInitialized:
    case kInitStateInitialized:
      return true;
    case kInitStateNoHardware:
      return false;
    default:
      break;
  }
  init_phy();
  return (s_initState != kInitStateNoHardware);
}

FLASHMEM void driver_set_chip_select_pin(const int pin) {
  LWIP_UNUSED_ARG(pin);
}

// Initializes the PHY and Ethernet interface. This sets the init state and
// returns whether the initialization was successful.
FLASHMEM bool driver_init(void) {
  if (s_initState == kInitStateInitialized) {
    return true;
  }

  init_phy();
  if (s_initState != kInitStatePHYInitialized) {
    return false;
  }

  // Configure pins
  // TODO: What should these actually be? Why pull-ups? Note that the reference code uses pull-ups.
  // Note: The original code left RXD0, RXEN, and RXER with PULLDOWN
  configure_rmii_pins();

  memset(s_rxRing, 0, sizeof(s_rxRing));
  memset(s_txRing, 0, sizeof(s_txRing));

  for (int i = 0; i < RX_SIZE; i++) {
    s_rxRing[i].buffer  = &s_rxBufs[i * BUF_SIZE];
    s_rxRing[i].status  = kEnetRxBdEmpty;
    s_rxRing[i].extend1 = kEnetRxBdInterrupt;
  }
  // The last buffer descriptor should be set with the wrap flag
  s_rxRing[RX_SIZE - 1].status |= kEnetRxBdWrap;

  for (int i = 0; i < TX_SIZE; i++) {
    s_txRing[i].buffer  = &s_txBufs[i * BUF_SIZE];
    s_txRing[i].status  = kEnetTxBdTransmitCrc;
    s_txRing[i].extend1 = kEnetTxBdTxInterrupt  |
                          kEnetTxBdProtChecksum |
                          kEnetTxBdIpHdrChecksum;
  }
  s_txRing[TX_SIZE - 1].status |= kEnetTxBdWrap;

  ENET2_EIMR = 0;  // This also deasserts all interrupts

  ENET2_RCR = 0
              | ENET_RCR_NLC        // Payload length is checked
              | ENET_RCR_MAX_FL(MAX_FRAME_LEN)
              | ENET_RCR_CFEN       // Discard non-pause MAC control frames
              | ENET_RCR_CRCFWD     // CRC is stripped (ignored if PADEN)
              | ENET_RCR_PADEN      // Padding is removed
              | ENET_RCR_RMII_MODE
              | ENET_RCR_FCE        // Flow control enable
#if QNETHERNET_ENABLE_PROMISCUOUS_MODE
             | ENET_RCR_PROM       // Promiscuous mode
#endif  // QNETHERNET_ENABLE_PROMISCUOUS_MODE
             | ENET_RCR_MII_MODE;
  ENET2_TCR = 0
              | ENET_TCR_ADDINS     // Overwrite with programmed MAC address
              | ENET_TCR_ADDSEL(0)
              // | ENET_TCR_RFC_PAUSE
              // | ENET_TCR_TFC_PAUSE
              | ENET_TCR_FDEN         // Enable full-duplex
              ;

  ENET2_TACC = 0
#if CHECKSUM_GEN_UDP == 0 || CHECKSUM_GEN_TCP == 0 || CHECKSUM_GEN_ICMP == 0
      | ENET_TACC_PROCHK  // Insert protocol checksum
#endif  // not(Generate all checksums)
#if CHECKSUM_GEN_IP == 0
      | ENET_TACC_IPCHK   // Insert IP header checksum
#endif  // CHECKSUM_GEN_IP == 0
#if ETH_PAD_SIZE == 2
      | ENET_TACC_SHIFT16
#endif  // ETH_PAD_SIZE == 2
      ;

  ENET2_RACC = 0
#if ETH_PAD_SIZE == 2
      | ENET_RACC_SHIFT16
#endif  // ETH_PAD_SIZE == 2
      | ENET_RACC_LINEDIS  // Discard bad frames
#if CHECKSUM_CHECK_UDP == 0 && \
    CHECKSUM_CHECK_TCP == 0 && \
    CHECKSUM_CHECK_ICMP == 0
      | ENET_RACC_PRODIS   // Discard frames with incorrect protocol checksum
                           // Requires RSFL == 0
#endif  // not(Check any checksums)
#if CHECKSUM_CHECK_IP == 0
      | ENET_RACC_IPDIS    // Discard frames with incorrect IPv4 header checksum
                           // Requires RSFL == 0
#endif  // CHECKSUM_CHECK_IP == 0
      | ENET_RACC_PADREM
      ;

  ENET2_TFWR = ENET_TFWR_STRFWD;
  ENET2_RSFL = 0;

  ENET2_RDSR = (uint32_t)s_rxRing;
  ENET2_TDSR = (uint32_t)s_txRing;
  ENET2_MRBR = BUF_SIZE;

  ENET2_RXIC = 0;
  ENET2_TXIC = 0;
  // ENET2_PALR = (mac[0] << 24) | (mac[1] << 16) | (mac[2] << 8) | mac[3];
  // ENET2_PAUR = (mac[4] << 24) | (mac[5] << 16) | 0x8808;

  ENET2_OPD = 0x10014;
  ENET2_RSEM = 0;
  ENET2_MIBC = 0;

  ENET2_IAUR = 0;
  ENET2_IALR = 0;
  ENET2_GAUR = 0;
  ENET2_GALR = 0;

  ENET2_EIMR = ENET_EIMR_RXF;
  attachInterruptVector(IRQ_ENET2, &enet_isr);
  NVIC_ENABLE_IRQ(IRQ_ENET2);

  // Last few things to do
  ENET2_EIR = 0x7fff8000;  // Clear any pending interrupts before setting ETHEREN
  atomic_flag_test_and_set(&s_rxNotAvail);

  // Last, enable the Ethernet MAC
  ENET2_ECR = 0x70000000 | ENET_ECR_DBSWP | ENET_ECR_EN1588 | ENET_ECR_ETHEREN;

  // Indicate there are empty RX buffers and available ready TX buffers
  ENET2_RDAR = ENET_RDAR_RDAR;
  ENET2_TDAR = ENET_TDAR_TDAR;

  // PHY soft reset
  // mdio_write(PHY_BMCR, 1 << 15);

  s_initState = kInitStateInitialized;

  return true;
}

void unused_interrupt_vector(void);  // startup.c

FLASHMEM void driver_deinit(void) {
  // Something about stopping Ethernet and the PHY kills performance if Ethernet
  // is restarted after calling end(), so gate the following two blocks with a
  // macro for now

#if QNETHERNET_INTERNAL_END_STOPS_ALL
  if (s_initState == kInitStateInitialized) {
    NVIC_DISABLE_IRQ(IRQ_ENET);
    attachInterruptVector(IRQ_ENET, &unused_interrupt_vector);
    ENET2_EIMR = 0;  // Disable interrupts

    // Gracefully stop any transmission before disabling the Ethernet MAC
    ENET2_EIR = ENET_EIR_GRA;  // Clear status
    ENET2_TCR |= ENET_TCR_GTS;
    while ((ENET_EIR & ENET_EIR_GRA) == 0) {
      // Wait until it's gracefully stopped
    }
    ENET2_EIR = ENET_EIR_GRA;

    // Disable the Ethernet MAC
    // Note: All interrupts are cleared when Ethernet is reinitialized,
    //       so nothing will be pending
    ENET2_ECR = 0x70000000;

    s_initState = kInitStatePHYInitialized;
  }

  if (s_initState == kInitStatePHYInitialized) {
    disable_enet_clocks();

    s_initState = kInitStateHasHardware;
  }
#endif  // QNETHERNET_INTERNAL_END_STOPS_ALL
}

struct pbuf *driver_proc_input(struct netif *const netif, const int counter) {
  // Finish any pending link status check
  if (s_checkLinkStatusState != 0) {
    s_checkLinkStatusState = check_link_status(netif, s_checkLinkStatusState);
  }

  if (counter == 0) {
    if (atomic_flag_test_and_set(&s_rxNotAvail)) {
      return NULL;
    }
  } else if (counter >= RX_SIZE * 2) {
    return NULL;
  }

  // Get the next chunk of input data
  volatile enetbufferdesc_t *const pBD = rxbd_next();
  if (pBD == NULL) {
    return NULL;
  }
  return low_level_input(pBD);
}

void driver_poll(struct netif *const netif) {
  s_checkLinkStatusState = check_link_status(netif, s_checkLinkStatusState);
}

int driver_link_speed(void) {
  return s_linkSpeed10Not100 ? 10 : 100;
}

bool driver_link_set_speed(const int speed) {
  LWIP_UNUSED_ARG(speed);
  return false;
}

bool driver_link_is_full_duplex(void) {
  return s_linkIsFullDuplex;
}

bool driver_link_set_full_duplex(const bool flag) {
  LWIP_UNUSED_ARG(flag);
  return false;
}

bool driver_link_is_crossover(void) {
  return s_linkIsCrossover;
}

// Outputs data from the MAC.
err_t driver_output(struct pbuf *const p) {
  // Note: The pbuf already contains the padding (ETH_PAD_SIZE)
  volatile enetbufferdesc_t *const pBD = get_bufdesc();
  if (pBD == NULL) {
    LINK_STATS_INC(link.memerr);
    LINK_STATS_INC(link.drop);
    return ERR_WOULDBLOCK;  // Could also use ERR_MEM, but this lets things like
                            // UDP senders know to retry
  }
  const uint16_t copied = pbuf_copy_partial(p, pBD->buffer, p->tot_len, 0);
  if (copied == 0) {
    LINK_STATS_INC(link.err);
    LINK_STATS_INC(link.drop);
    return ERR_BUF;
  }
#if !QNETHERNET_BUFFERS_IN_RAM1
  arm_dcache_flush_delete(pBD->buffer, MULTIPLE_OF_32(copied));
#endif  // !QNETHERNET_BUFFERS_IN_RAM1
  update_bufdesc(pBD, copied);
  return ERR_OK;
}

#if QNETHERNET_ENABLE_RAW_FRAME_SUPPORT
bool driver_output_frame(const void *const frame, const size_t len) {
  if (s_initState != kInitStateInitialized) {
    return false;
  }

  volatile enetbufferdesc_t *const pBD = get_bufdesc();
  if (pBD == NULL) {
    return false;
  }

  memcpy((uint8_t *)pBD->buffer + ETH_PAD_SIZE, frame, len);
#if !QNETHERNET_BUFFERS_IN_RAM1
  arm_dcache_flush_delete(pBD->buffer, MULTIPLE_OF_32(len + ETH_PAD_SIZE));
#endif  // !QNETHERNET_BUFFERS_IN_RAM1
  update_bufdesc(pBD, len + ETH_PAD_SIZE);

  return true;
}
#endif  // QNETHERNET_ENABLE_RAW_FRAME_SUPPORT

// --------------------------------------------------------------------------
//  MAC Address Filtering
// --------------------------------------------------------------------------

#if !QNETHERNET_ENABLE_PROMISCUOUS_MODE

// CRC-32 routine for computing the 4-byte FCS for multicast lookup. The initial
// value will be zero.
static uint32_t crc32(const void *const data, const size_t len) {
  // https://create.stephan-brumme.com/crc32/#fastest-bitwise-crc32

  uint32_t crc = 0;  // Initial value
  const uint8_t *pData = data;
  size_t lenRem = len;

  crc = ~crc;
  while (lenRem--) {
    crc ^= *(pData++);
    for (int j = 0; j < 8; j++) {
      crc = (crc >> 1) ^ (-(crc & 0x01) & 0xEDB88320);
    }
  }
  return crc;
}

bool driver_set_incoming_mac_address_allowed(const uint8_t mac[ETH_HWADDR_LEN],
                                             const bool allow) {
  // Don't release bits that have had a collision. Track these here.
  static uint32_t collisionGALR = 0;
  static uint32_t collisionGAUR = 0;
  static uint32_t collisionIALR = 0;
  static uint32_t collisionIAUR = 0;

  if (mac == NULL) {
    return false;
  }

  const uint32_t crc = (crc32(mac, ETH_HWADDR_LEN) >> 26) & 0x3f;
  const uint32_t value = 1 << (crc & 0x1f);

  // Choose which locations

  const bool isGroup = (mac[0] & 0x01) != 0;
  volatile uint32_t *reg;
  uint32_t *collision;

  if (crc < 0x20) {
    if (isGroup) {
      reg = &ENET2_GALR;
      collision = &collisionGALR;
    } else {
      reg = &ENET2_IALR;
      collision = &collisionIALR;
    }
  } else {
    if (isGroup) {  // Group
      reg = &ENET2_GAUR;
      collision = &collisionGAUR;
    } else {
      reg = &ENET2_IAUR;
      collision = &collisionIAUR;
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

#endif  // QNETHERNET_INTERNAL_DRIVER_TEENSYMM
