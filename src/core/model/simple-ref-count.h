#ifndef SIMPLE_REF_COUNT_H
#define SIMPLE_REF_COUNT_H

#include "assert.h"
#include "atomic-counter.h"
#include "default-deleter.h"

#include <limits>
#include <stdint.h>

namespace ns3 {

class Empty {};

template <typename T, typename PARENT = Empty,
          typename DELETER = DefaultDeleter<T>>
class SimpleRefCount : public PARENT {
public:
  SimpleRefCount() : m_count(1) {}

  SimpleRefCount(const SimpleRefCount &o [[maybe_unused]]) : m_count(1) {}

  SimpleRefCount &operator=(const SimpleRefCount &o [[maybe_unused]]) {
    return *this;
  }

  inline void Ref() const {
    NS_ASSERT(m_count < std::numeric_limits<uint32_t>::max());
    m_count++;
  }

  inline void Unref() const {
    if (m_count-- == 1) {
#ifdef NS3_MTP
      std::atomic_thread_fence(std::memory_order_acquire);
#endif
      DELETER::Delete(static_cast<T *>(const_cast<SimpleRefCount *>(this)));
    }
  }

  inline uint32_t GetReferenceCount() const { return m_count; }

private:
#ifdef NS3_MTP
  mutable AtomicCounter m_count;
#else
  mutable uint32_t m_count;
#endif
};

} // namespace ns3

#endif
