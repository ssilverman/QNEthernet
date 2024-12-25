// SPDX-FileCopyrightText: (c) 2024 Shawn Silverman <shawn@pobox.com>
// SPDX-License-Identifier: AGPL-3.0-or-later

// MbedTLSServer.cpp implements the MbedTLS server wrapper.
// This file is part of the QNEthernet library.

#include "MbedTLSServer.h"

// C++ includes
#include <algorithm>
#include <utility>

#include "QNEthernetClient.h"
#include "security/mbedtls_funcs.h"

namespace qindesign {
namespace network {

// PSK callback.
static int pskCallback(void *const p_psk, mbedtls_ssl_context *const ssl,
                       const unsigned char *const id, const size_t idLen) {
  if ((p_psk == nullptr) || (ssl == nullptr)) {
    return -1;
  }
  auto *f = static_cast<MbedTLSServer::pskf *>(p_psk);
  int retval = -1;
  (*f)(id, idLen, [ssl, &retval](const unsigned char *psk, size_t psk_len) {
    if (psk != nullptr && psk_len != 0) {
      retval = mbedtls_ssl_set_hs_psk(ssl, psk, psk_len);
    }
  });
  return retval;
}

MbedTLSServer::MbedTLSServer(EthernetServer &server)
    : server_(server),
      state_(States::kStart) {}

MbedTLSServer::~MbedTLSServer() {
  end();

  for (Cert &c : certs_) {
    c.clear();
  }
  certs_.clear();
  caCert_.clear();
}

void MbedTLSServer::begin() {
  if (state_ >= States::kStarted) {
    return;
  }

  server_.begin();
  state_ = States::kStarted;
}

void MbedTLSServer::end() {
  if (state_ >= States::kStarted) {
    server_.end();
  }
  state_ = States::kStart;
}

bool MbedTLSServer::Cert::valid() const {
  return !cert.empty() && !key.empty();
}

void MbedTLSServer::Cert::clear() {
  cert.clear();
  key.clear();
  keyPwd.clear();
}

bool MbedTLSServer::CACert::valid() const {
  return !cert.empty();
}

void MbedTLSServer::CACert::clear() {
  cert.clear();
}

void MbedTLSServer::setCACert(const uint8_t *const buf, const size_t len) {
  CACert c;
  c.cert.set(buf, len);
  if (c.valid()) {
    caCert_ = std::move(c);
  }
}

void MbedTLSServer::addServerCert(const uint8_t *const cert,
                                  const size_t certLen,
                                  const uint8_t *const key, const size_t keyLen,
                                  const uint8_t *const keyPwd,
                                  const size_t keyPwdLen) {
  Cert c;
  c.cert.set(cert, certLen);
  c.key.set(key, keyLen);
  c.keyPwd.set(keyPwd, keyPwdLen);
  if (c.valid()) {
    certs_.push_back(std::move(c));
  }
}

void MbedTLSServer::onPSK(pskf f) {
  pskCB_ = f;
}

MbedTLSClient MbedTLSServer::accept() {
  if (state_ >= States::kStarted) {
    EthernetClient c = server_.accept();
    if (c) {
      MbedTLSClient tlsClient{c};
      if (caCert_.valid()) {
        tlsClient.setCACert(caCert_.cert.v, caCert_.cert.size);
      }
      for (const Cert &c : certs_) {
        MbedTLSClient::Cert sc;
        sc.certBuf = c.cert;
        sc.keyBuf = c.key;
        sc.keyPwd = c.keyPwd;
        tlsClient.addServerCert(std::move(sc));
      }
      if (pskCB_) {
        tlsClient.setPSKCallback(&pskCallback, &pskCB_);
      }
      if (tlsClient.init(true)) {
        return tlsClient;
      } else {
        c.stop();
      }
    }
  }

  return MbedTLSClient{static_cast<Client *>(nullptr)};
}

size_t MbedTLSServer::write(const uint8_t b) {
  return 0;
}

size_t MbedTLSServer::write(const uint8_t *const buffer, const size_t size) {
  return 0;
}

int MbedTLSServer::availableForWrite() {
  return 0;
}

void MbedTLSServer::flush() {
  if (state_ >= States::kStarted) {
    server_.flush();
  }
}

MbedTLSServer::operator bool() const {
  return (state_ >= States::kStarted) && static_cast<bool>(server_);
}

}  // namespace qindesign
}  // namespace network
