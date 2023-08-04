// SPDX-FileCopyrightText: (c) 2023 Shawn Silverman <shawn@pobox.com>
// SPDX-License-Identifier: MIT

// test_main.cpp tests the library.
// This file is part of the QNEthernet library.

#include <algorithm>
#include <memory>
#include <vector>

#include <Arduino.h>
#include <QNEthernet.h>
#include <QNDNSClient.h>
#include <TimeLib.h>
#include <lwip_driver.h>
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
static constexpr uint32_t kDHCPTimeout = 30000;
static constexpr uint32_t kLinkTimeout = 5000;
static constexpr uint32_t kSNTPTimeout = 10000;
static constexpr uint32_t kConnectTimeout = 10000;

static constexpr uint32_t kDNSLookupTimeout =
    DNS_MAX_RETRIES * DNS_TMR_INTERVAL;

// Default static IP configuration
static const IPAddress kStaticIP  {192, 168, 0, 2};
static const IPAddress kSubnetMask{255, 255, 255, 0};
static const IPAddress kGateway   {192, 168, 0, 1};

// Test hostname for mDNS and DHCP option 12.
static constexpr char kTestHostname[]{"test-hostname"};

// Registry of objects that need destruction, per test.
std::unique_ptr<EthernetUDP> udp;
std::unique_ptr<EthernetClient> client;
std::unique_ptr<EthernetServer> server;

// Pre-test setup. This is run before every test.
void setUp() {
}

// Post-test teardown. This is run after every test.
void tearDown() {
  // Clean up any stray objects because of the longjmp
  udp = nullptr;
  client = nullptr;
  server = nullptr;

  // Stop Ethernet and other services
  Ethernet.end();
  TEST_ASSERT_FALSE_MESSAGE(static_cast<bool>(Ethernet), "Expected stopped");

  // Restore the MAC address
  uint8_t mac[6];
  uint8_t mac2[6];
  enet_get_mac(mac);
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

  // Restore DHCP
  Ethernet.setDHCPEnabled(true);
}

// Tests using the built-in MAC address.
static void test_builtin_mac() {
  static constexpr uint8_t zeros[6]{0, 0, 0, 0, 0, 0};

  enet_get_mac(nullptr);  // Test NULL input

  // Get the built-in MAC address
  uint8_t mac[6]{0, 0, 0, 0, 0, 0};
  enet_get_mac(mac);
  TEST_ASSERT_FALSE_MESSAGE(std::equal(&mac[0], &mac[6], zeros),
                            "Expected an internal MAC");

  // Test NULL inputs
  Ethernet.macAddress(nullptr);
  Ethernet.MACAddress(nullptr);

  uint8_t mac2[6]{1};
  Ethernet.macAddress(mac2);
  TEST_ASSERT_EQUAL_UINT8_ARRAY_MESSAGE(mac, mac2, 6, "Expected matching MAC");
  std::fill_n(mac2, 6, 0);

  Ethernet.MACAddress(mac2);
  TEST_ASSERT_EQUAL_UINT8_ARRAY_MESSAGE(mac, mac2, 6, "Expected matching MAC (old API)");
}

// Tests setting the MAC address.
static void test_set_mac() {
  uint8_t builtInMAC[6];
  Ethernet.macAddress(builtInMAC);

  const uint8_t testMAC[6]{0x01, 0x02, 0x03, 0x04, 0x05, 0x06};
  TEST_ASSERT_FALSE_MESSAGE(std::equal(&builtInMAC[0], &builtInMAC[6], testMAC),
                            "Expected internal MAC");
  Ethernet.setMACAddress(testMAC);

  uint8_t mac[6]{0, 0, 0, 0, 0, 0};
  Ethernet.macAddress(mac);
  TEST_ASSERT_EQUAL_UINT8_ARRAY_MESSAGE(testMAC, mac, 6, "Expected matching MAC");

  std::fill_n(mac, 6, 0);
  Ethernet.MACAddress(mac);
  TEST_ASSERT_EQUAL_UINT8_ARRAY_MESSAGE(testMAC, mac, 6, "Expected matching MAC (old API)");

  // NULL uses the built-in
  Ethernet.setMACAddress(nullptr);
  Ethernet.macAddress(mac);
  TEST_ASSERT_EQUAL_UINT8_ARRAY_MESSAGE(builtInMAC, mac, 6, "Expected matching MAC (old API)");
}

