// SPDX-FileCopyrightText: (c) 2023 Shawn Silverman <shawn@pobox.com>
// SPDX-License-Identifier: MIT

// AltcpTemplate shows how to use the altcp framework for creating
// custom connections. It defines a function that returns a TLS or
// proxy configuration.
//
// Prerequisites: Enable the LWIP_ALTCP and optionally the
//                LWIP_ALTCP_TLS lwIP options.
// Big caveat: This example will only do TLS if there's an available
//             TLS implementation.
//
// This file is part of the QNEthernet library.

#include <QNEthernet.h>
#if LWIP_ALTCP
#include <lwip/altcp_tcp.h>
#if LWIP_ALTCP_TLS
#include <lwip/altcp_tls.h>
#endif  // LWIP_ALTCP_TLS
#include <lwip/apps/altcp_proxyconnect.h>
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
constexpr uint16_t kPort = 80;   // TLS generally uses port 443

#if LWIP_ALTCP

// For proxy connections
constexpr bool kUseProxy = false;  // Whether to use altcp_proxyconnect
struct altcp_proxyconnect_config proxyConfig {
  IPADDR4_INIT_BYTES(0, 0, 0, 0),  // Change this
  8080,                            // TLS can be on 3128
};
// Note: There's also a TLS proxyconnect; this can be an exercise for the reader

// The qnethernet_get_allocator() function fills in the given
// allocator with an appropriate allocator function and argument,
// using the IP address and port to choose one. If creating the socket
// failed then qnethernet_free_allocator() is called to free any
// resources that haven't already been freed.
std::function<void(const ip_addr_t *, uint16_t, altcp_allocator_t *)>
    qnethernet_get_allocator = [](const ip_addr_t *ipaddr, uint16_t port,
                                  altcp_allocator_t *allocator) {
      printf("[[qnethernet_allocator_arg(%s, %u): %s]]\r\n",
             ipaddr_ntoa(ipaddr), port,
             (ipaddr == NULL) ? "Listen" : "Connect");

      if (kUseProxy) {
        if (ipaddr != nullptr) {
          printf("[[Using proxy: %s:%u]]\r\n",
                 ipaddr_ntoa(&proxyConfig.proxy_addr), proxyConfig.proxy_port);
          allocator->alloc = &altcp_proxyconnect_alloc;
          allocator->arg   = &proxyConfig;
        } else {
          allocator->alloc = &altcp_tcp_alloc;
          allocator->arg = nullptr;
        }
      } else {
        switch (port) {
          case 80:
            allocator->alloc = &altcp_tcp_alloc;
            allocator->arg   = nullptr;
            break;
  #if LWIP_ALTCP_TLS
          case 443:
            allocator->alloc = &altcp_tls_alloc;
            allocator->arg   = get_altcp_tls_config();  // TBD by you, the user
            break;
  #endif  // LWIP_ALTCP_TLS
          default:
            break;
        }
      }
    };

// The qnethernet_free_allocator() function frees any resources
// allocated with qnethernet_get_allocator() if they haven't already
// been freed. It is up to the implementation to decide if a resource
// has already been freed or not.
std::function<void(const altcp_allocator_t *)> qnethernet_free_allocator =
    [](const altcp_allocator_t *allocator) {
      printf("[[qnethernet_free_allocator()]]\r\n");
      // For the proxyConfig and for altcp_tcp_alloc,
      // there's nothing to free
#if LWIP_ALTCP_TLS
      if (allocator->alloc == &altcp_tls_alloc) {
        struct altcp_tls_config *config =
            (struct altcp_tls_config *)allocator->arg;
        altcp_tls_free_config(config);  // <-- Example without can-free check
            // Implementation MUST NOT free if already freed
      }
#endif  // LWIP_ALTCP_TLS
    };

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
    client.setConnectionTimeout(30'000);  // Proxies can take longer, maybe
  }
#endif  // LWIP_ALTCP
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
