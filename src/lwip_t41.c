// SPDX-FileCopyrightText: (c) 2021-2023 Shawn Silverman <shawn@pobox.com>
// SPDX-License-Identifier: MIT

// lwip_t41.c contains the Teensy 4.1 Ethernet interface implementation.
// Based on code from Paul Stoffregen and others:
// https://github.com/PaulStoffregen/teensy41_ethernet
// This file is part of the QNEthernet library.

#if defined(ARDUINO_TEENSY41)

#include "lwip_driver.h"

// C includes
#include <stdatomic.h>
#include <string.h>

#include <core_pins.h>
#include <imxrt.h>
#include <pgmspace.h>

#include "lwip/arch.h"
#include "lwip/autoip.h"
#include "lwip/dhcp.h"
#include "lwip/err.h"
#include "lwip/etharp.h"
#include "lwip/init.h"
#include "lwip/opt.h"
#include "lwip/pbuf.h"
#include "lwip/prot/ethernet.h"
#include "lwip/prot/ieee.h"
#include "lwip/stats.h"
#include "lwip/timeouts.h"
#include "netif/ethernet.h"

// https://forum.pjrc.com/threads/60532-Teensy-4-1-Beta-Test?p=237096&viewfull=1#post237096
// https://github.com/PaulStoffregen/teensy41_ethernet/blob/master/teensy41_ethernet.ino

// [PHY Datasheet](https://www.pjrc.com/teensy/dp83825i.pdf)
// [i.MX RT1062 Manual](https://www.pjrc.com/teensy/IMXRT1060RM_rev3.pdf)

// --------------------------------------------------------------------------
//  Defines
// --------------------------------------------------------------------------

#define CLRSET(reg, clear, set) ((reg) = ((reg) & ~(clear)) | (set))

#define RMII_PAD_PULLDOWN (IOMUXC_PAD_PUS(0)   | \
                           IOMUXC_PAD_PUE      | \
                           IOMUXC_PAD_PKE      | \
                           IOMUXC_PAD_SPEED(3) | \
                           IOMUXC_PAD_DSE(5)   | \
                           IOMUXC_PAD_SRE)
    // 0x30E9
    // HYS:0 PUS:00 PUE:1 PKE:1 ODE:0 000 SPEED:11 DSE:101 00 SRE:1
    // HYS_0_Hysteresis_Disabled
    // PUS_0_100K_Ohm_Pull_Down
    // PUE_1_Pull
    // PKE_1_Pull_Keeper_Enabled
    // ODE_0_Open_Drain_Disabled
    // SPEED_3_max_200MHz
    // DSE_5_R0_5
    // SRE_1_Fast_Slew_Rate

#define RMII_PAD_PULLUP (IOMUXC_PAD_PUS(2)   | \
                         IOMUXC_PAD_PUE      | \
                         IOMUXC_PAD_PKE      | \
                         IOMUXC_PAD_SPEED(3) | \
                         IOMUXC_PAD_DSE(5)   | \
                         IOMUXC_PAD_SRE)
    // 0xB0E9
    // HYS:0 PUS:10 PUE:1 PKE:1 ODE:0 000 SPEED:11 DSE:101 00 SRE:1
    // HYS_0_Hysteresis_Disabled
    // PUS_2_100K_Ohm_Pull_Up
    // PUE_1_Pull
    // PKE_1_Pull_Keeper_Enabled
    // ODE_0_Open_Drain_Disabled
    // SPEED_3_max_200MHz
    // DSE_5_R0_5
    // SRE_1_Fast_Slew_Rate

#define RMII_PAD_CLOCK (IOMUXC_PAD_DSE(6) | IOMUXC_PAD_SRE)
    // 0x0031
    // HYS:0 PUS:00 PUE:0 PKE:0 ODE:0 000 SPEED:00 DSE:110 00 SRE:1
    // HYS_0_Hysteresis_Disabled
    // PUS_0_100K_Ohm_Pull_Down
    // PUE_0_Keeper
    // PKE_0_Pull_Keeper_Disabled
    // ODE_0_Open_Drain_Disabled
    // SPEED_0_low_50MHz
    // DSE_6_R0_6
    // SRE_1_Fast_Slew_Rate

#define RX_SIZE 5
#define TX_SIZE 5
#define IRQ_PRIORITY 64

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

#ifndef QNETHERNET_BUFFERS_IN_RAM1
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
  kEnetRxBdWrap            = 0x2000U,  // Update buffer descriptor
  kEnetRxBdRxSoftOwner2    = 0x1000U,  // Receive software ownership
  kEnetRxBdLast            = 0x0800U,  // Last BD in the frame (L bit)
  kEnetRxBdMiss            = 0x0100U,  // In promiscuous mode; needs L
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
static enetbufferdesc_t s_rxRing[RX_SIZE] __attribute__((aligned(64)));
static enetbufferdesc_t s_txRing[TX_SIZE] __attribute__((aligned(64)));
static uint8_t s_rxBufs[RX_SIZE * BUF_SIZE] BUFFER_DMAMEM __attribute__((aligned(64)));
static uint8_t s_txBufs[TX_SIZE * BUF_SIZE] BUFFER_DMAMEM __attribute__((aligned(64)));
static volatile enetbufferdesc_t *s_pRxBD = &s_rxRing[0];
static volatile enetbufferdesc_t *s_pTxBD = &s_txRing[0];

