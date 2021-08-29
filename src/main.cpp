// main.cpp is a testing playground for this library.
// This file is part of the QNEthernet library.
// (c) 2021 Shawn Silverman

// C++ includes
#include <utility>

#include <Arduino.h>

#include <lwip/dhcp.h>
#include <lwip/dns.h>
#include <lwip/ip_addr.h>
#include <lwip/netif.h>

#include "QNEthernet.h"
#include "QNMDNS.h"
#include "OSC.h"

namespace qn = ::qindesign::network;

qn::EthernetUDP udpIn{};
qn::EthernetClient client{};
qn::EthernetServer server{5000};

qn::MDNS mdns;

static void netif_status_callback(struct netif *netif) {
  static char ip[IPADDR_STRLEN_MAX];
  static char mask[IPADDR_STRLEN_MAX];
  static char gw[IPADDR_STRLEN_MAX];
  static char dns[IPADDR_STRLEN_MAX];

  Serial.printf(
      "netif status changed: ip=%s, mask=%s, gw=%s",
      ipaddr_ntoa_r(netif_ip_addr4(netif), ip, IPADDR_STRLEN_MAX),
      ipaddr_ntoa_r(netif_ip_netmask4(netif), mask, IPADDR_STRLEN_MAX),
      ipaddr_ntoa_r(netif_ip_gw4(netif), gw, IPADDR_STRLEN_MAX));

  for (int i = 0; i < DNS_MAX_SERVERS; i++) {
    if (dns_getserver(i)->addr == 0) {
      continue;
    }
    ipaddr_ntoa_r(dns_getserver(i), dns, IPADDR_STRLEN_MAX);
    Serial.printf(", dns(%d)=%s", i + 1, dns);
  }
  Serial.println();
}

static void link_status_callback(struct netif *netif) {
  Serial.printf("enet link status: %s\n",
                netif_is_link_up(netif) ? "up" : "down");
}

void setupOSC();
void setupHTTPClient();
void setupServer();
void setupServerAvail();
void loopOSC();
void loopHTTPClient();
void loopServer();
void loopServerAvail();

void setup() {
  Serial.begin(115200);
  while (!Serial && millis() < 4000) {
    // Wait for Serial to initialize
  }
  delay(4000);
  Serial.println(CrashReport);
  Serial.println("Starting...");

  uint8_t mac[6];
  Ethernet.macAddress(mac);
  Serial.printf("MAC = %02x:%02x:%02x:%02x:%02x:%02x\n",
                mac[0], mac[1], mac[2], mac[3], mac[4], mac[5]);

  Ethernet.begin();
  netif_set_status_callback(netif_default, netif_status_callback);
  netif_set_link_callback(netif_default, link_status_callback);

  setupOSC();
  // setupHTTPClient();
  // setupServer();
  // setupServerAvail();
}

void setupOSC() {
  udpIn.begin(8000);
  Serial.println("Starting mDNS");
  mdns.begin("qeth");
  mdns.addService("_osc", "_udp", 8000);
}

void setupHTTPClient() {
}

void setupServer() {
  server.begin();
}

void setupServerAvail() {
  server.begin();
}

void loop() {
  loopOSC();
  // loopHTTPClient();
  // loopServer();
  // loopServerAvail();
}

void loopOSC() {
  int size = udpIn.parsePacket();
  if (size > 0) {
    unsigned char buf[size];
    udpIn.read(buf, size);
    printOSC(Serial, buf, size);
  }
}

int httpClientState = 0;

void loopHTTPClient() {
  switch (httpClientState) {
    case 0:
      if (Ethernet.localIP() != 0) {
        if (client.connect("google.com", 80)) {// google.com: 172, 217, 6, 46, example.com: 93, 184, 216, 34
          Serial.println("connected");
          client.print("GET /search?q=arduino HTTP/1.0\r\n");
          // client.print("GET / HTTP/1.1\r\n");
          // client.print("Host: example.com\r\n");
          // client.print("Connection: close\r\n");
          client.print("\r\n");
          // client.flush();
          httpClientState = 1;
        } else {
          Serial.println("connection failed");
          httpClientState = 2;
        }
      }
      break;

    case 1: {
      int avail = client.available();
      if (avail > 0) {
        for (int i = 0; i < avail; i++) {
          Serial.print((char) client.read());
        }
      }
      if (!client.connected()) {
        Serial.println("disconnecting.");
        client.stop();
        httpClientState = 2;
      }
      break;
    }

    default:
      break;
  }
}

qn::EthernetClient clients[8];

void loopServer() {
  // check for any new client connecting, and say hello (before any incoming data)
  qn::EthernetClient newClient = server.accept();
  if (newClient.connected()) {
    for (int i = 0; i < 8; i++) {
      if (!clients[i].connected()) {
        Serial.printf("Client %d\n", i);
        newClient.print("Hello, client number: ");
        newClient.println(i);
        // Once we "accept", the client is no longer tracked by EthernetServer
        // so we must store it into our list of clients
        clients[i] = std::move(newClient);
        break;
      }
    }
  }

  // check for incoming data from all clients
  for (int i = 0; i < 8; i++) {
    while (/*clients[i].connected() &&*/ clients[i].available() > 0) {
      // read incoming data from the client
      Serial.write(clients[i].read());
    }
  }

  // // stop any clients which disconnect
  // for (int i = 0; i < 8; i++) {
  //   if (!clients[i]) {
  //     Serial.printf("Stopping client %d\n", i);
  //     clients[i].stop();
  //   }
  // }
}

void loopServerAvail() {
  // if an incoming client connects, there will be bytes available to read:
  qn::EthernetClient client = server.available();
  if (client.connected()) {
    // read bytes from the incoming client and write them back
    // to any clients connected to the server:
    server.write(client.read());
  }
}
