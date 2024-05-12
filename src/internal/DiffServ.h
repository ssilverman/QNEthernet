// SPDX-FileCopyrightText: (c) 2024 Shawn Silverman <shawn@pobox.com>
// SPDX-License-Identifier: AGPL-3.0-or-later

// DiffServ.h defines an interface for DiffServ functions.
// This file is part of the QNEthernet library.

#pragma once

#include <cstdint>

namespace qindesign {
namespace network {
namespace internal {

// DiffServ defines some DiffServ-related methods.
class DiffServ {
 public:
  DiffServ() = default;
  virtual ~DiffServ() = default;

  // Sets the differentiated services (DiffServ, DS) field in the outgoing IP
  // header. The top 6 bits are the differentiated services code point (DSCP)
  // value, and the bottom 2 bits are the explicit congestion notification
  // (ECN) value.
  //
  // This returns whether successful.
  virtual bool setOutgoingDiffServ(uint8_t ds) = 0;

  // Returns the differentiated services (DiffServ) value from the outgoing
  // IP header.
  virtual uint8_t outgoingDiffServ() const = 0;
};

}  // namespace internal
}  // namespace network
}  // namespace qindesign
