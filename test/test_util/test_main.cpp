// SPDX-FileCopyrightText: (c) 2025-2026 Shawn Silverman <shawn@pobox.com>
// SPDX-License-Identifier: AGPL-3.0-or-later

// test_main.cpp tests the utility functions.
// This file is part of the QNEthernet library.

// C includes
#include <fcntl.h>
#include <unistd.h>

// C++ includes
#include <cerrno>
#include <cerrno>
#include <chrono>
#include <cstddef>
#include <cstdint>
#include <cstdio>
#include <limits>
#include <string>

#include <Arduino.h>
#include <Print.h>
#include <unity.h>

#include "lwip/def.h"
#include "lwip/ip4_addr.h"
#include "qnethernet/compat/c++11_compat.h"
#include "qnethernet/util/PrintUtils.h"
#include "qnethernet/util/chrono_clocks.h"
#include "qnethernet/util/elapsedTime.h"
#include "qnethernet/util/ip_tools.h"

using namespace qindesign::network::util;
using namespace std::chrono_literals;

using steady_clock = steady_clock_ms;

// --------------------------------------------------------------------------
//  Utilities
// --------------------------------------------------------------------------

// Treats a std::string as a Print.
class StringPrint final : public Print {
 public:
  explicit StringPrint(std::string& s) : s_(s) {}
  virtual ~StringPrint() = default;

  size_t write(const uint8_t b) override {
    s_ += static_cast<char>(b);
    return 1;
  }

  size_t write(const uint8_t* const buffer, const size_t size) override {
    (void)s_.append(reinterpret_cast<const char*>(buffer), size);
    return size;
  }

  int availableForWrite() {
    static_assert(sizeof(size_t) >= sizeof(int),
                  "size_t must be at least as big as int, for casting");
    return std::min(s_.max_size() - s_.size(),
                    static_cast<size_t>(std::numeric_limits<int>::max()));
  }

 private:
  std::string& s_;
};

// --------------------------------------------------------------------------
//  Program Variables
// --------------------------------------------------------------------------

// Output string for various tests
static std::string s_outputString;
static bool s_outputIsOpen = false;  // For stdio

// --------------------------------------------------------------------------
//  Main Program
// --------------------------------------------------------------------------

// Pre-test setup. This is run before every test.
void setUp() {
}

// Post-test teardown. This is run after every test.
void tearDown() {
}

// Tests writing a magic packet.
static void test_writeMagic() {
  s_outputString.clear();
  StringPrint sp{s_outputString};
  const uint8_t mac[6]{'1', '2', '3', '4', '5', '6'};
  TEST_ASSERT_EQUAL(17*ETH_HWADDR_LEN, writeMagic(sp, mac));
  TEST_ASSERT_EQUAL(
      "\xff\xff\xff\xff\xff\xff"
      "123456123456123456123456123456123456123456123456"
      "123456123456123456123456123456123456123456123456",
      s_outputString);
}

// Tests the StdioPrint wrapper.
static void test_StdioPrint() {
  constexpr char kMessage[]{"This is a message."};

  std::FILE* f = fopen("file", "w");
  TEST_ASSERT_NOT_NULL_MESSAGE(f, "Could not open file");

  StdioPrint stdioPrint{f};
  (void)stdioPrint.print(kMessage);
  stdioPrint.flush();
  TEST_ASSERT_EQUAL_MESSAGE(kMessage, s_outputString, "Contents not equal");

  TEST_ASSERT_EQUAL_MESSAGE(0, fclose(f), "Error closing");
}

// Tests the NullPrint class.
static void test_NullPrint() {
  NullPrint np;
  TEST_ASSERT_EQUAL_MESSAGE(1, np.write(1), "Expected byte written");
  TEST_ASSERT_EQUAL_MESSAGE(12, np.write((void*)nullptr, 12), "Expected bytes written");
  TEST_ASSERT_EQUAL_MESSAGE(std::numeric_limits<int>::max(),
                            np.availableForWrite(),
                            "Expected max. bytes available to write");
}

