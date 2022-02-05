// SPDX-FileCopyrightText: (c) 2021-2022 Shawn Silverman <shawn@pobox.com>
// SPDX-License-Identifier: MIT

// QNEthernetClient.h defines the TCP client interface.
// This file is part of the QNEthernet library.

#ifndef QNE_ETHERNETCLIENT_H_
#define QNE_ETHERNETCLIENT_H_

// C++ includes
#include <cstddef>
#include <memory>

#include <Client.h>
#include <IPAddress.h>
#include <Print.h>

#include "internal/ConnectionHolder.h"
#include "lwip/ip_addr.h"
#include "lwip/opt.h"

namespace qindesign {
namespace network {

class EthernetServer;

class EthernetClient final : public Client {
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

  int connect(IPAddress ip, uint16_t port) override;
  int connect(const char *host, uint16_t port) override;

  uint8_t connected() override;
  operator bool() override;

  void setConnectionTimeout(uint16_t timeout);

  void stop() override;

  // Closes the connection. This works the same as stop(), but without waiting
  // for the connection to close.
  void close();

  // Closes the sending side of this connection.
  void closeOutput();

  uint16_t localPort();
  IPAddress remoteIP();
  uint16_t remotePort();

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

  size_t write(uint8_t b) override;
  size_t write(const uint8_t *buf, size_t size) override;
  int availableForWrite() override;
  void flush() override;

  int available() override;
  int read() override;

  // A NULL buffer allows the caller to skip bytes without having to read into
  // a buffer.
  int read(uint8_t *buf, size_t size) override;

  int peek() override;

  // ----------------
  //  Socket options
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
  EthernetClient(std::shared_ptr<internal::ConnectionHolder> holder);

  // ip_addr_t version of connect() function.
  bool connect(const ip_addr_t *ipaddr, uint16_t port);

  // Closes the connection. The `wait` parameter indicates whether to wait for
  // close or timeout. Set to true to wait and false to not wait. stop() calls
  // this with true and close() calls this with false.
  void close(bool wait);

  // Connection state
  uint16_t connTimeout_;

  std::shared_ptr<internal::ConnectionHolder> conn_;
      // If this has not been stopped then conn_ might still be non-NULL, so we
      // can't use NULL as a "connected" check. We also need to check
      // conn_->connected.

  friend class EthernetServer;
};

}  // namespace network
}  // namespace qindesign

#endif  // QNE_ETHERNETCLIENT_H_
