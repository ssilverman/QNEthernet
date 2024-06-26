// SPDX-FileCopyrightText: (c) 2023-2024 Shawn Silverman <shawn@pobox.com>
// SPDX-License-Identifier: AGPL-3.0-or-later

// test_main.cpp tests the library.
// This file is part of the QNEthernet library.

#include <algorithm>
#include <cstdio>
#include <cstring>
#include <ctime>
#include <memory>
#include <vector>

#include <Arduino.h>
#include <QNEthernet.h>
#include <QNDNSClient.h>
#include <lwip_driver.h>
#include <lwip/dns.h>
#include <lwip/opt.h>
#include <qnethernet_opts.h>
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
static constexpr uint32_t kSNTPResendTimeout = 2500;
static constexpr uint32_t kConnectTimeout = 10000;

// Default static IP configuration
static const IPAddress kStaticIP  {192, 168, 0, 2};
static const IPAddress kSubnetMask{255, 255, 255, 0};
static const IPAddress kGateway   {192, 168, 0, 1};
static const IPAddress kDNS       {192, 168, 0, 253};

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
  EthernetFrame.clear();

  // Remove any listeners before calling Ethernet.end()
  // This avoids accessing any out-of-scope variables
  Ethernet.onLinkState(nullptr);
  Ethernet.onAddressChanged(nullptr);
  Ethernet.onInterfaceStatus(nullptr);

  // Stop Ethernet and other services
  Ethernet.end();
  TEST_ASSERT_FALSE_MESSAGE(static_cast<bool>(Ethernet), "Expected stopped");

  // Restore the MAC address
  uint8_t mac[6];
  uint8_t mac2[6];
  enet_get_system_mac(mac);
  Ethernet.macAddress(mac2);
  if (!std::equal(&mac[0], &mac[6], mac2)) {
    Ethernet.setMACAddress(mac);
  }

  // Restore to no hostname
  Ethernet.setHostname(nullptr);

  // Restore DHCP
  Ethernet.setDHCPEnabled(true);
}

// Tests using the built-in MAC address.
static void test_builtin_mac() {
  static constexpr uint8_t zeros[6]{0, 0, 0, 0, 0, 0};

  enet_get_system_mac(nullptr);  // Test NULL input

  // Get the built-in MAC address
  uint8_t mac[6]{0, 0, 0, 0, 0, 0};
  enet_get_system_mac(mac);
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
  enet_get_system_mac(builtInMAC);

  volatile bool interfaceState = false;
  volatile int downCount = 0;
  volatile int upCount = 0;
  Ethernet.onInterfaceStatus(
      [&interfaceState, &downCount, &upCount](bool state) {
        interfaceState = state;
        if (state) {
          upCount++;
        } else {
          downCount++;
        }
      });

  TEST_ASSERT_FALSE_MESSAGE(interfaceState, "Expected interface down");

  const uint8_t testMAC[6]{0x02, 0x01, 0x03, 0x04, 0x05, 0x06};
  TEST_ASSERT_FALSE_MESSAGE(std::equal(&builtInMAC[0], &builtInMAC[6], testMAC),
                            "Expected internal MAC");
  Ethernet.setMACAddress(testMAC);
  TEST_ASSERT_FALSE_MESSAGE(interfaceState, "Expected interface down");

  uint8_t mac[6]{0, 0, 0, 0, 0, 0};
  Ethernet.macAddress(mac);
  TEST_ASSERT_EQUAL_UINT8_ARRAY_MESSAGE(testMAC, mac, 6, "Expected matching MAC");

  std::fill_n(mac, 6, 0);
  Ethernet.MACAddress(mac);
  TEST_ASSERT_EQUAL_UINT8_ARRAY_MESSAGE(testMAC, mac, 6, "Expected matching MAC (old API)");
  TEST_ASSERT_FALSE_MESSAGE(interfaceState, "Expected interface down");

  // NULL uses the built-in
  Ethernet.setMACAddress(nullptr);
  Ethernet.macAddress(mac);
  TEST_ASSERT_EQUAL_UINT8_ARRAY_MESSAGE(builtInMAC, mac, 6, "Expected matching MAC (old API)");
  TEST_ASSERT_FALSE_MESSAGE(interfaceState, "Expected interface down");

  // Test changing the MAC address while Ethernet is up
  TEST_ASSERT_MESSAGE(Ethernet.localIP() == INADDR_NONE, "Expected no IP");
  TEST_ASSERT_MESSAGE(Ethernet.subnetMask() == INADDR_NONE, "Expected no netmask");
  TEST_ASSERT_MESSAGE(Ethernet.gatewayIP() == INADDR_NONE, "Expected no gateway");
  Ethernet.begin(kStaticIP, kSubnetMask, kGateway);
  TEST_ASSERT_TRUE_MESSAGE(interfaceState, "Expected interface up");
  TEST_ASSERT_EQUAL_MESSAGE(1, upCount, "Expected matching up count");
  TEST_ASSERT_EQUAL_MESSAGE(0, downCount, "Expected matching down count");
  TEST_ASSERT_MESSAGE(Ethernet.localIP() == kStaticIP, "Expected matching IP");
  TEST_ASSERT_MESSAGE(Ethernet.subnetMask() == kSubnetMask, "Expected matching netmask");
  TEST_ASSERT_MESSAGE(Ethernet.gatewayIP() == kGateway, "Expected matching gateway");
  Ethernet.setMACAddress(testMAC);
  Ethernet.macAddress(mac);
  TEST_ASSERT_EQUAL_UINT8_ARRAY_MESSAGE(testMAC, mac, 6, "Expected matching MAC");
  TEST_ASSERT_TRUE_MESSAGE(interfaceState, "Expected interface up");
  TEST_ASSERT_EQUAL_MESSAGE(2, upCount, "Expected matching up count");
  TEST_ASSERT_EQUAL_MESSAGE(1, downCount, "Expected matching down count");
}