// Misc. internal state
static uint8_t s_mac[ETH_HWADDR_LEN];
static struct netif s_t41_netif       = { .name = {'e', '0'} };
static atomic_flag s_rxNotAvail       = ATOMIC_FLAG_INIT;
static enet_init_states_t s_initState = kInitStateStart;
static bool s_isNetifAdded            = false;

// Structs for avoiding memory allocation
#if LWIP_DHCP
static struct dhcp s_dhcp;
#endif  // LWIP_DHCP
#if LWIP_AUTOIP
static struct autoip s_autoip;
#endif  // LWIP_AUTOIP

// PHY status, polled
static bool s_linkSpeed10Not100 = false;
static bool s_linkIsFullDuplex  = false;
static bool s_linkIsCrossover   = false;

// Forward declarations
static void enet_isr();

// --------------------------------------------------------------------------
//  PHY I/O
// --------------------------------------------------------------------------

// PHY register definitions
#define PHY_REGCR  0x0D
#define PHY_ADDAR  0x0E
#define PHY_LEDCR  0x18
#define PHY_RCSR   0x17
#define PHY_BMSR   0x01
#define PHY_PHYSTS 0x10

#define PHY_LEDCR_BLINK_RATE_20Hz (0 << 9)
#define PHY_LEDCR_BLINK_RATE_10Hz (1 << 9)
#define PHY_LEDCR_BLINK_RATE_5Hz  (2 << 9)
#define PHY_LEDCR_BLINK_RATE_2Hz  (3 << 9)
#define PHY_LEDCR_LED_LINK_POLARITY_ACTIVE_HIGH (1 << 7)

#define PHY_LEDCR_VALUE (PHY_LEDCR_BLINK_RATE_10Hz | \
                         PHY_LEDCR_LED_LINK_POLARITY_ACTIVE_HIGH)

#define PHY_RCSR_RMII_CLOCK_SELECT_50MHz               (1 << 7)
#define PHY_RCSR_RECEIVE_ELASTICITY_BUFFER_SIZE_14_BIT (0 << 0)
#define PHY_RCSR_RECEIVE_ELASTICITY_BUFFER_SIZE_2_BIT  (1 << 0)
#define PHY_RCSR_RECEIVE_ELASTICITY_BUFFER_SIZE_6_BIT  (2 << 0)
#define PHY_RCSR_RECEIVE_ELASTICITY_BUFFER_SIZE_10_BIT (3 << 0)

#define PHY_RCSR_VALUE (PHY_RCSR_RMII_CLOCK_SELECT_50MHz | \
                        PHY_RCSR_RECEIVE_ELASTICITY_BUFFER_SIZE_2_BIT)

#define PHY_BMSR_LINK_STATUS (1 << 2)  /* 0: No link, 1: Valid link */

#define PHY_PHYSTS_LINK_STATUS   (1 <<  0)  /* 0: No link, 1: Valid link */
#define PHY_PHYSTS_SPEED_STATUS  (1 <<  1)  /* 0: 100Mbps, 1: 10Mbps */
#define PHY_PHYSTS_DUPLEX_STATUS (1 <<  2)  /* 0: Half-Duplex, 1: Full-Duplex */
#define PHY_PHYSTS_MDI_MDIX_MODE (1 << 14)  /* 0: Normal, 1: Swapped */

// Reads a PHY register (using MDIO & MDC signals).
uint16_t mdio_read(uint16_t regaddr) {
  ENET_EIR = ENET_EIR_MII;  // Clear status

  ENET_MMFR = ENET_MMFR_ST(1) | ENET_MMFR_OP(2) | ENET_MMFR_PA(0/*phyaddr*/) |
              ENET_MMFR_RA(regaddr) | ENET_MMFR_TA(2);
  // int count = 0;
  while ((ENET_EIR & ENET_EIR_MII) == 0) {
    // count++;  // Wait
  }
  // printf("mdio read waited %d\r\n", count);
  uint16_t data = ENET_MMFR_DATA(ENET_MMFR);
  ENET_EIR = ENET_EIR_MII;
  // printf("mdio read (%04xh): %04xh\r\n", regaddr, data);
  return data;
}

// Writes a PHY register (using MDIO & MDC signals).
void mdio_write(uint16_t regaddr, uint16_t data) {
  ENET_EIR = ENET_EIR_MII;  // Clear status

  ENET_MMFR = ENET_MMFR_ST(1) | ENET_MMFR_OP(1) | ENET_MMFR_PA(0/*phyaddr*/) |
              ENET_MMFR_RA(regaddr) | ENET_MMFR_TA(2) |
              ENET_MMFR_DATA(data);
  // int count = 0;
  while ((ENET_EIR & ENET_EIR_MII) == 0) {
    // count++;  // wait
  }
  ENET_EIR = ENET_EIR_MII;
  // print("mdio write waited ", count);
  // printhex("mdio write :", data);
}

// --------------------------------------------------------------------------
//  Low-Level
// --------------------------------------------------------------------------

