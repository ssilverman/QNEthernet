// Ethernet IF implementation.
// (c) 2021 Shawn Silverman

#include <stdint.h>
#incldue <string.h>

#include <core_pins.h>
#include <lwip/etharp.h>
#include <lwip/netif.h>
#include <lwip/prot/ethernet.h>

/* Define those to better describe your network interface. */
#define IFNAME0 'e'
#define IFNAME1 'n'

// https://forum.pjrc.com/threads/60532-Teensy-4-1-Beta-Test?p=237096&viewfull=1#post237096
// https://github.com/PaulStoffregen/teensy41_ethernet/blob/master/teensy41_ethernet.ino

#define EXTDESC

typedef struct {
	uint16_t length;
	uint16_t flags;
	void *buffer;
#ifdef EXTDESC
	uint32_t moreflags;
	uint16_t checksum;
	uint16_t header;
	uint32_t dmadone;
	uint32_t timestamp;
	uint32_t unused1;
	uint32_t unused2;
#endif
} enetbufferdesc_t;

// #define RXSIZE 12
// #define TXSIZE 10
#define RXSIZE 4
#define TXSIZE 2
#define BUFSIZE 1518
static enetbufferdesc_t rx_ring[RXSIZE] __attribute__((aligned(64)));
static enetbufferdesc_t tx_ring[TXSIZE] __attribute__((aligned(64)));
uint32_t rxbufs[RXSIZE * BUFSIZE] __attribute__((aligned(32)));
uint32_t txbufs[TXSIZE * BUFSIZE] __attribute__((aligned(32)));

#define CLRSET(reg, clear, set) ((reg) = ((reg) & ~(clear)) | (set))
#define RMII_PAD_INPUT_PULLDOWN 0x30E9
#define RMII_PAD_INPUT_PULLUP   0xB0E9
#define RMII_PAD_CLOCK          0x0031

