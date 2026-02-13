// SPDX-FileCopyrightText: (c) 2022-2026 Shawn Silverman <shawn@pobox.com>
// SPDX-License-Identifier: AGPL-3.0-or-later

// PrintUtils.h declares Print utility functions.
// This file is part of the QNEthernet library.

#pragma once

// C++ includes
#include <cstddef>
#include <cstdint>
#include <cstdio>
#include <functional>
#include <limits>

#include <Print.h>
#include <Stream.h>

#include "lwip/arch.h"
#include "lwip/prot/ethernet.h"
#include "qnethernet/internal/PrintfChecked.h"

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
size_t writeFully(Print& p, const void* buf, size_t size,
                  std::function<bool()> breakf = nullptr);

// Calls writeFully() with a break function that static_casts the given object
// to a bool.
template <typename T>
size_t writeFully(Print& p, const void* const buf, const size_t size,
                  T* const breakobj) {
  return writeFully(p, buf, size,
                    [&]() { return !static_cast<bool>(*breakobj); });
}

// Writes "magic packet" bytes to the given Print object. This passes `breakf`
// to `writeFully()`.
//
// This utilizes `writeFully()` under the covers, meaning this loops until
// all bytes are written or `breakf` returns true.
size_t writeMagic(Print& p, const uint8_t mac[ETH_HWADDR_LEN],
                  std::function<bool()> breakf = nullptr);

// Calls writeMagic() with a break function that static_casts the given object
// to a bool.
template <typename T>
size_t writeMagic(Print& p, const uint8_t mac[ETH_HWADDR_LEN],
                  T* const breakobj) {
  return writeMagic(p, mac, [&]() { return !static_cast<bool>(*breakobj); });
}

// --------------------------------------------------------------------------
//  Useful Print and Stream classes
// --------------------------------------------------------------------------

// PrintBase provides a checked printf function.
class PrintBase : public Print, public internal::PrintfChecked {
 public:
  PrintBase() = default;

  // Use the one from here instead of the one from Print because it
  // does checking
  using internal::PrintfChecked::printf;

  using Print::write;  // Add to the overload set

  // A convenience function that can write any type of data.
  size_t write(const void* const buf, size_t size) {
    return write(static_cast<const uint8_t*>(buf), size);
  }
};

// StreamBase provides a checked printf function.
class StreamBase : public Stream, public internal::PrintfChecked {
 public:
  StreamBase() = default;

  // Use the one from here instead of the one from Print because it
  // does checking
  using internal::PrintfChecked::printf;

  using Print::write;  // Add to the overload set

  // A convenience function that can write any type of data.
  size_t write(const void* const buf, size_t size) {
    return write(static_cast<const uint8_t*>(buf), size);
  }
};

// A Print decorator for stdio output files. The purpose of this is to utilize
// the Print class's ability to print Printable objects but using the underlying
// FILE*. This ensures that a Printable object gets printed using the same
// settings and buffering that the file uses.
//
// On errors, the "write error" gets set to 1. Also, calling 'clearWriteError()'
// will cause future calls to clear any error via 'std::clearerr()'.
//
// See: https://pubs.opengroup.org/onlinepubs/9699919799/basedefs/stdio.h.html
class StdioPrint : public PrintBase {
 public:
  explicit StdioPrint(std::FILE* stream) : stream_(stream) {}
  virtual ~StdioPrint() = default;

  // See: https://en.cppreference.com/w/cpp/language/rule_of_three
  StdioPrint(const StdioPrint&) = delete;
  StdioPrint& operator=(const StdioPrint&) = delete;

  size_t write(uint8_t b) override;
  size_t write(const uint8_t* buffer, size_t size) override;
  int availableForWrite() override;
  void flush() override;

 protected:
  StdioPrint(StdioPrint&&) noexcept = default;
  StdioPrint& operator=(StdioPrint&&) noexcept = default;

  std::FILE* stream() const {
    return stream_;
  }

 private:
  // Checks and clears any error because clearWriteError() is not overridable.
  void checkAndClearErr();

  std::FILE* stream_;
};

// NullPrint is a Print object that accepts all writes and sends them nowhere.
class NullPrint final : public PrintBase {
 public:
  NullPrint() = default;
  virtual ~NullPrint() = default;

  // Rule of five
  NullPrint(const NullPrint&) = delete;
  NullPrint(NullPrint&&) = delete;
  NullPrint& operator=(const NullPrint&) = delete;
  NullPrint& operator=(NullPrint&&) = delete;

  size_t write(const uint8_t b) override {
    LWIP_UNUSED_ARG(b);
    return 1;
  }

  size_t write(const uint8_t* const buffer, const size_t size) override {
    LWIP_UNUSED_ARG(buffer);
    return size;
  }

  int availableForWrite() override {
    return std::numeric_limits<int>::max();
  }

  void flush() override {
  }
};

// PrintDecorator is a Print object that decorates another. This is meant to be
// a base class. This also guarantees calls to printf() are checked.
class PrintDecorator : public PrintBase {
 public:
  explicit PrintDecorator(Print& p) : p_(p) {}
  virtual ~PrintDecorator() = default;

  // Rule of five
  PrintDecorator(const PrintDecorator&) = delete;
  PrintDecorator(PrintDecorator&&) = delete;
  PrintDecorator& operator=(const PrintDecorator&) = delete;
  PrintDecorator& operator=(PrintDecorator&&) = delete;

  size_t write(const uint8_t b) override {
    return p_.write(b);
  }

  size_t write(const uint8_t* const buffer, const size_t size) override {
    return p_.write(buffer, size);
  }

  int availableForWrite() override {
    return p_.availableForWrite();
  }

  void flush() override {
    p_.flush();
  }

 private:
  Print& p_;
};

// StreamDecorator is a Stream object that decorates another. This is meant to
// be a base class. This also guarantees calls to printf() are checked.
class StreamDecorator : public StreamBase {
 public:
  explicit StreamDecorator(Stream& s) : s_(s) {}
  virtual ~StreamDecorator() = default;

  // Rule of five
  StreamDecorator(const StreamDecorator&) = delete;
  StreamDecorator(StreamDecorator&&) = delete;
  StreamDecorator& operator=(const StreamDecorator&) = delete;
  StreamDecorator& operator=(StreamDecorator&&) = delete;

  int available() override {
    return s_.available();
  }

  int read() override {
    return s_.read();
  }

  int peek() override {
    return  s_.peek();
  }

  size_t write(const uint8_t b) override {
    return s_.write(b);
  }

  size_t write(const uint8_t* const buffer, const size_t size) override {
    return s_.write(buffer, size);
  }

  int availableForWrite() override {
    return s_.availableForWrite();
  }

  void flush() override {
    s_.flush();
  }

 private:
  Stream& s_;
};

}  // namespace util
}  // namespace network
}  // namespace qindesign
