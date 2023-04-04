// SPDX-FileCopyrightText: (c) 2023 Shawn Silverman <shawn@pobox.com>
// SPDX-License-Identifier: MIT

// test_main.cpp tests the library.
// This file is part of the QNEthernet library.

#include <algorithm>
#include <vector>

#include <Arduino.h>
#include <QNEthernet.h>
#include <QNDNSClient.h>
#include <TimeLib.h>
#include <lwip_t41.h>
#include <lwip/dns.h>
#include <lwip/opt.h>
#include <unity.h>

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
static constexpr uint32_t kDHCPTimeout = 20000;
static constexpr uint32_t kLinkTimeout = 5000;
static constexpr uint32_t kSNTPTimeout = 20000;
static constexpr uint32_t kConnectTimeout = 5000;

static constexpr uint32_t kDNSLookupTimeout =
    DNS_MAX_RETRIES * DNS_TMR_INTERVAL;

// Default static IP configuration
static const IPAddress kStaticIP  {192, 168, 0, 2};
static const IPAddress kSubnetMask{255, 255, 255, 0};
static const IPAddress kGateway   {192, 168, 0, 1};

// Test hostname for mDNS and DHCP option 12.
static constexpr char kTestHostname[]{"test-hostname"};

// Pre-test setup. This is run before every test.
void setUp() {
}

// Post-test teardown. This is run after every test.
void tearDown() {
  // Stop Ethernet and other services
  Ethernet.end();

  // Restore the MAC address
  uint8_t mac[6];
  uint8_t mac2[6];
  enet_getmac(mac);
  Ethernet.macAddress(mac2);
  if (!std::equal(&mac[0], &mac[6], mac2)) {
    Ethernet.setMACAddress(mac);
  }

  // Restore to no hostname
  Ethernet.setHostname(nullptr);

  // Remove any listeners
  Ethernet.onLinkState(nullptr);
  Ethernet.onAddressChanged(nullptr);
  Ethernet.onInterfaceStatus(nullptr);
}

// Tests using the built-in MAC address.
static void test_builtin_mac() {
  static constexpr uint8_t zeros[6]{0, 0, 0, 0, 0, 0};

  // Get the built-in MAC address
  uint8_t mac[6]{0, 0, 0, 0, 0, 0};
  enet_getmac(mac);
  TEST_ASSERT_FALSE_MESSAGE(std::equal(&mac[0], &mac[6], zeros), "Expected an internal MAC");

  uint8_t mac2[6]{1};
  Ethernet.macAddress(mac2);
  TEST_ASSERT_EQUAL_UINT8_ARRAY_MESSAGE(mac, mac2, 6, "Expected matching MAC");
  std::fill_n(mac2, 6, 0);
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wdeprecated-declarations"
  Ethernet.MACAddress(mac2);
  TEST_ASSERT_EQUAL_UINT8_ARRAY_MESSAGE(mac, mac2, 6, "Expected matching MAC (deprecated API)");
#pragma GCC diagnostic pop
}

// Tests setting the MAC address.
static void test_set_mac() {
  uint8_t mac[6];
  Ethernet.macAddress(mac);

  uint8_t mac2[6]{0x01, 0x02, 0x03, 0x04, 0x05, 0x06};
  TEST_ASSERT_FALSE_MESSAGE(std::equal(&mac[0], &mac[6], mac2), "Expected internal MAC");
  Ethernet.setMACAddress(mac2);
  Ethernet.macAddress(mac);
  TEST_ASSERT_EQUAL_UINT8_ARRAY_MESSAGE(mac, mac2, 6, "Expected matching MAC");
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wdeprecated-declarations"
  Ethernet.MACAddress(mac);
  TEST_ASSERT_EQUAL_UINT8_ARRAY_MESSAGE(mac, mac2, 6, "Expected matching MAC (deprecated API)");
#pragma GCC diagnostic pop
}

