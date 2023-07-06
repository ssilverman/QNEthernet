// SPDX-FileCopyrightText: (c) 2021-2023 Shawn Silverman <shawn@pobox.com>
// SPDX-License-Identifier: MIT

// QNDNSClient.cpp implements DNSClient.
// This file is part of the QNEthernet library.

#include "QNDNSClient.h"

#if LWIP_DNS

#include <elapsedMillis.h>

#include "lwip/dns.h"
#include "util/ip_tools.h"

namespace qindesign {
namespace network {

void DNSClient::dnsFoundFunc([[maybe_unused]] const char *name,
                             const ip_addr_t *ipaddr,
                             void *callback_arg) {
  if (callback_arg == nullptr) {
    return;
  }

  Request *req = static_cast<Request *>(callback_arg);
  if (req->timeout == 0 || millis() - req->startTime < req->timeout) {
    req->callback(ipaddr);
  }
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
                              std::function<void(const ip_addr_t *)> callback,
                              uint32_t timeout) {
  if (callback == nullptr || hostname == nullptr) {
    return false;
  }

  Request *req = new Request{};
  req->callback = callback;
  req->startTime = millis();
  req->timeout = timeout;

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
  volatile bool found = false;
  volatile bool lookupDone = false;
  if (!DNSClient::getHostByName(
          hostname,
          [&lookupDone, &found, &ip](const ip_addr_t *foundIP) {
            if (foundIP != nullptr) {
              found = true;
              ip = ip_addr_get_ip4_uint32(foundIP);
            }
            lookupDone = true;
          },
          timeout)) {
    return false;
  }

  elapsedMillis timer;
  while (!lookupDone && timer < timeout) {
    // NOTE: Depends on Ethernet loop being called from yield()
    yield();
  }
  return found;
}

}  // namespace network
}  // namespace qindesign

#endif  // LWIP_DNS
