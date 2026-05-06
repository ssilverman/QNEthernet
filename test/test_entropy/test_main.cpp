// SPDX-FileCopyrightText: (c) 2023-2026 Shawn Silverman <shawn@pobox.com>
// SPDX-License-Identifier: AGPL-3.0-or-later

// test_main.cpp tests the entropy functions.
// This file is part of the QNEthernet library.

// C++ includes
#include <array>
#include <cerrno>
#include <climits>
#include <cmath>
#include <cstdint>
#include <cstdio>
#include <limits>
#include <string>
#include <utility>
#include <vector>

#include <Arduino.h>
#undef F  /* Undefine some Arduino nonsense */
#include <boost/math/distributions/chi_squared.hpp>
#include <qnethernet/entropy/entropy.h>
#include <qnethernet/entropy/random_device.h>
#include <unity.h>

#include "util/OnlineSerialCorrelation.h"

// --------------------------------------------------------------------------
//  Utilities
// --------------------------------------------------------------------------

// Formats into a char vector and returns the vector. The vector will always be
// terminated with a NUL. This returns an empty string if something goes wrong
// with the print function.
template <typename... Args>
static std::vector<char> format(const char* format, Args... args) {
  std::vector<char> out;

  int size = std::snprintf(nullptr, 0, format, args...) + 1;  // Include the NUL
  if (size <= 1) {
    out.resize(1, 0);
  } else {
    out.resize(size);
    TEST_ASSERT_EQUAL_MESSAGE(size - 1,
                              std::snprintf(out.data(), size, format, args...),
                              "Expected complete string fill");
  }
  return out;
}

// --------------------------------------------------------------------------
//  Main Program
// --------------------------------------------------------------------------

// Pre-test setup. This is run before every test.
void setUp() {
}

// Post-test teardown. This is run after every test.
void tearDown() {
}

#if !QNETHERNET_USE_ENTROPY_LIB && TEENSYDUINO && __IMXRT1062__
// Tests that the feature is active.
static void test_active() {
  TEST_ASSERT_TRUE_MESSAGE(::qindesign::entropy::trng_is_started(),
                           "Expected started");
}

// Tests that the feature is inactive.
static void test_inactive() {
  TEST_ASSERT_FALSE_MESSAGE(::qindesign::entropy::trng_is_started(),
                            "Expected not started");
}

// Tests data-available.
static void test_available() {
  // Assume we're going to read full entropy
  delay(100);  // Give it time to collect entropy
  TEST_ASSERT_EQUAL_MESSAGE(64, ::qindesign::entropy::trng_available(),
                            "Expected full entropy");
}

// Tests data access.
static void test_data() {
  uint8_t b[64];
  TEST_ASSERT_EQUAL_MESSAGE(1, ::qindesign::entropy::trng_data(b, 1),
                            "Expected 1-byte read");
  TEST_ASSERT_EQUAL_MESSAGE(63, ::qindesign::entropy::trng_available(),
                            "Expected full entropy");
  TEST_ASSERT_EQUAL_MESSAGE(63, ::qindesign::entropy::trng_data(&b[1], 63),
                            "Expected 63-byte read");
  TEST_ASSERT_EQUAL_MESSAGE(0, ::qindesign::entropy::trng_available(),
                            "Expected empty entropy");
}

// Tests entropy_random().
static void test_random() {
  uint32_t r;
  errno = 0;
  TEST_ASSERT_TRUE_MESSAGE(::qindesign::entropy::entropy_random(&r),
                           "Expected no error");
  TEST_ASSERT_EQUAL_MESSAGE(0, errno, "Expected no errno");
}

// Tests entropy_random_range(range).
static void test_random_range() {
  uint32_t r;

  errno = 0;
  TEST_ASSERT_FALSE_MESSAGE(::qindesign::entropy::entropy_random_range(0, &r),
                            "Expected error");
  TEST_ASSERT_EQUAL_MESSAGE(EDOM, errno, "Expected EDOM");

  errno = 0;
  TEST_ASSERT_TRUE_MESSAGE(::qindesign::entropy::entropy_random_range(1, &r),
                           "Expected no error");
  TEST_ASSERT_EQUAL_MESSAGE(0, r, "Expected zero");
  TEST_ASSERT_EQUAL_MESSAGE(0, errno, "Expected no errno");

  for (int i = 0; i < (1 << 10); ++i) {
    TEST_ASSERT_TRUE_MESSAGE(::qindesign::entropy::entropy_random_range(10, &r),
                             "Expected no error");
    TEST_ASSERT_EQUAL_MESSAGE(0, errno, "Expected no errno");
    if (r >= 10) {
      const auto msg = "Expected value < 10: iteration " + std::to_string(i);
      TEST_FAIL_MESSAGE(msg.c_str());
    }
  }
}
#endif  // !QNETHERNET_USE_ENTROPY_LIB && TEENSYDUINO && __IMXRT1062__

// Tests random_device.
static void test_random_device() {
  qindesign::entropy::random_device rd;

  TEST_ASSERT_EQUAL_MESSAGE(0, qindesign::entropy::random_device::min(),
                            "Expected full-range minimum");
  TEST_ASSERT_EQUAL_MESSAGE(std::numeric_limits<unsigned int>::max(),
                            qindesign::entropy::random_device::max(),
                            "Expected full-range maximum");
  TEST_ASSERT_EQUAL_MESSAGE(sizeof(unsigned int) * CHAR_BIT, rd.entropy(),
                            "Expected full entropy");

  errno = 0;
  for (int i = 0; i < (1 << 10); ++i) {
    (void)rd();
    TEST_ASSERT_EQUAL_MESSAGE(0, errno, "Expected no error");
  }
}

