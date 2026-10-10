
#ifndef REALTIME_SIMULATOR_IMPL_H
#define REALTIME_SIMULATOR_IMPL_H

#include "assert.h"
#include "event-impl.h"
#include "log.h"
#include "ptr.h"
#include "scheduler.h"
#include "simulator-impl.h"
#include "synchronizer.h"

#include <list>
#include <mutex>
#include <thread>

namespace ns3 {

class RealtimeSimulatorImpl : public SimulatorImpl {
public:
  static TypeId GetTypeId();

  enum SynchronizationMode {
    SYNC_BEST_EFFORT,
    SYNC_HARD_LIMIT,
  };

  RealtimeSimulatorImpl();
  ~RealtimeSimulatorImpl() override;

  void Destroy() override;
  bool IsFinished() const override;
  void Stop() override;
  void Stop(const Time &delay) override;
  EventId Schedule(const Time &delay, EventImpl *event) override;
  void ScheduleWithContext(uint32_t context, const Time &delay,
                           EventImpl *event) override;
  EventId ScheduleNow(EventImpl *event) override;
  EventId ScheduleDestroy(EventImpl *event) override;
  void Remove(const EventId &ev) override;
  void Cancel(const EventId &ev) override;
  bool IsExpired(const EventId &ev) const override;
  void Run() override;
  Time Now() const override;
  Time GetDelayLeft(const EventId &id) const override;
  Time GetMaximumSimulationTime() const override;
  void SetScheduler(ObjectFactory schedulerFactory) override;
  uint32_t GetSystemId() const override;
  uint32_t GetContext() const override;
  uint64_t GetEventCount() const override;

  void ScheduleRealtimeWithContext(uint32_t context, const Time &delay,
                                   EventImpl *event);
  void ScheduleRealtime(const Time &delay, EventImpl *event);
  void ScheduleRealtimeNowWithContext(uint32_t context, EventImpl *event);
  void ScheduleRealtimeNow(EventImpl *event);
  Time RealtimeNow() const;

  void SetSynchronizationMode(RealtimeSimulatorImpl::SynchronizationMode mode);
  RealtimeSimulatorImpl::SynchronizationMode GetSynchronizationMode() const;

  void SetHardLimit(Time limit);
  Time GetHardLimit() const;

private:
  bool Running() const;
  bool Realtime() const;
  uint64_t NextTs() const;
  void ProcessOneEvent();
  void DoDispose() override;

  typedef std::list<EventId> DestroyEvents;
  DestroyEvents m_destroyEvents;
  bool m_stop;
  bool m_running;

  Ptr<Scheduler> m_events;
  int m_unscheduledEvents;
  uint32_t m_uid;
  uint32_t m_currentUid;
  uint64_t m_currentTs;
  uint32_t m_currentContext;
  uint64_t m_eventCount;

  mutable std::mutex m_mutex;

  Ptr<Synchronizer> m_synchronizer;

  SynchronizationMode m_synchronizationMode;

  Time m_hardLimit;

  std::thread::id m_main;
};

} // namespace ns3

#endif
