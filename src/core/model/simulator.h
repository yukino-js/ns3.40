
#ifndef SIMULATOR_H
#define SIMULATOR_H

#include "event-id.h"
#include "event-impl.h"
#include "make-event.h"
#include "nstime.h"
#include "object-factory.h"

#include <stdint.h>
#include <string>

namespace ns3 {

class SimulatorImpl;
class Scheduler;

class Simulator {
public:
  Simulator() = delete;
  ~Simulator() = delete;

  static void SetImplementation(Ptr<SimulatorImpl> impl);

  static Ptr<SimulatorImpl> GetImplementation();

  static void SetScheduler(ObjectFactory schedulerFactory);

  static void Destroy();

  static bool IsFinished();

  static void Run();

  static void Stop();

  static void Stop(const Time &delay);

  static uint32_t GetContext();

  enum : uint32_t { NO_CONTEXT = 0xffffffff };

  static uint64_t GetEventCount();

  template <
      typename FUNC,
      std::enable_if_t<!std::is_convertible_v<FUNC, Ptr<EventImpl>>, int> = 0,
      std::enable_if_t<!std::is_function_v<std::remove_pointer_t<FUNC>>, int> =
          0,
      typename... Ts>
  static EventId Schedule(const Time &delay, FUNC f, Ts &&...args);

  template <typename... Us, typename... Ts>
  static EventId Schedule(const Time &delay, void (*f)(Us...), Ts &&...args);

  template <
      typename FUNC,
      std::enable_if_t<!std::is_convertible_v<FUNC, Ptr<EventImpl>>, int> = 0,
      std::enable_if_t<!std::is_function_v<std::remove_pointer_t<FUNC>>, int> =
          0,
      typename... Ts>
  static void ScheduleWithContext(uint32_t context, const Time &delay, FUNC f,
                                  Ts &&...args);

  template <typename... Us, typename... Ts>
  static void ScheduleWithContext(uint32_t context, const Time &delay,
                                  void (*f)(Us...), Ts &&...args);

  template <
      typename FUNC,
      std::enable_if_t<!std::is_convertible_v<FUNC, Ptr<EventImpl>>, int> = 0,
      std::enable_if_t<!std::is_function_v<std::remove_pointer_t<FUNC>>, int> =
          0,
      typename... Ts>
  static EventId ScheduleNow(FUNC f, Ts &&...args);

  template <typename... Us, typename... Ts>
  static EventId ScheduleNow(void (*f)(Us...), Ts &&...args);

  template <
      typename FUNC,
      std::enable_if_t<!std::is_convertible_v<FUNC, Ptr<EventImpl>>, int> = 0,
      std::enable_if_t<!std::is_function_v<std::remove_pointer_t<FUNC>>, int> =
          0,
      typename... Ts>
  static EventId ScheduleDestroy(FUNC f, Ts &&...args);

  template <typename... Us, typename... Ts>
  static EventId ScheduleDestroy(void (*f)(Us...), Ts &&...args);

  static void Remove(const EventId &id);

  static void Cancel(const EventId &id);

  static bool IsExpired(const EventId &id);

  static Time Now();

  static Time GetDelayLeft(const EventId &id);

  static Time GetMaximumSimulationTime();

  static EventId Schedule(const Time &delay, const Ptr<EventImpl> &event);

  static void ScheduleWithContext(uint32_t context, const Time &delay,
                                  EventImpl *event);

  static EventId ScheduleDestroy(const Ptr<EventImpl> &event);

  static EventId ScheduleNow(const Ptr<EventImpl> &event);

  static uint32_t GetSystemId();

private:
  static EventId DoSchedule(const Time &delay, EventImpl *event);
  static EventId DoScheduleNow(EventImpl *event);
  static EventId DoScheduleDestroy(EventImpl *event);
};

Time Now();

} // namespace ns3

namespace ns3 {

template <
    typename FUNC,
    std::enable_if_t<!std::is_convertible_v<FUNC, Ptr<EventImpl>>, int>,
    std::enable_if_t<!std::is_function_v<std::remove_pointer_t<FUNC>>, int>,
    typename... Ts>
EventId Simulator::Schedule(const Time &delay, FUNC f, Ts &&...args) {
  return DoSchedule(delay, MakeEvent(f, std::forward<Ts>(args)...));
}

template <typename... Us, typename... Ts>
EventId Simulator::Schedule(const Time &delay, void (*f)(Us...), Ts &&...args) {
  return DoSchedule(delay, MakeEvent(f, std::forward<Ts>(args)...));
}

template <
    typename FUNC,
    std::enable_if_t<!std::is_convertible_v<FUNC, Ptr<EventImpl>>, int>,
    std::enable_if_t<!std::is_function_v<std::remove_pointer_t<FUNC>>, int>,
    typename... Ts>
void Simulator::ScheduleWithContext(uint32_t context, const Time &delay, FUNC f,
                                    Ts &&...args) {
  return ScheduleWithContext(context, delay,
                             MakeEvent(f, std::forward<Ts>(args)...));
}

template <typename... Us, typename... Ts>
void Simulator::ScheduleWithContext(uint32_t context, const Time &delay,
                                    void (*f)(Us...), Ts &&...args) {
  return ScheduleWithContext(context, delay,
                             MakeEvent(f, std::forward<Ts>(args)...));
}

template <
    typename FUNC,
    std::enable_if_t<!std::is_convertible_v<FUNC, Ptr<EventImpl>>, int>,
    std::enable_if_t<!std::is_function_v<std::remove_pointer_t<FUNC>>, int>,
    typename... Ts>
EventId Simulator::ScheduleNow(FUNC f, Ts &&...args) {
  return DoScheduleNow(MakeEvent(f, std::forward<Ts>(args)...));
}

template <typename... Us, typename... Ts>
EventId Simulator::ScheduleNow(void (*f)(Us...), Ts &&...args) {
  return DoScheduleNow(MakeEvent(f, std::forward<Ts>(args)...));
}

template <
    typename FUNC,
    std::enable_if_t<!std::is_convertible_v<FUNC, Ptr<EventImpl>>, int>,
    std::enable_if_t<!std::is_function_v<std::remove_pointer_t<FUNC>>, int>,
    typename... Ts>
EventId Simulator::ScheduleDestroy(FUNC f, Ts &&...args) {
  return DoScheduleDestroy(MakeEvent(f, std::forward<Ts>(args)...));
}

template <typename... Us, typename... Ts>
EventId Simulator::ScheduleDestroy(void (*f)(Us...), Ts &&...args) {
  return DoScheduleDestroy(MakeEvent(f, std::forward<Ts>(args)...));
}

} // namespace ns3

#endif