#if TEENSYDUINO && __IMXRT1062__

// Creates an array of online serial correlation objects having
// consecutive lags.
template <size_t... I>
static constexpr auto make_corrs(std::index_sequence<I...>) {
  return std::array{util::OnlineSerialCorrelation<uint8_t>{I + 1}...};
}

// Tests the quality of the entropy by performing some basic tests.
// These include:
// 1. Entropy > 7.9
// 2. Mean between 127 and 128
// 3. Chi-squared p-value > 0.10
// 4. Various lagged autocorrelations are all < 0.2
//
// See:
// * https://www.fourmilab.ch/random/
// * https://github.com/Fourmilab/ent_random_sequence_tester
static void test_randomness() {
  static constexpr size_t kBufSize = 64;
  static constexpr size_t kFillCount = 2*1024;  // 64-byte fills
  static constexpr size_t kLagCount = 5;
  static constexpr size_t kTotalCount = kFillCount * kBufSize;

  // Progress bar in increments of 10%.
  static constexpr char kBar[]{"==========          "};

  qindesign::entropy::random_device rd;
  uint8_t buf[kBufSize];
  uint32_t counts[256]{0};
  uint32_t totalCount = 0;
  double p[256];  // Probabilities

  // Use different lags
  auto corrs = make_corrs(std::make_index_sequence<kLagCount>());

  TEST_MESSAGE(
      format("Starting randomness test: %zu bytes", kTotalCount).data());

  TEST_MESSAGE(format("[%s]  0%%", kBar + 10).data());
  uint32_t t = millis();
  for (size_t i = 0; i < kFillCount; ++i) {
    for (size_t j = 1; j <= 9; ++j) {
      if (i == j * kFillCount / 10) {
        TEST_MESSAGE(format("[%.*s] %zu%%", 10, kBar + 10 - j, j*10).data());
      }
    }

    rd(buf, kBufSize);  // Shouldn't throw
    for (size_t j = 0; j < kBufSize; ++j) {
      ++counts[buf[j]];
      ++totalCount;

      for (auto& corr : corrs) {
        corr.sample(buf[j]);
      }
    }
  }
  t = millis() - t;
  TEST_MESSAGE(format("[%.*s]100%%", 10, kBar).data());
  TEST_MESSAGE(
      format("Generated %zu bytes in %" PRIu32 " ms (%f bps)", totalCount, t,
             static_cast<double>(totalCount) * 1000.0 / static_cast<double>(t))
          .data());

  const double expectedCount = totalCount / 256.0;
  double chiSq   = 0.0;
  double sum     = 0.0;
  double entropy = 0.0;

  for (size_t i = 0; i < 256; i++) {
    const double a = counts[i] - expectedCount;

    p[i] = static_cast<double>(counts[i]) / static_cast<double>(totalCount);
    if (p[i] != 0.0) {
      entropy += p[i] * -std::log2(p[i]);
    }

    chiSq += (a * a) / expectedCount;
    sum += static_cast<double>(i) * counts[i];
  }

  const double mean = sum / totalCount;

  static const boost::math::chi_squared_distribution<double> dist(255);
  double pVal = 1.0 - boost::math::cdf(dist, chiSq);

  TEST_MESSAGE(format("entropy = %f", entropy).data());
  TEST_MESSAGE(format("mean    = %f", mean).data());
  TEST_MESSAGE(format("chi-sq  = %f", chiSq).data());
  TEST_MESSAGE(format("p-val   = %f", pVal).data());
  for (const auto& corr : corrs) {
    TEST_MESSAGE(
        format("corr_%zu  = %+f", corr.lag(), corr.correlation()).data());
  }

  // Tests
  TEST_ASSERT_GREATER_THAN_DOUBLE(7.9, entropy);
  TEST_ASSERT_DOUBLE_WITHIN(0.75, 127.5, mean);
  TEST_ASSERT_GREATER_THAN_DOUBLE(0.10, pVal);
  for (const auto& corr : corrs) {
    TEST_ASSERT_LESS_THAN_DOUBLE(0.2, std::abs(corr.correlation()));
  }
}

#endif  // TEENSYDUINO && __IMXRT1062__

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
#if !QNETHERNET_USE_ENTROPY_LIB && TEENSYDUINO && __IMXRT1062__
  RUN_TEST(test_inactive);
  ::qindesign::entropy::trng_init();
  RUN_TEST(test_active);
  RUN_TEST(test_available);
  RUN_TEST(test_data);
  RUN_TEST(test_random);
  RUN_TEST(test_random_range);
#endif  // !QNETHERNET_USE_ENTROPY_LIB && TEENSYDUINO && __IMXRT1062__
  RUN_TEST(test_random_device);
#if TEENSYDUINO && __IMXRT1062__
  RUN_TEST(test_randomness);
#endif  // TEENSYDUINO && __IMXRT1062__
#if !QNETHERNET_USE_ENTROPY_LIB && TEENSYDUINO && __IMXRT1062__
  ::qindesign::entropy::trng_deinit();
  RUN_TEST(test_inactive);
#endif  // !QNETHERNET_USE_ENTROPY_LIB && TEENSYDUINO && __IMXRT1062__
  UNITY_END();
}

// Main program loop.
void loop() {
}