// Obtains an IP address via DHCP.
static void waitForLocalIP() {
  TEST_ASSERT_FALSE_MESSAGE(static_cast<bool>(Ethernet), "Expected not started");
  TEST_ASSERT_FALSE_MESSAGE(Ethernet.isDHCPActive(), "Expected DHCP inactive");
  TEST_ASSERT_TRUE_MESSAGE(Ethernet.begin(), "Expected start with DHCP okay");
  TEST_ASSERT_TRUE_MESSAGE(static_cast<bool>(Ethernet), "Expected started");
  TEST_ASSERT_TRUE_MESSAGE(Ethernet.isDHCPActive(), "Expected DHCP active");

  TEST_MESSAGE("Waiting for DHCP...");
  uint32_t t = millis();
  TEST_ASSERT_TRUE_MESSAGE(Ethernet.waitForLocalIP(kDHCPTimeout), "Wait for IP failed");
  TEST_MESSAGE(format("DHCP time: %" PRIu32 "ms", millis() - t).data());

  IPAddress ip = Ethernet.localIP();
  IPAddress netmask = Ethernet.subnetMask();
  IPAddress gateway = Ethernet.gatewayIP();
  IPAddress dns = Ethernet.dnsServerIP();
  TEST_ASSERT_MESSAGE(ip != INADDR_NONE, "Expected valid IP");
  TEST_MESSAGE(format("DHCP IP:      %u.%u.%u.%u", ip[0], ip[1], ip[2], ip[3]).data());
  TEST_MESSAGE(format("     Netmask: %u.%u.%u.%u", netmask[0], netmask[1], netmask[2], netmask[3]).data());
  TEST_MESSAGE(format("     Gateway: %u.%u.%u.%u", gateway[0], gateway[1], gateway[2], gateway[3]).data());
  TEST_MESSAGE(format("     DNS:     %u.%u.%u.%u", dns[0], dns[1], dns[2], dns[3]).data());
  for (int i = 0; i < DNSClient::maxServers(); i++) {
    dns = DNSClient::getServer(i);
    TEST_MESSAGE(format("DNS Server %d: %u.%u.%u.%u", i, dns[0], dns[1], dns[2], dns[3]).data());
  }
}

// Tests NULL MAC address passed to the begin(...) functions.
static void test_other_null_mac() {
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wdeprecated-declarations"
  TEST_ASSERT_EQUAL_MESSAGE(1, Ethernet.begin(nullptr), "Expected start success (1)");
  Ethernet.begin(nullptr, INADDR_NONE);
  Ethernet.begin(nullptr, INADDR_NONE, INADDR_NONE);
  Ethernet.begin(nullptr, INADDR_NONE, INADDR_NONE, INADDR_NONE);
  Ethernet.begin(nullptr, INADDR_NONE, INADDR_NONE, INADDR_NONE, INADDR_NONE);
#pragma GCC diagnostic pop

  TEST_ASSERT_FALSE(enet_set_mac_address_allowed(nullptr, true));
  TEST_ASSERT_FALSE(enet_set_mac_address_allowed(nullptr, false));
}

// Tests NULL join/leave groups.
static void test_null_group() {
  TEST_ASSERT_FALSE_MESSAGE(enet_join_group(nullptr), "Expected join failed");
  TEST_ASSERT_FALSE_MESSAGE(enet_leave_group(nullptr), "Expected leave failed");
}

// Tests NULL output frames.
static void test_null_frame() {
  // Initialize Ethernet so these functions don't exit for the wrong reason
  TEST_ASSERT_TRUE_MESSAGE(Ethernet.isDHCPEnabled(), "Expected DHCP enabled");
  Ethernet.setDHCPEnabled(false);
  TEST_ASSERT_FALSE_MESSAGE(Ethernet.isDHCPEnabled(), "Expected DHCP disabled");
  TEST_ASSERT_TRUE_MESSAGE(Ethernet.begin(), "Expected start success");

  TEST_ASSERT_FALSE_MESSAGE(enet_output_frame(nullptr, 0), "Expected output failed");
  TEST_ASSERT_FALSE_MESSAGE(enet_output_frame(nullptr, 10), "Expected output failed");
}

