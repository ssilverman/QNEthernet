// SPDX-FileCopyrightText: (c) 2021-2025 Shawn Silverman <shawn@pobox.com>
// SPDX-License-Identifier: AGPL-3.0-or-later

// main.cpp is a testing playground for this library.
// This file is part of the QNEthernet library.

// Define MAIN_TEST_PROGRAM to use this test program.
#if defined(MAIN_TEST_PROGRAM) && !defined(PIO_UNIT_TESTING)

#include <cstring>

#include <Arduino.h>

#include "QNEthernet.h"
#include "lwip/dns.h"
#include "qnethernet/MbedTLSClient.h"
#include "qnethernet/QNDNSClient.h"

using namespace qindesign::network;

// Startup delay, in milliseconds.
static constexpr uint32_t kStartupDelay = 2'000;

// Timeouts
static constexpr uint32_t kDHCPTimeout = 15'000;

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

static void dnsLookup();
static void clientConnect();

// Main program loop.
void loop() {
  // Check for a network change
  if (networkChanged) {
    networkChanged = false;

    if ((Ethernet.localIP() != INADDR_NONE) && Ethernet.linkState()) {
      // Do network things here, but only if there's an address and a link

      dnsLookup();
      clientConnect();
    }
  }
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
  MbedTLSClient tlsclient{client};
  printf("[Main] Connecting to %s...\r\n", kHost);
  if (!tlsclient.connect(kHost, uint16_t{443})) {
    printf("[Main] Error connecting\r\n");
    return;
  }

  // Send the request
  ::util::writeFully(tlsclient, reinterpret_cast<const uint8_t *>(kRequest),
                     std::strlen(kRequest),
                     [&tlsclient]() { return !static_cast<bool>(tlsclient); });
  tlsclient.flush();

  // Read the response
  while (tlsclient.connected()) {
    int avail = tlsclient.available();
    if (avail <= 0) {
      continue;
    }
    for (int i = 0; i < avail; i++) {
      int c = tlsclient.read();
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