// Initial check for hardware. This does nothing if the init state isn't at
// START or HAS_HARDWARE. After this function returns, the init state will
// either be NO_HARDWARE or PHY_INITIALIZED, unless it wasn't START or
// HAS_HARDWARE when called.
static void t41_init_phy() {
  if (s_initState != kInitStateStart && s_initState != kInitStateHasHardware) {
    return;
  }

  // Enable the Ethernet clock
  CCM_CCGR1 |= CCM_CCGR1_ENET(CCM_CCGR_ON);

  // Configure PLL6 for 50 MHz, pg 1112 (Rev. 3, 1118 Rev. 2, 1173 Rev. 1)
  CCM_ANALOG_PLL_ENET_SET = CCM_ANALOG_PLL_ENET_BYPASS;
  CCM_ANALOG_PLL_ENET_CLR = CCM_ANALOG_PLL_ENET_BYPASS_CLK_SRC(3) |
                            CCM_ANALOG_PLL_ENET_ENET2_DIV_SELECT(3) |
                            CCM_ANALOG_PLL_ENET_DIV_SELECT(3);
  CCM_ANALOG_PLL_ENET_SET = CCM_ANALOG_PLL_ENET_ENET_25M_REF_EN |
                            // CCM_ANALOG_PLL_ENET_ENET2_REF_EN |
                            CCM_ANALOG_PLL_ENET_ENABLE |
                            // CCM_ANALOG_PLL_ENET_ENET2_DIV_SELECT(1) |
                            CCM_ANALOG_PLL_ENET_DIV_SELECT(1);
  CCM_ANALOG_PLL_ENET_CLR = CCM_ANALOG_PLL_ENET_POWERDOWN;
  while ((CCM_ANALOG_PLL_ENET & CCM_ANALOG_PLL_ENET_LOCK) == 0) {
    // Wait for PLL lock
  }
  CCM_ANALOG_PLL_ENET_CLR = CCM_ANALOG_PLL_ENET_BYPASS;
  // printf("PLL6 = %08" PRIX32 "h (should be 80202001h)\n", CCM_ANALOG_PLL_ENET);

  // Configure REFCLK to be driven as output by PLL6, pg 325 (Rev. 3, 328 Rev. 2, 326 Rev. 1)
  CLRSET(IOMUXC_GPR_GPR1,
         IOMUXC_GPR_GPR1_ENET1_CLK_SEL | IOMUXC_GPR_GPR1_ENET_IPG_CLK_S_EN,
         IOMUXC_GPR_GPR1_ENET1_TX_CLK_DIR);

  // Configure pins

  // PHY strap configuration (before power up or reset)
  // Note: The datasheet suggests 2.49kohm, but we can only do 100kohm pull-down
  IOMUXC_SW_PAD_CTL_PAD_GPIO_B1_04 = RMII_PAD_PULLDOWN;  // PhyAdd[0] = 0 (RX_D0, pin 18)
  IOMUXC_SW_PAD_CTL_PAD_GPIO_B1_06 = RMII_PAD_PULLDOWN;  // PhyAdd[1] = 0 (CRS_DV, pin 17)
  IOMUXC_SW_PAD_CTL_PAD_GPIO_B1_05 = RMII_PAD_PULLUP;    // Master/Slave = RMII Slave Mode (RX_D1, pin 20)
  IOMUXC_SW_PAD_CTL_PAD_GPIO_B1_11 = RMII_PAD_PULLDOWN;  // Auto MDIX Enable (RX_ER, pin 22)
  // 50MHzOut/LED2 (pin 2, pull-down): RX_DV_En: Pin 20 is configured as CRS_DV
  // LED0 (pin 4, pull-down): ANeg_Dis: Auto Negotiation Enable

  IOMUXC_SW_MUX_CTL_PAD_GPIO_B0_14 = 5;  // Reset    B0_14 Alt5 GPIO7.14
      // SION:0 MUX_MODE:0101
      // DISABLED
      // ALT5 (GPIO2_IO14)
  IOMUXC_SW_MUX_CTL_PAD_GPIO_B0_15 = 5;  // Power    B0_15 Alt5 GPIO7.15
      // SION:0 MUX_MODE:0101
      // DISABLED
      // ALT5 (GPIO2_IO15)

  IOMUXC_SW_PAD_CTL_PAD_GPIO_B1_10 = RMII_PAD_CLOCK;

  // Configure the MDIO and MDC pins
  // Note: The original code didn't have these
  IOMUXC_SW_PAD_CTL_PAD_GPIO_B1_15 = IOMUXC_PAD_PUS(2) |
                                     IOMUXC_PAD_PUE    |
                                     IOMUXC_PAD_PKE    |
                                     IOMUXC_PAD_DSE(5) |
                                     IOMUXC_PAD_SRE;
      // 0xB029 (RMII_PAD_PULLUP, but with low speed)
      // HYS:0 PUS:10 PUE:1 PKE:1 ODE:0 000 SPEED:00 DSE:101 00 SRE:1
      // HYS_0_Hysteresis_Disabled
      // PUS_2_100K_Ohm_Pull_Up
      // PUE_1_Pull
      // PKE_1_Pull_Keeper_Enabled
      // ODE_1_Open_Drain_Disabled (because only one PHY)
      // SPEED_0_low_50MHz
      // DSE_5_R0_5
      // SRE_1_Fast_Slew_Rate
  IOMUXC_SW_PAD_CTL_PAD_GPIO_B1_14 = RMII_PAD_PULLUP;

  IOMUXC_SW_MUX_CTL_PAD_GPIO_B1_10 = 6 | 0x10;  // REFCLK, pg 530 (Rev. 3, 534 Rev. 2, 530 Rev. 1)
      // SION:1 MUX_MODE:0110
      // ENABLED
      // ALT6 (ENET_REF_CLK)
  IOMUXC_SW_MUX_CTL_PAD_GPIO_B1_15 = 0;  // MDIO, pg 535 (Rev. 3, 539 Rev. 2, 535 Rev. 1)
      // SION:0 MUX_MODE:0000
      // DISABLED
      // ALT0 (ENET_MDIO)
  IOMUXC_SW_MUX_CTL_PAD_GPIO_B1_14 = 0;  // MDC, pg 534 (Rev. 3, 538 Rev. 2, 534 Rev. 1)
      // SION:0 MUX_MODE:0000
      // DISABLED
      // ALT0 (ENET_MDC)

  IOMUXC_ENET_MDIO_SELECT_INPUT = 2;  // pg 791 (Rev. 3, 795 Rev. 2, 792 Rev. 1)
      // DAISY:10
      // GPIO_B1_15_ALT0

  IOMUXC_ENET_IPG_CLK_RMII_SELECT_INPUT = 1;  // pg 791 (Rev. 3, 795 Rev. 2, 791 Rev. 1)
      // DAISY:1
      // GPIO_B1_10_ALT6

  ENET_MSCR = ENET_MSCR_MII_SPEED(9);

  // Note: Ensure the clock is present at the PHY (XI) at power up

  // Use delays to let things settle, just in case; there's been some
  // restart issues when calling Ethernet.end() and then restarting:
  // Not all packets seem to be received from the PHY; this attempts
  // to help mitigate this problem.
  GPIO7_GDIR    |= (1 << 14) | (1 << 15);
  GPIO7_DR_CLEAR = (1 << 15) | (1 << 14);  // Ensure power off and reset
  delay(50);  // Use T4, post power-up stabilization time for the off time
  GPIO7_DR_SET   = (1 << 15);              // Power on
  delay(50);  // Use T4, Post power-up stabilization time for the on time

  // PHY timing: pg 9, Section 6.6 "Timing Requirements"
  // Reset Timing:
  // T1: Minimum RESET_PULSE width (w/o debouncing caps): 25us
  // T2: Reset to SMI ready: Post reset stabilization time prior to MDC preamble
  //     for register access: 2ms
  // Power-Up Timing:
  // T4: Powerup to SMI ready: Post power-up stabilization time prior to MDC
  //     preamble for register access: 50ms
  // delayMicroseconds(25);  // T1, minimum RESET_PULSE width
  GPIO7_DR_SET = (1 << 14);  // Take out of reset
  delay(50);  // T4, power-up stabilization time

  // LEDCR offset 0x18, set LED_Link_Polarity and Blink_rate, pg 62
  // LED shows link status, active high, 10Hz
  mdio_write(PHY_LEDCR, PHY_LEDCR_VALUE);

  // Check for PHY presence
  if (mdio_read(PHY_LEDCR) != PHY_LEDCR_VALUE) {
    // Power down the PHY
    GPIO7_DR_CLEAR = (1 << 15);

    // Stop the PLL (first bypassing)
    CCM_ANALOG_PLL_ENET_SET = CCM_ANALOG_PLL_ENET_BYPASS;
    CCM_ANALOG_PLL_ENET_SET = CCM_ANALOG_PLL_ENET_POWERDOWN;

    // Disable the clock for ENET
    CCM_CCGR1 &= ~CCM_CCGR1_ENET(CCM_CCGR_ON);

    s_initState = kInitStateNoHardware;
    return;
  }

  s_initState = kInitStatePHYInitialized;
}

