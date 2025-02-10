// SPDX-FileCopyrightText: (c) 2024 Shawn Silverman <shawn@pobox.com>
// SPDX-License-Identifier: AGPL-3.0-or-later

// SimpleHTTPClient implements a very rudimentary HTTP client. It
// makes an HTTP request once upon acquiring an IP address and then
// reads until connection-close. It also demonstrates use of a state
// machine. Note that the response is not parsed.

#include <QNEthernet.h>

using namespace qindesign::network;

// -------------------------------------------------------------------
//  Configuration
// -------------------------------------------------------------------

constexpr char kHost[]{"www.google.com"};
constexpr uint16_t kPort = 80;

// -------------------------------------------------------------------
//  Program Variables
// -------------------------------------------------------------------

enum class States {
  kStart,
  kConnect,
  kSendRequest,
  kReadResponse,
  kEnd,
};

// Where we're at and what we're doing
States state = States::kStart;

// The network client
EthernetClient client;

// Program state
bool gotIP = false;  // Watch for when we get an IP
uint32_t responseStartTime = 0;
size_t responseSize = 0;

// Client read buffer
uint8_t readBuf[1024];

// -------------------------------------------------------------------
//  Main Program
// -------------------------------------------------------------------

// Forward declarations (not needed in the Arduino IDE)
static bool connect();
static void sendRequest();
static bool readResponse();

// Main program setup.
void setup() {
  Serial.begin(115200);
  while (!Serial && millis() < 4000) {
    // Wait for Serial
  }
  Serial.println("Starting...");

  // Watch for link changes
  Ethernet.onLinkState([](bool state) {
    if (state) {
      Serial.println("[Ethernet] Link ON");
    } else {
      Serial.println("[Ethernet] Link OFF");
    }
  });

  // Watch for address changes
  // It will take a little time to get an IP address, so watch for it
  Ethernet.onAddressChanged([]() {
    IPAddress ip = Ethernet.localIP();
    bool hasIP = (ip != INADDR_NONE);
    if (hasIP) {
      gotIP = true;
      Serial.printf("[Ethernet] Address changed: IP = %u.%u.%u.%u",
                    ip[0], ip[1], ip[2], ip[3]);
      Serial.println();
    } else {
      Serial.println("[Ethernet] Address changed: No IP");
    }
  });

  Serial.println();
  Serial.println("[Start]");
  Serial.println("Starting Ethernet with DHCP...");
  if (!Ethernet.begin()) {
    Serial.println("ERROR: Failed to start Ethernet");
    return;
  }
}

// Main program loop.
void loop() {
  switch (state) {
    case States::kStart:
      if (gotIP) {
        state = States::kConnect;
      }
      break;

    case States::kConnect:
      Serial.println();
      Serial.println("[Request]");
      if (connect()) {
        // For this demo, only send the request once
        state = States::kSendRequest;
      } else {
        state = States::kEnd;
      }
      break;

    case States::kSendRequest:
      sendRequest();

      Serial.println();
      Serial.println("[Response]");
      state = States::kReadResponse;

      // Initialize response state
      responseStartTime = micros();
      responseSize = 0;

      break;

    case States::kReadResponse:
      if (readResponse()) {
        state = States::kEnd;
      }
      break;

    case States::kEnd:
      break;
  }
}

// -------------------------------------------------------------------
//  Internal Functions
// -------------------------------------------------------------------

// Attempts to connect to the host and returns whether successful.
static bool connect() {
  Serial.printf("Connecting to %s:%" PRIu16 "...", kHost, kPort);
  Serial.println();

  // (This version waits; connectNoWait() does not wait, but would
  //  require another state)
  if (!client.connect(kHost, kPort)) {
    Serial.println("Could not connect to host");
    return false;
  }

  return true;
}

// Sends the HTTP request.
static void sendRequest() {
  Serial.println("Sending request...");
  client.writeFully(
      "GET / HTTP/1.1\r\n"
      "Host: ");
  client.writeFully(kHost);
  client.writeFully(
      "\r\n"
      "Connection: close\r\n"
      "\r\n");
  client.flush();
}

// Reads the response and returns whether complete.
static bool readResponse() {
  // Read what's available from the client and print it
  int avail = client.available();
  if (avail > 0) {
    int read = client.read(readBuf, sizeof(readBuf));
    if (read > 0) {
      responseSize += read;
      Serial.write(readBuf, read);
      Serial.flush();
    }
  }

  // Detect connection end and no data available
  if (!client.connected()) {
    uint32_t responseDuration = micros() - responseStartTime;
    Serial.println();
    Serial.printf("Response stats: %zu bytes in %g seconds",
                  responseSize, responseDuration / 1e6f);
    Serial.println();
    return true;
  }

  return false;
}
