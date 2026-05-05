// SPDX-FileCopyrightText: (c) 2026 Shawn Silverman <shawn@pobox.com>
// SPDX-License-Identifier: AGPL-3.0-or-later

// OnlineSerialCorrelation.h implements an online algorithm for calculating
// serial correlation at a specific lag.
// This file is part of the QNEthernet library.

#pragma once

// C++ includes
#include <cmath>
#include <cstddef>

#include "util/CircularBuffer.h"

namespace util {

template <typename T>
class OnlineSerialCorrelation {
 public:
  explicit OnlineSerialCorrelation(const size_t lag)
      : lag_(lag),
        queue_{lag + 1} {}

  // Returns the configured lag.
  size_t lag() const {
    return lag_;
  }

  void sample(const T x) {
    // Keep the most recent lag+1 samples, then pair newest with oldest.
    queue_.put() = x;
    if (!queue_.full()) {
      return;
    }

    ++n_;

    const double xNow = static_cast<double>(x);
    const double yLag = static_cast<double>(queue_.get());  // x_{t-lag}

    // Welford-style online updates for lagged pairs (x_t, x_{t-lag})

    const double dx = xNow - mean_x_;
    const double dy = yLag - mean_y_;

    mean_x_ += dx / static_cast<double>(n_);
    mean_y_ += dy / static_cast<double>(n_);

    M2_x_ += dx*(xNow - mean_x_);
    M2_y_ += dy*(yLag - mean_y_);

    C_ += dx*(yLag - mean_y_);
  }

  [[nodiscard]]
  double correlation() const {
    if ((n_ < 2) || (M2_x_ <= 0.0) || (M2_y_ <= 0.0)) {
      return 0.0;
    }

    return C_ / std::sqrt(M2_x_ * M2_y_);
  }

 private:
  const size_t lag_;

  size_t n_      = 0;    // Number of lagged pairs seen so far
  double mean_x_ = 0.0;  // Running mean of x_t
  double mean_y_ = 0.0;  // Running mean of x_{t-lag}
  double M2_x_   = 0.0;  // Sum of squared deviations of x_t
  double M2_y_   = 0.0;  // Sum of squared deviations of x_{t-lag}
  double C_      = 0.0;  // Co-moment sum for covariance numerator

  CircularBuffer<T> queue_;
};

}  // namespace util
