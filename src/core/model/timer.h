#ifndef TIMER_H
#define TIMER_H

#include "event-id.h"
#include "fatal-error.h"
#include "int-to-type.h"
#include "nstime.h"

namespace ns3 {

class TimerImpl;

class Timer {
public:
  enum DestroyPolicy {
    CANCEL_ON_DESTROY = (1 << 3),
    REMOVE_ON_DESTROY = (1 << 4),
    CHECK_ON_DESTROY = (1 << 5)
  };

  enum State {
    RUNNING,
    EXPIRED,
    SUSPENDED,
  };

  Timer();
  Timer(DestroyPolicy destroyPolicy);
  ~Timer();

  template <typename FN> void SetFunction(FN fn);

  template <typename MEM_PTR, typename OBJ_PTR>
  void SetFunction(MEM_PTR memPtr, OBJ_PTR objPtr);

  template <typename... Ts> void SetArguments(Ts... args);

  void SetDelay(const Time &delay);
  Time GetDelay() const;
  Time GetDelayLeft() const;
  void Cancel();
  void Remove();
  bool IsExpired() const;
  bool IsRunning() const;
  bool IsSuspended() const;
  Timer::State GetState() const;
  void Schedule();
  void Schedule(Time delay);

  void Suspend();
  void Resume();

private:
  static constexpr auto TIMER_SUSPENDED{1 << 7};

  int m_flags;
  Time m_delay;
  EventId m_event;
  TimerImpl *m_impl;
  Time m_delayLeft;
};

} // namespace ns3

#include "timer-impl.h"

namespace ns3 {

template <typename FN> void Timer::SetFunction(FN fn) {
  delete m_impl;
  m_impl = MakeTimerImpl(fn);
}

template <typename MEM_PTR, typename OBJ_PTR>
void Timer::SetFunction(MEM_PTR memPtr, OBJ_PTR objPtr) {
  delete m_impl;
  m_impl = MakeTimerImpl(memPtr, objPtr);
}

template <typename... Ts> void Timer::SetArguments(Ts... args) {
  if (m_impl == nullptr) {
    NS_FATAL_ERROR(
        "You cannot set the arguments of a Timer before setting its function.");
    return;
  }
  m_impl->SetArgs(args...);
}

} // namespace ns3

#endif
