// Arduino-style Ethernet driver for Teensy 4.1.
// (c) 2021 Shawn Silverman

// C++ includes
#include <cstdint>

#include <IPAddress.h>
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
  void macAddress(uint8_t mac[kMACAddrSize]);

  // Get the MTU.
  int mtu();

  // Call often.
  void loop();

  void begin();
  void begin(const IPAddress &ipaddr,
             const IPAddress &netmask,
             const IPAddress &gw);

  bool linkStatus();

  IPAddress localIP();
  IPAddress subnetMask();
  IPAddress gatewayIP();
  IPAddress dnsServerIP();
  void setLocalIP(const IPAddress &localIP);
  void setSubnetMask(const IPAddress &subnetMask);
  void setGatewayIP(const IPAddress &gatewayIP);
  void setDNSServerIP(const IPAddress &dnsServerIP);

 private:
  static struct netif nullNetif_;

  uint8_t mac_[kMACAddrSize];
  struct netif *netif_;
  bool isLinkUp_ = false;
};

}  // namespace network
}  // namespace qindesign
