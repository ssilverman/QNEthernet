// SPDX-FileCopyrightText: (c) 2024 Shawn Silverman <shawn@pobox.com>
// SPDX-License-Identifier: AGPL-3.0-or-later

// MbedTLSClient.h defines a Client wrapper for TLS connections that uses the
// MbedTLS library.
// This file is part of the QNEthernet library.

#pragma once

// C++ includes
#include <utility>
#include <vector>

#include <Client.h>
#include <mbedtls/ssl.h>

#include "util/Span.h"

namespace qindesign {
namespace network {

// Any data pointers are no longer needed after the handshake completes, unless
// a new connection needs the same data.
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
  void setCACert(const uint8_t *buf, size_t len);

  // Sets the pre-shared key. This only uses the value if both buffers are
  // non-NULL and the lengths are both positive. The pointers and lengths
  // are stored.
  void setPSK(const uint8_t *buf, size_t len,
              const uint8_t *id, size_t idLen);

  // Sets the client certificate, key, and key password. This only uses the
  // value if the first two buffers are non-NULL and the lengths are both
  // positive. The password can be NULL or have a length of zero. The pointers
  // and lengths are stored.
  //
  // If the cert or key is in PEM format, then it must be NUL-terminated. The
  // password can be NULL or its length zero if the key is not encrypted.
  void setClientCert(const uint8_t *cert, size_t certLen,
                     const uint8_t *key, size_t keyLen,
                     const uint8_t *keyPwd, size_t keyPwdLen);

  // Sets the hostname for the ServerName extension. If the given string is NULL
  // or empty then the hostname is cleared.
  void setHostname(const char *s);

  const char *hostname() const {
    return hostname_;
  }

  // Sets the handshake timeout, in milliseconds. The default is zero, meaning
  // "wait forever". If the handshake timeout is disabled, then the operation
  // will be non-blocking.
  //
  // See: setHandshakeTimeoutEnabled(flag)
  void setHandshakeTimeout(uint32_t timeout);

  // Returns the handshake timeout. The default is zero, meaning "wait forever".
  // This is only used if the property is enabled.
  //
  // See: isHandshakeTimeoutEnabled()
  uint32_t handshakeTimeout() const {
    return handshakeTimeout_;
  }

  // Sets whether to use the handshake-timeout property for connect(). If
  // disabled, the operation will be non-blocking. The default is enabled.
  void setHandshakeTimeoutEnabled(bool flag);

  // Returns whether handshake timeout is enabled. The default is enabled.
  bool isHandshakeTimeoutEnabled() const {
    return handshakeTimeoutEnabled_;
  }

  int connect(IPAddress ip, uint16_t port) final;
  int connect(const char *host, uint16_t port) final;

  // Write functions
  // The connection may be closed if there was an error.
  size_t write(uint8_t b) final;
  size_t write(const uint8_t *buf, size_t size) final;

  // Functions that loop until all bytes are written. If the connection is
  // closed before all bytes are sent then these break early and return the
  // actual number of bytes sent. In other words, these only return a value less
  // than the specified size if the connection was closed.
  size_t writeFully(uint8_t b);
  size_t writeFully(const char *s);
  size_t writeFully(const void *buf, size_t size);

  // Read functions
  // The connection may be closed if there was an error.
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
  // PSK callback function type.
  using pskf = int (*)(void *p_psk, mbedtls_ssl_context *ssl,
                       const unsigned char *id, size_t idLen);

  enum class States {
    kStart,
    kInitialized,
    kHandshake,
    kConnected,
  };

  // Cert holds a cert and its key.
  struct Cert {
    bool initted = false;
    mbedtls_x509_crt cert;
    mbedtls_pk_context key;

    util::ByteSpan certBuf;
    util::ByteSpan keyBuf;
    util::ByteSpan keyPwd;

    // Returns whether there's valid cert and key buffers.
    bool valid() const;

    // Clears the internal values.
    void clear();

    // Initialization
    void init();
    void deinit();
  };

  // CACert holds a CA cert.
  struct CACert {
    bool initted = false;
    mbedtls_x509_crt cert;

    util::ByteSpan certBuf;

    // Returns whether there's a valid cert buffer.
    bool valid() const;

    // Clears the internal values.
    void clear();

    // Initialization
    void init();
    void deinit();
  };

  // PSK holds pre-shared key data.
  struct PSK {
    util::ByteSpan psk;
    util::ByteSpan id;

    // Returns whether there's a valid key.
    bool valid() const;

    // Clears the internal values.
    void clear();
  };

  // Creates an empty client.
  MbedTLSClient(Client *client);

  // Initializes the client or server.
  bool init(bool server);

  // Uninitializes the client or server.
  void deinit();

  // Adds a server certificate. This does not add it if the cert or key don't
  // have content. The password is optional.
  //
  // If the certificate or key is in PEM format, then it must be NUL-terminated.
  void addServerCert(Cert &&c);

  // Sets the PSK callback for a server-side connection.
  void setPSKCallback(pskf f_psk, void *p_psk);

  // Performs a handshake with the given host and optionally waits. The hostname
  // may be NULL.
  bool handshake(const char *hostname, bool wait);

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

  // Returns whether the TLS client is connected. This doesn't check whether
  // there's data available.
  bool isConnected();

  bool isServer_;

  Client *client_;
  uint32_t handshakeTimeout_;
  bool handshakeTimeoutEnabled_;

  States state_;

  char hostname_[MBEDTLS_SSL_MAX_HOST_NAME_LEN + 1];

  int peeked_;  // < 0 for not there

  // State
  mbedtls_ssl_context ssl_;
  mbedtls_ssl_config conf_;

  // Certificates
  CACert caCert_;
  Cert clientCert_;
  std::vector<Cert> serverCerts_;

  // Key
  PSK psk_;
  pskf f_psk_;
  void *p_psk_;

  friend class MbedTLSServer;
};

}  // namespace network
}  // namespace qindesign
