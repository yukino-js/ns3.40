#include "simulator.h"

#include "assert.h"
#include "des-metrics.h"
#include "event-impl.h"
#include "global-value.h"
#include "log.h"
#include "map-scheduler.h"
#include "object-factory.h"
#include "ptr.h"
#include "scheduler.h"
#include "simulator-impl.h"
#include "string.h"

#include "ns3/core-config.h"

#include <cmath>
#include <fstream>
#include <iomanip>
#include <iostream>
#include <list>
#include <vector>

namespace ns3 {

NS_LOG_COMPONENT_DEFINE("Simulator");

static GlobalValue g_simTypeImpl =
    GlobalValue("SimulatorImplementationType",
                "The object class to use as the simulator implementation",
                StringValue("ns3::DefaultSimulatorImpl"), MakeStringChecker());

static GlobalValue g_schedTypeImpl = GlobalValue(
    "SchedulerType", "The object class to use as the scheduler implementation",
    TypeIdValue(MapScheduler::GetTypeId()), MakeTypeIdChecker());

static SimulatorImpl **PeekImpl() {
  static SimulatorImpl *impl = nullptr;
  return &impl;
}

static SimulatorImpl *GetImpl() {
  SimulatorImpl **pimpl = PeekImpl();
  if (*pimpl == nullptr) {
    {
      ObjectFactory factory;
      StringValue s;

      g_simTypeImpl.GetValue(s);
      factory.SetTypeId(s.Get());
      *pimpl = GetPointer(factory.Create<SimulatorImpl>());
    }
    {
      ObjectFactory factory;
      StringValue s;
      g_schedTypeImpl.GetValue(s);
      factory.SetTypeId(s.Get());
      (*pimpl)->SetScheduler(factory);
    }

    LogSetTimePrinter(&DefaultTimePrinter);
    LogSetNodePrinter(&DefaultNodePrinter);
  }
  return *pimpl;
}

void Simulator::Destroy() {
  NS_LOG_FUNCTION_NOARGS();

  SimulatorImpl **pimpl = PeekImpl();
  if (*pimpl == nullptr) {
    return;
  }
  LogSetTimePrinter(nullptr);
  LogSetNodePrinter(nullptr);
  (*pimpl)->Destroy();
  (*pimpl)->Unref();
  *pimpl = nullptr;
}

void Simulator::SetScheduler(ObjectFactory schedulerFactory) {
  NS_LOG_FUNCTION(schedulerFactory);
  GetImpl()->SetScheduler(schedulerFactory);
}

bool Simulator::IsFinished() {
  NS_LOG_FUNCTION_NOARGS();
  return GetImpl()->IsFinished();
}

void Simulator::Run() {
  NS_LOG_FUNCTION_NOARGS();
  Time::ClearMarkedTimes();
  GetImpl()->Run();
}

void Simulator::Stop() {
  NS_LOG_FUNCTION_NOARGS();
  NS_LOG_LOGIC("stop");
  GetImpl()->Stop();
}

void Simulator::Stop(const Time &delay) {
  NS_LOG_FUNCTION(delay);
  GetImpl()->Stop(delay);
}

Time Simulator::Now() { return GetImpl()->Now(); }

Time Simulator::GetDelayLeft(const EventId &id) {
  NS_LOG_FUNCTION(&id);
  return GetImpl()->GetDelayLeft(id);
}

EventId Simulator::Schedule(const Time &delay, const Ptr<EventImpl> &event) {
  return DoSchedule(delay, GetPointer(event));
}

EventId Simulator::ScheduleNow(const Ptr<EventImpl> &ev) {
  return DoScheduleNow(GetPointer(ev));
}

void Simulator::ScheduleWithContext(uint32_t context, const Time &delay,
                                    EventImpl *impl) {
#ifdef ENABLE_DES_METRICS
  DesMetrics::Get()->TraceWithContext(context, Now(), delay);
#endif
  return GetImpl()->ScheduleWithContext(context, delay, impl);
}

EventId Simulator::ScheduleDestroy(const Ptr<EventImpl> &ev) {
  return DoScheduleDestroy(GetPointer(ev));
}

EventId Simulator::DoSchedule(const Time &time, EventImpl *impl) {
#ifdef ENABLE_DES_METRICS
  DesMetrics::Get()->Trace(Now(), time);
#endif
  return GetImpl()->Schedule(time, impl);
}

EventId Simulator::DoScheduleNow(EventImpl *impl) {
#ifdef ENABLE_DES_METRICS
  DesMetrics::Get()->Trace(Now(), Time(0));
#endif
  return GetImpl()->ScheduleNow(impl);
}

EventId Simulator::DoScheduleDestroy(EventImpl *impl) {
  return GetImpl()->ScheduleDestroy(impl);
}

void Simulator::Remove(const EventId &id) {
  if (*PeekImpl() == nullptr) {
    return;
  }
  return GetImpl()->Remove(id);
}

void Simulator::Cancel(const EventId &id) {
  if (*PeekImpl() == nullptr) {
    return;
  }
  return GetImpl()->Cancel(id);
}

bool Simulator::IsExpired(const EventId &id) {
  if (*PeekImpl() == nullptr) {
    return true;
  }
  return GetImpl()->IsExpired(id);
}

Time Now() { return Simulator::Now(); }

Time Simulator::GetMaximumSimulationTime() {
  NS_LOG_FUNCTION_NOARGS();
  return GetImpl()->GetMaximumSimulationTime();
}

uint32_t Simulator::GetContext() { return GetImpl()->GetContext(); }

uint64_t Simulator::GetEventCount() { return GetImpl()->GetEventCount(); }

uint32_t Simulator::GetSystemId() {
  NS_LOG_FUNCTION_NOARGS();

  if (*PeekImpl() != nullptr) {
    return GetImpl()->GetSystemId();
  } else {
    return 0;
  }
}

void Simulator::SetImplementation(Ptr<SimulatorImpl> impl) {
  NS_LOG_FUNCTION(impl);
  if (*PeekImpl() != nullptr) {
    NS_FATAL_ERROR("It is not possible to set the implementation after calling "
                   "any Simulator:: function. "
                   "Call Simulator::SetImplementation earlier or after "
                   "Simulator::Destroy.");
  }
  *PeekImpl() = GetPointer(impl);
  ObjectFactory factory;
  StringValue s;
  g_schedTypeImpl.GetValue(s);
  factory.SetTypeId(s.Get());
  impl->SetScheduler(factory);
  LogSetTimePrinter(&DefaultTimePrinter);
  LogSetNodePrinter(&DefaultNodePrinter);
}

Ptr<SimulatorImpl> Simulator::GetImplementation() {
  NS_LOG_FUNCTION_NOARGS();
  return GetImpl();
}

} // namespace ns3
