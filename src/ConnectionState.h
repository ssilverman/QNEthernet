// SPDX-FileCopyrightText: (c) 2021 Shawn Silverman <shawn@pobox.com>
// SPDX-License-Identifier: MIT

// ConnectionState.h defines all the data needed to host a connection.
// This file is part of the QNEthernet library.

#ifndef QNE_CONNECTIONSTATE_H_
#define QNE_CONNECTIONSTATE_H_

// C++ includes
#include <functional>
#include <vector>

#include <lwip/tcp.h>

namespace qindesign {
namespace network {

// ConnectionState holds all the state needed for a connection.
struct ConnectionState final {
  ConnectionState(tcp_pcb *tpcb)
      : pcb(tpcb),
        inBufPos(0) {
    inBuf.reserve(TCP_WND);
  }

  ~ConnectionState() {
    if (removeFunc != nullptr) {
      removeFunc(this);
    }
  }

  // Connects the listeners. The `arg` parameter is what gets passed
  // to `tcp_arg`.
  void connect(void *arg, tcp_recv_fn recvFn, tcp_err_fn errFn) {
    tcp_arg(pcb, arg);
    tcp_err(pcb, errFn);
    tcp_recv(pcb, recvFn);
  }

  tcp_pcb *volatile pcb = nullptr;

  // Incoming buffer
  std::vector<unsigned char> inBuf;
  volatile size_t inBufPos = 0;

  // Called when this has been removed from the TCP stack.
  std::function<void(ConnectionState *)> removeFunc = nullptr;
};

}  // namespace network
}  // namespace qindesign

#endif  // QNE_CONNECTIONSTATE_H_
