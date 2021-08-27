#include <Arduino.h>

#include <EventResponder.h>
#include <lwip/dhcp.h>
#include <lwip/dns.h>
#include <lwip/ip_addr.h>
#include <lwip/netif.h>

#include "Ethernet.h"
#include "EthernetClient.h"
#include "EthernetUDP.h"
#include "OSC.h"

qindesign::network::Ethernet eth{};
qindesign::network::EthernetUDP udpIn{};
qindesign::network::EthernetClient client{};

EventResponder ethEvent;

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
void loopOSC();
void loopHTTPClient();

void setup() {
  Serial.begin(115200);
  while (!Serial && millis() < 4000) {
    // Wait for Serial to initialize
  }
  delay(4000);
  Serial.println("Starting...");

  uint8_t mac[6];
  eth.macAddress(mac);
  Serial.printf("MAC = %02x:%02x:%02x:%02x:%02x:%02x\n",
                mac[0], mac[1], mac[2], mac[3], mac[4], mac[5]);

  eth.begin();
  netif_set_status_callback(netif_default, netif_status_callback);
  netif_set_link_callback(netif_default, link_status_callback);

  ethEvent.attach([](EventResponderRef r) {
    eth.loop();
    r.triggerEvent();
  });
  ethEvent.triggerEvent();

  // setupOSC();
  setupHTTPClient();
}

void setupOSC() {
  udpIn.begin(8000);
}

void setupHTTPClient() {
}

void loop() {
  // loopOSC();
  loopHTTPClient();
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
      if (eth.localIP() != 0) {
        if (client.connect(IPAddress{172, 217, 6, 46}, 80)) {  // example.com: 93, 184, 216, 34
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
