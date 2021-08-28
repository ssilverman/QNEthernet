// ConnectionState.h defines all the data needed to host a connection.
// This file is part of the QNEthernet library.
// (c) 2021 Shawn Silverman

#ifndef QNE_CONNECTIONSTATE_H_
#define QNE_CONNECTIONSTATE_H_

// C++ includes
#include <functional>
#include <vector>

#include <lwip/tcp.h>

namespace qindesign {
namespace network {

class EthernetClient;

// Holds all the state needed for a connection.
struct ConnectionState final {
  ConnectionState() = default;

  ~ConnectionState() {
    if (removeFunc != nullptr) {
      removeFunc(this);
    }
  }

  // Initializes all the state.
  // 1. Sets up the buffer
  // 2. Connects the listeners
  //
  // This isn't in a constructor because we might want to initialize an
  // existing connection.
  void init(EthernetClient *c, tcp_pcb *tpcb, tcp_recv_fn recvFn, tcp_err_fn errFn) {
    client = c;
    pcb = tpcb;
    inBuf.reserve(TCP_WND);
    inBufPos = 0;
    tcp_arg(pcb, this);
    tcp_err(pcb, errFn);
    tcp_recv(pcb, recvFn);
  }

  // Holds the client that holds this object. This is necessary because
  // currently, the callbacks only know about ConnectionState.
  // I don't love this.
  EthernetClient *client = nullptr;

  tcp_pcb *volatile pcb = nullptr;

  // Incoming buffer
  std::vector<unsigned char> inBuf;
  volatile int inBufPos = 0;

  // Called when this has been removed from the TCP stack.
  std::function<void(ConnectionState *)> removeFunc = nullptr;
};

}  // namespace network
}  // namespace qindesign

#endif  // QNE_CONNECTIONSTATE_H_
