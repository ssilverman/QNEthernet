// SPDX-FileCopyrightText: (c) 2022-2026 Shawn Silverman <shawn@pobox.com>
// SPDX-License-Identifier: AGPL-3.0-or-later

// ReceiveQueueing.h defines an interface for using receive queueing.
// See also:
// * [IETF Recommendations Regarding Active Queue Management](https://www.rfc-editor.org/rfc/rfc7567.html)
// This file is part of the QNEthernet library.

#pragma once

#include <cstddef>

namespace qindesign {
namespace network {

// DropPolicy defines all the different receive queue drop policies upon
// queue-full.
enum class DropPolicy {
  kNewest,  // Newest packets are dropped
  kOldest,  // Oldest packets are dropped
};

namespace internal {

// ReceiveQueueing defines receive queue-related functions.
class ReceiveQueueing {
 public:
  ReceiveQueueing() = default;

 protected:
  ~ReceiveQueueing() = default;

 public:
  // Changes the receive queue capacity. This will use a minimum of 1.
  //
  // If the new capacity is smaller than the number of elements in the queue
  // then packets or frames that don't fit are dropped; which are dropped depend
  // on the drop policy.
  virtual void setReceiveQueueCapacity(size_t capacity) = 0;

  // Returns the receive queue capacity.
  virtual size_t receiveQueueCapacity() const = 0;

  // Returns the number of packets or frames currently in the receive queue.
  virtual size_t receiveQueueSize() const = 0;

  // Sets the drop policy. This is what happens when no more packets or
  // frames fit. The default is DropPolicy::kNewest, "tail drop", according to
  // RFC 7567.
  virtual void setDropPolicy(DropPolicy p) = 0;

  // Gets the drop policy. The default is DropPolicy::kNewest, "tail drop",
  // according to RFC 7567.
  virtual DropPolicy dropPolicy() const = 0;
};

}  // namespace internal
}  // namespace network
}  // namespace qindesign
