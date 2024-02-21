// SPDX-FileCopyrightText: (c) 2024 Shawn Silverman <shawn@pobox.com>
// SPDX-License-Identifier: AGPL-3.0-or-later

// driver_w5500_config.h contains the W5500 Ethernet interface configuration.
// This file is part of the QNEthernet library.

#pragma once

#include <SPI.h>

// SPI settings
// static SPISettings kSPISettings{14000000, MSBFIRST, SPI_MODE0};
static const SPISettings kSPISettings{30000000, MSBFIRST, SPI_MODE0};
static SPIClass &spi = SPI;
static constexpr int kDefaultCSPin = 10;

static constexpr bool kSocketInterruptsEnabled = false;

#if !(defined(TEENSYDUINO) && defined(__IMXRT1062__))
// The default MAC address if one isn't specified.
// Local, non-multicast: Lower two bits of the top byte must be 0b10.
static constexpr uint8_t kDefaultMACAddress[6]{0x02, 0, 0, 0, 0, 0};
#endif  // !(defined(TEENSYDUINO) && defined(__IMXRT1062__))
