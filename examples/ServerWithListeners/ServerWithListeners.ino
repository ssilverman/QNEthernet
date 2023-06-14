// SPDX-FileCopyrightText: (c) 2021-2023 Shawn Silverman <shawn@pobox.com>
// SPDX-License-Identifier: MIT

// ServerWithListeners demonstrates how to use listeners to start and
// stop services. Do some testing, then connect the Teensy to an
// entirely different network by moving the Ethernet connection and
// the program will still work.
//
// This also demonstrates:
// 1. Using a link state listener,
// 2. Setting a static IP if desired,
// 3. Managing connections and attaching state to each connection,
// 4. How to use `printf`,
// 5. Very rudimentary HTTP server behaviour,
// 6. Client timeouts, and
// 7. Use of a half closed connection.
//
// This is a rudimentary basis for a complete server program.
//
// Note that the configuration code and logic is just for illustration.
// Your program doesn't need to include everything here.
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

// NOTE: Not all the code here is needed

// The DHCP timeout, in milliseconds. Set to zero to not wait and
// instead rely on the listener to inform us of an address assignment.
constexpr uint32_t kDHCPTimeout = 15'000;  // 15 seconds

// The link timeout, in milliseconds. Set to zero to not wait and
// instead rely on the listener to inform us of a link.
constexpr uint32_t kLinkTimeout = 5'000;  // 5 seconds

constexpr uint16_t kServerPort = 80;

// Timeout for waiting for input from the client.
constexpr uint32_t kClientTimeout = 5'000;  // 5 seconds

// Timeout for waiting for a close from the client after a
// half close.
constexpr uint32_t kShutdownTimeout = 30'000;  // 30 seconds

// Set the static IP to something other than INADDR_NONE (all zeros)
// to not use DHCP. The values here are just examples.
IPAddress staticIP{0, 0, 0, 0};//{192, 168, 1, 101};
IPAddress subnetMask{255, 255, 255, 0};
IPAddress gateway{192, 168, 1, 1};

// --------------------------------------------------------------------------
//  Types
// --------------------------------------------------------------------------

// Keeps track of state for a single client.
struct ClientState {
  ClientState(EthernetClient client)
      : client(std::move(client)) {}

  EthernetClient client;
  bool closed = false;

  // For timeouts.
  uint32_t lastRead = millis();  // Mark creation time

  // For half closed connections, after "Connection: close" was sent
  // and closeOutput() was called
  uint32_t closedTime = 0;    // When the output was shut down
  bool outputClosed = false;  // Whether the output was shut down

  // Parsing state
  bool emptyLine = false;
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

  // Add listeners
  // It's important to add these before doing anything with Ethernet
  // so no events are missed.

  // Listen for link changes
  Ethernet.onLinkState([](bool state) {
    printf("[Ethernet] Link %s\r\n", state ? "ON" : "OFF");
  });

  // Listen for address changes
  Ethernet.onAddressChanged([]() {
    IPAddress ip = Ethernet.localIP();
    bool hasIP = (ip != INADDR_NONE);
    if (hasIP) {
      printf("[Ethernet] Address changed:\r\n");

      printf("    Local IP = %u.%u.%u.%u\r\n", ip[0], ip[1], ip[2], ip[3]);
      ip = Ethernet.subnetMask();
      printf("    Subnet   = %u.%u.%u.%u\r\n", ip[0], ip[1], ip[2], ip[3]);
      ip = Ethernet.gatewayIP();
      printf("    Gateway  = %u.%u.%u.%u\r\n", ip[0], ip[1], ip[2], ip[3]);
      ip = Ethernet.dnsServerIP();
      if (ip != INADDR_NONE) {  // May happen with static IP
        printf("    DNS      = %u.%u.%u.%u\r\n", ip[0], ip[1], ip[2], ip[3]);
      }
    } else {
      printf("[Ethernet] Address changed: No IP address\r\n");
    }
  });

  if (initEthernet()) {
    // Start the server
    printf("Starting server on port %u...", kServerPort);
    server.begin();
    printf("%s\r\n", (server) ? "Done." : "FAILED!");
  }
}

