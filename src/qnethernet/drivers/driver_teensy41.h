// SPDX-FileCopyrightText: (c) 2021-2024 Shawn Silverman <shawn@pobox.com>
// SPDX-License-Identifier: AGPL-3.0-or-later

// driver_teensy41.h defines Teensy 4.1-specific things.
// This file is part of the QNEthernet library.

#pragma once

// lwIP options

// ARP options
#define ETH_PAD_SIZE 2  /* 0 */

// Checksum options
#define CHECKSUM_GEN_IP      0  /* 1 */
#define CHECKSUM_GEN_UDP     0  /* 1 */
#define CHECKSUM_GEN_TCP     0  /* 1 */
#define CHECKSUM_GEN_ICMP    0  /* 1 */
// #define CHECKSUM_GEN_ICMP6   1
#define CHECKSUM_CHECK_IP    0  /* 1 */
#define CHECKSUM_CHECK_UDP   0  /* 1 */
#define CHECKSUM_CHECK_TCP   0  /* 1 */
#define CHECKSUM_CHECK_ICMP  0  /* 1 */
// #define CHECKSUM_CHECK_ICMP6 1
