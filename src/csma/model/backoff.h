
#ifndef BACKOFF_H
#define BACKOFF_H

#include "ns3/nstime.h"
#include "ns3/random-variable-stream.h"

#include <stdint.h>

namespace ns3 {

class Backoff {
public:
  uint32_t m_minSlots;

  uint32_t m_maxSlots;

  uint32_t m_ceiling;

  uint32_t m_maxRetries;

  Time m_slotTime;

  Backoff();
  Backoff(Time slotTime, uint32_t minSlots, uint32_t maxSlots, uint32_t ceiling,
          uint32_t maxRetries);

  Time GetBackoffTime();

  void ResetBackoffTime();

  bool MaxRetriesReached() const;

  void IncrNumRetries();

  int64_t AssignStreams(int64_t stream);

private:
  uint32_t m_numBackoffRetries;

  Ptr<UniformRandomVariable> m_rng;
};

} // namespace ns3

#endif
