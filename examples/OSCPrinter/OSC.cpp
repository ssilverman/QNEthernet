// SPDX-FileCopyrightText: (c) 2017-2023 Shawn Silverman <shawn@pobox.com>
// SPDX-License-Identifier: MIT

// OSC.cpp contains OSC function definitions.
// This file is part of the QNEthernet library.

// Note: This needs the LiteOSCParser library.

#include "OSC.h"

// C++ includes
#include <algorithm>
#include <cinttypes>

#include <LiteOSCParser.h>

namespace osc = ::qindesign::osc;

// Prints the current OSC message. This does not terminate with a newline.
static void printMessage(Print &out, const osc::LiteOSCParser &osc);

// Prints an OSC bundle, one message per line. This terminates with a newline.
static void printBundle(Print &out, const uint8_t *b, int len);

// Prints an OSC datum.
static void printOSCData(Print &out, const osc::LiteOSCParser &osc, int index);

// Checks if the buffer starts as a valid bundle. If this returns true, then
// the bundle starts at len+16.
static bool isBundleStart(const uint8_t *buf, size_t len) {
  if (len < 16 || (len & 0x03) != 0) {
    return false;
  }
  if (!std::equal(&buf[0], &buf[8], "#bundle")) {
    return false;
  }
  return true;
}

void printOSC(Print &out, const uint8_t *b, int len) {
  osc::LiteOSCParser osc;

  // For bundles, loop over all the messages in the bundle, not doing
  // anything recursive
  if (isBundleStart(b, len)) {
    printBundle(out, b, len);
    return;
  }

  if (!osc.parse(b, len)) {
    if (osc.isMemoryError()) {
      out.println("#MemoryError");
    } else {
      out.println("#ParseError");
    }
    return;
  }
  printMessage(out, osc);
  out.println();
}

void printMessage(Print &out, const osc::LiteOSCParser &osc) {
  out.printf("%s", osc.getAddress());

  int size = osc.getArgCount();
  for (int i = 0; i < size; i++) {
    if (i == 0) {
      out.print(": ");
    } else {
      out.print(", ");
    }
    printOSCData(out, osc, i);
  }
}

void printBundle(Print &out, const uint8_t *b, int len) {
  if (len < 16) {
    return;
  }
  if (!std::equal(&b[0], &b[8], "#bundle")) {
    return;
  }
  out.println("#bundle");
  osc::LiteOSCParser osc;
  int index = 16;
  while (index + 4 <= len) {
    int32_t size = static_cast<int32_t>(uint32_t{b[index]} << 24 |
                                        uint32_t{b[index + 1]} << 16 |
                                        uint32_t{b[index + 2]} << 8 |
                                        uint32_t{b[index + 3]});
    index += 4;
    if (index + size > len) {
      break;
    }
    if (index < len && b[index] == '/') {
      if (osc.parse(&b[index], size)) {
        printMessage(out, osc);
        out.println();
      } else {
        if (osc.isMemoryError()) {
          out.println("#MemoryError");
        } else {
          out.println("#ParseError");
        }
      }
    }
    index += size;
  }
  out.println("#endbundle");
}

void printOSCData(Print &out, const osc::LiteOSCParser &osc, int index) {
  out.printf("%c(", osc.getTag(index));
  switch (osc.getTag(index)) {
    case 'i':
      out.print(osc.getInt(index));
      break;
    case 'f':
      out.print(osc.getFloat(index));
      break;
    case 's':
      out.printf("\"%s\"", osc.getString(index));
      break;
    case 'b': {
      out.print('[');
      const uint8_t *p = osc.getBlob(index);
      for (int i = osc.getBlobLength(index); --i >= 0; ) {
        out.printf(" %02x", *(p++));
      }
      out.print(']');
      break;
    }
    case 't':
      out.printf("%" PRIu32 ".%" PRIu32,
                 static_cast<uint32_t>(osc.getTime(index) >> 32),
                 static_cast<uint32_t>(osc.getTime(index)));
      break;
    case 'd':
      out.print(osc.getDouble(index));
      break;
    case 'c':
      out.printf("'%c'", osc.getChar(index));
      break;
    case 'T':
      out.print("true");
      break;
    case 'F':
      out.print("false");
      break;
    case 'N':
      out.print("null");
      break;
    case 'I':
      out.print("impulse");
      break;
  }
  out.print(')');
}