static void low_level_init(struct netif *netif) {
  // struct ethernetif *ethernetif = netif->state;

  // Set MAC hardware address length
  netif->hwaddr_len = 6;//ETHARP_HWADDR_LEN;

  // Set MAC hardware address
  netif->hwaddr[0] = HW_OCOTP_MAC1 >> 8;
  netif->hwaddr[1] = HW_OCOTP_MAC1;
  netif->hwaddr[2] = HW_OCOTP_MAC0 >> 24;
  netif->hwaddr[3] = HW_OCOTP_MAC0 >> 16;
  netif->hwaddr[4] = HW_OCOTP_MAC0 >> 8;
  netif->hwaddr[5] = HW_OCOTP_MAC0;

  // Maximum transfer unit
  netif->mtu = 1500;

  // Device capabilities
  // Don't set NETIF_FLAG_ETHARP if this device is not an ethernet one
  netif->flags = NETIF_FLAG_BROADCAST | NETIF_FLAG_ETHARP | NETIF_FLAG_LINK_UP;

#if LWIP_IPV6 && LWIP_IPV6_MLD
  // For hardware/netifs that implement MAC filtering.
  // All-nodes link-local is handled by default, so we must let the hardware
  // know to allow multicast packets in. Should set mld_mac_filter previously.
  if (netif->mld_mac_filter != NULL) {
    ip6_addr_t ip6_allnodes_ll;
    ip6_addr_set_allnodes_linklocal(&ip6_allnodes_ll);
    netif->mld_mac_filter(netif, &ip6_allnodes_ll, NETIF_ADD_MAC_FILTER);
  }
#endif  // LWIP_IPV6 && LWIP_IPV6_MLD

  // Do whatever else is needed to initialize interface.

  CCM_CCGR1 |= CCM_CCGR1_ENET(CCM_CCGR_ON);
  // configure PLL6 for 50 MHz, pg 1118 (Rev. 2, 1173 Rev. 1)
  CCM_ANALOG_PLL_ENET_CLR = CCM_ANALOG_PLL_ENET_POWERDOWN |
                            CCM_ANALOG_PLL_ENET_BYPASS |
                            0x0F;
  CCM_ANALOG_PLL_ENET_SET = CCM_ANALOG_PLL_ENET_ENABLE |
                            CCM_ANALOG_PLL_ENET_BYPASS |
                            // CCM_ANALOG_PLL_ENET_ENET2_REF_EN |
                            CCM_ANALOG_PLL_ENET_ENET_25M_REF_EN |
                            // CCM_ANALOG_PLL_ENET_ENET2_DIV_SELECT(1) |
                            CCM_ANALOG_PLL_ENET_DIV_SELECT(1);
  while (!(CCM_ANALOG_PLL_ENET & CCM_ANALOG_PLL_ENET_LOCK)) {
    // wait for PLL lock
  }
  CCM_ANALOG_PLL_ENET_CLR = CCM_ANALOG_PLL_ENET_BYPASS;
  // Serial.printf("PLL6 = %08X (should be 80202001)\n", CCM_ANALOG_PLL_ENET);

	// configure REFCLK to be driven as output by PLL6, pg 329 (Rev. 2, 326 Rev. 1)
#if 1
  CLRSET(IOMUXC_GPR_GPR1,
         IOMUXC_GPR_GPR1_ENET1_CLK_SEL | IOMUXC_GPR_GPR1_ENET_IPG_CLK_S_EN,
         IOMUXC_GPR_GPR1_ENET1_TX_CLK_DIR);
#else
  //IOMUXC_GPR_GPR1 &= ~IOMUXC_GPR_GPR1_ENET1_TX_CLK_DIR; // do not use
	IOMUXC_GPR_GPR1 |= IOMUXC_GPR_GPR1_ENET1_TX_CLK_DIR; // 50 MHz REFCLK
	IOMUXC_GPR_GPR1 &= ~IOMUXC_GPR_GPR1_ENET_IPG_CLK_S_EN;
	//IOMUXC_GPR_GPR1 |= IOMUXC_GPR_GPR1_ENET_IPG_CLK_S_EN; // clock always on
	IOMUXC_GPR_GPR1 &= ~IOMUXC_GPR_GPR1_ENET1_CLK_SEL;
	////IOMUXC_GPR_GPR1 |= IOMUXC_GPR_GPR1_ENET1_CLK_SEL;
#endif
  // Serial.printf("GPR1 = %08X\n", IOMUXC_GPR_GPR1);


  // Configure pins
	IOMUXC_SW_MUX_CTL_PAD_GPIO_B0_14 = 5; // Reset   B0_14 Alt5 GPIO7.15
	IOMUXC_SW_MUX_CTL_PAD_GPIO_B0_15 = 5; // Power   B0_15 Alt5 GPIO7.14
	GPIO7_GDIR |= (1<<14) | (1<<15);
	GPIO7_DR_SET = (1<<15);   // power on
	GPIO7_DR_CLEAR = (1<<14); // reset PHY chip
	IOMUXC_SW_PAD_CTL_PAD_GPIO_B1_04 = RMII_PAD_INPUT_PULLDOWN; // PhyAdd[0] = 0
	IOMUXC_SW_PAD_CTL_PAD_GPIO_B1_06 = RMII_PAD_INPUT_PULLDOWN; // PhyAdd[1] = 1
	IOMUXC_SW_PAD_CTL_PAD_GPIO_B1_05 = RMII_PAD_INPUT_PULLUP;   // Master/Slave = slave mode
	IOMUXC_SW_PAD_CTL_PAD_GPIO_B1_11 = RMII_PAD_INPUT_PULLDOWN; // Auto MDIX Enable
	IOMUXC_SW_PAD_CTL_PAD_GPIO_B1_07 = RMII_PAD_INPUT_PULLUP;
	IOMUXC_SW_PAD_CTL_PAD_GPIO_B1_08 = RMII_PAD_INPUT_PULLUP;
	IOMUXC_SW_PAD_CTL_PAD_GPIO_B1_09 = RMII_PAD_INPUT_PULLUP;
	IOMUXC_SW_PAD_CTL_PAD_GPIO_B1_10 = RMII_PAD_CLOCK;
	IOMUXC_SW_MUX_CTL_PAD_GPIO_B1_05 = 3; // RXD1    B1_05 Alt3, pg 529 (Rev. 2, 525 Rev. 1)
	IOMUXC_SW_MUX_CTL_PAD_GPIO_B1_04 = 3; // RXD0    B1_04 Alt3, pg 528 (Rev. 2, 524 Rev. 1)
	IOMUXC_SW_MUX_CTL_PAD_GPIO_B1_10 = 6 | 0x10; // REFCLK  B1_10 Alt6, pg 534 (Rev. 2, 530 Rev. 1)
	IOMUXC_SW_MUX_CTL_PAD_GPIO_B1_11 = 3; // RXER    B1_11 Alt3, pg 535 (Rev. 2, 531 Rev. 1)
	IOMUXC_SW_MUX_CTL_PAD_GPIO_B1_06 = 3; // RXEN    B1_06 Alt3, pg 530 (Rev. 2, 526 Rev. 1)
	IOMUXC_SW_MUX_CTL_PAD_GPIO_B1_09 = 3; // TXEN    B1_09 Alt3, pg 533 (Rev. 2, 529 Rev. 1)
	IOMUXC_SW_MUX_CTL_PAD_GPIO_B1_07 = 3; // TXD0    B1_07 Alt3, pg 531 (Rev. 2, 527 Rev. 1)
	IOMUXC_SW_MUX_CTL_PAD_GPIO_B1_08 = 3; // TXD1    B1_08 Alt3, pg 532 (Rev. 2, 528 Rev. 1)
	IOMUXC_SW_MUX_CTL_PAD_GPIO_B1_15 = 0; // MDIO    B1_15 Alt0, pg 539 (Rev. 2, 535 Rev. 1)
	IOMUXC_SW_MUX_CTL_PAD_GPIO_B1_14 = 0; // MDC     B1_14 Alt0, pg 538 (Rev. 2, 534 Rev. 1)
	IOMUXC_ENET_MDIO_SELECT_INPUT = 2; // GPIO_B1_15_ALT0, pg 796 (Rev. 2, 792 Rev. 1)
	IOMUXC_ENET0_RXDATA_SELECT_INPUT = 1; // GPIO_B1_04_ALT3, pg 796 (Rev. 2, 792 Rev. 1)
	IOMUXC_ENET1_RXDATA_SELECT_INPUT = 1; // GPIO_B1_05_ALT3, pg 797 (Rev. 2, 793 Rev. 1)
	IOMUXC_ENET_RXEN_SELECT_INPUT = 1; // GPIO_B1_06_ALT3, pg 798 (Rev. 2, 794 Rev. 1)
	IOMUXC_ENET_RXERR_SELECT_INPUT = 1; // GPIO_B1_11_ALT3, pg 799 (Rev. 2, 795 Rev. 1)
	IOMUXC_ENET_IPG_CLK_RMII_SELECT_INPUT = 1; // GPIO_B1_10_ALT6, pg 795 (Rev. 2, 791 Rev. 1)
	delayMicroseconds(2);
	GPIO7_DR_SET = (1<<14); // start PHY chip
	ENET_MSCR = ENET_MSCR_MII_SPEED(9);
	delayMicroseconds(5);

#if 0
	while (1) {
		mdio_write(0, 0x18, 0x492); // force LED on
		delay(500);
		mdio_write(0, 0x18, 0x490); // force LED off
		delay(500);
	}
#endif

  // Serial.printf("RCSR:%04X, LEDCR:%04X, PHYCR %04X\n",
  //               mdio_read(0, 0x17), mdio_read(0, 0x18), mdio_read(0, 0x19));


  // LEDCR offset 0x18, set LED_Link_Polarity, pg 62
	mdio_write(0, 0x18, 0x0280); // LED shows link status, active high
	// RCSR offset 0x17, set RMII_Clock_Select, pg 61
	mdio_write(0, 0x17, 0x0081); // config for 50 MHz clock input

  // Serial.printf("RCSR:%04X, LEDCR:%04X, PHYCR %04X\n",
  //               mdio_read(0, 0x17), mdio_read(0, 0x18), mdio_read(0, 0x19));

  // ENET_EIR     2174  Interrupt Event Register
  // ENET_EIMR    2177  Interrupt Mask Register
  // ENET_RDAR    2180  Receive Descriptor Active Register
  // ENET_TDAR    2181  Transmit Descriptor Active Register
  // ENET_ECR     2181  Ethernet Control Register
  // ENET_RCR     2187  Receive Control Register
  // ENET_TCR     2190  Transmit Control Register
  // ENET_PALR/UR 2192  Physical Address
  // ENET_RDSR    2199  Receive Descriptor Ring Start
  // ENET_TDSR    2199  Transmit Buffer Descriptor Ring
  // ENET_MRBR    2200  Maximum Receive Buffer Size
  //              2278  receive buffer descriptor
  //              2281  transmit buffer descriptor

  // print("enetbufferdesc_t size = ", sizeof(enetbufferdesc_t));
	// print("rx_ring size = ", sizeof(rx_ring));
	memset(rx_ring, 0, sizeof(rx_ring));
	memset(tx_ring, 0, sizeof(tx_ring));

  for (int i=0; i < RXSIZE; i++) {
    rx_ring[i].flags = 0x8000; // empty flag
#ifdef EXTDESC
    rx_ring[i].moreflags = 0x00800000; // INT flag
#endif
    rx_ring[i].buffer = rxbufs + i*128;
  }
  rx_ring[RXSIZE - 1].flags = 0xA000; // empty & wrap flags
  for (int i=0; i < TXSIZE; i++) {
    tx_ring[i].buffer = txbufs + i*128;
#ifdef EXTDESC
    tx_ring[i].moreflags = 0x40000000; // INT flag
#endif
  }
  tx_ring[TXSIZE - 1].flags = 0x2000; // wrap flag

	//ENET_ECR |= ENET_ECR_RESET;

  ENET_EIMR = 0;
  ENET_MSCR = ENET_MSCR_MII_SPEED(9);  // 12 is fastest which seems to work
#if 1
  ENET_RCR = ENET_RCR_NLC |
             ENET_RCR_MAX_FL(1522) |
             // ENET_RCR_CFEN |
             ENET_RCR_CRCFWD |
             ENET_RCR_PADEN |
             ENET_RCR_RMII_MODE |
             ///* ENET_RCR_FCE | ENET_RCR_PROM | */ ENET_RCR_MII_MODE;
             ENET_RCR_PROM |
             ENET_RCR_MII_MODE;
  ENET_TCR = ENET_TCR_ADDINS |
             // ENET_TCR_RFC_PAUSE | ENET_TCR_TFC_PAUSE |
             ENET_TCR_FDEN;
#else
  ENET_RCR = ENET_RCR_MAX_FL(1518) | ENET_RCR_RMII_MODE | ENET_RCR_MII_MODE |
             ENET_RCR_PROM;
  ENET_TCR = ENET_TCR_FDEN;
#endif
  ENET_RXIC = 0;
  ENET_TXIC = 0;

  ENET_PALR = (HW_OCOTP_MAC1 << 16) | (HW_OCOTP_MAC0 >> 16);
  ENET_PALU = (HW_OCOTP_MAC0 << 16) | 0x8808;
  ENET_OPD = 0x10014;
  ENET_IAUR = 0;
  ENET_IALR = 0;
  ENET_GAUR = 0;
  ENET_GALR = 0;
  ENET_RDSR = (uint32_t)rx_ring;
  ENET_TDSR = (uint32_t)tx_ring;
  ENET_MRBR = 512;
  ENET_TACC = ENET_TACC_SHIFT16;
  // ENET_TACC = ENET_TACC_SHIFT16 | ENET_TACC_IPCHK | ENET_TACC_PROCHK;
  ENET_RACC = ENET_RACC_SHIFT16;

  // ENET_RSEM = 0;
  // ENET_RAEM = 16;
  // ENET_RAFL = 16;
  // ENET_TSEM = 16;
  // ENET_TAEM = 16;

	ENET_MIBC = 0;
  // Serial.printf("MIBC=%08X\n", ENET_MIBC);
  // Serial.printf("ECR=%08X\n", ENET_ECR);
  // ENET_ECR = 0x70000000 | ENET_ECR_DBSWP | ENET_ECR_EN1588 | ENET_ECR_ETHEREN;
#ifdef EXTDESC
  ENET_ECR |= ENET_ECR_DBSWP | ENET_ECR_EN1588 | ENET_ECR_ETHEREN;
#else
  ENET_ECR |= ENET_ECR_DBSWP | ENET_ECR_ETHEREN;
#endif
  //ENET_ECR = 0xF0000000 | ENET_ECR_DBSWP | ENET_ECR_EN1588 | ENET_ECR_ETHEREN;
  // Serial.printf("ECR=%08X\n", ENET_ECR);
  ENET_RDAR = ENET_RDAR_RDAR;
  ENET_TDAR = ENET_TDAR_TDAR;

	// printhex("MDIO PHY ID2 (LAN8720A is 0007, DP83825I is 2000): ",
  //          mdio_read(0, 2));
	// printhex("MDIO PHY ID3 (LAN8720A is C0F?, DP83825I is A140): ",
  //          mdio_read(0, 3));
  // // TODO: Is the following delay needed?
	// delay(2500);
	// printhex("BMCR: ", mdio_read(0, 0));
	// printhex("BMSR: ", mdio_read(0, 1));
}

