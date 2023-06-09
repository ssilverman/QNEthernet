// SPDX-FileCopyrightText: (c) 2021-2023 Shawn Silverman <shawn@pobox.com>
// SPDX-License-Identifier: MIT

// FixedWidthServer demonstrates how to serve a protocol having a
// continuous stream of fixed-size messages from multiple clients.
//
// This file is part of the QNEthernet library.

// C++ includes
#include <algorithm>
#include <cstdio>
#include <utility>
#include <vector>

#include <QNEthernet.h>

using namespace qindesign::network;

// --------------------------------------------------------------------------
//  Configuration
// --------------------------------------------------------------------------

constexpr uint32_t kDHCPTimeout = 15'000;  // 15 seconds

constexpr uint16_t kServerPort = 5000;

constexpr int kMessageSize = 10;  // Pretend the protocol specifies 10 bytes

// --------------------------------------------------------------------------
//  Types
// --------------------------------------------------------------------------

// Keeps track of state for a single client.
struct ClientState {
  ClientState(EthernetClient client)
      : client(std::move(client)) {}

  EthernetClient client;
  int bufSize = 0;  // Keeps track of how many bytes have been read
  uint8_t buf[kMessageSize];
  bool closed = false;
};

// --------------------------------------------------------------------------
//  Program State
// --------------------------------------------------------------------------

// Keeps track of what and where belong to whom.
std::vector<ClientState> clients;

// The server.
EthernetServer server{kServerPort};

// --------------------------------------------------------------------------
//  Main Program
// --------------------------------------------------------------------------

// Program setup.
void setup() {
  Serial.begin(115200);
  while (!Serial && millis() < 4000) {
    // Wait for Serial
  }
  printf("Starting...\r\n");

  // Unlike the Arduino API (which you can still use), QNEthernet uses
  // the Teensy's internal MAC address by default, so we can retrieve
  // it here
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
  } else {
    IPAddress ip = Ethernet.localIP();
    printf("    Local IP    = %u.%u.%u.%u\r\n", ip[0], ip[1], ip[2], ip[3]);
    ip = Ethernet.subnetMask();
    printf("    Subnet mask = %u.%u.%u.%u\r\n", ip[0], ip[1], ip[2], ip[3]);
    ip = Ethernet.gatewayIP();
    printf("    Gateway     = %u.%u.%u.%u\r\n", ip[0], ip[1], ip[2], ip[3]);
    ip = Ethernet.dnsServerIP();
    printf("    DNS         = %u.%u.%u.%u\r\n", ip[0], ip[1], ip[2], ip[3]);

    // Start the server
    printf("Listening for clients on port %u...\r\n", kServerPort);
    server.begin();
  }
}

// Process one message. This implementation simply dumps to Serial.
//
// We could pass just the buffer, but we're passing the whole state
// here so we know which client it's from.
void processMessage(const ClientState &state) {
  printf("Message: ");
  fwrite(state.buf, sizeof(state.buf[0]), kMessageSize, stdout);
  printf("\r\n");
}

// Main program loop.
void loop() {
  EthernetClient client = server.accept();
  if (client) {
    IPAddress ip = client.remoteIP();
    printf("Client connected: %u.%u.%u.%u\r\n", ip[0], ip[1], ip[2], ip[3]);
    clients.emplace_back(std::move(client));
    printf("Client count: %zu\r\n", clients.size());
  }

  // Process data from each client
  for (ClientState &state : clients) {  // Use a reference
    if (!state.client.connected()) {
      state.closed = true;
      continue;
    }

    int avail = state.client.available();
    if (avail > 0) {
      int toRead = std::min(kMessageSize - state.bufSize, avail);
      state.bufSize += state.client.read(&state.buf[state.bufSize], toRead);
      if (state.bufSize >= kMessageSize) {
        processMessage(state);
        state.bufSize = 0;
      }
    }
  }

  // Clean up all the closed clients
  size_t size = clients.size();
  clients.erase(std::remove_if(clients.begin(), clients.end(),
                               [](const auto &state) { return state.closed; }),
                clients.end());
  if (clients.size() != size) {
    printf("Client count: %zu\r\n", clients.size());
  }
}
