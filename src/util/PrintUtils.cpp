// SPDX-FileCopyrightText: (c) 2022 Shawn Silverman <shawn@pobox.com>
// SPDX-License-Identifier: MIT

// PrintUtils.cpp implements the Print utility functions.
// This file is part of the QNEthernet library.

#include "PrintUtils.h"

namespace qindesign {
namespace network {

util::StdoutPrint stdoutPrint;

namespace util {

static constexpr uint8_t kBroadcastMAC[6]{0xff, 0xff, 0xff, 0xff, 0xff, 0xff};

size_t writeFully(Print &p, const uint8_t *buf, size_t size,
                  std::function<bool()> breakf) {
  size_t total = size;

  if (breakf == nullptr) {
    while (size > 0) {
      size_t written = p.write(buf, size);
      size -= written;
      buf += written;
    }
    return total;
  }

  while (size > 0 && !breakf()) {
    size_t written = p.write(buf, size);
    size -= written;
    buf += written;
  }
  return total - size;
}

size_t writeMagic(Print &p, uint8_t mac[6], std::function<bool()> breakf) {
  size_t written = writeFully(p, kBroadcastMAC, 6, breakf);
  if (written < 6) {
    return written;
  }
  for (int i = 0; i < 16; i++) {
    size_t w = writeFully(p, mac, 6, breakf);
    written += w;
    if (w < 6) {
      return written;
    }
  }
  return written;
}

size_t StdoutPrint::write(uint8_t b) {
  if (std::putchar(b) == EOF) {
    setWriteError();
    return 0;
  }
  return 1;
}

size_t StdoutPrint::write(const uint8_t *buffer, size_t size) {
  size_t retval = std::fwrite(buffer, 1, size, stdout);
  if (std::ferror(stdout)) {
    setWriteError();
  }
  return retval;
}

int StdoutPrint::availableForWrite() {
  return 0;
}

void StdoutPrint::flush() {
  if (std::fflush(stdout) == EOF) {
    setWriteError();
  }
}

}  // namespace util
}  // namespace network
}  // namespace qindesign
