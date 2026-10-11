// SPDX-FileCopyrightText: (c) 2024-2026 Shawn Silverman <shawn@pobox.com>
// SPDX-License-Identifier: AGPL-3.0-or-later

// IPOpts.h defines an interface for getting and setting various IP options.
// This file is part of the QNEthernet library.

#pragma once

// C++ includes
#include <cstdint>

namespace qindesign {
namespace network {
namespace internal {

// IPOpts defines getters and setters for various IP options.
class IPOpts {
 public:
  IPOpts() = default;

 protected:
  ~IPOpts() = default;

 public:
  // Sets the Type of Service field in the outgoing IP header. The top 6 bits
  // are the differentiated services code point (DSCP) value, and the bottom
  // 2 bits are the explicit congestion notification (ECN) value.
  //
  // This returns whether successful.
  virtual bool setOutgoingTypeOfService(uint8_t tos) = 0;

  // Returns the Type of Service (DiffServ and ECN) value from the outgoing
  // IP header.
  virtual uint8_t outgoingTypeOfService() const = 0;

  // Sets the TTL field in the outgoing IP header.
  //
  // This returns whether successful.
  virtual bool setOutgoingTTL(uint8_t ttl) = 0;

  // Returns the TTL value from the outgoing IP header.
  virtual uint8_t outgoingTTL() const = 0;
};

}  // namespace internal
}  // namespace network
}  // namespace qindesign