static void t41_low_level_init() {
  t41_init_phy();
  if (s_initState != kInitStatePHYInitialized) {
    return;
  }

  // Configure pins
  // TODO: What should these actually be? Why pull-ups? Note that the reference code uses pull-ups.
  // Note: The original code left RXD0, RXEN, and RXER with PULLDOWN
  IOMUXC_SW_PAD_CTL_PAD_GPIO_B1_04 = RMII_PAD_PULLUP;  // Reset this (RXD0)
  IOMUXC_SW_PAD_CTL_PAD_GPIO_B1_06 = RMII_PAD_PULLUP;  // Reset this (RXEN)
  // IOMUXC_SW_PAD_CTL_PAD_GPIO_B1_05 = RMII_PAD_PULLUP;  // Reset this (RXD1)
  IOMUXC_SW_PAD_CTL_PAD_GPIO_B1_11 = RMII_PAD_PULLUP;  // Reset this (RXER)
  IOMUXC_SW_PAD_CTL_PAD_GPIO_B1_07 = RMII_PAD_PULLUP;  // TXD0
  IOMUXC_SW_PAD_CTL_PAD_GPIO_B1_08 = RMII_PAD_PULLUP;  // TXD1
  IOMUXC_SW_PAD_CTL_PAD_GPIO_B1_09 = RMII_PAD_PULLUP;  // TXEN

  IOMUXC_SW_MUX_CTL_PAD_GPIO_B1_05 = 3;  // RXD1, pg 525 (Rev. 3, 529 Rev. 2, 525 Rev. 1)
      // SION:0 MUX_MODE:0011
      // DISABLED
      // ALT3 (ENET_RX_DATA01)
  IOMUXC_SW_MUX_CTL_PAD_GPIO_B1_04 = 3;  // RXD0, pg 524 (Rev. 3, 528 Rev. 2, 524 Rev. 1)
      // SION:0 MUX_MODE:0011
      // DISABLED
      // ALT3 (ENET_RX_DATA00)
  IOMUXC_SW_MUX_CTL_PAD_GPIO_B1_11 = 3;  // RXER, pg 531 (Rev. 3, 535 Rev. 2, 531 Rev. 1)
      // SION:0 MUX_MODE:0011
      // DISABLED
      // ALT3 (ENET_RX_ER)
  IOMUXC_SW_MUX_CTL_PAD_GPIO_B1_06 = 3;  // RXEN, pg 526 (Rev. 3, 530 Rev. 2, 526 Rev. 1)
      // SION:0 MUX_MODE:0011
      // DISABLED
      // ALT3 (ENET_RX_EN)
  IOMUXC_SW_MUX_CTL_PAD_GPIO_B1_09 = 3;  // TXEN, pg 529 (Rev. 3, 533 Rev. 2, 529 Rev. 1)
      // SION:0 MUX_MODE:0011
      // DISABLED
      // ALT3 (ENET_TX_EN)
  IOMUXC_SW_MUX_CTL_PAD_GPIO_B1_07 = 3;  // TXD0, pg 527 (Rev. 3, 531 Rev. 2, 527 Rev. 1)
      // SION:0 MUX_MODE:0011
      // DISABLED
      // ALT3 (ENET_TX_DATA00)
  IOMUXC_SW_MUX_CTL_PAD_GPIO_B1_08 = 3;  // TXD1, pg 528 (Rev. 3, 532 Rev. 2, 528 Rev. 1)
      // SION:0 MUX_MODE:0011
      // DISABLED
      // ALT3 (ENET_TX_DATA01)

  IOMUXC_ENET0_RXDATA_SELECT_INPUT = 1;  // pg 792 (Rev. 3, 796 Rev. 2, 792 Rev. 1)
      // DAISY:1
      // GPIO_B1_04_ALT3
  IOMUXC_ENET1_RXDATA_SELECT_INPUT = 1;  // pg 793 (Rev. 3, 797 Rev. 2, 793 Rev. 1)
      // DAISY:1
      // GPIO_B1_05_ALT3
  IOMUXC_ENET_RXEN_SELECT_INPUT = 1;     // pg 794 (Rev. 3, 798 Rev. 2, 794 Rev. 1)
      // DAISY:1
      // GPIO_B1_06_ALT3
  IOMUXC_ENET_RXERR_SELECT_INPUT = 1;    // pg 795 (Rev. 3, 799 Rev. 2, 795 Rev. 1)
      // DAISY:1
      // GPIO_B1_11_ALT3

  // RCSR offset 0x17, set RMII_Clock_Select, pg 61
  mdio_write(PHY_RCSR, PHY_RCSR_VALUE);  // Config for 50 MHz clock input

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
    s_txRing[i].extend1 = kEnetTxBdTxInterrupt |
                          kEnetTxBdProtChecksum |
                          kEnetTxBdIpHdrChecksum;
  }
  s_txRing[TX_SIZE - 1].status |= kEnetTxBdWrap;

  ENET_EIMR = 0;  // This also deasserts all interrupts

  ENET_RCR = ENET_RCR_NLC |     // Payload length is checked
             ENET_RCR_MAX_FL(MAX_FRAME_LEN) |
             ENET_RCR_CFEN |    // Discard non-pause MAC control frames
             ENET_RCR_CRCFWD |  // CRC is stripped (ignored if PADEN)
             ENET_RCR_PADEN |   // Padding is removed
             ENET_RCR_RMII_MODE |
             ENET_RCR_FCE |     // Flow control enable
