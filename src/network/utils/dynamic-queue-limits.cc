
#include "dynamic-queue-limits.h"

#include "ns3/log.h"
#include "ns3/simulator.h"
#include "ns3/string.h"
#include "ns3/uinteger.h"

static const uint32_t UINTMAX = std::numeric_limits<uint32_t>::max();
static const uint32_t DQL_MAX_OBJECT = UINTMAX / 16;
static const uint32_t DQL_MAX_LIMIT = (UINTMAX / 2) - DQL_MAX_OBJECT;

namespace ns3 {

NS_LOG_COMPONENT_DEFINE("DynamicQueueLimits");

NS_OBJECT_ENSURE_REGISTERED(DynamicQueueLimits);

TypeId DynamicQueueLimits::GetTypeId() {
  static TypeId tid =
      TypeId("ns3::DynamicQueueLimits")
          .SetParent<Object>()
          .SetParent<QueueLimits>()
          .SetGroupName("Network")
          .AddConstructor<DynamicQueueLimits>()
          .AddAttribute("HoldTime", "The DQL algorithm hold time",
                        StringValue("1s"),
                        MakeTimeAccessor(&DynamicQueueLimits::m_slackHoldTime),
                        MakeTimeChecker())
          .AddAttribute("MaxLimit", "Maximum limit",
                        UintegerValue(DQL_MAX_LIMIT),
                        MakeUintegerAccessor(&DynamicQueueLimits::m_maxLimit),
                        MakeUintegerChecker<uint32_t>(0, DQL_MAX_LIMIT))
          .AddAttribute("MinLimit", "Minimum limit", UintegerValue(0),
                        MakeUintegerAccessor(&DynamicQueueLimits::m_minLimit),
                        MakeUintegerChecker<uint32_t>())
          .AddTraceSource("Limit", "Limit value calculated by DQL",
                          MakeTraceSourceAccessor(&DynamicQueueLimits::m_limit),
                          "ns3::TracedValueCallback::Uint32");
  return tid;
}

DynamicQueueLimits::DynamicQueueLimits() {
  NS_LOG_FUNCTION(this);
  Reset();
}

DynamicQueueLimits::~DynamicQueueLimits() { NS_LOG_FUNCTION(this); }

void DynamicQueueLimits::Reset() {
  NS_LOG_FUNCTION(this);
  m_limit = 0;
  m_numQueued = 0;
  m_numCompleted = 0;
  m_lastObjCnt = 0;
  m_prevNumQueued = 0;
  m_prevLastObjCnt = 0;
  m_prevOvlimit = 0;
  m_lowestSlack = UINTMAX;
  m_slackStartTime = Simulator::Now();
}

void DynamicQueueLimits::Completed(uint32_t count) {
  NS_LOG_FUNCTION(this << count);
  uint32_t inprogress;
  uint32_t prevInprogress;
  uint32_t limit;
  uint32_t ovlimit;
  uint32_t completed;
  uint32_t numQueued;
  bool allPrevCompleted;

  numQueued = m_numQueued;

  NS_ASSERT(count <= numQueued - m_numCompleted);

  completed = m_numCompleted + count;
  limit = m_limit;
  ovlimit = Posdiff(numQueued - m_numCompleted, limit);
  inprogress = numQueued - completed;
  prevInprogress = m_prevNumQueued - m_numCompleted;
  allPrevCompleted = ((int32_t)(completed - m_prevNumQueued)) >= 0;

  if ((ovlimit && !inprogress) || (m_prevOvlimit && allPrevCompleted)) {
    NS_LOG_DEBUG("Queue starved, increase limit");
    limit += Posdiff(completed, m_prevNumQueued) + m_prevOvlimit;
    m_slackStartTime = Simulator::Now();
    m_lowestSlack = UINTMAX;
  } else if (inprogress && prevInprogress && !allPrevCompleted) {
    NS_LOG_DEBUG("Queue not starved, check decrease limit");
    uint32_t slack;
    uint32_t slackLastObjs;

    slack = Posdiff(limit + m_prevOvlimit, 2 * (completed - m_numCompleted));
    slackLastObjs =
        m_prevOvlimit ? Posdiff(m_prevLastObjCnt, m_prevOvlimit) : 0;

    slack = std::max(slack, slackLastObjs);

    if (slack < m_lowestSlack) {
      m_lowestSlack = slack;
    }

    if (Simulator::Now() > (m_slackStartTime + m_slackHoldTime)) {
      limit = Posdiff(limit, m_lowestSlack);
      m_slackStartTime = Simulator::Now();
      m_lowestSlack = UINTMAX;
    }
  }

  limit = std::min((uint32_t)std::max(limit, m_minLimit), m_maxLimit);

  if (limit != m_limit) {
    NS_LOG_DEBUG("Update limit");
    m_limit = limit;
    ovlimit = 0;
  }

  m_adjLimit = limit + completed;
  m_prevOvlimit = ovlimit;
  m_prevLastObjCnt = m_lastObjCnt;
  m_numCompleted = completed;
  m_prevNumQueued = numQueued;
}

int32_t DynamicQueueLimits::Available() const {
  NS_LOG_FUNCTION(this);
  return (m_adjLimit - m_numQueued);
}

void DynamicQueueLimits::Queued(uint32_t count) {
  NS_LOG_FUNCTION(this << count);
  NS_ASSERT(count <= DQL_MAX_OBJECT);

  m_lastObjCnt = count;
  m_numQueued += count;
}

int32_t DynamicQueueLimits::Posdiff(int32_t a, int32_t b) {
  NS_LOG_FUNCTION(this << a << b);
  return std::max((a - b), 0);
}

} // namespace ns3
