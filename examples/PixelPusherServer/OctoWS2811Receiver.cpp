// SPDX-FileCopyrightText: (c) 2022-2023 Shawn Silverman <shawn@pobox.com>
// SPDX-License-Identifier: MIT

// OctoWS2811Receiver.cpp implements OctoWS2811Receiver.
//
// This file is part of the QNEthernet library.

#include "OctoWS2811Receiver.h"

// C++ includes
#include <algorithm>
#include <cstring>

OctoWS2811Receiver::OctoWS2811Receiver(PixelPusherServer &pp, int numStrips,
                                       int pixelsPerStrip)
    : pp_(pp),
      numStrips_(std::min(std::max(0, numStrips), UINT8_MAX)),
      pixelsPerStrip_(std::max(0, pixelsPerStrip)),
      displayMem_{std::make_unique<int[]>(pixelsPerStrip_ * 6)},
      drawingMem_{std::make_unique<int[]>(pixelsPerStrip_ * 6)},
      leds_{static_cast<uint32_t>(pixelsPerStrip_),
            displayMem_.get(), drawingMem_.get(),
            WS2811_GRB | WS2811_800kHz,
            static_cast<uint8_t>(numStrips_)} {}

bool OctoWS2811Receiver::begin() {
  leds_.begin();

  // Ensure everything is cleared to black
  int numPixels = leds_.numPixels();
  for (int i = 0; i < numPixels; i++) {
    leds_.setPixel(i, 0);
  }
  leds_.show();

  return true;
}

uint8_t OctoWS2811Receiver::stripFlags(int stripNum) const {
  return 0;
}

void OctoWS2811Receiver::handleCommand(int command,
                                       const uint8_t *data, int len) {
  switch (command) {
    case PixelPusherServer::Commands::GLOBALBRIGHTNESS_SET:
      if (len >= 2) {
        std::memcpy(&globalBri_, data, 2);
      }
      break;
    case PixelPusherServer::Commands::LED_CONFIGURE:
      // uint32_t num_strips
      // uint32_t strip_length
      // uint8_t strip_type[8]
      // uint8_t colour_order[8]
      // uint16_t group
      // uint16_t controller
      // uint16_t artnet_universe
      // uint16_t artnet_channel
      if (len >= 32) {
        uint16_t n;
        std::memcpy(&n, &data[24], 2);
        pp_.setGroupNum(n);
        std::memcpy(&n, &data[26], 2);
        pp_.setControllerNum(n);
      }
      break;
    default:
      // Unknown command
      break;
  }
}

// https://github.com/FastLED/FastLED/blob/4d73cddfe4bd2b370ee882b6f68769bf7f8309f4/src/lib8tion/scale8.h#L20
static inline uint8_t scale8(uint8_t b, uint8_t scale) {
  return (uint16_t{b} * (1 + uint16_t{scale})) >> 8;
}

void OctoWS2811Receiver::pixels(int stripNum, const uint8_t *pixels,
                                int pixelsPerStrip) {
  if (stripNum < 0 || numStrips_ <= stripNum) {
    return;
  }

  int it = stripNum * pixelsPerStrip;
  int end = it + pixelsPerStrip;
  uint8_t bri = globalBri_ >> 8;
  if (bri == 0xff) {
    while (it != end) {
      leds_.setPixel(it, pixels[0], pixels[1], pixels[2]);
      pixels += 3;
      it++;
    }
  } else {
    while (it != end) {
      leds_.setPixel(it,
                     scale8(pixels[0], bri),
                     scale8(pixels[1], bri),
                     scale8(pixels[2], bri));
      pixels += 3;
      it++;
    }
  }
}

void OctoWS2811Receiver::endPixels() {
  leds_.show();
}
