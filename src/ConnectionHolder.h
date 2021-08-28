// ConnectionHolder.h defines a proxy for connection state.
// This file is part of the QNEthernet library.
// (c) 2021 Shawn Silverman

#ifndef QNE_CONNECTIONHOLDER_H_
#define QNE_CONNECTIONHOLDER_H_

#include "ConnectionState.h"

// C++ includes
#include <vector>

namespace qindesign {
namespace network {

struct ConnectionHolder final {
  volatile bool connected = false;
  ConnectionState *volatile state = nullptr;

  // Remaining data after a connection is closed
  // Will only be non-empty after the connection is closed
  volatile int remainingPos = 0;
  std::vector<unsigned char> remaining;
};

}  // namespace network
}  // namespace qindesign

#endif  // QNE_CONNECTIONHOLDER_H_