// Makes a 32-bit IP address in network order.
ATTRIBUTE_NODISCARD
static inline constexpr uint32_t makeIP(uint8_t a, uint8_t b, uint8_t c,
                                        uint8_t d) {
  return PP_HTONL(LWIP_MAKEU32((a), (b), (c), (d)));
}

static void test_isBroadcast() {
  static constexpr uint32_t kLocalIP = makeIP(10, 0, 0, 59);
  static constexpr uint32_t kSubnet  = makeIP(255, 0, 0, 0);
  static constexpr uint32_t tests[][4]{
      {IPADDR_ANY,                kLocalIP, kSubnet, true},
      {IPADDR_BROADCAST,          kLocalIP, kSubnet, true},
      {makeIP(10, 255, 255, 255), kLocalIP, kSubnet, true},
      {makeIP(11, 255, 255, 255), kLocalIP, kSubnet, false},
      {makeIP(10, 0, 0, 2),       kLocalIP, kSubnet, false},
  };

  for (const uint32_t* test : tests) {
    TEST_ASSERT_EQUAL(test[3], qindesign::network::util::isBroadcast(
                                   test[0], test[1], test[2]));
  }
}

// Tests the elapsedTime<Clock> utility class comparison functions.
static void test_elapsedTime_compare() {
  elapsedTime<steady_clock> timer;

  delay(2000);
  auto x = std::chrono::duration_cast<std::chrono::seconds>(timer.dur());
  TEST_ASSERT_EQUAL(2, x.count());

  // Comparison operators

  TEST_ASSERT_TRUE(timer == 2s);
  TEST_ASSERT_TRUE(timer == 2000);
  TEST_ASSERT_TRUE(2s == timer);
  TEST_ASSERT_TRUE(2000 == timer);
  TEST_ASSERT_TRUE(timer != 1s);
  TEST_ASSERT_TRUE(timer != 1000);
  TEST_ASSERT_TRUE(1s != timer);
  TEST_ASSERT_TRUE(1000 != timer);
  TEST_ASSERT_TRUE(timer < 3s);
  TEST_ASSERT_TRUE(timer < 3000);
  TEST_ASSERT_TRUE(3s > timer);
  TEST_ASSERT_TRUE(3000 > timer);
  TEST_ASSERT_TRUE(timer > 1s);
  TEST_ASSERT_TRUE(timer > 1000);
  TEST_ASSERT_TRUE(1s < timer);
  TEST_ASSERT_TRUE(1000 < timer);
  TEST_ASSERT_TRUE(timer <= 3s);
  TEST_ASSERT_TRUE(timer <= 3000);
  TEST_ASSERT_TRUE(3s >= timer);
  TEST_ASSERT_TRUE(3000 >= timer);
  TEST_ASSERT_TRUE(timer >= 2s);
  TEST_ASSERT_TRUE(timer >= 2000);
  TEST_ASSERT_TRUE(2s <= timer);
  TEST_ASSERT_TRUE(2000 <= timer);

  TEST_ASSERT_TRUE(timer == 2000ms);
  TEST_ASSERT_TRUE(2000ms == timer);
  TEST_ASSERT_TRUE(timer != 1000ms);
  TEST_ASSERT_TRUE(1000ms != timer);
  TEST_ASSERT_TRUE(timer < 3000ms);
  TEST_ASSERT_TRUE(3000ms > timer);
  TEST_ASSERT_TRUE(timer > 1000ms);
  TEST_ASSERT_TRUE(1000ms < timer);
  TEST_ASSERT_TRUE(timer <= 3000ms);
  TEST_ASSERT_TRUE(3000ms >= timer);
  TEST_ASSERT_TRUE(timer >= 2000ms);
  TEST_ASSERT_TRUE(2000ms <= timer);

  TEST_ASSERT_TRUE(timer == 2.0s);
  TEST_ASSERT_TRUE(2.0s == timer);
  TEST_ASSERT_TRUE(timer != 1.9s);
  TEST_ASSERT_TRUE(1.9s != timer);
  TEST_ASSERT_TRUE(timer < 3.0s);
  TEST_ASSERT_TRUE(3.0s > timer);
  TEST_ASSERT_TRUE(timer > 1.9s);
  TEST_ASSERT_TRUE(1.9s < timer);
  TEST_ASSERT_TRUE(timer <= 3.0s);
  TEST_ASSERT_TRUE(3.0s >= timer);
  TEST_ASSERT_TRUE(timer >= 0.5s);
  TEST_ASSERT_TRUE(0.5s <= timer);
}

