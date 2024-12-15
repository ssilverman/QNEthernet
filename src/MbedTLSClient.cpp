// SPDX-FileCopyrightText: (c) 2024 Shawn Silverman <shawn@pobox.com>
// SPDX-License-Identifier: AGPL-3.0-or-later

// MbedTLSClient.cpp implements the MbedTLS client wrapper.
// This file is part of the QNEthernet library.

#include "MbedTLSClient.h"

#include <lwip/arch.h>
#include <lwip/ip_addr.h>
#include <mbedtls/ctr_drbg.h>
#include <mbedtls/entropy.h>

extern "C" {
size_t qnethernet_hal_fill_rand(uint8_t *buf, size_t len);
uint32_t qnethernet_hal_millis(void);

// Global random state
static bool s_randInit = false;
static mbedtls_ctr_drbg_context s_drbg;
static mbedtls_entropy_context s_entropy;

// Initializes the random context. This uses a single context.
static void initRand() {
  if (s_randInit) {
    return;
  }
  s_randInit = true;

  mbedtls_ctr_drbg_init(&s_drbg);
  mbedtls_entropy_init(&s_entropy);

  // Build a nonce
  uint8_t nonce[128];
  uint8_t *pNonce = nonce;
  size_t sizeRem = sizeof(nonce);
  while (sizeRem != 0) {
    size_t size = qnethernet_hal_fill_rand(pNonce, sizeRem);
    sizeRem -= size;
    pNonce += size;
  }

  mbedtls_ctr_drbg_seed(&s_drbg, mbedtls_entropy_func, &s_entropy,
                        nonce, sizeof(nonce));
}

// Gets entropy for MbedTLS.
int mbedtls_hardware_poll(void *const data,
                          unsigned char *const output, const size_t len,
                          size_t *const olen) {
  LWIP_UNUSED_ARG(data);

  size_t filled = qnethernet_hal_fill_rand(output, len);
  if (olen != NULL) {
    *olen = filled;
  }
  return 0;  // Success
}

mbedtls_ms_time_t mbedtls_ms_time(void) {
  static int64_t top = 0;
  static uint32_t last = 0;
  uint32_t t = qnethernet_hal_millis();
  if (t < last) {
    top += (int64_t{1} << 32);
  }
  last = t;
  return top | t;
}
}  // extern "C"