#ifdef QNETHERNET_ENABLE_PROMISCUOUS_MODE
             ENET_RCR_PROM |    // Promiscuous mode
#endif  // QNETHERNET_ENABLE_PROMISCUOUS_MODE
             ENET_RCR_MII_MODE;
  ENET_TCR = ENET_TCR_ADDINS |  // Overwrite with programmed MAC address
             ENET_TCR_ADDSEL(0) |
             // ENET_TCR_RFC_PAUSE |
             // ENET_TCR_TFC_PAUSE |
             ENET_TCR_FDEN;     // Enable full-duplex

  ENET_TACC = 0
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

  ENET_RACC = 0
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

  ENET_TFWR = ENET_TFWR_STRFWD;
  ENET_RSFL = 0;

  ENET_RDSR = (uint32_t)s_rxRing;
  ENET_TDSR = (uint32_t)s_txRing;
  ENET_MRBR = BUF_SIZE;

  ENET_RXIC = 0;
  ENET_TXIC = 0;
  ENET_PALR = (s_mac[0] << 24) | (s_mac[1] << 16) | (s_mac[2] << 8) | s_mac[3];
  ENET_PAUR = (s_mac[4] << 24) | (s_mac[5] << 16) | 0x8808;

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
  ENET_EIR = 0;  // Clear any pending interrupts before setting ETHEREN
  atomic_flag_test_and_set(&s_rxNotAvail);

  // Last, enable the Ethernet MAC
  ENET_ECR = 0x70000000 | ENET_ECR_DBSWP | ENET_ECR_EN1588 | ENET_ECR_ETHEREN;

  // Indicate there are empty RX buffers and available ready TX buffers
  ENET_RDAR = ENET_RDAR_RDAR;
  ENET_TDAR = ENET_TDAR_TDAR;

  // PHY soft reset
  // mdio_write(PHY_BMCR, 1 << 15);

  s_initState = kInitStateInitialized;
}

// Low-level input function that transforms a received frame into an lwIP pbuf.
// This returns a newly-allocated pbuf, or NULL if there was a frame error or
// allocation error.
static struct pbuf *t41_low_level_input(volatile enetbufferdesc_t *pBD) {
  const u16_t err_mask = kEnetRxBdTrunc |
                         kEnetRxBdOverrun |
                         kEnetRxBdCrc |
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
    p = pbuf_alloc(PBUF_RAW, pBD->length, PBUF_POOL);
    if (p) {
#ifndef QNETHERNET_BUFFERS_IN_RAM1
      arm_dcache_delete(pBD->buffer, MULTIPLE_OF_32(p->tot_len));
#endif  // !QNETHERNET_BUFFERS_IN_RAM1
      pbuf_take(p, pBD->buffer, p->tot_len);
      LINK_STATS_INC(link.recv);
    } else {
      LINK_STATS_INC(link.drop);
      LINK_STATS_INC(link.memerr);
    }
  }

  // Set rx bd empty
  pBD->status = (pBD->status & kEnetRxBdWrap) | kEnetRxBdEmpty;

  ENET_RDAR = ENET_RDAR_RDAR;

  return p;
}

