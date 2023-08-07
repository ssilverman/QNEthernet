// SPDX-FileCopyrightText: (c) 2021-2023 Shawn Silverman <shawn@pobox.com>
// SPDX-License-Identifier: MIT

// QNEthernetClient.h defines the TCP client interface.
// This file is part of the QNEthernet library.

#ifndef QNETHERNET_ETHERNETCLIENT_H_
#define QNETHERNET_ETHERNETCLIENT_H_

#include "lwip/opt.h"

#if LWIP_TCP

// C++ includes
#include <cstddef>
#include <cstdint>
#include <memory>

#include <Client.h>
#include <IPAddress.h>
#include <Print.h>

#include "internal/ConnectionHolder.h"
#include "lwip/ip_addr.h"

namespace qindesign {
namespace network {

class EthernetServer;

class EthernetClient : public Client {
 public:
  EthernetClient();
  ~EthernetClient();

  // Ideally, we only want move semantics because the state should only be owned
  // by one client at a time. However, user code may need to copy and the writer
  // may not get that compiler errors are because std::move was required.

  EthernetClient(const EthernetClient &) = default;
  EthernetClient &operator=(const EthernetClient &) = default;
  EthernetClient(EthernetClient &&other) = default;
  EthernetClient &operator=(EthernetClient &&other) = default;

  // Returns the maximum number of TCP connections.
  static constexpr int maxSockets() {
    return MEMP_NUM_TCP_PCB;
  }

  int connect(IPAddress ip, uint16_t port) final;

  // Returns INVALID_SERVER (-2) if DNS is disabled.
  int connect(const char *host, uint16_t port) final;

  // These functions start the connection process but don't wait for the
  // connection to be complete. Note that DNS lookup might still take some time.
  // Neither of these will return TIMED_OUT (-1).
  int connectNoWait(const IPAddress &ip, uint16_t port);

  // Returns INVALID_SERVER (-2) if DNS is disabled.
  int connectNoWait(const char *host, uint16_t port);

  uint8_t connected() final;
  explicit operator bool() final;

  void setConnectionTimeout(uint16_t timeout);

  // Returns the current timeout value.
  uint16_t connectionTimeout() const {
    return connTimeout_;
  }

  void stop() final;

  // Closes the connection. This works the same as stop(), but without waiting
  // for the connection to close.
  void close();

  // Closes the sending side of this connection.
  void closeOutput();

  // Kills the connection without going through the TCP close process.
  void abort();

  uint16_t localPort();
  IPAddress remoteIP();
  uint16_t remotePort();

  // Returns an ID for the connection to which this client refers. It will
  // return non-zero if connected and zero if not connected.
  //
  // This is useful because of the way EthernetClient objects can be passed
  // around, copied, and moved, etc. Just taking an address of the object won't
  // work because more than one object could refer to the same connection.
  //
  // Note that while multiple active connections won't share the same ID, it's
  // possible for new connections to reuse IDs that aren't currently in use. In
  // other words, there is a one-to-one correspondence between the set of
  // connection IDs and currently active connections.
  uintptr_t connectionId();

  // Bring Print::write functions into scope
  using Print::write;

  // Functions that loop until all bytes are written. If the connection is
  // closed before all bytes are sent then these break early and return the
  // actual number of bytes sent. In other words, these only return a value less
  // than the specified size if the connection was closed.
  size_t writeFully(uint8_t b);
  size_t writeFully(const char *s);
  size_t writeFully(const char *s, size_t size);
  size_t writeFully(const uint8_t *buf, size_t size);

  size_t write(uint8_t b) final;
  size_t write(const uint8_t *buf, size_t size) final;
  int availableForWrite() final;
  void flush() final;

  int available() final;
  int read() final;

  // A NULL buffer allows the caller to skip bytes without having to read into
  // a buffer.
  int read(uint8_t *buf, size_t size) final;

  int peek() final;

  // ----------------
  //  Socket Options
  // ----------------

  // Disables or enables Nagle's algorithm. This sets or clears the TCP_NODELAY
  // flag. If the flag is set then Nagle's algorithm is disabled, otherwise it
  // is enabled. Note that this option must be set for each new connection.
  void setNoDelay(bool flag);

  // Returns the value of the TCP_NODELAY flag for the current connection. This
  // returns false if not connected.
  bool isNoDelay();

 private:
  // Sets up an already-connected client. If the holder is NULL then a new
  // unconnected client will be created.
  explicit EthernetClient(std::shared_ptr<internal::ConnectionHolder> holder);

  // ip_addr_t version of connect() function.
  int connect(const ip_addr_t *ipaddr, uint16_t port, bool wait);

  // Checks if there's a pending connection. If there is, the state is modified
  // appropriately. This returns false if the connection is inactive; 'conn_' is
  // set to NULL. This returns true otherwise; 'pendingConnection_' is set to
  // '!conn_->connection' and 'Ethernet.loop()' is called.
  //
  // This should only be called if 'pendingConnect_' is true and 'conn_' is
  // not NULL.
  bool watchPendingConnect();

  // Closes the connection. The `wait` parameter indicates whether to wait for
  // close or timeout. Set to true to wait and false to not wait. stop() calls
  // this with true and close() calls this with false.
  void close(bool wait);

  // Connection state
  uint16_t connTimeout_;
  bool pendingConnect_;

  std::shared_ptr<internal::ConnectionHolder> conn_;
      // If this has not been stopped then conn_ might still be non-NULL, so we
      // can't use NULL as a "connected" check. We also need to check
      // conn_->connected.

  friend class EthernetServer;
};

}  // namespace network
}  // namespace qindesign

#endif  // LWIP_TCP

#endif  // QNETHERNET_ETHERNETCLIENT_H_
