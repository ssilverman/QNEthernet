// SPDX-FileCopyrightText: (c) 2022-2025 Shawn Silverman <shawn@pobox.com>
// SPDX-License-Identifier: MIT

// QNEthernetIEEE1588.cpp contains the EthernetIEEE1588 implementation.
// This file is part of the QNEthernet library.

#include "qnethernet/QNEthernetIEEE1588.h"

#include "qnethernet/lwip_driver.h"

namespace qindesign {
namespace network {

// A reference to the singleton.
STATIC_INIT_DEFN(EthernetIEEE1588Class, EthernetIEEE1588);

void EthernetIEEE1588Class::begin() const {
  driver::ieee1588::init();
}

void EthernetIEEE1588Class::end() const {
  driver::ieee1588::deinit();
}

void EthernetIEEE1588Class::readTimer(timespec& t) const {
  driver::ieee1588::read_timer(&t);
}

void EthernetIEEE1588Class::writeTimer(const timespec& t) const {
  driver::ieee1588::write_timer(&t);
}

void EthernetIEEE1588Class::timestampNextFrame() const {
  driver::ieee1588::timestamp_next_frame();
}

bool EthernetIEEE1588Class::readAndClearTxTimestamp(
    struct timespec& timestamp) const {
  return driver::ieee1588::read_and_clear_tx_timestamp(&timestamp);
}

void EthernetIEEE1588Class::adjustTimer(uint32_t corrInc,
                                        uint32_t corrPeriod) const {
  driver::ieee1588::adjust_timer(corrInc, corrPeriod);
}

void EthernetIEEE1588Class::adjustFreq(int nsps) const {
  driver::ieee1588::adjust_freq(nsps);
}

// ----------
//  Channels
// ----------

size_t EthernetIEEE1588Class::channelCount() const {
  return driver::ieee1588::get_channel_count();
}

uint32_t EthernetIEEE1588Class::channelStatus(size_t channel) const {
  uint32_t status;
  driver::ieee1588::get_channel_status(channel, &status);
  return status;
}

void EthernetIEEE1588Class::setChannelStatus(const size_t channel,
                                             const uint32_t status) const {
  driver::ieee1588::set_channel_status(channel, status);
}

void EthernetIEEE1588Class::waitForChannelStatus(const size_t channel,
                                                 const uint32_t status) const {
  driver::ieee1588::wait_for_channel_status(channel, status);
}

void EthernetIEEE1588Class::setChannelMode(uint32_t* const status,
                                           TimerChannelModes mode) const {
  if (status == nullptr) {
    return;
  }
  driver::ieee1588::set_channel_mode(status, static_cast<uint32_t>(mode));
}

EthernetIEEE1588Class::TimerChannelModes EthernetIEEE1588Class::channelMode(
    const uint32_t status) const {
  uint32_t mode = driver::ieee1588::get_channel_mode(status);
  switch (mode) {
    case 8:
      ATTRIBUTE_FALLTHROUGH;
    case 12:
      ATTRIBUTE_FALLTHROUGH;
    case 13:
      return TimerChannelModes::kReserved;
    case 9:
      ATTRIBUTE_FALLTHROUGH;
    case 11:
      return TimerChannelModes::kSetOnCompareClearOnOverflow;
    default:
      return static_cast<TimerChannelModes>(mode);
  }
}

void EthernetIEEE1588Class::setChannelPulseWidth(uint32_t* const status,
                                                 uint32_t pulseWidth) const {
  if ((status == nullptr) || (pulseWidth < 1) || (32 < pulseWidth)) {
    return;
  }
  driver::ieee1588::set_channel_pulse_width(status, pulseWidth);
}

uint32_t EthernetIEEE1588Class::channelPulseWidth(const uint32_t status) const {
  return driver::ieee1588::get_channel_pulse_width(status);
}

void EthernetIEEE1588Class::setChannelInterruptEnable(uint32_t* const status,
                                                      const bool flag) const {
  if (status == nullptr) {
    return;
  }
  driver::ieee1588::set_channel_interrupt_enable(status, flag);
}

bool EthernetIEEE1588Class::channelInterruptEnable(
    const uint32_t status) const {
  return driver::ieee1588::get_channel_interrupt_enable(status);
}

void EthernetIEEE1588Class::setChannelDMARequestEnable(uint32_t* const status,
                                                       const bool flag) const {
  if (status == nullptr) {
    return;
  }
  driver::ieee1588::set_channel_dma_request_enable(status, flag);
}

bool EthernetIEEE1588Class::channelDMARequestEnable(
    const uint32_t status) const {
  return driver::ieee1588::get_channel_dma_request_enable(status);
}

void EthernetIEEE1588Class::setChannelCompareValue(const size_t channel,
                                                   const uint32_t value) const {
  driver::ieee1588::set_channel_compare_value(channel, value);
}

uint32_t EthernetIEEE1588Class::channelCompareValue(
    const size_t channel) const {
  return driver::ieee1588::get_channel_compare_value(channel);
}

bool EthernetIEEE1588Class::getAndClearChannelStatus(size_t channel) const {
  return driver::ieee1588::get_and_clear_channel_status(channel);
}

EthernetIEEE1588Class::operator bool() const {
  return driver::ieee1588::is_enabled();
}

}  // namespace network
}  // namespace qindesign
