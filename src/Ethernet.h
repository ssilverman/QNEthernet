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
  Ethernet();
  Ethernet(const uint8_t mac[ETH_HWADDR_LEN]);

  ~Ethernet();

  // Call often.
  static void loop();

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

  const bool isDefaultMAC_;
  uint8_t mac_[ETH_HWADDR_LEN];

  struct netif *netif_;

  bool isLinkUp_ = false;
};

}  // namespace network
}  // namespace qindesign
