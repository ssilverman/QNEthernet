// SPDX-FileCopyrightText: (c) 2021-2022 Shawn Silverman <shawn@pobox.com>
// SPDX-License-Identifier: MIT

// QNEthernetServer.h defines the TCP server interface.
// This file is part of the QNEthernet library.

#ifndef QNE_ETHERNETSERVER_H_
#define QNE_ETHERNETSERVER_H_

// C++ includes
#include <cstdint>

#include <Print.h>
#include <Server.h>

#include "QNEthernetClient.h"
#include "lwip/opt.h"

namespace qindesign {
namespace network {

class EthernetServer final : public Server {
 public:
  EthernetServer();
  EthernetServer(uint16_t port);

  ~EthernetServer();

  // Returns the maximum number of TCP listeners.
  static constexpr int maxListeners() {
    return MEMP_NUM_TCP_PCB_LISTEN;
  }

  // Returns the server port. This will return -1 if it has not been set.
  //
  // Note that the value is still a 16-bit quantity if it is non-negative.
  int32_t port() const {
    return port_;
  }

  // Starts listening on the server port, if set. This calls begin(false).
  void begin() override;

  // Starts listening on the server port, if set, and sets the SO_REUSEADDR
  // socket option according to the `reuse` parameter. This returns whether the
  // server started listening. This will always return false if the port is
  // not set.
  bool begin(bool reuse);

  // Starts listening on the specified port. This calls begin(port, false).
  //
  // If the port is already set and the server is listening, then it is first
  // stopped with a call to end().
  bool begin(uint16_t port);

  // Starts listening on the specified port, if set, and sets the SO_REUSEADDR
  // socket option according to the `reuse` parameter. This returns whether the
  // server started listening.
  //
  // If the port is already set and the server is listening, then it is first
  // stopped with a call to end(). This is to prevent a single server object
  // from representing more than one listening socket.
  bool begin(uint16_t port, bool reuse);

  // Stops listening and returns whether the server is stopped. This will always
  // return true if the port is not set.
  bool end();

  // Accepts a connection and returns a client, possibly unconnected. This
  // returns an unconnected client if the port is not set.
  EthernetClient accept() const;

  // Finds a connection with available data. This returns an unconnected client
  // if there is no client with available data or if the port is not set.
  EthernetClient available() const;

  // Bring Print::write functions into scope
  using Print::write;

  // Writes a byte to all the connections. This does nothing and returns 1 if
  // the port is not set.
  size_t write(uint8_t b) override;

  // Writes data to all the connections. This does nothing and returns size if
  // the port is not set.
  size_t write(const uint8_t *buffer, size_t size) override;

  // Returns the minimum availability of all the connections, or zero if there
  // are no connections or if the port is not set.
  int availableForWrite() override;

  // Flushes all the connections, but does nothing is the port is not set.
  void flush() override;

  operator bool();

 private:
  int32_t port_;  // We also want to be able to represent a signed value
                  // Non-negative is 16 bits
  bool listening_;
};

}  // namespace network
}  // namespace qindesign

#endif  // QNE_ETHERNETSERVER_H_
