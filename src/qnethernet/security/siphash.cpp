// SPDX-FileCopyrightText: (c) 2025-2026 Shawn Silverman <shawn@pobox.com>
// SPDX-License-Identifier: AGPL-3.0-or-later

// siphash.cpp implements the SipHash functions.
// See also: https://github.com/veorq/SipHash
// This file is part of the QNEthernet library.

#include "qnethernet/security/siphash.h"

#include "qnethernet/compat/c++11_compat.h"

// C++ includes
#include <cstring>

// Rotates 'x' left by 's' bits.
ATTRIBUTE_NODISCARD
static inline uint64_t rotl(const uint64_t x, const int s) {
  // int r = s % 64;
  return (x << s) | (x >> (64 - s));
}

// Performs one SIP round.
static inline void sipround(uint64_t* const v0, uint64_t* const v1,
                            uint64_t* const v2, uint64_t* const v3) {
  *v0 += *v1; *v1 = rotl(*v1, 13); *v1 ^= *v0; *v0 = rotl(*v0, 32);
  *v2 += *v3; *v3 = rotl(*v3, 16); *v3 ^= *v2;
  *v0 += *v3; *v3 = rotl(*v3, 21); *v3 ^= *v0;
  *v2 += *v1; *v1 = rotl(*v1, 17); *v1 ^= *v2; *v2 = rotl(*v2, 32);
}

extern "C" uint64_t siphash(const size_t c, const size_t d,
                            const void* const key,
                            const void* const msg, const size_t len) {
  uint64_t k0;
  uint64_t k1;
  (void)std::memcpy(&k0, key, 8);
  (void)std::memcpy(&k1, (uint8_t*)key + 8, 8);
#if __BYTE_ORDER__ == __ORDER_BIG_ENDIAN__
  k0 = __builtin_bswap64(k0);
  k1 = __builtin_bswap64(k1);
#endif  // Big-endian

  // Initialization
  uint64_t v0 = k0 ^ UINT64_C(0x736f6d6570736575);  // somepseu
  uint64_t v1 = k1 ^ UINT64_C(0x646f72616e646f6d);  // dorandom
  uint64_t v2 = k0 ^ UINT64_C(0x6c7967656e657261);  // lygenera
  uint64_t v3 = k1 ^ UINT64_C(0x7465646279746573);  // tedbytes

  const uint8_t* pMsg = static_cast<const uint8_t*>(msg);

  // Compression
  size_t count = (len + 8)/8 - 1;
  uint64_t m;
  while (count-- != 0) {
    (void)std::memcpy(&m, pMsg, 8);
#if __BYTE_ORDER__ == __ORDER_BIG_ENDIAN__
    m = __builtin_bswap64(m);
#endif  // Big-endian
    v3 ^= m;
    for (size_t i = 0; i < c; ++i) {
      sipround(&v0, &v1, &v2, &v3);
    }
    v0 ^= m;
    pMsg += 8;
  }

#if __BYTE_ORDER__ == __ORDER_LITTLE_ENDIAN__
  m = ((uint64_t)len << 56);
  (void)std::memcpy(&m, pMsg, len % 8);
#else
  m = len & 0xff;
  (void)memcpy(&m, pMsg, len % 8);
  m = __builtin_bswap64(m);
#endif  // Big-endian
  v3 ^= m;
  for (size_t i = 0; i < c; ++i) {
    sipround(&v0, &v1, &v2, &v3);
  }
  v0 ^= m;

  // Finalization
  v2 ^= 0xff;
  for (size_t i = 0; i < d; ++i) {
    sipround(&v0, &v1, &v2, &v3);
  }

  return v0 ^ v1 ^ v2 ^ v3;
}
