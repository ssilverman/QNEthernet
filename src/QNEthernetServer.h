// SPDX-FileCopyrightText: (c) 2021 Shawn Silverman <shawn@pobox.com>
// SPDX-License-Identifier: MIT

// QNEthernetServer.h defines the TCP server interface.
// This file is part of the QNEthernet library.

#ifndef QNE_ETHERNETSERVER_H_
#define QNE_ETHERNETSERVER_H_

// C++ includes
#include <memory>

#include <Print.h>
#include <Server.h>
#include "QNEthernetClient.h"

namespace qindesign {
namespace network {

class EthernetServer final : public Server {
 public:
  EthernetServer(uint16_t port);
  ~EthernetServer();

  void begin() override;
  bool end();

  EthernetClient accept() const;
  EthernetClient available() const;

  // Bring Print::write functions into scope
  using Print::write;

  size_t write(uint8_t b) override;
  size_t write(const uint8_t *buffer, size_t size) override;

  // Always returns zero.
  int availableForWrite() override;

  void flush() override;

  operator bool();

 private:
  uint16_t port_;
  bool listening_;
};

}  // namespace network
}  // namespace qindesign

#endif  // QNE_ETHERNETSERVER_H_
