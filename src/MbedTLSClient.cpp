// SPDX-FileCopyrightText: (c) 2024 Shawn Silverman <shawn@pobox.com>
// SPDX-License-Identifier: AGPL-3.0-or-later

// MbedTLSClient.cpp implements the MbedTLS client wrapper.
// This file is part of the QNEthernet library.

#include "MbedTLSClient.h"

#include <cstring>

#include "lwip/ip_addr.h"
#include "security/mbedtls_funcs.h"
#include "util/PrintUtils.h"

extern "C" {
uint32_t qnethernet_hal_millis(void);
}  // extern "C"

namespace qindesign {
namespace network {

MbedTLSClient::MbedTLSClient(Client *client)
    : isServer_(false),
      client_(client),
      handshakeTimeout_(0),
      handshakeTimeoutEnabled_(true),
      state_(States::kStart),
      hostname_{0},
      peeked_(-1),
      ssl_{},
      conf_{},
      caCert_{},
      clientCert_{},
      serverCerts_(),
      psk_{},
      f_psk_(nullptr),
      p_psk_(nullptr) {}

MbedTLSClient::MbedTLSClient(Client &client) : MbedTLSClient(&client) {}

MbedTLSClient::~MbedTLSClient() {
  // Clear everything
  deinit();

  // Certificates
  caCert_.clear();
  clientCert_.clear();
  for (Cert &c: serverCerts_) {
    c.clear();
  }
  serverCerts_.clear();

  // Keys
  psk_.clear();

  f_psk_ = nullptr;
  p_psk_ = nullptr;
}

void MbedTLSClient::setCACert(const uint8_t *const buf, const size_t len) {
  CACert c;
  c.buf = buf;
  c.len = len;
  if (c.valid()) {
    caCert_ = c;
  }
}

void MbedTLSClient::setPSK(const uint8_t *const buf, const size_t len,
                           const uint8_t *const id, const size_t idLen) {
  PSK psk;
  psk.psk = buf;
  psk.len = len;
  psk.id = id;
  psk.idLen = idLen;
  if (psk.valid()) {
    psk_ = psk;
  }
}

void MbedTLSClient::setClientCert(const uint8_t *const cert,
                                  const size_t certLen,
                                  const uint8_t *const key, const size_t keyLen,
                                  const uint8_t *const keyPwd,
                                  const size_t keyPwdLen) {
  if ((cert != nullptr) && (certLen != 0) &&
      (key != nullptr) && (keyLen != 0)) {
    clientCert_.certBuf = cert;
    clientCert_.certLen = certLen;
    clientCert_.keyBuf = key;
    clientCert_.keyLen = keyLen;
    clientCert_.keyPwd = keyPwd;
    clientCert_.keyPwdLen = keyPwdLen;
  }
}

void MbedTLSClient::addServerCert(Cert &&c) {
  if (c.valid()) {
    serverCerts_.push_back(std::move(c));
  }
}

void MbedTLSClient::setPSKCallback(pskf f_psk, void *const p_psk) {
  f_psk_ = f_psk;
  p_psk_ = p_psk;
}

void MbedTLSClient::setHostname(const char *const s) {
  if (s == nullptr) {
    hostname_[0] = '\0';
  } else {
    std::strncpy(hostname_, s, sizeof(hostname_) - 1);
  }
}

void MbedTLSClient::setHandshakeTimeout(const uint32_t timeout) {
  handshakeTimeout_ = timeout;
}

void MbedTLSClient::setHandshakeTimeoutEnabled(const bool flag) {
  handshakeTimeoutEnabled_ = flag;
}

bool MbedTLSClient::Cert::valid() const {
  return (certBuf != nullptr) && (certLen != 0) &&
         (keyBuf != nullptr) && (keyLen != 0);
}

void MbedTLSClient::Cert::init() {
  if (valid() && !initted) {
    mbedtls_x509_crt_init(&cert);
    mbedtls_pk_init(&key);
    initted = true;
  }
}

void MbedTLSClient::Cert::deinit() {
  if (initted) {
    initted = false;
    mbedtls_pk_free(&key);
    mbedtls_x509_crt_free(&cert);
  }
}

void MbedTLSClient::Cert::clear() {
  certBuf = nullptr;
  certLen = 0;
  keyBuf = nullptr;
  keyLen = 0;
}

bool MbedTLSClient::CACert::valid() const {
  return (buf != nullptr) && (len != 0);
}

void MbedTLSClient::CACert::init() {
  if (valid() && !initted) {
    mbedtls_x509_crt_init(&cert);
  }
}

void MbedTLSClient::CACert::deinit() {
  if (initted) {
    initted = false;
    mbedtls_x509_crt_free(&cert);
  }
}

void MbedTLSClient::CACert::clear() {
  buf = nullptr;
  len = 0;
}

bool MbedTLSClient::PSK::valid() const {
  return (psk != nullptr) && (len != 0) &&
         (id != nullptr) && (idLen != 0);
}

void MbedTLSClient::PSK::clear() {
  psk = nullptr;
  len = 0;
  id = nullptr;
  idLen = 0;
}

bool MbedTLSClient::init(bool server) {
  if (state_ >= States::kInitialized) {
    return true;
  }

  isServer_ = server;

  // Initialize state
  mbedtls_ssl_init(&ssl_);
  mbedtls_ssl_config_init(&conf_);
  caCert_.init();
  if (!server) {
    clientCert_.init();
  } else {
    for (Cert &c : serverCerts_) {
      c.init();
    }
  }

  // Function that parses a certificate and its key, and marks it as ours
  auto parsef = [&](Cert &c) -> bool {
    // Also disallow partially-parsed certificates
    return (mbedtls_x509_crt_parse(&c.cert, c.certBuf, c.certLen) != 0) &&
           (mbedtls_pk_parse_key(&c.key, c.keyBuf, c.keyLen, c.keyPwd,
                                 c.keyPwdLen, qnethernet_mbedtls_rand_f_rng,
                                 qnethernet_mbedtls_rand_p_rng) != 0) &&
           (mbedtls_ssl_conf_own_cert(&conf_, &c.cert, &c.key) != 0);

  };

  if (mbedtls_ssl_config_defaults(
          &conf_, !server ? MBEDTLS_SSL_IS_CLIENT : MBEDTLS_SSL_IS_SERVER,
          MBEDTLS_SSL_TRANSPORT_STREAM, MBEDTLS_SSL_PRESET_DEFAULT) != 0) {
    goto init_error;
  }

  if (!qnethernet_mbedtls_init_rand(&conf_)) {
    goto init_error;
  }
  // mbedtls_ssl_conf_read_timeout(&sslConf_, timeout);

  // Certificate chain
  if (caCert_.valid()) {
    if (mbedtls_x509_crt_parse(&caCert_.cert, caCert_.buf, caCert_.len) < 0) {
      goto init_error;
    }
    mbedtls_ssl_conf_ca_chain(&conf_, &caCert_.cert, nullptr);
  }

  // Certificate(s)
  if (!server) {
    if (caCert_.valid()) {
      mbedtls_ssl_conf_authmode(&conf_, MBEDTLS_SSL_VERIFY_REQUIRED);
    } else {
      mbedtls_ssl_conf_authmode(&conf_, MBEDTLS_SSL_VERIFY_NONE);
    }

    if (clientCert_.valid()) {
      if (!parsef(clientCert_)) {
        goto init_error;
      }
    }
  } else {
    for (Cert &c : serverCerts_) {
      if (!parsef(c)) {
        goto init_error;
      }
    }
  }

  // Pre-shared key
  if (!server) {
    if (psk_.valid()) {
      if (mbedtls_ssl_conf_psk(&conf_, psk_.psk, psk_.len, psk_.id, psk_.idLen) !=
          0) {
        goto init_error;
      }
    }
  } else {
    mbedtls_ssl_conf_psk_cb(&conf_, f_psk_, p_psk_);
  }

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
  state_ = States::kStart;

  clientCert_.deinit();
  for (Cert &c : serverCerts_) {
    c.deinit();
  }
  caCert_.deinit();

  mbedtls_ssl_config_free(&conf_);
  mbedtls_ssl_free(&ssl_);
}

int MbedTLSClient::connect(const IPAddress ip, const uint16_t port) {
  if (state_ >= States::kConnected) {
    stop();
  }
  if (client_ == nullptr || !init(false)) {
    return false;
  }

  if (client_->connect(ip, port) == 0) {
    deinit();
    return false;
  }

  const char *hostname;
  if (std::strlen(hostname_) != 0) {
    hostname = hostname_;
  } else {
    const ip_addr_t ipaddr IPADDR4_INIT(static_cast<uint32_t>(ip));
    hostname = ipaddr_ntoa(&ipaddr);
  }
  return handshake(hostname, handshakeTimeoutEnabled_);
}

int MbedTLSClient::connect(const char *const host, const uint16_t port) {
  if (state_ >= States::kConnected) {
    stop();
  }
  if (client_ == nullptr || !init(false)) {
    return false;
  }

  if (client_->connect(host, port) == 0) {
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

static int sendf(void *const ctx,
                 const unsigned char *const buf, const size_t len) {
  Client *const c = static_cast<Client *>(ctx);
  if (c == nullptr || !c->connected()) {
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
  if (c == nullptr || !c->connected()) {
    return 0;
  }
  int read = c->read(buf, len);
  if (read <= 0) {
    return MBEDTLS_ERR_SSL_WANT_READ;
  }
  return read;
}

bool MbedTLSClient::watchHandshake() {
  if (mbedtls_ssl_is_handshake_over(&ssl_)) {
    state_ = States::kConnected;
    return true;
  }
  int ret = mbedtls_ssl_handshake_step(&ssl_);
  switch (ret) {
    case MBEDTLS_ERR_SSL_WANT_READ:
    case MBEDTLS_ERR_SSL_WANT_WRITE:
    case MBEDTLS_ERR_SSL_ASYNC_IN_PROGRESS:
    case MBEDTLS_ERR_SSL_CRYPTO_IN_PROGRESS:
    case MBEDTLS_ERR_SSL_RECEIVED_NEW_SESSION_TICKET:
    case MBEDTLS_ERR_SSL_RECEIVED_EARLY_DATA:
      return true;

    default:
      deinit();
      return false;
  }
}

bool MbedTLSClient::handshake(const char *const hostname, const bool wait) {
  if (client_ == nullptr) {
    return false;
  }
  if (mbedtls_ssl_setup(&ssl_, &conf_) != 0) {
    return false;
  }
  if (!isServer_) {
    if (mbedtls_ssl_set_hostname(&ssl_, hostname) != 0) {
      return false;
    }
  }
  mbedtls_ssl_set_bio(&ssl_, client_, &sendf, &recvf, nullptr);

  if (!wait) {
    state_ = States::kHandshake;
    return watchHandshake();
  }

  uint32_t startTime = qnethernet_hal_millis();
  while (!isConnected()) {
    if (state_ < States::kInitialized) {
      return false;
    }

    if (handshakeTimeout_ != 0 &&
        qnethernet_hal_millis() - startTime >= handshakeTimeout_) {
      return false;
    }
  }
  return true;
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
  if (!isConnected() || size == 0) {
    return 0;
  }

  int written = mbedtls_ssl_write(&ssl_, buf, size);
  if (written >= 0) {  // TODO: Should we continue looping on zero?
    return written;
  }
  (void)checkWrite(written);
  return 0;
}

size_t MbedTLSClient::writeFully(const uint8_t b) {
  return writeFully(&b, 1);
}

size_t MbedTLSClient::writeFully(const char *const buf) {
  return writeFully(buf, std::strlen(buf));
}

size_t MbedTLSClient::writeFully(const void *const buf, const size_t size) {
  // Don't use connected() as the "connected" check because that will
  // return true if there's data available, and the loop doesn't check
  // for data available. Instead, use operator bool().

  return util::writeFully(*this, static_cast<const uint8_t *>(buf), size, this);
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
  if (!isConnected()) {
    return 0;
  }

  size_t avail = mbedtls_ssl_get_bytes_avail(&ssl_);
  if (peeked_ >= 0) {
    return 1 + avail;
  }
  if (avail != 0) {
    return avail;
  }

  // Move the stack along
  uint8_t b;
  int read = mbedtls_ssl_read(&ssl_, &b, 1);
  if (read == 1) {
    peeked_ = b;
    return 1 + mbedtls_ssl_get_bytes_avail(&ssl_);
  } else if (checkRead(read)) {
    return mbedtls_ssl_get_bytes_avail(&ssl_);
  } else {
    return 0;
  }
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
  if (!isConnected() || size == 0) {
    return 0;
  }

  // TODO: Handle NULL buffer for skipping?

  uint8_t *pBuf = buf;
  size_t sizeRem = size;
  int totalRead = 0;

  // Process any peeked byte
  if (peeked_ >= 0) {
    *buf = peeked_;
    peeked_ = -1;
    if (size == 1) {
      return 1;
    }

    totalRead = 1;
    pBuf++;
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
  if (peeked_ < 0) {
    peeked_ = read();
  }
  return peeked_;
}

int MbedTLSClient::availableForWrite() {
  if (client_ == nullptr) {
    return 0;
  }
  int avail = client_->availableForWrite();
  if (avail <= 0) {
    return 0;
  }
  int expan = mbedtls_ssl_get_record_expansion(&ssl_);
  if (expan < 0 || avail <= expan) {
    return 0;
  }
  avail -= expan;
  int payload = mbedtls_ssl_get_max_out_record_payload(&ssl_);
  if (payload < 0 || avail <= payload) {
    return avail;
  }
  return payload;
}

void MbedTLSClient::flush() {
  if (isConnected()) {
    client_->flush();
  }
}

void MbedTLSClient::stop() {
  if (state_ >= States::kConnected) {
    // TODO: Should we process the return value?
    mbedtls_ssl_close_notify(&ssl_);
    client_->flush();
    client_->stop();  // TODO: Should we stop the underlying Client?
    state_ = States::kInitialized;
  }

  peeked_ = -1;
  deinit();
}

bool MbedTLSClient::isConnected() {
  if (state_ == States::kHandshake) {
    if (!watchHandshake()) {
      return false;
    }
  }
  return (state_ >= States::kConnected);
}

uint8_t MbedTLSClient::connected() {
  // TODO: Should we cache readable data?
  return isConnected() || (peeked_ >= 0) ||
         (mbedtls_ssl_get_bytes_avail(&ssl_) > 0);
}

// This also moves any pending handshake along.
MbedTLSClient::operator bool() {
  return isConnected() && static_cast<bool>(*client_);
}

}  // namespace network
}  // namespace qindesign
