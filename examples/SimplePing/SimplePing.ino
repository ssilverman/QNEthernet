// SPDX-FileCopyrightText: (c) 2025 Shawn Silverman <shawn@pobox.com>
// SPDX-License-Identifier: AGPL-3.0-or-later

// SimplePing demonstrates a simpler Ping program.
//
// In order to use this example, define the QNETHERNET_ENABLE_PING_SEND macro.
//
// Note: the configuration macros must either be defined in the
//       project build options or in the qnethernet_opts.h
//       library file.
//
// This file is part of the QNEthernet library.

#include <QNEthernet.h>

using namespace qindesign::network;

// --------------------------------------------------------------------------
//  Configuration
// --------------------------------------------------------------------------

constexpr uint32_t kDHCPTimeout = 15'000;  // 15 seconds

constexpr uint32_t kPingInterval = 1'000;  // 1 second

constexpr char kHostname[]{"arduino.cc"};

// --------------------------------------------------------------------------
//  Program State
// --------------------------------------------------------------------------

static bool running = false;  // Whether the program is still running

static IPAddress hostIP;
static elapsedMillis pingTimer = kPingInterval;  // Start expired
static unsigned int pingCounter = 0;

// --------------------------------------------------------------------------
//  Main Program
// --------------------------------------------------------------------------

// Program setup.
void setup() {
  Serial.begin(115200);
  while (!Serial && (millis() < 4000)) {
    // Wait for Serial
  }
  printf("Starting...\r\n");

  uint8_t mac[6];
  Ethernet.macAddress(mac);  // This is informative; it retrieves, not sets
  printf("MAC = %02x:%02x:%02x:%02x:%02x:%02x\r\n",
         mac[0], mac[1], mac[2], mac[3], mac[4], mac[5]);

  printf("Starting Ethernet with DHCP...\r\n");
  if (!Ethernet.begin()) {
    printf("Failed to start Ethernet\r\n");
    return;
  }
  if (!Ethernet.waitForLocalIP(kDHCPTimeout)) {
    printf("Failed to get IP address from DHCP\r\n");
    return;
  }

  IPAddress ip = Ethernet.localIP();
  printf("    Local IP    = %u.%u.%u.%u\r\n", ip[0], ip[1], ip[2], ip[3]);
  ip = Ethernet.subnetMask();
  printf("    Subnet mask = %u.%u.%u.%u\r\n", ip[0], ip[1], ip[2], ip[3]);
  ip = Ethernet.gatewayIP();
  printf("    Gateway     = %u.%u.%u.%u\r\n", ip[0], ip[1], ip[2], ip[3]);
  ip = Ethernet.dnsServerIP();
  printf("    DNS         = %u.%u.%u.%u\r\n", ip[0], ip[1], ip[2], ip[3]);

  printf("\r\n");

  if (!Ethernet.hostByName(kHostname, hostIP)) {
    printf("Failed to look up host (%s)\r\n", kHostname);
    return;
  }

  printf("Pinging %s (%u.%u.%u.%u)...\r\n",
         kHostname, hostIP[0], hostIP[1], hostIP[2], hostIP[3]);

  running = true;
}

// Main program loop.
void loop() {
  if (!running || (pingTimer < kPingInterval)) {
    return;
  }

  pingCounter++;

  printf("%u. ", pingCounter);
  long rtt = Ethernet.ping(hostIP);
  if (rtt >= 0) {
    printf("Time = %ld ms\r\n", rtt);
    pingTimer = rtt;
  } else {
    printf("Ping failed\r\n");
    pingTimer = 0;
  }
}
