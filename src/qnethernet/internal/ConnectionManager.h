// SPDX-FileCopyrightText: (c) 2021-2026 Shawn Silverman <shawn@pobox.com>
// SPDX-License-Identifier: AGPL-3.0-or-later

// ConnectionManager.h defines the connection manager for all connections.
// This file is part of the QNEthernet library.

#pragma once

#include "lwip/opt.h"

#if LWIP_TCP

// C++ includes
#include <cstdint>
#include <memory>
#include <functional>
#include <vector>

#include "lwip/altcp.h"
#include "lwip/ip_addr.h"
#include "qnethernet/compat/c++11_compat.h"
#include "qnethernet/internal/ConnectionHolder.h"

namespace qindesign {
namespace network {
namespace internal {

// ConnectionState holds all the state needed for a connection.
class ConnectionManager final {
 public:
  // Accesses the singleton instance.
  static ConnectionManager& instance();

  ATTRIBUTE_NODISCARD
  std::shared_ptr<ConnectionHolder> connect(const ip_addr_t* ipaddr,
                                            uint16_t port);

  // Listens on a port. The `reuse` parameter controls the SO_REUSEADDR flag.
  // This returns a negative value if the attempt was not successful or the port
  // number otherwise. In theory, this shouldn't return zero.
  ATTRIBUTE_NODISCARD
  int32_t listen(uint16_t port, bool reuse);

  ATTRIBUTE_NODISCARD
  bool isListening(uint16_t port) const;

  // Stops listening on the specified port. This returns true if the listener
  // was found and successfully stopped. This returns false if the listener was
  // not found or was found and not successfully stopped.
  ATTRIBUTE_NODISCARD
  bool stopListening(uint16_t port);

  // Finds a connection whose local port is the specified port, and which has
  // not yet been acknowledged by accept().
  ATTRIBUTE_NODISCARD
  std::shared_ptr<ConnectionHolder> findUnacknowledged(uint16_t port) const;

  // Finds a connection on the given port that has data available.
  ATTRIBUTE_NODISCARD
  std::shared_ptr<ConnectionHolder> findAvailable(uint16_t port) const;

  // Removes the given connection and returns whether the connection existed in
  // the list and was removed.
  ATTRIBUTE_NODISCARD
  bool remove(const std::shared_ptr<ConnectionHolder>& holder);

  // Output routines
  ATTRIBUTE_NODISCARD
  size_t write(uint16_t port, uint8_t b);
  ATTRIBUTE_NODISCARD
  size_t write(uint16_t port, const void* b, size_t len);
  void flush(uint16_t port);
  ATTRIBUTE_NODISCARD
  int availableForWrite(uint16_t port);  // Finds the minimum, or zero for none

  // Aborts all connections. The motivation is outlined in the "On connections
  // that hang around after cable disconnect" section of the Readme.
  //
  // This calls altcp_abort() for each PCB using iterateConnections().
  void abortAll();

  // Iterates over all the valid connections and calls the specified function
  // for each. Don't call anything that can invaliate the connections_ member,
  // for example, Ethernet.loop().
  void iterateConnections(std::function<void(struct altcp_pcb* pcb)> f);

  // Iterates over all the listeners and calls the specified function for each.
  // Don't call anything that can invaliate the listeners_ member, for example,
  // listen() or stopListening().
  void iterateListeners(std::function<void(struct altcp_pcb* pcb)> f);

 private:
  ConnectionManager() = default;

  static err_t connectedFunc(void* arg, struct altcp_pcb* tpcb, err_t err);
  static void errFunc(void* arg, err_t err);
  static err_t recvFunc(void* arg, struct altcp_pcb* tpcb, struct pbuf* p,
                        err_t err);
  static err_t acceptFunc(void* arg, struct altcp_pcb* newpcb, err_t err);

  // Adds a created connection to the list. It is expected that the object is
  // already set up.
  void addConnection(const std::shared_ptr<ConnectionHolder>& holder);

  std::vector<std::shared_ptr<ConnectionHolder>> connections_;
  std::vector<struct altcp_pcb*> listeners_;
};

}  // namespace internal
}  // namespace network
}  // namespace qindesign

#endif  // LWIP_TCP
