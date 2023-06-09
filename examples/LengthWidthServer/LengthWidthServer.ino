// SPDX-FileCopyrightText: (c) 2021-2023 Shawn Silverman <shawn@pobox.com>
// SPDX-License-Identifier: MIT

// LengthWidthServer demonstrates how to serve a protocol having a
// continuous stream of messages from multiple clients, where each
// message starts with a one-byte length field. This is similar to the
// FixedWidthServer example, but the data stream indicates the size of
// each message. It shows a simple version of how to use states when
// parsing, when the position in the client stream is arbitrary.
//
// This file is part of the QNEthernet library.

// C++ includes
#include <algorithm>
#include <utility>
#include <vector>

#include <QNEthernet.h>

using namespace qindesign::network;

// --------------------------------------------------------------------------
//  Configuration
// --------------------------------------------------------------------------

constexpr uint32_t kDHCPTimeout = 15'000;  // 15 seconds

constexpr uint16_t kServerPort = 5000;

// --------------------------------------------------------------------------
//  Types
// --------------------------------------------------------------------------

// Where are we with message parsing?
enum class MessageParseState {
  kStart,  // Starting state
  kValue,  // Reading the value
};

// Keeps track of state for a single client.
struct ClientState {
  ClientState(EthernetClient client)
      : client(std::move(client)) {
    reset();
  }

  EthernetClient client;
  bool closed = false;

  MessageParseState parseState;
  int messageSize;   // The current message size
  int bufSize;       // Keeps track of how many bytes have been read
  uint8_t buf[255];  // Do the easy thing and allocate the maximum possible

  // Reset the client state.
  void reset() {
    messageSize = 0;
    bufSize = 0;
    parseState = MessageParseState::kStart;
  }
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

// Process one message. This implementation simply prints to Serial,
// escaping some characters.
//
// We could pass just the buffer, but we're passing the whole state
// here so we know which client it's from.
void processMessage(const ClientState &state) {
  printf("Message [%d]: ", state.messageSize);
  for (int i = 0; i < state.messageSize; i++) {
    uint8_t b = state.buf[i];
    if (b < 0x20) {
      switch (b) {
        case '\a': printf("\\q"); break;
        case '\b': printf("\\b"); break;
        case '\t': printf("\\t"); break;
        case '\n': printf("\\n"); break;
        case '\v': printf("\\v"); break;
        case '\f': printf("\\f"); break;
        case '\r': printf("\\r"); break;
        case '\\': printf("\\\\"); break;
        default:
          printf("\\x%x%x", (b >> 4) & 0x0f, b & 0x0f);
      }
    } else if (0x7f <= b && b < 0xa0) {
      printf("\\x%x%x", (b >> 4) & 0x0f, b & 0x0f);
    } else {
      printf("%c", b);
    }
  }
  printf("\r\n");
}

// Main program loop.
void loop() {
  EthernetClient client = server.accept();
  if (client) {
    IPAddress ip = client.remoteIP();
    printf("Client connected: %u.%u.%u.%u", ip[0], ip[1], ip[2], ip[3]);
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
    while (avail > 0) {
      switch (state.parseState) {
        case MessageParseState::kStart:
          state.messageSize = state.client.read();
          avail--;
          state.parseState = MessageParseState::kValue;
          break;

        case MessageParseState::kValue: {
          int read = std::min(state.messageSize - state.bufSize, avail);
          read = state.client.read(&state.buf[state.bufSize], read);
          state.bufSize += read;
          avail -= read;
          if (state.bufSize >= state.messageSize) {
            processMessage(state);
            state.messageSize = 0;
            state.bufSize = 0;
            state.parseState = MessageParseState::kStart;
          }
          break;
        }

        default:
          break;  // Shouldn't happen because we put in all the states
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
