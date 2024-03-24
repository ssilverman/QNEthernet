// SPDX-FileCopyrightText: (c) 2024 Shawn Silverman <shawn@pobox.com>
// SPDX-License-Identifier: AGPL-3.0-or-later

// mac_tools.c implements the functions defined in mac_tools.h.
// This file is part of the QNEthernet library.

#include "mac_tools.h"

// C includes
#include <stddef.h>
#include <string.h>

#include "qnethernet_opts.h"

#if defined(TEENSYDUINO)

#if defined(__IMXRT1062__)
#include <imxrt.h>
#elif defined(ARDUINO_TEENSY30) || \
      defined(ARDUINO_TEENSY32) || defined(ARDUINO_TEENSY31) || \
      defined(ARDUINO_TEENSYLC) || \
      defined(ARDUINO_TEENSY35) || \
      defined(ARDUINO_TEENSY36)
#include <kinetis.h>
#endif  // Teensy type

#else

static const uint8_t kDefaultMACAddress[ETH_HWADDR_LEN] = {
    QNETHERNET_DEFAULT_MAC_ADDRESS,
};

#endif  // defined(TEENSYDUINO)

void qnethernet_get_system_mac_address(uint8_t mac[ETH_HWADDR_LEN]) {
  if (mac != NULL) {
#if defined(TEENSYDUINO) && defined(__IMXRT1062__)
    uint32_t m1 = HW_OCOTP_MAC1;
    uint32_t m2 = HW_OCOTP_MAC0;
    mac[0] = m1 >> 8;
    mac[1] = m1 >> 0;
    mac[2] = m2 >> 24;
    mac[3] = m2 >> 16;
    mac[4] = m2 >> 8;
    mac[5] = m2 >> 0;
#elif defined(ARDUINO_TEENSY30) || \
      defined(ARDUINO_TEENSY32) || defined(ARDUINO_TEENSY31) || \
      defined(ARDUINO_TEENSYLC)
    // usb_desc.c:usb_init_serialnumber()
    __disable_irq();
		FTFL_FSTAT = FTFL_FSTAT_RDCOLERR | FTFL_FSTAT_ACCERR | FTFL_FSTAT_FPVIOL;
		FTFL_FCCOB0 = 0x41;
		FTFL_FCCOB1 = 15;
		FTFL_FSTAT = FTFL_FSTAT_CCIF;
		while (!(FTFL_FSTAT & FTFL_FSTAT_CCIF)) {
      // Wait
    }
		uint32_t num = *(uint32_t *)&FTFL_FCCOB7;
    __enable_irq();
    mac[0] = 0x04;
    mac[1] = 0xE9;
    mac[2] = 0xE5;
    mac[3] = num >> 16;
    mac[4] = num >> 8;
    mac[5] = num;
#elif defined(ARDUINO_TEENSY35) || defined(ARDUINO_TEENSY36)
    // usb_desc.c:usb_init_serialnumber()
    __disable_irq();
    kinetis_hsrun_disable();
    FTFL_FSTAT = FTFL_FSTAT_RDCOLERR | FTFL_FSTAT_ACCERR | FTFL_FSTAT_FPVIOL;
    *(uint32_t *)&FTFL_FCCOB3 = 0x41070000;
    FTFL_FSTAT = FTFL_FSTAT_CCIF;
    while (!(FTFL_FSTAT & FTFL_FSTAT_CCIF)) {
      // Wait
    }
		uint32_t num = *(uint32_t *)&FTFL_FCCOBB;
		kinetis_hsrun_enable();
    __enable_irq();
    mac[0] = 0x04;
    mac[1] = 0xE9;
    mac[2] = 0xE5;
    mac[3] = num >> 16;
    mac[4] = num >> 8;
    mac[5] = num;
#else
    memcpy(mac, kDefaultMACAddress, ETH_HWADDR_LEN);
#endif  // Board type
  }
}
