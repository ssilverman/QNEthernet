// SPDX-FileCopyrightText: (c) 2022-2025 Shawn Silverman <shawn@pobox.com>
// SPDX-License-Identifier: MIT

// QNEthernetIEEE1588.cpp contains the EthernetIEEE1588 implementation.
// This file is part of the QNEthernet library.

#include "qnethernet/QNEthernetIEEE1588.h"

#include "lwip_driver.h"

namespace qindesign {
namespace network {

// A reference to the singleton.
STATIC_INIT_DEFN(EthernetIEEE1588Class, EthernetIEEE1588);

void EthernetIEEE1588Class::begin() const {
  driver_ieee1588_init();
}

void EthernetIEEE1588Class::end() const {
  driver_ieee1588_deinit();
}

bool EthernetIEEE1588Class::readTimer(timespec &t) const {
  return driver_ieee1588_read_timer(&t);
}

bool EthernetIEEE1588Class::writeTimer(const timespec &t) const {
  return driver_ieee1588_write_timer(&t);
}

void EthernetIEEE1588Class::timestampNextFrame() const {
  driver_ieee1588_timestamp_next_frame();
}

bool EthernetIEEE1588Class::readAndClearTxTimestamp(
    struct timespec &timestamp) const {
  return driver_ieee1588_read_and_clear_tx_timestamp(&timestamp);
}

bool EthernetIEEE1588Class::adjustTimer(uint32_t corrInc,
                                        uint32_t corrPeriod) const {
  return driver_ieee1588_adjust_timer(corrInc, corrPeriod);
}

bool EthernetIEEE1588Class::adjustFreq(int nsps) const {
  return driver_ieee1588_adjust_freq(nsps);
}

bool EthernetIEEE1588Class::setChannelMode(int channel,
                                           TimerChannelModes mode) const {
  return driver_ieee1588_set_channel_mode(channel, static_cast<int>(mode));
}

bool EthernetIEEE1588Class::setChannelOutputPulseWidth(int channel,
                                                       int pulseWidth) const {
  return driver_ieee1588_set_channel_output_pulse_width(channel, pulseWidth);
}

bool EthernetIEEE1588Class::setChannelCompareValue(int channel,
                                                   uint32_t value) const {
  return driver_ieee1588_set_channel_compare_value(channel, value);
}

bool EthernetIEEE1588Class::getAndClearChannelStatus(int channel) const {
  return driver_ieee1588_get_and_clear_channel_status(channel);
}

EthernetIEEE1588Class::operator bool() const {
  return driver_ieee1588_is_enabled();
}

}  // namespace network
}  // namespace qindesign
