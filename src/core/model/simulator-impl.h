
#ifndef SIMULATOR_IMPL_H
#define SIMULATOR_IMPL_H

#include "event-id.h"
#include "event-impl.h"
#include "nstime.h"
#include "object-factory.h"
#include "object.h"
#include "ptr.h"

namespace ns3 {

class Scheduler;

class SimulatorImpl : public Object {
public:
  static TypeId GetTypeId();

  virtual void Destroy() = 0;
  virtual bool IsFinished() const = 0;
  virtual void Stop() = 0;
  virtual void Stop(const Time &delay) = 0;
  virtual EventId Schedule(const Time &delay, EventImpl *event) = 0;
  virtual void ScheduleWithContext(uint32_t context, const Time &delay,
                                   EventImpl *event) = 0;
  virtual EventId ScheduleNow(EventImpl *event) = 0;
  virtual EventId ScheduleDestroy(EventImpl *event) = 0;
  virtual void Remove(const EventId &id) = 0;
  virtual void Cancel(const EventId &id) = 0;
  virtual bool IsExpired(const EventId &id) const = 0;
  virtual void Run() = 0;
  virtual Time Now() const = 0;
  virtual Time GetDelayLeft(const EventId &id) const = 0;
  virtual Time GetMaximumSimulationTime() const = 0;
  virtual void SetScheduler(ObjectFactory schedulerFactory) = 0;
  virtual uint32_t GetSystemId() const = 0;
  virtual uint32_t GetContext() const = 0;
  virtual uint64_t GetEventCount() const = 0;

  virtual void PreEventHook(const EventId &id) {};
};

} // namespace ns3

#endif
