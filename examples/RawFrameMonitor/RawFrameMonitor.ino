// SPDX-FileCopyrightText: (c) 2022 Shawn Silverman <shawn@pobox.com>
// SPDX-License-Identifier: MIT

// RawFrameMonitor prints all unknown raw Ethernet frames. Known frame types
// won't be printed and include:
// 1. IPv4 (0x0800)
// 2. ARP  (0x0806)
// 3. IPv6 (0x86DD) (if enabled)
//
// Currently, in order to receive frames not addressed to the device's MAC
// address or to a subscribed multicast address, the destination address must be
// tagged as "allowed".
// See: Ethernet.setMACAddressAllowed(mac, flag)
//
// This file is part of the QNEthernet library.

// C++ includes
#include <algorithm>

#include <QNEthernet.h>

using namespace qindesign::network;

// VLAN EtherType constants
constexpr uint16_t kEtherTypeVLAN = 0x8100u;
constexpr uint16_t kEtherTypeQinQ = 0x88A8u;

// Buffer for reading frames.
uint8_t buf[EthernetFrame.maxFrameLen()];

// Tracks the received frame count.
int frameCount = 0;

// Main program setup.
void setup() {
  Serial.begin(115200);
  while (!Serial && millis() < 4000) {
    // Wait for Serial to initialize
  }
  stdPrint = &Serial;  // Make printf work (a QNEthernet feature)
  printf("Starting...\n");

  // Print the MAC address
  uint8_t mac[6];
  Ethernet.macAddress(mac);  // This is informative; it retrieves, not sets
  printf("MAC = %02x:%02x:%02x:%02x:%02x:%02x\n",
         mac[0], mac[1], mac[2], mac[3], mac[4], mac[5]);

  // Initialize Ethernet, in this case with DHCP
  printf("Starting Ethernet with DHCP...\n");
  if (!Ethernet.begin()) {
    printf("Failed to start Ethernet\n");
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

  size = EthernetFrame.read(buf, size);
  if (size < EthernetFrame.minFrameLen() - 4) {
    printf("%d: SHORT Frame[%d]: ", frameCount, size);
    for (int i = 0; i < size; i++) {
      printf(" %02x", buf[i]);
    }
    printf("\n");
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
      printf(" TRUNCATED QinQ\n");
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
      printf(" TRUNCATED VLAN\n");
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
    printf(" type=%04Xh\n", tag);
  } else {
    printf(" length=%d\n", tag);
    payloadEnd = std::min(payloadStart + tag, payloadEnd);
  }

  printf("\tpayload[%d]=", payloadEnd - payloadStart);
  for (int i = payloadStart; i < payloadEnd; i++) {
    printf(" %02x", buf[i]);
  }
  printf("\n");
}
