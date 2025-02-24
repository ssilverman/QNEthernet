// SPDX-FileCopyrightText: (c) 2024-2025 Shawn Silverman <shawn@pobox.com>
// SPDX-License-Identifier: AGPL-3.0-or-later

// MbedTLSCert.cpp implements MbedTLSCert.
// This file is part of the QNEthernet library.

#include "qnethernet/security/MbedTLSCert.h"

#include "qnethernet/security/mbedtls_funcs.h"

namespace qindesign {
namespace security {

MbedTLSCert::MbedTLSCert()
    : hasCerts_(false),
      hasKey_(false),
      cert_{} {
  mbedtls_x509_crt_init(&cert_);
  mbedtls_pk_init(&key_);
}

MbedTLSCert::~MbedTLSCert() {
  mbedtls_pk_free(&key_);
  mbedtls_x509_crt_free(&cert_);
}

bool MbedTLSCert::parse(const void *const buf, const size_t len) {
  if (buf == nullptr) {
    return false;
  }
  int err =
      mbedtls_x509_crt_parse(&cert_, static_cast<const uint8_t *>(buf), len);
  if (err >= 0) {
    hasCerts_ = true;
    return (err == 0);
  }
  return false;
}

bool MbedTLSCert::parseKey(const void *const buf, const size_t len,
                           const void *const pwd, const size_t pwdLen) {
  hasKey_ = (mbedtls_pk_parse_key(&key_, static_cast<const uint8_t *>(buf), len,
                                  static_cast<const uint8_t *>(pwd), pwdLen,
                                  qnethernet_mbedtls_rand_f_rng,
                                  qnethernet_mbedtls_rand_p_rng) == 0);
  return hasKey_;
}

size_t MbedTLSCert::size() const {
  if (!hasCerts_) {
    return 0;
  }

  auto *p = &cert_;
  size_t size = 0;
  while (p != nullptr) {
    size++;
    p = p->next;
  }
  return size;
}

bool MbedTLSCert::empty() const {
  return (size() == 0);
}

}  // namespace security
}  // namespace qindesign
