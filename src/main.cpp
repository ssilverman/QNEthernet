// SPDX-FileCopyrightText: (c) 2021-2025 Shawn Silverman <shawn@pobox.com>
// SPDX-License-Identifier: AGPL-3.0-or-later

// main.cpp is a testing playground for this library.
// This file is part of the QNEthernet library.

// Define MAIN_TEST_PROGRAM to use this test program.
#if defined(MAIN_TEST_PROGRAM) && !defined(PIO_UNIT_TESTING)

#include <Arduino.h>

#include "QNEthernet.h"
#include "lwip/dns.h"
#include "qnethernet/QNDNSClient.h"

using namespace qindesign::network;

// Startup delay, in milliseconds.
static constexpr uint32_t kStartupDelay = 2'000;

// PHY functions
extern "C" {
void init_phy();
uint16_t mdio_read(uint16_t regaddr);
void mdio_write(uint16_t regaddr, uint16_t data);
}  // extern "C"

#define PHY_CDCR 0x1E  // Cable Diagnostic Control Register
#define PHY_CDCR_START  (1 << 15)
#define PHY_CDCR_STATUS (1 <<  1)
#define PHY_CDCR_FAIL   (1 <<  0)

#define PHY_CDSCR 0x170  // Cable Diagnostic Specific Control Register
#define PHY_CDSCR_CROSS_DISABLE        (1 << 14)  // 0: Look for reflections on opposite channel, 1: same channel
#define PHY_CDSCR_CHANNEL_SELECT       (1 << 13)  // 0: Channel A, 1: Channel B
#define PHY_CDSCR_CROSS_AND_CHANNEL(n) ((uint16_t)(((n) & 0x03) << 13))  // Combined Cross_Disable and chan_sel
#define PHY_CDSCR_SEGMENT_NUM(n)       ((uint16_t)(((n) & 0x07) << 4))
  //< 1: 0-10m, 2: 10-20m, 3: 20-40m, 4: 40-80m, 5: 80m-

#define PHY_CDLRR1 0x0180  // Cable Diagnostic Location Result Register 1
#define PHY_CDLRR1_PEAK_LOCATION(n) ((n) & 0xff)

#define PHY_CDLAR1 0x0185  // Cable Diagnostic Amplitude Result Register 1
#define PHY_CDLAR1_PEAK_AMPLITUDE(n) ((n) & 0x7f)

#define PHY_CDLAR6 0x018A  // Cable Diagnostic Amplitude Result Register 6
#define PHY_CDLAR6_PEAK_POLARITY(n) (((n) >> 11) & 0x01)
#define PHY_CDLAR6_CROSS_DETECT(n)  (((n) >> 5) & 0x01)

#define CLRSET(reg, clear, set) ((reg) = ((reg) & ~(clear)) | (set))

// Describes information about one register.
struct Reg {
  uint16_t addr;
  uint16_t mask;
  uint16_t shift;
};

static constexpr size_t kRegCount     = 7;
static constexpr size_t kSegmentCount = 5;

// Registers to set for each segment
static constexpr Reg kRegList[kRegCount]{  // All surmised
    { 0x0456, 0x0003, 9 },
    { 0x0411, 0x001f, 0 },
    { 0x0416, 0x000f, 4 },
    { 0x0170, 0x1fff, 0 },
    { 0x0173, 0xffff, 0 },
    { 0x0175, 0xffff, 0 },
    { 0x0178, 0x0007, 0 },
};

static constexpr uint16_t kSegmentRegs[kSegmentCount][kRegCount]{
    { 0x0003, 0x0013, 0x000a, 0x1c12, 0x0d07, 0x1004, 0x0002 },  // Surmised
    { 0x0003, 0x0013, 0x000a, 0x1c22, 0x0d14, 0x1004, 0x0002 },
    { 0x0003, 0x0015, 0x000a, 0x1e32, 0x0d14, 0x1004, 0x0002 },
    { 0x0003, 0x0016, 0x000a, 0x1e42, 0x343b, 0x1008, 0x0002 },  // Surmised
    { 0x0003, 0x0016, 0x0009, 0x1e52, 0x8f6f, 0x100b, 0x0006 },
};

// Forward declarations
static void runTests();

// Main program setup.
void setup() {
  Serial.begin(115200);
  while (!Serial && millis() < 4000) {
    // Wait for Serial
  }
  printf("Waiting for %" PRIu32 "ms...\r\n", kStartupDelay);
  delay(kStartupDelay);

#if defined(TEENSYDUINO)
  if (CrashReport) {
    util::StdioPrint p{stdout};
    p.println();
    p.println(CrashReport);
    p.println();
    CrashReport.clear();
  }
#endif  // defined(TEENSYDUINO)

  printf("Starting...\r\n");

  printf("Starting PHY\r\n");
  init_phy();

  runTests();
}

// Waits for TDR to be ready. This returns the last read value of CDCR.
static uint16_t waitForTDRReady() {
  while (true) {
    uint16_t cdcr = mdio_read(PHY_CDCR);
    if ((cdcr & PHY_CDCR_STATUS) != 0) {
      return cdcr;
    }
  }
}

