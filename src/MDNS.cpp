// MDNS.cpp implements MDNS.
// (c) 2021 Shawn Silverman

#include "MDNS.h"

#include <lwip/apps/mdns.h>

namespace qindesign {
namespace network {

// Count array elements.
template <typename T, size_t N>
static constexpr size_t countof(const T (&)[N]) {
  return N;
}

// static void srv_txt(struct mdns_service *service, void *txt_userdata) {
//   err_t res = mdns_resp_add_service_txtitem(service, "path=/", 6);
//   LWIP_ERROR("mdns add service txt failed\n", (res == ERR_OK), return );
// }

bool MDNS::begin(const String &host) {
  netif_ = netif_default;
  if (netif_ == nullptr) {
    return false;
  }

  mdns_resp_init();

  if (mdns_resp_add_netif(netif_, host.c_str()) != ERR_OK) {
    netif_ = nullptr;
    return false;
  }
  host_ = host;
  return true;
}

bool MDNS::end() {
  if (netif_ == nullptr) {
    // Return true for no netif
    return true;
  }
  bool retval = (mdns_resp_remove_netif(netif_) == ERR_OK);
  netif_ = nullptr;
  return retval;
}

// toProto converts a protocol to a protocol enum. This returns DNSSD_PROTO_TCP
// for "_tcp" and DNSSD_PROTO_UDP for all else.
enum mdns_sd_proto toProto(const String &protocol) {
  if (protocol.equalsIgnoreCase("_tcp")) {
    return DNSSD_PROTO_TCP;
  } else {
    return DNSSD_PROTO_UDP;
  }
}

bool MDNS::addService(const String &type, const String &protocol,
                      uint16_t port) {
  if (netif_ == nullptr) {
    return false;
  }

  int8_t slot =
      mdns_resp_add_service(netif_, host_.c_str(), type.c_str(),
                            toProto(protocol), port, nullptr, nullptr);
  if (slot < 0 || countof(slots_) <= static_cast<size_t>(slot)) {
    return false;
  }
  auto tuple = std::make_tuple(type, protocol, port);
  slots_[slot] = std::make_unique<decltype(tuple)>(tuple);
  return true;
}

int MDNS::findService(const String &type, const String &protocol,
                      uint16_t port) {
  auto tuple = std::make_tuple(type, protocol, port);
  for (size_t i = 0; i < countof(slots_); i++) {
    if (slots_[i] != nullptr && *slots_[i] == tuple) {
      return i;
    }
  }
  return -1;
}

bool MDNS::removeService(const String &type, const String &protocol, uint16_t port) {
  if (netif_ == nullptr) {
    // Return true for no netif
    return true;
  }

  // Find a matching service
  int found = findService(type, protocol, port);
  if (found < 0) {
    return false;
  }
  bool retval = (mdns_resp_del_service(netif_, found) == ERR_OK);
  if (retval) {
    slots_[found] = nullptr;
  }
  return retval;
}

}  // namespace network
}  // namespace qindesign
