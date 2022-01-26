// SPDX-FileCopyrightText: (c) 2022 Shawn Silverman <shawn@pobox.com>
// SPDX-License-Identifier: MIT

// QNEthernetIEEE1588.h defines an interface to IEEE 1588 functions.
// This file is part of the QNEthernet library.

#ifndef QNE_ETHERNETIEEE1588_H_
#define QNE_ETHERNETIEEE1588_H_

// C++ includes
#include <cstdint>
#include <ctime>

namespace qindesign {
namespace network {

// Provides an API for raw Ethernet frames, similar to the UDP API.
class EthernetIEEE1588Class final {
 public:
  enum class TimerChannelModes {
    kDisable = 0,
    kCaptureOnRising = 1,
    kCaptureOnFalling = 2,
    kCaptureOnBoth = 3,
    kSoftwareCompare = 4,
    kToggleOnCompare = 5,
    kClearOnCompare = 6,
    kSetOnCompare = 7,
    kClearOnCompareSetOnOverflow = 10,
    kSetOnCompareClearOnOverflow = 11,
    kPulseLowOnCompare = 14,
    kPulseHighOnCompare = 15,
  };

  // Accesses the singleton instance.
  static EthernetIEEE1588Class &instance() {
    return instance_;
  }

  // EthernetIEEE1588Class is neither copyable nor movable.
  EthernetIEEE1588Class(const EthernetIEEE1588Class &) = delete;
  EthernetIEEE1588Class &operator=(const EthernetIEEE1588Class &) = delete;

  // Starts the IEEE 1588 timer.
  void begin() const;

  // Stops the IEEE 1588 timer.
  void end() const;

  // Reads the current IEEE 1588 timer value. This returns whether successful.
  bool readTimer(timespec &t) const;

  // Writes the current IEEE 1588 timer value. This returns whether successful.
  bool writeTimer(const timespec &t) const;

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
  bool readAndClearTxTimestamp(timespec &timestamp) const;

  // Adjusts the raw correction settings. The increment must be in the range
  // 0-127 and the period must be in the range 0-(2^31-1), zero meaning
  // no correction. This will return whether successful.
  bool adjustTimer(uint32_t corrInc, uint32_t corrPeriod) const;

  // Adjusts the correction frequency in nanoseconds per second. To slow down
  // the timer, specify a negative value. To speed it up, specify a positive
  // value. This returns whether successful.
  bool adjustFreq(int nsps) const;

  // Sets the channel mode for the given channel. This does not set the output
  // compare pulse modes. This returns whether successful.
  //
  // This will return false for an unknown channel or if the mode is one of the
  // output compare pulse modes.
  bool setChannelMode(int channel, TimerChannelModes mode) const;

  // Sets the output compare pulse mode and pulse width for the given channel.
  // The pulse width must be in the range 1-32. This only sets the output
  // compare pulse modes. This returns whether successful.
  bool setChannelOutputPulseWidth(int channel,
                                  TimerChannelModes mode,
                                  int pulseWidth) const;

  // Sets the channel compare value. This returns whether successful.
  bool setChannelCompareValue(int channel, uint32_t value) const;

  // Retrieves and then clears the status for the given channel. This will
  // return false for an unknown channel.
  bool getAndClearChannelStatus(int channel) const;

  // Tests if the IEEE 1588 timer has been started.
  operator bool() const;

 private:
  EthernetIEEE1588Class() = default;
  ~EthernetIEEE1588Class() = default;

  // The singleton instance.
  static EthernetIEEE1588Class instance_;
};

}  // namespace network
}  // namespace qindesign

#endif  // #ifndef QNE_ETHERNETIEEE1588_H_
