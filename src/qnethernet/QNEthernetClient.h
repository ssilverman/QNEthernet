// SPDX-FileCopyrightText: (c) 2021-2025 Shawn Silverman <shawn@pobox.com>
// SPDX-License-Identifier: AGPL-3.0-or-later

// QNEthernetClient.h defines the TCP client interface.
// This file is part of the QNEthernet library.

#pragma once

#include "lwip/opt.h"

#if LWIP_TCP

// C++ includes
#include <cstddef>
#include <cstdint>
#include <memory>

#include <IPAddress.h>
#include <Print.h>

#include "lwip/ip_addr.h"
#include "lwip/tcpbase.h"
#include "qnethernet/internal/ClientEx.h"
#include "qnethernet/internal/ConnectionHolder.h"
#include "qnethernet/internal/IPOpts.h"
#include "qnethernet/internal/PrintfChecked.h"

namespace qindesign {
namespace network {

class EthernetServer;

class EthernetClient : public internal::ClientEx,
                       public internal::IPOpts,
                       public internal::PrintfChecked {
 public:
  EthernetClient();
  virtual ~EthernetClient();

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

  // Returns a Boolean value.
  //
  // Waiting can be disabled by setConnectionTimeoutEnabled(false).
  //
  // This function is defined by the Arduino API.
  int connect(IPAddress ip, uint16_t port) final;

  // Returns false if DNS is disabled.
  //
  // If this returns false and there was an error then errno will be set.
  //
  // Waiting can be disabled by setConnectionTimeoutEnabled(false).
  //
  // This function is defined by the Arduino API.
  int connect(const char *host, uint16_t port) final;

  // Starts the connection process but doesn't wait for the connection to
  // be complete.
  //
  // Note: This has been superseded by setConnectionTimeoutEnabled(false) used
  // with connect().
  bool connectNoWait(const IPAddress &ip, uint16_t port);

  // Starts the connection process but doesn't wait for the connection to
  // be complete. Note that DNS lookup might still take some time.
  //
  // This returns false if DNS is disabled.
  //
  // If this returns false and there was an error then errno will be set.
  //
  // Note: This has been superseded by setConnectionTimeoutEnabled(false) used
  // with connect().
  bool connectNoWait(const char *host, uint16_t port);

  // Returns whether the client is in the process of connecting. This is used
  // when doing a non-blocking connect.
  bool connecting() final;

  // These functions are defined by the Arduino API:

  uint8_t connected() final;  // Wish: Boolean return
  explicit operator bool() final;

  // Sets the connection timeout, in milliseconds. The default is 1000. If the
  // connection timeout is disabled, then the operation will be non-blocking.
  //
  // This function is defined by the Arduino API.
  //
  // See: setConnectionTimeoutEnabled(flag)
  void setConnectionTimeout(uint32_t timeout) final {
    connTimeout_ = timeout;
  }

  // Returns the connection timeout. The default is 1000. This is only used if
  // the property is enabled.
  //
  // See: isConnectionTimeoutEnabled()
  uint32_t connectionTimeout() const final {
    return connTimeout_;
  }

  // Sets whether to use the connection-timeout property for connect() and
  // stop(). If disabled, these operations will be non-blocking. The default
  // is enabled.
  void setConnectionTimeoutEnabled(bool flag) final {
    connTimeoutEnabled_ = flag;
  }

  // Returns whether connection timeout is enabled. The default is enabled.
  bool isConnectionTimeoutEnabled() const final {
    return connTimeoutEnabled_;
  }

  // Waiting can be disabled by setConnectionTimeoutEnabled(false).
  //
  // This function is defined by the Arduino API.
  void stop() final;

  // Closes the connection. This works the same as stop(), but without waiting
  // for the connection to close.
  //
  // Note: This has been superseded by setConnectionTimeoutEnabled(false) used
  // with stop().
  void close();

  // Closes the sending side of this connection.
  void closeOutput();

  // Kills the connection without going through the TCP close process. This
  // sends a RST segment to the remote host.
  void abort();

  // Functions defned by the Arduino API
  uint16_t localPort() final;
  IPAddress remoteIP() final;
  uint16_t remotePort() final;

  // Returns the local IP address for this connection, or INADDR_NONE if this
  // client is not connected.
  IPAddress localIP() final;

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
  size_t writeFully(const void *buf, size_t size);

  // Use the one from here instead of the one from Print
  using internal::PrintfChecked::printf;

  // If this returns zero and there was an error then errno will be set.
  size_t write(uint8_t b) final;

  // If this returns zero and there was an error then errno will be set.
  size_t write(const uint8_t *buf, size_t size) final;

  int availableForWrite() final;
  void flush() final;

  int available() final;
  int read() final;

  // A NULL buffer allows the caller to skip bytes without having to read into
  // a buffer.
  int read(uint8_t *buf, size_t size) final;

  int peek() final;

#if !LWIP_ALTCP || defined(LWIP_DEBUG)
  // Returns one of the TCP states from:
  // [RFC 9293, Section 3.3.2](https://www.rfc-editor.org/rfc/rfc9293#name-state-machine-overview)
  tcp_state status() const;
  // Note: This isn't implemented for altcp without LWIP_DEBUG defined to avoid
  //       having to modify the lwIP code for this feature
#endif  // !LWIP_ALTCP || defined(LWIP_DEBUG)

  // ----------------
  //  Socket Options
  // ----------------

  // Disables or enables Nagle's algorithm. This sets or clears the TCP_NODELAY
  // flag. If the flag is true then Nagle's algorithm is disabled, otherwise it
  // is enabled. Note that this option must be set for each new connection.
  //
  // This returns true if connected and the option was set, and false otherwise.
  bool setNoDelay(bool flag);

  // Returns the value of the TCP_NODELAY flag for the current connection. This
  // returns false if not connected.
  bool isNoDelay() const;

  // Sets the differentiated services (DiffServ, DS) field in the outgoing IP
  // header. The top 6 bits are the differentiated services code point (DSCP)
  // value, and the bottom 2 bits are the explicit congestion notification
  // (ECN) value.
  //
  // This returns true if connected and the value was set, and false otherwise.
  //
  // Note that this must be set for each new connection.
  bool setOutgoingDiffServ(uint8_t ds) final;

  // Returns the differentiated services (DiffServ) value from the outgoing IP
  // header. This will return zero if not connected.
  uint8_t outgoingDiffServ() const final;

  // Sets the TTL field in the outgoing IP header.
  //
  // This returns true if connected and the value was set, and false otherwise.
  //
  // Note that this must be set for each new connection.
  bool setOutgoingTTL(uint8_t ttl) final;

  // Returns the TTL value from the outgoing IP header. This will return zero if
  // not connected.
  uint8_t outgoingTTL() const final;

 private:
  // Sets up an already-connected client. If the holder is NULL then a new
  // unconnected client will be created.
  explicit EthernetClient(std::shared_ptr<internal::ConnectionHolder> holder);

  // ip_addr_t version of connect() function.
  bool connect(const ip_addr_t *ipaddr, uint16_t port, bool wait);

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

  // Gets address info for this connection. This returns whether the client is
  // connected and there was information to get.
  bool getAddrInfo(bool local, ip_addr_t *addr, u16_t *port);

  // Connection state
  uint32_t connTimeout_;
  bool pendingConnect_;
  bool connTimeoutEnabled_;

  std::shared_ptr<internal::ConnectionHolder> conn_;
      // If this has not been stopped then conn_ might still be non-NULL, so we
      // can't use NULL as a "connected" check. We also need to check
      // conn_->connected.

  friend class EthernetServer;
};

}  // namespace network
}  // namespace qindesign

#endif  // LWIP_TCP
