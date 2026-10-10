

#include "hybrid-simulator-impl.h"

#include "granted-time-window-mpi-interface.h"
#include "mpi-interface.h"

#include "ns3/channel.h"
#include "ns3/mtp-interface.h"
#include "ns3/node-container.h"
#include "ns3/node-list.h"
#include "ns3/node.h"
#include "ns3/simulator.h"
#include "ns3/uinteger.h"

#include <algorithm>
#include <mpi.h>
#include <queue>
#include <thread>

namespace ns3 {

NS_LOG_COMPONENT_DEFINE("HybridSimulatorImpl");

NS_OBJECT_ENSURE_REGISTERED(HybridSimulatorImpl);

HybridSimulatorImpl::HybridSimulatorImpl() {
  NS_LOG_FUNCTION(this);

  MtpInterface::Enable(1, 0);
  m_myId = MpiInterface::GetSystemId();
  m_systemCount = MpiInterface::GetSize();

  m_pLBTS = new LbtsMessage[m_systemCount];
  m_smallestTime = Seconds(0);
  m_globalFinished = false;
}

HybridSimulatorImpl::~HybridSimulatorImpl() { NS_LOG_FUNCTION(this); }

TypeId HybridSimulatorImpl::GetTypeId(void) {
  static TypeId tid =
      TypeId("ns3::HybridSimulatorImpl")
          .SetParent<SimulatorImpl>()
          .SetGroupName("Mtp")
          .AddConstructor<HybridSimulatorImpl>()
          .AddAttribute(
              "MaxThreads", "The maximum threads used in simulation",
              UintegerValue(std::thread::hardware_concurrency()),
              MakeUintegerAccessor(&HybridSimulatorImpl::m_maxThreads),
              MakeUintegerChecker<uint32_t>(1))
          .AddAttribute("MinLookahead", "The minimum lookahead in a partition",
                        TimeValue(TimeStep(1)),
                        MakeTimeAccessor(&HybridSimulatorImpl::m_minLookahead),
                        MakeTimeChecker(TimeStep(0)));
  return tid;
}

void HybridSimulatorImpl::Destroy() {
  while (!m_destroyEvents.empty()) {
    Ptr<EventImpl> ev = m_destroyEvents.front().PeekEventImpl();
    m_destroyEvents.pop_front();
    NS_LOG_LOGIC("handle destroy " << ev);
    if (!ev->IsCancelled()) {
      ev->Invoke();
    }
  }

  MtpInterface::Disable();
  MpiInterface::Destroy();
}

bool HybridSimulatorImpl::IsFinished(void) const { return m_globalFinished; }

bool HybridSimulatorImpl::IsLocalFinished(void) const {
  return MtpInterface::isFinished();
}

void HybridSimulatorImpl::Stop(void) {
  NS_LOG_FUNCTION(this);
  for (uint32_t i = 0; i < MtpInterface::GetSize(); i++) {
    MtpInterface::GetSystem(i)->Stop();
  }
}

void HybridSimulatorImpl::Stop(const Time &delay) {
  NS_LOG_FUNCTION(this << delay.GetTimeStep());
  Simulator::Schedule(delay, &Simulator::Stop);
}

EventId HybridSimulatorImpl::Schedule(const Time &delay, EventImpl *event) {
  NS_LOG_FUNCTION(this << delay.GetTimeStep() << event);
  return MtpInterface::GetSystem()->Schedule(delay, event);
}

void HybridSimulatorImpl::ScheduleWithContext(uint32_t context,
                                              const Time &delay,
                                              EventImpl *event) {
  NS_LOG_FUNCTION(this << context << delay.GetTimeStep() << event);

  if (MtpInterface::GetSize() == 1) {
    LogicalProcess *local = MtpInterface::GetSystem();
    local->ScheduleWithContext(local, context, delay, event);
  } else {
    LogicalProcess *remote = MtpInterface::GetSystem(
        NodeList::GetNode(context)->GetSystemId() >> 16);
    MtpInterface::GetSystem()->ScheduleWithContext(remote, context, delay,
                                                   event);
  }
}

EventId HybridSimulatorImpl::ScheduleNow(EventImpl *event) {
  return Schedule(TimeStep(0), event);
}

EventId HybridSimulatorImpl::ScheduleDestroy(EventImpl *event) {
  EventId id(Ptr<EventImpl>(event, false),
             GetMaximumSimulationTime().GetTimeStep(), 0xffffffff,
             EventId::DESTROY);
  MtpInterface::CriticalSection cs;
  m_destroyEvents.push_back(id);
  return id;
}

void HybridSimulatorImpl::Remove(const EventId &id) {
  if (id.GetUid() == EventId::DESTROY) {
    for (std::list<EventId>::iterator i = m_destroyEvents.begin();
         i != m_destroyEvents.end(); i++) {
      if (*i == id) {
        m_destroyEvents.erase(i);
        break;
      }
    }
  } else {
    MtpInterface::GetSystem()->Remove(id);
  }
}

void HybridSimulatorImpl::Cancel(const EventId &id) {
  if (!IsExpired(id)) {
    id.PeekEventImpl()->Cancel();
  }
}

bool HybridSimulatorImpl::IsExpired(const EventId &id) const {
  if (id.GetUid() == EventId::DESTROY) {
    if (id.PeekEventImpl() == 0 || id.PeekEventImpl()->IsCancelled()) {
      return true;
    }
    for (std::list<EventId>::const_iterator i = m_destroyEvents.begin();
         i != m_destroyEvents.end(); i++) {
      if (*i == id) {
        return false;
      }
    }
    return true;
  } else {
    return MtpInterface::GetSystem()->IsExpired(id);
  }
}

void HybridSimulatorImpl::Run(void) {
  NS_LOG_FUNCTION(this);

  Partition();
  MtpInterface::RunBefore();

  m_globalFinished = false;
  while (!m_globalFinished) {
    GrantedTimeWindowMpiInterface::ReceiveMessages();
    GrantedTimeWindowMpiInterface::TestSendComplete();
    MtpInterface::CalculateSmallestTime();
    LbtsMessage lMsg(GrantedTimeWindowMpiInterface::GetRxCount(),
                     GrantedTimeWindowMpiInterface::GetTxCount(), m_myId,
                     IsLocalFinished(), MtpInterface::GetSmallestTime());
    m_pLBTS[m_myId] = lMsg;
    MPI_Allgather(&lMsg, sizeof(LbtsMessage), MPI_BYTE, m_pLBTS,
                  sizeof(LbtsMessage), MPI_BYTE,
                  MpiInterface::GetCommunicator());
    m_smallestTime = m_pLBTS[0].GetSmallestTime();

    uint32_t totRx = m_pLBTS[0].GetRxCount();
    uint32_t totTx = m_pLBTS[0].GetTxCount();
    m_globalFinished = m_pLBTS[0].IsFinished();

    for (uint32_t i = 1; i < m_systemCount; ++i) {
      if (m_pLBTS[i].GetSmallestTime() < m_smallestTime) {
        m_smallestTime = m_pLBTS[i].GetSmallestTime();
      }
      totRx += m_pLBTS[i].GetRxCount();
      totTx += m_pLBTS[i].GetTxCount();
      m_globalFinished &= m_pLBTS[i].IsFinished();
    }
    MtpInterface::SetSmallestTime(m_smallestTime);

    m_globalFinished &= totRx == totTx;

    if (totRx == totTx && !IsLocalFinished()) {
      MtpInterface::ProcessOneRound();
    }
  }

  MtpInterface::RunAfter();
}

Time HybridSimulatorImpl::Now(void) const {
  return MtpInterface::GetSystem()->Now();
}

Time HybridSimulatorImpl::GetDelayLeft(const EventId &id) const {
  if (IsExpired(id)) {
    return TimeStep(0);
  } else {
    return MtpInterface::GetSystem()->GetDelayLeft(id);
  }
}

Time HybridSimulatorImpl::GetMaximumSimulationTime(void) const {
  return Time::Max() / 2;
}

void HybridSimulatorImpl::SetScheduler(ObjectFactory schedulerFactory) {
  NS_LOG_FUNCTION(this << schedulerFactory);
  for (uint32_t i = 0; i < MtpInterface::GetSize(); i++) {
    MtpInterface::GetSystem(i)->SetScheduler(schedulerFactory);
  }
  m_schedulerTypeId = schedulerFactory.GetTypeId();
}

uint32_t HybridSimulatorImpl::GetSystemId() const { return m_myId; }

uint32_t HybridSimulatorImpl::GetContext(void) const {
  return MtpInterface::GetSystem()->GetContext();
}

uint64_t HybridSimulatorImpl::GetEventCount(void) const {
  uint64_t eventCount = 0;
  for (uint32_t i = 0; i < MtpInterface::GetSize(); i++) {
    eventCount += MtpInterface::GetSystem(i)->GetEventCount();
  }
  return eventCount;
}

void HybridSimulatorImpl::DoDispose(void) {
  delete[] m_pLBTS;
  SimulatorImpl::DoDispose();
}

void HybridSimulatorImpl::Partition() {
  NS_LOG_FUNCTION(this);
  uint32_t localSystemId = 0;
  NodeContainer nodes = NodeContainer::GetGlobal();
  bool *visited = new bool[nodes.GetN()]{false};
  std::queue<Ptr<Node>> q;

  if (m_minLookahead == TimeStep(0)) {
    std::vector<Time> delays;
    for (NodeContainer::Iterator it = nodes.Begin(); it != nodes.End(); it++) {
      Ptr<Node> node = *it;
      if (node->GetSystemId() == m_myId) {
        for (uint32_t i = 0; i < node->GetNDevices(); i++) {
          Ptr<NetDevice> localNetDevice = node->GetDevice(i);
          Ptr<Channel> channel = localNetDevice->GetChannel();
          if (!channel) {
            continue;
          }
          if (localNetDevice->IsPointToPoint()) {
            TimeValue delay;
            channel->GetAttribute("Delay", delay);
            delays.push_back(delay.Get());
          }
        }
      }
    }
    std::sort(delays.begin(), delays.end());
    if (delays.size() == 0) {
      m_minLookahead = TimeStep(0);
    } else if (delays.size() % 2 == 1) {
      m_minLookahead = delays[delays.size() / 2];
    } else {
      m_minLookahead =
          (delays[delays.size() / 2 - 1] + delays[delays.size() / 2]) / 2;
    }
    NS_LOG_INFO("Min lookahead is set to " << m_minLookahead);
  }

  for (NodeContainer::Iterator it = nodes.Begin(); it != nodes.End(); it++) {
    Ptr<Node> node = *it;
    if (!visited[node->GetId()] && node->GetSystemId() == m_myId) {
      q.push(node);
      localSystemId++;
      while (!q.empty()) {
        node = q.front();
        q.pop();
        visited[node->GetId()] = true;
        node->SetSystemId(localSystemId << 16 | m_myId);
        NS_LOG_INFO("node " << node->GetId() << " is set to local system "
                            << localSystemId);

        for (uint32_t i = 0; i < node->GetNDevices(); i++) {
          Ptr<NetDevice> localNetDevice = node->GetDevice(i);
          Ptr<Channel> channel = localNetDevice->GetChannel();
          if (!channel) {
            continue;
          }
          if (localNetDevice->IsPointToPoint()) {
            TimeValue delay;
            channel->GetAttribute("Delay", delay);
            if (delay.Get() >= m_minLookahead) {
              continue;
            }
          }
          for (uint32_t j = 0; j < channel->GetNDevices(); j++) {
            Ptr<Node> remote = channel->GetDevice(j)->GetNode();
            if (!visited[remote->GetId()] && node->GetSystemId() == m_myId) {
              q.push(remote);
            }
          }
        }
      }
    }
  }
  delete[] visited;

  const uint32_t systemCount = localSystemId;
  const uint32_t threadCount = std::min(m_maxThreads, systemCount);
  NS_LOG_INFO("Partition done! " << systemCount << " systems share "
                                 << threadCount << " threads");

  const Ptr<Scheduler> events = MtpInterface::GetSystem()->GetPendingEvents();
  MtpInterface::Disable();
  MtpInterface::Enable(threadCount, systemCount);

  ObjectFactory schedulerFactory;
  schedulerFactory.SetTypeId(m_schedulerTypeId);
  for (uint32_t i = 0; i <= systemCount; i++) {
    MtpInterface::GetSystem(i)->SetScheduler(schedulerFactory);
  }

  while (!events->IsEmpty()) {
    Scheduler::Event ev = events->RemoveNext();
    if (ev.key.m_ts == 0) {
      MtpInterface::GetSystem(
          ev.key.m_context == Simulator::NO_CONTEXT
              ? 0
              : NodeList::GetNode(ev.key.m_context)->GetSystemId() >> 16)
          ->InvokeNow(ev);
    } else if (ev.key.m_context == Simulator::NO_CONTEXT) {
      Schedule(TimeStep(ev.key.m_ts), ev.impl);
    } else {
      ScheduleWithContext(ev.key.m_context, TimeStep(ev.key.m_ts), ev.impl);
    }
  }
}

} // namespace ns3
