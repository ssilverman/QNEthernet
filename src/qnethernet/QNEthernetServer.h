// SPDX-FileCopyrightText: (c) 2021-2025 Shawn Silverman <shawn@pobox.com>
// SPDX-License-Identifier: AGPL-3.0-or-later

// QNEthernetServer.h defines the TCP server interface.
// This file is part of the QNEthernet library.

#pragma once

#include "lwip/opt.h"

#if LWIP_TCP

// C++ includes
#include <cstddef>
#include <cstdint>

#include <Print.h>
#include <Server.h>

#include "qnethernet/QNEthernetClient.h"
#include "qnethernet/internal/optional.h"
#include "qnethernet/internal/PrintfChecked.h"

namespace qindesign {
namespace network {

class EthernetServer : public Server, public internal::PrintfChecked {
 public:
  EthernetServer() = default;
  explicit EthernetServer(uint16_t port);

  // Disallow copying but allow moving
  EthernetServer(const EthernetServer&) = delete;
  EthernetServer& operator=(const EthernetServer&) = delete;
  EthernetServer(EthernetServer&&) = default;
  EthernetServer& operator=(EthernetServer&&) = default;

  virtual ~EthernetServer();

  // Returns the maximum number of TCP listeners.
  static constexpr int maxListeners() {
    return MEMP_NUM_TCP_PCB_LISTEN;
  }

  // Returns the server port. This will return -1 if it is not set. If the port
  // was specified to be zero, then this will return a system-chosen value if
  // the socket has been started or zero if the socket has not been started.
  //
  // Note that the value is still a 16-bit quantity if it is non-negative.
  int32_t port() const;

  // Starts listening on the server port, if set. This does not set the
  // SO_REUSEADDR socket option.
  //
  // This first calls end() if the _reuse_ socket option differs.
  //
  // This function is defined by the Arduino API.
  void begin() final;  // Wish: Boolean return

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
  //
  // This function is defined by the Arduino API.
  EthernetClient accept() const;

  // Finds a connection with available data. This returns an unconnected client
  // if there is no client with available data or if the port is not set.
  //
  // This function is defined by the Arduino API.
  EthernetClient available() const;

  // Use the one from here instead of the one from Print
  using internal::PrintfChecked::printf;

  // Bring Print::write functions into scope
  using Print::write;

  // Writes a byte to all the connections. This does nothing and returns 1 if
  // the port is not set.
  size_t write(uint8_t b) final;

  // Writes data to all the connections. This does nothing and returns size if
  // the port is not set.
  size_t write(const uint8_t* buffer, size_t size) final;

  // Returns the minimum availability of all the connections, or zero if there
  // are no connections or if the port is not set.
  int availableForWrite() final;

  // Flushes all the connections, but does nothing if the port is not set.
  void flush() final;

  // Returns whether the server is listening on a port.
  //
  // This function is defined by the Arduino API.
  explicit operator bool() const;

 private:
  bool begin(uint16_t port, bool reuse);

  internal::optional<uint16_t>
      port_;            // Zero means let the system choose a port
  bool reuse_ = false;  // Whether the SO_REUSEADDR socket option is set

  // The listening port may be different from the requested port, say if the
  // requested port is zero.
  uint16_t listeningPort_ = 0;
};

}  // namespace network
}  // namespace qindesign

#endif  // LWIP_TCP
