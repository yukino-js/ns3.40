
#ifndef DYNAMIC_QUEUE_LIMITS_H
#define DYNAMIC_QUEUE_LIMITS_H

#include "queue-limits.h"

#include "ns3/nstime.h"
#include "ns3/traced-value.h"

#include <limits.h>

namespace ns3 {

class DynamicQueueLimits : public QueueLimits {
public:
  static TypeId GetTypeId();

  DynamicQueueLimits();
  ~DynamicQueueLimits() override;

  void Reset() override;
  void Completed(uint32_t count) override;
  int32_t Available() const override;
  void Queued(uint32_t count) override;

private:
  int32_t Posdiff(int32_t a, int32_t b);

  uint32_t m_numQueued{0};
  uint32_t m_adjLimit{0};
  uint32_t m_lastObjCnt{0};

  TracedValue<uint32_t> m_limit;
  uint32_t m_numCompleted{0};

  uint32_t m_prevOvlimit{0};
  uint32_t m_prevNumQueued{0};
  uint32_t m_prevLastObjCnt{0};

  uint32_t m_lowestSlack{std::numeric_limits<uint32_t>::max()};
  Time m_slackStartTime{Seconds(0)};

  uint32_t m_maxLimit;
  uint32_t m_minLimit;
  Time m_slackHoldTime;
};

} // namespace ns3

#endif
