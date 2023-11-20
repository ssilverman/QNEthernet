// SPDX-FileCopyrightText: (c) 2023 Shawn Silverman <shawn@pobox.com>
// SPDX-License-Identifier: AGPL-3.0-or-later

// MbedTLSDemo shows how to use Mbed TLS 2.x.x.
//
// Prerequisites:
// 1. Enable the following lwIP options:
//    1. LWIP_ALTCP
//    2. LWIP_ALTCP_TLS
//    3. LWIP_ALTCP_TLS_MBEDTLS
// 2. Install the latest 2.x.x version of Mbed TLS. As of this
//    writing, the current version is 2.28.6. In order to use this
//    example program in the Arduino IDE, Mbed TLS needs to be
//    installed as a library. Please see the README for more
//    information.
//
// This file is part of the QNEthernet library.

#include <QNEthernet.h>
#if LWIP_ALTCP
#if LWIP_ALTCP_TLS && LWIP_ALTCP_TLS_MBEDTLS
#include <mbedtls.h>  // Must be included before referencing other mbedtls headers
#endif  // LWIP_ALTCP_TLS && LWIP_ALTCP_TLS_MBEDTLS
#endif  // LWIP_ALTCP

using namespace qindesign::network;

constexpr uint32_t kDHCPTimeout = 15'000;  // 15 seconds

// Connection information
#define HOST "www.example.com"
constexpr char kHost[]{HOST};
constexpr char kRequest[]{
    "HEAD / HTTP/1.1\r\n"
    "Host: " HOST "\r\n"
    "Connection: close\r\n"
    "\r\n"
};
constexpr uint16_t kPort = 443;   // HTTPS generally uses port 443

EthernetClient client;

bool disconnectedPrintLatch = false;  // Print "disconnected" only once
size_t dataCount = 0;

// Mbed TLS adapter functions

std::function<bool(const ip_addr_t *, uint16_t)> qnethernet_mbedtls_is_tls =
    [](const ip_addr_t *ipaddr, uint16_t port) {
      printf("[[qnethernet_mbedtls_is_tls(%s, %u): %s]]\r\n",
             (ipaddr == nullptr) ? nullptr : ipaddr_ntoa(ipaddr), port,
             (ipaddr == nullptr) ? "Listen" : "Connect");

      if (port == 443) {
        if (ipaddr == nullptr) {
          printf("qnethernet_mbedtls_is_tls: creating server config...\r\n");
        } else {
          printf("qnethernet_mbedtls_is_tls: creating client config...\r\n");
        }
        return true;
      }
      return false;
    };

std::function<void(const uint8_t *&, size_t &)>
    qnethernet_altcp_tls_client_cert =
        [](const uint8_t *&cert, size_t &cert_len) {
          // All are initialized to NULL or zero
          printf("[[qnethernet_altcp_tls_client_cert]] No certificate\r\n");
        };

std::function<void(const uint8_t *&, size_t &,
                   const uint8_t *&, size_t &,
                   const uint8_t *&, size_t &)>
    qnethernet_altcp_tls_server_cert =
        [](const uint8_t *&privkey,      size_t &privkey_len,
           const uint8_t *&privkey_pass, size_t &privkey_pass_len,
           const uint8_t *&cert,         size_t &cert_len) {
          // All are initialized to NULL or zero
          printf("[[qnethernet_altcp_tls_server_cert]] No certificate\r\n");
        };

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
  if (client.connect(kHost, kPort) != 1) {
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