// Obtains an IP address via DHCP.
static void waitForLocalIP() {
  TEST_ASSERT_FALSE_MESSAGE(Ethernet.isDHCPActive(), "Expected DHCP inactive");
  TEST_ASSERT_TRUE_MESSAGE(Ethernet.begin(), "Expected start with DHCP okay");
  TEST_ASSERT_TRUE_MESSAGE(Ethernet.isDHCPActive(), "Expected DHCP active");

  TEST_MESSAGE("Waiting for DHCP...");
  uint32_t t = millis();
  TEST_ASSERT_TRUE_MESSAGE(Ethernet.waitForLocalIP(kDHCPTimeout), "Wait for IP failed");
  TEST_MESSAGE(format("DHCP time: %" PRIu32 "ms", millis() - t).data());

  IPAddress ip = Ethernet.localIP();
  TEST_ASSERT_MESSAGE(ip != INADDR_NONE, "Expected valid IP");
  TEST_MESSAGE(format("DHCP IP: %u.%u.%u.%u", ip[0], ip[1], ip[2], ip[3]).data());
}

// Tests DHCP.
static void test_dhcp() {
  TEST_ASSERT_MESSAGE(Ethernet.localIP() == INADDR_NONE, "Expected invalid IP");
  waitForLocalIP();
}

// Tests using a static IP.
void test_static_ip() {
  TEST_ASSERT_FALSE_MESSAGE(Ethernet.isDHCPActive(), "Expected inactive DHCP before start");
  TEST_ASSERT_MESSAGE(Ethernet.localIP() == INADDR_NONE, "Expected no local IP before start");

  // Without a DNS server
  TEST_ASSERT_TRUE_MESSAGE(Ethernet.begin(kStaticIP, kSubnetMask, kGateway), "Expected start success (1)");
  TEST_ASSERT_FALSE_MESSAGE(Ethernet.isDHCPActive(), "Expected inactive DHCP (1)");
  TEST_ASSERT_MESSAGE(Ethernet.localIP() == kStaticIP, "Expected matching local IP (1)");
  TEST_ASSERT_MESSAGE(Ethernet.subnetMask() == kSubnetMask, "Expected matching subnet mask (1)");
  TEST_ASSERT_MESSAGE(Ethernet.gatewayIP() == kGateway, "Expected matching gateway (1)");
  TEST_ASSERT_MESSAGE(Ethernet.dnsServerIP() == INADDR_NONE, "Expected unset DNS (1)");
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wdeprecated-declarations"
  Ethernet.setDnsServerIP(kGateway);
#pragma GCC diagnostic pop
  TEST_ASSERT_MESSAGE(Ethernet.dnsServerIP() == kGateway, "Expected gateway as DNS after set (deprecated API)");
  Ethernet.setDNSServerIP(INADDR_NONE);
  TEST_ASSERT_MESSAGE(Ethernet.dnsServerIP() == INADDR_NONE, "Expected unset DNS after set");

  const IPAddress ip{192, 168, 1, 3};
  Ethernet.setLocalIP(ip);
  TEST_ASSERT_MESSAGE(Ethernet.localIP() == ip, "Expected matching local IP after set new");
  Ethernet.setLocalIP(kStaticIP);
  TEST_ASSERT_MESSAGE(Ethernet.localIP() == kStaticIP, "Expected matching local IP after set static");

  // With a DNS server
  TEST_ASSERT_TRUE_MESSAGE(Ethernet.begin(kStaticIP, kSubnetMask, kGateway, kGateway), "Expected start success (2)");
  TEST_ASSERT_FALSE_MESSAGE(Ethernet.isDHCPActive(), "Expected inactive DHCP (2)");
  TEST_ASSERT_MESSAGE(Ethernet.localIP() == kStaticIP, "Expected matching local IP (2)");
  TEST_ASSERT_MESSAGE(Ethernet.subnetMask() == kSubnetMask, "Expected matching subnet mask (2)");
  TEST_ASSERT_MESSAGE(Ethernet.gatewayIP() == kGateway, "Expected matching gateway (2)");
  TEST_ASSERT_MESSAGE(Ethernet.dnsServerIP() == kGateway, "Expecting matching DNS (2)");
}

// Tests mDNS.
static void test_mdns() {
  waitForLocalIP();
  TEST_MESSAGE(format("Starting mDNS: %s", kTestHostname).data());
  TEST_ASSERT_TRUE_MESSAGE(MDNS.begin(kTestHostname), "Expected start success");

  TEST_ASSERT_MESSAGE(MDNS.hostname() == String{kTestHostname}, "Expected matching hostname");
}

