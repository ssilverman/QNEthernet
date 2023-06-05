// SPDX-FileCopyrightText: (c) 2021-2022 Shawn Silverman <shawn@pobox.com>
// SPDX-License-Identifier: MIT

// ConnectionHolder.h holds everything needed to define a connection.
// This file is part of the QNEthernet library.

#ifndef QNETHERNET_INTERNAL_CONNECTIONHOLDER_H_
#define QNETHERNET_INTERNAL_CONNECTIONHOLDER_H_

#include "lwip/opt.h"

#if LWIP_TCP

// C++ includes
#include <memory>

#include "ConnectionState.h"
#include "lwip/err.h"

namespace qindesign {
namespace network {
namespace internal {

// ConnectionHolder is effectively a connection proxy, useful because
// connections need to be managed by both a server and client. This is the
// argument passed to the lwIP callbacks.
struct ConnectionHolder final {
  err_t lastError = ERR_OK;
  volatile bool connected = false;
  std::unique_ptr<ConnectionState> state;

  // Remaining data after a connection is closed
  // Will only be non-empty after the connection is closed
  volatile size_t remainingPos = 0;
  std::vector<uint8_t> remaining;
  // `remainingPos` should never be past the end of `remaining`
};

}  // namespace internal
}  // namespace network
}  // namespace qindesign

#endif  // LWIP_TCP

#endif  // QNETHERNET_INTERNAL_CONNECTIONHOLDER_H_
