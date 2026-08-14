// SPDX-FileCopyrightText: (c) 2021-2026 Shawn Silverman <shawn@pobox.com>
// SPDX-License-Identifier: AGPL-3.0-or-later

// SNTPClient demonstrates a simple SNTP client.
// See:
// * [Network Time Protocol Version 4: Protocol and Algorithms Specification](https://datatracker.ietf.org/doc/html/rfc5905)
// * [Simple Network Time Protocol (SNTP) Version 4 for IPv4, IPv6 and OSI](https://datatracker.ietf.org/doc/html/rfc4330)
//
// This file is part of the QNEthernet library.

// C includes
#include <sys/time.h>
// Assume settimeofday() exists

// C++ includes
#include <algorithm>
#include <array>
#include <cerrno>
#include <cmath>
#include <cstring>
#include <ctime>

#include <QNEthernet.h>
#include <lwip/def.h>

using namespace qindesign::network;

// --------------------------------------------------------------------------
//  Configuration
// --------------------------------------------------------------------------

constexpr uint32_t kDHCPTimeout = 15000;  // 15 seconds

constexpr uint16_t kNTPPort = 123;

constexpr uint32_t kNTPTimeout = 5000;

// --------------------------------------------------------------------------
//  Internal State and Functions
// --------------------------------------------------------------------------

namespace {  // Internal linkage section

// 01-Jan-1900 00:00:00 -> 01-Jan-1970 00:00:00
constexpr int64_t kEpochDiff = 2208988800;

// 2^32
constexpr double k2e32 = double{uint64_t{1} << 32};

// Converts an NTP timestamp to a double. The pivot helps determine the era and
// do proper wraparound.
double ntpToDouble(const uint8_t* const ts, const double pivot) {
  uint32_t secs;
  uint32_t frac;
  std::memcpy(&secs, &ts[0], 4);
  std::memcpy(&frac, &ts[4], 4);
  secs = ntohl(secs);
  frac = ntohl(frac);

  double t = static_cast<double>(secs) + (static_cast<double>(frac)/k2e32);

  // Choose the NTP era closest to the known local request time.
  return t + std::round((pivot - t)/k2e32)*k2e32;
}

// Retrieves the current NTP-based time, returning a quiet NaN if there was an
// error. This will also fill a non-NULL 'raw' with the raw timestamp bytes.
double currentTime(uint8_t* const raw) {
  ::timeval tv;
  if (::gettimeofday(&tv, nullptr) != 0) {
    // Something is wrong. This should always return zero.
    printf("Failed to get current time: errno=%d\r\n", errno);
    return std::numeric_limits<double>::quiet_NaN();
  }

  const auto secs = static_cast<int64_t>(tv.tv_sec) + kEpochDiff;
  const auto frac = static_cast<double>(tv.tv_usec) * 1e-6;

  if (raw != nullptr) {
    const uint32_t net_secs = htonl(static_cast<uint32_t>(secs));
    const uint32_t net_frac = htonl(static_cast<uint32_t>(frac * k2e32));

    (void)std::memcpy(&raw[0], &net_secs, 4);
    (void)std::memcpy(&raw[4], &net_frac, 4);
  }

  return static_cast<double>(secs) + frac;
}

// Checks if an NTP timestamp is all zero.
inline bool isZero(const uint8_t* const ts) {
  return std::all_of(&ts[0], &ts[8], [](const uint8_t b) { return b == 0; });
}

// UDP port.
EthernetUDP udp;

// Buffer.
std::array<uint8_t, 48> packet;

uint32_t lastSendTime = millis() - kNTPTimeout;
bool gotReply = false;

}  // namespace

// --------------------------------------------------------------------------
//  Main Program
// --------------------------------------------------------------------------

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

  // Start UDP listening on the NTP port
  if (!udp.begin(kNTPPort)) {
    printf("UDP listen error: %d\r\n", errno);
  }
}

