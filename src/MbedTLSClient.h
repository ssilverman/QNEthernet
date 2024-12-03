// SPDX-FileCopyrightText: (c) 2024 Shawn Silverman <shawn@pobox.com>
// SPDX-License-Identifier: AGPL-3.0-or-later

// MbedTLSClient.h defines a Client wrapper for TLS connections that uses the
// MbedTLS library.
// This file is part of the QNEthernet library.

#pragma once

#include <Client.h>
#include <mbedtls/ssl.h>

namespace qindesign {
namespace network {

class MbedTLSClient : public Client {
 public:
  MbedTLSClient(Client &c);
  virtual ~MbedTLSClient() = default;

  int connect(IPAddress ip, uint16_t port) override;

 protected:
  Client &c_;

  mbedtls_ssl_context sslCtx_;
  mbedtls_ssl_config sslConf_;
  mbedtls_ctr_drbg_context drbgCtx_;
  mbedtls_entropy_context entropyCtx_;
};

}  // namespace network
}  // namespace qindesign
