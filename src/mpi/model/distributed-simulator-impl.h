

#ifndef NS3_DISTRIBUTED_SIMULATOR_IMPL_H
#define NS3_DISTRIBUTED_SIMULATOR_IMPL_H

#include "ns3/event-impl.h"
#include "ns3/ptr.h"
#include "ns3/scheduler.h"
#include "ns3/simulator-impl.h"

#include <list>

namespace ns3 {

class LbtsMessage {
public:
  LbtsMessage() : m_txCount(0), m_rxCount(0), m_myId(0), m_isFinished(false) {}

  LbtsMessage(uint32_t rxc, uint32_t txc, uint32_t id, bool isFinished,
              const Time &t)
      : m_txCount(txc), m_rxCount(rxc), m_myId(id), m_smallestTime(t),
        m_isFinished(isFinished) {}

  ~LbtsMessage();

  Time GetSmallestTime();
  uint32_t GetTxCount() const;
  uint32_t GetRxCount() const;
  uint32_t GetMyId() const;
  bool IsFinished() const;

private:
  uint32_t m_txCount;
  uint32_t m_rxCount;
  uint32_t m_myId;
  Time m_smallestTime;
  bool m_isFinished;
};

class DistributedSimulatorImpl : public SimulatorImpl {
public:
  static TypeId GetTypeId();

  DistributedSimulatorImpl();
  ~DistributedSimulatorImpl() override;

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

  virtual void BoundLookAhead(const Time lookAhead);

private:
  void DoDispose() override;

  void CalculateLookAhead();
  bool IsLocalFinished() const;

  void ProcessOneEvent();
  uint64_t NextTs() const;
  Time Next() const;

  typedef std::list<EventId> DestroyEvents;

  DestroyEvents m_destroyEvents;
  bool m_stop;
  bool m_globalFinished;
  Ptr<Scheduler> m_events;

  uint32_t m_uid;
  uint32_t m_currentUid;
  uint64_t m_currentTs;
  uint32_t m_currentContext;
  uint64_t m_eventCount;
  int m_unscheduledEvents;

  LbtsMessage *m_pLBTS;
  uint32_t m_myId;
  uint32_t m_systemCount;
  Time m_grantedTime;
  static Time m_lookAhead;
};

} // namespace ns3

#endif
