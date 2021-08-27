// ConnectionHolder.h defines all the data needed to host a connection.
// (c) 2021 Shawn Silverman

#ifndef CONNECTIONHOLDER_H_
#define CONNECTIONHOLDER_H_

// C++ includes
#include <functional>
#include <vector>

#include <Arduino.h>
#include <lwip/tcp.h>

namespace qindesign {
namespace network {

class EthernetClient;

// Holds all the state needed for a connection.
struct ConnectionHolder final {
  ConnectionHolder() {
    Serial.printf("ConnectionHolder: %p\n", this);
  }

  ~ConnectionHolder() {
    Serial.printf("~ConnectionHolder: %p\n", this);
    if (removeFunc != nullptr) {
      removeFunc(this);
    }
  }

  // Initializes all the state. This does not set the connection state.
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
  // currently, the callbacks only know about ConnectionHolder (the state).
  EthernetClient *client = nullptr;

  tcp_pcb *volatile pcb = nullptr;

  // Incoming buffer
  std::vector<unsigned char> inBuf;
  volatile int inBufPos = 0;

  // Called when this has been removed from the TCP stack.
  std::function<void(ConnectionHolder *)> removeFunc = nullptr;
};

}  // namespace network
}  // namespace qindesign

#endif  // CONNECTIONHOLDER_H_
