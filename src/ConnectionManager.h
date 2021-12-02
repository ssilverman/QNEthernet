// SPDX-FileCopyrightText: (c) 2021 Shawn Silverman <shawn@pobox.com>
// SPDX-License-Identifier: MIT

// ConnectionManager.h defines the connection manager for all connections.
// This file is part of the QNEthernet library.

#ifndef QNE_CONNECTIONMANAGER_H_
#define QNE_CONNECTIONMANAGER_H_

// C++ includes
#include <memory>
#include <vector>

#include "ConnectionHolder.h"
#include "lwip/ip_addr.h"
#include "lwip/tcp.h"

namespace qindesign {
namespace network {

// ConnectionState holds all the state needed for a connection.
class ConnectionManager final {
 public:
  static ConnectionManager &instance() {
    return manager;
  }

  std::shared_ptr<ConnectionHolder> connect(ip_addr_t *ipaddr, uint16_t port);

  // Listens on a port. The `reuse` parameter controls the SO_REUSEADDR flag.
  // This returns whether the attempt was successful.
  bool listen(uint16_t port, bool reuse);

  bool isListening(uint16_t port) const;

  // Stop listening on the specified port. This returns true if the listener was
  // found and successfully stopped. This returns false if the listener was not
  // found or was found and not successfully stopped.
  bool stopListening(uint16_t port);

  // Find a connection whose local port is the specified port.
  std::shared_ptr<ConnectionHolder> findConnected(uint16_t port) const;

  // Find a connection on the given port that has data available.
  std::shared_ptr<ConnectionHolder> findAvailable(uint16_t port) const;

  // Remove the given connection and returns whether the connection existed in
  // the list and was removed.
  bool remove(const std::shared_ptr<ConnectionHolder> &holder);

  // Output routines
  size_t write(uint16_t port, uint8_t b);
  size_t write(uint16_t port, const uint8_t *b, size_t len);
  void flush(uint16_t port);

 private:
  static ConnectionManager manager;

  ConnectionManager() = default;
  ~ConnectionManager() = default;

  static err_t connectedFunc(void *arg, struct tcp_pcb *tpcb, err_t err);
  static void errFunc(void *arg, err_t err);
  static err_t recvFunc(void *arg, struct tcp_pcb *tpcb, struct pbuf *p,
                        err_t err);
  static err_t acceptFunc(void *arg, struct tcp_pcb *newpcb, err_t err);

  // Add a created connection to the list. It is expected that the object is
  // already set up.
  void addConnection(const std::shared_ptr<ConnectionHolder> &holder);

  std::vector<std::shared_ptr<ConnectionHolder>> connections_;
  std::vector<struct tcp_pcb *> listeners_;

  friend class EthernetServer;
};

}  // namespace network
}  // namespace qindesign

#endif  // QNE_CONNECTIONMANAGER_H_
