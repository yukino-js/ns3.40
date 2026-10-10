
#ifndef DEFAULT_SIMULATOR_IMPL_H
#define DEFAULT_SIMULATOR_IMPL_H

#include "simulator-impl.h"

#include <list>
#include <mutex>
#include <thread>

namespace ns3 {

class Scheduler;

class DefaultSimulatorImpl : public SimulatorImpl {
public:
  static TypeId GetTypeId();

  DefaultSimulatorImpl();
  ~DefaultSimulatorImpl() override;

  void Destroy() override;
  bool IsFinished() const override;
  void Stop() override;
  void Stop(const Time &delay) override;
  EventId Schedule(const Time &delay, EventImpl *event) override;
  void ScheduleWithContext(uint32_t context, const Time &delay,
                           EventImpl *event) override;
  EventId ScheduleNow(EventImpl *event) override;
  EventId ScheduleDestroy(EventImpl *event) override;
  void Remove(const EventId &id) override;
  void Cancel(const EventId &id) override;
  bool IsExpired(const EventId &id) const override;
  void Run() override;
  Time Now() const override;
  Time GetDelayLeft(const EventId &id) const override;
  Time GetMaximumSimulationTime() const override;
  void SetScheduler(ObjectFactory schedulerFactory) override;
  uint32_t GetSystemId() const override;
  uint32_t GetContext() const override;
  uint64_t GetEventCount() const override;

private:
  void DoDispose() override;

  void ProcessOneEvent();
  void ProcessEventsWithContext();

  struct EventWithContext {
    uint32_t context;
    uint64_t timestamp;
    EventImpl *event;
  };

  typedef std::list<EventWithContext> EventsWithContext;
  EventsWithContext m_eventsWithContext;
  bool m_eventsWithContextEmpty;
  std::mutex m_eventsWithContextMutex;

  typedef std::list<EventId> DestroyEvents;
  DestroyEvents m_destroyEvents;
  bool m_stop;
  Ptr<Scheduler> m_events;

  uint32_t m_uid;
  uint32_t m_currentUid;
  uint64_t m_currentTs;
  uint32_t m_currentContext;
  uint64_t m_eventCount;
  int m_unscheduledEvents;

  std::thread::id m_mainThreadId;
};

} // namespace ns3

#endif
