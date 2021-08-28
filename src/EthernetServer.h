// EthernetServer.h defines the TCP server interface.
// (c) 2021 Shawn Silverman

#ifndef ETHERNETSERVER_H_
#define ETHERNETSERVER_H_

// C++ includes
#include <vector>
#include <memory>

#include <Client.h>
#include <Server.h>
#include <lwip/tcp.h>
#include "EthernetClient.h"

namespace qindesign {
namespace network {

class EthernetServer final : public Server {
 public:
  EthernetServer(uint16_t port);
  ~EthernetServer();

  void begin() override;

  EthernetClient accept();
  EthernetClient available();

  size_t write(uint8_t b) override;
  size_t write(const uint8_t *buffer, size_t size) override;

  // Always returns zero.
  int availableForWrite() override;

  void flush() override;

  operator bool();

 private:
  static void errFunc(void *arg, err_t err);
  static err_t acceptFunc(void *arg, struct tcp_pcb *newpcb, err_t err);
  // static err_t recvFunc(void *arg, struct tcp_pcb *tpcb, struct pbuf *p,
  //                       err_t err);

  uint16_t port_;
  tcp_pcb *volatile pcb_;

  // tcp_pcb/data-available pairs.
  std::vector<ConnectionHolder *> clients_;
};

}  // namespace network
}  // namespace qindesign

#endif  // ETHERNETSERVER_H_
