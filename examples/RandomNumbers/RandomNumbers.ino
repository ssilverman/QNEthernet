// SPDX-FileCopyrightText: (c) 2023 Shawn Silverman <shawn@pobox.com>
// SPDX-License-Identifier: AGPL-3.0-or-later

// RandomNumbers demonstrates how to use random number distributions
// and engines using the included entropy functions.
//
// See also:
// * https://en.cppreference.com/w/cpp/numeric/random#Random_number_distributions
// * https://en.cppreference.com/w/cpp/named_req/UniformRandomBitGenerator
// * https://en.cppreference.com/w/cpp/named_req/RandomNumberEngine
//
// This file is part of the QNEthernet library.

#if TEENSYDUINO <= 158
// Undefine Arduino nonsense so <random> can be included
#undef round
#undef abs
#endif  // TEENSYDUINO <= 158

#include <random>

#include <QNEthernet.h>

// The 'RandomDevice' class is in this namespace. Note that you could
// also directly qualify the class and not use this directive.
//
// The object satisfies the UniformRandomBitGenerator C++
// named requirement.
using namespace qindesign::security;

// Some distributions included with the standard library;
// there's many more
std::uniform_int_distribution<int> diceDist(1, 6);  // Inclusive
std::uniform_real_distribution<float> realDist(0.0f, 1.0f);  // [0, 1)
std::normal_distribution<float> normalDist(0.0f, 1.0f);  // mean=0 stddev=1

// Linear congruential generator (LCG) engine.
std::minstd_rand randomEngine{RandomDevice::instance()()};

// Program setup.
void setup() {
  Serial.begin(115200);
  while (!Serial && millis() < 4000) {
    // Wait for Serial
  }

  printf("[Hardware Entropy]\r\n");
  demo(RandomDevice::instance());

  printf("\r\n");

  printf("[Pseudo-Random]\r\n");
  demo(randomEngine);
}

// Generates numbers from a variety of distributions using the given
// UniformRandomBitGenerator, G.
template <typename G>
void demo(G &g) {
  printf("Raw numbers:");
  for (int i = 0; i < 20; i++) {
    if (i % 5 == 0) {
      printf("\r\n");
    }
    printf(" %10lu", (unsigned long){g()});
  }
  printf("\r\n");

  // Fill dice buckets with zero
  int buckets[6]{};

  printf("Dice rolls:");
  for (int i = 0; i < 100; i++) {
    if (i % 20 == 0) {
      printf("\r\n");
    }
    int roll = diceDist(g);
    printf(" %d", roll);
    buckets[roll - 1]++;
  }
  printf("\r\n");

  // Print the dice roll counts
  printf("Dice counts:");
  for (int i = 0; i < 6; i++) {
    printf(" [%d]=%d", i + 1, buckets[i]);
  }
  printf("\r\n");

  printf("Reals:");
  for (int i = 0; i < 20; i++) {
    if (i % 5 == 0) {
      printf("\r\n");
    }
    printf(" % f", realDist(g));
  }
  printf("\r\n");

  printf("Normals:");
  for (int i = 0; i < 20; i++) {
    if (i % 5 == 0) {
      printf("\r\n");
    }
    printf(" % f", normalDist(g));
  }
  printf("\r\n");
}

// Program loop.
void loop() {
}
