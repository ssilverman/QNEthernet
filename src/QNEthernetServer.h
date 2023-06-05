// SPDX-FileCopyrightText: (c) 2021-2023 Shawn Silverman <shawn@pobox.com>
// SPDX-License-Identifier: MIT

// QNEthernetServer.h defines the TCP server interface.
// This file is part of the QNEthernet library.

#ifndef QNETHERNET_ETHERNETSERVER_H_
#define QNETHERNET_ETHERNETSERVER_H_

#include "lwip/opt.h"

#if LWIP_TCP

// C++ includes
#include <cstddef>
#include <cstdint>

#include <Print.h>
#include <Server.h>

#include "QNEthernetClient.h"

namespace qindesign {
namespace network {

class EthernetServer : public Server {
 public:
  EthernetServer();
  explicit EthernetServer(uint16_t port);

  // Disallow copying but allow moving
  EthernetServer(const EthernetServer &) = delete;
  EthernetServer &operator=(const EthernetServer &) = delete;
  EthernetServer(EthernetServer &&) = default;
  EthernetServer &operator=(EthernetServer &&) = default;

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

  // Starts listening on the server port, if set. This does not set the
  // SO_REUSEADDR socket option.
  //
  // This first calls end() if the _reuse_ socket option differs.
  void begin() final;

  // Starts listening on the server port, if set, and sets the SO_REUSEADDR
  // socket option. This returns whether the server started listening. This will
  // always return false if the port is not set.
  //
  // This first calls end() if the _reuse_ socket option differs.
  bool beginWithReuse();

  // Starts listening on the specified port. This does not set the SO_REUSEADDR
  // socket option. This returns whether the server started listening.
  //
  // This first calls end() if the port or _reuse_ socket option differ.
  bool begin(uint16_t port);

  // Starts listening on the specified port, if set, and sets the SO_REUSEADDR
  // socket option. This returns whether the server started listening.
  //
  // If the port or _reuse_ socket option differ then this first calls end() to
  // prevent a single server object from representing more than one
  // listening socket.
  bool beginWithReuse(uint16_t port);

  // Stops listening. This does nothing if the port is not set or the server is
  // not listening.
  void end();

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
  size_t write(uint8_t b) final;

  // Writes data to all the connections. This does nothing and returns size if
  // the port is not set.
  size_t write(const uint8_t *buffer, size_t size) final;

  // Returns the minimum availability of all the connections, or zero if there
  // are no connections or if the port is not set.
  int availableForWrite() final;

  // Flushes all the connections, but does nothing is the port is not set.
  void flush() final;

  explicit operator bool() const;

 private:
  bool begin(uint16_t port, bool reuse);

  int32_t port_;  // We also want to be able to represent a signed value
                  // Non-negative is 16 bits
  bool reuse_;    // Whether the SO_REUSEADDR socket option is set
  bool listening_;
};

}  // namespace network
}  // namespace qindesign

#endif  // LWIP_TCP

#endif  // QNETHERNET_ETHERNETSERVER_H_
