// SPDX-FileCopyrightText: (c) 2021-2023 Shawn Silverman <shawn@pobox.com>
// SPDX-License-Identifier: MIT

// ConnectionManager.h defines the connection manager for all connections.
// This file is part of the QNEthernet library.

#ifndef QNETHERNET_INTERNAL_CONNECTIONMANAGER_H_
#define QNETHERNET_INTERNAL_CONNECTIONMANAGER_H_

#include "lwip/opt.h"

#if LWIP_TCP

// C++ includes
#include <memory>
#include <vector>

#include "ConnectionHolder.h"
#include "lwip/altcp.h"
#include "lwip/ip_addr.h"

namespace qindesign {
namespace network {
namespace internal {

// ConnectionState holds all the state needed for a connection.
class ConnectionManager final {
 public:
  // Accesses the singleton instance.
  static ConnectionManager &instance();

  std::shared_ptr<ConnectionHolder> connect(const ip_addr_t *ipaddr,
                                            uint16_t port);

  // Listens on a port. The `reuse` parameter controls the SO_REUSEADDR flag.
  // This returns whether the attempt was successful.
  bool listen(uint16_t port, bool reuse);

  bool isListening(uint16_t port) const;

  // Stops listening on the specified port. This returns true if the listener
  // was found and successfully stopped. This returns false if the listener was
  // not found or was found and not successfully stopped.
  bool stopListening(uint16_t port);

  // Finds a connection whose local port is the specified port.
  std::shared_ptr<ConnectionHolder> findConnected(uint16_t port) const;

  // Finds a connection on the given port that has data available.
  std::shared_ptr<ConnectionHolder> findAvailable(uint16_t port) const;

  // Removes the given connection and returns whether the connection existed in
  // the list and was removed.
  bool remove(const std::shared_ptr<ConnectionHolder> &holder);

  // Output routines
  size_t write(uint16_t port, uint8_t b);
  size_t write(uint16_t port, const uint8_t *b, size_t len);
  void flush(uint16_t port);
  int availableForWrite(uint16_t port);  // Finds the minimum, or zero for none

 private:
  ConnectionManager() = default;
  ~ConnectionManager() = default;

  static err_t connectedFunc(void *arg, struct altcp_pcb *tpcb, err_t err);
  static void errFunc(void *arg, err_t err);
  static err_t recvFunc(void *arg, struct altcp_pcb *tpcb, struct pbuf *p,
                        err_t err);
  static err_t acceptFunc(void *arg, struct altcp_pcb *newpcb, err_t err);

  // Adds a created connection to the list. It is expected that the object is
  // already set up.
  void addConnection(const std::shared_ptr<ConnectionHolder> &holder);

  std::vector<std::shared_ptr<ConnectionHolder>> connections_;
  std::vector<struct altcp_pcb *> listeners_;
};

}  // namespace internal
}  // namespace network
}  // namespace qindesign

#endif  // LWIP_TCP

#endif  // QNETHERNET_INTERNAL_CONNECTIONMANAGER_H_
