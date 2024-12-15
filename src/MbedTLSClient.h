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
  virtual ~MbedTLSClient();

  // Allow moving but not copying
  MbedTLSClient(const MbedTLSClient &) = delete;
  MbedTLSClient &operator=(const MbedTLSClient &) = delete;
  MbedTLSClient(MbedTLSClient &&other) = default;
  MbedTLSClient &operator=(MbedTLSClient &&other) = default;

  // Sets the CA cert(s). This only uses the value if it is non-NULL and the
  // length is positive. The pointer and length are stored.
  //
  // If it is in PEM format then it must be NUL-terminated.
  void setCACert(const uint8_t *caCert, size_t caCertLen);

  // Sets the pre-shared key. This only uses the value if both buffers are
  // non-NULL and the lengths are both positive. The pointers and lengths
  // are stored.
  void setPSK(const uint8_t *psk, size_t pskLen,
               const uint8_t *pskId, size_t pskIdLen);

  // Sets the client certificate, key, and key password. This only uses the
  // value if the first two buffers are non-NULL and the lengths are both
  // positive. The password can be NULL or have a length of zero. The pointers
  // and lengths are stored.
  //
  // If the cert or key is in PEM format, then it must be NUL-terminated. The
  // password can be NULL or its length zero if the key is not encrypted.
  void setClientCert(const uint8_t *clientCert, size_t clientCertLen,
                     const uint8_t *clientKey, size_t clientKeyLen,
                     const uint8_t *pwd, size_t pwdLen);

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

 private:
  enum class States {
    kStart,
    kInitialized,
    kHandshake,
    kConnected,
  };

  // Initializes the client.
  bool init();

  // Uninitializes the client.
  void deinit();

  // Connects to the given host and optionally waits. This performs the
  // handshake step.
  bool connect(const char *hostname, bool wait);

  // If we're in the middle of a handshake then this moves the handshake along.
  // If the handshake is complete then this sets the state to Connected.
  //
  // This returns true if the handshake is still in flight or complete, and
  // false on error. If there was an error then deinit() will be called.
  //
  // This assumes that we're in the Handshake state.
  bool watchHandshake();

  // Checks the value returned from mbedtls_ssl_read(). If this returns false
  // then stop() will have been called.
  bool checkRead(int ret);

  // Checks the value returned from mbedtls_ssl_write(). If this returns false
  // then stop() will have been called.
  bool checkWrite(int ret);

  Client &client_;
  uint32_t handshakeTimeout_;

  States state_;

  int peeked_;  // < 0 for not there

  // State
  mbedtls_ssl_context ssl_;
  mbedtls_ssl_config conf_;
  mbedtls_x509_crt caCert_;
  mbedtls_x509_crt clientCert_;
  mbedtls_pk_context clientKey_;

  // Certificates
  const uint8_t *caCertBuf_;
  size_t caCertLen_;
  const uint8_t *clientCertBuf_;
  size_t clientCertLen_;

  // Keys
  const uint8_t *psk_;
  size_t pskLen_;
  const uint8_t *pskId_;
  size_t pskIdLen_;
  const uint8_t *clientKeyBuf_;
  size_t clientKeyLen_ = 0;
  const uint8_t *clientKeyPwd_;
  size_t clientKeyPwdLen_;
};

}  // namespace network
}  // namespace qindesign