// Waits for TDR ready and then prints the results.
static void waitAndPrintResult(uint16_t seg, bool cross, bool channel) {
  const char *segName;
  switch (seg) {
    case 1: segName = " 0-10m"; break;
    case 2: segName = "10-20m"; break;
    case 3: segName = "20-40m"; break;
    case 4: segName = "40-80m"; break;
    case 5: segName = "80m-  "; break;
    default:
      segName = "unknown";
  }

  printf("%s (%s%s):",
         segName,
         cross ? "X-" : "  ",
         channel ? "B" : "A");

  uint16_t cdcr = waitForTDRReady();
  if ((cdcr & PHY_CDCR_FAIL) != 0) {
    printf(" Failed\r\n");
    return;
  }

  uint16_t amp = PHY_CDLAR1_PEAK_AMPLITUDE(mdio_read(PHY_CDLAR1));
  if (amp == 0) {
    printf(" No peak\r\n");
    return;
  }

  uint16_t loc = PHY_CDLRR1_PEAK_LOCATION(mdio_read(PHY_CDLRR1));
  uint16_t cdlar6 = mdio_read(PHY_CDLAR6);
  uint16_t sign = PHY_CDLAR6_PEAK_POLARITY(cdlar6);
  uint16_t isCross = PHY_CDLAR6_CROSS_DETECT(cdlar6);
  float location = ((float)loc - 7.0f)/1.3f;
  printf(" loc=%fm sign=%u cross=%u\r\n", location, sign, isCross);
}

// Runs tests over all the segment and channel possibilities.
static void runTests() {
  // Start ready
  waitForTDRReady();

  for (uint16_t seg = 1; seg <= 5; seg++) {
    // Set some registers for this segment
    for (size_t i = 0; i < kRegCount; i++) {
      const Reg &reg = kRegList[i];
      uint16_t r = mdio_read(reg.addr);
      CLRSET(r,
             (reg.mask << reg.shift),
             (kSegmentRegs[seg - 1][i] & reg.mask) << reg.shift);
      mdio_write(reg.addr, r);
    }

    // Loop over all the channel options
    for (uint16_t i = 0; i < 4; i++) {
      // Select which measurement
      uint16_t cdscr = mdio_read(PHY_CDSCR);
      CLRSET(cdscr,
             PHY_CDSCR_CROSS_AND_CHANNEL(3) | PHY_CDSCR_SEGMENT_NUM(7),
             PHY_CDSCR_CROSS_AND_CHANNEL(i) | PHY_CDSCR_SEGMENT_NUM(seg));
      mdio_write(PHY_CDSCR, cdscr);

      // Start the test
      mdio_write(PHY_CDCR, mdio_read(PHY_CDCR) | PHY_CDCR_START);
      waitAndPrintResult(seg,
                         (cdscr & PHY_CDSCR_CROSS_DISABLE) == 0,
                         (cdscr & PHY_CDSCR_CHANNEL_SELECT) != 0);
    }
  }
}

static void dnsLookup();
static void clientConnect();

// Main program loop.
void loop() {
}

static void dnsLookup() {
#if LWIP_DNS
  static constexpr char kHostname[]{"dns.google"};
  IPAddress ip;
  printf("[Main] Looking up \"%s\"...\r\n", kHostname);
  if (!DNSClient::getHostByName(kHostname, ip,
                                QNETHERNET_DEFAULT_DNS_LOOKUP_TIMEOUT)) {
    printf("[Main] Lookup failed\r\n");
  } else {
    printf("[Main] IP address: %u.%u.%u.%u\r\n", ip[0], ip[1], ip[2], ip[3]);
  }
#endif  // LWIP_DNS
}

#define HOST "www.google.com"
static constexpr char kHost[]{HOST};
static constexpr char kRequest[]{
    "HEAD / HTTP/1.1\r\n"
    "Host: " HOST "\r\n"
    "Connection: close\r\n"
    "\r\n"
};
#undef HOST
static constexpr uint16_t kPort = 80;

static void clientConnect() {
#if LWIP_TCP
  EthernetClient client;
  printf("[Main] Connecting to %s...\r\n", kHost);
  if (!client.connect(kHost, 80)) {
    printf("[Main] Error connecting\r\n");
    return;
  }

  // Send the request
  client.writeFully(kRequest);
  client.flush();

  // Read the response
  while (client.connected()) {
    int avail = client.available();
    if (avail <= 0) {
      continue;
    }
    for (int i = 0; i < avail; i++) {
      int c = client.read();
      switch (c) {
        case '\t':
        case '\n':
        case '\r':
          printf("%c", c);
          break;
        default:
          if (c < ' ' || 0x7f <= c) {
            printf("<%02x>", c);
          } else {
            printf("%c", c);
          }
          break;
      }
    }
  }
  printf("\r\n");
#endif  // LWIP_TCP
}

#endif  // MAIN_TEST_PROGRAM

/*

PDF Table 1 printing errors (Segment 4 combined with the registers):
0x456[100x:39]    - 0x456[10:9] 0x3
0x411[04x:106]    - 0x411[4:0]  0x16
0x416[07x:4A]     - 0x416[7:4]  0xA
0x1700[x112E:04]2 - 0x170[12:0] 0x1E42
0x107x343B        - 0x173       0x343B
0x107x51008       - 0x175       0x1008
0x178[20:x02]     - 0x178[2:0]  0x2

See:
* https://www.ti.com/lit/an/snla330/snla330.pdf
* https://e2e.ti.com/support/interface-group/interface/f/interface-forum/988906/dp83825i-tdr-test-issue-and-conflict-between-the-datasheet-register-map-and-the-application-note-issue

Segment 1 (from the forum link):
0x456: 0x0608 [10:9]=0x3
0x411: 0x0813 [ 4:0]=0x13
0x416: 0x08A0 [ 7:4]=0xA
0x170: 0x5C12 [12:0]=0x1C12
0x173: 0x0D07
0x175: 0x1004
// 0x177: 0x1E00
0x178: 0x2    [ 2:0]=0x2

*/
