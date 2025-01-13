// SPDX-FileCopyrightText: (c) 2024-2025 Shawn Silverman <shawn@pobox.com>
// SPDX-License-Identifier: AGPL-3.0-or-later

// MbedTLSClient.h defines a Client wrapper for TLS connections that uses the
// MbedTLS library.
// This file is part of the QNEthernet library.

#pragma once

// C++ includes
#include <cstring>
#include <utility>
#include <vector>

#include <mbedtls/ssl.h>

#include "qnethernet/internal/ClientEx.h"
#include "qnethernet/security/MbedTLSCert.h"
#include "qnethernet/security/MbedTLSPSK.h"

namespace qindesign {
namespace network {

// Any data pointers are no longer needed after the handshake completes, unless
// a new connection needs the same data.
class MbedTLSClient : public internal::ClientEx {
 public:
  // Creates an unconnectable client.
  MbedTLSClient();

  MbedTLSClient(Client &client);
  MbedTLSClient(ClientEx &client);
  virtual ~MbedTLSClient();

  // Allow moving but not copying
  MbedTLSClient(const MbedTLSClient &) = delete;
  MbedTLSClient &operator=(const MbedTLSClient &) = delete;
  MbedTLSClient(MbedTLSClient &&other) = default;
  MbedTLSClient &operator=(MbedTLSClient &&other) = default;

  // Sets a new client. This calls stop() first.
  void setClient(Client &client);

  // Sets a new client. This calls stop() first.
  void setClient(internal::ClientEx &client);

  // Sets the CA certificate(s).
  void setCACert(security::MbedTLSCert *ca) {
    ca_ = ca;
  }

  // Sets the client certificate.
  void setClientCert(security::MbedTLSCert *cert) {
    clientCert_ = cert;
  }

  // Sets the pre-shared key.
  void setPSK(const security::MbedTLSPSK *psk) {
    psk_ = psk;
  }

  // Sets the hostname for the ServerName extension. If the given string is NULL
  // or empty then the hostname is cleared.
  void setHostname(const char *s);

  // Gets the hostname for the ServerName extension.
  const char *hostname() const {
    return hostname_;
  }

  // Sets the handshake timeout, in milliseconds. The default is zero, meaning
  // "wait forever". If the handshake timeout is disabled, then the operation
  // will be non-blocking.
  //
  // See: setHandshakeTimeoutEnabled(flag)
  void setConnectionTimeout(uint32_t timeout) final {
    handshakeTimeout_ = timeout;
  }

  // Returns the handshake timeout. The default is zero, meaning "wait forever".
  // This is only used if the property is enabled.
  //
  // See: isHandshakeTimeoutEnabled()
  uint32_t connectionTimeout() const final {
    return handshakeTimeout_;
  }

  // Sets whether to use the handshake-timeout property for connect(). If
  // disabled, the operation will be non-blocking. The default is enabled.
  void setConnectionTimeoutEnabled(bool flag) final {
    handshakeTimeoutEnabled_ = flag;
  }

  // Returns whether handshake timeout is enabled. The default is enabled.
  bool isConnectionTimeoutEnabled() const final {
    return handshakeTimeoutEnabled_;
  }

  // Two forms of the connect() function
  int connect(IPAddress ip, uint16_t port) final;
  int connect(const char *host, uint16_t port) final;

  // Returns whether the client is still in the process of doing the handshake.
  // This is useful when doing a non-blocking connect.
  bool connecting() final;

  IPAddress localIP() final;
  uint16_t localPort() final;
  IPAddress remoteIP() final;
  uint16_t remotePort() final;

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

  MbedTLSClient(Client *client, bool isClientEx);

  // Initializes the client or server.
  bool init(bool server);

  // Uninitializes the client or server.
  void deinit();

  // Adds a server certificate. This does not add it if the cert or key don't
  // have content. The password is optional.
  void addServerCert(security::MbedTLSCert *cert);

  // Sets the PSK callback for a server-side connection.
  void setPSKCallback(pskf f_psk, void *p_psk);

  // Connects to either an IP address or hostname. This is a template because
  // there are two Client connect() functions.
  template <typename T>
  int connect(const char *const host, const T hostOrIp, const uint16_t port);

  // Performs a handshake with the given host and optionally waits. The hostname
  // may be NULL. This expects the client to be initialized and the underlying
  // client to be connected. If this returns false then the client will be
  // deinitialized and the underlying client stopped.
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
  bool isClientEx_;

  uint32_t handshakeTimeout_;
  bool handshakeTimeoutEnabled_;

  States state_;

  char hostname_[MBEDTLS_SSL_MAX_HOST_NAME_LEN + 1];

  int peeked_;  // < 0 for not there

  // State
  mbedtls_ssl_context ssl_;
  mbedtls_ssl_config conf_;

  // Certificates
  security::MbedTLSCert *ca_;
  security::MbedTLSCert *clientCert_;
  std::vector<security::MbedTLSCert *> serverCerts_;  // Guaranteed no NULLs

  // Key
  const security::MbedTLSPSK *psk_;
  pskf f_psk_;
  void *p_psk_;

  friend class MbedTLSServer;
};

// Connects to either an IP address or hostname. This is a template because
// there are two Client connect() functions.
template <typename T>
inline int MbedTLSClient::connect(const char *const host, const T hostOrIp,
                                  const uint16_t port) {
  stop();
  if (client_ == nullptr || !init(false)) {
    return false;
  }

  if (client_->connect(hostOrIp, port) == 0) {
    deinit();
    return false;
  }

  const char *hostname;
  if (std::strlen(hostname_) != 0) {
    hostname = hostname_;
  } else {
    hostname = host;
  }
  return handshake(hostname, handshakeTimeoutEnabled_);
}

}  // namespace network
}  // namespace qindesign
