// SPDX-FileCopyrightText: (c) 2024 Shawn Silverman <shawn@pobox.com>
// SPDX-License-Identifier: AGPL-3.0-or-later

// MbedTLSServer.h defines a Server wrapper for TLS connections that uses the
// MbedTLS library.
// This file is part of the QNEthernet library.

#pragma once

// C++ includes
#include <cstdint>
#include <functional>
#include <vector>

#include <Server.h>
#include <mbedtls/ssl.h>

#include "MbedTLSClient.h"
#include "QNEthernetServer.h"

namespace qindesign {
namespace network {

class MbedTLSServer : public Server {
 public:
  // Callback function for processing a client PSK request.
  using pskf = std::function<void(
      const unsigned char *id, size_t idLen,
      std::function<void(const unsigned char *psk, size_t psk_len)> psk)>;

  MbedTLSServer(EthernetServer &Server);
  virtual ~MbedTLSServer();

  // Allow moving but not copying
  MbedTLSServer(const MbedTLSServer &) = delete;
  MbedTLSServer &operator=(const MbedTLSServer &) = delete;
  MbedTLSServer(MbedTLSServer &&other) = default;
  MbedTLSServer &operator=(MbedTLSServer &&other) = default;

  void begin() final;
  void end();

  // Sets the CA cert(s). This only uses the value if it is non-NULL and the
  // length is positive. The pointer and length are stored.
  //
  // If it is in PEM format then it must be NUL-terminated.
  void setCACert(const uint8_t *buf, size_t len);

  // Adds a server certificate. This does not add it if the cert or key don't
  // have content. The password is optional.
  //
  // If the certificate or key is in PEM format, then it must be NUL-terminated.
  void addServerCert(const uint8_t *cert, size_t certLen,
                     const uint8_t *key, size_t keyLen,
                     const uint8_t *keyPwd, size_t keyPwdLen);

  // Sets the callback for processing a PSK request from the client.
  void onPSK(pskf f);

  MbedTLSClient accept();
  // MbedTLSClient available() const;

  // Defined by Print; these do nothing or return 0
  size_t write(uint8_t b) override;
  size_t write(const uint8_t *buffer, size_t size) override;
  int availableForWrite()	override;
  void flush() override;

  explicit operator bool() const;

 private:
  enum class States {
    kStart,
    kStarted,
  };

  // Cert holds pointers to certificate data.
  struct Cert {
    const uint8_t *cert = nullptr;
    size_t certLen = 0;
    const uint8_t *key = nullptr;
    size_t keyLen = 0;
    const uint8_t *keyPwd = nullptr;
    size_t keyPwdLen = 0;

    // Returns whether there is valid cert and key data.
    bool valid() const;

    // Clears the internal values.
    void clear();
  };

  // Cert holds a pointer to CA certificate data.
  struct CACert {
    const uint8_t *buf = nullptr;
    size_t len = 0;

    // Returns whether the data is valid.
    bool valid() const;

    // Clears the internal values.
    void clear();
  };

  EthernetServer &server_;
  States state_;

  CACert caCert_;
  std::vector<Cert> certs_;
  pskf pskCB_;
};

}  // namespace network
}  // namespace qindesign
