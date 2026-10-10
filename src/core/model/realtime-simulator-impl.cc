
#include "realtime-simulator-impl.h"

#include "assert.h"
#include "boolean.h"
#include "enum.h"
#include "event-impl.h"
#include "fatal-error.h"
#include "log.h"
#include "pointer.h"
#include "ptr.h"
#include "scheduler.h"
#include "simulator.h"
#include "synchronizer.h"
#include "wall-clock-synchronizer.h"

#include <cmath>
#include <mutex>
#include <thread>

namespace ns3 {

NS_LOG_COMPONENT_DEFINE("RealtimeSimulatorImpl");

NS_OBJECT_ENSURE_REGISTERED(RealtimeSimulatorImpl);

TypeId RealtimeSimulatorImpl::GetTypeId() {
  static TypeId tid =
      TypeId("ns3::RealtimeSimulatorImpl")
          .SetParent<SimulatorImpl>()
          .SetGroupName("Core")
          .AddConstructor<RealtimeSimulatorImpl>()
          .AddAttribute(
              "SynchronizationMode",
              "What to do if the simulation cannot keep up with real time.",
              EnumValue(SYNC_BEST_EFFORT),
              MakeEnumAccessor(&RealtimeSimulatorImpl::SetSynchronizationMode),
              MakeEnumChecker(SYNC_BEST_EFFORT, "BestEffort", SYNC_HARD_LIMIT,
                              "HardLimit"))
          .AddAttribute(
              "HardLimit",
              "Maximum acceptable real-time jitter (used in conjunction with "
              "SynchronizationMode=HardLimit)",
              TimeValue(Seconds(0.1)),
              MakeTimeAccessor(&RealtimeSimulatorImpl::m_hardLimit),
              MakeTimeChecker());
  return tid;
}

RealtimeSimulatorImpl::RealtimeSimulatorImpl() {
  NS_LOG_FUNCTION(this);

  m_stop = false;
  m_running = false;
  m_uid = EventId::UID::VALID;
  m_currentUid = EventId::UID::INVALID;
  m_currentTs = 0;
  m_currentContext = Simulator::NO_CONTEXT;
  m_unscheduledEvents = 0;
  m_eventCount = 0;

  m_main = std::this_thread::get_id();

  m_synchronizer = CreateObject<WallClockSynchronizer>();
}

RealtimeSimulatorImpl::~RealtimeSimulatorImpl() { NS_LOG_FUNCTION(this); }

void RealtimeSimulatorImpl::DoDispose() {
  NS_LOG_FUNCTION(this);
  while (!m_events->IsEmpty()) {
    Scheduler::Event next = m_events->RemoveNext();
    next.impl->Unref();
  }
  m_events = nullptr;
  m_synchronizer = nullptr;
  SimulatorImpl::DoDispose();
}

void RealtimeSimulatorImpl::Destroy() {
  NS_LOG_FUNCTION(this);

  while (!m_destroyEvents.empty()) {
    Ptr<EventImpl> ev = m_destroyEvents.front().PeekEventImpl();
    m_destroyEvents.pop_front();
    NS_LOG_LOGIC("handle destroy " << ev);
    if (!ev->IsCancelled()) {
      ev->Invoke();
    }
  }
}

void RealtimeSimulatorImpl::SetScheduler(ObjectFactory schedulerFactory) {
  NS_LOG_FUNCTION(this << schedulerFactory);

  Ptr<Scheduler> scheduler = schedulerFactory.Create<Scheduler>();

  {
    std::unique_lock lock{m_mutex};

    if (m_events) {
      while (!m_events->IsEmpty()) {
        Scheduler::Event next = m_events->RemoveNext();
        scheduler->Insert(next);
      }
    }
    m_events = scheduler;
  }
}

void RealtimeSimulatorImpl::ProcessOneEvent() {

  for (;;) {
    uint64_t tsDelay = 0;
    uint64_t tsNext = 0;

    uint64_t tsNow;

    {
      std::unique_lock lock{m_mutex};
      NS_ASSERT_MSG(m_synchronizer->Realtime(),
                    "RealtimeSimulatorImpl::ProcessOneEvent (): Synchronizer "
                    "reports not Realtime ()");

      tsNow = m_synchronizer->GetCurrentRealtime();
      tsNext = NextTs();

      if (tsNext <= tsNow) {
        tsDelay = 0;
      } else {
        tsDelay = tsNext - tsNow;
      }

      m_synchronizer->SetCondition(false);
    }

    if (m_synchronizer->Synchronize(tsNow, tsDelay)) {
      NS_LOG_LOGIC("Interrupted ...");
      break;
    }
  }

  Scheduler::Event next;

  {
    std::unique_lock lock{m_mutex};

    NS_ASSERT_MSG(
        m_events->IsEmpty() == false,
        "RealtimeSimulatorImpl::ProcessOneEvent(): event queue is empty");
    next = m_events->RemoveNext();

    PreEventHook(
        EventId(next.impl, next.key.m_ts, next.key.m_context, next.key.m_uid));

    m_unscheduledEvents--;
    m_eventCount++;

    NS_ASSERT_MSG(next.key.m_ts >= m_currentTs,
                  "RealtimeSimulatorImpl::ProcessOneEvent(): "
                  "next.GetTs() earlier than m_currentTs (list order error)");
    NS_LOG_LOGIC("handle " << next.key.m_ts);

    m_currentTs = next.key.m_ts;
    m_currentContext = next.key.m_context;
    m_currentUid = next.key.m_uid;

    if (m_synchronizationMode == SYNC_HARD_LIMIT) {
      uint64_t tsFinal = m_synchronizer->GetCurrentRealtime();
      uint64_t tsJitter;

      if (tsFinal >= m_currentTs) {
        tsJitter = tsFinal - m_currentTs;
      } else {
        tsJitter = m_currentTs - tsFinal;
      }

      if (tsJitter > static_cast<uint64_t>(m_hardLimit.GetTimeStep())) {
        NS_FATAL_ERROR("RealtimeSimulatorImpl::ProcessOneEvent (): "
                       "Hard real-time limit exceeded (jitter = "
                       << tsJitter << ")");
      }
    }
  }

  EventImpl *event = next.impl;
  m_synchronizer->EventStart();
  event->Invoke();
  m_synchronizer->EventEnd();
  event->Unref();
}

bool RealtimeSimulatorImpl::IsFinished() const {
  bool rc;
  {
    std::unique_lock lock{m_mutex};
    rc = m_events->IsEmpty() || m_stop;
  }

  return rc;
}

uint64_t RealtimeSimulatorImpl::NextTs() const {
  NS_ASSERT_MSG(m_events->IsEmpty() == false,
                "RealtimeSimulatorImpl::NextTs(): event queue is empty");
  Scheduler::Event ev = m_events->PeekNext();
  return ev.key.m_ts;
}

void RealtimeSimulatorImpl::Run() {
  NS_LOG_FUNCTION(this);

  NS_ASSERT_MSG(m_running == false,
                "RealtimeSimulatorImpl::Run(): Simulator already running");

  m_main = std::this_thread::get_id();

  m_stop = false;
  m_running = true;
  m_synchronizer->SetOrigin(m_currentTs);

  uint64_t tsNow = 0;
  uint64_t tsDelay = 1000000000;

  while (!m_stop) {
    bool process = false;
    {
      std::unique_lock lock{m_mutex};

      if (!m_events->IsEmpty()) {
        process = true;
      } else {
        tsNow = m_synchronizer->GetCurrentRealtime();
      }
    }

    if (!process) {
      tsNow = m_synchronizer->Synchronize(tsNow, tsDelay);

      continue;
    }

    ProcessOneEvent();
  }

  {
    std::unique_lock lock{m_mutex};

    NS_ASSERT_MSG(
        m_events->IsEmpty() == false || m_unscheduledEvents == 0,
        "RealtimeSimulatorImpl::Run(): Empty queue and unprocessed events");
  }

  m_running = false;
}

bool RealtimeSimulatorImpl::Running() const { return m_running; }

bool RealtimeSimulatorImpl::Realtime() const {
  return m_synchronizer->Realtime();
}

void RealtimeSimulatorImpl::Stop() {
  NS_LOG_FUNCTION(this);
  m_stop = true;
}

void RealtimeSimulatorImpl::Stop(const Time &delay) {
  NS_LOG_FUNCTION(this << delay);
  Simulator::Schedule(delay, &Simulator::Stop);
}

EventId RealtimeSimulatorImpl::Schedule(const Time &delay, EventImpl *impl) {
  NS_LOG_FUNCTION(this << delay << impl);

  Scheduler::Event ev;
  {
    std::unique_lock lock{m_mutex};
    Time tAbsolute = Simulator::Now() + delay;
    NS_ASSERT_MSG(delay.IsPositive(),
                  "RealtimeSimulatorImpl::Schedule(): Negative delay");
    ev.impl = impl;
    ev.key.m_ts = (uint64_t)tAbsolute.GetTimeStep();
    ev.key.m_context = GetContext();
    ev.key.m_uid = m_uid;
    m_uid++;
    m_unscheduledEvents++;
    m_events->Insert(ev);
    m_synchronizer->Signal();
  }

  return EventId(impl, ev.key.m_ts, ev.key.m_context, ev.key.m_uid);
}

void RealtimeSimulatorImpl::ScheduleWithContext(uint32_t context,
                                                const Time &delay,
                                                EventImpl *impl) {
  NS_LOG_FUNCTION(this << context << delay << impl);

  {
    std::unique_lock lock{m_mutex};
    uint64_t ts;

    if (m_main == std::this_thread::get_id()) {
      ts = m_currentTs + delay.GetTimeStep();
    } else {
      ts = m_running ? m_synchronizer->GetCurrentRealtime() : m_currentTs;
      ts += delay.GetTimeStep();
    }

    NS_ASSERT_MSG(ts >= m_currentTs, "RealtimeSimulatorImpl::ScheduleRealtime()"
                                     ": schedule for time < m_currentTs");
    Scheduler::Event ev;
    ev.impl = impl;
    ev.key.m_ts = ts;
    ev.key.m_context = context;
    ev.key.m_uid = m_uid;
    m_uid++;
    m_unscheduledEvents++;
    m_events->Insert(ev);
    m_synchronizer->Signal();
  }
}

EventId RealtimeSimulatorImpl::ScheduleNow(EventImpl *impl) {
  NS_LOG_FUNCTION(this << impl);
  return Schedule(Time(0), impl);
}

Time RealtimeSimulatorImpl::Now() const { return TimeStep(m_currentTs); }

void RealtimeSimulatorImpl::ScheduleRealtimeWithContext(uint32_t context,
                                                        const Time &time,
                                                        EventImpl *impl) {
  NS_LOG_FUNCTION(this << context << time << impl);

  {
    std::unique_lock lock{m_mutex};

    uint64_t ts = m_synchronizer->GetCurrentRealtime() + time.GetTimeStep();
    NS_ASSERT_MSG(ts >= m_currentTs, "RealtimeSimulatorImpl::ScheduleRealtime()"
                                     ": schedule for time < m_currentTs");
    Scheduler::Event ev;
    ev.impl = impl;
    ev.key.m_ts = ts;
    ev.key.m_uid = m_uid;
    m_uid++;
    m_unscheduledEvents++;
    m_events->Insert(ev);
    m_synchronizer->Signal();
  }
}

void RealtimeSimulatorImpl::ScheduleRealtime(const Time &time,
                                             EventImpl *impl) {
  NS_LOG_FUNCTION(this << time << impl);
  ScheduleRealtimeWithContext(GetContext(), time, impl);
}

void RealtimeSimulatorImpl::ScheduleRealtimeNowWithContext(uint32_t context,
                                                           EventImpl *impl) {
  NS_LOG_FUNCTION(this << context << impl);
  {
    std::unique_lock lock{m_mutex};

    uint64_t ts =
        m_running ? m_synchronizer->GetCurrentRealtime() : m_currentTs;
    NS_ASSERT_MSG(ts >= m_currentTs,
                  "RealtimeSimulatorImpl::ScheduleRealtimeNowWithContext(): "
                  "schedule for time "
                  "< m_currentTs");
    Scheduler::Event ev;
    ev.impl = impl;
    ev.key.m_ts = ts;
    ev.key.m_uid = m_uid;
    ev.key.m_context = context;
    m_uid++;
    m_unscheduledEvents++;
    m_events->Insert(ev);
    m_synchronizer->Signal();
  }
}

void RealtimeSimulatorImpl::ScheduleRealtimeNow(EventImpl *impl) {
  NS_LOG_FUNCTION(this << impl);
  ScheduleRealtimeNowWithContext(GetContext(), impl);
}

Time RealtimeSimulatorImpl::RealtimeNow() const {
  return TimeStep(m_synchronizer->GetCurrentRealtime());
}

EventId RealtimeSimulatorImpl::ScheduleDestroy(EventImpl *impl) {
  NS_LOG_FUNCTION(this << impl);

  EventId id;
  {
    std::unique_lock lock{m_mutex};

    id = EventId(Ptr<EventImpl>(impl, false), m_currentTs, 0xffffffff,
                 EventId::UID::DESTROY);
    m_destroyEvents.push_back(id);
    m_uid++;
  }

  return id;
}

Time RealtimeSimulatorImpl::GetDelayLeft(const EventId &id) const {
  if (IsExpired(id)) {
    return TimeStep(0);
  }

  return TimeStep(id.GetTs() - m_currentTs);
}

void RealtimeSimulatorImpl::Remove(const EventId &id) {
  if (id.GetUid() == EventId::UID::DESTROY) {
    for (auto i = m_destroyEvents.begin(); i != m_destroyEvents.end(); i++) {
      if (*i == id) {
        m_destroyEvents.erase(i);
        break;
      }
    }
    return;
  }
  if (IsExpired(id)) {
    return;
  }

  {
    std::unique_lock lock{m_mutex};

    Scheduler::Event event;
    event.impl = id.PeekEventImpl();
    event.key.m_ts = id.GetTs();
    event.key.m_context = id.GetContext();
    event.key.m_uid = id.GetUid();

    m_events->Remove(event);
    m_unscheduledEvents--;
    event.impl->Cancel();
    event.impl->Unref();
  }
}

void RealtimeSimulatorImpl::Cancel(const EventId &id) {
  if (!IsExpired(id)) {
    id.PeekEventImpl()->Cancel();
  }
}

bool RealtimeSimulatorImpl::IsExpired(const EventId &id) const {
  if (id.GetUid() == EventId::UID::DESTROY) {
    if (id.PeekEventImpl() == nullptr || id.PeekEventImpl()->IsCancelled()) {
      return true;
    }
    for (auto i = m_destroyEvents.begin(); i != m_destroyEvents.end(); i++) {
      if (*i == id) {
        return false;
      }
    }
    return true;
  }

  return id.PeekEventImpl() == nullptr || id.GetTs() < m_currentTs ||
         (id.GetTs() == m_currentTs && id.GetUid() <= m_currentUid) ||
         id.PeekEventImpl()->IsCancelled();
}

Time RealtimeSimulatorImpl::GetMaximumSimulationTime() const {
  return TimeStep(0x7fffffffffffffffLL);
}

uint32_t RealtimeSimulatorImpl::GetSystemId() const { return 0; }

uint32_t RealtimeSimulatorImpl::GetContext() const { return m_currentContext; }

uint64_t RealtimeSimulatorImpl::GetEventCount() const { return m_eventCount; }

void RealtimeSimulatorImpl::SetSynchronizationMode(SynchronizationMode mode) {
  NS_LOG_FUNCTION(this << mode);
  m_synchronizationMode = mode;
}

RealtimeSimulatorImpl::SynchronizationMode
RealtimeSimulatorImpl::GetSynchronizationMode() const {
  NS_LOG_FUNCTION(this);
  return m_synchronizationMode;
}

void RealtimeSimulatorImpl::SetHardLimit(Time limit) {
  NS_LOG_FUNCTION(this << limit);
  m_hardLimit = limit;
}

Time RealtimeSimulatorImpl::GetHardLimit() const {
  NS_LOG_FUNCTION(this);
  return m_hardLimit;
}

} // namespace ns3