static void test_get_mac() {
  TEST_ASSERT_NOT_NULL_MESSAGE(Ethernet.macAddress(), "Expected not NULL");

  // Get the built-in MAC address
  uint8_t mac[6];
  enet_get_system_mac(mac);
  TEST_ASSERT_EQUAL_UINT8_ARRAY_MESSAGE(Ethernet.macAddress(), mac, 6, "Expected the internal MAC");

  uint8_t mac2[6]{1, 2, 3, 4, 5, 6};
  Ethernet.macAddress(mac2);
  TEST_ASSERT_NOT_NULL_MESSAGE(Ethernet.macAddress(), "Expected not NULL");
  TEST_ASSERT_EQUAL_UINT8_ARRAY_MESSAGE(Ethernet.macAddress(), mac2, 6, "Expected new MAC");
}

// Obtains an IP address via DHCP and returns whether successful.
static bool waitForLocalIP() {
  TEST_ASSERT_FALSE_MESSAGE(static_cast<bool>(Ethernet), "Expected not started");
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
  TEST_MESSAGE(format("DHCP IP:      %u.%u.%u.%u", ip[0], ip[1], ip[2], ip[3]).data());
  TEST_MESSAGE(format("     Netmask: %u.%u.%u.%u", netmask[0], netmask[1], netmask[2], netmask[3]).data());
  TEST_MESSAGE(format("     Gateway: %u.%u.%u.%u", gateway[0], gateway[1], gateway[2], gateway[3]).data());
  TEST_MESSAGE(format("     DNS:     %u.%u.%u.%u", dns[0], dns[1], dns[2], dns[3]).data());
  for (int i = 0; i < DNSClient::maxServers(); i++) {
    dns = DNSClient::getServer(i);
    TEST_MESSAGE(format("DNS Server %d: %u.%u.%u.%u", i, dns[0], dns[1], dns[2], dns[3]).data());
  }

  return true;
}

