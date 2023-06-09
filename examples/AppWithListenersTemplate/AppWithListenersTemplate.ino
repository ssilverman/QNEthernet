// SPDX-FileCopyrightText: (c) 2021-2023 Shawn Silverman <shawn@pobox.com>
// SPDX-License-Identifier: MIT

// AppWithListenersTemplate shows one way to start writing a network
// application using a listener approach. It doesn't do any network
// communication, but it configures the network and shows possible
// places to hook in your own code.
//
// Not everything in this template needs to be included in your
// own application.
//
// This demonstrates:
// 1. Using Ethernet listeners,
// 2. Using `printf`,
// 3. Configuring an IP address, and
// 4. How to properly start using the network.
//
// This file is part of the QNEthernet library.

#include <QNEthernet.h>

// *** Your own includes go here

// Include everything from this namespace so that you don't have to
// namespace-qualify any QNEthernet variables or classes.
using namespace qindesign::network;

// --------------------------------------------------------------------------
//  Configuration
// --------------------------------------------------------------------------

// The DHCP timeout, in milliseconds. Set to zero to not wait and
// instead rely on the listener to inform us of an address assignment.
constexpr uint32_t kDHCPTimeout = 15'000;  // 15 seconds

// The link timeout, in milliseconds. Set to zero to not wait and
// instead rely on the listener to inform us of a link.
constexpr uint32_t kLinkTimeout = 5'000;  // 5 seconds

// Indicates whether to always try DHCP first. If this is false, then
// the value of `staticIP` determines whether DHCP is tried first. If
// true then DHCP will always be attempted first, regardless of the
// value of `staticIP`.
//
// **This is for illustration.**
constexpr bool kStartWithDHCP = true;

// Whether to wait for DHCP at program start. Note that the
// `setNetworkReady(hasIP, linkState, interfaceUp)` function will
// always be called when the address, link, or interface status
// changes, so starting DHCP and not waiting for an IP address is
// sufficient. In other words, you don't necessarily need to wait;
// you can rely on the listener telling you.
constexpr bool kWaitForDHCP = true;

// The static IP is either used as a fallback, if `kStartWithDHCP` is
// true, or it determines whether to use DHCP, if `kStartWithDHCP` is
// false. In all cases, if it's set to INADDR_NONE (zero), then DHCP
// will be used.
IPAddress staticIP{0, 0, 0, 0};//{192, 168, 1, 101};
IPAddress subnetMask{255, 255, 255, 0};
IPAddress gateway{192, 168, 1, 1};
IPAddress dnsServer = gateway;

// --------------------------------------------------------------------------
//  Program State
// --------------------------------------------------------------------------

volatile bool networkReadyLatch = false;

// --------------------------------------------------------------------------
//  Main Program
// --------------------------------------------------------------------------

// Forward declarations
void setNetworkReady(bool hasIP, bool hasLink, bool interfaceUp);