/**
 * This function should do the actual transmission of the packet. The packet is
 * contained in the pbuf that is passed to the function. This pbuf
 * might be chained.
 *
 * @param netif the lwip network interface structure for this ethernetif
 * @param p the MAC packet to send (e.g. IP packet including MAC addresses and type)
 * @return ERR_OK if the packet could be sent
 *         an err_t value if the packet couldn't be sent
 *
 * @note Returning ERR_MEM here if a DMA queue of your MAC is full can lead to
 *       strange results. You might consider waiting for space in the DMA queue
 *       to become available since the stack doesn't retry to send a packet
 *       dropped because of memory failure (except for the TCP timers).
 */
static err_t low_level_output(struct netif *netif, struct pbuf *p) {
  struct ethernetif *ethernetif = netif->state;
  struct pbuf *q;

  static int txnum = 0;
  volatile enetbufferdesc_t *buf;
  uint16_t flags;

  // initiate transfer();
  buf = tx_ring + txnum;

#if ETH_PAD_SIZE
  pbuf_remove_header(p, ETH_PAD_SIZE); /* drop the padding word */
#endif

  if (p->tot_len > BUFSIZE) {  // TODO: Is this check needed?
    return ERR_BUF;  // TODO: Is this error correct?
  }

  // Wait for a free TX descriptor
  while ((buf->flags & 0x8000) != 0) {
    // Wait
    // TODO: Delay here?
  }
  buf->length = p->tot_len;
  void *buffer = buf->buffer;

  for (q = p; q != NULL; q = q->next) {
    // Send the data from the pbuf to the interface, one pbuf at a
    // time. The size of the data in each pbuf is kept in the ->len
    // variable.

    // send data from(q->payload, q->len);
    memcpy(buffer, q->payload, q->len);
    buffer += q->len;
  }

  // signal that packet should be sent();
  buf->flags |= 0x8C00;
  ENET_TDAR = ENET_TDAR_TDAR;
  if (txnum < TXSIZE - 1) {
    txnum++;
  } else {
    txnum = 0;
  }

  MIB2_STATS_NETIF_ADD(netif, ifoutoctets, p->tot_len);
  if (((u8_t *)p->payload)[0] & 1) {
    /* broadcast or multicast packet*/
    MIB2_STATS_NETIF_INC(netif, ifoutnucastpkts);
  } else {
    /* unicast packet */
    MIB2_STATS_NETIF_INC(netif, ifoutucastpkts);
  }
  /* increase ifoutdiscards or ifouterrors on error */

#if ETH_PAD_SIZE
  pbuf_add_header(p, ETH_PAD_SIZE); /* reclaim the padding word */
#endif

  LINK_STATS_INC(link.xmit);

  return ERR_OK;
}

