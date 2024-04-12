// SPDX-FileCopyrightText: (c) 2021-2024 Shawn Silverman <shawn@pobox.com>
// SPDX-License-Identifier: AGPL-3.0-or-later

// main.cpp is a testing playground for this library.
// This file is part of the QNEthernet library.

// Define MAIN_TEST_PROGRAM to use this test program.
#if defined(MAIN_TEST_PROGRAM) && !defined(PIO_UNIT_TESTING)

#include <Arduino.h>

#include "QNEthernet.h"
#include "QNDNSClient.h"
#include "lwip/dns.h"

using namespace qindesign::network;

// Startup delay, in milliseconds.
static constexpr uint32_t kStartupDelay = 2000;

// Timeouts
static constexpr uint32_t kDHCPTimeout = 15000;

// Flag that indicates something about the network changed.
static volatile bool networkChanged = false;

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

  uint8_t mac[6];
  Ethernet.macAddress(mac);
  printf("[Main] MAC = %02x:%02x:%02x:%02x:%02x:%02x\r\n",
         mac[0], mac[1], mac[2], mac[3], mac[4], mac[5]);

  Ethernet.onInterfaceStatus([](bool status) {
    printf("[Ethernet] Interface %s\r\n", status ? "UP" : "DOWN");
  });

  Ethernet.onLinkState([](bool state) {
    if (state) {
      printf("[Ethernet] Link: ON, %d Mbps, %s duplex, %s crossover\r\n",
             Ethernet.linkSpeed(),
             Ethernet.linkIsFullDuplex() ? "full" : "half",
             Ethernet.linkIsCrossover() ? "is" : "not");
    } else {
      printf("[Ethernet] Link: OFF\r\n");
    }
    networkChanged = true;
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
    networkChanged = true;
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
  // Check for a network change
  if (networkChanged) {
    networkChanged = false;

    if ((Ethernet.localIP() != INADDR_NONE) && Ethernet.linkState()) {
      // Do network things here, but only if there's an address and a link

#if LWIP_DNS
      IPAddress ip;
      constexpr char kHostname[]{"dns.google"};
      if (!DNSClient::getHostByName(kHostname, ip,
                                    QNETHERNET_DEFAULT_DNS_LOOKUP_TIMEOUT)) {
        printf("[Main] Lookup for \"%s\" failed\r\n", kHostname);
      } else {
        printf("[Main] Lookup \"%s\": %u.%u.%u.%u\r\n",
               kHostname, ip[0], ip[1], ip[2], ip[3]);
      }
#endif  // LWIP_DNS
    }
  }
}

#endif  // MAIN_TEST_PROGRAM
