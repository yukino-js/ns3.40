

#include "distributed-simulator-impl.h"

#include "granted-time-window-mpi-interface.h"
#include "mpi-interface.h"

#include "ns3/assert.h"
#include "ns3/channel.h"
#include "ns3/event-impl.h"
#include "ns3/log.h"
#include "ns3/node-container.h"
#include "ns3/pointer.h"
#include "ns3/ptr.h"
#include "ns3/scheduler.h"
#include "ns3/simulator.h"

#include <cmath>
#include <mpi.h>

namespace ns3 {

NS_LOG_COMPONENT_DEFINE("DistributedSimulatorImpl");

NS_OBJECT_ENSURE_REGISTERED(DistributedSimulatorImpl);

LbtsMessage::~LbtsMessage() {}

Time LbtsMessage::GetSmallestTime() { return m_smallestTime; }

uint32_t LbtsMessage::GetTxCount() const { return m_txCount; }

uint32_t LbtsMessage::GetRxCount() const { return m_rxCount; }

uint32_t LbtsMessage::GetMyId() const { return m_myId; }

bool LbtsMessage::IsFinished() const { return m_isFinished; }

Time DistributedSimulatorImpl::m_lookAhead = Time::Max();

TypeId DistributedSimulatorImpl::GetTypeId() {
  static TypeId tid = TypeId("ns3::DistributedSimulatorImpl")
                          .SetParent<SimulatorImpl>()
                          .SetGroupName("Mpi")
                          .AddConstructor<DistributedSimulatorImpl>();
  return tid;
}

DistributedSimulatorImpl::DistributedSimulatorImpl() {
  NS_LOG_FUNCTION(this);

  m_myId = MpiInterface::GetSystemId();
  m_systemCount = MpiInterface::GetSize();

  m_pLBTS = new LbtsMessage[m_systemCount];
  m_grantedTime = Seconds(0);

  m_stop = false;
  m_globalFinished = false;
  m_uid = EventId::UID::VALID;
  m_currentUid = EventId::UID::INVALID;
  m_currentTs = 0;
  m_currentContext = Simulator::NO_CONTEXT;
  m_unscheduledEvents = 0;
  m_eventCount = 0;
  m_events = nullptr;
}

DistributedSimulatorImpl::~DistributedSimulatorImpl() { NS_LOG_FUNCTION(this); }

void DistributedSimulatorImpl::DoDispose() {
  NS_LOG_FUNCTION(this);

  while (!m_events->IsEmpty()) {
    Scheduler::Event next = m_events->RemoveNext();
    next.impl->Unref();
  }
  m_events = nullptr;
  delete[] m_pLBTS;
  SimulatorImpl::DoDispose();
}

void DistributedSimulatorImpl::Destroy() {
  NS_LOG_FUNCTION(this);

  while (!m_destroyEvents.empty()) {
    Ptr<EventImpl> ev = m_destroyEvents.front().PeekEventImpl();
    m_destroyEvents.pop_front();
    NS_LOG_LOGIC("handle destroy " << ev);
    if (!ev->IsCancelled()) {
      ev->Invoke();
    }
  }

  MpiInterface::Destroy();
}

void DistributedSimulatorImpl::CalculateLookAhead() {
  NS_LOG_FUNCTION(this);

  if (MpiInterface::GetSize() <= 1) {
    m_lookAhead = Seconds(0);
  } else {
    NodeContainer c = NodeContainer::GetGlobal();
    for (auto iter = c.Begin(); iter != c.End(); ++iter) {
      if ((*iter)->GetSystemId() != MpiInterface::GetSystemId()) {
        continue;
      }

      for (uint32_t i = 0; i < (*iter)->GetNDevices(); ++i) {
        Ptr<NetDevice> localNetDevice = (*iter)->GetDevice(i);
        if (!localNetDevice->IsPointToPoint()) {
          continue;
        }
        Ptr<Channel> channel = localNetDevice->GetChannel();
        if (!channel) {
          continue;
        }

        Ptr<Node> remoteNode;
        if (channel->GetDevice(0) == localNetDevice) {
          remoteNode = (channel->GetDevice(1))->GetNode();
        } else {
          remoteNode = (channel->GetDevice(0))->GetNode();
        }

        if (remoteNode->GetSystemId() == MpiInterface::GetSystemId()) {
          continue;
        }

        TimeValue delay;
        channel->GetAttribute("Delay", delay);

        if (delay.Get() < m_lookAhead) {
          m_lookAhead = delay.Get();
        }
      }
    }
  }

  m_grantedTime = m_lookAhead;

  long sendbuf;
  long recvbuf;

  if (m_lookAhead == GetMaximumSimulationTime()) {
    sendbuf = 0;
  } else {
    sendbuf = m_lookAhead.GetInteger();
  }

  MPI_Allreduce(&sendbuf, &recvbuf, 1, MPI_LONG, MPI_MAX,
                MpiInterface::GetCommunicator());

  if (m_lookAhead == GetMaximumSimulationTime() && recvbuf != 0) {
    m_lookAhead = Time(recvbuf);
    m_grantedTime = m_lookAhead;
  }
}

void DistributedSimulatorImpl::BoundLookAhead(const Time lookAhead) {
  if (lookAhead > Time(0)) {
    NS_LOG_FUNCTION(this << lookAhead);
    m_lookAhead = Min(m_lookAhead, lookAhead);
  } else {
    NS_LOG_WARN("attempted to set lookahead to a negative time: " << lookAhead);
  }
}

void DistributedSimulatorImpl::SetScheduler(ObjectFactory schedulerFactory) {
  NS_LOG_FUNCTION(this << schedulerFactory);

  Ptr<Scheduler> scheduler = schedulerFactory.Create<Scheduler>();

  if (m_events) {
    while (!m_events->IsEmpty()) {
      Scheduler::Event next = m_events->RemoveNext();
      scheduler->Insert(next);
    }
  }
  m_events = scheduler;
}

void DistributedSimulatorImpl::ProcessOneEvent() {
  NS_LOG_FUNCTION(this);

  Scheduler::Event next = m_events->RemoveNext();

  PreEventHook(
      EventId(next.impl, next.key.m_ts, next.key.m_context, next.key.m_uid));

  NS_ASSERT(next.key.m_ts >= m_currentTs);
  m_unscheduledEvents--;
  m_eventCount++;

  NS_LOG_LOGIC("handle " << next.key.m_ts);
  m_currentTs = next.key.m_ts;
  m_currentContext = next.key.m_context;
  m_currentUid = next.key.m_uid;
  next.impl->Invoke();
  next.impl->Unref();
}

bool DistributedSimulatorImpl::IsFinished() const { return m_globalFinished; }

bool DistributedSimulatorImpl::IsLocalFinished() const {
  return m_events->IsEmpty() || m_stop;
}

uint64_t DistributedSimulatorImpl::NextTs() const {
  if (IsLocalFinished()) {
    return GetMaximumSimulationTime().GetTimeStep();
  } else {
    Scheduler::Event ev = m_events->PeekNext();
    return ev.key.m_ts;
  }
}

Time DistributedSimulatorImpl::Next() const { return TimeStep(NextTs()); }

void DistributedSimulatorImpl::Run() {
  NS_LOG_FUNCTION(this);

  CalculateLookAhead();
  m_stop = false;
  m_globalFinished = false;
  while (!m_globalFinished) {
    Time nextTime = Next();

    if (nextTime > m_grantedTime || IsLocalFinished()) {
      GrantedTimeWindowMpiInterface::ReceiveMessages();
      nextTime = Next();
      GrantedTimeWindowMpiInterface::TestSendComplete();
      LbtsMessage lMsg(GrantedTimeWindowMpiInterface::GetRxCount(),
                       GrantedTimeWindowMpiInterface::GetTxCount(), m_myId,
                       IsLocalFinished(), nextTime);
      m_pLBTS[m_myId] = lMsg;
      MPI_Allgather(&lMsg, sizeof(LbtsMessage), MPI_BYTE, m_pLBTS,
                    sizeof(LbtsMessage), MPI_BYTE,
                    MpiInterface::GetCommunicator());
      Time smallestTime = m_pLBTS[0].GetSmallestTime();
      uint32_t totRx = m_pLBTS[0].GetRxCount();
      uint32_t totTx = m_pLBTS[0].GetTxCount();
      m_globalFinished = m_pLBTS[0].IsFinished();

      for (uint32_t i = 1; i < m_systemCount; ++i) {
        if (m_pLBTS[i].GetSmallestTime() < smallestTime) {
          smallestTime = m_pLBTS[i].GetSmallestTime();
        }
        totRx += m_pLBTS[i].GetRxCount();
        totTx += m_pLBTS[i].GetTxCount();
        m_globalFinished &= m_pLBTS[i].IsFinished();
      }

      m_globalFinished &= totRx == totTx;

      if (totRx == totTx) {
        if (m_lookAhead == GetMaximumSimulationTime()) {
          m_grantedTime = GetMaximumSimulationTime();
        } else {
          m_grantedTime = smallestTime + m_lookAhead;
        }
      }
    }

    if ((nextTime <= m_grantedTime) && (!IsLocalFinished())) {
      ProcessOneEvent();
    }
  }

  NS_ASSERT(!m_events->IsEmpty() || m_unscheduledEvents == 0);
}

uint32_t DistributedSimulatorImpl::GetSystemId() const { return m_myId; }

void DistributedSimulatorImpl::Stop() {
  NS_LOG_FUNCTION(this);

  m_stop = true;
}

void DistributedSimulatorImpl::Stop(const Time &delay) {
  NS_LOG_FUNCTION(this << delay.GetTimeStep());

  Simulator::Schedule(delay, &Simulator::Stop);
}

EventId DistributedSimulatorImpl::Schedule(const Time &delay,
                                           EventImpl *event) {
  NS_LOG_FUNCTION(this << delay.GetTimeStep() << event);

  Time tAbsolute = delay + TimeStep(m_currentTs);

  NS_ASSERT(tAbsolute.IsPositive());
  NS_ASSERT(tAbsolute >= TimeStep(m_currentTs));
  Scheduler::Event ev;
  ev.impl = event;
  ev.key.m_ts = static_cast<uint64_t>(tAbsolute.GetTimeStep());
  ev.key.m_context = GetContext();
  ev.key.m_uid = m_uid;
  m_uid++;
  m_unscheduledEvents++;
  m_events->Insert(ev);
  return EventId(event, ev.key.m_ts, ev.key.m_context, ev.key.m_uid);
}

void DistributedSimulatorImpl::ScheduleWithContext(uint32_t context,
                                                   const Time &delay,
                                                   EventImpl *event) {
  NS_LOG_FUNCTION(this << context << delay.GetTimeStep() << m_currentTs
                       << event);

  Scheduler::Event ev;
  ev.impl = event;
  ev.key.m_ts = m_currentTs + delay.GetTimeStep();
  ev.key.m_context = context;
  ev.key.m_uid = m_uid;
  m_uid++;
  m_unscheduledEvents++;
  m_events->Insert(ev);
}

EventId DistributedSimulatorImpl::ScheduleNow(EventImpl *event) {
  NS_LOG_FUNCTION(this << event);
  return Schedule(Time(0), event);
}

EventId DistributedSimulatorImpl::ScheduleDestroy(EventImpl *event) {
  NS_LOG_FUNCTION(this << event);

  EventId id(Ptr<EventImpl>(event, false), m_currentTs, 0xffffffff, 2);
  m_destroyEvents.push_back(id);
  m_uid++;
  return id;
}

Time DistributedSimulatorImpl::Now() const { return TimeStep(m_currentTs); }

Time DistributedSimulatorImpl::GetDelayLeft(const EventId &id) const {
  if (IsExpired(id)) {
    return TimeStep(0);
  } else {
    return TimeStep(id.GetTs() - m_currentTs);
  }
}

void DistributedSimulatorImpl::Remove(const EventId &id) {
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
  Scheduler::Event event;
  event.impl = id.PeekEventImpl();
  event.key.m_ts = id.GetTs();
  event.key.m_context = id.GetContext();
  event.key.m_uid = id.GetUid();
  m_events->Remove(event);
  event.impl->Cancel();
  event.impl->Unref();

  m_unscheduledEvents--;
}

void DistributedSimulatorImpl::Cancel(const EventId &id) {
  if (!IsExpired(id)) {
    id.PeekEventImpl()->Cancel();
  }
}

bool DistributedSimulatorImpl::IsExpired(const EventId &id) const {
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

Time DistributedSimulatorImpl::GetMaximumSimulationTime() const {
  return TimeStep(0x7fffffffffffffffLL);
}

uint32_t DistributedSimulatorImpl::GetContext() const {
  return m_currentContext;
}

uint64_t DistributedSimulatorImpl::GetEventCount() const {
  return m_eventCount;
}

} // namespace ns3