/**
 * Should allocate a pbuf and transfer the bytes of the incoming
 * packet from the interface into the pbuf.
 *
 * @param netif the lwip network interface structure for this ethernetif
 * @return a pbuf filled with the received packet (including MAC header)
 *         NULL on memory error
 */
static struct pbuf *low_level_input(struct netif *netif) {
  // struct ethernetif *ethernetif = netif->state;
  struct pbuf *p, *q;
  u16_t len;

  static int rxnum = 0;
  volatile enetbufferdesc_t *buf;

  buf = rx_ring + rxnum;

  if ((buf->flags & 0x8000) != 0) {
    return NULL;
  }

  // Obtain the size of the packet and put it into the "len"
  // variable.
  len = buf->length;

#if ETH_PAD_SIZE
  len += ETH_PAD_SIZE; /* allow room for Ethernet padding */
#endif

  // We allocate a pbuf chain of pbufs from the pool.
  p = pbuf_alloc(PBUF_RAW, len, PBUF_POOL);
  void *buffer = buf->buffer;

  if (p != NULL) {
#if ETH_PAD_SIZE
    pbuf_remove_header(p, ETH_PAD_SIZE); /* drop the padding word */
#endif

    // We iterate over the pbuf chain until we have read the entire
    // packet into the pbuf.
    for (q = p; q != NULL; q = q->next) {
      // Read enough bytes to fill this pbuf in the chain. The
      // available data in the pbuf is given by the q->len
      // variable.
      // This does not necessarily have to be a memcpy, you can also preallocate
      // pbufs for a DMA-enabled MAC and after receiving truncate it to the
      // actually received size. In this case, ensure the tot_len member of the
      // pbuf is the sum of the chained pbuf len members.
      // read data into(q->payload, q->len);
      memcpy(q->payload, buffer, q->len);
      buffer += q->len;
    }

    //acknowledge that packet has been read();
    if (rxnum < RXSIZE - 1) {
      buf->flags = 0x8000;
      rxnum++;
    } else {
      buf->flags = 0xA000;
      rxnum = 0;
    }

    MIB2_STATS_NETIF_ADD(netif, ifinoctets, p->tot_len);
    if (((u8_t *)p->payload)[0] & 1) {
      /* broadcast or multicast packet*/
      MIB2_STATS_NETIF_INC(netif, ifinnucastpkts);
    } else {
      /* unicast packet*/
      MIB2_STATS_NETIF_INC(netif, ifinucastpkts);
    }
#if ETH_PAD_SIZE
    pbuf_add_header(p, ETH_PAD_SIZE); /* reclaim the padding word */
#endif

    LINK_STATS_INC(link.recv);
  } else {
    //drop packet();
    if (rxnum < RXSIZE - 1) {
      buf->flags = 0x8000;
      rxnum++;
    } else {
      buf->flags = 0xA000;
      rxnum = 0;
    }

    LINK_STATS_INC(link.memerr);
    LINK_STATS_INC(link.drop);
    MIB2_STATS_NETIF_INC(netif, ifindiscards);
  }

