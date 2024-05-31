// SPDX-FileCopyrightText: (c) 2022-2024 Shawn Silverman <shawn@pobox.com>
// SPDX-License-Identifier: AGPL-3.0-or-later

// PrintUtils.h declares Print utility functions.
// This file is part of the QNEthernet library.

#pragma once

// C++ includes
#include <cstddef>
#include <cstdint>
#include <cstdio>
#include <functional>

#include <Print.h>

#include "lwip/prot/ethernet.h"

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
size_t writeMagic(Print &p, const uint8_t mac[ETH_HWADDR_LEN],
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
  ~StdioPrint() = default;

  size_t write(uint8_t b) override;
  size_t write(const uint8_t *buffer, size_t size) override;
  int availableForWrite() override;
  void flush() override;

 protected:
  std::FILE *stream() const {
    return stream_;
  }

 private:
  // Checks and clears any error because clearWriteError() is not overridable.
  void checkAndClearErr();

  std::FILE *stream_;
};

// NullPrint is a Print object that accepts all writes and sends them nowhere.
class NullPrint final : public Print {
 public:
  NullPrint() = default;
  ~NullPrint() = default;

  size_t write(uint8_t b) override {
    LWIP_UNUSED_ARG(b);
    return 1;
  }

  size_t write(const uint8_t *buffer, size_t size) override {
    LWIP_UNUSED_ARG(buffer);
    return size;
  }

  int availableForWrite() override {
    return 0;
  }

  void flush() override {
  }
};

// PrintDecorator is a Print object that decorates another. This is meant to be
// a base class.
class PrintDecorator : public Print {
 public:
  PrintDecorator(Print &p) : p_(p) {}
  ~PrintDecorator() = default;

  size_t write(uint8_t b) override {
    return p_.write(b);
  }

  size_t write(const uint8_t *buffer, size_t size) override {
    return p_.write(buffer, size);
  }

  int availableForWrite() override {
    return p_.availableForWrite();
  }

  void flush() override {
    p_.flush();
  }

 private:
  Print &p_;
};

}  // namespace util
}  // namespace network
}  // namespace qindesign