// Tests DNS lookup.
static void test_dns_lookup() {
  waitForLocalIP();

  constexpr char kName[]{"dns.google"};
  const IPAddress ip1{8, 8, 8, 8};
  const IPAddress ip2{8, 8, 4, 4};
  // Alternative: one.one.one.one: 1.1.1.1, 1.0.0.1

  TEST_MESSAGE(format("Waiting for DNS lookup [%s]...", kName).data());
  IPAddress ip;
  uint32_t t = millis();
  TEST_ASSERT_TRUE_MESSAGE(DNSClient::getHostByName(kName, ip, kDNSLookupTimeout), "Expected lookup success");
  TEST_MESSAGE(format("Lookup time: %" PRIu32 "ms", millis() - t).data());
  TEST_MESSAGE(format("IP: %u.%u.%u.%u", ip[0], ip[1], ip[2], ip[3]).data());
  TEST_ASSERT_MESSAGE((ip == ip1) || (ip == ip2), "Expected different IP address");

  constexpr char kName2[]{"dms.goomgle"};
  TEST_MESSAGE(format("Waiting for DNS lookup [%s]...", kName2).data());
  t = millis();
  TEST_ASSERT_FALSE(DNSClient::getHostByName(kName2, ip, kDNSLookupTimeout));
  t = millis() - t;
  TEST_MESSAGE(format("Lookup time: %" PRIu32 "ms", t).data());
  TEST_ASSERT_MESSAGE(t < kDNSLookupTimeout, "Expected no timeout");
}

// Tests setting and getting the option 12 hostname.
static void test_hostname() {
  TEST_ASSERT_MESSAGE(Ethernet.hostname().length() == 0, "Expected no hostname");
  Ethernet.setHostname(kTestHostname);
  TEST_ASSERT_MESSAGE(Ethernet.hostname() == kTestHostname, "Expected set hostname");
}

// Tests hardware type.
static void test_hardware() {
  if (!Ethernet.begin(kStaticIP, kSubnetMask, kGateway)) {
    TEST_ASSERT_MESSAGE(Ethernet.hardwareStatus() == EthernetNoHardware, "Expected no hardware");
  } else {
    TEST_ASSERT_MESSAGE(Ethernet.hardwareStatus() == EthernetOtherHardware, "Expected other hardware");
  }
}

// Waits for a link.
static void waitForLink() {
  TEST_ASSERT_FALSE_MESSAGE(Ethernet.linkState(), "Expected link down");

  TEST_MESSAGE("Waiting for link...");
  uint32_t t = millis();
  TEST_ASSERT_TRUE_MESSAGE(Ethernet.waitForLink(kLinkTimeout), "Wait for link failed");
  TEST_MESSAGE(format("Link time: %" PRIu32 "ms", millis() - t).data());

  TEST_ASSERT_TRUE_MESSAGE(Ethernet.linkState(), "Expected link up");
}

// Tests seeing a link.
static void test_link() {
  TEST_ASSERT(Ethernet.linkStatus() == LinkOFF);
  Ethernet.begin(kStaticIP, kSubnetMask, kGateway);
  waitForLink();
  TEST_ASSERT(Ethernet.linkStatus() == LinkON);
}

// Tests the link listener.
static void test_link_listener() {
  TEST_ASSERT_FALSE(Ethernet.linkState());

  volatile bool latch = false;
  volatile bool linkState = false;
  Ethernet.onLinkState([&latch, &linkState](bool state) {
    latch = true;
    linkState = state;
  });

  Ethernet.begin(kStaticIP, kSubnetMask, kGateway);
  waitForLink();
  TEST_ASSERT_TRUE_MESSAGE(latch, "Expected callback to be called on up");
  TEST_ASSERT_TRUE_MESSAGE(linkState, "Expected link up in callback");
  TEST_ASSERT_TRUE_MESSAGE(Ethernet.linkState(), "Expected link up");

  latch = false;
  linkState = true;
  Ethernet.end();
  TEST_MESSAGE("Waiting for link down...");
  elapsedMillis timer;
  while (Ethernet.linkState() && timer < kLinkTimeout) {
    yield();
  }
  TEST_MESSAGE(format("Link down time: %" PRIu32 "ms", static_cast<uint32_t>(timer)).data());
  TEST_ASSERT_TRUE_MESSAGE(latch, "Expected callback to be called on down");
  TEST_ASSERT_FALSE_MESSAGE(linkState, "Expected link down in callback");
  TEST_ASSERT_FALSE_MESSAGE(Ethernet.linkState(), "Expected link down");
}

