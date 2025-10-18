// SPDX-FileCopyrightText: (c) 2023 Jens Schleusner <github@schleusner.dev>
// SPDX-License-Identifier: MIT

// PPSOut generates a Pulse per Second using the IEEE1588-Timer on Pin 24
// We read the pulse back on channels 0 and 2 to measure the timer delay
// Connect Oscilloscope to Pin 24
// Connect Pins 24(out), 15(in), 35(in)
//
// This file is part of the QNEthernet library.

#include <QNEthernet.h>
using namespace qindesign::network;

void setup() {

  Serial.begin(2000000);
  Serial.println("Setup EthernetIEEE1588");

  qindesign::network::Ethernet.begin();
  qindesign::network::EthernetIEEE1588.begin();

  //PPS-Out on Channel 1
  IOMUXC_SW_MUX_CTL_PAD_GPIO_AD_B0_12 = 6; //ENET_1588_EVENT1_OUT, IOMUX: ALT6, Teensy Pin: 24
  EthernetIEEE1588.setChannelCompareValue(1, 0); //Compare at counter value 0
  EthernetIEEE1588.setChannelMode(1, qindesign::network::EthernetIEEE1588.TimerChannelModes::kPulseHighOnCompare); //enable Channel0 positive pulse
  EthernetIEEE1588.setChannelOutputPulseWidth(1, 25); //Generate a Pulse width of 25 25MHz clock cycles (1us)
  
  //PPS-IN
  attachInterruptVector(IRQ_ENET_TIMER, interrupt_1588_timer); //Configure Interrupt Handler
  NVIC_ENABLE_IRQ(IRQ_ENET_TIMER); //Enable Interrupt Handling

  //PPS-In on Channel 2
  IOMUXC_SW_MUX_CTL_PAD_GPIO_AD_B1_03 = 4; //ENET_1588_EVENT2_IN, IOMUX: ALT4, Teensy Pin: 15
  EthernetIEEE1588.setChannelMode(2, qindesign::network::EthernetIEEE1588.TimerChannelModes::kCaptureOnRising); //enable Channel2 rising edge trigger
  EthernetIEEE1588.setChannelInterruptEnable(2, true); //Configure Interrupt generation

  //PPS-In on Channel 0
  IOMUXC_SW_MUX_CTL_PAD_GPIO_B1_12 = 3; //ENET_1588_EVENT0_IN, IOMUX: ALT3, Teensy Pin: 35
  IOMUXC_ENET0_TIMER_SELECT_INPUT = 0b10; // route B1_12 to 1588 timer (pg 796, Rev. 3)
  EthernetIEEE1588.setChannelMode(0, qindesign::network::EthernetIEEE1588.TimerChannelModes::kCaptureOnFalling);
  EthernetIEEE1588.setChannelInterruptEnable(0, true);


  Serial.printf("TCSR Register state: ENET_TCSR0 %08" PRIX32 "h ENET_TCSR1 %08" PRIX32 "h ENET_TCSR2 %08" PRIX32 "h\n", ENET_TCSR0, ENET_TCSR1, ENET_TCSR2); // (pg 2247, Rev. 3)
}

void loop() {
  // put your main code here, to run repeatedly:

}

static void interrupt_1588_timer() {
  uint32_t t;
  if(EthernetIEEE1588.getAndClearChannelStatus(0)){
    EthernetIEEE1588.getChannelCompareValue(0,t);
    Serial.printf("Timer0 Falling Edge: %d\n\n",  t);
  }
  if (EthernetIEEE1588.getAndClearChannelStatus(2)) {
    EthernetIEEE1588.getChannelCompareValue(2,t);
    Serial.printf("Timer2 Rising  Edge: %d\n",  t);
  }
  asm("dsb"); // allow write to complete so the interrupt doesn't fire twice
}