  return p;
}

// read a PHY register (using MDIO & MDC signals)
uint16_t mdio_read(int phyaddr, int regaddr) {
  ENET_MMFR = ENET_MMFR_ST(1) |
              ENET_MMFR_OP(2) |
              ENET_MMFR_TA(0) |
              ENET_MMFR_PA(phyaddr) |
              ENET_MMFR_RA(regaddr);
  // TODO: what is the proper value for ENET_MMFR_TA ???
  // int count=0;
  while ((ENET_EIR & ENET_EIR_MII) == 0) {
    // count++; // wait
  }
  // print("mdio read waited ", count);
  uint16_t data = ENET_MMFR;
  ENET_EIR = ENET_EIR_MII;
  // printhex("mdio read:", data);
  return data;
}

// write a PHY register (using MDIO & MDC signals)
void mdio_write(int phyaddr, int regaddr, uint16_t data) {
  ENET_MMFR = ENET_MMFR_ST(1) |
              ENET_MMFR_OP(1) |
              ENET_MMFR_TA(0) |
              ENET_MMFR_PA(phyaddr) |
              ENET_MMFR_RA(regaddr) |
              ENET_MMFR_DATA(data);
  // TODO: what is the proper value for ENET_MMFR_TA ???
  // int count = 0;
  while ((ENET_EIR & ENET_EIR_MII) == 0) {
    // count++;  // wait
  }
  ENET_EIR = ENET_EIR_MII;
  // print("mdio write waited ", count);
  // printhex("mdio write :", data);
}

