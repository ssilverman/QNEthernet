// SPDX-FileCopyrightText: (c) 2022 Shawn Silverman <shawn@pobox.com>
// SPDX-License-Identifier: MIT

// QNEthernetIEEE1588.cpp contains the EthernetIEEE1588 implementation.
// This file is part of the QNEthernet library.

#include "QNEthernetIEEE1588.h"
#include "lwip_driver.h" 

namespace qindesign {
namespace network {

// Define the singleton instance.
EthernetIEEE1588Class EthernetIEEE1588Class::instance_;

// A reference to the singleton.
EthernetIEEE1588Class &EthernetIEEE1588 = EthernetIEEE1588Class::instance();

void EthernetIEEE1588Class::begin() const {
  enet_ieee1588_init();
}

void EthernetIEEE1588Class::end() const {
  enet_ieee1588_deinit();
}

bool EthernetIEEE1588Class::readTimer(timespec &t) const {
  return enet_ieee1588_read_timer(&t);
}

bool EthernetIEEE1588Class::writeTimer(const timespec &t) const {
  return enet_ieee1588_write_timer(&t);
}

bool EthernetIEEE1588Class::offsetTimer(int64_t ns) const{
  return enet_ieee1588_offset_timer(ns);
}

void EthernetIEEE1588Class::timestampNextFrame() const {
  enet_ieee1588_timestamp_next_frame();
}

bool EthernetIEEE1588Class::readAndClearTxTimestamp(
    struct timespec &timestamp) const {
  return enet_ieee1588_read_and_clear_tx_timestamp(&timestamp);
}

bool EthernetIEEE1588Class::adjustTimer(uint32_t corrInc,
                                        uint32_t corrPeriod) const {
  return enet_ieee1588_adjust_timer(corrInc, corrPeriod);
}

bool EthernetIEEE1588Class::adjustFreq(double nsps) const {
  return enet_ieee1588_adjust_freq(nsps);
}

bool EthernetIEEE1588Class::setChannelMode(int channel,
                                           TimerChannelModes mode) const {
  return enet_ieee1588_set_channel_mode(channel, static_cast<int>(mode));
}

bool EthernetIEEE1588Class::setChannelOutputPulseWidth(int channel,
                                                       int pulseWidth) const {
  return enet_ieee1588_set_channel_output_pulse_width(channel,
                                                      pulseWidth);
}

bool EthernetIEEE1588Class::setChannelCompareValue(int channel,
                                                   uint32_t value) const {
  return enet_ieee1588_set_channel_compare_value(channel, value);
}

bool EthernetIEEE1588Class::getChannelCompareValue(int channel,
                                                   uint32_t &value) const {
  return enet_ieee1588_get_channel_compare_value(channel, &value);
}

bool EthernetIEEE1588Class::getAndClearChannelStatus(int channel) const {
  return enet_ieee1588_get_and_clear_channel_status(channel);
}

bool EthernetIEEE1588Class::setChannelInterruptEnable(int channel, bool enable) const {
  return enet_ieee1588_set_channel_interrupt_enable(channel, enable);
}

EthernetIEEE1588Class::operator bool() const {
  return enet_ieee1588_is_enabled();
}

}  // namespace network
}  // namespace qindesign
