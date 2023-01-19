// SPDX-FileCopyrightText: (c) 2022-2023 Shawn Silverman <shawn@pobox.com>
// SPDX-License-Identifier: MIT

// OctoWS2811Receiver.h defines a Receiver implementation that uses an
// OctoWS2811 for the LEDs.
//
// This file is part of the QNEthernet library.

#ifndef OCTOWS2811RECEIVER_H_
#define OCTOWS2811RECEIVER_H_

// C++ includes
#include <cstdint>
#include <memory>

#include <OctoWS2811.h>

#include "PixelPusherServer.h"
#include "Receiver.h"

class OctoWS2811Receiver : public Receiver {
 public:
  // Creates a new object. The parameters are restricted:
  // `numStrips`: 0-255
  // `pixelsPerStrip`: >= 0
  OctoWS2811Receiver(PixelPusherServer &pp, int numStrips, int pixelsPerStrip);
  ~OctoWS2811Receiver() override = default;

  // Initializes the receiver. This performs tasks that can't be done
  // upon program creation. For example, OctoWS2811::begin() can't be
  // called until setup() or the program will crash.
  //
  // This will always return true.
  bool begin() override;

  void end() override {}

  int numStrips() const override {
    return numStrips_;
  };

  int pixelsPerStrip() const override {
    return pixelsPerStrip_;
  }

  uint8_t stripFlags(int stripNum) const override;
  void handleCommand(int command, const uint8_t *data, int len) override;
  void startPixels(bool complete) override {}
  void pixels(int stripNum, const uint8_t *pixels, int pixelsPerStrip) override;
  void endPixels() override;
  void loop() override {}

 private:
  PixelPusherServer &pp_;
  const int numStrips_;
  const int pixelsPerStrip_;

  std::unique_ptr<int[]> displayMem_;
  std::unique_ptr<int[]> drawingMem_;
  OctoWS2811 leds_;

  uint16_t globalBri_ = UINT16_MAX;
};

#endif  // OCTOWS2811RECEIVER_H_
