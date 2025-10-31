// SPDX-FileCopyrightText: (c) 2025 Shawn Silverman <shawn@pobox.com>
// SPDX-License-Identifier: AGPL-3.0-or-later

// Ping demonstrates a simple Ping program.
// See: https://en.wikipedia.org/wiki/Ping_(networking_utility)
//
// In order to use this example, define the QNETHERNET_ENABLE_PING macro.
//
// Note: the configuration macros must either be defined in the project build
//       options or in the qnethernet_opts.h library file.
//
// This file is part of the QNEthernet library.

#include <array>
#include <cerrno>
#include <utility>

#include <QNEthernet.h>
#include <qnethernet/QNPing.h>

using namespace qindesign::network;

// --------------------------------------------------------------------------
//  Configuration
// --------------------------------------------------------------------------

constexpr uint32_t kDHCPTimeout = 15'000;  // 15 seconds

constexpr uint32_t kPingInterval = 1'000;  // 1 second

constexpr char kHostname[]{"arduino.cc"};

constexpr uint8_t kPingTTL = 64;
constexpr uint16_t kPingId = 0x514E;
constexpr size_t kPayloadSize = 56;

// Create a payload containing incrementing bytes up to the payload
// size. For example, if the payload size is 3, then the array will
// contain {0, 1, 2}.
// This is a fancy alternative to just filling in a byte array.
template <typename T, size_t N, T... I>
constexpr std::array<T, N> make_seq_array(std::integer_sequence<T, I...>) {
  return {{I...}};
}
constexpr auto kPayload = make_seq_array<uint8_t, kPayloadSize>(
    std::make_integer_sequence<uint8_t, kPayloadSize>{});

// --------------------------------------------------------------------------
//  Program State
// --------------------------------------------------------------------------

static bool running = false;  // Whether the program is still running

static IPAddress pingIP;
static elapsedMillis pingTimer = kPingInterval;  // Start expired

static uint16_t seq = 0;        // Current sequence number
static bool doNextSend = true;  // Indicates if ready to send the next ping

// --------------------------------------------------------------------------
//  Main Program
// --------------------------------------------------------------------------

// The Echo Reply callback.
static void replyCallback(const PingData &reply) {
  // Check that the ID is ours
  if (reply.id != kPingId) {
    return;
  }

  // Print the ping result
  // Add 8 to the data size to print the whole ICMP packet size
  printf("%zu bytes from %u.%u.%u.%u: seq=%" PRIu16 " ttl=%u time=%lu ms\r\n",
         reply.dataSize + 8, reply.ip[0], reply.ip[1], reply.ip[2], reply.ip[3],
         reply.seq, reply.ttl, static_cast<unsigned long>(pingTimer));

  doNextSend = true;
}

// Ping object, for sending and receiving echo requests and replies.
static Ping ping{&replyCallback};

// Program setup.
void setup() {
  Serial.begin(115200);
  while (!Serial && millis() < 4000) {
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
  if (!running || (pingTimer < kPingInterval) || !doNextSend) {
    return;
  }

  doNextSend = false;
  const PingData req{.ip       = pingIP,
                     .ttl      = kPingTTL,
                     .id       = kPingId,
                     .seq      = seq++,
                     .data     = kPayload.data(),
                     .dataSize = kPayload.size()};
  if (ping.send(req)) {
    pingTimer = 0;
  } else {
    printf("Error sending: errno=%d\r\n", errno);
    running = false;
  }
}