// Acquire a buffer descriptor. Meant to be used with update_bufdesc().
static inline volatile enetbufferdesc_t *get_bufdesc() {
  volatile enetbufferdesc_t *pBD = s_pTxBD;

  while (pBD->status & kEnetTxBdReady) {
    // Wait until BD is free
  }

  return pBD;
}

// Update a buffer descriptor. Meant to be used with get_bufdesc().
static inline void update_bufdesc(volatile enetbufferdesc_t *pBD,
                                  uint16_t len) {
  pBD->length = len;
  pBD->status = (pBD->status & kEnetTxBdWrap) |
                kEnetTxBdTransmitCrc |
                kEnetTxBdLast |
                kEnetTxBdReady;

  ENET_TDAR = ENET_TDAR_TDAR;

  if (pBD->status & kEnetTxBdWrap) {
    s_pTxBD = &s_txRing[0];
  } else {
    s_pTxBD++;
  }

  LINK_STATS_INC(link.xmit);
}

static err_t t41_low_level_output(struct netif *netif, struct pbuf *p) {
  LWIP_UNUSED_ARG(netif);
  if (p == NULL) {
    return ERR_ARG;
  }

  // Note: The pbuf already contains the padding (ETH_PAD_SIZE)
  volatile enetbufferdesc_t *pBD = get_bufdesc();
  uint16_t copied = pbuf_copy_partial(p, pBD->buffer, p->tot_len, 0);
  if (copied == 0) {
    LINK_STATS_INC(link.drop);
    return ERR_BUF;
  }
#ifndef QNETHERNET_BUFFERS_IN_RAM1
  arm_dcache_flush_delete(pBD->buffer, MULTIPLE_OF_32(copied));
#endif  // !QNETHERNET_BUFFERS_IN_RAM1
  update_bufdesc(pBD, copied);
  return ERR_OK;
}

static err_t t41_netif_init(struct netif *netif) {
  if (netif == NULL) {
    return ERR_ARG;
  }

  netif->linkoutput = t41_low_level_output;
  netif->output = etharp_output;
  netif->mtu = MTU;
  netif->flags = NETIF_FLAG_BROADCAST |
                 NETIF_FLAG_ETHARP |
#if LWIP_IGMP
                 NETIF_FLAG_IGMP |
#endif  // LWIP_IGMP
                 NETIF_FLAG_ETHERNET;

  SMEMCPY(netif->hwaddr, s_mac, ETH_HWADDR_LEN);
  netif->hwaddr_len = ETH_HWADDR_LEN;
#if LWIP_NETIF_HOSTNAME
  netif_set_hostname(netif, NULL);
#endif  // LWIP_NETIF_HOSTNAME

  t41_low_level_init();

  return ERR_OK;
}

