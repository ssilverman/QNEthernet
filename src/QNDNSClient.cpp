// SPDX-FileCopyrightText: (c) 2021-2024 Shawn Silverman <shawn@pobox.com>
// SPDX-License-Identifier: AGPL-3.0-or-later

// QNDNSClient.cpp implements DNSClient.
// This file is part of the QNEthernet library.

#include "QNDNSClient.h"

#if LWIP_DNS

// C++ includes
#include <cerrno>

#include "lwip/dns.h"
#include "lwip/err.h"
#include "lwip/sys.h"
#include "util/ip_tools.h"

extern "C" void yield();

namespace qindesign {
namespace network {

void DNSClient::dnsFoundFunc([[maybe_unused]] const char *name,
                             const ip_addr_t *ipaddr,
                             void *callback_arg) {
  if (callback_arg == nullptr) {
    return;
  }

  Request *req = static_cast<Request *>(callback_arg);
  if (req->timeout == 0 || sys_now() - req->startTime < req->timeout) {
    req->callback(ipaddr);
  }
  delete req;
}

bool DNSClient::setServer(int index, const IPAddress &ip) {
#if LWIP_IPV4
  if (index < 0 || maxServers() <= index) {
    return false;
  }
  ip_addr_t addr IPADDR4_INIT(get_uint32(ip));
  dns_setserver(index, &addr);
  return true;
#else
  LWIP_UNUSED_ARG(index);
  LWIP_UNUSED_ARG(ip);
  return false;
#endif  // LWIP_IPV4
}

IPAddress DNSClient::getServer(int index) {
#if LWIP_IPV4
  if (index < 0 || maxServers() <= index) {
    return INADDR_NONE;
  }
  return ip_addr_get_ip4_uint32(dns_getserver(index));
#else
  LWIP_UNUSED_ARG(index);
  return INADDR_NONE;
#endif  // LWIP_IPV4
}

bool DNSClient::getHostByName(const char *hostname,
                              std::function<void(const ip_addr_t *)> callback,
                              uint32_t timeout) {
  if (callback == nullptr || hostname == nullptr) {
    errno = EINVAL;
    return false;
  }

  Request *req = new Request{};
  req->callback = callback;
  req->startTime = sys_now();
  req->timeout = timeout;

  ip_addr_t addr;
  err_t err;
  switch (err = dns_gethostbyname(hostname, &addr, &dnsFoundFunc, req)) {
    case ERR_OK:
      delete req;
      callback(&addr);
      return true;

    case ERR_INPROGRESS:
      return true;

    case ERR_ARG:
    default:
      delete req;
      errno = err_to_errno(err);
      return false;
  }
}

bool DNSClient::getHostByName(const char *hostname, IPAddress &ip,
                              uint32_t timeout) {
#if LWIP_IPV4
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

  uint32_t t = sys_now();
  while (!lookupDone && (sys_now() - t) < timeout) {
    // NOTE: Depends on Ethernet loop being called from yield()
    yield();
  }
  return found;
#else
  LWIP_UNUSED_ARG(hostname);
  LWIP_UNUSED_ARG(ip);
  LWIP_UNUSED_ARG(timeout);
  return false;
#endif  // LWIP_IPV4
}

}  // namespace network
}  // namespace qindesign

#endif  // LWIP_DNS