namespace qindesign {
namespace network {

MbedTLSClient::MbedTLSClient(Client &client)
    : client_(client) {}

void MbedTLSClient::setCACert(const uint8_t *const caCert,
                              const size_t caCertLen) {
  caCertBuf_ = caCert;
  caCertLen_ = caCertLen;
}

void MbedTLSClient::setPSK(const uint8_t *const psk, const size_t pskLen,
                           const uint8_t *const pskId, const size_t pskIdLen) {
  psk_ = psk;
  pskLen_ = pskLen;
  pskId_ = pskId;
  pskIdLen_ = pskIdLen;
}

void MbedTLSClient::setClientCert(const uint8_t *const clientCert,
                                  const size_t clientCertLen,
                                  const uint8_t *const clientKey,
                                  const size_t clientKeyLen) {
  clientCertBuf_ = clientCert;
  clientCertLen_ = clientCertLen;
  clientKeyBuf_ = clientKey;
  clientKeyLen_ = clientKeyLen;
}

void MbedTLSClient::setHandshakeTimeout(uint32_t timeout) {
  handshakeTimeout_ = timeout;
}

bool MbedTLSClient::init() {
  if (state_ >= States::kInitialized) {
    return true;
  }

  const bool hasCACert = (caCertBuf_ != nullptr) && (caCertLen_ != 0);
  const bool hasPSK = (psk_ != nullptr) && (pskLen_ != 0) &&
                      (pskId_ != nullptr) && (pskIdLen_ != 0);
  const bool hasClientCert =
      (clientCertBuf_ != nullptr) && (clientCertLen_ != 0) &&
      (clientKeyBuf_ != nullptr) && (clientKeyLen_ != 0);

  // Initialize state
  mbedtls_ssl_init(&ssl_);
  mbedtls_ssl_config_init(&conf_);
  mbedtls_x509_crt_init(&caCert_);
  mbedtls_x509_crt_init(&clientCert_);
  mbedtls_pk_init(&clientKey_);

  if (mbedtls_ssl_config_defaults(&conf_, MBEDTLS_SSL_IS_CLIENT,
                                  MBEDTLS_SSL_TRANSPORT_STREAM,
                                  MBEDTLS_SSL_PRESET_DEFAULT) != 0) {
    goto init_error;
  }

  initRand();

  mbedtls_ssl_conf_rng(&conf_, mbedtls_ctr_drbg_random, &s_drbg);
  // mbedtls_ssl_conf_read_timeout(&sslConf_, timeout);

  if (hasCACert) {
    mbedtls_ssl_conf_authmode(&conf_, MBEDTLS_SSL_VERIFY_REQUIRED);
    if (mbedtls_x509_crt_parse(&caCert_, caCertBuf_, caCertLen_) < 0) {
      goto init_error;
    }
    mbedtls_ssl_conf_ca_chain(&conf_, &caCert_, nullptr);
  } else {
    mbedtls_ssl_conf_authmode(&conf_, MBEDTLS_SSL_VERIFY_NONE);
  }

  if (hasPSK) {
    if (mbedtls_ssl_conf_psk(&conf_, psk_, pskLen_, pskId_, pskIdLen_) != 0) {
      goto init_error;
    }
    if (mbedtls_ssl_conf_own_cert(&conf_, &clientCert_, &clientKey_) != 0) {
      goto init_error;
    }
  }

  if (hasClientCert) {
    if (mbedtls_x509_crt_parse(&clientCert_, clientCertBuf_,
                               clientCertLen_) < 0) {
      goto init_error;
    }
    mbedtls_ssl_conf_ca_chain(&conf_, &caCert_, nullptr);
  }

  mbedtls_ssl_conf_rng(&conf_, mbedtls_ctr_drbg_random, &s_drbg);

  state_ = States::kInitialized;
  return true;

init_error:
  deinit();
  return false;
}

void MbedTLSClient::deinit() {
  if (state_ < States::kInitialized) {
    return;
  }

  mbedtls_pk_free(&clientKey_);
  mbedtls_x509_crt_free(&clientCert_);
  mbedtls_x509_crt_free(&caCert_);
  mbedtls_ssl_config_free(&conf_);
  mbedtls_ssl_free(&ssl_);

  state_ = States::kStart;
}

int MbedTLSClient::connect(const IPAddress ip, const uint16_t port) {
  if (state_ >= States::kConnected) {
    stop();
  }
  if (!init()) {
    return false;
  }

  const ip_addr_t ipaddr IPADDR4_INIT(static_cast<uint32_t>(ip));

  if (client_.connect(ip, port) == 0 || !connect(ipaddr_ntoa(&ipaddr), true)) {
    deinit();
    return false;
  }

  return true;
}

int MbedTLSClient::connect(const char *const host, const uint16_t port) {
  if (state_ >= States::kConnected) {
    stop();
  }
  if (!init()) {
    return false;
  }

  if (client_.connect(host, port) == 0 || !connect(host, true)) {
    deinit();
    return false;
  }

  return true;
}

static int sendf(void *const ctx,
                 const unsigned char *const buf, const size_t len) {
  Client *const c = static_cast<Client *>(ctx);
  if (c == nullptr) {
    return -1;
  }
  size_t written = c->write(buf, len);  // TODO: Flush?
  if (len != 0 && written == 0) {
    return MBEDTLS_ERR_SSL_WANT_WRITE;
  }
  return written;
}

static int recvf(void *const ctx, unsigned char *const buf, const size_t len) {
  Client *const c = static_cast<Client *>(ctx);
  if (c == nullptr) {
    return -1;
  }
  if (!(*c)) {
    return -1;
  }
  int read = c->read(buf, len);
  if (read <= 0) {
    return MBEDTLS_ERR_SSL_WANT_READ;
  }
  return read;
}

bool MbedTLSClient::connect(const char *const hostname, const bool wait) {
  if (mbedtls_ssl_setup(&ssl_, &conf_) != 0) {
    return false;
  }
  if (mbedtls_ssl_set_hostname(&ssl_, hostname) != 0) {
    return false;
  }
  mbedtls_ssl_set_bio(&ssl_, &client_, &sendf, &recvf, nullptr);

  uint32_t startTime = qnethernet_hal_millis();
  while (true) {
    int ret = mbedtls_ssl_handshake(&ssl_);
    if (ret == 0) {
      state_ = States::kConnected;
      return true;
    }

    if (handshakeTimeout_ != 0 &&
        qnethernet_hal_millis() - startTime >= handshakeTimeout_) {
      return false;
    }

    switch (ret) {
      case MBEDTLS_ERR_SSL_WANT_READ:
      case MBEDTLS_ERR_SSL_WANT_WRITE:
      case MBEDTLS_ERR_SSL_ASYNC_IN_PROGRESS:
      case MBEDTLS_ERR_SSL_CRYPTO_IN_PROGRESS:
      case MBEDTLS_ERR_SSL_RECEIVED_NEW_SESSION_TICKET:
      case MBEDTLS_ERR_SSL_RECEIVED_EARLY_DATA:
        continue;

      default:
        return false;
    }
  }
}

bool MbedTLSClient::checkWrite(int ret) {
  switch (ret) {
    case MBEDTLS_ERR_SSL_WANT_READ:
    case MBEDTLS_ERR_SSL_WANT_WRITE:
    case MBEDTLS_ERR_SSL_ASYNC_IN_PROGRESS:
    case MBEDTLS_ERR_SSL_CRYPTO_IN_PROGRESS:
    case MBEDTLS_ERR_SSL_RECEIVED_NEW_SESSION_TICKET:
    case MBEDTLS_ERR_SSL_RECEIVED_EARLY_DATA:
      return true;

    default:
      stop();
      return false;
  }
}

size_t MbedTLSClient::write(const uint8_t b) {
  return write(&b, 1);
}

size_t MbedTLSClient::write(const uint8_t *const buf, const size_t size) {
  if (state_ < States::kConnected || size == 0) {
    return 0;
  }

  while (true) {
    int written = mbedtls_ssl_write(&ssl_, buf, size);
    if (written >= 0) {  // TODO: Should we continue looping on zero?
      return written;
    }
    if (!checkWrite(written)) {
      return 0;
    }
  }
}

bool MbedTLSClient::checkRead(int ret) {
  switch (ret) {
    case MBEDTLS_ERR_SSL_WANT_READ:
    case MBEDTLS_ERR_SSL_WANT_WRITE:
    case MBEDTLS_ERR_SSL_ASYNC_IN_PROGRESS:
    case MBEDTLS_ERR_SSL_CRYPTO_IN_PROGRESS:
    // case MBEDTLS_ERR_SSL_CLIENT_RECONNECT:  // Only server-side
    case MBEDTLS_ERR_SSL_RECEIVED_NEW_SESSION_TICKET:
    case MBEDTLS_ERR_SSL_RECEIVED_EARLY_DATA:
      return true;

    default:
      stop();
      return false;
  }
}

int MbedTLSClient::available() {
  int peekSize = (peeked_ >= 0) ? 1 : 0;
  if (state_ < States::kConnected) {
    return peekSize;
  }

  int read = mbedtls_ssl_read(&ssl_, nullptr, 0);  // Move the stack along
  if (read != 0) {  // Ordinarily, zero is an error
    (void)checkRead(read);
  }
  return peekSize + mbedtls_ssl_get_bytes_avail(&ssl_);
}

int MbedTLSClient::read() {
  uint8_t data;
  int retval = read(&data, 1);
  if (retval <= 0) {
    return -1;
  }
  return data;
}

int MbedTLSClient::read(uint8_t *const buf, const size_t size) {
  if (state_ < States::kConnected || size == 0) {
    return 0;
  }

  // TODO: Handle NULL buffer for skipping?

  uint8_t *pBuf = buf;
  size_t sizeRem = size;
  int totalRead = 0;

  // Process any peeked byte
  if (peeked_ >= 0) {
    *(pBuf++) = peeked_;
    peeked_ = -1;

    totalRead = 1;
    sizeRem--;
  }

  int read = mbedtls_ssl_read(&ssl_, pBuf, sizeRem);
  if (read > 0) {
    totalRead += read;
  } else {
    (void)checkRead(read);
  }
  return totalRead;
}

int MbedTLSClient::peek() {
  if (peeked_ >= 0) {
    return peeked_;
  }

  peeked_ = read();
  return peeked_;
}

int MbedTLSClient::availableForWrite() {
  return 0;
}

void MbedTLSClient::flush() {
  if (state_ < States::kConnected) {
    return;
  }

  client_.flush();
}

void MbedTLSClient::stop() {
  if (state_ < States::kConnected) {
    return;
  }

  // TODO: Should we process the return value?
  mbedtls_ssl_close_notify(&ssl_);

  deinit();
}

uint8_t MbedTLSClient::connected() {
  return static_cast<bool>(*this) || (peeked_ >= 0);
}

MbedTLSClient::operator bool() {
  return const_cast<const MbedTLSClient *>(this)->operator bool();
}

MbedTLSClient::operator bool() const {
  return (state_ >= States::kConnected);
}

}  // namespace network
}  // namespace qindesign
