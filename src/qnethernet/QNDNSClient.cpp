// SPDX-FileCopyrightText: (c) 2021-2026 Shawn Silverman <shawn@pobox.com>
// SPDX-License-Identifier: AGPL-3.0-or-later

// QNDNSClient.cpp implements DNSClient.
// This file is part of the QNEthernet library.

#include "qnethernet/QNDNSClient.h"

#if LWIP_DNS

// C++ includes
#include <cerrno>

#include "QNEthernet.h"
#include "lwip/arch.h"
#include "lwip/err.h"
#include "lwip/sys.h"
#include "qnethernet/compat/c++11_compat.h"
#include "qnethernet/util/ip_tools.h"

extern "C" void yield();

namespace qindesign {
namespace network {

void DNSClient::dnsFoundFunc(const char* const name,
                             const ip_addr_t* const ipaddr,
                             void* const callback_arg) {
  LWIP_UNUSED_ARG(name);

  if (callback_arg == nullptr) {
    return;
  }

  const auto req = static_cast<const Request*>(callback_arg);
  if ((req->timeout == 0) || ((sys_now() - req->startTime) < req->timeout)) {
    req->callback(ipaddr);
  }
  delete req;
}

bool DNSClient::setServer(const size_t index, const IPAddress& ip) {
#if LWIP_IPV4
  if (index >= maxServers()) {
    return false;
  }
  const ip_addr_t addr IPADDR4_INIT(static_cast<uint32_t>(ip));
  dns_setserver(static_cast<uint8_t>(index), &addr);
  return true;
#else
  LWIP_UNUSED_ARG(index);
  LWIP_UNUSED_ARG(ip);
  errno = ENOSYS;
  return false;
#endif  // LWIP_IPV4
}

IPAddress DNSClient::getServer(const size_t index) {
#if LWIP_IPV4
  if (index >= maxServers()) {
    return INADDR_NONE;
  }
  return util::ip_addr_get_ip4_uint32(
      dns_getserver(static_cast<uint8_t>(index)));
#else
  LWIP_UNUSED_ARG(index);
  errno = ENOSYS;
  return INADDR_NONE;
#endif  // LWIP_IPV4
}

bool DNSClient::getHostByName(
    const char* const hostname,
    const std::function<void(const ip_addr_t*)> callback,
    const uint32_t timeout) {
  if ((callback == nullptr) || (hostname == nullptr)) {
    errno = EINVAL;
    return false;
  }

  Request* const req = new Request{};
  req->callback = callback;
  req->startTime = sys_now();
  req->timeout = timeout;

  ip_addr_t addr;
  const err_t err = dns_gethostbyname(hostname, &addr, &dnsFoundFunc, req);
  switch (err) {
    case ERR_OK:
      delete req;
      callback(&addr);
      return true;

    case ERR_INPROGRESS:
      return true;

    case ERR_ARG:
      ATTRIBUTE_FALLTHROUGH;
    default:
      delete req;
      errno = err_to_errno(err);
      return false;
  }
}

bool DNSClient::getHostByName(const char* const hostname, IPAddress& ip,
                              const uint32_t timeout) {
#if LWIP_IPV4
  volatile bool found = false;
  volatile bool lookupDone = false;
  if (!DNSClient::getHostByName(
          hostname,
          [&lookupDone, &found, &ip](const ip_addr_t* const foundIP) {
            if (foundIP != nullptr) {
              found = true;
              ip = util::ip_addr_get_ip4_uint32(foundIP);
            }
            lookupDone = true;
          },
          timeout)) {
    // Note: errno set by getHostByName()
    return false;
  }

  const uint32_t t = sys_now();
  while (!lookupDone) {
    if ((sys_now() - t) >= timeout) {
      errno = ETIMEDOUT;
      break;
    }
    yield();
#if !QNETHERNET_DO_LOOP_IN_YIELD
    Ethernet.loop();
#endif  // !QNETHERNET_DO_LOOP_IN_YIELD
  }
  return found;
#else
  LWIP_UNUSED_ARG(hostname);
  LWIP_UNUSED_ARG(ip);
  LWIP_UNUSED_ARG(timeout);
  errno = ENOSYS;
  return false;
#endif  // LWIP_IPV4
}

}  // namespace network
}  // namespace qindesign

#endif  // LWIP_DNS
