// SPDX-FileCopyrightText: (c) 2022-2023 Shawn Silverman <shawn@pobox.com>
// SPDX-License-Identifier: MIT

// IPerfServer implements an iPerf server for TCP traffic.
// Useful command: iperf -c <IP address> -i 1 -l 1460
// Other supported options:
// * -C (compatibility)
// * -r (tradeoff)
// * -d (dualtest)
//
// Specifying the -l (len) option with a value of 1460 appears to give
// better results when the server (the Teensy) is sending traffic back
// to the iperf client using the "tradeoff" option.
//
// With this command: `iperf -c <IP address> -i 1 -l 1460 -r`
// it appears that the QNEthernet stack can achieve about 94.9 Mbps in
// both directions.
//
// Multiple connections are supported.
//
// This code was inspired by "lwiperf" by Simon Goldschmidt:
// https://git.savannah.nongnu.org/cgit/lwip.git/tree/src?h=STABLE-2_1_3_RELEASE
//
// Other references:
// * Dan Drown's iPerf experiments (June 25, 2020):
//   [Teensy 4.1 ethernet](https://blog.dan.drown.org/teensy-4-1-ethernet/)
//
// This file is part of the QNEthernet library.

// C++ includes
#include <algorithm>
#include <cstdint>
#include <cstring>
#include <iterator>
#include <utility>
#include <vector>

#include <QNEthernet.h>
#include <lwip/opt.h>

using namespace qindesign::network;

// -------------------------------------------------------------------
//  Configuration
// -------------------------------------------------------------------

constexpr uint32_t kDHCPTimeout = 15'000;  // 15 seconds

constexpr uint16_t kServerPort = 5001;

// The settings are sent after every set of bytes of this size.
constexpr size_t kDefaultRepeatSize = 128 * 1024;  // 128 KiB

// -------------------------------------------------------------------
//  Types
// -------------------------------------------------------------------

enum Flags : uint32_t {
  kVersion1 = 0x80000000,
  kExtend   = 0x40000000,
  kRunNow   = 0x00000001,
};

// v1 header.
struct SettingsV1 {
  uint32_t flags;
  uint32_t numThreads;
  uint32_t port;
  uint32_t bufLen;
  uint32_t winBand;
  int32_t amount;  // Non-negative: bytes
                   // Negative: time (in centiseconds)

  // Fixes the endianness.
  void reorder() {
    flags = ntohl(flags);
    numThreads = ntohl(numThreads);
    port = ntohl(port);
    bufLen = ntohl(bufLen);
    winBand = ntohl(winBand);
    amount = ntohl(amount);
  }
};

// "Extended" header.
struct ExtSettings {
  // Extended fields
  int32_t type;
  int32_t length;
  uint16_t upperFlags;
  uint16_t lowerFlags;
  uint32_t versionUpper;
  uint32_t versionLower;
  uint16_t reserved;
  uint16_t tos;
  uint32_t rateLower;
  uint32_t rateUpper;
  uint32_t tcpWritePrefetch;

  // Fixes the endianness.
  void reorder() {
    type = ntohl(type);
    length = ntohl(length);
    upperFlags = ntohs(upperFlags);
    lowerFlags = ntohs(lowerFlags);
    versionUpper = ntohl(versionUpper);
    versionLower = ntohl(versionLower);
    reserved = ntohs(reserved);
    tos = ntohs(tos);
    rateLower = ntohl(rateLower);
    rateUpper = ntohl(rateUpper);
    tcpWritePrefetch = ntohl(tcpWritePrefetch);
  }
};

// This is the iPerf settings struct sent from the client.
struct Settings {
  SettingsV1 settingsV1;
  ExtSettings extSettings;
};

enum class IOStates {
  kReadSettingsV1,     // First settings
  kReadExtSettings,    // First settings
  kReadBlockSettings,  // Settings in front of a block
  kRead,
  kWrite,  // Clients use this state
};

// Keeps track of state for a single connection.
struct ConnectionState {
  ConnectionState(EthernetClient client, bool isClient)
      : remoteIP{client.remoteIP()},
        remotePort(client.remotePort()),
        client{std::move(client)},
        ioState(isClient ? IOStates::kWrite : IOStates::kReadSettingsV1) {}

  // Put these before the moved client
  IPAddress remoteIP;
  uint16_t remotePort;

  EthernetClient client;
  bool closed = false;

  IOStates ioState;

  Settings settings;
  uint8_t settingsRaw[sizeof(Settings)];  // For raw comparisons
                                          // without having to
                                          // consider ordering

  size_t settingsSize = 0;
  size_t repeatSize = kDefaultRepeatSize;
  size_t byteCount = 0;
  uint32_t startTime = millis();
};

