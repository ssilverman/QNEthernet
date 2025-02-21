// SPDX-FileCopyrightText: (c) 2022-2024 Shawn Silverman <shawn@pobox.com>
// SPDX-License-Identifier: AGPL-3.0-or-later

// PrintUtils.cpp implements the Print utility functions.
// This file is part of the QNEthernet library.

#include "qnethernet/util/PrintUtils.h"

namespace qindesign {
namespace network {

namespace util {

static constexpr uint8_t kBroadcastMAC[ETH_HWADDR_LEN]{
    0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
};

size_t writeFully(Print &p, const void *const buf, const size_t size,
                  const std::function<bool()> breakf) {
  const uint8_t *pBuf = static_cast<const uint8_t *>(buf);
  size_t sizeRem = size;

  if (breakf == nullptr) {
    while (sizeRem > 0) {
      const size_t written = p.write(pBuf, sizeRem);
      sizeRem -= written;
      pBuf += written;
    }
    return size;
  }

  while (sizeRem > 0 && !breakf()) {
    const size_t written = p.write(pBuf, sizeRem);
    sizeRem -= written;
    pBuf += written;
  }
  return size - sizeRem;
}

size_t writeMagic(Print &p, const uint8_t mac[ETH_HWADDR_LEN],
                  const std::function<bool()> breakf) {
  size_t written = writeFully(p, kBroadcastMAC, ETH_HWADDR_LEN, breakf);
  if (written < ETH_HWADDR_LEN) {
    return written;
  }
  for (int i = 0; i < 16; i++) {
    const size_t w = writeFully(p, mac, ETH_HWADDR_LEN, breakf);
    written += w;
    if (w < ETH_HWADDR_LEN) {
      return written;
    }
  }
  return written;
}

// NOTE: It's not possible to override clearWriteError(), so check it in
//       each function

size_t StdioPrint::write(const uint8_t b) {
  checkAndClearErr();

  if (std::fputc(b, stream_) == EOF) {
    setWriteError(1);
    return 0;
  }
  return 1;
}

size_t StdioPrint::write(const uint8_t *const buffer, const size_t size) {
  checkAndClearErr();

  const size_t retval = std::fwrite(buffer, 1, size, stream_);
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