// Tests DHCP.
static void test_dhcp() {
  TEST_ASSERT_TRUE_MESSAGE(Ethernet.isDHCPEnabled(), "Expected DHCP enabled");
  Ethernet.setDHCPEnabled(false);
  TEST_ASSERT_FALSE_MESSAGE(Ethernet.isDHCPEnabled(), "Expected DHCP disabled");
  Ethernet.setDHCPEnabled(true);
  TEST_ASSERT_TRUE_MESSAGE(Ethernet.isDHCPEnabled(), "Expected DHCP enabled");

  TEST_ASSERT_MESSAGE(Ethernet.localIP() == INADDR_NONE, "Expected invalid IP");
  waitForLocalIP();
}

// Tests using a static IP.
void test_static_ip() {
  TEST_ASSERT_FALSE_MESSAGE(Ethernet.isDHCPActive(), "Expected inactive DHCP before start");
  TEST_ASSERT_MESSAGE(Ethernet.localIP() == INADDR_NONE, "Expected no local IP before start");

  // Without a DNS server
  TEST_ASSERT_TRUE_MESSAGE(Ethernet.begin(kStaticIP, kSubnetMask, kGateway),
                           "Expected start success (1)");
  TEST_ASSERT_FALSE_MESSAGE(Ethernet.isDHCPActive(), "Expected inactive DHCP (1)");
  TEST_ASSERT_MESSAGE(Ethernet.localIP() == kStaticIP, "Expected matching local IP (1)");
  TEST_ASSERT_MESSAGE(Ethernet.subnetMask() == kSubnetMask, "Expected matching subnet mask (1)");
  TEST_ASSERT_MESSAGE(Ethernet.gatewayIP() == kGateway, "Expected matching gateway (1)");
  TEST_ASSERT_MESSAGE(Ethernet.dnsServerIP() == INADDR_NONE, "Expected unset DNS (1)");

  Ethernet.setDnsServerIP(kGateway);
  TEST_ASSERT_MESSAGE(Ethernet.dnsServerIP() == kGateway, "Expected gateway as DNS after set (old API)");
  Ethernet.setDNSServerIP(INADDR_NONE);
  TEST_ASSERT_MESSAGE(Ethernet.dnsServerIP() == INADDR_NONE, "Expected unset DNS after set");

  const IPAddress ip{192, 168, 1, 3};
  Ethernet.setLocalIP(ip);
  TEST_ASSERT_MESSAGE(Ethernet.localIP() == ip, "Expected matching local IP after set new");
  Ethernet.setLocalIP(kStaticIP);
  TEST_ASSERT_MESSAGE(Ethernet.localIP() == kStaticIP, "Expected matching local IP after set static");

  // With a DNS server
  TEST_ASSERT_TRUE_MESSAGE(Ethernet.begin(kStaticIP, kSubnetMask, kGateway, kGateway),
                           "Expected start success (2)");
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
  TEST_ASSERT_TRUE_MESSAGE(DNSClient::getHostByName(kName, ip, kDNSLookupTimeout),
                           "Expected lookup success");
  TEST_MESSAGE(format("Lookup time: %" PRIu32 "ms", millis() - t).data());
  TEST_MESSAGE(format("IP: %u.%u.%u.%u", ip[0], ip[1], ip[2], ip[3]).data());
  TEST_ASSERT_MESSAGE((ip == ip1) || (ip == ip2), "Expected different IP address");

  constexpr char kName2[]{"dms.goomgle"};
  TEST_MESSAGE(format("Waiting for DNS lookup [%s]...", kName2).data());
  t = millis();
  TEST_ASSERT_FALSE(DNSClient::getHostByName(kName2, ip, kDNSLookupTimeout));
  t = millis() - t;
  TEST_MESSAGE(format("Lookup time: %" PRIu32 "ms", t).data());
  TEST_ASSERT_LESS_THAN_MESSAGE(kDNSLookupTimeout, t, "Expected no timeout");
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
    TEST_ASSERT_EQUAL_MESSAGE(EthernetNoHardware, Ethernet.hardwareStatus(),
                              "Expected no hardware");
  } else {
    TEST_ASSERT_EQUAL_MESSAGE(EthernetOtherHardware, Ethernet.hardwareStatus(),
                              "Expected other hardware");
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
  TEST_ASSERT_EQUAL_MESSAGE(LinkOFF, Ethernet.linkStatus(), "Expected no link");
  TEST_ASSERT_FALSE_MESSAGE(Ethernet.linkState(), "Expected no link");
  TEST_ASSERT_TRUE_MESSAGE(Ethernet.begin(kStaticIP, kSubnetMask, kGateway),
                           "Expected start success");
  TEST_ASSERT_EQUAL_MESSAGE(LinkOFF, Ethernet.linkStatus(), "Expected no link");
  TEST_ASSERT_FALSE_MESSAGE(Ethernet.linkState(), "Expected no link");

  waitForLink();

  TEST_ASSERT_EQUAL_MESSAGE(LinkON, Ethernet.linkStatus(), "Expected link");
  TEST_ASSERT_TRUE_MESSAGE(Ethernet.linkState(), "Expected link");

  Ethernet.end();

  TEST_ASSERT_EQUAL_MESSAGE(LinkOFF, Ethernet.linkStatus(), "Expected no link");
  TEST_ASSERT_FALSE_MESSAGE(Ethernet.linkState(), "Expected no link");
}

