// SPDX-FileCopyrightText: (c) 2022-2024 Shawn Silverman <shawn@pobox.com>
// SPDX-License-Identifier: AGPL-3.0-or-later

// OctoWS2811Receiver.cpp implements OctoWS2811Receiver.
//
// This file is part of the QNEthernet library.

#include "OctoWS2811Receiver.h"

// C++ includes
#include <algorithm>
#include <cstring>

// Pixel configuration
static constexpr size_t kBytesPerPixel = 3;
static constexpr uint8_t kConfig = WS2811_RGB | WS2811_800kHz;
    // Use RGB because the actual ordering is captured in the strip config

// Define this object.
const OctoWS2811Receiver::StripConfig OctoWS2811Receiver::kDefaultStripConfig;

OctoWS2811Receiver::OctoWS2811Receiver(PixelPusherServer &pp, size_t numStrips,
                                       size_t pixelsPerStrip)
    : pp_(pp),
      numStrips_(std::min(numStrips, size_t{UINT8_MAX})),
      pixelsPerStrip_(pixelsPerStrip),
      displayMem_{std::make_unique<uint8_t[]>(numStrips * pixelsPerStrip *
                                              kBytesPerPixel)},
      drawingMem_{std::make_unique<uint8_t[]>(numStrips * pixelsPerStrip *
                                              kBytesPerPixel)},
      leds_{static_cast<uint32_t>(pixelsPerStrip_),
            displayMem_.get(), drawingMem_.get(),
            kConfig, static_cast<uint8_t>(numStrips_)} {}

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

const OctoWS2811Receiver::StripConfig &OctoWS2811Receiver::getStripConfig(
    size_t stripNum) const {
  return (stripNum < stripConfigs_.size())
             ? stripConfigs_[stripNum]
             : OctoWS2811Receiver::kDefaultStripConfig;
}

uint8_t OctoWS2811Receiver::stripFlags(size_t stripNum) const {
  return getStripConfig(stripNum).flags;
}

void OctoWS2811Receiver::handleCommand(uint8_t command,
                                       const uint8_t *data, size_t len) {
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
        // Colour order
        for (size_t i = 0; i < 8; i++) {
          auto &config = stripConfigs_[i];
          switch (data[16 + i]) {
            case PixelPusherServer::ColourOrders::RGB:
              config.rgbOrder[0] = 0;
              config.rgbOrder[1] = 1;
              config.rgbOrder[2] = 2;
              break;
            case PixelPusherServer::ColourOrders::RBG:
              config.rgbOrder[0] = 0;
              config.rgbOrder[1] = 2;
              config.rgbOrder[2] = 1;
              break;
            case PixelPusherServer::ColourOrders::GBR:
              config.rgbOrder[0] = 1;
              config.rgbOrder[1] = 2;
              config.rgbOrder[2] = 0;
              break;
            case PixelPusherServer::ColourOrders::GRB:
              config.rgbOrder[0] = 1;
              config.rgbOrder[1] = 0;
              config.rgbOrder[2] = 2;
              break;
            case PixelPusherServer::ColourOrders::BGR:
              config.rgbOrder[0] = 2;
              config.rgbOrder[1] = 1;
              config.rgbOrder[2] = 0;
              break;
            case PixelPusherServer::ColourOrders::BRG:
              config.rgbOrder[0] = 2;
              config.rgbOrder[1] = 0;
              config.rgbOrder[2] = 1;
              break;
          }
        }

        uint16_t n;
        std::memcpy(&n, &data[24], 2);
        pp_.setGroupNum(n);
        std::memcpy(&n, &data[26], 2);
        pp_.setControllerNum(n);
      }
      break;

    case PixelPusherServer::Commands::STRIPBRIGHTNESS_SET:
      if (len >= 3) {
        if (data[0] < 8) {  // Strip number
          std::memcpy(&stripConfigs_[data[0]].brightness, &data[1], 2);
        }
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

// Scales a 16-bit value by another.
static inline uint16_t scale16(uint16_t b, uint16_t scale) {
  return (uint32_t{b} * (1 + uint32_t{scale})) >> 16;
}

void OctoWS2811Receiver::pixels(size_t stripNum, const uint8_t *pixels,
                                size_t pixelsPerStrip) {
  if (stripNum < 0 || numStrips_ <= stripNum) {
    return;
  }

  size_t it = stripNum * pixelsPerStrip;
  size_t end = it + pixelsPerStrip;

  const auto &config = getStripConfig(stripNum);
  const uint8_t bri = scale16(config.brightness, globalBri_) >> 8;
  const auto &rgbOrder = config.rgbOrder;

  if (bri == UINT8_MAX) {
    while (it != end) {
      leds_.setPixel(it,
                     pixels[rgbOrder[0]],
                     pixels[rgbOrder[1]],
                     pixels[rgbOrder[2]]);
      pixels += 3;
      it++;
    }
  } else {
    while (it != end) {
      leds_.setPixel(it,
                     scale8(pixels[rgbOrder[0]], bri),
                     scale8(pixels[rgbOrder[1]], bri),
                     scale8(pixels[rgbOrder[2]], bri));
      pixels += 3;
      it++;
    }
  }
}

void OctoWS2811Receiver::endPixels() {
  leds_.show();
}
