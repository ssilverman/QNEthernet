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