/**
 * This function should be called when a packet is ready to be read
 * from the interface. It uses the function low_level_input() that
 * should handle the actual reception of bytes from the network
 * interface. Then the type of the received packet is determined and
 * the appropriate input function is called.
 *
 * @param netif the lwip network interface structure for this ethernetif
 */
static void ethernetif_input(struct netif *netif) {
  // struct ethernetif *ethernetif;
  struct eth_hdr *ethhdr;
  struct pbuf *p;

  // ethernetif = netif->state;

  do {
    // move received packet into a new pbuf
    p = low_level_input(netif);
    // if no packet could be read, silently ignore this
    if (p != NULL) {
      /* pass all packets to ethernet_input, which decides what packets it
      * supports */
      if (netif->input(p, netif) != ERR_OK) {
        LWIP_DEBUGF(NETIF_DEBUG, ("ethernetif_input: IP input error\n"));
        pbuf_free(p);
        p = NULL;
      }
    }
  } while (p != NULL);
}

/**
 * Should be called at the beginning of the program to set up the
 * network interface. It calls the function low_level_init() to do the
 * actual setup of the hardware.
 *
 * This function should be passed as a parameter to netif_add().
 *
 * @param netif the lwip network interface structure for this ethernetif
 * @return ERR_OK if the loopif is initialized
 *         ERR_MEM if private data couldn't be allocated
 *         any other err_t on error
 */
