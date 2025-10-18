// SPDX-FileCopyrightText: (c) 2023 Jens Schleusner <github@schleusner.dev>
// SPDX-License-Identifier: MIT

// PPSOut generates a Pulse per Second using the IEEE1588-Timer
// Connect Oscilloscope to Pins 14,24,34
// Connect LED to Pin 14
//
// This file is part of the QNEthernet library.

#include <QNEthernet.h>
using namespace qindesign::network;

void setup() {

  Serial.begin(2000000);
  Serial.println("Setup EthernetIEEE1588");

  qindesign::network::Ethernet.begin();
  qindesign::network::EthernetIEEE1588.begin();

  IOMUXC_SW_MUX_CTL_PAD_GPIO_B1_13 = 3; //ENET_1588_EVENT0_OUT, IOMUX: ALT3, Teensy Pin: 34
  EthernetIEEE1588.setChannelCompareValue(0, 0); //Compare at counter value 0
  EthernetIEEE1588.setChannelMode(0, qindesign::network::EthernetIEEE1588.TimerChannelModes::kPulseHighOnCompare); //enable Channel0 positive pulse
  EthernetIEEE1588.setChannelOutputPulseWidth(0, 25); //Generate a Pulse width of 25 25MHz clock cycles (1us)

  IOMUXC_SW_MUX_CTL_PAD_GPIO_AD_B0_12 = 6; //ENET_1588_EVENT1_OUT, IOMUX: ALT6, Teensy Pin: 24
  EthernetIEEE1588.setChannelCompareValue(1, 1000); //Compare at counter value 1000
  EthernetIEEE1588.setChannelOutputPulseWidth(1, 10); //Generate a Pulse width of 10 25MHz clock cycles (400ns)
  EthernetIEEE1588.setChannelMode(1, qindesign::network::EthernetIEEE1588.TimerChannelModes::kPulseLowOnCompare); //enable Channel1 negative pulse

  IOMUXC_SW_MUX_CTL_PAD_GPIO_AD_B1_02 = 4; //ENET_1588_EVENT2_OUT, IOMUX: ALT4, Teensy Pin: 14
  EthernetIEEE1588.setChannelCompareValue(2, 500 * 1000 * 1000); //Compare at counter 500ms
  EthernetIEEE1588.setChannelMode(2, qindesign::network::EthernetIEEE1588.TimerChannelModes::kClearOnCompareSetOnOverflow); //enable Channel2 for 50/50 On-Off Signal

  Serial.printf("TCSR Register state: ENET_TCSR0 %08" PRIX32 "h ENET_TCSR1 %08" PRIX32 "h ENET_TCSR2 %08" PRIX32 "h\n", ENET_TCSR0, ENET_TCSR1, ENET_TCSR2); // (pg 2247, Rev. 3)
}

void loop() {
  // put your main code here, to run repeatedly:

}
