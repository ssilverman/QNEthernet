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

class EthernetClass final {
 public:
  static constexpr int kMACAddrSize = ETH_HWADDR_LEN;

  EthernetClass();
  EthernetClass(const uint8_t mac[kMACAddrSize]);

  ~EthernetClass();

  // Retrieve the MAC address.
  void macAddress(uint8_t mac[kMACAddrSize]) const;

  // Get the MTU.
  int mtu() const;

  // Call often.
  void loop();

  // Starts Ethernet and a DHCP client. This returns whether starting the DHCP
  // client was successful.
  bool begin();

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

  // The MAC addresses are used in the following begin() functions
  [[deprecated]] int begin(const uint8_t mac[6]);
  [[deprecated]] void begin(const uint8_t mac[6], const IPAddress &ip);
  [[deprecated]] void begin(const uint8_t mac[6], const IPAddress &ip,
                            const IPAddress &dns);
  [[deprecated]] void begin(const uint8_t mac[6], const IPAddress &ip,
                            const IPAddress &dns, const IPAddress &gateway);
  [[deprecated]] void begin(const uint8_t mac[6], const IPAddress &ip,
                            const IPAddress &dns, const IPAddress &gateway,
                            const IPAddress &subnet);

  // Deprecated and unused functions
  [[deprecated]] int hardwareStatus() const { return 0; }
  [[deprecated]] void init(uint8_t sspin) const {}
  [[deprecated]] void MACAddress(uint8_t mac[6]) const { macAddress(mac); }
  [[deprecated]] uint8_t maintain() const { return 0; }
  [[deprecated]] void setDnsServerIP(const IPAddress &dnsServerIP) {
    setDNSServerIP(dnsServerIP);
  }
  [[deprecated]] void setMACAddress(uint8_t mac[6]) {}
  [[deprecated]] void setRetransmissionCount(uint8_t number) {}
  [[deprecated]] void setRetransmissionTimeout(uint16_t milliseconds) {}

 private:
  elapsedMillis loopTimer_;

  uint8_t mac_[kMACAddrSize];
  struct netif *netif_ = nullptr;
  bool isLinkUp_ = false;
};

}  // namespace network
}  // namespace qindesign

extern qindesign::network::EthernetClass Ethernet;

#endif  // ETHERNET_H_
