// SPDX-FileCopyrightText: (c) 2026 Shawn Silverman <shawn@pobox.com>
// SPDX-License-Identifier: AGPL-3.0-or-later

// ElapsedTime demonstrates how to use the elapsedTime utility. This
// toggles the LED every 1/2 second.
//
// This file is part of the QNEthernet library.

#include <chrono>

#include <QNEthernet.h>
#include <qnethernet/util/chrono_clocks.h>
#include <qnethernet/util/elapsedTime.h>

namespace {  // Internal linkage section

// Make these easier to type
using steady_clock = qindesign::network::util::steady_clock_ms;
using elapsedTime = qindesign::network::util::elapsedTime<steady_clock>;

// This check can be removed and the code simplified if it is known
// that your system supports C++14
#if __cplusplus >= 201402L
// Allows us to specify things like 1000ms
using namespace std::chrono_literals;

constexpr auto kInterval = 500ms;  // 0.5s is valid too
#else
constexpr auto kInterval = std::chrono::milliseconds(500);
#endif  // C++ >= 14

elapsedTime timer;

}  // namespace

// Note: Could use digitalWriteFast() and digitalToggleFast() where supported

// Main program setup.
void setup() {
  pinMode(LED_BUILTIN, OUTPUT);
  digitalWrite(LED_BUILTIN, HIGH);
}

// Main program loop.
void loop() {
  if (timer >= kInterval) {
    // Some platforms support digitalToggle()
    digitalWrite(LED_BUILTIN, (digitalRead(LED_BUILTIN) == LOW) ? HIGH : LOW);
    timer.reset();  // 'timer = 0' or 'timer = 0s', etc. would also work
  }
}
