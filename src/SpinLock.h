// Defines a spin lock using an atomic flag.
// (c) 2021 Shawn Silverman

#ifndef SPINLOCK_H_
#define SPINLOCK_H_

#include <atomic>
#include <mutex>

// Uses RAII to acquire and release a spin lock.
//
// See example from: https://en.cppreference.com/w/cpp/atomic/atomic_flag
class SpinLock final {
 public:
  explicit SpinLock(std::atomic_flag &lock) : lock_(lock) {
    while (lock.test_and_set(std::memory_order_acquire)) {  // acquire lock
      // Since C++20, it is possible to update atomic_flag's
      // value only when there is a chance to acquire the lock.
      // See also: https://stackoverflow.com/questions/62318642
      #if defined(__cpp_lib_atomic_flag_test)
        while (lock.test(std::memory_order_relaxed)) {}     // test lock
      #endif
    }
  }

  ~SpinLock() {
    lock_.clear(std::memory_order_release);
  }

 private:
  std::atomic_flag &lock_;
};

#endif  // SPINLOCK_H_