// -------------------------------------------------------------------
//  Main Program
// -------------------------------------------------------------------

// Digits buffer.
uint8_t kDigitsBuf[TCP_SND_BUF + 10];

// Keeps track of what and where belong to whom.
std::vector<ConnectionState> conns;

// The server.
EthernetServer server{kServerPort};

// Forward declarations
void networkChanged(bool hasIP, bool linkState);
bool connectToClient(ConnectionState &state,
                     std::vector<ConnectionState> &list);
void processConnection(ConnectionState &state,
                       std::vector<ConnectionState> &list);

// Main program setup.
void setup() {
  Serial.begin(115200);
  while (!Serial && millis() < 4000) {
    // Wait for Serial to
  }
  printf("Starting IPerfServer...\r\n");

  uint8_t mac[6];
  Ethernet.macAddress(mac);  // This is informative; it retrieves, not sets
  printf("MAC = %02x:%02x:%02x:%02x:%02x:%02x\r\n",
         mac[0], mac[1], mac[2], mac[3], mac[4], mac[5]);

  // Listen for link changes
  Ethernet.onLinkState([](bool state) {
    if (state) {
      printf("[Ethernet] Link ON, %d Mbps, %s duplex\r\n",
             Ethernet.linkSpeed(),
             Ethernet.linkIsFullDuplex() ? "Full" : "Half");
    } else {
      printf("[Ethernet] Link OFF\r\n");
    }
    networkChanged(Ethernet.localIP() != INADDR_NONE, state);
  });

  // Listen for address changes
  Ethernet.onAddressChanged([]() {
    IPAddress ip = Ethernet.localIP();
    bool hasIP = (ip != INADDR_NONE);
    if (hasIP) {
      IPAddress subnet = Ethernet.subnetMask();
      IPAddress gw = Ethernet.gatewayIP();
      IPAddress dns = Ethernet.dnsServerIP();

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

    // Tell interested parties the network state, for example, servers,
    // SNTP clients, and other sub-programs that need to know whethe
    // to stop/start/restart/etc
    networkChanged(hasIP, Ethernet.linkState());
  });

  printf("Starting Ethernet with DHCP...\r\n");
  if (!Ethernet.begin()) {
    printf("Failed to start Ethernet\r\n");
    return;
  }

  // We don't really need to do the following because the
  // address-changed listener will notify us
  // if (!Ethernet.waitForLocalIP(kDHCPTimeout)) {
  //   printf("Failed to get IP address from DHCP\r\n");
  //   return;
  // }

  // Initialize the digits buffer
  for (size_t i = 0; i < sizeof(kDigitsBuf); i++) {
    kDigitsBuf[i] = (i % 10) + '0';
  }
}

// The address or link has changed. For example, a DHCP address arrived.
void networkChanged(bool hasIP, bool linkState) {
  if (!hasIP || !linkState) {
    return;
  }

  // Start the server and keep it up
  if (!server) {
    printf("Starting server on port %u...", kServerPort);
    fflush(stdout);  // Print what we have so far if line buffered
    server.begin();
    printf("%s\r\n", server ? "done." : "FAILED!");
  }
}

static inline bool isExtended(const ConnectionState &s) {
  return (s.settingsSize > 0) &&
         ((s.settings.settingsV1.flags &
           static_cast<uint32_t>(Flags::kExtend)) != 0);
}

static inline bool isV1(const ConnectionState &s) {
  return (s.settingsSize > 0) &&
         ((s.settings.settingsV1.flags &
           static_cast<uint32_t>(Flags::kVersion1)) != 0);
}

static inline bool isRunNow(const ConnectionState &s) {
  return (s.settingsSize > 0) &&
         ((s.settings.settingsV1.flags &
           static_cast<uint32_t>(Flags::kRunNow)) != 0);
}

static inline bool isClient(const ConnectionState &s) {
  return s.ioState == IOStates::kWrite;
}

// Main program loop.
void loop() {
  EthernetClient client = server.accept();
  if (client) {
    // We got a connection!
    IPAddress ip = client.remoteIP();
    uint16_t port = client.remotePort();
    conns.emplace_back(std::move(client), false);
    printf("Connected: %u.%u.%u.%u:%u\r\n", ip[0], ip[1], ip[2], ip[3], port);
    printf("Connection count: %u\r\n", conns.size());
  }

  std::vector<ConnectionState> list;  // Add new connections to here

  // Process data from each client
  for (ConnectionState &state : conns) {  // Use a reference so we don't copy
    if (!state.client.connected()) {
      printf("Disconnected: %u.%u.%u.%u:%u\r\n",
             state.remoteIP[0],
             state.remoteIP[1],
             state.remoteIP[2],
             state.remoteIP[3],
             state.remotePort);
      // First check to see if we need to open a connection back to
      // the client
      if (isV1(state) && !isRunNow(state)) {
        if (!isClient(state)) {
          connectToClient(state, list);
        }
      }
      state.closed = true;
      continue;
    }

    processConnection(state, list);
  }

  if (!list.empty()) {
    conns.insert(conns.end(),
                std::make_move_iterator(list.begin()),
                std::make_move_iterator(list.end()));
    list.clear();
  }

  // Clean up all the closed clients
  size_t size = conns.size();
  conns.erase(std::remove_if(conns.begin(), conns.end(),
                             [](const auto &state) { return state.closed; }),
              conns.end());
  if (conns.size() != size) {
    printf("Connection count: %zu\r\n", conns.size());
  }
}

// Connects back to the client and returns whether the connection was
// successful. This adds any new connection to the given list.
bool connectToClient(ConnectionState &state,
                     std::vector<ConnectionState> &list) {
  printf("Connecting back to client: %u.%u.%u.%u:%" PRIu32 "...",
         state.remoteIP[0],
         state.remoteIP[1],
         state.remoteIP[2],
         state.remoteIP[3],
         state.settings.settingsV1.port);

  EthernetClient client;
  if (!client.connect(state.remoteIP, state.settings.settingsV1.port)) {
    printf("FAILED.\r\n");
    return false;
  }
  printf("done.\r\n");

  list.emplace_back(std::move(client), true);
  ConnectionState &newState = list[list.size() - 1];
  newState.settings = state.settings;
  std::memcpy(newState.settingsRaw, state.settingsRaw, state.settingsSize);
  newState.settingsSize = state.settingsSize;
  newState.repeatSize = state.repeatSize;

  return true;
}

// Sends data until it can't fill the buffer.
void send(ConnectionState &state) {
  while (true) {
    if (state.settings.settingsV1.amount < 0) {
      // The session is time-limited
      uint32_t diff = millis() - state.startTime;
      uint32_t time = -state.settings.settingsV1.amount * 10;
          // Convert to milliseconds (from centiseconds)
      if (diff >= time) {
        printf("Closing client (time): %u.%u.%u.%u:%u\r\n",
               state.remoteIP[0],
               state.remoteIP[1],
               state.remoteIP[2],
               state.remoteIP[3],
               state.remotePort);
        state.client.close();
        return;
      }
    } else {
      // The session is byte-limited
      if (state.byteCount >=
          static_cast<size_t>(state.settings.settingsV1.amount)) {
        printf("Closing client (bytes): %u.%u.%u.%u:%u\r\n",
               state.remoteIP[0],
               state.remoteIP[1],
               state.remoteIP[2],
               state.remoteIP[3],
               state.remotePort);
        state.client.close();
        return;
      }
    }

    const uint8_t *buf;
    size_t len;

    int avail = state.client.availableForWrite();
    if (avail <= 0) {
      return;
    }
    size_t already;
    size_t size = std::min(state.settingsSize, state.repeatSize);
    if (state.byteCount < state.settingsSize) {
      already = state.byteCount;
    } else {
      already = (state.byteCount - state.settingsSize)%state.repeatSize;
    }
    if (already < size) {
      buf = &state.settingsRaw[already];
      len = size - already;
    } else {
      buf = &kDigitsBuf[already % 10];
      len = state.repeatSize - already;
    }
    len = std::min(len, static_cast<size_t>(avail));

    state.client.write(buf, len);
    state.byteCount += len;
  }
}

// Processes data from a single connection. This adds any new
// connections to the given list.
void processConnection(ConnectionState &state,
                       std::vector<ConnectionState> &list) {
  while (true) {
    switch (state.ioState) {
      case IOStates::kReadSettingsV1: {
        if (state.client.available() < static_cast<int>(sizeof(SettingsV1))) {
          return;
        }

        // Read a SettingsV1
        SettingsV1 s;
        state.client.read(state.settingsRaw, sizeof(SettingsV1));
        state.byteCount += sizeof(SettingsV1);
        std::memcpy(&s, state.settingsRaw, sizeof(SettingsV1));
        s.reorder();

        // Set up the state
        if (s.flags == 0x30313233) {
          state.settingsSize = 0;
          state.ioState = IOStates::kRead;
          printf("%u.%u.%u.%u:%u: Older version\r\n",
                 state.remoteIP[0],
                 state.remoteIP[1],
                 state.remoteIP[2],
                 state.remoteIP[3],
                 state.remotePort);
        } else {
          state.settings.settingsV1 = s;
          state.settingsSize = sizeof(SettingsV1);
          if (isExtended(state)) {
            state.settingsSize += sizeof(ExtSettings);
            state.ioState = IOStates::kReadExtSettings;
          } else {
            state.ioState = IOStates::kReadBlockSettings;
          }
          state.repeatSize = state.settings.settingsV1.bufLen;
          if (state.repeatSize == 0) {
            state.repeatSize = kDefaultRepeatSize;
          }
          if (isV1(state) && isRunNow(state)) {
            connectToClient(state, list);
          }

          printf("%u.%u.%u.%u:%u: Settings:\r\n"
                "    flags=0x%08" PRIx32 "\r\n"
                "    numThreads=%" PRIu32 "\r\n"
                "    port=%" PRIu32 "\r\n"
                "    bufLen=%" PRIu32 "\r\n"
                "    winBand=%" PRIu32 "\r\n"
                "    amount=%" PRId32 "\r\n",
                state.remoteIP[0],
                state.remoteIP[1],
                state.remoteIP[2],
                state.remoteIP[3],
                state.remotePort,
                s.flags, s.numThreads, s.port, s.bufLen, s.winBand, s.amount);
        }

        break;
      }

      case IOStates::kReadExtSettings: {
        if (state.client.available() < static_cast<int>(sizeof(ExtSettings))) {
          return;
        }

        // Read an ExtSettings
        ExtSettings s;
        state.client.read(state.settingsRaw + sizeof(SettingsV1),
                          sizeof(ExtSettings));
        state.byteCount += sizeof(ExtSettings);
        std::memcpy(&s, state.settingsRaw + sizeof(SettingsV1),
                    sizeof(ExtSettings));
        s.reorder();

        // Do more setup
        state.settings.extSettings = s;

        printf("%u.%u.%u.%u:%u: ExtSettings:\r\n"
              "    type=%" PRId32 "\r\n"
              "    length=%" PRId32 "\r\n"
              "    flags=0x%04" PRIu16 "%04" PRIu16 "\r\n"
              "    version=%u.%u.%u.%u\r\n"
              "    rate=%" PRIu64 "\r\n"
              "    tcpWritePrefetch=%" PRIu32 "\r\n",
              state.remoteIP[0],
              state.remoteIP[1],
              state.remoteIP[2],
              state.remoteIP[3],
              state.remotePort,
              s.type, s.length, s.upperFlags, s.lowerFlags,
              static_cast<uint16_t>(s.versionUpper >> 16),
              static_cast<uint16_t>(s.versionUpper),
              static_cast<uint16_t>(s.versionLower >> 16),
              static_cast<uint16_t>(s.versionLower),
              (static_cast<uint64_t>(s.rateUpper >> 8) << 32) |
                  static_cast<uint64_t>(s.rateLower),
              s.tcpWritePrefetch);

        state.ioState = IOStates::kReadBlockSettings;

        break;
      }

      case IOStates::kReadBlockSettings: {
        size_t size = std::min(state.settingsSize, state.repeatSize);
        if (state.client.available() < static_cast<int>(size)) {
          return;
        }

        // Read settings
        uint8_t buf[size];
        state.client.read(buf, size);
        state.byteCount += size;

        // Compare with the existing settings
        if (std::memcmp(buf, state.settingsRaw, size) != 0) {
          printf("%u.%u.%u.%u:%u: Settings error: bytes=%zu\r\n",
                 state.remoteIP[0],
                 state.remoteIP[1],
                 state.remoteIP[2],
                 state.remoteIP[3],
                 state.remotePort,
                 state.byteCount);
          state.client.close();
          return;
        }

        if (size != state.repeatSize) {  // Stay here otherwise
          state.ioState = IOStates::kRead;
        }
        break;
      }

      case IOStates::kRead: {
        // Assume: byteCount >= settingsSize

        int avail = state.client.available();
        if (avail <= 0) {
          return;
        }
        if (state.settingsSize > 0) {
          size_t rem = state.repeatSize -
                       (state.byteCount - state.settingsSize)%state.repeatSize;
          if (rem == state.repeatSize) {
            // Edge case where settingsSize >= repeatSize
            state.ioState = IOStates::kReadBlockSettings;
            break;
          }
          if (avail >= static_cast<int>(rem)) {
            avail = rem;
            state.ioState = IOStates::kReadBlockSettings;
          }
        }

        uint8_t buf[avail];
        state.client.read(buf, avail);
        state.byteCount += avail;
        break;
      }

      case IOStates::kWrite:
        send(state);
        return;
    }
  }
}
