// SPDX-FileCopyrightText: (c) 2024 Shawn Silverman <shawn@pobox.com>
// SPDX-License-Identifier: AGPL-3.0-or-later

// TLSClient.h defines a Client wrapper for TLS connections.
// This file is part of the QNEthernet library.

#include "MbedTLSClient.h"

#include <mbedtls/ctr_drbg.h>
#include <mbedtls/entropy.h>

namespace qindesign {
namespace network {

MbedTLSClient::MbedTLSClient(Client &c)
    : c_(c) {
  mbedtls_ssl_init(&sslCtx_);
  mbedtls_ssl_config_init(&sslConf_);
  mbedtls_ctr_drbg_init(&drbgCtx_);
  mbedtls_entropy_init(&entropyCtx_);

  if (mbedtls_ctr_drbg_seed(&drbgCtx_, mbedtls_entropy_func, &entropyCtx_,
                            nullptr, 0) != 0) {
    // Error
  }

  if (mbedtls_ssl_config_defaults(&sslConf_, MBEDTLS_SSL_IS_CLIENT,
                                  MBEDTLS_SSL_TRANSPORT_STREAM,
                                  MBEDTLS_SSL_PRESET_DEFAULT) != 0) {
    // Error
  }
}

MbedTLSClient::connect();

}  // namespace network
}  // namespace qindesign
