
#ifndef WALL_CLOCK_CLOCK_SYNCHRONIZER_H
#define WALL_CLOCK_CLOCK_SYNCHRONIZER_H

#include "synchronizer.h"

#include <condition_variable>
#include <mutex>

namespace ns3 {

class WallClockSynchronizer : public Synchronizer {
public:
  static TypeId GetTypeId();

  WallClockSynchronizer();
  ~WallClockSynchronizer() override;

  static const uint64_t US_PER_NS = (uint64_t)1000;
  static const uint64_t US_PER_SEC = (uint64_t)1000000;
  static const uint64_t NS_PER_SEC = (uint64_t)1000000000;

protected:
  bool SpinWait(uint64_t ns);
  bool SleepWait(uint64_t ns);

  void DoSetOrigin(uint64_t ns) override;
  bool DoRealtime() override;
  uint64_t DoGetCurrentRealtime() override;
  bool DoSynchronize(uint64_t nsCurrent, uint64_t nsDelay) override;
  void DoSignal() override;
  void DoSetCondition(bool cond) override;
  int64_t DoGetDrift(uint64_t ns) override;
  void DoEventStart() override;
  uint64_t DoEventEnd() override;

  uint64_t DriftCorrect(uint64_t nsNow, uint64_t nsDelay);

  uint64_t GetRealtime();
  uint64_t GetNormalizedRealtime();

  uint64_t m_jiffy;
  uint64_t m_nsEventStart;

  std::condition_variable m_conditionVariable;
  std::mutex m_mutex;
  bool m_condition;
};

} // namespace ns3

#endif
