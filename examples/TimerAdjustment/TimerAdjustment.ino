// SPDX-FileCopyrightText: (c) 2023 Jens Schleusner <github@schleusner.dev>
// SPDX-License-Identifier: MIT
//
// This file is part of the QNEthernet library.

#include <QNEthernet.h>
using namespace qindesign::network;

IntervalTimer myTimer;

int adjustment = 0;
timespec tmlast;

void setup() {

  Serial.begin(2000000);
  Serial.println("Setup EthernetIEEE1588");

  qindesign::network::Ethernet.begin();
  qindesign::network::EthernetIEEE1588.begin();

  myTimer.begin(timerInterrupt, 1000000);
  EthernetIEEE1588.readTimer(tmlast);

}

void timerInterrupt() {
  timespec tm;
  EthernetIEEE1588.readTimer(tm);//read current timer value
  int lastadjustment=adjustment;
  adjustment += 100; //increase timer spped by 100ns per second
  EthernetIEEE1588.adjustFreq(adjustment);

  int diff = (static_cast<int64_t>(tm.tv_sec) - static_cast<int64_t>(tmlast.tv_sec)) * (1000 * 1000 * 1000) + (static_cast<int64_t>(tm.tv_nsec) - static_cast<int64_t>(tmlast.tv_nsec));
  diff -= (1000 * 1000 * 1000);

  Serial.printf("Adjustment:%d nsps ATINC %08" PRIX32 "h ATCOR %d Diff %d\n",lastadjustment,ENET_ATINC,ENET_ATCOR,diff);
  //Serial.printf("%d %d\n", lastadjustment, diff);

  tmlast = tm;
  
}

void loop() {

}
