// SPDX-FileCopyrightText: (c) 2021 Shawn Silverman <shawn@pobox.com>
// SPDX-License-Identifier: MIT

// main.cpp is a testing playground for this library.
// This file is part of the QNEthernet library.

// NOTE: The server and client code in this program should not be used as an
//       example of how to structure your own code. Its structure was merely
//       adapted from https://www.arduino.cc/en/Reference/Ethernet to test if
//       the library worked using this style of API usage.

#include <Arduino.h>

#include <lwip/dns.h>
#include <lwip/ip_addr.h>
#include <lwip/netif.h>

#include "QNEthernet.h"
#include "QNMDNS.h"
#include "OSC.h"

using namespace qindesign::network;

EthernetUDP udp{};
EthernetClient client{};
EthernetServer server{5000};

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
void setupSendUDP();
void loopOSC();
void loopHTTPClient();
void loopServer();
void loopServerAvail();
void loopSendUDP();

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
  // setupSendUDP();
}

void setupOSC() {
  udp.begin(8000);
  Serial.println("Starting mDNS");
  MDNS.begin("qeth");
  MDNS.addService("_osc", "_udp", 8000);
}

void setupHTTPClient() {
}

void setupServer() {
  server.begin();
}

void setupServerAvail() {
  server.begin();
}

void setupSendUDP() {
  while (Ethernet.localIP() == INADDR_NONE) {
    delay(1000);
  }
  Serial.print("Local IP: ");
  Ethernet.localIP().printTo(Serial);
  Serial.println();
}

void loop() {
  loopOSC();
  // loopHTTPClient();
  // loopServer();
  // loopServerAvail();
  // loopSendUDP();
}

void loopOSC() {
  int size = udp.parsePacket();
  if (size > 0) {
    unsigned char buf[size];
    udp.read(buf, size);
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

EthernetClient clients[8];

void loopServer() {
  // check for any new client connecting, and say hello (before any incoming data)
  EthernetClient newClient = server.accept();
  if (newClient) {
    for (int i = 0; i < 8; i++) {
      if (!clients[i]) {
        Serial.printf("Client %d\n", i);
        newClient.print("Hello, client number: ");
        newClient.println(i);
        newClient.flush();
        // Once we "accept", the client is no longer tracked by EthernetServer
        // so we must store it into our list of clients
        clients[i] = newClient;
        break;
      }
    }
  }

  // check for incoming data from all clients
  for (int i = 0; i < 8; i++) {
    while (clients[i].available() > 0) {
      // read incoming data from the client
      Serial.write(clients[i].read());
    }
  }

  // stop any clients which disconnect
  for (int i = 0; i < 8; i++) {
    if (clients[i].connected()) {
      clients[i].stop();
    }
  }
}

void loopServerAvail() {
  // if an incoming client connects, there will be bytes available to read:
  EthernetClient client = server.available();
  if (client.connected()) {
    // read bytes from the incoming client and write them back
    // to any clients connected to the server:
    server.write(client.read());
  }
}

void loopSendUDP() {
  udp.beginPacket(IPAddress{255, 255, 255, 255}, 5000);
  udp.write("Hello!");
  udp.endPacket();
  Serial.println("Sent.");

  delay(2000);
}