// Tests the address-changed listener.
static void test_address_listener() {
  TEST_ASSERT_TRUE(Ethernet.localIP() == INADDR_NONE);

  volatile bool latch = false;
  volatile bool hasIP = false;
  Ethernet.onAddressChanged([&latch, &hasIP]() {
    latch = true;
    hasIP = (Ethernet.localIP() != INADDR_NONE);
  });

  Ethernet.begin(kStaticIP, kSubnetMask, kGateway);
  TEST_ASSERT_TRUE_MESSAGE(latch, "Expected callback to be called on up");
  TEST_ASSERT_TRUE_MESSAGE(hasIP, "Expected valid IP in callback");
  TEST_ASSERT_MESSAGE(Ethernet.localIP() != INADDR_NONE, "Expected valid IP");

  latch = false;
  hasIP = true;
  Ethernet.end();
  // TEST_MESSAGE("Waiting for no-address...");
  // elapsedMillis timer;
  // while ((Ethernet.localIP() != INADDR_NONE) && timer < kLinkTimeout) {
  //   yield();
  // }
  // TEST_MESSAGE(format("No-address time: %" PRIu32 "ms", static_cast<uint32_t>(timer)).data());
  TEST_ASSERT_TRUE_MESSAGE(latch, "Expected callback to be called on down");
  TEST_ASSERT_FALSE_MESSAGE(hasIP, "Expected no IP in callback");
  TEST_ASSERT_MESSAGE(Ethernet.localIP() == INADDR_NONE, "Expected invalid IP");
}

// Tests the interface status listener.
static void test_interface_listener() {
  TEST_ASSERT_FALSE(Ethernet.interfaceStatus());

  volatile bool latch = false;
  volatile bool interfaceState = false;
  Ethernet.onInterfaceStatus([&latch, &interfaceState](bool state) {
    latch = true;
    interfaceState = state;
  });

  Ethernet.begin(kStaticIP, kSubnetMask, kGateway);
  TEST_ASSERT_TRUE_MESSAGE(latch, "Expected callback to be called on up");
  TEST_ASSERT_TRUE_MESSAGE(interfaceState, "Expected interface up in callback");
  TEST_ASSERT_TRUE_MESSAGE(Ethernet.interfaceStatus(), "Expected interface up");

  latch = false;
  interfaceState = true;
  Ethernet.end();
  // TEST_MESSAGE("Waiting for interface-down...");
  // elapsedMillis timer;
  // while (Ethernet.interfaceStatus() && timer < kLinkTimeout) {
  //   yield();
  // }
  // TEST_MESSAGE(format("Interface-down time: %" PRIu32 "ms", static_cast<uint32_t>(timer)).data());
  TEST_ASSERT_TRUE_MESSAGE(latch, "Expected callback to be called on down");
  TEST_ASSERT_FALSE_MESSAGE(interfaceState, "Expected interface down in callback");
  TEST_ASSERT_FALSE_MESSAGE(Ethernet.interfaceStatus(), "Expected interface down");
}

