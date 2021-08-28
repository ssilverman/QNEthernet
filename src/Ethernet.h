// Arduino-style Ethernet driver for Teensy 4.1.
// (c) 2021 Shawn Silverman

#ifndef ETHERNET_H_
#define ETHERNET_H_

// C++ includes
#include <cstdint>

#include <IPAddress.h>
#include <elapsedMillis.h>
#include <lwip/netif.h>
#include <lwip/prot/ethernet.h>

namespace qindesign {
namespace network {

class Ethernet final {
 public:
  static constexpr int kMACAddrSize = ETH_HWADDR_LEN;

  Ethernet();
  Ethernet(const uint8_t mac[kMACAddrSize]);

  ~Ethernet();

  // Retrieve the MAC address.
  void macAddress(uint8_t mac[kMACAddrSize]) const;

  // Get the MTU.
  int mtu() const;

  // Call often.
  void loop();

  void begin();
  void begin(const IPAddress &ipaddr,
             const IPAddress &netmask,
             const IPAddress &gw);

  // Shut down the Ethernet peripheral(s).
  void end();

  bool linkStatus() const;

  IPAddress localIP() const;
  IPAddress subnetMask() const;
  IPAddress gatewayIP() const;
  IPAddress dnsServerIP() const;
  void setLocalIP(const IPAddress &localIP);
  void setSubnetMask(const IPAddress &subnetMask);
  void setGatewayIP(const IPAddress &gatewayIP);
  void setDNSServerIP(const IPAddress &dnsServerIP);

 private:
  elapsedMillis loopTimer_;

  uint8_t mac_[kMACAddrSize];
  struct netif *netif_ = nullptr;
  bool isLinkUp_ = false;
};

}  // namespace network
}  // namespace qindesign

#endif  // ETHERNET_H_
