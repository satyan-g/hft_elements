#ifndef HFT_ELEMENTS_COMMON_ATOMIC_PRIMITIVE_H
#define HFT_ELEMENTS_COMMON_ATOMIC_PRIMITIVE_H

#include <atomic>

namespace hft_elements {
namespace atomic_primitive {

struct SeqLock {
  std::atomic<uint64_t> seq{0};
};

/*
 * SeqWriteGuard is a helper class to write to a SeqLock.
 * It is used to write to a SeqLock without blocking.
 *
 * Example usage:
 *   {
 *     SeqWriteGuard g(book.lock);
 *     // mutate qty/masks/best freely
 *   } // commit happens even on exceptions/early returns
 */
struct SeqWriteGuard {
  SeqLock& l;
  explicit SeqWriteGuard(SeqLock& l_) : l(l_) {
    l.seq.fetch_add(1, std::memory_order_release); // begin (odd)
  }
  ~SeqWriteGuard() {
    l.seq.fetch_add(1, std::memory_order_release); // commit (even)
  }
  SeqWriteGuard(const SeqWriteGuard&) = delete;
  SeqWriteGuard& operator=(const SeqWriteGuard&) = delete;
};

/*
 * SeqReadAttempt is a helper class to check if a read can be performed.
 * It is used to check if a read can be performed without blocking.
 *
 * Example usage:
 *   for (;;) {
 *     SeqReadAttempt a(foo_lock.lock);
 *     if (!a.can_read()) {
 *       continue;
 *     }
 *
 *     // read fields...
 *     auto foo = calculate_foo();
 *
 *     if (a.validate()) {
 *       return foo;
 *     }
 *   }
 */
struct SeqReadAttempt {
  SeqLock const& l;
  uint64_t s1;

  explicit SeqReadAttempt(SeqLock const& l_) : l(l_) {
    s1 = l.seq.load(std::memory_order_acquire);
  }

  bool can_read() const { return (s1 & 1) == 0; }

  bool validate() const {
    uint64_t s2 = l.seq.load(std::memory_order_acquire);
    return s1 == s2; // and s1 was even when started
  }
};

} // namespace atomic_primitive
} // namespace hft_elements

#endif