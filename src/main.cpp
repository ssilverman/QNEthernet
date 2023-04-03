// SPDX-FileCopyrightText: (c) 2021-2023 Shawn Silverman <shawn@pobox.com>
// SPDX-License-Identifier: MIT

// main.cpp is a testing playground for this library.
// This file is part of the QNEthernet library.

// Define MAIN_TEST_PROGRAM to use this test program.
#ifdef MAIN_TEST_PROGRAM

#include <Arduino.h>

#include "QNEthernet.h"

using namespace qindesign::network;

// Startup delay, in milliseconds.
static constexpr uint32_t kStartupDelay = 2000;

// Timeouts
static constexpr uint32_t kDHCPTimeout = 15000;

// Main program setup.
void setup() {
  Serial.begin(115200);
  while (!Serial && millis() < 4000) {
    // Wait for Serial
  }
  printf("Waiting for %" PRIu32 "ms...\r\n", kStartupDelay);
  delay(kStartupDelay);

  if (CrashReport) {
    util::StdioPrint p{stdout};
    p.println();
    p.println(CrashReport);
    p.println();
    CrashReport.clear();
  }

  printf("Starting...\r\n");

  uint8_t mac[6];
  Ethernet.macAddress(mac);
  printf("[Main] MAC = %02x:%02x:%02x:%02x:%02x:%02x\r\n",
         mac[0], mac[1], mac[2], mac[3], mac[4], mac[5]);

  Ethernet.onLinkState([](bool state) {
    if (state) {
      printf("[Ethernet] Link: ON, %d Mbps\r\n", Ethernet.linkSpeed());
    } else {
      printf("[Ethernet] Link: OFF\r\n");
    }
  });

  Ethernet.onAddressChanged([]() {
    IPAddress ip = Ethernet.localIP();
    bool hasIP = (ip != INADDR_NONE);
    if (hasIP) {
      IPAddress subnet = Ethernet.subnetMask();
      IPAddress gw = Ethernet.gatewayIP();
      IPAddress dns = Ethernet.dnsServerIP();
      printf("[Ethernet] Address changed:\r\n"
             "    Local IP = %" PRIu8 ".%" PRIu8 ".%" PRIu8 ".%" PRIu8 "\r\n"
             "    Subnet   = %" PRIu8 ".%" PRIu8 ".%" PRIu8 ".%" PRIu8 "\r\n"
             "    Gateway  = %" PRIu8 ".%" PRIu8 ".%" PRIu8 ".%" PRIu8 "\r\n"
             "    DNS      = %" PRIu8 ".%" PRIu8 ".%" PRIu8 ".%" PRIu8 "\r\n",
             ip[0], ip[1], ip[2], ip[3],
             subnet[0], subnet[1], subnet[2], subnet[3],
             gw[0], gw[1], gw[2], gw[3],
             dns[0], dns[1], dns[2], dns[3]);
    } else {
      printf("[Ethernet] Address changed: No IP address\r\n");
    }
  });

  // Start DHCP
  printf("[Main] Starting Ethernet (DHCP)...\r\n");
  if (!Ethernet.begin()) {
    printf("[Main] ERROR: Failed to start Ethernet\r\n");
    return;
  }

#if LWIP_MDNS_RESPONDER
  if (!MDNS.begin("qneth")) {
    printf("[Main] ERROR: Failed to start mDNS\r\n");
  } else {
    if (!MDNS.addService("_http", "_tcp", 80)) {
      printf("[Main] ERROR: Failed to add mDNS service\r\n");
    }
  }
#endif  // LWIP_MDNS_RESPONDER

  if (!Ethernet.waitForLocalIP(kDHCPTimeout)) {
    printf("[Main] ERROR: Failed to get DHCP address within %" PRIu32 "ms\r\n",
           kDHCPTimeout);
  }
}

// Main program loop.
void loop() {
}

#endif  // MAIN_TEST_PROGRAM
