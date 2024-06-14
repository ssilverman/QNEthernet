// SPDX-FileCopyrightText: (c) 2022-2024 Shawn Silverman <shawn@pobox.com>
// SPDX-License-Identifier: AGPL-3.0-or-later

// Receiver.h defines an interface for handling PixelPusher commands
// and pixel data.
//
// This file is part of the QNEthernet library.

#pragma once

// C++ includes
#include <cstddef>
#include <cstdint>

class Receiver {
 public:
  Receiver() = default;
  virtual ~Receiver() = default;

  // Avoid slicing
  Receiver(const Receiver &) = delete;
  Receiver &operator=(const Receiver &) = delete;

  // Initializes the receiver. This performs tasks that must be done
  // after the system is booted and is meant to be called from setup()
  // or later. This returns whether the call was successful.
  virtual bool begin() = 0;

  // Stops the receiver. For some receivers, this may be a no-op.
  virtual void end() = 0;

  // Returns the total number of strips. This will be limited to 255.
  virtual size_t numStrips() const = 0;

  // Returns the number of pixels per strip.
  virtual size_t pixelsPerStrip() const = 0;

  // Gets the strip flags for the given strip number.
  virtual uint8_t stripFlags(size_t stripNum) const = 0;

  // Handles a PixelPusher command.
  virtual void handleCommand(uint8_t command, const uint8_t *data, size_t len) {
  }

  // Starts receiving pixels from a packet.
  virtual void startPixels() {}

  // Processes pixels for one strip.
  virtual void pixels(size_t stripNum, const uint8_t *pixels,
                      size_t pixelsPerStrip) = 0;

  // All the pixels from the packet have been sent to this receiver.
  virtual void endPixels() = 0;

  // Executes periodically whenever PixelPusherServer::loop()
  // is called.
  virtual void loop() {}
};
