// SPDX-FileCopyrightText: (c) 2021 Shawn Silverman <shawn@pobox.com>
// SPDX-License-Identifier: MIT

// QNDNSClient.cpp implements DNSClient.
// This file is part of the QNEthernet library.

#include "QNDNSClient.h"

#include <elapsedMillis.h>
#include <lwip/dns.h>

namespace qindesign {
namespace network {

void DNSClient::dnsFoundFunc(const char *name, const ip_addr_t *ipaddr,
                             void *callback_arg) {
  if (callback_arg == nullptr || ipaddr == nullptr) {
    return;
  }

  Request *req = reinterpret_cast<Request *>(callback_arg);
  req->callback(ipaddr->addr);
  delete req;
}

bool DNSClient::setServer(int index, const IPAddress &ip) {
  if (index < 0 || maxServers() <= index) {
    return false;
  }
  const ip_addr_t addr =
      IPADDR4_INIT(static_cast<uint32_t>(const_cast<IPAddress &>(ip)));
  dns_setserver(index, &addr);
  return true;
}

bool DNSClient::getHostByName(const char *hostname,
                              std::function<void(IPAddress)> callback) {
  if (callback == nullptr) {
    return false;
  }

  Request *req = new Request{};
  req->callback = callback;

  ip_addr_t addr;
  switch (dns_gethostbyname(hostname, &addr, &dnsFoundFunc, req)) {
    case ERR_OK:
      delete req;
      callback(addr.addr);
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
                                [&found, &ip](IPAddress foundIP) {
                                  found = true;
                                  ip = foundIP;
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
