// SPDX-FileCopyrightText: (c) 2026 Shawn Silverman <shawn@pobox.com>
// SPDX-License-Identifier: AGPL-3.0-or-later

// ChronoClocks demonstrates how to use the std::chrono library with
// the qindesign::network::util Clocks. This is not intended to be a
// std::chrono tutorial.
//
// This file is part of the QNEthernet library.

// C++ includes
#include <chrono>
#include <cinttypes>
#include <cstdint>
#include <cstdio>

#include <QNEthernet.h>
#include <qnethernet/util/chrono_clocks.h>

// On C++ >= 14:
// // Allows us to specify things like 1000ms
// using namespace std::chrono_literals;

// Make it easier to type
using steady_clock = qindesign::network::util::steady_clock_ms;
using high_resolution_clock = qindesign::network::util::arm_high_resolution_clock;

// --------------------------------------------------------------------------
//  Program State
// --------------------------------------------------------------------------

namespace {  // Internal linkage section

// Loop counter
unsigned long s_loopCounter = 0;

}  // namespace

// --------------------------------------------------------------------------
//  Main Program
// --------------------------------------------------------------------------

// Program setup.
void setup() {
  Serial.begin(115200);
  while (!Serial && (steady_clock::now().time_since_epoch() <
                     std::chrono::milliseconds(4000))) {
    // Wait for Serial
  }

  const auto hr = std::chrono::duration_cast<std::chrono::nanoseconds>(
      high_resolution_clock::now().time_since_epoch());
  printf("highres_now=%" PRId64 " ns\r\n"
         "highres wraparound period = %g s\r\n"
         "\r\n",
         hr.count(),
         high_resolution_clock::wraparoundPeriod());
}

// Main program loop.
void loop() {
  ++s_loopCounter;  // Want 1-based prints and this starts at zero

  // Capture the start time
  const auto start = steady_clock::now();

  // Print some durations, the time
  const auto now = steady_clock::now();
  const auto dur_ms = now.time_since_epoch();
  const auto dur_s =
      std::chrono::duration_cast<std::chrono::duration<float>>(dur_ms);
  printf("%lu: Loop work: ms=%" PRId64 " s=%g\r\n",
         s_loopCounter, dur_ms.count(), dur_s.count());

  // The milliseconds value (count) could be cast to a uint32_t to get
  // the same value as millis() would return

  // Repeat every second
  delay(static_cast<uint32_t>(std::chrono::milliseconds(1000).count()));

  // Capture the end time
  const auto end = steady_clock::now();

  // Print timing info
  printf("%lu: Loop time = %" PRId64 " ms\r\n",
         s_loopCounter, (end - start).count());
}
