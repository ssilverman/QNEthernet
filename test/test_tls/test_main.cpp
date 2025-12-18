// SPDX-FileCopyrightText: (c) 2025 Shawn Silverman <shawn@pobox.com>
// SPDX-License-Identifier: AGPL-3.0-or-later

// test_main.cpp tests the TLS portion of the library.
// This file is part of the QNEthernet library.

#include <cstdio>
#include <memory>
#include <vector>

#include <Arduino.h>
#include <QNEthernet.h>
// #include <mbedtls/debug.h>
#include <qnethernet/QNDNSClient.h>
#include <qnethernet/MbedTLSClient.h>
// #include <qnethernet/security/MbedTLSCert.h>
#include <unity.h>

// #include "root_ca_der.h"

using namespace ::qindesign::network;

// --------------------------------------------------------------------------
//  Utilities
// --------------------------------------------------------------------------

// Formats into a char vector and returns the vector. The vector will always be
// terminated with a NUL. This returns an empty string if something goes wrong
// with the print function.
template <typename... Args>
std::vector<char> format(const char *format, Args... args) {
  std::vector<char> out;

  int size = std::snprintf(nullptr, 0, format, args...) + 1;  // Include the NUL
  if (size <= 1) {
    out.resize(1, 0);
  } else {
    out.resize(size);
    std::snprintf(out.data(), size, format, args...);
  }
  return out;
}

// --------------------------------------------------------------------------
//  Main Program
// --------------------------------------------------------------------------

// Timeouts
static constexpr uint32_t kDHCPTimeout = 30'000;
static constexpr uint32_t kConnectTimeout = 10'000;

// Registry of objects that need destruction, per test.
std::unique_ptr<EthernetClient> client;
std::unique_ptr<MbedTLSClient> tlsClient;

// Pre-test setup. This is run before every test.
void setUp() {
}

// Post-test teardown. This is run after every test.
void tearDown() {
  // Clean up any stray objects because of the longjmp
  if (tlsClient != nullptr) {
    tlsClient->setConnectionTimeoutEnabled(false);
    tlsClient->stop();
    tlsClient = nullptr;
  }
  if (client != nullptr) {
    client->setConnectionTimeoutEnabled(false);
    client->stop();
    client = nullptr;
  }

  // Remove any listeners before calling Ethernet.end()
  // This avoids accessing any out-of-scope variables
  Ethernet.onLinkState(nullptr);
  Ethernet.onAddressChanged(nullptr);
  Ethernet.onInterfaceStatus(nullptr);

  // Stop Ethernet and other services
  Ethernet.end();
  TEST_ASSERT_FALSE_MESSAGE(static_cast<bool>(Ethernet), "Expected stopped");
}

// Obtains an IP address via DHCP and returns whether successful.
static bool waitForLocalIP() {
  TEST_ASSERT_FALSE_MESSAGE(static_cast<bool>(Ethernet),
                            "Expected not started");
  TEST_ASSERT_FALSE_MESSAGE(Ethernet.isDHCPActive(), "Expected DHCP inactive");
  TEST_ASSERT_TRUE_MESSAGE(Ethernet.begin(), "Expected start with DHCP okay");
  TEST_ASSERT_TRUE_MESSAGE(static_cast<bool>(Ethernet), "Expected started");
  TEST_ASSERT_TRUE_MESSAGE(Ethernet.isDHCPActive(), "Expected DHCP active");

  TEST_MESSAGE("Waiting for DHCP...");
  uint32_t t = millis();
  bool result = Ethernet.waitForLocalIP(kDHCPTimeout);
  TEST_ASSERT_TRUE_MESSAGE(result, "Wait for IP failed");
  TEST_MESSAGE(format("DHCP time: %" PRIu32 "ms", millis() - t).data());
  if (!result) {
    return false;
  }

  IPAddress ip = Ethernet.localIP();
  IPAddress netmask = Ethernet.subnetMask();
  IPAddress gateway = Ethernet.gatewayIP();
  IPAddress dns = Ethernet.dnsServerIP();
  TEST_ASSERT_MESSAGE(ip != INADDR_NONE, "Expected valid IP");
  TEST_MESSAGE(format("DHCP IP:      %u.%u.%u.%u",
                      ip[0], ip[1], ip[2], ip[3]).data());
  TEST_MESSAGE(format("     Netmask: %u.%u.%u.%u",
                      netmask[0], netmask[1], netmask[2], netmask[3]).data());
  TEST_MESSAGE(format("     Gateway: %u.%u.%u.%u",
                      gateway[0], gateway[1], gateway[2], gateway[3]).data());
  TEST_MESSAGE(format("     DNS:     %u.%u.%u.%u",
                      dns[0], dns[1], dns[2], dns[3]).data());
  for (int i = 0; i < DNSClient::maxServers(); ++i) {
    dns = DNSClient::getServer(i);
    TEST_MESSAGE(format("DNS Server %d: %u.%u.%u.%u", i,
                        dns[0], dns[1], dns[2], dns[3]).data());
  }

  return true;
}

