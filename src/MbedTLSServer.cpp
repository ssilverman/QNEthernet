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

void MbedTLSServer::setCACert(security::MbedTLSCert *ca) {
  ca_ = ca;
}

void MbedTLSServer::addServerCert(security::MbedTLSCert *cert) {
  if (cert != nullptr && !cert->empty() && cert->hasKey()) {
    certs_.push_back(cert);
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
      if (ca_ != nullptr && !ca_->empty()) {
        tlsClient.setCACert(ca_);
      }
      for (security::MbedTLSCert *c : certs_) {
        tlsClient.addServerCert(c);
      }
      if (pskCB_) {
        tlsClient.setPSKCallback(&pskCallback, &pskCB_);
      }
      if (tlsClient.init(true)) {
        // A false return from handshake() also stops the client
        if (tlsClient.handshake(nullptr, false)) {
          return tlsClient;
        }
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
