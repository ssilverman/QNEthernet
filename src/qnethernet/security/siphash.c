// SPDX-FileCopyrightText: (c) 2025 Shawn Silverman <shawn@pobox.com>
// SPDX-License-Identifier: AGPL-3.0-or-later

// siphash.c implements the SipHash functions.
// See also: https://github.com/veorq/SipHash
// This file is part of the QNEthernet library.

#include "qnethernet/security/siphash.h"

#include <string.h>

// Rotates 'x' left by 's' bits.
static inline uint64_t rotl(const uint64_t x, const int s) {
  // int r = s % 64;
  return (x << s) | (x >> (64 - s));
}

#define SIPROUND                                                            \
  (v0) += (v1); (v1) = rotl((v1), 13); (v1) ^= (v0); (v0) = rotl((v0), 32); \
  (v2) += (v3); (v3) = rotl((v3), 16); (v3) ^= (v2);                        \
  (v0) += (v3); (v3) = rotl((v3), 21); (v3) ^= (v0);                        \
  (v2) += (v1); (v1) = rotl((v1), 17); (v1) ^= (v2); (v2) = rotl((v2), 32)

uint64_t siphash(const size_t c, const size_t d,
                 const void *const key,
                 const void *const msg, const size_t len) {
  uint64_t k0;
  uint64_t k1;
  memcpy(&k0, key, 8);
  memcpy(&k1, (uint8_t *)key + 8, 8);
#if __BYTE_ORDER__ == __ORDER_BIG_ENDIAN__
  k0 = __builtin_bswap64(k0);
  k1 = __builtin_bswap64(k1);
#endif  // Big-endian

  // Initialization
  uint64_t v0 = k0 ^ UINT64_C(0x736f6d6570736575);  // somepseu
  uint64_t v1 = k1 ^ UINT64_C(0x646f72616e646f6d);  // dorandom
  uint64_t v2 = k0 ^ UINT64_C(0x6c7967656e657261);  // lygenera
  uint64_t v3 = k1 ^ UINT64_C(0x7465646279746573);  // tedbytes

  const uint8_t *pMsg = msg;

  // Compression
  size_t count = (len + 8)/8 - 1;
  uint64_t m;
  while (count-- != 0) {
    memcpy(&m, pMsg, 8);
#if __BYTE_ORDER__ == __ORDER_BIG_ENDIAN__
    m = __builtin_bswap64(m);
#endif  // Big-endian
    v3 ^= m;
    for (size_t i = 0; i < c; i++) {
      SIPROUND;
    }
    v0 ^= m;
    pMsg += 8;
  }

#if __BYTE_ORDER__ == __ORDER_LITTLE_ENDIAN__
  m = ((uint64_t)len << 56);
  memcpy(&m, pMsg, len % 8);
#else
  m = len & 0xff;
  memcpy(&m, pMsg, len % 8);
  m = __builtin_bswap64(m);
#endif  // Big-endian
  v3 ^= m;
  for (size_t i = 0; i < c; i++) {
    SIPROUND;
  }
  v0 ^= m;

  // Finalization
  v2 ^= 0xff;
  for (size_t i = 0; i < d; i++) {
    SIPROUND;
  }

  return v0 ^ v1 ^ v2 ^ v3;
}
