#include <Arduino.h>

extern "C" {
#include "lwip_t41.h"
#include "lwip/dhcp.h"
#include "lwip/dns.h"
#include "lwip/ip_addr.h"
#include "lwip/netif.h"
}

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

void setup() {
  Serial.begin(115200);
  while (!Serial && millis() < 4000) {
    // Wait for Serial to initialize
  }
  delay(4000);

  enet_init(NULL, NULL, NULL, NULL);
  netif_set_status_callback(netif_default, netif_status_callback);
  netif_set_link_callback(netif_default, link_status_callback);
  netif_set_up(netif_default);

  dhcp_start(netif_default);
}

void loop() {
  static elapsedMillis timer;

  enet_proc_input();

  if (timer >= 100) {
    enet_poll();
    timer = 0;
  }
}
