// EthernetClient.h defines the TCP client interface.
// (c) 2021 Shawn Silverman

#ifndef ETHERNETCLIENT_H_
#define ETHERNETCLIENT_H_

// C++ includes
#include <memory>
#include <vector>

#include <Client.h>
#include <IPAddress.h>
#include <WString.h>
#include "ConnectionHolder.h"

namespace qindesign {
namespace network {

class EthernetServer;

class EthernetClient final : public Client {
 public:
  EthernetClient();
  ~EthernetClient();

  // We only want move semantics because the state should only be owned by one
  // client at a time. When a moved object is destroyed, stop() is called, and
  // we don't want the state to get closed.
  // OLD: In theory, copying should work, but this will catch any odd uses.

  EthernetClient(const EthernetClient &) = delete;
  EthernetClient &operator=(const EthernetClient &) = delete;

  EthernetClient(EthernetClient &&other) {
    connected_ = other.connected_;
    connTimeout_ = other.connTimeout_;
    state_ = other.state_;
    stateExternallyManaged_ = other.stateExternallyManaged_;

    if (state_ != nullptr) {
      state_->client = this;
    }

    // Move the state
    other.state_ = nullptr;
  }

  EthernetClient &operator=(EthernetClient &&other) {
    connected_ = other.connected_;
    connTimeout_ = other.connTimeout_;
    state_ = other.state_;
    stateExternallyManaged_ = other.stateExternallyManaged_;

    if (state_ != nullptr) {
      state_->client = this;
    }

    // Move the state
    other.state_ = nullptr;
    return *this;
  }

  int connect(IPAddress ip, uint16_t port) override;
  int connect(const char *host, uint16_t port) override;

  uint8_t connected() override;
  operator bool() override;

  void setConnectionTimeout(uint16_t timeout);

  void stop() override;

  uint16_t localPort() const;
  IPAddress remoteIP() const;
  uint16_t remotePort() const;

  size_t write(uint8_t b) override;
  size_t write(const uint8_t *buf, size_t size) override;
  int availableForWrite() override;
  void flush() override;

  int available() override;
  int read() override;
  int read(uint8_t *buf, size_t size) override;
  int peek() override;

 private:
  // Set up an already-connected client. If the state is NULL then a new
  // unconnected client will be created. If the state is not NULL then it is
  // assumed that the client is already connected and the state will be set
  // up accordingly.
  //
  // The `externallyManaged` parameter indicates that an external party will
  // take care of stopping the connection and freeing the object.
  EthernetClient(ConnectionHolder *state, bool externallyManaged);

  static void dnsFoundFunc(const char *name, const ip_addr_t *ipaddr,
                           void *callback_arg);
  static err_t connectedFunc(void *arg, struct tcp_pcb *tpcb, err_t err);
  static void errFunc(void *arg, err_t err);
  static err_t recvFunc(void *arg, struct tcp_pcb *tpcb, struct pbuf *p,
                        err_t err);

  // Check if there's data still available in the buffer.
  bool isAvailable();

  // Copy any remaining data from the state to the "remaining" buffer.
  void maybeCopyRemaining();

  // Connection state
  volatile bool connected_ = false;
  uint16_t connTimeout_;

  // DNS lookups
  String lookupHost_;   // For matching DNS lookups
  IPAddress lookupIP_;  // Set by a DNS lookup
  volatile bool lookupFound_;

  ConnectionHolder *state_;
  bool stateExternallyManaged_;  // Indicates if we shouldn't close
                                 // the connection

  // Remaining data after a connection is closed
  // Will only be non-empty after the connection is closed
  int remainingPos_;
  std::vector<unsigned char> remaining_;

  friend class EthernetServer;
};

}  // namespace network
}  // namespace qindesign

#endif  // ETHERNETCLIENT_H_