bool initEthernet() {
  // DHCP
  if (staticIP == INADDR_NONE) {
    printf("Starting Ethernet with DHCP...\r\n");
    if (!Ethernet.begin()) {
      printf("Failed to start Ethernet\r\n");
      return false;
    }

    // We can choose not to wait and rely on the listener to tell us
    // when an address has been assigned
    if (kDHCPTimeout > 0) {
      printf("Waiting for IP address...\r\n");
      if (!Ethernet.waitForLocalIP(kDHCPTimeout)) {
        printf("No IP address yet\r\n");
        // We may still get an address later, after the timeout,
        // so continue instead of returning
      }
    }
  } else {
    // Static IP
    printf("Starting Ethernet with static IP...\r\n");
    if (!Ethernet.begin(staticIP, subnetMask, gateway)) {
      printf("Failed to start Ethernet\r\n");
      return false;
    }

    // When setting a static IP, the address is changed immediately,
    // but the link may not be up; optionally wait for the link here
    if (kLinkTimeout > 0) {
      printf("Waiting for link...\r\n");
      if (!Ethernet.waitForLink(kLinkTimeout)) {
        printf("No link yet\r\n");
        // We may still see a link later, after the timeout, so
        // continue instead of returning
      }
    }
  }

  return true;
}

// The simplest possible (very non-compliant) HTTP server. Respond to
// any input with an HTTP/1.1 response.
void processClientData(ClientState &state) {
  // Loop over available data until an empty line or no more data
  // Note that if emptyLine starts as false then this will ignore any
  // initial blank line.
  while (true) {
    int avail = state.client.available();
    if (avail <= 0) {
      return;
    }

    state.lastRead = millis();
    int c = state.client.read();
    printf("%c", c);
    if (c == '\n') {
      if (state.emptyLine) {
        break;
      }

      // Start a new empty line
      state.emptyLine = true;
    } else if (c != '\r') {
      // Ignore carriage returns because CRLF is a likely pattern in
      // an HTTP request
      state.emptyLine = false;
    }
  }

  IPAddress ip = state.client.remoteIP();
  printf("Sending to client: %u.%u.%u.%u\r\n", ip[0], ip[1], ip[2], ip[3]);
  state.client.writeFully("HTTP/1.1 200 OK\r\n"
                          "Connection: close\r\n"
                          "Content-Type: text/plain\r\n"
                          "\r\n"
                          "Hello, Client!\r\n");
  state.client.flush();

  // Half close the connection, per
  // [Tear-down](https://datatracker.ietf.org/doc/html/rfc7230#section-6.6)
  state.client.closeOutput();
  state.closedTime = millis();
  state.outputClosed = true;
}

// Main program loop.
void loop() {
  EthernetClient client = server.accept();
  if (client) {
    // We got a connection!
    IPAddress ip = client.remoteIP();
    printf("Client connected: %u.%u.%u.%u\r\n", ip[0], ip[1], ip[2], ip[3]);
    clients.emplace_back(std::move(client));
    printf("Client count: %zu\r\n", clients.size());
  }

  // Process data from each client
  for (ClientState &state : clients) {  // Use a reference so we don't copy
    if (!state.client.connected()) {
      state.closed = true;
      continue;
    }

    // Check if we need to force close the client
    if (state.outputClosed) {
      if (millis() - state.closedTime >= kShutdownTimeout) {
        IPAddress ip = state.client.remoteIP();
        printf("Client shutdown timeout: %u.%u.%u.%u\r\n",
               ip[0], ip[1], ip[2], ip[3]);
        state.client.close();
        state.closed = true;
        continue;
      }
    } else {
      if (millis() - state.lastRead >= kClientTimeout) {
        IPAddress ip = state.client.remoteIP();
        printf("Client timeout: %u.%u.%u.%u\r\n", ip[0], ip[1], ip[2], ip[3]);
        state.client.close();
        state.closed = true;
        continue;
      }
    }

    processClientData(state);
  }

  // Clean up all the closed clients
  size_t size = clients.size();
  clients.erase(std::remove_if(clients.begin(), clients.end(),
                               [](const auto &state) { return state.closed; }),
                clients.end());
  if (clients.size() != size) {
    printf("New client count: %zu\r\n", clients.size());
  }
}