// Tests a TLS client.
template <bool ClientNonBlocking, bool TLSNonBlocking>
static void test_client() {
#define HOST "www.google.com"
  constexpr char kHost[]{HOST};
  constexpr char kRequest[]{
      "HEAD / HTTP/1.1\r\n"
      "Host: " HOST "\r\n"
      "Connection: close\r\n"
      "\r\n"
  };
#undef HOST
  constexpr uint16_t kPort = 443;

  if (!waitForLocalIP()) {
    return;
  }

  client = std::make_unique<EthernetClient>();
  if (ClientNonBlocking) {
    client->setConnectionTimeoutEnabled(false);
  } else {
    client->setConnectionTimeout(kConnectTimeout);
  }

  tlsClient = std::make_unique<MbedTLSClient>(*client);
  if (TLSNonBlocking) {
    tlsClient->setConnectionTimeoutEnabled(false);
  } else {
    tlsClient->setConnectionTimeout(kConnectTimeout);
    TEST_ASSERT_EQUAL_MESSAGE(kConnectTimeout, tlsClient->connectionTimeout(),
                              "Expected set timeout");
  }

  TEST_ASSERT_FALSE_MESSAGE(static_cast<bool>(*tlsClient),
                            "Expected not connected");
  TEST_ASSERT_EQUAL_MESSAGE(false, tlsClient->connected(),
                            "Expected not connected (no data)");

  // // Certificates
  // // mbedtls_debug_set_threshold(3);
  // qindesign::security::MbedTLSCert ca;
  // if (!kRootCAIndexes.empty()) {
  //   for (size_t i = 0; i < kRootCAIndexes.size() - 1; ++i) {
  //     ca.parseDERNoCopy(&kRootCAs[i],
  //                       kRootCAIndexes[i + 1] - kRootCAIndexes[i]);
  //   }
  // }
  // tlsClient->setCACert(&ca);

  // Connect and send the request
  TEST_MESSAGE("Connecting and sending HTTP HEAD request...");
  uint32_t t = millis();
  TEST_ASSERT_EQUAL_MESSAGE(true, tlsClient->connect(kHost, kPort),
                            "Expected connect success");

  if (TLSNonBlocking) {
    uint32_t start = millis();
    while (!static_cast<bool>(*tlsClient) &&
           millis() - start < kConnectTimeout) {
      yield();
    }
  }

  TEST_ASSERT_TRUE_MESSAGE(static_cast<bool>(*tlsClient), "Expected connected");
  TEST_ASSERT_EQUAL_MESSAGE(true, tlsClient->connected(),
                            "Expected connected (or data)");
  TEST_MESSAGE(format("Lookup and connect time: %" PRIu32 "ms",
                      millis() - t).data());
  TEST_ASSERT_EQUAL_MESSAGE(std::strlen(kRequest),
                            tlsClient->writeFully(kRequest),
                            "Expected written fully");
  tlsClient->flush();

  // Read the response
  t = millis();
  TEST_MESSAGE("The response:");
  while (tlsClient->connected()) {
    int avail = tlsClient->available();
    if (avail <= 0) {
      continue;
    }
    for (int i = 0; i < avail; ++i) {
      UNITY_OUTPUT_CHAR(tlsClient->read());
    }
    UNITY_OUTPUT_FLUSH();
  }
  UNITY_PRINT_EOL();
  TEST_MESSAGE(format("Read and print response time: %" PRIu32 "ms",
                      millis() - t).data());

  TEST_ASSERT_EQUAL_MESSAGE(false, tlsClient->connected(),
                            "Expected not connected (no more data)");
  TEST_ASSERT_FALSE_MESSAGE(static_cast<bool>(*tlsClient),
                            "Expected not connected");
}

// Main program setup.
void setup() {
  Serial.begin(115200);
  while (!Serial && millis() < 4000) {
    // Wait for Serial
  }

#if QNETHERNET_CUSTOM_WRITE
  stdoutPrint = &Serial;
  stderrPrint = &Serial;
#endif  // QNETHERNET_CUSTOM_WRITE

  // NOTE!!! Wait for >2 secs
  // if board doesn't support software reset via Serial.DTR/RTS
  delay(2000);

#if defined(TEENSYDUINO)
  if (CrashReport) {
    Serial.println(CrashReport);
  }
#endif  // defined(TEENSYDUINO)

  UNITY_BEGIN();
  RUN_TEST((test_client<false, false>));
  RUN_TEST((test_client<true, false>));
  RUN_TEST((test_client<false, true>));
  RUN_TEST((test_client<true, true>));
  UNITY_END();
}

// Main program loop.
void loop() {
}
