// SPDX-FileCopyrightText: (c) 2021-2022 Shawn Silverman <shawn@pobox.com>
// SPDX-License-Identifier: MIT

// ConnectionState.h defines all the data needed to host a connection.
// This file is part of the QNEthernet library.

#ifndef QNE_INTERNAL_CONNECTIONSTATE_H_
#define QNE_INTERNAL_CONNECTIONSTATE_H_

// C++ includes
#include <functional>
#include <vector>

#include "lwip/tcp.h"

namespace qindesign {
namespace network {
namespace internal {

// ConnectionState holds all the state needed for a connection.
struct ConnectionState final {
  // Creates a new object and sets `arg` as the pcb's arg. This also reserves
  // TCP_WND bytes as buffer space.
  ConnectionState(tcp_pcb *tpcb, void *arg) : pcb(tpcb) {
    tcp_arg(tpcb, arg);
    buf.reserve(TCP_WND);
  }

  // Sets the callback arg to nullptr and then calls the 'remove' function. The
  // object should be deleted before more 'tcp' functions are called.
  ~ConnectionState() {
    // Ensure callbacks are no longer called with this as the argument
    tcp_arg(pcb, nullptr);

    if (removeFunc != nullptr) {
      removeFunc(this);
    }
  }

  tcp_pcb *volatile pcb = nullptr;

  // Incoming data buffer
  std::vector<unsigned char> buf;
  volatile size_t bufPos = 0;

  // Called from the desctructor after the callback arg is deleted.
  std::function<void(ConnectionState *)> removeFunc = nullptr;
};

}  // namespace internal
}  // namespace network
}  // namespace qindesign

#endif  // QNE_INTERNAL_CONNECTIONSTATE_H_
