// ConnectionHolder.h defines all the data needed to host a connection.
// (c) 2021 Shawn Silverman

#ifndef CONNECTIONHOLDER_H_
#define CONNECTIONHOLDER_H_

// C++ includes
#include <vector>

#include <lwip/tcp.h>

namespace qindesign {
namespace network {

// Holds all the state needed for a connection.
struct ConnectionHolder final {
  tcp_pcb *volatile pcb = nullptr;

  // Connection state
  volatile bool connecting = false;
  volatile bool connected = false;

  // Incoming buffer
  std::vector<unsigned char> inBuf;
  volatile int inBufPos = 0;
};

}  // namespace network
}  // namespace qindesign

#endif  // CONNECTIONHOLDER_H_