// Find the next non-empty BD.
static inline volatile enetbufferdesc_t *rxbd_next() {
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

static void enet_isr() {
  if ((ENET_EIR & ENET_EIR_RXF) != 0) {
    ENET_EIR = ENET_EIR_RXF;
    atomic_flag_clear(&s_rxNotAvail);
  }
}

static inline void check_link_status() {
  if (s_initState != kInitStateInitialized) {
    return;
  }

  // Note: PHY_PHYSTS doesn't seem to contain the live link information unless
  //       BMSR is read too
  uint16_t status = mdio_read(PHY_BMSR);
  uint8_t is_link_up = ((status & PHY_BMSR_LINK_STATUS) != 0);

  if (netif_is_link_up(&s_t41_netif) != is_link_up) {
    if (is_link_up) {
      status = mdio_read(PHY_PHYSTS);
      s_linkSpeed10Not100 = ((status & PHY_PHYSTS_SPEED_STATUS) != 0);
      s_linkIsFullDuplex  = ((status & PHY_PHYSTS_DUPLEX_STATUS) != 0);
      s_linkIsCrossover   = ((status & PHY_PHYSTS_MDI_MDIX_MODE) != 0);

      netif_set_link_up(&s_t41_netif);
    } else {
      netif_set_link_down(&s_t41_netif);
    }
  }
}

#if LWIP_IGMP && !defined(QNETHERNET_ENABLE_PROMISCUOUS_MODE)
// Multicast filter for letting the hardware know which packets to let in.
static err_t multicast_filter(struct netif *netif, const ip4_addr_t *group,
                              enum netif_mac_filter_action action) {
  LWIP_UNUSED_ARG(netif);

  bool retval = true;
  switch (action) {
    case NETIF_ADD_MAC_FILTER:
      retval = enet_join_group(group);
      break;
    case NETIF_DEL_MAC_FILTER:
      retval = enet_leave_group(group);
      break;
    default:
      break;
  }
  return retval ? ERR_OK : ERR_USE;
      // ERR_USE seems like the best fit of the choices
      // Next best seems to be ERR_IF
}
#endif  // LWIP_IGMP && !defined(QNETHERNET_ENABLE_PROMISCUOUS_MODE)

// --------------------------------------------------------------------------
//  Public Interface
// --------------------------------------------------------------------------

bool enet_is_unknown() {
  return s_initState == kInitStateStart;
}

void enet_get_mac(uint8_t *mac) {
  if (mac == NULL) {
    return;
  }

  uint32_t m1 = HW_OCOTP_MAC1;
  uint32_t m2 = HW_OCOTP_MAC0;
  mac[0] = m1 >> 8;
  mac[1] = m1 >> 0;
  mac[2] = m2 >> 24;
  mac[3] = m2 >> 16;
  mac[4] = m2 >> 8;
  mac[5] = m2 >> 0;
}

NETIF_DECLARE_EXT_CALLBACK(netif_callback)/*;*/

bool enet_has_hardware() {
  switch (s_initState) {
    case kInitStateHasHardware:
      return true;
    case kInitStateNoHardware:
      return false;
    default:
      break;
  }
  t41_init_phy();
  return (s_initState != kInitStateNoHardware);
}

bool enet_init(const uint8_t mac[ETH_HWADDR_LEN],
               const ip4_addr_t *ipaddr,
               const ip4_addr_t *netmask,
               const ip4_addr_t *gw,
               netif_ext_callback_fn callback) {
  // Only execute the following code once
  static bool isFirstInit = true;
  if (isFirstInit) {
    lwip_init();
    isFirstInit = false;
  }

  if (ipaddr == NULL)  ipaddr  = IP4_ADDR_ANY4;
  if (netmask == NULL) netmask = IP4_ADDR_ANY4;
  if (gw == NULL)      gw      = IP4_ADDR_ANY4;

  // First test if the MAC address has changed
  // If it's changed then remove the interface and start again
  uint8_t m[ETH_HWADDR_LEN];
  if (mac == NULL) {
    enet_get_mac(m);
    mac = m;
  }
  if (memcmp(s_mac, mac, ETH_HWADDR_LEN)) {
    // MAC address has changed

    if (s_isNetifAdded) {
      // Remove any previous configuration
      netif_remove(&s_t41_netif);
      netif_remove_ext_callback(&netif_callback);
      s_isNetifAdded = false;
    }
    SMEMCPY(s_mac, mac, ETH_HWADDR_LEN);
  }

  if (s_isNetifAdded) {
    netif_set_addr(&s_t41_netif, ipaddr, netmask, gw);
  } else {
    netif_add_ext_callback(&netif_callback, callback);
    if (netif_add(&s_t41_netif, ipaddr, netmask, gw,
                  NULL, t41_netif_init, ethernet_input) == NULL) {
      netif_remove_ext_callback(&netif_callback);
      return false;
    }
    netif_set_default(&s_t41_netif);
    s_isNetifAdded = true;

#if LWIP_DHCP
    // netif_add() clears these
    dhcp_set_struct(&s_t41_netif, &s_dhcp);
#endif  // LWIP_DHCP
#if LWIP_AUTOIP
    autoip_set_struct(&s_t41_netif, &s_autoip);
#endif  // LWIP_AUTOIP
  }

#if LWIP_IGMP && !defined(QNETHERNET_ENABLE_PROMISCUOUS_MODE)
  // Multicast filtering, to allow desired multicast packets in
  netif_set_igmp_mac_filter(&s_t41_netif, &multicast_filter);
#endif  // LWIP_IGMP && !defined(QNETHERNET_ENABLE_PROMISCUOUS_MODE)

  return true;
}

extern void unused_interrupt_vector(void);  // startup.c

void enet_deinit() {
  netif_set_addr(&s_t41_netif, IP4_ADDR_ANY4, IP4_ADDR_ANY4, IP4_ADDR_ANY4);
  netif_set_link_down(&s_t41_netif);
  netif_set_down(&s_t41_netif);

  if (s_isNetifAdded) {
    netif_set_default(NULL);
    netif_remove(&s_t41_netif);
    netif_remove_ext_callback(&netif_callback);
    s_isNetifAdded = false;
  }

  if (s_initState == kInitStateInitialized) {
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
    ENET_ECR &= ~ENET_ECR_ETHEREN;

    s_initState = kInitStatePHYInitialized;
  }

  if (s_initState == kInitStatePHYInitialized) {
    // Power down the PHY
    GPIO7_DR_CLEAR = (1 << 15);

    // Stop the PLL (first bypassing)
    CCM_ANALOG_PLL_ENET_SET = CCM_ANALOG_PLL_ENET_BYPASS;
    CCM_ANALOG_PLL_ENET_SET = CCM_ANALOG_PLL_ENET_POWERDOWN;

    // Disable the clock for ENET
    CCM_CCGR1 &= ~CCM_CCGR1_ENET(CCM_CCGR_ON);

    s_initState = kInitStateHasHardware;
  }
}

struct netif *enet_netif() {
  return &s_t41_netif;
}

void enet_proc_input(void) {
  if (atomic_flag_test_and_set(&s_rxNotAvail)) {
    return;
  }

  for (int i = RX_SIZE*2; --i >= 0; ) {
    // Get the next chunk of input data
    volatile enetbufferdesc_t *p_bd = rxbd_next();
    if (p_bd == NULL) {
      break;
    }
    struct pbuf *p = t41_low_level_input(p_bd);
    if (p != NULL) {  // Happens on frame error or pbuf allocation error
      // Process one chunk of input data
      if (s_t41_netif.input(p, &s_t41_netif) != ERR_OK) {
        pbuf_free(p);
      }
    }
  }
}

void enet_poll() {
  sys_check_timeouts();
  check_link_status();
}

int phy_link_speed() {
  return s_linkSpeed10Not100 ? 10 : 100;
}

bool phy_link_is_full_duplex() {
  return s_linkIsFullDuplex;
}

bool phy_link_is_crossover() {
  return s_linkIsCrossover;
}

bool enet_output_frame(const uint8_t *frame, size_t len) {
  if (s_initState != kInitStateInitialized) {
    return false;
  }
  if (frame == NULL || len < (6 + 6 + 2)) {  // dst + src + len/type
    return false;
  }

  // Check length depending on VLAN
  if (frame[12] == (uint8_t)(ETHTYPE_VLAN >> 8) &&
      frame[13] == (uint8_t)(ETHTYPE_VLAN)) {
    if (len < (6 + 6 + 2 + 2 + 2)) {  // dst + src + VLAN tag + VLAN info + len/type
      return false;
    }
    if (len > MAX_FRAME_LEN - 4) {  // Don't include 4-byte FCS
      return false;
    }
  } else {
    if (len > MAX_FRAME_LEN - 4 - 4) {  // Don't include 4-byte FCS and VLAN
      return false;
    }
  }

  volatile enetbufferdesc_t *pBD = get_bufdesc();
#if ETH_PAD_SIZE
  memcpy((uint8_t *)pBD->buffer + ETH_PAD_SIZE, frame, len);
#ifndef QNETHERNET_BUFFERS_IN_RAM1
  arm_dcache_flush_delete(pBD->buffer, MULTIPLE_OF_32(len + ETH_PAD_SIZE));
#endif  // !QNETHERNET_BUFFERS_IN_RAM1
  update_bufdesc(pBD, len + ETH_PAD_SIZE);
#else
  memcpy(pBD->buffer, frame, len);
#ifndef QNETHERNET_BUFFERS_IN_RAM1
  arm_dcache_flush_delete(pBD->buffer, MULTIPLE_OF_32(len));
#endif  // !QNETHERNET_BUFFERS_IN_RAM1
  update_bufdesc(pBD, len);
#endif  // ETH_PAD_SIZE
  return true;
}

// --------------------------------------------------------------------------
//  MAC Address Filtering
// --------------------------------------------------------------------------

#ifndef QNETHERNET_ENABLE_PROMISCUOUS_MODE

// CRC-32 routine for computing the 4-byte FCS for multicast lookup.
static uint32_t crc32(uint32_t crc, const uint8_t *data, size_t len) {
  // https://create.stephan-brumme.com/crc32/#fastest-bitwise-crc32
  crc = ~crc;
  while (len--) {
    crc ^= *(data++);
    for (int j = 0; j < 8; j++) {
      crc = (crc >> 1) ^ (-(crc & 0x01) & 0xEDB88320);
    }
  }
  return crc;
}

bool enet_set_mac_address_allowed(const uint8_t *mac, bool allow) {
  if (mac == NULL) {
    return false;
  }

  // Don't release bits that have had a collision. Track these here.
  static uint32_t collisionGALR = 0;
  static uint32_t collisionGAUR = 0;
  static uint32_t collisionIALR = 0;
  static uint32_t collisionIAUR = 0;

  volatile uint32_t *lower;
  volatile uint32_t *upper;
  uint32_t *collisionLower;
  uint32_t *collisionUpper;
  if ((mac[0] & 0x01) != 0) {  // Group
    lower = &ENET_GALR;
    upper = &ENET_GAUR;
    collisionLower = &collisionGALR;
    collisionUpper = &collisionGAUR;
  } else {  // Individual
    lower = &ENET_IALR;
    upper = &ENET_IAUR;
    collisionLower = &collisionIALR;
    collisionUpper = &collisionIAUR;
  }

  uint32_t crc = (crc32(0, mac, 6) >> 26) & 0x3f;
  uint32_t value = 1 << (crc & 0x1f);
  if (crc < 0x20) {
    if (allow) {
      if ((*lower & value) != 0) {
        *collisionLower |= value;
      } else {
        *lower |= value;
      }
    } else {
      // Keep collided bits set
      *lower &= ~value | *collisionLower;
      return ((*collisionLower & value) == 0);  // False if can't remove
    }
  } else {
    if (allow) {
      if ((*upper & value) != 0) {
        *collisionUpper |= value;
      } else {
        *upper |= value;
      }
    } else {
      // Keep collided bits set
      *upper &= ~value | *collisionUpper;
      return ((*collisionUpper & value) == 0);  // False if can't remove
    }
  }

  return true;
}

// Joins or leaves a multicast group. The flag should be true to join and false
// to leave. This returns whether successful.
static bool enet_join_notleave_group(const ip4_addr_t *group, bool flag) {
  if (group == NULL) {
    return false;
  }

  // Multicast MAC address.
  static uint8_t multicastMAC[6] = {
      LL_IP4_MULTICAST_ADDR_0,
      LL_IP4_MULTICAST_ADDR_1,
      LL_IP4_MULTICAST_ADDR_2,
      0,
      0,
      0,
  };

  multicastMAC[3] = ip4_addr2(group) & 0x7f;
  multicastMAC[4] = ip4_addr3(group);
  multicastMAC[5] = ip4_addr4(group);

  return enet_set_mac_address_allowed(multicastMAC, flag);
}

bool enet_join_group(const ip4_addr_t *group) {
  return enet_join_notleave_group(group, true);
}

bool enet_leave_group(const ip4_addr_t *group) {
  return enet_join_notleave_group(group, false);
}

#endif  // !QNETHERNET_ENABLE_PROMISCUOUS_MODE

#endif  // ARDUINO_TEENSY41
