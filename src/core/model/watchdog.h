#ifndef WATCHDOG_H
#define WATCHDOG_H

#include "event-id.h"
#include "nstime.h"

namespace ns3 {

class TimerImpl;

class Watchdog {
public:
  Watchdog();
  ~Watchdog();

  void Ping(Time delay);

  template <typename FN> void SetFunction(FN fn);

  template <typename MEM_PTR, typename OBJ_PTR>
  void SetFunction(MEM_PTR memPtr, OBJ_PTR objPtr);

  template <typename... Ts> void SetArguments(Ts &&...args);

private:
  void Expire();
  TimerImpl *m_impl;
  EventId m_event;
  Time m_end;
};

} // namespace ns3

#include "timer-impl.h"

namespace ns3 {

template <typename FN> void Watchdog::SetFunction(FN fn) {
  delete m_impl;
  m_impl = MakeTimerImpl(fn);
}

template <typename MEM_PTR, typename OBJ_PTR>
void Watchdog::SetFunction(MEM_PTR memPtr, OBJ_PTR objPtr) {
  delete m_impl;
  m_impl = MakeTimerImpl(memPtr, objPtr);
}

template <typename... Ts> void Watchdog::SetArguments(Ts &&...args) {
  if (m_impl == nullptr) {
    NS_FATAL_ERROR("You cannot set the arguments of a Watchdog before setting "
                   "its function.");
    return;
  }
  m_impl->SetArgs(std::forward<Ts>(args)...);
}

} // namespace ns3

#endif
