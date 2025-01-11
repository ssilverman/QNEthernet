// SPDX-FileCopyrightText: (c) 2022-2025 Shawn Silverman <shawn@pobox.com>
// SPDX-License-Identifier: AGPL-3.0-or-later

// PixelPusherServer.cpp implements the PixelPusher server.
//
// This file is part of the QNEthernet library.

#include "PixelPusherServer.h"

// C++ includes
#include <algorithm>
#include <cstring>

#include <IPAddress.h>
#include <QNEthernet.h>

using namespace qindesign::network;

// This uses direct memory copies and the protocol is little-endian,
// so check that this is being compiled on a little-endian platform.
// The alternative would be to use bit shifting.
static_assert(__BYTE_ORDER__ == __ORDER_LITTLE_ENDIAN__, "Not little-endian");

static constexpr size_t kMaxUDPSize = UINT16_MAX - 8 - 20;
static constexpr size_t kMaxPixelsPerStrip = (kMaxUDPSize - 4 - 1)/3;

// Indicates that the packet is a PixelPusher command.
static constexpr uint8_t kCommandMagic[]{
    0x40, 0x09, 0x2d, 0xa6, 0x15, 0xa5, 0xdd, 0xe5,
    0x6a, 0x9d, 0x4d, 0x5a, 0xcf, 0x09, 0xaf, 0x50,
};

PixelPusherServer::NullReceiver PixelPusherServer::nullReceiver_;

PixelPusherServer::~PixelPusherServer() {
  end();
}

bool PixelPusherServer::begin(Receiver *recv, uint16_t port,
                              int controllerNum, int groupNum,
                              uint16_t vendorId, uint16_t productId,
                              uint16_t hwRevision,
                              uint32_t flags) {
  if (recv == nullptr) {
    recv = &nullReceiver_;
  }
  recv_ = recv;
  size_t numStrips = std::min(recv->numStrips(), size_t{UINT8_MAX});
  size_t pixelsPerStrip = std::min(recv->pixelsPerStrip(), kMaxPixelsPerStrip);

  broadcastIP_ = Ethernet.broadcastIP();
  stripSize_ = 1 + pixelsPerStrip*3;
  lastSeq_ = -1;
  stripFlagsSize_ = std::max(size_t{8}, numStrips);
  stripFlags_ = std::make_unique<uint8_t[]>(stripFlagsSize_);

  Ethernet.macAddress(deviceData_.macAddr);
  IPAddress localIP = Ethernet.localIP();
  deviceData_.ipAddr[0] = localIP[0];
  deviceData_.ipAddr[1] = localIP[1];
  deviceData_.ipAddr[2] = localIP[2];
  deviceData_.ipAddr[3] = localIP[3];
  deviceData_.deviceType = DeviceTypes::PIXELPUSHER;
  deviceData_.protocolVersion = 1;  // ?
  deviceData_.vendorId = vendorId;
  deviceData_.productId = productId;
  deviceData_.hwRevision = hwRevision;
  deviceData_.swRevision = kSoftwareRevision;
  deviceData_.linkSpeed = Ethernet.linkSpeed() * 1'000'000;

  ppData1_.stripsAttached = numStrips;
  ppData1_.maxStripsPerPacket =
      std::min((kMaxUDPSize - 4)/stripSize_, numStrips);
  ppData1_.pixelsPerStrip = pixelsPerStrip;
  ppData1_.updatePeriod = 100'000;  // Start at 100ms
  ppData1_.powerTotal = 0;
  ppData1_.deltaSequence = 0;
  ppData1_.controllerOrdinal = controllerNum;
  ppData1_.groupOrdinal = groupNum;
  ppData1_.artnetUniverse = 0;
  ppData1_.artnetChannel = 0;
  ppData1_.myPort = port;
  // 2-byte padding
  // Strip flags go here, at least 8
  // 2-byte padding
  ppData2_.pusherFlags = flags;
  ppData2_.segments = 0;  // ?
  ppData2_.powerDomain = 0;
  std::fill_n(ppData2_.lastDrivenIp, 4, 0);
  ppData2_.lastDrivenPort = 0;

  // Fill in the strip flags
  for (size_t i = 0; i < numStrips; i++) {
    stripFlags_[i] = recv->stripFlags(i);
  }

  // Prepare the circular buffer with capacity equal to the minimum
  // number of packets needed to accomplish one frame
  size_t packetsPerFrame = (numStrips + ppData1_.maxStripsPerPacket - 1) /
                           size_t{ppData1_.maxStripsPerPacket};
  lastUpdateTimes_ =
      std::make_unique<CircularBuffer<uint32_t>>(packetsPerFrame);
  printf("k=%zu\r\n", lastUpdateTimes_->capacity());

  // Prepare the frame strip tracker
  frameStrips_.resize(numStrips);
  frameStrips_.clear();

  if (started_) {
    end();
  }
  started_ = true;
  return pixelsUDP_.begin(port);
}

void PixelPusherServer::end() {
  if (!started_) {
    return;
  }
  started_ = false;
  pixelsUDP_.stop();
  lastUpdateTimes_ = nullptr;
}

uint16_t PixelPusherServer::pixelsPort() {
  return pixelsUDP_.localPort();
}

void PixelPusherServer::setControllerNum(int n) {
  ppData1_.controllerOrdinal = n;
}

void PixelPusherServer::setGroupNum(int n) {
  ppData1_.groupOrdinal = n;
}

