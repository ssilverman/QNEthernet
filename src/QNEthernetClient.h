// QNEthernetClient.h defines the TCP client interface.
// This file is part of the QNEthernet library.
// (c) 2021 Shawn Silverman

#ifndef QNE_ETHERNETCLIENT_H_
#define QNE_ETHERNETCLIENT_H_

// C++ includes
#include <utility>

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
    connTimeout_ = other.connTimeout_;
    connHolder_ = std::move(other.connHolder_);

    // Move the state if internally managed
    if (other.pHolder_ == &other.connHolder_) {
      pHolder_ = &connHolder_;
      if (other.pHolder_->state != nullptr) {
        tcp_arg(other.pHolder_->state->pcb, pHolder_);
        other.pHolder_->state = nullptr;
      }
    } else {
      pHolder_ = other.pHolder_;
    }
  }

  EthernetClient &operator=(EthernetClient &&other) {
    connTimeout_ = other.connTimeout_;
    connHolder_ = std::move(other.connHolder_);

    // Move the state if internally managed
    if (other.pHolder_ == &other.connHolder_) {
      pHolder_ = &connHolder_;
      if (other.pHolder_->state != nullptr) {
        tcp_arg(other.pHolder_->state->pcb, pHolder_);
        other.pHolder_->state = nullptr;
      }
    } else {
      pHolder_ = other.pHolder_;
    }
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
  // Set up an already-connected client. If the holder is NULL then a new
  // unconnected client will be created.
  //
  // The state is externally managed if `holder` it not NULL and does not equal
  // `&connHolder_`. In this case an external party will take care of stopping
  // the connection and freeing the object.
  EthernetClient(ConnectionHolder *holder);

  // Set up an already-connected client. If the state is NULL then a new
  // unconnected client will be created. If the state is not NULL then it is
  // assumed that the client is already connected and the state will be set
  // up accordingly.
  //
  // The state will be internally managed.
  EthernetClient(ConnectionState *state);

  static void dnsFoundFunc(const char *name, const ip_addr_t *ipaddr,
                           void *callback_arg);
  static err_t connectedFunc(void *arg, struct tcp_pcb *tpcb, err_t err);
  static void errFunc(void *arg, err_t err);
  static err_t recvFunc(void *arg, struct tcp_pcb *tpcb, struct pbuf *p,
                        err_t err);

  // Connection state
  uint16_t connTimeout_;

  // DNS lookups
  String lookupHost_;   // For matching DNS lookups
  IPAddress lookupIP_;  // Set by a DNS lookup
  volatile bool lookupFound_;

  ConnectionHolder connHolder_;
  ConnectionHolder *pHolder_;  // Points to the state being used
                               // It is externally managed if it doesn't point
                               // to connHolder_

  friend class EthernetServer;
};

}  // namespace network
}  // namespace qindesign

#endif  // QNE_ETHERNETCLIENT_H_