// Main program setup.
void setup() {
  // Initialize Serial output
  Serial.begin(115200);
  while (!Serial && millis() < 4000) {
    // Wait for Serial
  }
  delay(1500);  // Give external monitors a chance to start
  if (CrashReport) {  // Print any crash report
    Serial.println(CrashReport);
    CrashReport.clear();
  }
  printf("Starting...\r\n");

  // Unlike the Arduino API (which you can still use), QNEthernet uses
  // the Teensy's internal MAC address by default, so we can retrieve
  // it here
  uint8_t mac[6];
  Ethernet.macAddress(mac);  // This is informative; it retrieves, not sets
  printf("MAC = %02x:%02x:%02x:%02x:%02x:%02x\r\n",
         mac[0], mac[1], mac[2], mac[3], mac[4], mac[5]);

  // Add listeners
  // It's important to add these before doing anything with Ethernet
  // so no events are missed.

  // Listen for link changes
  Ethernet.onLinkState([](bool state) {
    printf("[Ethernet] Link %s\r\n", state ? "ON" : "OFF");

    // When setting a static IP, the address will be set but a link
    // might not yet exist
    bool hasIP = (Ethernet.localIP() != INADDR_NONE);
    setNetworkReady(hasIP, state, Ethernet.interfaceStatus());
  });

  // Listen for address changes
  Ethernet.onAddressChanged([]() {
    IPAddress ip = Ethernet.localIP();
    bool hasIP = (ip != INADDR_NONE);
    if (hasIP) {
      IPAddress subnet = Ethernet.subnetMask();
      IPAddress gw = Ethernet.gatewayIP();
      IPAddress dns = Ethernet.dnsServerIP();
      // Note: In order for the DNS address to not be INADDR_NONE
      //       (zero) when setting a static IP, it must be set first

      printf(
          "[Ethernet] Address changed:\r\n"
          "    Local IP = %u.%u.%u.%u\r\n"
          "    Subnet   = %u.%u.%u.%u\r\n"
          "    Gateway  = %u.%u.%u.%u\r\n"
          "    DNS      = %u.%u.%u.%u\r\n",
          ip[0], ip[1], ip[2], ip[3],
          subnet[0], subnet[1], subnet[2], subnet[3],
          gw[0], gw[1], gw[2], gw[3],
          dns[0], dns[1], dns[2], dns[3]);
    } else {
      printf("[Ethernet] Address changed: No IP address\r\n");
    }

    // Tell interested parties the state of the IP address and system
    // readiness, for example, servers, SNTP clients, and other
    // sub-programs that need to know whether to stop/start/restart/etc
    // Note: When setting a static IP, the address will be set but a
    //       link might not yet exist
    setNetworkReady(hasIP, Ethernet.linkState(), Ethernet.interfaceStatus());
  });

  // Listen for network interface status changes
  Ethernet.onInterfaceStatus([](bool status) {
    // When setting a static IP, the address will be set but the
    // network interface might not yet be up
    bool hasIP = (Ethernet.localIP() != INADDR_NONE);
    setNetworkReady(hasIP, Ethernet.linkState(), Ethernet.interfaceStatus());
  });

  bool startWithStatic = false;

  // This code shows a few ways you might want to set an IP address.
  // **It's just for illustration. Your program doesn't need to
  // include everything here.**
  if (kStartWithDHCP) {
    // Option 1 - Always start with DHCP

    printf("Starting Ethernet with DHCP...\r\n");
    if (Ethernet.begin()) {
      if (kWaitForDHCP) {
        // Option 1.1 - Wait for a DHCP-assigned address

        if (!Ethernet.waitForLocalIP(kDHCPTimeout)) {
          printf("No address from DHCP; setting static IP...\r\n");
          startWithStatic = true;
        }
      } else {
        // Option 1.2 - Don't wait for DHCP
      }
    } else {
      printf("Error: DHCP not started\r\n");
      startWithStatic = true;
    }

    if (startWithStatic && staticIP == INADDR_NONE) {
      printf("Error: No static IP\r\n");
      return;
    }
  } else {
    // Option 2 - staticIP determines

    if (staticIP == INADDR_NONE) {
      printf("Starting Ethernet with DHCP...\r\n");
      if (!Ethernet.begin()) {
        printf("Error: DHCP not started\r\n");
        return;
      }

      if (kWaitForDHCP) {
        if (!Ethernet.waitForLocalIP(kDHCPTimeout)) {
          printf("Warning: No address from DHCP\r\n");
          // An address could still come in later
        }
      }
    } else {
      startWithStatic = true;
    }
  }

  // At this point, a static IP is set to a valid value
  if (startWithStatic) {
    printf("Starting Ethernet with static IP...\r\n");
    Ethernet.setDNSServerIP(dnsServer);  // Set first so that the
                                         // listener sees it
    Ethernet.begin(staticIP, subnetMask, gateway);

    // When setting a static IP, the address is changed immediately,
    // but the link may not be up; optionally wait for the link here
    if (kLinkTimeout > 0) {
      if (!Ethernet.waitForLink(kLinkTimeout)) {
        printf("Warning: No link detected\r\n");
        // We may still see a link later, after the timeout, so
        // continue instead of returning
      }
    }
  }

  // *** Additional setup code goes here
}

// This is called when the network readiness has changed. The network
// is considered ready if there's an IP address and the link and network
// interface are up.
void setNetworkReady(bool hasIP, bool hasLink, bool interfaceUp) {
  networkReadyLatch = hasIP && hasLink && interfaceUp;

  printf("Network is%s READY\r\n", networkReadyLatch ? "" : " NOT");

  // To successfully perform network startup tasks, test the latch
  // somewhere in the main loop, and, if it is true, perform any
  // network tasks and then set the latch to false. No network calls
  // should be done from inside a listener.

  // Similar logic could be applied for when the network is not ready.

  // Servers technically only need the address state because they can
  // be brought up and active even when there's no link or no active
  // network interface, unlike clients and connections, which require
  // all of an address, link, and active network interface.
}

// Main program loop.
void loop() {
  // *** Main program code goes here

  // Perform any network startup:
  if (networkReadyLatch) {
    // *** Do any network startup tasks that must run when the network
    // *** comes up
    networkReadyLatch = false;
  }
}