// Checks if all values in a std::vector<bool> are a specific value.
static bool isAll(const std::vector<bool> &v, bool flag) {
  return (std::find(v.begin(), v.end(), !flag) == v.end());
}

void PixelPusherServer::loop() {
  if (!started_) {
    return;
  }

  // Send the discovery packet every once in a while
  if (discoveryTimer_ >= kDiscoveryPeriod) {
    sendDiscovery();
    discoveryTimer_ = 0;
  }

  recv_->loop();

  // Check for a valid packet
  int packetSize = pixelsUDP_.parsePacket();
  if (packetSize < 4) {
    return;
  }
  size_t size = packetSize;

  uint32_t startTime = micros();

  // Packet length == 4 + strips*(1 + width*3)
  // Allow extra strips in this packet, but ignore them, for robustness
  size_t stripsInPacket = (size - 4)/stripSize_;
  if (stripsInPacket*stripSize_ != size - 4) {
    return;
  }

  const uint8_t *data = pixelsUDP_.data();

  uint32_t seq;
  std::memcpy(&seq, data, 4);
  data += 4;
  size -= 4;

  // Possibly a PP command
  if (size >= sizeof(kCommandMagic) &&
      std::memcmp(data, kCommandMagic, sizeof(kCommandMagic)) == 0) {
    data += sizeof(kCommandMagic);
    size -= sizeof(kCommandMagic);
    if (size >= 1) {
      recv_->handleCommand(*data, data + 1, size - 1);
    }
    lastSeq_ = seq;
    return;
  }

  if (stripsInPacket > 0) {
    // Start a new frame if no strips have been received
    if (isAll(frameStrips_, false)) {
      recv_->startPixels();
    }

    for (size_t i = 0; i < stripsInPacket; i++) {
      // If we've already seen a particular strip, it probably means a
      // new frame has started, so show what we've got and start again

      size_t stripNum = *data;
      bool useData = true;  // Only use if we haven't seen this strip
                            // or the sequence number increased

      if (stripNum < frameStrips_.size()) {
        if (frameStrips_[stripNum]) {
          // Check for incrementing sequence, in case we're seeing an
          // old or duplicate packet
          if (static_cast<int32_t>(seq - lastSeq_) > 0) {
            // We've already seen the strip so trigger an end-of-frame
            // and restart
            recv_->endPixels();
            frameStrips_.clear();
            recv_->startPixels();
          } else {
            // This appears to be duplicate data
            useData = false;
          }
        }
        frameStrips_[stripNum] = true;
      }

      if (useData) {
        recv_->pixels(stripNum, data + 1, ppData1_.pixelsPerStrip);
      }
      data += 1 + uintptr_t{ppData1_.pixelsPerStrip}*3;
  }

    // If there's a whole frame then show the pixels
    if (isAll(frameStrips_, true)) {
      recv_->endPixels();
      frameStrips_.clear();
    }
  }

  // Update the discovery packet
  IPAddress ip = pixelsUDP_.remoteIP();
  ppData2_.lastDrivenIp[0] = ip[0];
  ppData2_.lastDrivenIp[1] = ip[1];
  ppData2_.lastDrivenIp[2] = ip[2];
  ppData2_.lastDrivenIp[3] = ip[3];
  ppData2_.lastDrivenPort = pixelsUDP_.remotePort();
  int32_t seqDiff = seq - lastSeq_ - 1;
  if (seqDiff > 0) {  // Want a difference of at least 2
    ppData1_.deltaSequence += seqDiff;
  }
  lastSeq_ = seq;

  // Take the average of the last 'k' times
  uint32_t updateTime = micros() - startTime;
  size_t k = lastUpdateTimes_->capacity();
  if (lastUpdateTimes_->size() < k) {
    avUpdateTime_ = (avUpdateTime_*lastUpdateTimes_->size() + updateTime)/
                    static_cast<float>(lastUpdateTimes_->size() + 1);
  } else {
    avUpdateTime_ = avUpdateTime_ +
                    static_cast<int32_t>(updateTime - lastUpdateTimes_->get())/
                        static_cast<float>(k);
  }
  lastUpdateTimes_->put(updateTime);
  ppData1_.updatePeriod = avUpdateTime_;
}

void PixelPusherServer::sendDiscovery() {
  discoveryUDP_.beginPacket(broadcastIP_, kDiscoveryPort);
  discoveryUDP_.write(reinterpret_cast<unsigned char *>(&deviceData_),
                      sizeof(deviceData_));
  discoveryUDP_.write(reinterpret_cast<unsigned char *>(&ppData1_),
                      sizeof(ppData1_));

  // Mystery padding. Why?
  // The claim is that the compiler for later versions of PixelPusher
  // software aligns the strip flags, a byte array, on a 4-byte
  // boundary, even though this goes against common C struct alignment
  // rules, where byte arrays don't need to be aligned.
  discoveryUDP_.write(uint8_t{0});
  discoveryUDP_.write(uint8_t{0});

  discoveryUDP_.write(stripFlags_.get(), stripFlagsSize_);

  // More mystery padding
  discoveryUDP_.write(uint8_t{0});
  discoveryUDP_.write(uint8_t{0});

  discoveryUDP_.write(reinterpret_cast<unsigned char *>(&ppData2_),
                      sizeof(ppData2_));
  discoveryUDP_.endPacket();

  // Some debug output
  // printf("update=%" PRIu32 " delta=%" PRIu32 "\r\n",
  //        ppData1_.updatePeriod, ppData1_.deltaSequence);

  ppData1_.deltaSequence = 0;
}
