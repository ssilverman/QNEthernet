// EthernetClient.h defines the TCP client interface.
// (c) 2021 Shawn Silverman

#ifndef ETHERNETCLIENT_H_
#define ETHERNETCLIENT_H_

// C++ includes
#include <vector>

#include <Client.h>
#include <IPAddress.h>
#include <WString.h>
#include <lwip/tcp.h>

#include "ConnectionHolder.h"

namespace qindesign {
namespace network {

class EthernetClient final : public Client {
 public:
  EthernetClient();
  ~EthernetClient();

  int connect(IPAddress ip, uint16_t port) override;
  int connect(const char *host, uint16_t port) override;

  uint8_t connected() override;
  operator bool() override;

  void setConnectionTimeout(uint16_t timeout);

  size_t write(uint8_t b) override;
  size_t write(const uint8_t *buf, size_t size) override;
  int availableForWrite() override;

  int available() override;
  int read() override;
  int read(uint8_t *buf, size_t size) override;
  int peek() override;
  void flush() override;
  void stop() override;

 private:
  // Set up an already-connected client.
  EthernetClient(ConnectionHolder *state);

  static void dnsFoundFunc(const char *name, const ip_addr_t *ipaddr,
                           void *callback_arg);
  static err_t connectedFunc(void *arg, struct tcp_pcb *tpcb, err_t err);
  static void errFunc(void *arg, err_t err);
  static err_t recvFunc(void *arg, struct tcp_pcb *tpcb, struct pbuf *p,
                        err_t err);

  // Check if there's data still available in the buffer.
  bool isAvailable();

  uint16_t connTimeout_;

  // DNS lookups
  String lookupHost_;   // For matching DNS lookups
  IPAddress lookupIP_;  // Set by a DNS lookup
  volatile bool lookupFound_;

  ConnectionHolder *state_;
  bool stateNeedsDelete_;
};

}  // namespace network
}  // namespace qindesign

#endif  // ETHERNETCLIENT_H_
