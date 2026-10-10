

#ifndef NULLMESSAGE_SIMULATOR_IMPL_H
#define NULLMESSAGE_SIMULATOR_IMPL_H

#include <ns3/event-impl.h>
#include <ns3/ptr.h>
#include <ns3/scheduler.h>
#include <ns3/simulator-impl.h>

#include <fstream>
#include <iostream>
#include <list>

namespace ns3 {

class NullMessageEvent;
class NullMessageMpiInterface;
class RemoteChannelBundle;

class NullMessageSimulatorImpl : public SimulatorImpl {
public:
  static TypeId GetTypeId();

  NullMessageSimulatorImpl();

  ~NullMessageSimulatorImpl() override;

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

  static NullMessageSimulatorImpl *GetInstance();

private:
  friend class NullMessageEvent;
  friend class NullMessageMpiInterface;
  friend class RemoteChannelBundleManager;

  void HandleArrivingMessagesNonBlocking();

  void HandleArrivingMessagesBlocking();

  void DoDispose() override;

  void CalculateLookAhead();

  void ProcessOneEvent();

  Time Next() const;

  void CalculateSafeTime();

  Time GetSafeTime();

  void ScheduleNullMessageEvent(Ptr<RemoteChannelBundle> bundle);

  void RescheduleNullMessageEvent(Ptr<RemoteChannelBundle> bundle);

  void RescheduleNullMessageEvent(uint32_t nodeSysId);

  Time CalculateGuaranteeTime(uint32_t systemId);

  void NullMessageEventHandler(RemoteChannelBundle *bundle);

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

  uint32_t m_myId;
  uint32_t m_systemCount;

  Time m_safeTime;

  double m_schedulerTune;

  static NullMessageSimulatorImpl *g_instance;
};

} // namespace ns3

#endif