err_t ethernetif_init(struct netif *netif) {
  // struct ethernetif *ethernetif;

  LWIP_ASSERT("netif != NULL", (netif != NULL));

  // ethernetif = mem_malloc(sizeof(struct ethernetif));
  // if (ethernetif == NULL) {
  //   LWIP_DEBUGF(NETIF_DEBUG, ("ethernetif_init: out of memory\n"));
  //   return ERR_MEM;
  // }

#if LWIP_NETIF_HOSTNAME
  /* Initialize interface hostname */
  netif->hostname = "lwip";
#endif /* LWIP_NETIF_HOSTNAME */

  /*
   * Initialize the snmp variables and counters inside the struct netif.
   * The last argument should be replaced with your link speed, in units
   * of bits per second.
   */
  MIB2_INIT_NETIF(netif, snmp_ifType_ethernet_csmacd,
                  LINK_SPEED_OF_YOUR_NETIF_IN_BPS);

  netif->state = ethernetif;
  netif->name[0] = IFNAME0;
  netif->name[1] = IFNAME1;
  // We directly use etharp_output() here to save a function call.
  // You can instead declare your own function an call etharp_output()
  // from it if you have to do some checks before sending (e.g. if link
  // is available...)
#if LWIP_IPV4
  netif->output = etharp_output;
#endif /* LWIP_IPV4 */
#if LWIP_IPV6
  netif->output_ip6 = ethip6_output;
#endif /* LWIP_IPV6 */
  netif->linkoutput = low_level_output;

  // ethernetif->ethaddr = (struct eth_addr *)&(netif->hwaddr[0]);

  /* initialize the hardware */
  low_level_init(netif);

  return ERR_OK;
}
