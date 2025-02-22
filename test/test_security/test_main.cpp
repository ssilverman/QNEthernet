// SPDX-FileCopyrightText: (c) 2025 Shawn Silverman <shawn@pobox.com>
// SPDX-License-Identifier: AGPL-3.0-or-later

// test_main.cpp tests security functions.
// This file is part of the QNEthernet library.

#include <cstdint>
#include <cstdio>
#include <cstring>
#include <vector>

#include <Arduino.h>
#include <qnethernet/security/siphash.h>
#include <unity.h>

#include "siphash_vectors.h"

// --------------------------------------------------------------------------
//  Utilities
// --------------------------------------------------------------------------

// Formats into a char vector and returns the vector. The vector will always be
// terminated with a NUL. This returns an empty string if something goes wrong
// with the print function.
template <typename... Args>
std::vector<char> format(const char *format, Args... args) {
  std::vector<char> out;

  int size = std::snprintf(nullptr, 0, format, args...) + 1;  // Include the NUL
  if (size <= 1) {
    out.resize(1, 0);
  } else {
    out.resize(size);
    std::snprintf(out.data(), size, format, args...);
  }
  return out;
}

// --------------------------------------------------------------------------
//  Main Program
// --------------------------------------------------------------------------

// Pre-test setup. This is run before every test.
void setUp() {
}

// Post-test teardown. This is run after every test.
void tearDown() {
}

// Tests the SipHash implementation.
static void test_siphash() {
  // Test from the paper
  constexpr uint8_t kKey[16]{
      0x00, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07,
      0x08, 0x09, 0x0a, 0x0b, 0x0c, 0x0d, 0x0e, 0x0f,
  };
  constexpr uint8_t kMsg[15]{
      0x00, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07,
      0x08, 0x09, 0x0a, 0x0b, 0x0c, 0x0d, 0x0e,
  };

  uint64_t want = uint64_t{0xa129ca6149be45e5};
  uint64_t got = siphash(2, 4, kKey, kMsg, 15);
  TEST_ASSERT_EQUAL(want, got);

  // Test vectors

  uint8_t msg[64];
  for (size_t i = 0; i < 64; i++) {
    msg[i] = i;
    uint64_t h = siphash(2, 4, kKey, msg, i);
    TEST_ASSERT_EQUAL_MESSAGE(h, vectors_sip64[i],
                              format("Expected match: i=%zu", i).data());
  }
}

// Main program setup.
void setup() {
  Serial.begin(115200);
  while (!Serial && millis() < 4000) {
    // Wait for Serial
  }

  // NOTE!!! Wait for >2 secs
  // if board doesn't support software reset via Serial.DTR/RTS
  delay(2000);

  if (CrashReport) {
    Serial.println(CrashReport);
  }

  UNITY_BEGIN();
  RUN_TEST(test_siphash);
  UNITY_END();
}

// Main program loop.
void loop() {
}
