// SPDX-FileCopyrightText: (c) 2022-2023 Shawn Silverman <shawn@pobox.com>
// SPDX-License-Identifier: MIT

// RawFrameMonitor prints all unknown raw Ethernet frames. Known frame
// types won't be printed and include:
// 1. IPv4 (0x0800)
// 2. ARP  (0x0806)
// 3. IPv6 (0x86DD) (if enabled)
//
// Currently, in order to receive frames not addressed to the device's
// MAC address or to a subscribed multicast address, the destination
// address must be tagged as "allowed" by calling
// `Ethernet.setMACAddressAllowed(mac, flag)`. Alternatively, enable
// promiscuous mode by defining the QNETHERNET_ENABLE_PROMISCUOUS_MODE
// macro.
//
// In order to use this example, define the
// QNETHERNET_ENABLE_RAW_FRAME_SUPPORT macro.
//
// Note: the configuration macros must be defined in the project
//       build options.
//
// This file is part of the QNEthernet library.

// C++ includes
#include <algorithm>

#include <QNEthernet.h>

using namespace qindesign::network;

// VLAN EtherType constants
constexpr uint16_t kEtherTypeVLAN = 0x8100u;
constexpr uint16_t kEtherTypeQinQ = 0x88A8u;

// We can access the frame's internal data buffer directly,
// so we don't need the following:
// // Buffer for reading frames.
// uint8_t buf[EthernetFrame.maxFrameLen()];

// Tracks the received frame count.
int frameCount = 0;

// Main program setup.
void setup() {
  Serial.begin(115200);
  while (!Serial && millis() < 4000) {
    // Wait for Serial
  }
  printf("Starting...\r\n");

  // Print the MAC address
  uint8_t mac[6];
  Ethernet.macAddress(mac);  // This is informative; it retrieves, not sets
  printf("MAC = %02x:%02x:%02x:%02x:%02x:%02x\r\n",
         mac[0], mac[1], mac[2], mac[3], mac[4], mac[5]);

  // Add listeners before starting Ethernet

  Ethernet.onLinkState([](bool state) {
    printf("[Ethernet] Link %s\r\n", state ? "ON" : "OFF");
  });

  Ethernet.onAddressChanged([]() {
    IPAddress ip = Ethernet.localIP();
    bool hasIP = (ip != INADDR_NONE);
    if (hasIP) {
      IPAddress subnet = Ethernet.subnetMask();
      IPAddress broadcast = Ethernet.broadcastIP();
      IPAddress gw = Ethernet.gatewayIP();
      IPAddress dns = Ethernet.dnsServerIP();

      printf("[Ethernet] Address changed:\r\n"
             "    Local IP     = %u.%u.%u.%u\r\n"
             "    Subnet       = %u.%u.%u.%u\r\n"
             "    Broadcast IP = %u.%u.%u.%u\r\n"
             "    Gateway      = %u.%u.%u.%u\r\n"
             "    DNS          = %u.%u.%u.%u\r\n",
             ip[0], ip[1], ip[2], ip[3],
             subnet[0], subnet[1], subnet[2], subnet[3],
             broadcast[0], broadcast[1], broadcast[2], broadcast[3],
             gw[0], gw[1], gw[2], gw[3],
             dns[0], dns[1], dns[2], dns[3]);
    } else {
      printf("[Ethernet] Address changed: No IP address\r\n");
    }
  });

  // Initialize Ethernet, in this case with DHCP
  printf("Starting Ethernet with DHCP...\r\n");
  if (!Ethernet.begin()) {
    printf("Failed to start Ethernet\r\n");
    return;
  }
}

// Main program loop.
void loop() {
  int size = EthernetFrame.parseFrame();
  if (size <= 0) {
    return;
  }

  frameCount++;

  // Access the frame's data directly instead of using read()
  // size = EthernetFrame.read(buf, size);
  const uint8_t *buf = EthernetFrame.data();
  if (size < EthernetFrame.minFrameLen() - 4) {
    printf("%d: SHORT Frame[%d]: ", frameCount, size);
    for (int i = 0; i < size; i++) {
      printf(" %02x", buf[i]);
    }
    printf("\r\n");
    return;
  }

  printf("%d: Frame[%d]:"
         " dst=%02x:%02x:%02x:%02x:%02x:%02x"
         " src=%02x:%02x:%02x:%02x:%02x:%02x",
         frameCount, size,
         buf[0], buf[1], buf[2], buf[3], buf[4], buf[5],
         buf[6], buf[7], buf[8], buf[9], buf[10], buf[11]);
  uint16_t tag = (uint16_t{buf[12]} << 8) | buf[13];

  // Tag and (possibly stacked) VLAN processing
  int payloadStart = 14;
  int vlanTagNum = 0;

  // Loop because there could be more than one of these
  while (tag == kEtherTypeQinQ) {  // IEEE 802.1ad (QinQ)
    if (payloadStart + 4 > size) {
      printf(" TRUNCATED QinQ\r\n");
      return;
    }

    uint16_t info = (uint16_t{buf[payloadStart]} << 8) | buf[payloadStart + 1];
    payloadStart += 2;
    printf(" VLAN tag %d info=%04Xh", ++vlanTagNum, info);
    tag = (uint16_t{buf[payloadStart]} << 8) | buf[payloadStart + 1];
    payloadStart += 2;
  }

  if (tag == kEtherTypeVLAN) {  // IEEE 802.1Q (VLAN tagging)
    if (payloadStart + 4 > size) {
      printf(" TRUNCATED VLAN\r\n");
      return;
    }

    uint16_t info = (uint16_t{buf[payloadStart]} << 8) | buf[payloadStart + 1];
    payloadStart += 2;
    if (vlanTagNum > 0) {
      printf(" VLAN tag %d info=%04Xh", ++vlanTagNum, info);
    } else {
      printf(" VLAN info=%04Xh", info);
    }
    tag = (uint16_t{buf[payloadStart]} << 8) | buf[payloadStart + 1];
    payloadStart += 2;
  } else if (vlanTagNum > 0) {
    printf(" MISSING VLAN");
  }
  // 'tag' now holds the length/type field

  int payloadEnd = size;
  if (tag > EthernetFrame.maxFrameLen()) {
    printf(" type=%04Xh\r\n", tag);
  } else {
    printf(" length=%u\r\n", tag);
    payloadEnd = std::min(payloadStart + tag, payloadEnd);
  }

  printf("\tpayload[%d]=", payloadEnd - payloadStart);
  for (int i = payloadStart; i < payloadEnd; i++) {
    printf(" %02x", buf[i]);
  }
  printf("\r\n");
}
