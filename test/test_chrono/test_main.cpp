// SPDX-FileCopyrightText: (c) 2026 Shawn Silverman <shawn@pobox.com>
// SPDX-License-Identifier: AGPL-3.0-or-later

// test_main.cpp tests the chrono functions.
// This file is part of the QNEthernet library.

// C++ includes
#include <chrono>
#include <cstddef>

#include <Arduino.h>
#include <unity.h>

#include "qnethernet/chrono/chrono_clocks.h"
#include "qnethernet/chrono/elapsedTime.h"
#include "qnethernet/compat/c++11_compat.h"

using namespace std::chrono_literals;

using steady_clock = qindesign::chrono::steady_clock_ms;
using elapsedTime = qindesign::chrono::elapsedTime<steady_clock>;

// Pre-test setup. This is run before every test.
void setUp() {
}

// Post-test teardown. This is run after every test.
void tearDown() {
}

// Tests the elapsedTime<Clock> utility class comparison functions.
static void test_elapsedTime_compare() {
  elapsedTime timer;

  delay(2000);
  auto x = std::chrono::duration_cast<std::chrono::seconds>(timer.dur());
  TEST_ASSERT_EQUAL(2, x.count());

  // Comparison operators

  TEST_ASSERT_TRUE(timer == 2s);
  TEST_ASSERT_TRUE(timer == 2000);
  TEST_ASSERT_TRUE(2s == timer);
  TEST_ASSERT_TRUE(2000 == timer);
  TEST_ASSERT_TRUE(timer != 1s);
  TEST_ASSERT_TRUE(timer != 1000);
  TEST_ASSERT_TRUE(1s != timer);
  TEST_ASSERT_TRUE(1000 != timer);
  TEST_ASSERT_TRUE(timer < 3s);
  TEST_ASSERT_TRUE(timer < 3000);
  TEST_ASSERT_TRUE(3s > timer);
  TEST_ASSERT_TRUE(3000 > timer);
  TEST_ASSERT_TRUE(timer > 1s);
  TEST_ASSERT_TRUE(timer > 1000);
  TEST_ASSERT_TRUE(1s < timer);
  TEST_ASSERT_TRUE(1000 < timer);
  TEST_ASSERT_TRUE(timer <= 3s);
  TEST_ASSERT_TRUE(timer <= 3000);
  TEST_ASSERT_TRUE(3s >= timer);
  TEST_ASSERT_TRUE(3000 >= timer);
  TEST_ASSERT_TRUE(timer >= 2s);
  TEST_ASSERT_TRUE(timer >= 2000);
  TEST_ASSERT_TRUE(2s <= timer);
  TEST_ASSERT_TRUE(2000 <= timer);

  TEST_ASSERT_TRUE(timer == 2000ms);
  TEST_ASSERT_TRUE(2000ms == timer);
  TEST_ASSERT_TRUE(timer != 1000ms);
  TEST_ASSERT_TRUE(1000ms != timer);
  TEST_ASSERT_TRUE(timer < 3000ms);
  TEST_ASSERT_TRUE(3000ms > timer);
  TEST_ASSERT_TRUE(timer > 1000ms);
  TEST_ASSERT_TRUE(1000ms < timer);
  TEST_ASSERT_TRUE(timer <= 3000ms);
  TEST_ASSERT_TRUE(3000ms >= timer);
  TEST_ASSERT_TRUE(timer >= 2000ms);
  TEST_ASSERT_TRUE(2000ms <= timer);

  TEST_ASSERT_TRUE(timer == 2.0s);
  TEST_ASSERT_TRUE(2.0s == timer);
  TEST_ASSERT_TRUE(timer != 1.9s);
  TEST_ASSERT_TRUE(1.9s != timer);
  TEST_ASSERT_TRUE(timer < 3.0s);
  TEST_ASSERT_TRUE(3.0s > timer);
  TEST_ASSERT_TRUE(timer > 1.9s);
  TEST_ASSERT_TRUE(1.9s < timer);
  TEST_ASSERT_TRUE(timer <= 3.0s);
  TEST_ASSERT_TRUE(3.0s >= timer);
  TEST_ASSERT_TRUE(timer >= 0.5s);
  TEST_ASSERT_TRUE(0.5s <= timer);
}

// Tests "other" things about the elapsedTime<Clock> utility class.
static void test_elapsedTime_other() {
  // Constructor (duration), assignment, and arithmetic
  elapsedTime t1{2s};
  TEST_ASSERT_TRUE(t1 == 2s);
  t1 = 3s;
  TEST_ASSERT_TRUE(t1 == 3s);
  t1 += 500ms;
  TEST_ASSERT_TRUE(t1 == 3.5s);
  t1 -= 0.2s;
  TEST_ASSERT_TRUE(t1 == 3.3s);
  TEST_ASSERT_TRUE((t1 + 0.6s) == 3.9s);
  TEST_ASSERT_TRUE((t1 - 0.6s) == 2.7s);

  // Constructor (rep), assignment, and arithmetic
  elapsedTime t2{2000};
  TEST_ASSERT_TRUE(t2 == 2s);
  t2 = 3000;
  TEST_ASSERT_TRUE(t2 == 3s);
  t2 += 500;
  TEST_ASSERT_TRUE(t2 == 3.5s);
  t2 -= 200;
  TEST_ASSERT_TRUE(t2 == 3.3s);
  TEST_ASSERT_TRUE((t2 + 600) == 3.9s);
  TEST_ASSERT_TRUE((t2 - 600) == 2.7s);
}

// Main program setup.
void setup() {
  Serial.begin(115200);
  while (!Serial && (millis() < 4000)) {
    // Wait for Serial
  }

  // NOTE!!! Wait for >2 secs
  // if board doesn't support software reset via Serial.DTR/RTS
  delay(2000);

#if defined(TEENSYDUINO)
  if (CrashReport) {
    (void)Serial.println(CrashReport);
  }
#endif  // defined(TEENSYDUINO)

  UNITY_BEGIN();
  RUN_TEST(test_elapsedTime_compare);
  RUN_TEST(test_elapsedTime_other);
  UNITY_END();
}

// Main program loop.
void loop() {
}