// Sends an SNTP request and returns if successful.
static bool sendRequest() {
  std::fill(packet.begin(), packet.end(), 0);

#if __cplusplus < 201402L
  packet[0] = 0x23;
#else
  packet[0] = 0b00'100'011;  // LI=0, VN=4, Mode=3 (Client)
#endif  // C++ < 14

  // Get the current time as the Transmit Time
  if (std::isnan(currentTime(&packet[40]))) {
    return false;
  }

  // Send the packet
  // Note: If your gateway doesn't have an SNTP server — many routers do — then
  //       use something like "pool.ntp.org" or "time.nist.gov" instead
  printf("Sending SNTP request to the gateway...");
  if (!udp.send(Ethernet.gatewayIP(), kNTPPort, packet.data(), packet.size())) {
    printf("ERROR.\r\n");
    return false;
  }
  printf("Sent.\r\n");
  lastSendTime = millis();
  return true;

  // Alternative:
  // udp.beginPacket(Ethernet.gatewayIP(), kNTPPort);
  // udp.write(packet.data(), packet.size());
  // udp.endPacket();
}

// Main program loop.
void loop() {
  if (!gotReply && ((millis() - lastSendTime) >= kNTPTimeout)) {
    if (!sendRequest()) {
      // Don't re-send immediately
      lastSendTime = millis();
      return;
    }
  }

  int size = udp.parsePacket();
  if (size < 0) {
    return;
  }

  // Arrival timestamp
  const double t4 = currentTime(nullptr);
  if (std::isnan(t4)) {
    return;
  }

  if (((size != 48) && (size != 68)) || (udp.remotePort() != kNTPPort)) {
    return;
  }

  const uint8_t* const buf = udp.data();
  // Alternative:
  // if (udp.read(buf, 48) != 48) {
  //   printf("Not enough bytes\r\n");
  //   return;
  // }

   // [SNTP Client Operations](https://datatracker.ietf.org/doc/html/rfc4330#section-5)
  const unsigned int mode    = buf[0] & 0x07u;
  const unsigned int li      = (buf[0] & 0xc0u) >> 6;
  const unsigned int version = (buf[0] & 0x38u) >> 3;

  if ((li == 3) ||                        // LI == 3 (Unknown (clock unsynchronized))
      (buf[1] == 0) || (buf[1] >= 16) ||  // Stratum == 0 (Kiss-o'-Death)
      !((mode == 4) || (mode == 5)) ||    // Must be Server or Broadcast Server mode
      (version != 4)) {
    printf("Discarding reply\r\n");
    return;
  }

  if (isZero(&buf[40])) {
    printf("Discarding zero reply\r\n");
  }

  gotReply = true;

  // Transmit timestamp
  const double t3 = ntpToDouble(&buf[40], t4);
  double newTime = t3 - static_cast<double>(kEpochDiff);
  // Note: the new time should also add the one-way delay, if known or estimated

  // Set the RTC and time
  const auto secs = static_cast<time_t>(newTime);
  const auto usecs =
      static_cast<suseconds_t>((newTime - static_cast<double>(secs))*1e6);
  const ::timeval tv{secs, usecs};
  if (::settimeofday(&tv, nullptr) != 0) {
    printf("Error setting time: errno=%d\r\n", errno);
  } else {
    printf("Time was set\r\n");
  }

  // Print the time
  std::time_t t = tv.tv_sec;
  std::tm* tm = std::gmtime(&t);
  if (tm != nullptr) {
    printf("SNTP reply: %04d-%02d-%02d %02d:%02d:%02d (%" PRId32 " µs) UTC\r\n",
           tm->tm_year + 1900, tm->tm_mon + 1, tm->tm_mday,
           tm->tm_hour, tm->tm_min, tm->tm_sec,
           static_cast<int32_t>(tv.tv_usec));
  } else {
    printf("std::gmtime() failed!\r\n");
  }
}
