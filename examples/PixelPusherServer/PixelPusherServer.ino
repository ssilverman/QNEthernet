// SPDX-FileCopyrightText: (c) 2022 Shawn Silverman <shawn@pobox.com>
// SPDX-License-Identifier: MIT

// PixelPusherServer implements a simple PixelPusher receiver. The
// output is sent to an OctoWS2811. The parameters are configurable
// via constants.
//
// This file is part of the QNEthernet library.

// C++ includes
#include <cstdio>

#include <QNEthernet.h>

#include "OctoWS2811Receiver.h"
#include "PixelPusherServer.h"

using namespace qindesign::network;

// -------------------------------------------------------------------
//  Configuration
// -------------------------------------------------------------------

// PixelPusher configuration
constexpr int kNumStrips = 8;
constexpr int kPixelsPerStrip = 240;
// ----
constexpr int kControllerNum = 0;
constexpr int kGroupNum = 0;
constexpr uint16_t kVendorId = 0;
constexpr uint16_t kProductId = 0;
constexpr uint16_t kHWRev = 0;

// The OctoWS2811Receiver implementation supports global brightness.
constexpr uint32_t kPixelPusherFlags =
    PixelPusherServer::PusherFlags::GLOBALBRIGHTNESS;

// -------------------------------------------------------------------
//  Main program
// -------------------------------------------------------------------

PixelPusherServer pp;
OctoWS2811Receiver receiver{pp, kNumStrips, kPixelsPerStrip};

// Forward declarations
void addressChanged(bool hasIP);

// Program setup.
void setup() {
  Serial.begin(115200);
  while (!Serial && millis() < 4000) {
    // Wait for Serial to initialize
  }
  stdPrint = &Serial;  // Make printf work (a QNEthernet feature)

  printf("Starting PixelPusherServer...\n");

  uint8_t mac[6];
  Ethernet.macAddress(mac);  // This is informative; it retrieves, not sets
  printf("MAC = %02x:%02x:%02x:%02x:%02x:%02x\n",
         mac[0], mac[1], mac[2], mac[3], mac[4], mac[5]);

  // Listen for link changes
  Ethernet.onLinkState([](bool state) {
    printf("[Ethernet] Link %s\n", state ? "ON" : "OFF");
  });

  // Listen for address changes
  Ethernet.onAddressChanged([]() {
    IPAddress ip = Ethernet.localIP();
    bool hasIP = (ip != INADDR_NONE);
    if (hasIP) {
      IPAddress ip = Ethernet.localIP();
      IPAddress subnet = Ethernet.subnetMask();
      IPAddress broadcast = Ethernet.broadcastIP();
      IPAddress gw = Ethernet.gatewayIP();
      IPAddress dns = Ethernet.dnsServerIP();

      printf("[Ethernet] Address changed:\n"
             "    Local IP     = %u.%u.%u.%u\n"
             "    Subnet       = %u.%u.%u.%u\n"
             "    Broadcast IP = %u.%u.%u.%u\n"
             "    Gateway      = %u.%u.%u.%u\n"
             "    DNS          = %u.%u.%u.%u\n",
             ip[0], ip[1], ip[2], ip[3],
             subnet[0], subnet[1], subnet[2], subnet[3],
             broadcast[0], broadcast[1], broadcast[2], broadcast[3],
             gw[0], gw[1], gw[2], gw[3],
             dns[0], dns[1], dns[2], dns[3]);
    } else {
      printf("[Ethernet] Address changed: No IP address\n");
    }

    // Tell interested parties the state of the IP address, for
    // example, servers, SNTP clients, and other sub-programs that
    // need to know whether to stop/start/restart/etc
    addressChanged(hasIP);
  });

  printf("Starting Ethernet with DHCP...\n");
  if (!Ethernet.begin()) {
    printf("ERROR: Failed to start Ethernet\n");
    return;
  }
}

// The address has changed. For example, a DHCP address arrived.
void addressChanged(bool hasIP) {
  if (!hasIP) {
    printf("Stopping server and LEDs...");
    fflush(stdout);  // Print what we have so far if line buffered
    pp.end();
    receiver.end();
    printf("done.\n");
    return;
  }

  printf("Starting server and LEDs...");
  fflush(stdout);  // Print what we have so far if line buffered
  if (!receiver.begin()) {
    printf("ERROR: Starting LEDs\n");
    return;
  }
  if (!pp.begin(&receiver, PixelPusherServer::kDefaultPixelsPort,
                kControllerNum, kGroupNum,
                kVendorId, kProductId,
                kHWRev,
                kPixelPusherFlags)) {
    printf("ERROR: Starting PixelPusher server\n");
    return;
  }

  printf("done.\n");
  printf("-----------------------\n");
  printf("Num strips:       %d\n", kNumStrips);
  printf("Pixels per strip: %d\n", kPixelsPerStrip);
  printf("-----------------------\n");
  printf("Port:        %u\n", pp.pixelsPort());
  printf("Controller:  %d\n", kControllerNum);
  printf("Group:       %d\n", kGroupNum);
  printf("Vendor ID:   %04Xh\n", kVendorId);
  printf("Product ID:  %04Xh\n", kProductId);
  printf("HW revision: %u\n", kHWRev);
  printf("Flags:       %08" PRIX32 "h\n", kPixelPusherFlags);
  printf("-----------------------\n");
}

// Main program loop.
void loop() {
  pp.loop();
}