// Tests "other" things about the elapsedTime<Clock> utility class.
static void test_elapsedTime_other() {
  // Constructor (duration), assignment, and arithmetic
  elapsedTime<steady_clock> t1{2s};
  TEST_ASSERT_TRUE(t1 == 2s);
  t1 = 3s;
  TEST_ASSERT_TRUE(t1 == 3s);
  t1 += 500ms;
  TEST_ASSERT_TRUE(t1 == 3.5s);
  t1 -= 0.2s;
  TEST_ASSERT_TRUE(t1 == 3.3s);
  TEST_ASSERT_TRUE((t1 + 0.6s) == 3.9s);
  TEST_ASSERT_TRUE((t1 - 0.6s) == 2.7s);

  // Constructor (rep), assignment, and arithmetic
  elapsedTime<steady_clock> t2{2000};
  TEST_ASSERT_TRUE(t2 == 2s);
  t2 = 3000;
  TEST_ASSERT_TRUE(t2 == 3s);
  t2 += 500;
  TEST_ASSERT_TRUE(t2 == 3.5s);
  t2 -= 200;
  TEST_ASSERT_TRUE(t2 == 3.3s);
  TEST_ASSERT_TRUE((t2 + 600) == 3.9s);
  TEST_ASSERT_TRUE((t2 - 600) == 2.7s);
}

// Main program setup.
void setup() {
  Serial.begin(115200);
  while (!Serial && (millis() < 4000)) {
    // Wait for Serial
  }

  // NOTE!!! Wait for >2 secs
  // if board doesn't support software reset via Serial.DTR/RTS
  delay(2000);

#if defined(TEENSYDUINO)
  if (CrashReport) {
    (void)Serial.println(CrashReport);
  }
#endif  // defined(TEENSYDUINO)

  UNITY_BEGIN();
  RUN_TEST(test_writeMagic);
  RUN_TEST(test_StdioPrint);
  RUN_TEST(test_NullPrint);
  RUN_TEST(test_isBroadcast);
  RUN_TEST(test_elapsedTime_compare);
  RUN_TEST(test_elapsedTime_other);
  UNITY_END();
}

// Main program loop.
void loop() {
}

// --------------------------------------------------------------------------
//  Syscalls
// --------------------------------------------------------------------------

// The output string's file descriptor.
constexpr int kFD = 3;

extern "C" {

// Low-level stdio write for Newlib. This outputs stdout and stderr to Serial
// and FD 3 to the output string.
int _write(const int file, const void* const buf, const size_t len) {
  switch (file) {
    case STDOUT_FILENO:
      ATTRIBUTE_FALLTHROUGH;
    case STDERR_FILENO:
      return Serial.write(static_cast<const uint8_t*>(buf), len);
    case STDIN_FILENO:
      errno = EBADF;
      return -1;
    default:
      if ((file != kFD) || !s_outputIsOpen) {
        errno = EBADF;
        return -1;
      }
  }

  s_outputString.append(reinterpret_cast<const char*>(buf), len);
  return len;
}

// Opens any file for write-only and returns the file descriptor
// for s_outputString.
int _open(const char* const path, const int flags, const int mode) {
  if (((flags & O_CREAT) != 0) && ((flags & O_EXCL) != 0)) {
    errno = EEXIST;
    return -1;
  }
  if ((flags & (O_RDONLY | O_WRONLY | O_RDWR)) != O_WRONLY) {
    errno = EBADF;
    return -1;
  }

  if ((flags & O_TRUNC) != 0) {
    s_outputString.clear();
  }

  s_outputIsOpen = true;
  return kFD;
}

int _close(const int fd) {
  if ((fd != kFD) || !s_outputIsOpen) {
    errno = EBADF;
    return -1;
  }

  s_outputIsOpen = false;
  return 0;
}

}  // extern "C"
