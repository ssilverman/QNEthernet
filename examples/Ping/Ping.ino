// SPDX-FileCopyrightText: (c) 2025-2026 Shawn Silverman <shawn@pobox.com>
// SPDX-License-Identifier: AGPL-3.0-or-later

// Ping demonstrates a ping program. This uses the echo packet's
// payload to hold the ping counter.
// See: https://en.wikipedia.org/wiki/Ping_(networking_utility)
//
// In order to use this example, define the `QNETHERNET_ENABLE_PING_SEND` macro
// to `1`, either externally or in the _qnethernet_opts.h_ file.
//
// Note: the configuration macros must either be defined in the project build
//       options or in the qnethernet_opts.h library file.
//
// This file is part of the QNEthernet library.

#include <algorithm>
#include <array>
#include <cerrno>
#include <cinttypes>
#include <cstdint>
#include <cstring>

#include <QNEthernet.h>
#include <qnethernet/QNPing.h>

using namespace qindesign::network;

// --------------------------------------------------------------------------
//  Configuration
// --------------------------------------------------------------------------

constexpr uint32_t kDHCPTimeout = 15000;  // 15 seconds

constexpr unsigned long kPingInterval = 1000;  // 1 second

constexpr char kHostname[]{"arduino.cc"};

constexpr uint8_t kPingTTL = 64;
constexpr uint16_t kPingId = 0x514E;
constexpr size_t kPayloadSize = 56;
static_assert(kPayloadSize >= 4, "Payload size must be >= 4");

// --------------------------------------------------------------------------
//  Program State
// --------------------------------------------------------------------------

static bool running = false;  // Whether the program is still running

static IPAddress pingIP;
static unsigned long pingTimer = millis() - kPingInterval;  // Start expired
static uint32_t pingCounter = 0;

static std::array<uint8_t, kPayloadSize> payload;

static uint16_t seq = 0;            // Current sequence number
static bool replyReceived = false;  // Indicates if the current reply has
                                    // been received

// --------------------------------------------------------------------------
//  Main Program
// --------------------------------------------------------------------------

// The Echo Reply callback.
static void replyCallback(const PingData& reply) {
  // Check that the ID is ours
  if (reply.id != kPingId) {
    return;
  }

  // Check that the payload data matches
  bool payloadMatches =
      (reply.dataSize == kPayloadSize) &&
      std::equal(&reply.data[4], &reply.data[kPayloadSize], &payload[4]);
  uint32_t counter;
  std::memcpy(&counter, &reply.data[0], 4);
  counter = ntohl(counter);  // Correct the byte order

  // Print the ping result
  // Add 8 to the data size to print the whole ICMP packet size
  printf("%" PRIu32 ". %zu bytes from %u.%u.%u.%u:"
         " seq=%" PRIu16 " ttl=%u time=%lu ms%s\r\n",
         counter, reply.dataSize + 8,
         reply.ip[0], reply.ip[1], reply.ip[2], reply.ip[3],
         reply.seq, reply.ttl, millis() - pingTimer,
         payloadMatches ? "" : " (payload mismatch)");

  replyReceived = true;
}

// Ping object, for sending and receiving echo requests and replies.
static Ping ping{&replyCallback};

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
  printf("Waiting for local IP...\r\n");
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

  // Fill the payload with some data
  // It could also be random data or something else
  for (size_t i = 0; i < kPayloadSize; ++i) {
    payload[i] = i;
  }

  // Look up the hostname
  printf("Looking up \"%s\"...", kHostname);
  if (Ethernet.hostByName(kHostname, pingIP)) {
    printf("\r\nIP = %u.%u.%u.%u\r\n",
           pingIP[0], pingIP[1], pingIP[2], pingIP[3]);
    running = true;
  } else {
    printf("errno=%d\r\n", errno);
  }
}

// Main program loop.
void loop() {
  if (!running || ((millis() - pingTimer) < kPingInterval)) {
    return;
  }

  if (!replyReceived && (pingCounter != 0)) {
    printf("%" PRIu32 ". Timeout\r\n", pingCounter);
  }
  replyReceived = false;

  const PingData req{.ip       = pingIP,
                     .ttl      = kPingTTL,
                     .id       = kPingId,
                     .seq      = seq++,
                     .data     = payload.data(),
                     .dataSize = payload.size()};

  // Track the ping counter in the payload data
  uint32_t counter = htonl(++pingCounter);  // Send in network order
  std::memcpy(payload.data(), &counter, 4);

  if (ping.send(req)) {
    pingTimer = millis();
  } else {
    printf("Error sending: errno=%d\r\n", errno);
    running = false;
  }
}
