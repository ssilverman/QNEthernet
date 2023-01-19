// SPDX-FileCopyrightText: (c) 2022-2023 Shawn Silverman <shawn@pobox.com>
// SPDX-License-Identifier: MIT

// Receiver.h defines an interface for handling PixelPusher commands
// and pixel data.
//
// This file is part of the QNEthernet library.

#ifndef RECEIVER_H_
#define RECEIVER_H_

// C++ includes
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

  // Returns the total number of strips.
  virtual int numStrips() const = 0;

  // Returns the number of pixels per strip.
  virtual int pixelsPerStrip() const = 0;

  // Gets the strip flags for the given strip number.
  virtual uint8_t stripFlags(int stripNum) const = 0;

  // Handles a PixelPusher command.
  virtual void handleCommand(int command, const uint8_t *data, int len) {}

  // Starts receiving pixels from a packet. The `complete` parameter
  // indicates whether the complete set of pixels will be included in
  // the pixel data.
  virtual void startPixels(bool complete) = 0;

  // Processes pixels for one strip.
  virtual void pixels(int stripNum, const uint8_t *pixels,
                      int pixelsPerStrip) = 0;

  // All the pixels from the packet have been sent to this receiver.
  virtual void endPixels() = 0;

  // Executes periodically whenever PixelPusherServer::loop()
  // is called.
  virtual void loop() = 0;
};

#endif  // RECEIVER_H_
