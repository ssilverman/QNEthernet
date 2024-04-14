// SPDX-FileCopyrightText: (c) 2023-2024 Shawn Silverman <shawn@pobox.com>
// SPDX-License-Identifier: AGPL-3.0-or-later

// AltcpTemplate shows how to use the altcp framework for creating
// custom connections. It defines a function that returns a TLS or
// proxy configuration.
//
// Prerequisites: Enable the LWIP_ALTCP and optionally the
//                LWIP_ALTCP_TLS lwIP options. The altcp_tls_adapter
//                functions could also be disabled by disabling
//                the QNETHERNET_ALTCP_TLS_ADAPTER option (it's off
//                by default). (See the lwipopts.h and qnethernet_opts.h library
//                files or the project build options.)
// Big caveat: This example will only do TLS if there's an available
//             TLS implementation.
//
// This file is part of the QNEthernet library.

#include <QNEthernet.h>
#if LWIP_ALTCP
#include <lwip/altcp_tcp.h>
#if LWIP_ALTCP_TLS
#if LWIP_ALTCP_TLS_MBEDTLS
#include <mbedtls.h>
#endif  // LWIP_ALTCP_TLS_MBEDTLS
#include <lwip/altcp_tls.h>
#endif  // LWIP_ALTCP_TLS
#include <lwip/apps/altcp_proxyconnect.h>
#endif  // LWIP_ALTCP

using namespace qindesign::network;

constexpr uint32_t kDHCPTimeout = 15000;  // 15 seconds

// Connection information
#define HOST "www.example.com"
constexpr char kHost[]{HOST};
constexpr char kRequest[]{
    "HEAD / HTTP/1.1\r\n"
    "Host: " HOST "\r\n"
    "Connection: close\r\n"
    "\r\n"
};
constexpr uint16_t kPort = 80;   // TLS generally uses port 443

#if LWIP_ALTCP

// For proxy connections
extern const bool kUseProxy = false;  // Whether to use altcp_proxyconnect
struct altcp_proxyconnect_config proxyConfig{
    IPADDR4_INIT_BYTES(0, 0, 0, 0),  // Change this
    8080,                            // TLS can be on 3128
};
// Note: There's also a TLS proxyconnect; this can be an exercise for the reader

#endif  // LWIP_ALTCP

EthernetClient client;

bool disconnectedPrintLatch = false;  // Print "disconnected" only once
size_t dataCount = 0;

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

  // Get an IP address
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

  // Connect and send the request
  printf("Connecting and sending request...\r\n");
#if LWIP_ALTCP
  if (kUseProxy) {
    client.setConnectionTimeout(30000);  // Proxies can take longer, maybe
  }
#endif  // LWIP_ALTCP
  if (!client.connect(kHost, kPort)) {
    printf("Failed to connect\r\n");
    disconnectedPrintLatch = true;
  } else {
    client.writeFully(kRequest);
    client.flush();
    dataCount = 0;
    printf("[Awaiting response...]\r\n");
  }
}

// Main program loop.
void loop() {
  // Read the response
  if (client.connected()) {
    int avail = client.available();
    if (avail > 0) {
      dataCount += avail;
      for (int i = 0; i < avail; i++) {
        putc(client.read(), stdout);
      }
    }
  } else {
    if (!disconnectedPrintLatch) {
      disconnectedPrintLatch = true;
      printf("[Client disconnected]\r\n"
             "[Data count = %zu]\r\n", dataCount);
    }
  }
}
