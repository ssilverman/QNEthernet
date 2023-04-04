// SPDX-FileCopyrightText: (c) 2023 Shawn Silverman <shawn@pobox.com>
// SPDX-License-Identifier: MIT

// Unity test framework configuration.
// This file is part of the QNEthernet library.

// #define UNITY_INCLUDE_CONFIG_H <-- Already defined, includes this file
#define UNITY_USE_FLUSH_STDOUT
#define UNITY_PRINT_EOL() { UNITY_OUTPUT_CHAR('\r'); UNITY_OUTPUT_CHAR('\n'); }
// Not including UNITY_INCLUDE_PRINT_FORMATTED because it doesn't support all
// the printf options
// #define UNITY_INCLUDE_PRINT_FORMATTED
