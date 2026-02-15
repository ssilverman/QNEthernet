// SPDX-FileCopyrightText: (c) 2026 Shawn Silverman <shawn@pobox.com>
// SPDX-License-Identifier: AGPL-3.0-or-later

// SimpleIPerfServer implements a very simple iPerf server for TCP
// traffic. Well, it's more of a "server that just reads all the
// incoming data and does nothing with it" than a true iPerf server.
//
// Useful command: iperf -c <IP address> -i 1 -l 1460
//
// A relevant thread:
// [Teensy 4.1 - W5500 using SPI DMA](https://forum.pjrc.com/index.php?threads/teensy-4-1-w5500-using-spi-dma.77652/)
//
// This file is part of the QNEthernet library.

#include <QNEthernet.h>

using namespace qindesign::network;

// The server port.
constexpr uint16_t kPort = 5001;

namespace {  // Internal linkage section

// Set these to something to use a static IP instead of one from DHCP
const IPAddress kStaticIP;
const IPAddress kSubnet;
const IPAddress kGateway;

EthernetServer server{kPort};

}  // namespace

// Main program setup.
void setup() {
  Serial.begin(115200);
  while (!Serial && millis() < 4000) {
    // Wait for Serial
  }
  printf("Starting program...\r\n");

  const uint8_t* const mac = Ethernet.macAddress();
  printf("MAC = %02x:%02x:%02x:%02x:%02x:%02x\r\n",
         mac[0], mac[1], mac[2], mac[3], mac[4], mac[5]);

  // Listen for link changes
  Ethernet.onLinkState([](bool state) {
    if (state) {
      const LinkInfo li = Ethernet.linkInfo();
      printf("[Ethernet] Link ON, %d Mbps, %s duplex\r\n",
             li.speed,
             li.fullNotHalfDuplex ? "Full" : "Half");
    } else {
      printf("[Ethernet] Link OFF\r\n");
    }
  });

  // Listen for address changes
  Ethernet.onAddressChanged([]() {
    IPAddress ip = Ethernet.localIP();
    bool hasIP = (ip != INADDR_NONE);
    if (hasIP) {
      IPAddress subnet = Ethernet.subnetMask();
      IPAddress gw     = Ethernet.gatewayIP();
      IPAddress dns    = Ethernet.dnsServerIP();

      printf("[Ethernet] Address changed:\r\n"
             "    Local IP = %u.%u.%u.%u\r\n"
             "    Subnet   = %u.%u.%u.%u\r\n"
             "    Gateway  = %u.%u.%u.%u\r\n"
             "    DNS      = %u.%u.%u.%u\r\n",
             ip[0], ip[1], ip[2], ip[3],
             subnet[0], subnet[1], subnet[2], subnet[3],
             gw[0], gw[1], gw[2], gw[3],
             dns[0], dns[1], dns[2], dns[3]);
    } else {
      printf("[Ethernet] Address changed: No IP address\r\n");
    }
  });

  // Start Ethernet
  bool success = false;
  if (kStaticIP == INADDR_NONE) {
    printf("Starting Ethernet with DHCP...\r\n");
    success = Ethernet.begin();
  } else {
    printf("Starting Ethernet with a static IP...\r\n");
    success = Ethernet.begin(kStaticIP, kSubnet, kGateway);
  }
  if (!success) {
    printf("Failed to start Ethernet\r\n");
    return;
  }

  // Start the server
  server.begin();
}

// Main program loop.
void loop() {
  // Accept a connection
  EthernetClient client = server.accept();
  if (!client) {
    return;
  }

  IPAddress ip = client.remoteIP();
  printf("Accepted connection: %u.%u.%u.%u:%" PRIu16 "\r\n",
         ip[0], ip[1], ip[2], ip[3], client.remotePort());
  printf("Processing input...\r\n");

  // Start timing and counting
  uint32_t t = millis();
  uint32_t readCount = 0;

  // Loop until there's no more data
  while (client.connected()) {
    int avail = client.available();
    if (avail <= 0) {
      continue;
    }

    // The QNEthernet library can skip bytes by using NULL for the
    // buffer; there's no need to use a temporary one
    int read = client.read(nullptr, avail);
    if (read > 0) {
      readCount += static_cast<size_t>(read);
    }
  }
  t = millis() - t;

  const float rate =
      static_cast<float>(readCount) * 8.0f / static_cast<float>(t);
  printf("Rate = %g kbps\r\n", rate);
}
