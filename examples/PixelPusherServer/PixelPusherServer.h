// SPDX-FileCopyrightText: (c) 2022-2023 Shawn Silverman <shawn@pobox.com>
// SPDX-License-Identifier: MIT

// PixelPusherServer.h is the include file for the PixelPusher server.
//
// Useful links that helped decipher the protocol:
// * https://github.com/hzeller/pixelpusher-server
// * https://github.com/robot-head/PixelPusher-java
//
// This file is part of the QNEthernet library.

#ifndef PIXELPUSHERSERVER_H_
#define PIXELPUSHERSERVER_H_

// C++ includes
#include <cstdint>
#include <memory>

#include <QNEthernet.h>
#include <elapsedMillis.h>

#include "CircularBuffer.h"
#include "Receiver.h"

using namespace qindesign::network;

class PixelPusherServer final {
 public:
  enum StripFlags : uint8_t {
    RGBOW         = (1 << 0),
    WIDEPIXELS    = (1 << 1),
    LOGARITHMIC   = (1 << 2),
    MOTION        = (1 << 3),
    NOTIDEMPOTENT = (1 << 4),
    BRIGHTNESS    = (1 << 5),
    MONOCHROME    = (1 << 6),
  };

  // Not a class so we can use the values directly
  enum PusherFlags : uint32_t {
    PROTECTED             = (1 << 0),
    FIXEDSIZE             = (1 << 1),
    GLOBALBRIGHTNESS      = (1 << 2),
    STRIPBRIGHTNESS       = (1 << 3),
    MONOCHROME_NOT_PACKED = (1 << 4),
  };

  enum Commands : uint8_t {
    RESET                = 0x01,
    GLOBALBRIGHTNESS_SET = 0x02,
    WIFI_CONFIGURE       = 0x03,
    LED_CONFIGURE        = 0x04,
    STRIPBRIGHTNESS_SET  = 0x05,
  };

  enum ColourOrders : uint8_t {
    RGB = 0,
    RBG = 1,
    GBR = 2,
    GRB = 3,
    BGR = 4,
    BRG = 5,
  };

  enum StripTypes : uint8_t {
    LPD8806 = 0,
    WS2801 = 1,
    WS2811 = 2,
    APA102 = 3,
  };

  // The default port on which to receive pixel data.
  static constexpr uint16_t kDefaultPixelsPort = 5078;//9897;

  PixelPusherServer() = default;
  ~PixelPusherServer();

  // Initializes the server and starts listening for pixel data on the
  // specified port. This uses the current Ethernet information. This
  // should be called whenever the Ethernet information changes.
  //
  // This will return false if there was a problem starting the UDP
  // listening socket.
  //
  // This does not call recv->begin().
  //
  // See: kDefaultPixelsPort
  bool begin(Receiver *recv, uint16_t port,
             int controllerNum, int groupNum,
             uint16_t vendorId, uint16_t productId,
             uint16_t hwRevision,
             uint32_t flags);

  // Stops listening for pixels.
  //
  // This does not call Receiver::end().
  void end();

  // Call this repeatedly.
  void loop();

  // Returns the port on which this listens for pixel data.
  uint16_t pixelsPort();

  void setControllerNum(int n);
  void setGroupNum(int n);

  // Tests if the server has been started.
  operator bool() const {
    return started_;
  }

 private:
  enum DeviceTypes : uint8_t {
    ETHERDREAM = 0,
    LUMIABRIDGE = 1,
    PIXELPUSHER = 2,
  };

  struct DeviceData {
    uint8_t macAddr[6];
    uint8_t ipAddr[4];
    uint8_t deviceType;
    uint8_t protocolVersion;  // For the device, not the discovery
    uint16_t vendorId;
    uint16_t productId;
    uint16_t hwRevision;
    uint16_t swRevision;
    uint32_t linkSpeed;  // In bits per second
  } __attribute__((__packed__));

  struct PixelPusherData1 {
    uint8_t stripsAttached;
    uint8_t maxStripsPerPacket;
    uint16_t pixelsPerStrip;    // uint16_t used to make alignment work
    uint32_t updatePeriod;      // In microseconds
    uint32_t powerTotal;        // In PWM units
    uint32_t deltaSequence;     // Difference between received and expected
                                // sequence numbers
    int32_t controllerOrdinal;  // Configured order number for this controller
    int32_t groupOrdinal;       // Configured group number for this controller

    uint16_t artnetUniverse;  // Index 24
    uint16_t artnetChannel;
    uint16_t myPort;  // Index 28

    // [strip flags, one per strip, at least 8], Index 32
  } __attribute__((__packed__));

  struct PixelPusherData2 {
    uint32_t pusherFlags;  // Flags for the whole pusher
    uint32_t segments;     // Number of segments in each strip
    uint32_t powerDomain;  // Power domain of this pusher
    uint8_t lastDrivenIp[4];
    uint16_t lastDrivenPort;
  } __attribute__((__packed__));

  class NullReceiver final : public Receiver {
    bool begin() override { return true; }
    void end() override {}
    int numStrips() const override { return 0; }
    int pixelsPerStrip() const override { return 0; }
    uint8_t stripFlags(int stripNum) const override { return 0; }

    void startPixels(bool complete) override {}
    void pixels(int stripNum, const uint8_t *pixels,
                int pixelsPerStrip) override {}
    void endPixels() override {}
    void loop() override {}
  };

  static constexpr uint32_t kDiscoveryPeriod = 1'000;

  static constexpr uint16_t kSoftwareRevision = 142;

  static constexpr uint16_t kDiscoveryPort = 7331;

  void sendDiscovery();

  static NullReceiver nullReceiver_;

  bool started_;

  // UDP sockets
  EthernetUDP discoveryUDP_;  // Send
  EthernetUDP pixelsUDP_;     // Receive

  // Data receiver.
  Receiver *recv_ = &nullReceiver_;

  // Useful cached values
  IPAddress broadcastIP_;
  size_t stripSize_ = 0;  // Strip size in bytes

  // Computed packet data
  elapsedMillis discoveryTimer_;
  uint32_t lastSeq_ = 0;
  std::unique_ptr<CircularBuffer<uint32_t>> lastUpdateTimes_;
      // Used for averaging the last set of update times
  float avUpdateTime_;

  // Packet data
  DeviceData deviceData_;
  size_t stripFlagsSize_ = 8;
  std::unique_ptr<uint8_t[]> stripFlags_{std::make_unique<uint8_t[]>(8)};
  PixelPusherData1 ppData1_;
  PixelPusherData2 ppData2_;
};

#endif  // PIXELPUSHERSERVER_H_