// Tests NULL MAC address passed to the begin(...) functions.
static void test_other_null_mac() {
  TEST_MESSAGE(format("Starting Ethernet with DHCP timeout of %d ms...",
                      QNETHERNET_DEFAULT_DHCP_CLIENT_TIMEOUT)
                   .data());
  TEST_ASSERT_EQUAL_MESSAGE(1, Ethernet.begin(nullptr), "Expected start success (1)");
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wdeprecated-declarations"
  TEST_ASSERT_TRUE_MESSAGE(Ethernet.begin(nullptr, INADDR_NONE),
                           "Expected start success (2)");
  TEST_ASSERT_TRUE_MESSAGE(Ethernet.begin(nullptr, INADDR_NONE, INADDR_NONE),
                           "Expected start success (3)");
  TEST_ASSERT_TRUE_MESSAGE(
      Ethernet.begin(nullptr, INADDR_NONE, INADDR_NONE, INADDR_NONE),
      "Expected start success (4)");
#pragma GCC diagnostic pop
  TEST_ASSERT_TRUE_MESSAGE(
      Ethernet.begin(nullptr, INADDR_NONE, INADDR_NONE, INADDR_NONE, INADDR_NONE),
      "Expected start success (5)");

  TEST_ASSERT_FALSE(driver_set_mac_address_allowed(nullptr, true));
  TEST_ASSERT_FALSE(driver_set_mac_address_allowed(nullptr, false));
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

// Tests the Arduino-style begin() functions.
static void test_arduino_begin() {
  const uint8_t testMAC[6]{0x02, 0x01, 0x03, 0x04, 0x05, 0x06};
  uint8_t systemMAC[6];
  Ethernet.macAddress(systemMAC);

  TEST_ASSERT_FALSE_MESSAGE(Ethernet.isDHCPActive(), "Expected inactive DHCP before start");
  TEST_ASSERT_MESSAGE(Ethernet.localIP() == INADDR_NONE, "Expected no local IP before start");

#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wdeprecated-declarations"

  TEST_ASSERT_TRUE_MESSAGE(Ethernet.begin(nullptr, kStaticIP), "Expected start success (1)");
  TEST_ASSERT_FALSE_MESSAGE(Ethernet.isDHCPActive(), "Expected inactive DHCP (1)");
  TEST_ASSERT_EQUAL_UINT8_ARRAY_MESSAGE(systemMAC, Ethernet.macAddress(), 6, "Expected matching MAC (1)");
  TEST_ASSERT_MESSAGE(Ethernet.localIP() == kStaticIP, "Expected matching local IP (1)");
  TEST_ASSERT_MESSAGE(Ethernet.subnetMask() == kSubnetMask, "Expected matching subnet mask (1)");
  TEST_ASSERT_MESSAGE(Ethernet.gatewayIP() == kGateway, "Expected matching gateway (1)");
  TEST_ASSERT_MESSAGE(Ethernet.dnsServerIP() == kGateway, "Expected matching DNS (1)");

  TEST_ASSERT_TRUE_MESSAGE(Ethernet.begin(testMAC, kStaticIP), "Expected start success (2)");
  TEST_ASSERT_FALSE_MESSAGE(Ethernet.isDHCPActive(), "Expected inactive DHCP (2)");
  TEST_ASSERT_EQUAL_UINT8_ARRAY_MESSAGE(testMAC, Ethernet.macAddress(), 6, "Expected matching MAC (2)");
  TEST_ASSERT_MESSAGE(Ethernet.localIP() == kStaticIP, "Expected matching local IP (2)");
  TEST_ASSERT_MESSAGE(Ethernet.subnetMask() == kSubnetMask, "Expected matching subnet mask (2)");
  TEST_ASSERT_MESSAGE(Ethernet.gatewayIP() == kGateway, "Expected matching gateway (2)");
  TEST_ASSERT_MESSAGE(Ethernet.dnsServerIP() == kGateway, "Expected matching DNS (2)");

  TEST_ASSERT_TRUE_MESSAGE(Ethernet.begin(nullptr, kStaticIP, kDNS), "Expected start success (3)");
  TEST_ASSERT_FALSE_MESSAGE(Ethernet.isDHCPActive(), "Expected inactive DHCP (3)");
  TEST_ASSERT_EQUAL_UINT8_ARRAY_MESSAGE(systemMAC, Ethernet.macAddress(), 6, "Expected matching MAC (3)");
  TEST_ASSERT_MESSAGE(Ethernet.localIP() == kStaticIP, "Expected matching local IP (3)");
  TEST_ASSERT_MESSAGE(Ethernet.subnetMask() == kSubnetMask, "Expected matching subnet mask (3)");
  TEST_ASSERT_MESSAGE(Ethernet.gatewayIP() == kGateway, "Expected matching gateway (3)");
  TEST_ASSERT_MESSAGE(Ethernet.dnsServerIP() == kDNS, "Expected matching DNS (3)");

  TEST_ASSERT_TRUE_MESSAGE(Ethernet.begin(testMAC, kStaticIP, kDNS), "Expected start success (4)");
  TEST_ASSERT_FALSE_MESSAGE(Ethernet.isDHCPActive(), "Expected inactive DHCP (4)");
  TEST_ASSERT_EQUAL_UINT8_ARRAY_MESSAGE(testMAC, Ethernet.macAddress(), 6, "Expected matching MAC (4)");
  TEST_ASSERT_MESSAGE(Ethernet.localIP() == kStaticIP, "Expected matching local IP (4)");
  TEST_ASSERT_MESSAGE(Ethernet.subnetMask() == kSubnetMask, "Expected matching subnet mask (4)");
  TEST_ASSERT_MESSAGE(Ethernet.gatewayIP() == kGateway, "Expected matching gateway (4)");
  TEST_ASSERT_MESSAGE(Ethernet.dnsServerIP() == kDNS, "Expected matching DNS (4)");

  TEST_ASSERT_TRUE_MESSAGE(Ethernet.begin(nullptr, kStaticIP, kDNS, kGateway), "Expected start success (5)");
  TEST_ASSERT_FALSE_MESSAGE(Ethernet.isDHCPActive(), "Expected inactive DHCP (5)");
  TEST_ASSERT_EQUAL_UINT8_ARRAY_MESSAGE(systemMAC, Ethernet.macAddress(), 6, "Expected matching MAC (5)");
  TEST_ASSERT_MESSAGE(Ethernet.localIP() == kStaticIP, "Expected matching local IP (5)");
  TEST_ASSERT_MESSAGE(Ethernet.subnetMask() == kSubnetMask, "Expected matching subnet mask (5)");
  TEST_ASSERT_MESSAGE(Ethernet.gatewayIP() == kGateway, "Expected matching gateway (5)");
  TEST_ASSERT_MESSAGE(Ethernet.dnsServerIP() == kDNS, "Expected matching DNS (5)");

  TEST_ASSERT_TRUE_MESSAGE(Ethernet.begin(testMAC, kStaticIP, kDNS, kGateway), "Expected start success (6)");
  TEST_ASSERT_FALSE_MESSAGE(Ethernet.isDHCPActive(), "Expected inactive DHCP (6)");
  TEST_ASSERT_EQUAL_UINT8_ARRAY_MESSAGE(testMAC, Ethernet.macAddress(), 6, "Expected matching MAC (6)");
  TEST_ASSERT_MESSAGE(Ethernet.localIP() == kStaticIP, "Expected matching local IP (6)");
  TEST_ASSERT_MESSAGE(Ethernet.subnetMask() == kSubnetMask, "Expected matching subnet mask (6)");
  TEST_ASSERT_MESSAGE(Ethernet.gatewayIP() == kGateway, "Expected matching gateway (6)");
  TEST_ASSERT_MESSAGE(Ethernet.dnsServerIP() == kDNS, "Expected matching DNS (6)");

  TEST_ASSERT_TRUE_MESSAGE(Ethernet.begin(nullptr, kStaticIP, kDNS, kGateway, kSubnetMask), "Expected start success (7)");
  TEST_ASSERT_FALSE_MESSAGE(Ethernet.isDHCPActive(), "Expected inactive DHCP (7)");
  TEST_ASSERT_EQUAL_UINT8_ARRAY_MESSAGE(systemMAC, Ethernet.macAddress(), 6, "Expected matching MAC (7)");
  TEST_ASSERT_MESSAGE(Ethernet.localIP() == kStaticIP, "Expected matching local IP (7)");
  TEST_ASSERT_MESSAGE(Ethernet.subnetMask() == kSubnetMask, "Expected matching subnet mask (7)");
  TEST_ASSERT_MESSAGE(Ethernet.gatewayIP() == kGateway, "Expected matching gateway (7)");
  TEST_ASSERT_MESSAGE(Ethernet.dnsServerIP() == kDNS, "Expected matching DNS (7)");

  TEST_ASSERT_TRUE_MESSAGE(Ethernet.begin(testMAC, kStaticIP, kDNS, kGateway, kSubnetMask), "Expected start success (8)");
  TEST_ASSERT_FALSE_MESSAGE(Ethernet.isDHCPActive(), "Expected inactive DHCP (8)");
  TEST_ASSERT_EQUAL_UINT8_ARRAY_MESSAGE(testMAC, Ethernet.macAddress(), 6, "Expected matching MAC (8)");
  TEST_ASSERT_MESSAGE(Ethernet.localIP() == kStaticIP, "Expected matching local IP (8)");
  TEST_ASSERT_MESSAGE(Ethernet.subnetMask() == kSubnetMask, "Expected matching subnet mask (8)");
  TEST_ASSERT_MESSAGE(Ethernet.gatewayIP() == kGateway, "Expected matching gateway (8)");
  TEST_ASSERT_MESSAGE(Ethernet.dnsServerIP() == kDNS, "Expected matching DNS (8)");

  TEST_ASSERT_TRUE_MESSAGE(Ethernet.begin(nullptr, INADDR_NONE, INADDR_NONE, INADDR_NONE, INADDR_NONE), "Expected start success (9)");
  TEST_ASSERT_TRUE_MESSAGE(Ethernet.isDHCPActive(), "Expected active DHCP (9)");
  TEST_ASSERT_EQUAL_UINT8_ARRAY_MESSAGE(systemMAC, Ethernet.macAddress(), 6, "Expected matching MAC (9)");

  TEST_ASSERT_TRUE_MESSAGE(Ethernet.begin(testMAC, INADDR_NONE, INADDR_NONE, INADDR_NONE, INADDR_NONE), "Expected start success (10)");
  TEST_ASSERT_TRUE_MESSAGE(Ethernet.isDHCPActive(), "Expected active DHCP (10)");
  TEST_ASSERT_EQUAL_UINT8_ARRAY_MESSAGE(testMAC, Ethernet.macAddress(), 6, "Expected matching MAC (10)");

#pragma GCC diagnostic pop
}

// Tests mDNS.
static void test_mdns() {
  if (!waitForLocalIP()) {
    return;
  }
  TEST_MESSAGE(format("Starting mDNS: %s", kTestHostname).data());
  TEST_ASSERT_TRUE_MESSAGE(MDNS.begin(kTestHostname), "Expected start success");

  TEST_ASSERT_MESSAGE(MDNS.hostname() == String{kTestHostname}, "Expected matching hostname");
}

// Tests DNS lookup.
static void test_dns_lookup() {
  if (!waitForLocalIP()) {
    return;
  }

  constexpr char kName[]{"dns.google"};
  const IPAddress ip1{8, 8, 8, 8};
  const IPAddress ip2{8, 8, 4, 4};
  // Alternative: one.one.one.one: 1.1.1.1, 1.0.0.1

  TEST_MESSAGE(format("Waiting for DNS lookup [%s]...", kName).data());
  IPAddress ip;
  uint32_t t = millis();
  TEST_ASSERT_TRUE_MESSAGE(
      DNSClient::getHostByName(kName, ip,
                               QNETHERNET_DEFAULT_DNS_LOOKUP_TIMEOUT),
      "Expected lookup success");
  TEST_MESSAGE(format("Lookup time: %" PRIu32 "ms", millis() - t).data());
  TEST_MESSAGE(format("IP: %u.%u.%u.%u", ip[0], ip[1], ip[2], ip[3]).data());
  TEST_ASSERT_MESSAGE((ip == ip1) || (ip == ip2), "Expected different IP address");

  constexpr char kName2[]{"dms.goomgle"};
  TEST_MESSAGE(format("Waiting for DNS lookup [%s]...", kName2).data());
  t = millis();
  TEST_ASSERT_FALSE(DNSClient::getHostByName(
      kName2, ip, QNETHERNET_DEFAULT_DNS_LOOKUP_TIMEOUT));
  t = millis() - t;
  TEST_MESSAGE(format("Lookup time: %" PRIu32 "ms", t).data());
  TEST_ASSERT_LESS_THAN_MESSAGE(QNETHERNET_DEFAULT_DNS_LOOKUP_TIMEOUT, t,
                                "Expected no timeout");
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
    TEST_ASSERT_NOT_EQUAL_MESSAGE(EthernetNoHardware, Ethernet.hardwareStatus(),
                                  "Expected not no hardware");
  }
}

