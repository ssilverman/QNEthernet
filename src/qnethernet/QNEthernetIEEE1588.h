// SPDX-FileCopyrightText: (c) 2022-2023,2025 Shawn Silverman <shawn@pobox.com>
// SPDX-License-Identifier: MIT

// QNEthernetIEEE1588.h defines an interface to IEEE 1588 functions.
// This file is part of the QNEthernet library.

#pragma once

// C++ includes
#include <cstdint>
#include <ctime>
#include <limits>

#include "qnethernet/StaticInit.h"

namespace qindesign {
namespace network {

// Provides an API for raw Ethernet frames, similar to the UDP API.
class EthernetIEEE1588Class final {
 public:
  enum class TimerChannelModes : uint32_t {
    kDisable                     = 0,
    kCaptureOnRising             = 1,
    kCaptureOnFalling            = 2,
    kCaptureOnBoth               = 3,
    kSoftwareCompare             = 4,
    kToggleOnCompare             = 5,
    kClearOnCompare              = 6,
    kSetOnCompare                = 7,
    kClearOnCompareSetOnOverflow = 10,
    kSetOnCompareClearOnOverflow = 11,
    kPulseLowOnCompare           = 14,
    kPulseHighOnCompare          = 15,
    kReserved = std::numeric_limits<uint32_t>::max(),
  };

  // EthernetIEEE1588Class is neither copyable nor movable.
  EthernetIEEE1588Class(const EthernetIEEE1588Class&) = delete;
  EthernetIEEE1588Class(EthernetIEEE1588Class&&) = delete;
  EthernetIEEE1588Class& operator=(const EthernetIEEE1588Class&) = delete;
  EthernetIEEE1588Class& operator=(EthernetIEEE1588Class&&) = delete;

  // Starts the IEEE 1588 timer.
  void begin() const;

  // Stops the IEEE 1588 timer.
  void end() const;

  // Reads the current IEEE 1588 timer value.
  void readTimer(timespec& t) const;

  // Writes the current IEEE 1588 timer value.
  void writeTimer(const timespec& t) const;

  // Tells the driver to timestamp the next transmitted frame. This should be
  // called before functions like `EthernetUDP::endPacket()`,
  // `EthernetUDP::send()`, and any of the `EthernetFrame` send functions.
  void timestampNextFrame() const;

  // Attempts to retrieve the timestamp of the last transmitted frame and
  // returns whether one is available. If available and the parameter is not
  // NULL then the timestamp is assigned to `*timestamp`. This clears the
  // timestamp state so that a subsequent call will return false.
  //
  // This will always returns false if `EthernetIEEE1588.timestampNextFrame()`
  // was not called before this.
  bool readAndClearTxTimestamp(timespec& timestamp) const;

  // Adjusts the raw correction settings. The increment must be in the range
  // 0-127 (7 bits) and the period must be in the range 0-(2^31-1) (31 bits),
  // zero meaning no correction.
  void adjustTimer(uint32_t corrInc, uint32_t corrPeriod) const;

  // Adjusts the correction frequency in nanoseconds per second. To slow down
  // the timer, specify a negative value. To speed it up, specify a positive
  // value.
  void adjustFreq(int nsps) const;

  // ----------
  //  Channels
  // ----------

  // Returns the channel count.
  size_t channelCount() const;

  // Returns the current complete channel status. This returns zero for channels
  // that are out of range.
  uint32_t channelStatus(size_t channel) const;

  // Sets the channel status to the given value. Note that it may take a few
  // 1588 clock cycles for the settings to be applied. This ignores channels
  // that are out of range.
  //
  // See: waitForChannelStatus(channel, status)
  void setChannelStatus(size_t channel, uint32_t status) const;

  // Waits for the channel status to match the given value. This ignores the
  // TF flag and channels that are out of range.
  void waitForChannelStatus(size_t channel, uint32_t status) const;

  // Sets the channel mode for the given channel status.
  void setChannelMode(uint32_t* status, TimerChannelModes mode) const;

  // Returns the channel mode for the given channel status.
  TimerChannelModes channelMode(uint32_t status) const;

  // Sets the pulse width for the given channel status. The pulse width must be
  // in the range 1-32. This ignores pulse widths that are out of range.
  //
  // Note that the pulse width only has an effect when the mode is one of the
  // output compare pulse modes.
  void setChannelPulseWidth(uint32_t* status, uint32_t pulseWidth) const;

  // Returns the pulse width for the given channel status.
  uint32_t channelPulseWidth(uint32_t status) const;

  // Sets the Timer Interrupt Enable flag for the given channel status.
  void setChannelInterruptEnable(uint32_t* status, bool flag) const;

  // Returns the Timer Interrupt Enable flag for the given channel status.
  bool channelInterruptEnable(uint32_t status) const;

  // Sets the Timer DMA Request Enable flag for the given channel status.
  void setChannelDMARequestEnable(uint32_t* status, bool flag) const;

  // Returns the Timer DMA Request Enable flag for the given channel status.
  bool channelDMARequestEnable(uint32_t status) const;

  // Sets the channel compare value. This ignores channels that are out
  // of range.
  void setChannelCompareValue(size_t channel, uint32_t value) const;

  // Returns the channel compare value. This ignores channels that are out
  // of range.
  uint32_t channelCompareValue(size_t channel) const;

  // Retrieves and then clears the status for the given channel. This will
  // return false for an unknown channel.
  bool getAndClearChannelStatus(size_t channel) const;

  // Tests if the IEEE 1588 timer has been started.
  operator bool() const;

 private:
  EthernetIEEE1588Class() = default;
  ~EthernetIEEE1588Class() = default;

  friend class StaticInit<EthernetIEEE1588Class>;
};

// Instance for using IEEE 1588 functions.
STATIC_INIT_DECL(EthernetIEEE1588Class, EthernetIEEE1588);

}  // namespace network
}  // namespace qindesign
