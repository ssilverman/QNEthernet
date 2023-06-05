// SPDX-FileCopyrightText: (c) 2021-2023 Shawn Silverman <shawn@pobox.com>
// SPDX-License-Identifier: MIT

// ConnectionState.h defines all the data needed to host a connection.
// This file is part of the QNEthernet library.

#ifndef QNETHERNET_INTERNAL_CONNECTIONSTATE_H_
#define QNETHERNET_INTERNAL_CONNECTIONSTATE_H_

#include "lwip/opt.h"

#if LWIP_TCP

// C++ includes
#include <functional>
#include <vector>

#include "lwip/altcp.h"

namespace qindesign {
namespace network {
namespace internal {

// ConnectionState holds all the state needed for a connection.
struct ConnectionState final {
  // Creates a new object and sets `arg` as the pcb's arg. This also reserves
  // TCP_WND bytes as buffer space.
  ConnectionState(altcp_pcb *tpcb, void *arg) : pcb(tpcb) {
    altcp_arg(tpcb, arg);
    buf.reserve(TCP_WND);
  }

  // Sets the callback arg to nullptr and then calls the 'remove' function. The
  // object should be deleted before more 'tcp' functions are called.
  ~ConnectionState() {
    // Ensure callbacks are no longer called with this as the argument
    altcp_arg(pcb, nullptr);

    if (removeFunc != nullptr) {
      removeFunc(this);
    }
  }

  altcp_pcb *volatile pcb = nullptr;

  // Incoming data buffer
  std::vector<uint8_t> buf;
  volatile size_t bufPos = 0;

  // Called from the destructor after the callback arg is deleted.
  std::function<void(ConnectionState *)> removeFunc = nullptr;
};

}  // namespace internal
}  // namespace network
}  // namespace qindesign

#endif  // LWIP_TCP

#endif  // QNETHERNET_INTERNAL_CONNECTIONSTATE_H_