// Waits for a link.
static bool waitForLink() {
  TEST_ASSERT_FALSE_MESSAGE(Ethernet.linkState(), "Expected link down");

  TEST_MESSAGE("Waiting for link...");
  uint32_t t = millis();
  bool result = Ethernet.waitForLink(kLinkTimeout);
  TEST_ASSERT_TRUE_MESSAGE(result, "Wait for link failed");
  TEST_MESSAGE(format("Link time: %" PRIu32 "ms", millis() - t).data());

  TEST_ASSERT_TRUE_MESSAGE(Ethernet.linkState(), "Expected link up");

  return result;
}

// Tests seeing a link.
static void test_link() {
  EthernetLinkStatus ls = Ethernet.linkStatus();
  TEST_ASSERT_TRUE_MESSAGE(ls == LinkOFF || ls == Unknown, "Expected no link");
  TEST_ASSERT_FALSE_MESSAGE(Ethernet.linkState(), "Expected no link");
  TEST_ASSERT_TRUE_MESSAGE(Ethernet.begin(kStaticIP, kSubnetMask, kGateway),
                           "Expected start success");
  TEST_ASSERT_EQUAL_MESSAGE(LinkOFF, Ethernet.linkStatus(), "Expected no link");
  TEST_ASSERT_FALSE_MESSAGE(Ethernet.linkState(), "Expected no link");

  if (!waitForLink()) {
    return;
  }

  TEST_ASSERT_EQUAL_MESSAGE(LinkON, Ethernet.linkStatus(), "Expected link");
  TEST_ASSERT_TRUE_MESSAGE(Ethernet.linkState(), "Expected link");

  Ethernet.end();

  EthernetLinkStatus status = Ethernet.linkStatus();
  TEST_ASSERT_TRUE_MESSAGE(status == LinkOFF || status == Unknown, "Expected no link");
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
  if (!waitForLink()) {
    return;
  }
  TEST_ASSERT_TRUE_MESSAGE(latch, "Expected callback to be called on up");
  TEST_ASSERT_TRUE_MESSAGE(linkState, "Expected link up in callback");
  TEST_ASSERT_TRUE_MESSAGE(Ethernet.linkState(), "Expected link up");

  latch = false;
  linkState = true;
  Ethernet.end();
  TEST_ASSERT_FALSE_MESSAGE(static_cast<bool>(Ethernet), "Expected stopped");
  TEST_MESSAGE("Waiting for link down...");
  uint32_t timer = millis();
  while (Ethernet.linkState() && (millis() - timer) < kLinkTimeout) {
    yield();
  }
  TEST_MESSAGE(format("Link down time: %" PRIu32 "ms", (millis() - timer)).data());
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
  constexpr std::time_t kEpochDiff = 2'208'988'800;

  // Epoch -> 07-Feb-2036 06:28:16
  constexpr std::time_t kBreakTime = 2'085'978'496;

  constexpr uint16_t kNTPPort = 123;

  if (!waitForLocalIP()) {
    return;
  }

  uint8_t buf[48];
  std::fill_n(buf, 48, 0);
  buf[0] = 0b00'100'011;  // LI=0, VN=4, Mode=3 (Client)

  // Set the Transmit Timestamp
  std::time_t t = std::time(nullptr);
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
  udp = std::make_unique<EthernetUDP>();
  TEST_MESSAGE("Listening on SNTP port...");
  TEST_ASSERT_TRUE_MESSAGE(udp->begin(kNTPPort), "Expected UDP listen success");

  bool validReply = false;
  uint32_t sntpTime = 0;
  uint32_t timer = millis();
  uint32_t resendTimer = millis() + kSNTPResendTimeout;
  bool first = true;

  while ((millis() - timer) < kSNTPTimeout) {
    // Do SNTP resends
    if ((millis() - resendTimer) >= kSNTPResendTimeout) {
      if (first) {
        TEST_MESSAGE("Sending SNTP request...");
        first = false;
      } else {
        TEST_MESSAGE("Resending SNTP request...");
      }
      TEST_ASSERT_TRUE_MESSAGE(
          udp->send(Ethernet.gatewayIP(), kNTPPort, buf, 48),
          "Expected UDP send success");
      resendTimer = millis();
    }

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
  if (!validReply) {
    return;
  }

  TEST_MESSAGE(format("SNTP reply time: %" PRIu32 "ms",
                      (millis() - timer)).data());

  if ((sntpTime & 0x80000000U) == 0) {
    // See: Section 3, "NTP Timestamp Format"
    sntpTime += kBreakTime;
  } else {
    sntpTime -= kEpochDiff;
  }

  // Print the time
  std::time_t time = sntpTime;
  std::tm *tm = std::gmtime(&time);
  TEST_MESSAGE(format("SNTP reply: %04u-%02u-%02u %02u:%02u:%02u (UTC)",
                      tm->tm_year + 1900, tm->tm_mon + 1, tm->tm_mday,
                      tm->tm_hour, tm->tm_min, tm->tm_sec).data());
}

static void test_udp_receive_queueing() {
  constexpr uint16_t kPort = 1025;

  TEST_ASSERT_TRUE_MESSAGE(Ethernet.begin(kStaticIP, kSubnetMask, kGateway),
                           "Expected successful Ethernet start");
  Ethernet.setLinkState(true);  // send() won't work unless there's a link

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
  TEST_ASSERT_MESSAGE(udp->size() > 0 && udp->data()[0] == 2, "Expected packet 2 data");
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
  TEST_ASSERT_MESSAGE(udp->size() > 0 && udp->data()[0] == 3, "Expected packet 3 data");
  TEST_ASSERT_EQUAL_MESSAGE(1, udp->parsePacket(), "Expected packet 4 with size 1");
  TEST_ASSERT_MESSAGE(udp->size() > 0 && udp->data()[0] == 4, "Expected packet 4 data");

  udp->stop();
}

static void test_udp_receive_timestamp() {
  constexpr uint16_t kPort = 1025;

  TEST_ASSERT_TRUE_MESSAGE(Ethernet.begin(kStaticIP, kSubnetMask, kGateway),
                           "Expected successful Ethernet start");
  Ethernet.setLinkState(true);  // send() won't work unless there's a link

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
  TEST_ASSERT_MESSAGE(udp->size() > 0 && udp->data()[0] == b, "Expected packet data");

  TEST_ASSERT_GREATER_OR_EQUAL_MESSAGE(t, udp->receivedTimestamp(), "Expected valid timestamp");

  udp->stop();
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

  udp->stop();
}

// Tests IP DiffServ value.
static void test_udp_options() {
  TEST_ASSERT_TRUE_MESSAGE(Ethernet.begin(kStaticIP, kSubnetMask, kGateway),
                           "Expected successful Ethernet start");

  udp = std::make_unique<EthernetUDP>();

  TEST_ASSERT_TRUE(udp->setOutgoingDiffServ(0xa5));
  TEST_ASSERT_EQUAL(0xa5, udp->outgoingDiffServ());
  TEST_ASSERT_TRUE(udp->setOutgoingDiffServ(0));
  TEST_ASSERT_EQUAL(0, udp->outgoingDiffServ());
}

static void test_udp_zero_length() {
  constexpr uint16_t kPort = 1025;

  TEST_ASSERT_TRUE_MESSAGE(Ethernet.begin(kStaticIP, kSubnetMask, kGateway),
                           "Expected successful Ethernet start");
  Ethernet.setLinkState(true);  // send() won't work unless there's a link

  // Create and listen
  udp = std::make_unique<EthernetUDP>();
  TEST_ASSERT_EQUAL_MESSAGE(1, udp->begin(kPort), "Expected UDP listen success");

  TEST_ASSERT_EQUAL_MESSAGE(-1, udp->parsePacket(), "Expected nothing there");

  // Send a packet with send()
  TEST_ASSERT_TRUE_MESSAGE(udp->send(Ethernet.localIP(), kPort, nullptr, 0),
                           "Expected packet send success");

  // Test that we actually received the packet
  TEST_ASSERT_EQUAL_MESSAGE(0, udp->parsePacket(), "Expected packet with size 0");

  // Send a packet with beginPacket()/endPacket()
  TEST_ASSERT_EQUAL_MESSAGE(1, udp->beginPacket(Ethernet.localIP(), kPort),
                           "Expected beginPacket() success");
  TEST_ASSERT_EQUAL_MESSAGE(1, udp->endPacket(), "Expected endPacket() success");

  // Test that we actually received the packet
  TEST_ASSERT_EQUAL_MESSAGE(0, udp->parsePacket(), "Expected packet with size 0");

  TEST_ASSERT_EQUAL_MESSAGE(-1, udp->parsePacket(), "Expected nothing there");

  udp->stop();
}

static void test_udp_diffserv() {
  constexpr uint16_t kPort = 1025;
  constexpr uint8_t kDiffServ = (0x2c << 2) | 1;

  TEST_ASSERT_TRUE_MESSAGE(Ethernet.begin(kStaticIP, kSubnetMask, kGateway),
                           "Expected successful Ethernet start");
  Ethernet.setLinkState(true);  // send() won't work unless there's a link

  // Create and listen
  udp = std::make_unique<EthernetUDP>();
  TEST_ASSERT_EQUAL_MESSAGE(1, udp->begin(kPort), "Expected UDP listen success");
  udp->setOutgoingDiffServ(kDiffServ);
  TEST_ASSERT_EQUAL_UINT8_MESSAGE(kDiffServ, udp->outgoingDiffServ(),
                                  "Expected correct outgoing DiffServ");

  uint8_t b = 13;

  // Send a packet
  TEST_ASSERT_TRUE_MESSAGE(udp->send(Ethernet.localIP(), kPort, &b, 1),
                           "Expected packet send success");

  // Test that we actually received the packet
  TEST_ASSERT_EQUAL_MESSAGE(1, udp->parsePacket(), "Expected packet with size 1");
  TEST_ASSERT_MESSAGE(udp->size() > 0 && udp->data()[0] == b, "Expected packet data");
  TEST_ASSERT_EQUAL_UINT8_MESSAGE(kDiffServ, udp->receivedDiffServ(), "Expected matching DiffServ");

  udp->stop();
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
#undef HOST
  constexpr uint16_t kPort = 80;

  if (!waitForLocalIP()) {
    return;
  }

  client = std::make_unique<EthernetClient>();
  TEST_ASSERT_EQUAL_MESSAGE(1000, client->connectionTimeout(), "Expected default connection timeout");
  client->setConnectionTimeout(kConnectTimeout);
  TEST_ASSERT_EQUAL_MESSAGE(kConnectTimeout, client->connectionTimeout(), "Expected set timeout");

  TEST_ASSERT_FALSE_MESSAGE(static_cast<bool>(*client), "Expected not connected");
  TEST_ASSERT_EQUAL_MESSAGE(false, client->connected(), "Expected not connected (no data)");

  // Connect and send the request
  TEST_MESSAGE("Connecting and sending HTTP HEAD request...");
  uint32_t t = millis();
  TEST_ASSERT_EQUAL_MESSAGE(true, client->connect(kHost, kPort), "Expected connect success");
  TEST_ASSERT_TRUE_MESSAGE(static_cast<bool>(*client), "Expected connected");
  TEST_ASSERT_EQUAL_MESSAGE(true, client->connected(), "Expected connected (or data)");
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

  TEST_ASSERT_EQUAL_MESSAGE(false, client->connected(), "Expected not connected (no more data)");
  TEST_ASSERT_FALSE_MESSAGE(static_cast<bool>(*client), "Expected not connected");
}

static void test_client_write_single_bytes() {
#define HOST "www.example.com"
  constexpr char kHost[]{HOST};
  constexpr char kRequest[]{
      "HEAD / HTTP/1.1\r\n"
      "Host: " HOST "\r\n"
      "Connection: close\r\n"
      "\r\n"
  };
#undef HOST
  constexpr uint16_t kPort = 80;

  if (!waitForLocalIP()) {
    return;
  }

  client = std::make_unique<EthernetClient>();
  client->setConnectionTimeout(kConnectTimeout);

  // Connect and send the request
  TEST_MESSAGE("Connecting and sending HTTP HEAD request...");
  uint32_t t = millis();
  TEST_ASSERT_EQUAL_MESSAGE(true, client->connect(kHost, kPort), "Expected connect success");
  TEST_ASSERT_TRUE_MESSAGE(static_cast<bool>(*client), "Expected connected");
  TEST_MESSAGE(format("Lookup and connect time: %" PRIu32 "ms", millis() - t).data());

  size_t len = std::strlen(kRequest);
  for (size_t i = 0; i < len; i++) {
    while (client->write(kRequest[i]) == 0) {
      // Try until written
    }
  }
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
}

static void test_client_connectNoWait() {
  constexpr uint16_t kPort = 80;

  TEST_ASSERT_TRUE_MESSAGE(Ethernet.begin(kStaticIP, kSubnetMask, kGateway),
                           "Expected start success");
  Ethernet.setLinkState(true);  // Use loopback

  client = std::make_unique<EthernetClient>();

  TEST_ASSERT_FALSE_MESSAGE(static_cast<bool>(*client), "Expected not connected");
  TEST_ASSERT_EQUAL_MESSAGE(false, client->connected(), "Expected not connected (no data)");

  // Connect
  TEST_MESSAGE("Connecting ...");
  TEST_ASSERT_EQUAL_MESSAGE(true, client->connectNoWait(Ethernet.localIP(), kPort),
                            "Expected connect success");
  TEST_ASSERT_FALSE_MESSAGE(static_cast<bool>(*client), "Expected not connected");
  TEST_ASSERT_EQUAL_MESSAGE(false, client->connected(), "Expected not connected (no data)");

  client->close();
}

static void test_client_timeout() {
  constexpr uint16_t kPort = 1025;

  TEST_ASSERT_TRUE_MESSAGE(Ethernet.begin(kStaticIP, kSubnetMask, kGateway),
                           "Expected start success");
  Ethernet.setLinkState(true);  // Use loopback

  client = std::make_unique<EthernetClient>();
  TEST_ASSERT_EQUAL_MESSAGE(1000, client->connectionTimeout(), "Expected default connection timeout");
  TEST_ASSERT_FALSE_MESSAGE(static_cast<bool>(*client), "Expected not connected");
  TEST_ASSERT_EQUAL_MESSAGE(false, client->connected(), "Expected not connected (no data)");

  uint32_t t = millis();
  TEST_ASSERT_EQUAL_MESSAGE(false, client->connect(Ethernet.localIP(), kPort), "Expected connect failure");
  TEST_ASSERT_GREATER_OR_EQUAL_MESSAGE(1000, millis() - t, "Expected timeout duration");

  TEST_ASSERT_FALSE_MESSAGE(static_cast<bool>(*client), "Expected not connected");
  TEST_ASSERT_EQUAL_MESSAGE(false, client->connected(), "Expected not connected (no data)");

  client->close();
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

static void test_client_addr_info() {
  constexpr char kHost[]{"www.example.com"};
  constexpr uint16_t kPort = 80;

  if (!waitForLocalIP()) {
    return;
  }

  TEST_MESSAGE(format("Waiting for DNS lookup [%s]...", kHost).data());
  IPAddress hostIP;
  uint32_t t = millis();
  TEST_ASSERT_TRUE_MESSAGE(
      DNSClient::getHostByName(kHost, hostIP,
                               QNETHERNET_DEFAULT_DNS_LOOKUP_TIMEOUT),
      "Expected lookup success");
  TEST_MESSAGE(format("Lookup time: %" PRIu32 "ms", millis() - t).data());
  TEST_MESSAGE(format("IP: %u.%u.%u.%u", hostIP[0], hostIP[1], hostIP[2], hostIP[3]).data());

  client = std::make_unique<EthernetClient>();

  // Connect and check address info
  TEST_MESSAGE("Connecting...");
  client->setConnectionTimeout(kConnectTimeout);
  t = millis();
  TEST_ASSERT_EQUAL_MESSAGE(true, client->connect(hostIP, kPort), "Expected connect success");
  t = millis() - t;
  TEST_ASSERT_TRUE_MESSAGE(static_cast<bool>(*client), "Expected connected");
  TEST_MESSAGE(format("Connect time: %" PRIu32 "ms", t).data());

  TEST_ASSERT_EQUAL_MESSAGE(kPort, client->remotePort(), "Expected correct remote port");
  TEST_ASSERT_TRUE_MESSAGE(hostIP == client->remoteIP(), "Expected correct remote IP");
  TEST_ASSERT_TRUE_MESSAGE(client->localPort() >= 49152, "Expected correct local port");
  TEST_ASSERT_TRUE_MESSAGE(Ethernet.localIP() == client->localIP(), "Expected correct local IP");

  TEST_MESSAGE("Stopping client...");
  t = millis();
  client->stop();
  t = millis() - t;
  TEST_ASSERT_FALSE_MESSAGE(static_cast<bool>(*client), "Expected disconnected");
  TEST_MESSAGE(format("Stop time: %" PRIu32 "ms", t).data());
}

// Tests Nagle option and IP DiffServ value.
static void test_client_options() {
  constexpr uint16_t kPort = 80;

  TEST_ASSERT_TRUE_MESSAGE(Ethernet.begin(kStaticIP, kSubnetMask, kGateway),
                           "Expected start success");
  Ethernet.setLinkState(true);  // Use loopback

  client = std::make_unique<EthernetClient>();

  TEST_ASSERT_FALSE_MESSAGE(static_cast<bool>(*client), "Expected not connected");
  TEST_ASSERT_EQUAL_MESSAGE(false, client->connected(), "Expected not connected (no data)");

  // Connect
  TEST_ASSERT_EQUAL_MESSAGE(true, client->connectNoWait(Ethernet.localIP(), kPort),
                            "Expected connect success");

  TEST_ASSERT_TRUE(client->setNoDelay(true));
  TEST_ASSERT_TRUE(client->isNoDelay());
  TEST_ASSERT_TRUE(client->setNoDelay(false));
  TEST_ASSERT_FALSE(client->isNoDelay());

  TEST_ASSERT_TRUE(client->setOutgoingDiffServ(0xa5));
  TEST_ASSERT_EQUAL(0xa5, client->outgoingDiffServ());
  TEST_ASSERT_TRUE(client->setOutgoingDiffServ(0));
  TEST_ASSERT_EQUAL(0, client->outgoingDiffServ());

  client->close();
}

static void test_client_diffserv() {
  constexpr uint16_t kPort = 80;
  constexpr uint8_t kDiffServ = (0x2c << 2) | 1;
  constexpr char kHost[]{"www.example.com"};

  if (!waitForLocalIP()) {
    return;
  }

  client = std::make_unique<EthernetClient>();
  client->setConnectionTimeout(kConnectTimeout);

  // Check that can't set DiffServ before connect
  TEST_ASSERT_FALSE_MESSAGE(static_cast<bool>(*client), "Expected not connected");
  TEST_ASSERT_EQUAL_MESSAGE(false, client->connected(), "Expected not connected (no data)");
  TEST_ASSERT_FALSE_MESSAGE(client->setOutgoingDiffServ(kDiffServ), "Expected can't set DiffServ");

  // Connect and set DiffServ
  TEST_MESSAGE("Connecting ...");
  TEST_ASSERT_EQUAL_MESSAGE(true, client->connect(kHost, kPort), "Expected connect success");
  TEST_ASSERT_TRUE_MESSAGE(static_cast<bool>(*client), "Expected connected");
  TEST_ASSERT_EQUAL_MESSAGE(true, client->connected(), "Expected connected (or data)");
  TEST_ASSERT_TRUE_MESSAGE(client->setOutgoingDiffServ(kDiffServ), "Expected can set DiffServ");
  TEST_ASSERT_EQUAL_UINT8_MESSAGE(kDiffServ, client->outgoingDiffServ(), "Expected matching DiffServ");

  client->close();
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

// Tests constructing EthernetServer with an int.
static void test_server_construct_int_port() {
  constexpr int port = 1025;

  server = std::make_unique<EthernetServer>(port);
  TEST_ASSERT_EQUAL(uint16_t{1025}, server->port());
}

// Tests setting a server port of zero to have the system choose one.
static void test_server_zero_port() {
  TEST_ASSERT_TRUE_MESSAGE(Ethernet.begin(kStaticIP, kSubnetMask, kGateway),
                           "Expected successful Ethernet start");

  server = std::make_unique<EthernetServer>();

  TEST_ASSERT_FALSE_MESSAGE(static_cast<bool>(*server), "Expected not listening");
  TEST_ASSERT_EQUAL_MESSAGE(-1, server->port(), "Expected invalid port");
  TEST_ASSERT_TRUE_MESSAGE(server->begin(0), "Expected TCP listen success");
  TEST_ASSERT_TRUE_MESSAGE(static_cast<bool>(*server), "Expected listening");
  int32_t port = server->port();
  TEST_ASSERT_NOT_EQUAL_MESSAGE(0, port, "Expected non-zero port");
  TEST_MESSAGE(format("Server port = %" PRId32, port).data());
  server->end();
  TEST_ASSERT_FALSE_MESSAGE(static_cast<bool>(*server), "Expected not listening");
  TEST_ASSERT_EQUAL_MESSAGE(-1, server->port(), "Expected invalid port");

  server = std::make_unique<EthernetServer>(0);

  TEST_ASSERT_FALSE_MESSAGE(static_cast<bool>(*server), "Expected not listening");
  TEST_ASSERT_EQUAL_MESSAGE(0, server->port(), "Expected zero port");
  server->begin();
  TEST_ASSERT_TRUE_MESSAGE(static_cast<bool>(*server), "Expected listening");
  TEST_ASSERT_NOT_EQUAL_MESSAGE(0, server->port(), "Expected zero port");
  server->end();
  TEST_ASSERT_FALSE_MESSAGE(static_cast<bool>(*server), "Expected not listening");
  TEST_ASSERT_EQUAL_MESSAGE(-1, server->port(), "Expected invalid port");
}

// Tests server accept().
static void test_server_accept() {
  constexpr uint16_t kPort = 1025;

  TEST_ASSERT_TRUE_MESSAGE(Ethernet.begin(kStaticIP, kSubnetMask, kGateway),
                           "Expected successful Ethernet start");
  Ethernet.setLinkState(true);  // connect() won't work unless there's a link

  server = std::make_unique<EthernetServer>();
  client = std::make_unique<EthernetClient>();

  TEST_ASSERT_TRUE_MESSAGE(server->beginWithReuse(kPort), "Expected listen success");
  TEST_ASSERT_TRUE_MESSAGE(client->connect(Ethernet.localIP(), kPort), "Expected connect success");
  EthernetClient c = server->accept();
  TEST_ASSERT_TRUE_MESSAGE(c, "Expected accepted connection");
  c.close();
  client->close();
  server->end();
}

// Tests state from some of the other classes.
static void test_other_state() {
  TEST_ASSERT_EQUAL_MESSAGE(DNS_MAX_SERVERS, DNSClient::maxServers(), "Expected default DNS max. servers");
  TEST_ASSERT_EQUAL_MESSAGE(MEMP_NUM_IGMP_GROUP > 0 ? MEMP_NUM_IGMP_GROUP - 1 : 0,
                            Ethernet.maxMulticastGroups(),
                            "Expected default max. multicast groups");
  TEST_ASSERT_EQUAL_MESSAGE(driver_get_mtu(), Ethernet.mtu(), "Expected default MTU");
  TEST_ASSERT_EQUAL_MESSAGE(driver_get_max_frame_len(), EthernetFrame.maxFrameLen(),
                            "Expected default max. frame len");
  TEST_ASSERT_EQUAL_MESSAGE(64, EthernetFrame.minFrameLen(), "Expected default min. frame len");
  TEST_ASSERT_EQUAL_MESSAGE(MDNS_MAX_SERVICES, MDNS.maxServices(), "Expected default mDNS max. services");
}

// Tests EthernetFrame.
static void test_raw_frames() {
  constexpr uint8_t srcMAC[6]{QNETHERNET_DEFAULT_MAC_ADDRESS};
  constexpr uint8_t data[10]{0, 1, 2, 3, 4, 5, 6, 7, 8, 9};

  Ethernet.setDHCPEnabled(false);
  TEST_ASSERT_TRUE_MESSAGE(Ethernet.begin(), "Expected Ethernet start success");

  // Check that there's nothing there.
  TEST_ASSERT_EQUAL_MESSAGE(-1, EthernetFrame.parseFrame(), "Expected nothing there");

  // Create and listen
  EthernetFrame.beginFrame(Ethernet.macAddress(), srcMAC, sizeof(data));
  EthernetFrame.write(data, sizeof(data));

  uint32_t t = millis();  // Current timestamp

  TEST_ASSERT_TRUE_MESSAGE(EthernetFrame.endFrame(), "Expected send success");

  // Test that we actually received the packet
  TEST_ASSERT_EQUAL(14 + sizeof(data), EthernetFrame.parseFrame());
  TEST_ASSERT_EQUAL(14 + sizeof(data), EthernetFrame.size());

  if (EthernetFrame.size() > 0) {  // Avoid potentially accessing NULL data
    const uint8_t *frameData = EthernetFrame.data();
    TEST_ASSERT_EQUAL_UINT8_ARRAY(Ethernet.macAddress(), &frameData[0], 6);
    TEST_ASSERT_EQUAL_UINT8_ARRAY(srcMAC, &frameData[6], 6);
    TEST_ASSERT_EQUAL(sizeof(data) >> 8, frameData[12]);
    TEST_ASSERT_EQUAL(sizeof(data) & 0xff, frameData[13]);
    TEST_ASSERT_EQUAL_UINT8_ARRAY(data, &frameData[14], 10);

    TEST_ASSERT_GREATER_OR_EQUAL_MESSAGE(t, EthernetFrame.receivedTimestamp(),
                                         "Expected valid timestamp");
  }
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
  RUN_TEST(test_builtin_mac);
  RUN_TEST(test_set_mac);
  RUN_TEST(test_get_mac);
  RUN_TEST(test_other_null_mac);
  RUN_TEST(test_null_group);
  RUN_TEST(test_null_frame);
  RUN_TEST(test_dhcp);
  RUN_TEST(test_static_ip);
  RUN_TEST(test_arduino_begin);
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
  RUN_TEST(test_udp_options);
  RUN_TEST(test_udp_zero_length);
  RUN_TEST(test_udp_diffserv);
  RUN_TEST(test_client);
  RUN_TEST(test_client_write_single_bytes);
  RUN_TEST(test_client_connectNoWait);
  RUN_TEST(test_client_timeout);
  RUN_TEST(test_client_state);
  RUN_TEST(test_client_addr_info);
  RUN_TEST(test_client_options);
  RUN_TEST(test_client_diffserv);
  RUN_TEST(test_server_state);
  RUN_TEST(test_server_construct_int_port);
  RUN_TEST(test_server_zero_port);
  RUN_TEST(test_server_accept);
  RUN_TEST(test_other_state);
  RUN_TEST(test_raw_frames);
  UNITY_END();
}

// Main program loop.
void loop() {
}
