// SPDX-FileCopyrightText: (c) 2022-2024 Shawn Silverman <shawn@pobox.com>
// SPDX-License-Identifier: AGPL-3.0-or-later

// PrintUtils.cpp implements the Print utility functions.
// This file is part of the QNEthernet library.

#include "PrintUtils.h"

namespace qindesign {
namespace network {

namespace util {

static constexpr uint8_t kBroadcastMAC[ETH_HWADDR_LEN]{
    0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
};

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

size_t writeMagic(Print &p, const uint8_t mac[ETH_HWADDR_LEN],
                  std::function<bool()> breakf) {
  size_t written = writeFully(p, kBroadcastMAC, ETH_HWADDR_LEN, breakf);
  if (written < ETH_HWADDR_LEN) {
    return written;
  }
  for (int i = 0; i < 16; i++) {
    size_t w = writeFully(p, mac, ETH_HWADDR_LEN, breakf);
    written += w;
    if (w < ETH_HWADDR_LEN) {
      return written;
    }
  }
  return written;
}

// NOTE: It's not possible to override clearWriteError(), so check it in
//       each function

size_t StdioPrint::write(uint8_t b) {
  checkAndClearErr();

  if (std::fputc(b, stream_) == EOF) {
    setWriteError(1);
    return 0;
  }
  return 1;
}

size_t StdioPrint::write(const uint8_t *buffer, size_t size) {
  checkAndClearErr();

  size_t retval = std::fwrite(buffer, 1, size, stream_);
  if (std::ferror(stream_) != 0) {
    setWriteError(1);
  }
  return retval;
}

int StdioPrint::availableForWrite() {
  return stream_->_w;
}

void StdioPrint::flush() {
  checkAndClearErr();

  if (std::fflush(stream_) == EOF) {
    setWriteError(1);
  }
}

void StdioPrint::checkAndClearErr() {
  if (std::ferror(stream_) != 0 && getWriteError() == 0) {
    std::clearerr(stream_);
  }
}

}  // namespace util
}  // namespace network
}  // namespace qindesign
