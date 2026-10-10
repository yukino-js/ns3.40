
#ifndef TRICKLE_TIMER_H
#define TRICKLE_TIMER_H

#include "event-id.h"
#include "nstime.h"
#include "random-variable-stream.h"

namespace ns3 {

class TimerImpl;

class TrickleTimer {
public:
  TrickleTimer();

  TrickleTimer(Time minInterval, uint8_t doublings, uint16_t redundancy);

  ~TrickleTimer();

  int64_t AssignStreams(int64_t streamNum);

  void SetParameters(Time minInterval, uint8_t doublings, uint16_t redundancy);

  Time GetMinInterval() const;

  Time GetMaxInterval() const;

  uint8_t GetDoublings() const;

  uint16_t GetRedundancy() const;

  Time GetDelayLeft() const;

  Time GetIntervalLeft() const;

  void Enable();

  void ConsistentEvent();

  void InconsistentEvent();

  void Reset();

  void Stop();

  template <typename FN> void SetFunction(FN fn);

  template <typename MEM_PTR, typename OBJ_PTR>
  void SetFunction(MEM_PTR memPtr, OBJ_PTR objPtr);

  template <typename... Ts> void SetArguments(Ts &&...args);

private:
  void TimerExpire();
  void IntervalExpire();

  TimerImpl *m_impl;

  EventId m_timerExpiration;

  EventId m_intervalExpiration;

  Time m_minInterval;
  Time m_maxInterval;
  uint16_t m_redundancy;

  uint64_t m_ticks;
  Time m_currentInterval;
  uint16_t m_counter;

  Ptr<UniformRandomVariable> m_uniRand;
};

} // namespace ns3

#include "timer-impl.h"

namespace ns3 {

template <typename FN> void TrickleTimer::SetFunction(FN fn) {
  delete m_impl;
  m_impl = MakeTimerImpl(fn);
}

template <typename MEM_PTR, typename OBJ_PTR>
void TrickleTimer::SetFunction(MEM_PTR memPtr, OBJ_PTR objPtr) {
  delete m_impl;
  m_impl = MakeTimerImpl(memPtr, objPtr);
}

template <typename... Ts> void TrickleTimer::SetArguments(Ts &&...args) {
  if (m_impl == nullptr) {
    NS_FATAL_ERROR("You cannot set the arguments of a TrickleTimer before "
                   "setting its function.");
    return;
  }
  m_impl->SetArgs(std::forward<Ts>(args)...);
}

} // namespace ns3

#endif
