// SPDX-FileCopyrightText: (c) 2022-2023 Shawn Silverman <shawn@pobox.com>
// SPDX-License-Identifier: MIT

// PrintUtils.h declares Print utility functions.
// This file is part of the QNEthernet library.

#ifndef QNETHERNET_UTIL_PRINTUTILS_H_
#define QNETHERNET_UTIL_PRINTUTILS_H_

// C++ includes
#include <cstddef>
#include <cstdint>
#include <cstdio>
#include <functional>

#include <Print.h>

namespace qindesign {
namespace network {
namespace util {

// Attempts to completely write the specified bytes. This loops until everything
// is written or `breakf` returns true. If `breakf` is NULL then it is assumed
// to never return true.
//
// In other words, the `breakf` function is the stopping condition.
//
// This returns the number of bytes actually written. If `breakf` never returns
// true then this will have written all the bytes upon return.
size_t writeFully(Print &p, const uint8_t *buf, size_t size,
                  std::function<bool()> breakf = nullptr);

// Writes "magic packet" bytes to the given Print object. This passes `breakf`
// to `writeFully()`.
//
// This utilizes `writeFully()` under the covers, meaning this loops until
// all bytes are written or `breakf` returns true.
size_t writeMagic(Print &p, uint8_t mac[6],
                  std::function<bool()> breakf = nullptr);

// A Print decorator for stdio output files. The purpose of this is to utilize
// the Print class's ability to print Printable objects but using the underlying
// FILE*. This ensures that a Printable object gets printed using the same
// settings and buffering that the file uses.
//
// On errors, the "write error" gets set to 'errno'. Also, calling
// 'clearWriteError()' will cause future calls to clear any error via
// 'std::clearerr()'.
//
// See: https://pubs.opengroup.org/onlinepubs/9699919799/basedefs/stdio.h.html
class StdioPrint : public Print {
 public:
  explicit StdioPrint(std::FILE *stream) : stream_(stream) {}

  size_t write(uint8_t b) override;
  size_t write(const uint8_t *buffer, size_t size) override;
  int availableForWrite() override;
  void flush() override;

 private:
  // Checks and clears any error because clearWriteError() is not overridable.
  void checkAndClearErr();

  std::FILE *stream_;
};

}  // namespace util
}  // namespace network
}  // namespace qindesign

#endif  // QNETHERNET_UTIL_PRINTUTILS_H_
