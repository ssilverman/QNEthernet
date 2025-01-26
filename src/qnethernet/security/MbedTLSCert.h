// SPDX-FileCopyrightText: (c) 2024 Shawn Silverman <shawn@pobox.com>
// SPDX-License-Identifier: AGPL-3.0-or-later

// MbedTLSCert.h defines an object that holds an MbedTLS certificate.
// This file is part of the QNEthernet library.

// C++ includes
#include <cstdint>

#include <mbedtls/ssl.h>

namespace qindesign {
namespace security {

class MbedTLSCert {
 public:
  MbedTLSCert();
  ~MbedTLSCert();

  // Allow moving but not copying
  MbedTLSCert(const MbedTLSCert &) = delete;
  MbedTLSCert &operator=(const MbedTLSCert &) = delete;
  MbedTLSCert(MbedTLSCert &&other) = default;
  MbedTLSCert &operator=(MbedTLSCert &&other) = default;

  // Parses a one DER-encoded certificate or a list of PEM-encoded certificates.
  // PEM-encoded data must be NUL-terminated.
  //
  // The certificates are added to the chain.
  //
  // This returns whether the parse was successful. If the parse was not
  // successful and the input is more than one PEM-encoded certificate then
  // some of the certificates may have been loaded. See size().
  //
  // This does nothing and returns false if the buffer is NULL.
  bool parse(const uint8_t *buf, size_t len);

  // Parses a key. If the key is PEM-encoded, then it must be NUL-terminated and
  // the NUL terminator must be included in the count. The password may be NULL.
  //
  // Client and server certificates should have a key.
  bool parseKey(const uint8_t *buf, size_t len,
                const uint8_t *pwd, size_t pwdLen);

  // Returns a reference to the internal certificate object.
  mbedtls_x509_crt &cert() {
    return cert_;
  }

  // Returns a reference to the internal private key object.
  mbedtls_pk_context &key() {
    return key_;
  }

  // Returns the number of certificates in the chain.
  size_t size() const;

  // Returns whether the size is zero.
  bool empty() const;

  // Returns whether this certificate has an attached key.
  bool hasKey() const {
    return hasKey_;
  }

 private:
  bool hasCerts_;
  bool hasKey_;

  mbedtls_x509_crt cert_;
  mbedtls_pk_context key_;
};

}  // namespace security
}  // namespace qindesign