// Tests UDP by using SNTP.
static void test_udp() {
  // 01-Jan-1900 00:00:00 -> 01-Jan-1970 00:00:00
  constexpr uint32_t kEpochDiff = 2'208'988'800;

  // Epoch -> 07-Feb-2036 06:28:16
  constexpr uint32_t kBreakTime = 2'085'978'496;

  constexpr uint16_t kNTPPort = 123;

  waitForLocalIP();

  uint8_t buf[48];
  std::fill_n(buf, 48, 0);
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
  TEST_MESSAGE("Sending SNTP request...");
  EthernetUDP udp;
  udp.begin(kNTPPort);
  TEST_ASSERT_TRUE_MESSAGE(udp.send(Ethernet.gatewayIP(), kNTPPort, buf, 48), "Expected UDP send success");

  bool validReply = false;
  uint32_t sntpTime = 0;
  elapsedMillis timer;
  while (timer < kSNTPTimeout) {
    yield();

    int size = udp.parsePacket();
    if (size != 48 && size != 68) {
      continue;
    }

    const uint8_t *data = udp.data();

    // See: Section 5, "SNTP Client Operations"
    int mode = data[0] & 0x07;
    if (((data[0] & 0xc0) == 0xc0) ||  // LI == 3 (Alarm condition)
        (data[1] == 0) ||              // Stratum == 0 (Kiss-o'-Death)
        !(mode == 4 || mode == 5)) {   // Must be Server or Broadcast mode
      TEST_MESSAGE("Discarding SNTP reply");
      continue;
    }

    sntpTime = (uint32_t{data[40]} << 24) |
               (uint32_t{data[41]} << 16) |
               (uint32_t{data[42]} << 8) |
               uint32_t{data[43]};
    if (sntpTime == 0) {
      TEST_MESSAGE("Discarding SNTP reply: timestamp is zero");
      continue;
    }

    validReply = true;
    break;
  }

  TEST_ASSERT_TRUE_MESSAGE(validReply, "Expected valid reply");

  TEST_MESSAGE(format("SNTP reply time: %" PRIu32 "ms", static_cast<uint32_t>(timer)).data());

  if ((sntpTime & 0x80000000U) == 0) {
    // See: Section 3, "NTP Timestamp Format"
    sntpTime += kBreakTime;
  } else {
    sntpTime -= kEpochDiff;
  }

  // Print the time
  tmElements_t tm;
  breakTime(t, tm);
  TEST_MESSAGE(format("SNTP reply: %04u-%02u-%02u %02u:%02u:%02u",
                      tm.Year + 1970, tm.Month, tm.Day,
                      tm.Hour, tm.Minute, tm.Second).data());
}

static void test_client() {
  constexpr char kHost[]{"www.google.com"};
  constexpr char kRequest[]{
      "HEAD / HTTP/1.1\r\n"
      "Host: www.google.com\r\n"
      "Connection: close\r\n"
      "\r\n"
  };
  constexpr uint16_t kPort = 80;

  waitForLocalIP();

  EthernetClient client;
  client.setConnectionTimeout(kConnectTimeout);

  // Connect and send the request
  TEST_MESSAGE("Connecting and sending request...");
  uint32_t t = millis();
  TEST_ASSERT_EQUAL_MESSAGE(1, client.connect(kHost, kPort), "Expected connect success");
  TEST_MESSAGE(format("Lookup and connect time: %" PRIu32 "ms", millis() - t).data());
  client.writeFully(kRequest);
  client.flush();

  // Read the response
  TEST_MESSAGE("The response:");
  while (client.connected()) {
    int avail = client.available();
    if (avail <= 0) {
      continue;
    }
    for (int i = 0; i < avail; i++) {
      UNITY_OUTPUT_CHAR(client.read());
    }
    UNITY_OUTPUT_FLUSH();
  }
  UNITY_PRINT_EOL();

  TEST_ASSERT_FALSE_MESSAGE(client.connected(), "Expected not connected");
}

// Main program setup.
void setup() {
  Serial.begin(115200);
  while (!Serial && millis() < 4000) {
    // Wait for Serial
  }

  // NOTE!!! Wait for >2 secs
  // if board doesn't support software reset via Serial.DTR/RTS
  delay(2000);

  if (CrashReport) {
    Serial.println(CrashReport);
  }

  UNITY_BEGIN();
  RUN_TEST(test_builtin_mac);
  RUN_TEST(test_set_mac);
  RUN_TEST(test_dhcp);
  RUN_TEST(test_static_ip);
  RUN_TEST(test_mdns);
  RUN_TEST(test_dns_lookup);
  RUN_TEST(test_hostname);
  RUN_TEST(test_hardware);
  RUN_TEST(test_link);
  RUN_TEST(test_link_listener);
  RUN_TEST(test_address_listener);
  RUN_TEST(test_interface_listener);
  RUN_TEST(test_udp);
  RUN_TEST(test_client);
  UNITY_END();
}

// Main program loop.
void loop() {
}
