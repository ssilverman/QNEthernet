// SPDX-FileCopyrightText: (c) 2024 Shawn Silverman <shawn@pobox.com>
// SPDX-License-Identifier: AGPL-3.0-or-later

// MbedTLSClient.h defines a Client wrapper for TLS connections that uses the
// MbedTLS library.
// This file is part of the QNEthernet library.

#pragma once

#include <vector>

#include <Client.h>
#include <mbedtls/ctr_drbg.h>
#include <mbedtls/ssl.h>

namespace qindesign {
namespace network {

class MbedTLSClient : public Client {
 public:
  MbedTLSClient(Client &client);
  virtual ~MbedTLSClient() = default;

  // Sets the CA cert. This only uses the value if it is non-NULL and the length
  // is positive. The pointer and length are stored.
  void setCACert(const uint8_t *caCert, size_t caCertLen);

  // Sets the pre-shared key. This only uses the value if both buffers are
  // non-NULL and the lengths are both positive. The pointers and lengths
  // are stored.
  void setPSK(const uint8_t *psk, size_t pskLen,
               const uint8_t *pskId, size_t pskIdLen);

  // Sets the client certificate and key. This only uses the value if both
  // buffers are non-NULL and the lengths are both positive. The pointers and
  // lengths are stored.
  void setClientCert(const uint8_t *clientCert, size_t clientCertLen,
                     const uint8_t *clientKey, size_t clientKeyLen);

  // Sets the handshake timeout, in milliseconds. The default is zero, meaning
  // "wait forever".
  void setHandshakeTimeout(uint32_t timeout);

  int connect(IPAddress ip, uint16_t port) final;
  int connect(const char *host, uint16_t port) final;
  size_t write(uint8_t b) final;
  size_t write(const uint8_t *buf, size_t size) final;
  int available() final;
  int read() final;
  int read(uint8_t *buf, size_t size) final;
  int peek() final;
  int availableForWrite() final;
  void flush() final;
  void stop() final;
  uint8_t connected() final;
  explicit operator bool() final;

  explicit operator bool() const;

 private:
  // Initializes the client.
  bool init();

  // Uninitializes the client.
  void deinit();

  // Connects to the given host. This performs the handshake step.
  bool connect(const char *hostname);

  // Checks the value returned from mbedtls_ssl_read(). If this returns false
  // then stop() will have been called.
  bool checkRead(int ret);

  // Checks the value returned from mbedtls_ssl_write(). If this returns false
  // then stop() will have been called.
  bool checkWrite(int ret);

  Client &client_;
  uint32_t handshakeTimeout_ = 0;

  int peeked_ = -1;  // < 0 for not there
  bool connected_ = false;

  // State
  mbedtls_ssl_context ssl_{};
  mbedtls_ssl_config conf_{};
  mbedtls_x509_crt caCert_{};
  mbedtls_x509_crt clientCert_{};
  mbedtls_pk_context clientKey_{};

  // Certificates and keys
  const uint8_t *caCertBuf_ = nullptr;
  size_t caCertLen_ = 0;
  const uint8_t *clientCertBuf_ = nullptr;
  size_t clientCertLen_ = 0;

  // Keys
  const uint8_t *psk_ = nullptr;
  size_t pskLen_ = 0;
  const uint8_t *pskId_ = nullptr;
  size_t pskIdLen_ = 0;
  const uint8_t *clientKeyBuf_ = nullptr;
  size_t clientKeyLen_ = 0;
};

}  // namespace network
}  // namespace qindesign
