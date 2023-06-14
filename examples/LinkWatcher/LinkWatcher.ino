// SPDX-FileCopyrightText: (c) 2023 Shawn Silverman <shawn@pobox.com>
// SPDX-License-Identifier: MIT

// LinkWatcher is a simple application that watches the link state
// using a listener. It doesn't try to set an address.
//
// This file is part of the QNEthernet library.

#include <QNEthernet.h>

using namespace qindesign::network;

// Program setup.
void setup() {
  Serial.begin(115200);
  while (!Serial && millis() < 4000) {
    // Wait for Serial
  }
  printf("Starting...\r\n");

  // Listen for link changes
  Ethernet.onLinkState([](bool state) {
    if (state) {
      printf("[Ethernet] Link ON: %d Mbps, %s duplex, %s crossover\r\n",
             Ethernet.linkSpeed(),
             Ethernet.linkIsFullDuplex() ? "full" : "half",
             Ethernet.linkIsCrossover() ? "is" : "not");
    } else {
      printf("[Ethernet] Link OFF\r\n");
    }
  });

  // We just want to watch the link in this example, so don't start DHCP
  Ethernet.setDHCPEnabled(false);
  if (!Ethernet.begin()) {
    printf("ERROR: Failed to start Ethernet\r\n");
  }
}

// Program loop.
void loop() {
}