// Tests the link listener.
static void test_link_listener() {
  TEST_ASSERT_FALSE_MESSAGE(Ethernet.linkState(), "Expected no link");

  volatile bool latch = false;
  volatile bool linkState = false;
  Ethernet.onLinkState([&latch, &linkState](bool state) {
    latch = true;
    linkState = state;
  });

  TEST_ASSERT_FALSE_MESSAGE(static_cast<bool>(Ethernet), "Expected not started");
  TEST_ASSERT_TRUE_MESSAGE(Ethernet.begin(kStaticIP, kSubnetMask, kGateway),
                           "Expected start success");
  TEST_ASSERT_TRUE_MESSAGE(static_cast<bool>(Ethernet), "Expected started");
  waitForLink();
  TEST_ASSERT_TRUE_MESSAGE(latch, "Expected callback to be called on up");
  TEST_ASSERT_TRUE_MESSAGE(linkState, "Expected link up in callback");
  TEST_ASSERT_TRUE_MESSAGE(Ethernet.linkState(), "Expected link up");

  latch = false;
  linkState = true;
  Ethernet.end();
  TEST_ASSERT_FALSE_MESSAGE(static_cast<bool>(Ethernet), "Expected stopped");
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

static void test_setLinkState() {
  TEST_ASSERT_FALSE_MESSAGE(Ethernet.linkState(), "Expected no link");

  volatile bool linkState = false;
  volatile int count = 0;
  Ethernet.onLinkState([&linkState, &count](bool state) {
    linkState = state;
    count++;
  });

  TEST_ASSERT_TRUE_MESSAGE(Ethernet.begin(kStaticIP, kSubnetMask, kGateway),
                           "Expected start success");
  TEST_ASSERT_FALSE_MESSAGE(Ethernet.linkState(), "Expected no link");
  Ethernet.setLinkState(true);
  TEST_ASSERT_TRUE_MESSAGE(Ethernet.linkState(), "Expected link");
  TEST_ASSERT_TRUE_MESSAGE(linkState, "Expected link up in callback");
  TEST_ASSERT_EQUAL_MESSAGE(1, count, "Expected callback called once");
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

  TEST_ASSERT_FALSE_MESSAGE(static_cast<bool>(Ethernet), "Expected not started");
  TEST_ASSERT_TRUE_MESSAGE(Ethernet.begin(kStaticIP, kSubnetMask, kGateway),
                           "Expected start success");
  TEST_ASSERT_TRUE_MESSAGE(static_cast<bool>(Ethernet), "Expected started");
  TEST_ASSERT_TRUE_MESSAGE(latch, "Expected callback to be called on up");
  TEST_ASSERT_TRUE_MESSAGE(hasIP, "Expected valid IP in callback");
  TEST_ASSERT_MESSAGE(Ethernet.localIP() != INADDR_NONE, "Expected valid IP");

  latch = false;
  hasIP = true;
  Ethernet.end();
  TEST_ASSERT_FALSE_MESSAGE(static_cast<bool>(Ethernet), "Expected stopped");
  // TEST_MESSAGE("Waiting for no-address...");
  // elapsedMillis timer;
  // while ((Ethernet.localIP() != INADDR_NONE) && timer < kLinkTimeout) {
  //   yield();
  // }
  // TEST_MESSAGE(format("No-address time: %" PRIu32 "ms",
  //                     static_cast<uint32_t>(timer)).data());
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

  TEST_ASSERT_FALSE_MESSAGE(static_cast<bool>(Ethernet), "Expected not started");
  TEST_ASSERT_TRUE_MESSAGE(Ethernet.begin(kStaticIP, kSubnetMask, kGateway),
                           "Expected start success");
  TEST_ASSERT_TRUE_MESSAGE(static_cast<bool>(Ethernet), "Expected started");
  TEST_ASSERT_TRUE_MESSAGE(latch, "Expected callback to be called on up");
  TEST_ASSERT_TRUE_MESSAGE(interfaceState, "Expected interface up in callback");
  TEST_ASSERT_TRUE_MESSAGE(Ethernet.interfaceStatus(), "Expected interface up");

  latch = false;
  interfaceState = true;
  Ethernet.end();
  TEST_ASSERT_FALSE_MESSAGE(static_cast<bool>(Ethernet), "Expected stopped");
  // TEST_MESSAGE("Waiting for interface-down...");
  // elapsedMillis timer;
  // while (Ethernet.interfaceStatus() && timer < kLinkTimeout) {
  //   yield();
  // }
  // TEST_MESSAGE(format("Interface-down time: %" PRIu32 "ms",
  //                     static_cast<uint32_t>(timer)).data());
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
  TEST_MESSAGE("Sending SNTP request (after delay)...");
  udp = std::make_unique<EthernetUDP>();
  TEST_ASSERT_TRUE_MESSAGE(udp->begin(kNTPPort), "Expected UDP listen success");
  delay(3000);  // Waiting a few seconds seems to be necessary
  TEST_ASSERT_TRUE_MESSAGE(udp->send(Ethernet.gatewayIP(), kNTPPort, buf, 48),
                           "Expected UDP send success");

  bool validReply = false;
  uint32_t sntpTime = 0;
  elapsedMillis timer;
  while (timer < kSNTPTimeout) {
    yield();

    int size = udp->parsePacket();
    if (size < 0) {
      continue;
    }
    if (size != 48 && size != 68) {
      TEST_MESSAGE("Discarding incorrect-sized reply");
      continue;
    }

    const uint8_t *data = udp->data();

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

  TEST_MESSAGE(format("SNTP reply time: %" PRIu32 "ms",
                      static_cast<uint32_t>(timer)).data());

  if ((sntpTime & 0x80000000U) == 0) {
    // See: Section 3, "NTP Timestamp Format"
    sntpTime += kBreakTime;
  } else {
    sntpTime -= kEpochDiff;
  }

  // Print the time
  tmElements_t tm;
  breakTime(sntpTime, tm);
  TEST_MESSAGE(format("SNTP reply: %04u-%02u-%02u %02u:%02u:%02u",
                      tm.Year + 1970, tm.Month, tm.Day,
                      tm.Hour, tm.Minute, tm.Second).data());
}

static void test_udp_receive_queueing() {
  constexpr uint16_t kPort = 1025;

  TEST_ASSERT_TRUE_MESSAGE(Ethernet.begin(kStaticIP, kSubnetMask, kGateway),
                           "Expected successful Ethernet start");
  waitForLink();  // send() won't work unless there's a link

  // Create and listen
  udp = std::make_unique<EthernetUDP>();  // Receive queue of 1
  TEST_ASSERT_EQUAL_MESSAGE(1, udp->receiveQueueSize(), "Expected default queue size");
  TEST_ASSERT_TRUE_MESSAGE(udp->begin(kPort), "Expected UDP listen success");

  uint8_t b = 0;  // The buffer

  // Send two packets
  b = 1;
  TEST_ASSERT_TRUE_MESSAGE(udp->send(Ethernet.localIP(), kPort, &b, 1),
                           "Expected packet 1 send success");
  b = 2;
  TEST_ASSERT_TRUE_MESSAGE(udp->send(Ethernet.localIP(), kPort, &b, 1),
                           "Expected packet 2 send success");

  // Expect to receive only the last packet
  TEST_ASSERT_EQUAL_MESSAGE(1, udp->parsePacket(), "Expected packet with size 1");
  TEST_ASSERT_EQUAL_MESSAGE(2, udp->data()[0], "Expected packet 2 data");
  TEST_ASSERT_LESS_THAN_MESSAGE(0, udp->parsePacket(), "Expected no second packet");

  // Increase the buffer to two
  udp->setReceiveQueueSize(2);
  TEST_ASSERT_EQUAL_MESSAGE(2, udp->receiveQueueSize(), "Expected updated queue size");

  // Send the two packets again
  b = 3;
  TEST_ASSERT_TRUE_MESSAGE(udp->send(Ethernet.localIP(), kPort, &b, 1),
                           "Expected packet 3 send success");
  b = 4;
  TEST_ASSERT_TRUE_MESSAGE(udp->send(Ethernet.localIP(), kPort, &b, 1),
                           "Expected packet 4 send success");

  // Expect to receive both packets
  TEST_ASSERT_EQUAL_MESSAGE(1, udp->parsePacket(), "Expected packet 3 with size 1");
  TEST_ASSERT_EQUAL_MESSAGE(3, udp->data()[0], "Expected packet 3 data");
  TEST_ASSERT_EQUAL_MESSAGE(1, udp->parsePacket(), "Expected packet 4 with size 1");
  TEST_ASSERT_EQUAL_MESSAGE(4, udp->data()[0], "Expected packet 4 data");
}

static void test_udp_receive_timestamp() {
  constexpr uint16_t kPort = 1025;

  TEST_ASSERT_TRUE_MESSAGE(Ethernet.begin(kStaticIP, kSubnetMask, kGateway),
                           "Expected successful Ethernet start");
  waitForLink();  // send() won't work unless there's a link

  // Create and listen
  udp = std::make_unique<EthernetUDP>();
  TEST_ASSERT_EQUAL_MESSAGE(1, udp->beginWithReuse(kPort), "Expected UDP listen success");

  uint8_t b = 13;  // The buffer

  uint32_t t = millis();  // Current timestamp

  // Send a packet
  TEST_ASSERT_TRUE_MESSAGE(udp->send(Ethernet.localIP(), kPort, &b, 1),
                           "Expected packet send success");

  // Test that we actually received the packet
  TEST_ASSERT_EQUAL_MESSAGE(1, udp->parsePacket(), "Expected packet with size 1");
  TEST_ASSERT_EQUAL_MESSAGE(b, udp->data()[0], "Expected packet data");

  TEST_ASSERT_GREATER_OR_EQUAL_MESSAGE(t, udp->receivedTimestamp(), "Expected valid timestamp");
}

// Tests a variety of UDP object states.
static void test_udp_state() {
  constexpr uint16_t kPort = 1025;

  TEST_ASSERT_TRUE_MESSAGE(Ethernet.begin(kStaticIP, kSubnetMask, kGateway),
                           "Expected successful Ethernet start");

  udp = std::make_unique<EthernetUDP>();

  TEST_ASSERT_FALSE_MESSAGE(static_cast<bool>(*udp), "Expected not listening");
  TEST_ASSERT_EQUAL_MESSAGE(0, udp->localPort(), "Expected invalid local port");
  TEST_ASSERT_EQUAL_MESSAGE(1, udp->begin(kPort), "Expected UDP listen success");
  TEST_ASSERT_TRUE_MESSAGE(static_cast<bool>(*udp), "Expected listening");
  TEST_ASSERT_EQUAL_MESSAGE(kPort, udp->localPort(), "Expected valid local port");
  udp->stop();
  TEST_ASSERT_FALSE_MESSAGE(static_cast<bool>(*udp), "Expected not listening");
  TEST_ASSERT_EQUAL_MESSAGE(0, udp->localPort(), "Expected invalid local port");

  TEST_ASSERT_EQUAL_MESSAGE(MEMP_NUM_UDP_PCB, EthernetUDP::maxSockets(),
                            "Expected default UDP max. sockets");
}

static void test_client() {
#define HOST "www.example.com"
  constexpr char kHost[]{HOST};
  constexpr char kRequest[]{
      "HEAD / HTTP/1.1\r\n"
      "Host: " HOST "\r\n"
      "Connection: close\r\n"
      "\r\n"
  };
  constexpr uint16_t kPort = 80;

  waitForLocalIP();

  client = std::make_unique<EthernetClient>();
  TEST_ASSERT_EQUAL_MESSAGE(1000, client->connectionTimeout(), "Expected default connection timeout");
  client->setConnectionTimeout(kConnectTimeout);
  TEST_ASSERT_EQUAL_MESSAGE(kConnectTimeout, client->connectionTimeout(), "Expected set timeout");

  TEST_ASSERT_FALSE_MESSAGE(static_cast<bool>(*client), "Expected not connected");
  TEST_ASSERT_EQUAL_MESSAGE(0, client->connected(), "Expected not connected (no data)");

  // Connect and send the request
  TEST_MESSAGE("Connecting and sending HTTP HEAD request...");
  uint32_t t = millis();
  TEST_ASSERT_EQUAL_MESSAGE(1, client->connect(kHost, kPort), "Expected connect success");
  TEST_ASSERT_TRUE_MESSAGE(static_cast<bool>(*client), "Expected connected");
  TEST_MESSAGE(format("Lookup and connect time: %" PRIu32 "ms", millis() - t).data());
  client->writeFully(kRequest);
  client->flush();

  // Read the response
  t = millis();
  TEST_MESSAGE("The response:");
  while (client->connected()) {
    int avail = client->available();
    if (avail <= 0) {
      continue;
    }
    for (int i = 0; i < avail; i++) {
      UNITY_OUTPUT_CHAR(client->read());
    }
    UNITY_OUTPUT_FLUSH();
  }
  UNITY_PRINT_EOL();
  TEST_MESSAGE(format("Read and print response time: %" PRIu32 "ms", millis() - t).data());

  TEST_ASSERT_EQUAL_MESSAGE(0, client->connected(), "Expected not connected (no more data)");
  TEST_ASSERT_FALSE_MESSAGE(static_cast<bool>(*client), "Expected not connected");
}

static void test_client_connectNoWait() {
  constexpr uint16_t kPort = 80;

  TEST_ASSERT_TRUE_MESSAGE(Ethernet.begin(kStaticIP, kSubnetMask, kGateway),
                           "Expected start success");
  waitForLink();

  client = std::make_unique<EthernetClient>();

  TEST_ASSERT_FALSE_MESSAGE(static_cast<bool>(*client), "Expected not connected");
  TEST_ASSERT_EQUAL_MESSAGE(0, client->connected(), "Expected not connected (no data)");

  // Connect
  TEST_MESSAGE("Connecting ...");
  TEST_ASSERT_EQUAL_MESSAGE(1, client->connectNoWait(Ethernet.localIP(), kPort),
                            "Expected connect success");
  TEST_ASSERT_FALSE_MESSAGE(static_cast<bool>(*client), "Expected not connected");
  TEST_ASSERT_EQUAL_MESSAGE(0, client->connected(), "Expected not connected (no data)");
}

static void test_client_timeout() {
  constexpr uint16_t kPort = 1025;

  TEST_ASSERT_TRUE_MESSAGE(Ethernet.begin(kStaticIP, kSubnetMask, kGateway),
                           "Expected start success");
  waitForLink();

  client = std::make_unique<EthernetClient>();
  TEST_ASSERT_EQUAL_MESSAGE(1000, client->connectionTimeout(), "Expected default connection timeout");
  TEST_ASSERT_FALSE_MESSAGE(static_cast<bool>(*client), "Expected not connected");
  TEST_ASSERT_EQUAL_MESSAGE(0, client->connected(), "Expected not connected (no data)");

  uint32_t t = millis();
  TEST_ASSERT_EQUAL_MESSAGE(-1, client->connect(Ethernet.localIP(), kPort), "Expected timeout");
  TEST_ASSERT_GREATER_OR_EQUAL_MESSAGE(1000, millis() - t, "Expected timeout duration");

  TEST_ASSERT_FALSE_MESSAGE(static_cast<bool>(*client), "Expected not connected");
  TEST_ASSERT_EQUAL_MESSAGE(0, client->connected(), "Expected not connected (no data)");
}

// Tests a variety of client object states.
static void test_client_state() {
  client = std::make_unique<EthernetClient>();

  TEST_ASSERT_FALSE_MESSAGE(static_cast<bool>(*client), "Expected not connected");
  TEST_ASSERT_EQUAL_MESSAGE(0, client->localPort(), "Expected invalid local port");
  TEST_ASSERT_EQUAL_MESSAGE(0, client->remotePort(), "Expected invalid remote port");
  TEST_ASSERT_MESSAGE(client->remoteIP() == INADDR_NONE, "Expected no remote IP");

  TEST_ASSERT_EQUAL_MESSAGE(1000, client->connectionTimeout(), "Expected default connection timeout");
  TEST_ASSERT_EQUAL_MESSAGE(MEMP_NUM_TCP_PCB, EthernetClient::maxSockets(),
                            "Expected default TCP max. sockets");
}

// Tests a variety of server object states.
static void test_server_state() {
  constexpr uint16_t kPort = 1025;

  TEST_ASSERT_TRUE_MESSAGE(Ethernet.begin(kStaticIP, kSubnetMask, kGateway),
                           "Expected successful Ethernet start");

  server = std::make_unique<EthernetServer>();

  TEST_ASSERT_FALSE_MESSAGE(static_cast<bool>(*server), "Expected not listening");
  TEST_ASSERT_EQUAL_MESSAGE(-1, server->port(), "Expected invalid port");
  TEST_ASSERT_TRUE_MESSAGE(server->begin(kPort), "Expected TCP listen success");
  TEST_ASSERT_TRUE_MESSAGE(static_cast<bool>(*server), "Expected listening");
  TEST_ASSERT_EQUAL_MESSAGE(kPort, server->port(), "Expected valid port");
  server->end();
  TEST_ASSERT_FALSE_MESSAGE(static_cast<bool>(*server), "Expected not listening");
  TEST_ASSERT_EQUAL_MESSAGE(-1, server->port(), "Expected invalid port");

  TEST_ASSERT_EQUAL_MESSAGE(MEMP_NUM_TCP_PCB_LISTEN, EthernetServer::maxListeners(),
                            "Expected default TCP max. listeners");
}

// Tests state from some of the other classes.
static void test_other_state() {
  TEST_ASSERT_EQUAL_MESSAGE(DNS_MAX_SERVERS, DNSClient::maxServers(), "Expected default DNS max. servers");
  TEST_ASSERT_EQUAL_MESSAGE(MEMP_NUM_IGMP_GROUP > 0 ? MEMP_NUM_IGMP_GROUP - 1 : 0,
                            Ethernet.maxMulticastGroups(),
                            "Expected default max. multicast groups");
  TEST_ASSERT_EQUAL_MESSAGE(enet_get_mtu(), Ethernet.mtu(), "Expected default MTU");
  TEST_ASSERT_EQUAL_MESSAGE(enet_get_max_frame_len(), EthernetFrame.maxFrameLen(),
                            "Expected default max. frame len");
  TEST_ASSERT_EQUAL_MESSAGE(64, EthernetFrame.minFrameLen(), "Expected default min. frame len");
  TEST_ASSERT_EQUAL_MESSAGE(MDNS_MAX_SERVICES, MDNS.maxServices(), "Expected default mDNS max. services");
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
  RUN_TEST(test_other_null_mac);
  RUN_TEST(test_null_group);
  RUN_TEST(test_null_frame);
  RUN_TEST(test_dhcp);
  RUN_TEST(test_static_ip);
  RUN_TEST(test_mdns);
  RUN_TEST(test_dns_lookup);
  RUN_TEST(test_hostname);
  RUN_TEST(test_hardware);
  RUN_TEST(test_link);
  RUN_TEST(test_link_listener);
  RUN_TEST(test_setLinkState);
  RUN_TEST(test_address_listener);
  RUN_TEST(test_interface_listener);
  RUN_TEST(test_udp);
  RUN_TEST(test_udp_receive_queueing);
  RUN_TEST(test_udp_receive_timestamp);
  RUN_TEST(test_udp_state);
  RUN_TEST(test_client);
  RUN_TEST(test_client_connectNoWait);
  RUN_TEST(test_client_timeout);
  RUN_TEST(test_client_state);
  RUN_TEST(test_server_state);
  RUN_TEST(test_other_state);
  UNITY_END();
}

// Main program loop.
void loop() {
}
