

#include "pie-queue-disc.h"

#include "ns3/abort.h"
#include "ns3/double.h"
#include "ns3/drop-tail-queue.h"
#include "ns3/enum.h"
#include "ns3/log.h"
#include "ns3/simulator.h"
#include "ns3/uinteger.h"

namespace ns3 {

NS_LOG_COMPONENT_DEFINE("PieQueueDisc");

NS_OBJECT_ENSURE_REGISTERED(PieQueueDisc);

TypeId PieQueueDisc::GetTypeId() {
  static TypeId tid =
      TypeId("ns3::PieQueueDisc")
          .SetParent<QueueDisc>()
          .SetGroupName("TrafficControl")
          .AddConstructor<PieQueueDisc>()
          .AddAttribute("MeanPktSize", "Average of packet size",
                        UintegerValue(1000),
                        MakeUintegerAccessor(&PieQueueDisc::m_meanPktSize),
                        MakeUintegerChecker<uint32_t>())
          .AddAttribute("A", "Value of alpha", DoubleValue(0.125),
                        MakeDoubleAccessor(&PieQueueDisc::m_a),
                        MakeDoubleChecker<double>())
          .AddAttribute("B", "Value of beta", DoubleValue(1.25),
                        MakeDoubleAccessor(&PieQueueDisc::m_b),
                        MakeDoubleChecker<double>())
          .AddAttribute("Tupdate", "Time period to calculate drop probability",
                        TimeValue(MilliSeconds(15)),
                        MakeTimeAccessor(&PieQueueDisc::m_tUpdate),
                        MakeTimeChecker())
          .AddAttribute("Supdate", "Start time of the update timer",
                        TimeValue(Seconds(0)),
                        MakeTimeAccessor(&PieQueueDisc::m_sUpdate),
                        MakeTimeChecker())
          .AddAttribute(
              "MaxSize",
              "The maximum number of packets accepted by this queue disc",
              QueueSizeValue(QueueSize("25p")),
              MakeQueueSizeAccessor(&QueueDisc::SetMaxSize,
                                    &QueueDisc::GetMaxSize),
              MakeQueueSizeChecker())
          .AddAttribute(
              "DequeueThreshold",
              "Minimum queue size in bytes before dequeue rate is measured",
              UintegerValue(16384),
              MakeUintegerAccessor(&PieQueueDisc::m_dqThreshold),
              MakeUintegerChecker<uint32_t>())
          .AddAttribute("QueueDelayReference", "Desired queue delay",
                        TimeValue(MilliSeconds(15)),
                        MakeTimeAccessor(&PieQueueDisc::m_qDelayRef),
                        MakeTimeChecker())
          .AddAttribute("MaxBurstAllowance",
                        "Current max burst allowance before random drop",
                        TimeValue(MilliSeconds(150)),
                        MakeTimeAccessor(&PieQueueDisc::m_maxBurst),
                        MakeTimeChecker())
          .AddAttribute(
              "UseDequeueRateEstimator",
              "Enable/Disable usage of Dequeue Rate Estimator",
              BooleanValue(false),
              MakeBooleanAccessor(&PieQueueDisc::m_useDqRateEstimator),
              MakeBooleanChecker())
          .AddAttribute(
              "UseCapDropAdjustment",
              "Enable/Disable Cap Drop Adjustment feature mentioned in RFC "
              "8033",
              BooleanValue(true),
              MakeBooleanAccessor(&PieQueueDisc::m_isCapDropAdjustment),
              MakeBooleanChecker())
          .AddAttribute(
              "UseEcn",
              "True to use ECN (packets are marked instead of being dropped)",
              BooleanValue(false), MakeBooleanAccessor(&PieQueueDisc::m_useEcn),
              MakeBooleanChecker())
          .AddAttribute("MarkEcnThreshold",
                        "ECN marking threshold (RFC 8033 suggests 0.1 (i.e., "
                        "10%) default)",
                        DoubleValue(0.1),
                        MakeDoubleAccessor(&PieQueueDisc::m_markEcnTh),
                        MakeDoubleChecker<double>(0, 1))
          .AddAttribute(
              "UseDerandomization",
              "Enable/Disable Derandomization feature mentioned in RFC 8033",
              BooleanValue(false),
              MakeBooleanAccessor(&PieQueueDisc::m_useDerandomization),
              MakeBooleanChecker())
          .AddAttribute("ActiveThreshold",
                        "Threshold for activating PIE (disabled by default)",
                        TimeValue(Time::Max()),
                        MakeTimeAccessor(&PieQueueDisc::m_activeThreshold),
                        MakeTimeChecker())
          .AddAttribute(
              "CeThreshold", "The FqPie CE threshold for marking packets",
              TimeValue(Time::Max()),
              MakeTimeAccessor(&PieQueueDisc::m_ceThreshold), MakeTimeChecker())
          .AddAttribute(
              "UseL4s",
              "True to use L4S (only ECT1 packets are marked at CE threshold)",
              BooleanValue(false), MakeBooleanAccessor(&PieQueueDisc::m_useL4s),
              MakeBooleanChecker());

  return tid;
}

PieQueueDisc::PieQueueDisc()
    : QueueDisc(QueueDiscSizePolicy::SINGLE_INTERNAL_QUEUE) {
  NS_LOG_FUNCTION(this);
  m_uv = CreateObject<UniformRandomVariable>();
  m_rtrsEvent = Simulator::Schedule(m_sUpdate, &PieQueueDisc::CalculateP, this);
}

PieQueueDisc::~PieQueueDisc() { NS_LOG_FUNCTION(this); }

void PieQueueDisc::DoDispose() {
  NS_LOG_FUNCTION(this);
  m_uv = nullptr;
  m_rtrsEvent.Cancel();
  QueueDisc::DoDispose();
}

Time PieQueueDisc::GetQueueDelay() { return m_qDelay; }

int64_t PieQueueDisc::AssignStreams(int64_t stream) {
  NS_LOG_FUNCTION(this << stream);
  m_uv->SetStream(stream);
  return 1;
}

bool PieQueueDisc::DoEnqueue(Ptr<QueueDiscItem> item) {
  NS_LOG_FUNCTION(this << item);

  QueueSize nQueued = GetCurrentSize();
  bool isEct1 = false;
  if (item && m_useL4s) {
    uint8_t tosByte = 0;
    if (item->GetUint8Value(QueueItem::IP_DSFIELD, tosByte) &&
        (((tosByte & 0x3) == 1) || (tosByte & 0x3) == 3)) {
      if ((tosByte & 0x3) == 1) {
        NS_LOG_DEBUG("Enqueueing ECT1 packet "
                     << static_cast<uint16_t>(tosByte & 0x3));
      } else {
        NS_LOG_DEBUG("Enqueueing CE packet "
                     << static_cast<uint16_t>(tosByte & 0x3));
      }
      isEct1 = true;
    }
  }

  if (nQueued + item > GetMaxSize()) {
    DropBeforeEnqueue(item, FORCED_DROP);
    m_accuProb = 0;
    return false;
  } else if ((m_activeThreshold == Time::Max() || m_active) && !isEct1 &&
             DropEarly(item, nQueued.GetValue())) {
    if (!m_useEcn || m_dropProb >= m_markEcnTh || !Mark(item, UNFORCED_MARK)) {
      DropBeforeEnqueue(item, UNFORCED_DROP);
      m_accuProb = 0;
      return false;
    }
  }

  bool retval = GetInternalQueue(0)->Enqueue(item);

  if (m_activeThreshold != Time::Max() && !m_active &&
      m_qDelay >= m_activeThreshold) {
    m_active = true;
    m_qDelayOld = Seconds(0);
    m_dropProb = 0;
    m_inMeasurement = true;
    m_dqCount = 0;
    m_avgDqRate = 0;
    m_burstAllowance = m_maxBurst;
    m_accuProb = 0;
    m_dqStart = Now();
  }

  if (m_activeThreshold != Time::Max() && m_dropProb == 0 &&
      m_qDelayOld.GetMilliSeconds() == 0 && m_qDelay.GetMilliSeconds() == 0) {
    m_active = false;
    m_inMeasurement = false;
  }

  NS_LOG_LOGIC("\t bytesInQueue  " << GetInternalQueue(0)->GetNBytes());
  NS_LOG_LOGIC("\t packetsInQueue  " << GetInternalQueue(0)->GetNPackets());

  return retval;
}

void PieQueueDisc::InitializeParams() {
  m_inMeasurement = false;
  m_dqCount = DQCOUNT_INVALID;
  m_dropProb = 0;
  m_avgDqRate = 0.0;
  m_dqStart = Seconds(0);
  m_burstState = NO_BURST;
  m_qDelayOld = Seconds(0);
  m_accuProb = 0.0;
  m_active = false;
}

bool PieQueueDisc::DropEarly(Ptr<QueueDiscItem> item, uint32_t qSize) {
  NS_LOG_FUNCTION(this << item << qSize);
  if (m_burstAllowance.GetSeconds() > 0) {
    return false;
  }

  if (m_burstState == NO_BURST) {
    m_burstState = IN_BURST_PROTECTING;
    m_burstAllowance = m_maxBurst;
  }

  double p = m_dropProb;

  uint32_t packetSize = item->GetSize();

  if (GetMaxSize().GetUnit() == QueueSizeUnit::BYTES) {
    p = p * packetSize / m_meanPktSize;
  }

  if ((m_qDelayOld.GetSeconds() < (0.5 * m_qDelayRef.GetSeconds())) &&
      (m_dropProb < 0.2)) {
    return false;
  } else if (GetMaxSize().GetUnit() == QueueSizeUnit::BYTES &&
             qSize <= 2 * m_meanPktSize) {
    return false;
  } else if (GetMaxSize().GetUnit() == QueueSizeUnit::PACKETS && qSize <= 2) {
    return false;
  }

  if (m_useDerandomization) {
    if (m_dropProb == 0) {
      m_accuProb = 0;
    }
    m_accuProb += m_dropProb;
    if (m_accuProb < 0.85) {
      return false;
    } else if (m_accuProb >= 8.5) {
      return true;
    }
  }

  double u = m_uv->GetValue();
  return u <= p;
}

void PieQueueDisc::CalculateP() {
  NS_LOG_FUNCTION(this);
  Time qDelay;
  double p = 0.0;
  bool missingInitFlag = false;

  if (m_useDqRateEstimator) {
    if (m_avgDqRate > 0) {
      qDelay = Seconds(GetInternalQueue(0)->GetNBytes() / m_avgDqRate);
    } else {
      qDelay = Seconds(0);
      missingInitFlag = true;
    }
    m_qDelay = qDelay;
  } else {
    qDelay = m_qDelay;
  }
  NS_LOG_DEBUG("Queue delay while calculating probability: "
               << qDelay.GetMilliSeconds() << "ms");

  if (m_burstAllowance.GetSeconds() > 0) {
    m_dropProb = 0;
  } else {
    p = m_a * (qDelay.GetSeconds() - m_qDelayRef.GetSeconds()) +
        m_b * (qDelay.GetSeconds() - m_qDelayOld.GetSeconds());
    if (m_dropProb < 0.000001) {
      p /= 2048;
    } else if (m_dropProb < 0.00001) {
      p /= 512;
    } else if (m_dropProb < 0.0001) {
      p /= 128;
    } else if (m_dropProb < 0.001) {
      p /= 32;
    } else if (m_dropProb < 0.01) {
      p /= 8;
    } else if (m_dropProb < 0.1) {
      p /= 2;
    } else {
    }

    if (m_isCapDropAdjustment && (m_dropProb >= 0.1) && (p > 0.02)) {
      p = 0.02;
    }
  }

  p += m_dropProb;

  if (qDelay.GetSeconds() == 0 && m_qDelayOld.GetSeconds() == 0) {
    p *= 0.98;
  }

  if (p < 0) {
    m_dropProb = 0;
  } else if (p > 1) {
    m_dropProb = 1;
  } else {
    m_dropProb = p;
  }

  if (m_burstAllowance < m_tUpdate) {
    m_burstAllowance = Seconds(0);
  } else {
    m_burstAllowance -= m_tUpdate;
  }

  auto burstResetLimit =
      static_cast<uint32_t>(BURST_RESET_TIMEOUT / m_tUpdate.GetSeconds());
  if ((qDelay.GetSeconds() < 0.5 * m_qDelayRef.GetSeconds()) &&
      (m_qDelayOld.GetSeconds() < (0.5 * m_qDelayRef.GetSeconds())) &&
      (m_dropProb == 0) && !missingInitFlag) {
    m_dqCount = DQCOUNT_INVALID;
    m_avgDqRate = 0.0;
  }
  if ((qDelay.GetSeconds() < 0.5 * m_qDelayRef.GetSeconds()) &&
      (m_qDelayOld.GetSeconds() < (0.5 * m_qDelayRef.GetSeconds())) &&
      (m_dropProb == 0) && (m_burstAllowance.GetSeconds() == 0)) {
    if (m_burstState == IN_BURST_PROTECTING) {
      m_burstState = IN_BURST;
      m_burstReset = 0;
    } else if (m_burstState == IN_BURST) {
      m_burstReset++;
      if (m_burstReset > burstResetLimit) {
        m_burstReset = 0;
        m_burstState = NO_BURST;
      }
    }
  } else if (m_burstState == IN_BURST) {
    m_burstReset = 0;
  }

  m_qDelayOld = qDelay;
  m_rtrsEvent = Simulator::Schedule(m_tUpdate, &PieQueueDisc::CalculateP, this);
}

Ptr<QueueDiscItem> PieQueueDisc::DoDequeue() {
  NS_LOG_FUNCTION(this);

  if (GetInternalQueue(0)->IsEmpty()) {
    NS_LOG_LOGIC("Queue empty");
    return nullptr;
  }

  Ptr<QueueDiscItem> item = GetInternalQueue(0)->Dequeue();
  NS_ASSERT_MSG(item, "Dequeue null, but internal queue not empty");

  if (m_useL4s) {
    uint8_t tosByte = 0;
    if (item->GetUint8Value(QueueItem::IP_DSFIELD, tosByte) &&
        (((tosByte & 0x3) == 1) || (tosByte & 0x3) == 3)) {
      if ((tosByte & 0x3) == 1) {
        NS_LOG_DEBUG("ECT1 packet " << static_cast<uint16_t>(tosByte & 0x3));
      } else {
        NS_LOG_DEBUG("CE packet " << static_cast<uint16_t>(tosByte & 0x3));
      }
      if ((Now() - item->GetTimeStamp() > m_ceThreshold) &&
          Mark(item, CE_THRESHOLD_EXCEEDED_MARK)) {
        NS_LOG_LOGIC("Marking due to CeThreshold "
                     << m_ceThreshold.GetSeconds());
      }
      return item;
    }
  }

  if (m_useDqRateEstimator) {
    if ((GetInternalQueue(0)->GetNBytes() >= m_dqThreshold) &&
        (!m_inMeasurement)) {
      m_dqStart = Now();
      m_dqCount = 0;
      m_inMeasurement = true;
    }

    if (m_inMeasurement) {
      m_dqCount += item->GetSize();

      if (m_dqCount >= m_dqThreshold) {
        Time dqTime = Now() - m_dqStart;
        if (dqTime > Seconds(0)) {
          if (m_avgDqRate == 0) {
            m_avgDqRate = m_dqCount / dqTime.GetSeconds();
          } else {
            m_avgDqRate =
                (0.5 * m_avgDqRate) + (0.5 * (m_dqCount / dqTime.GetSeconds()));
          }
        }
        NS_LOG_DEBUG("Average Dequeue Rate after Dequeue: " << m_avgDqRate);

        if (GetInternalQueue(0)->GetNBytes() > m_dqThreshold) {
          m_dqStart = Now();
          m_dqCount = 0;
          m_inMeasurement = true;
        } else {
          m_dqCount = 0;
          m_inMeasurement = false;
        }
      }
    }
  } else {
    m_qDelay = Now() - item->GetTimeStamp();

    if (GetInternalQueue(0)->GetNBytes() == 0) {
      m_qDelay = Seconds(0);
    }
  }
  return item;
}

bool PieQueueDisc::CheckConfig() {
  NS_LOG_FUNCTION(this);
  if (GetNQueueDiscClasses() > 0) {
    NS_LOG_ERROR("PieQueueDisc cannot have classes");
    return false;
  }

  if (GetNPacketFilters() > 0) {
    NS_LOG_ERROR("PieQueueDisc cannot have packet filters");
    return false;
  }

  if (GetNInternalQueues() == 0) {
    AddInternalQueue(CreateObjectWithAttributes<DropTailQueue<QueueDiscItem>>(
        "MaxSize", QueueSizeValue(GetMaxSize())));
  }

  if (GetNInternalQueues() != 1) {
    NS_LOG_ERROR("PieQueueDisc needs 1 internal queue");
    return false;
  }

  return true;
}

} // namespace ns3
