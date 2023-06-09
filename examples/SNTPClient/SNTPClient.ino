// SPDX-FileCopyrightText: (c) 2021-2023 Shawn Silverman <shawn@pobox.com>
// SPDX-License-Identifier: MIT

// SNTPClient demonstrates a simple SNTP client.
// See: https://tools.ietf.org/html/rfc4330
//
// This file is part of the QNEthernet library.

#include <QNEthernet.h>
#include <TimeLib.h>

using namespace qindesign::network;

// --------------------------------------------------------------------------
//  Configuration
// --------------------------------------------------------------------------

constexpr uint32_t kDHCPTimeout = 15'000;  // 15 seconds

constexpr uint16_t kNTPPort = 123;

// 01-Jan-1900 00:00:00 -> 01-Jan-1970 00:00:00
constexpr uint32_t kEpochDiff = 2'208'988'800;

// Epoch -> 07-Feb-2036 06:28:16
constexpr uint32_t kBreakTime = 2'085'978'496;

// --------------------------------------------------------------------------
//  Program State
// --------------------------------------------------------------------------

// UDP port.
EthernetUDP udp;

// Buffer.
uint8_t buf[48];

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

  // Start UDP listening on the NTP port
  udp.begin(kNTPPort);

  // Send an SNTP request

  memset(buf, 0, 48);
  buf[0] = 0b00'100'011;  // LI=0, VN=4, Mode=3 (Client)

  // Set the Transmit Timestamp
  uint32_t t = Teensy3Clock.get();
  if (t >= kBreakTime) {
    t -= kBreakTime;
  } else {
    t += kEpochDiff;
  }
  buf[40] = t >> 24;
  buf[41] = t >> 16;
  buf[42] = t >> 8;
  buf[43] = t;

  // Send the packet
  printf("Sending SNTP request to the gateway...");
  if (!udp.send(Ethernet.gatewayIP(), kNTPPort, buf, 48)) {
    printf("ERROR.");
  }
  printf("\r\n");

  // Alternative:
  // udp.beginPacket(Ethernet.gatewayIP(), kNTPPort);
  // udp.write(buf, 48);
  // udp.endPacket();
}

// Main program loop.
void loop() {
  int size = udp.parsePacket();
  if (size != 48 && size != 68) {
    return;
  }

  const uint8_t *buf = udp.data();
  // Alternative:
  // if (udp.read(buf, 48) != 48) {
  //   printf("Not enough bytes\r\n");
  //   return;
  // }

  // See: Section 5, "SNTP Client Operations"
  int mode = buf[0] & 0x07;
  if (((buf[0] & 0xc0) == 0xc0) ||  // LI == 3 (Alarm condition)
      (buf[1] == 0) ||              // Stratum == 0 (Kiss-o'-Death)
      !(mode == 4 || mode == 5)) {  // Must be Server or Broadcast mode
    printf("Discarding reply\r\n");
    return;
  }

  uint32_t t = (uint32_t{buf[40]} << 24) |
               (uint32_t{buf[41]} << 16) |
               (uint32_t{buf[42]} << 8) |
               uint32_t{buf[43]};
  if (t == 0) {
    printf("Discarding reply\r\n");
    return;  // Also discard when the Transmit Timestamp is zero
  }
  if ((t & 0x80000000U) == 0) {
    // See: Section 3, "NTP Timestamp Format"
    t += kBreakTime;
  } else {
    t -= kEpochDiff;
  }

  // Set the RTC and time
  Teensy3Clock.set(t);
  setTime(t);

  // Print the time
  tmElements_t tm;
  breakTime(t, tm);
  printf("SNTP reply: %04u-%02u-%02u %02u:%02u:%02u\r\n",
         tm.Year + 1970, tm.Month, tm.Day,
         tm.Hour, tm.Minute, tm.Second);
}
