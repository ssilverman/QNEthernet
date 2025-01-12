// SPDX-FileCopyrightText: (c) 2025 Shawn Silverman <shawn@pobox.com>
// SPDX-License-Identifier: AGPL-3.0-or-later

// ClientEx.h extends the Client interface by adding some additional functions.
// This file is part of the QNEthernet library.

#pragma once

// C++ includes
#include <cstdint>

#include <Client.h>
#include <IPAddress.h>

namespace qindesign {
namespace network {
namespace internal {

// ClientEx extends Client and adds some Arduino API and other functions.
class ClientEx : public Client {
 public:
  ClientEx() = default;
  virtual ~ClientEx() = default;

  // Local and remote IP address and port
  virtual IPAddress localIP() = 0;
  virtual uint16_t localPort() = 0;   // Arduino API
  virtual IPAddress remoteIP() = 0;   // Arduino API
  virtual uint16_t remotePort() = 0;  // Arduino API

  virtual void setConnectionTimeout(uint32_t timeout) = 0;  // Arduino API
  virtual uint32_t connectionTimeout() const = 0;

  // Returns whether the client is in the process of connecting. This is used
  // when doing a non-blocking connect.
  virtual bool connecting() = 0;

  // Sets whether to use the connection-timeout property for connect() and
  // stop(). If disabled, these operations will be non-blocking. The default
  // is enabled.
  virtual void setConnectionTimeoutEnabled(bool flag) = 0;

  // Returns whether connection timeout is enabled. The default is enabled.
  virtual bool isConnectionTimeoutEnabled() const = 0;
};

}  // namespace internal
}  // namespace network
}  // namespace qindesign
