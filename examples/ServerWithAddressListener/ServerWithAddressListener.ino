// SPDX-FileCopyrightText: (c) 2021 Shawn Silverman <shawn@pobox.com>
// SPDX-License-Identifier: MIT

// ServerWithAddressListener demonstrates how to use an address
// listener to start and stop services. Do some testing, then connect
// the Teensy to an entirely different network by moving the Ethernet
// connection and the program will still work.
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

constexpr uint32_t kDHCPTimeout = 10000;  // 10 seconds
constexpr uint16_t kServerPort = 80;

// --------------------------------------------------------------------------
//  Types
// --------------------------------------------------------------------------

// Keeps track of state for a single client.
struct ClientState {
  ClientState(EthernetClient client)
      : client(std::move(client)) {}

  EthernetClient client;
  bool closed = false;
};

// --------------------------------------------------------------------------
//  Program state
// --------------------------------------------------------------------------

// Keeps track of what and where belong to whom.
std::vector<ClientState> clients;

// The server.
EthernetServer server{kServerPort};

// QNEthernet links this variable with lwIP's `printf` calls for
// assertions and debugging. User code can also use `printf`.
extern Print *stdPrint;

// --------------------------------------------------------------------------
//  Main program
// --------------------------------------------------------------------------

// Forward declarations
void tellServer(bool hasIP);

// Program setup.
void setup() {
  Serial.begin(115200);
  while (!Serial && millis() < 4000) {
    // Wait for Serial to initialize
  }
  stdPrint = &Serial;  // Make printf work
  printf("Starting...\n");

  // Unlike the Arduino API (which you can still use), Ethernet uses
  // the Teensy's internal MAC address by default
  uint8_t mac[6];
  Ethernet.macAddress(mac);
  printf("MAC = %02x:%02x:%02x:%02x:%02x:%02x\n",
         mac[0], mac[1], mac[2], mac[3], mac[4], mac[5]);

  // Add listeners
  // It's important to add these before doing anything with Ethernet
  // so no events are missed.

  // Listen for link changes, for demonstration
  Ethernet.onLinkState([](bool state) {
    printf("Ethernet: Link %s\n", state ? "ON" : "OFF");
  });

  // Listen for address changes
  Ethernet.onAddressChanged([]() {
    IPAddress ip = Ethernet.localIP();
    bool hasIP = !(ip == INADDR_NONE);  // IPAddress has no operator!=()
    if (hasIP) {
      printf("Ethernet: Address changed:\n");

      IPAddress ip = Ethernet.localIP();
      printf("    Local IP = %u.%u.%u.%u\n", ip[0], ip[1], ip[2], ip[3]);
      ip = Ethernet.subnetMask();
      printf("    Subnet   = %u.%u.%u.%u\n", ip[0], ip[1], ip[2], ip[3]);
      ip = Ethernet.gatewayIP();
      printf("    Gateway  = %u.%u.%u.%u\n", ip[0], ip[1], ip[2], ip[3]);
      ip = Ethernet.dnsServerIP();
      if (!(ip == INADDR_NONE)) {  // May happen with static IP
        printf("    DNS      = %u.%u.%u.%u\n", ip[0], ip[1], ip[2], ip[3]);
      }
    } else {
      printf("Ethernet: Address changed: No IP address\n");
    }

    // Tell interested parties the state of the IP address, for
    // example, servers, SNTP clients, and other sub-programs that
    // need to know whether to stop/start/restart/etc
    tellServer(hasIP);
  });

  printf("Starting Ethernet with DHCP...\n");
  if (!Ethernet.begin()) {
    printf("Failed to start Ethernet\n");
    return;
  }
  if (!Ethernet.waitForLocalIP(kDHCPTimeout)) {
    printf("Failed to get IP address from DHCP\n");
    // We may still get an address later, after the timeout,
    // so continue instead of returning
  }
}

// Tell the server there's been an IP address change.
void tellServer(bool hasIP) {
  // If there's no IP address, could optionally stop the server,
  // depending on your needs
  if (hasIP) {
    if (!server) {
      printf("Starting server...");
      server.begin();
      printf("done.\n");
    }
  }
}

// The simplest possible (very non-compliant) HTTP server. Respond to
// any input with an HTTP/1.1 response.
void processClientData(ClientState &state) {
  int avail = state.client.available();
  if (avail <= 0) {
    return;
  }

  IPAddress ip = state.client.remoteIP();
  printf("Sending to client: %u.%u.%u.%u\n", ip[0], ip[1], ip[2], ip[3]);
  state.client.write("HTTP/1.1 200 OK\r\n"
                     "Connection: close\r\n"
                     "\r\n"
                     "Hello, Client!\r\n");
  state.client.flush();
  state.client.stop();
}

// Main program loop.
void loop() {
  EthernetClient client = server.accept();
  if (client) {
    // We got a connection!
    IPAddress ip = client.remoteIP();
    printf("Client connected: %u.%u.%u.%u\n", ip[0], ip[1], ip[2], ip[3]);
    clients.emplace_back(std::move(client));
    printf("Client count: %u\n", clients.size());
  }

  // Process data from each client
  for (ClientState &state : clients) {  // Use a reference so we don't copy
    if (!state.client.connected()) {
      state.closed = true;
      continue;
    }
    processClientData(state);
  }

  // Clean up all the closed clients
  size_t size = clients.size();
  clients.erase(std::remove_if(clients.begin(), clients.end(),
                               [](const auto &state) { return state.closed; }),
                clients.end());
  if (clients.size() != size) {
    printf("Client count: %u\n", clients.size());
  }
}
