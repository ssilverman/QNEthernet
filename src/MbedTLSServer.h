// SPDX-FileCopyrightText: (c) 2024 Shawn Silverman <shawn@pobox.com>
// SPDX-License-Identifier: AGPL-3.0-or-later

// MbedTLSServer.h defines a Server wrapper for TLS connections that uses the
// MbedTLS library.
// This file is part of the QNEthernet library.

#pragma once

// C++ includes
#include <cstdint>
#include <functional>
#include <vector>

#include <Server.h>
#include <mbedtls/ssl.h>

#include "MbedTLSClient.h"
#include "QNEthernetServer.h"

namespace qindesign {
namespace network {

class MbedTLSServer : public Server {
 public:
  // Callback function for processing a client PSK request.
  using pskf = std::function<void(
      const unsigned char *id, size_t idLen,
      std::function<void(const unsigned char *psk, size_t psk_len)> psk)>;

  MbedTLSServer(EthernetServer &Server);
  virtual ~MbedTLSServer();

  // Allow moving but not copying
  MbedTLSServer(const MbedTLSServer &) = delete;
  MbedTLSServer &operator=(const MbedTLSServer &) = delete;
  MbedTLSServer(MbedTLSServer &&other) = default;
  MbedTLSServer &operator=(MbedTLSServer &&other) = default;

  void begin() final;
  void end();

  // Sets the CA certificate(s).
  void setCACert(security::MbedTLSCert *ca);

  // Adds a server certificate. This does not add it if the cert or key don't
  // have content. The password is optional.
  void addServerCert(security::MbedTLSCert *cert);

  // Sets the callback for processing a PSK request from the client.
  void onPSK(pskf f);

  // Returns either a connected client or a disconnected client. If the client
  // is connected then the handshake has started but has yet to be completed.
  // The caller must keep checking that this is so.
  MbedTLSClient accept();
  // MbedTLSClient available() const;

  // Defined by Print; these do nothing or return 0. flush() flushes the
  // underlying Server.
  size_t write(uint8_t b) override;
  size_t write(const uint8_t *buffer, size_t size) override;
  int availableForWrite()	override;
  void flush() override;

  explicit operator bool() const;

 private:
  enum class States {
    kStart,
    kStarted,
  };

  EthernetServer &server_;
  States state_;

  security::MbedTLSCert *ca_;
  std::vector<security::MbedTLSCert *> certs_;  // Won't contain NULLs
  pskf pskCB_;
};

}  // namespace network
}  // namespace qindesign
