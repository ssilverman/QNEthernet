// SPDX-FileCopyrightText: (c) 2021-2022 Shawn Silverman <shawn@pobox.com>
// SPDX-License-Identifier: MIT

// QNDNSClient.cpp implements DNSClient.
// This file is part of the QNEthernet library.

#include "QNDNSClient.h"

#include <elapsedMillis.h>

#include "lwip/dns.h"
#include "util/ip_tools.h"

namespace qindesign {
namespace network {

void DNSClient::dnsFoundFunc(const char *name, const ip_addr_t *ipaddr,
                             void *callback_arg) {
  if (callback_arg == nullptr || ipaddr == nullptr) {
    return;
  }

  Request *req = reinterpret_cast<Request *>(callback_arg);
  req->callback(ipaddr);
  delete req;
}

bool DNSClient::setServer(int index, const IPAddress &ip) {
  if (index < 0 || maxServers() <= index) {
    return false;
  }
  ip_addr_t addr IPADDR4_INIT(get_uint32(ip));
  dns_setserver(index, &addr);
  return true;
}

IPAddress DNSClient::getServer(int index) {
  if (index < 0 || maxServers() <= index) {
    return INADDR_NONE;
  }
  return ip_addr_get_ip4_uint32(dns_getserver(index));
}

bool DNSClient::getHostByName(const char *hostname,
                              std::function<void(const ip_addr_t *)> callback) {
  if (callback == nullptr) {
    return false;
  }

  Request *req = new Request{};
  req->callback = callback;

  ip_addr_t addr;
  switch (dns_gethostbyname(hostname, &addr, &dnsFoundFunc, req)) {
    case ERR_OK:
      delete req;
      callback(&addr);
      return true;

    case ERR_INPROGRESS:
      return true;

    case ERR_ARG:
    default:
      delete req;
      return false;
  }
}

bool DNSClient::getHostByName(const char *hostname, IPAddress &ip,
                              uint32_t timeout) {
  bool found = false;
  if (!DNSClient::getHostByName(hostname,
                                [&found, &ip](const ip_addr_t *foundIP) {
                                  found = true;
                                  ip = ip_addr_get_ip4_uint32(foundIP);
                                })) {
    return false;
  }

  elapsedMillis timer;
  while (!found && timer < timeout) {
    // NOTE: Depends on Ethernet loop being called from yield()
    yield();
  }
  return found;
}

}  // namespace network
}  // namespace qindesign
