
#define NS_LOG_APPEND_CONTEXT                                                  \
  {                                                                            \
    std::clog << Simulator::Now().GetSeconds() << " ";                         \
  }

#include "tcp-cubic.h"

#include "ns3/log.h"

NS_LOG_COMPONENT_DEFINE("TcpCubic");

namespace ns3 {

NS_OBJECT_ENSURE_REGISTERED(TcpCubic);

TypeId TcpCubic::GetTypeId() {
  static TypeId tid =
      TypeId("ns3::TcpCubic")
          .SetParent<TcpSocketBase>()
          .AddConstructor<TcpCubic>()
          .SetGroupName("Internet")
          .AddAttribute("FastConvergence",
                        "Enable (true) or disable (false) fast convergence",
                        BooleanValue(true),
                        MakeBooleanAccessor(&TcpCubic::m_fastConvergence),
                        MakeBooleanChecker())
          .AddAttribute("Beta", "Beta for multiplicative decrease",
                        DoubleValue(0.7), MakeDoubleAccessor(&TcpCubic::m_beta),
                        MakeDoubleChecker<double>(0.0))
          .AddAttribute(
              "HyStart",
              "Enable (true) or disable (false) hybrid slow start algorithm",
              BooleanValue(true), MakeBooleanAccessor(&TcpCubic::m_hystart),
              MakeBooleanChecker())
          .AddAttribute("HyStartLowWindow",
                        "Lower bound cWnd for hybrid slow start (segments)",
                        UintegerValue(16),
                        MakeUintegerAccessor(&TcpCubic::m_hystartLowWindow),
                        MakeUintegerChecker<uint32_t>())
          .AddAttribute("HyStartDetect",
                        "Hybrid Slow Start detection mechanisms:"
                        "packet train, delay, both",
                        EnumValue(HybridSSDetectionMode::BOTH),
                        MakeEnumAccessor(&TcpCubic::m_hystartDetect),
                        MakeEnumChecker(HybridSSDetectionMode::PACKET_TRAIN,
                                        "PACKET_TRAIN",
                                        HybridSSDetectionMode::DELAY, "DELAY",
                                        HybridSSDetectionMode::BOTH, "BOTH"))
          .AddAttribute(
              "HyStartMinSamples",
              "Number of delay samples for detecting the increase of delay",
              UintegerValue(8),
              MakeUintegerAccessor(&TcpCubic::m_hystartMinSamples),
              MakeUintegerChecker<uint8_t>())
          .AddAttribute(
              "HyStartAckDelta", "Spacing between ack's indicating train",
              TimeValue(MilliSeconds(2)),
              MakeTimeAccessor(&TcpCubic::m_hystartAckDelta), MakeTimeChecker())
          .AddAttribute("HyStartDelayMin", "Minimum time for hystart algorithm",
                        TimeValue(MilliSeconds(4)),
                        MakeTimeAccessor(&TcpCubic::m_hystartDelayMin),
                        MakeTimeChecker())
          .AddAttribute("HyStartDelayMax", "Maximum time for hystart algorithm",
                        TimeValue(MilliSeconds(1000)),
                        MakeTimeAccessor(&TcpCubic::m_hystartDelayMax),
                        MakeTimeChecker())
          .AddAttribute(
              "CubicDelta",
              "Delta Time to wait after fast recovery before adjusting param",
              TimeValue(MilliSeconds(10)),
              MakeTimeAccessor(&TcpCubic::m_cubicDelta), MakeTimeChecker())
          .AddAttribute(
              "CntClamp",
              "Counter value when no losses are detected (counter is used"
              " when incrementing cWnd in congestion avoidance, to avoid"
              " floating point arithmetic). It is the modulo of the (avoided)"
              " division",
              UintegerValue(20), MakeUintegerAccessor(&TcpCubic::m_cntClamp),
              MakeUintegerChecker<uint8_t>())
          .AddAttribute("C", "Cubic Scaling factor", DoubleValue(0.4),
                        MakeDoubleAccessor(&TcpCubic::m_c),
                        MakeDoubleChecker<double>(0.0));
  return tid;
}

TcpCubic::TcpCubic()
    : TcpCongestionOps(), m_cWndCnt(0), m_lastMaxCwnd(0), m_bicOriginPoint(0),
      m_bicK(0.0), m_delayMin(Time::Min()), m_epochStart(Time::Min()),
      m_found(false), m_roundStart(Time::Min()), m_endSeq(0),
      m_lastAck(Time::Min()), m_cubicDelta(Time::Min()), m_currRtt(Time::Min()),
      m_sampleCnt(0) {
  NS_LOG_FUNCTION(this);
}

TcpCubic::TcpCubic(const TcpCubic &sock)
    : TcpCongestionOps(sock), m_fastConvergence(sock.m_fastConvergence),
      m_beta(sock.m_beta), m_hystart(sock.m_hystart),
      m_hystartDetect(sock.m_hystartDetect),
      m_hystartLowWindow(sock.m_hystartLowWindow),
      m_hystartAckDelta(sock.m_hystartAckDelta),
      m_hystartDelayMin(sock.m_hystartDelayMin),
      m_hystartDelayMax(sock.m_hystartDelayMax),
      m_hystartMinSamples(sock.m_hystartMinSamples),
      m_initialCwnd(sock.m_initialCwnd), m_cntClamp(sock.m_cntClamp),
      m_c(sock.m_c), m_cWndCnt(sock.m_cWndCnt),
      m_lastMaxCwnd(sock.m_lastMaxCwnd),
      m_bicOriginPoint(sock.m_bicOriginPoint), m_bicK(sock.m_bicK),
      m_delayMin(sock.m_delayMin), m_epochStart(sock.m_epochStart),
      m_found(sock.m_found), m_roundStart(sock.m_roundStart),
      m_endSeq(sock.m_endSeq), m_lastAck(sock.m_lastAck),
      m_cubicDelta(sock.m_cubicDelta), m_currRtt(sock.m_currRtt),
      m_sampleCnt(sock.m_sampleCnt) {
  NS_LOG_FUNCTION(this);
}

std::string TcpCubic::GetName() const { return "TcpCubic"; }

void TcpCubic::HystartReset(Ptr<const TcpSocketState> tcb) {
  NS_LOG_FUNCTION(this);

  m_roundStart = m_lastAck = Simulator::Now();
  m_endSeq = tcb->m_highTxMark;
  m_currRtt = Time::Min();
  m_sampleCnt = 0;
}

void TcpCubic::IncreaseWindow(Ptr<TcpSocketState> tcb, uint32_t segmentsAcked) {
  NS_LOG_FUNCTION(this << tcb << segmentsAcked);

  if (tcb->m_cWnd < tcb->m_ssThresh) {
    if (m_hystart && tcb->m_lastAckedSeq > m_endSeq) {
      HystartReset(tcb);
    }

    tcb->m_cWnd += segmentsAcked * tcb->m_segmentSize;
    segmentsAcked = 0;

    NS_LOG_INFO("In SlowStart, updated to cwnd " << tcb->m_cWnd << " ssthresh "
                                                 << tcb->m_ssThresh);
  }

  if (tcb->m_cWnd >= tcb->m_ssThresh && segmentsAcked > 0) {
    m_cWndCnt += segmentsAcked;
    uint32_t cnt = Update(tcb);

    if (m_cWndCnt >= cnt) {
      tcb->m_cWnd += tcb->m_segmentSize;
      m_cWndCnt -= cnt;
      NS_LOG_INFO("In CongAvoid, updated to cwnd " << tcb->m_cWnd);
    } else {
      NS_LOG_INFO("Not enough segments have been ACKed to increment cwnd."
                  "Until now "
                  << m_cWndCnt << " cnd " << cnt);
    }
  }
}

uint32_t TcpCubic::Update(Ptr<TcpSocketState> tcb) {
  NS_LOG_FUNCTION(this);
  Time t;
  uint32_t delta;
  uint32_t bicTarget;
  uint32_t cnt = 0;
  double offs;
  uint32_t segCwnd = tcb->GetCwndInSegments();

  if (m_epochStart == Time::Min()) {
    m_epochStart = Simulator::Now();

    if (m_lastMaxCwnd <= segCwnd) {
      NS_LOG_DEBUG("lastMaxCwnd <= m_cWnd. K=0 and origin=" << segCwnd);
      m_bicK = 0.0;
      m_bicOriginPoint = segCwnd;
    } else {
      m_bicK = std::pow((m_lastMaxCwnd - segCwnd) / m_c, 1 / 3.);
      m_bicOriginPoint = m_lastMaxCwnd;
      NS_LOG_DEBUG("lastMaxCwnd > m_cWnd. K=" << m_bicK << " and origin="
                                              << m_lastMaxCwnd);
    }
  }

  t = Simulator::Now() + m_delayMin - m_epochStart;

  if (t.GetSeconds() < m_bicK) {
    offs = m_bicK - t.GetSeconds();
    NS_LOG_DEBUG("t=" << t.GetSeconds() << " <k: offs=" << offs);
  } else {
    offs = t.GetSeconds() - m_bicK;
    NS_LOG_DEBUG("t=" << t.GetSeconds() << " >= k: offs=" << offs);
  }

  delta = m_c * std::pow(offs, 3);

  NS_LOG_DEBUG("delta: " << delta);

  if (t.GetSeconds() < m_bicK) {
    bicTarget = m_bicOriginPoint - delta;
    NS_LOG_DEBUG("t < k: Bic Target: " << bicTarget);
  } else {
    bicTarget = m_bicOriginPoint + delta;
    NS_LOG_DEBUG("t >= k: Bic Target: " << bicTarget);
  }

  if (bicTarget > segCwnd) {
    cnt = segCwnd / (bicTarget - segCwnd);
    NS_LOG_DEBUG("target>cwnd. cnt=" << cnt);
  } else {
    cnt = 100 * segCwnd;
  }

  if (m_lastMaxCwnd == 0 && cnt > m_cntClamp) {
    cnt = m_cntClamp;
  }

  return std::max(cnt, 2U);
}

void TcpCubic::PktsAcked(Ptr<TcpSocketState> tcb, uint32_t segmentsAcked,
                         const Time &rtt) {
  NS_LOG_FUNCTION(this << tcb << segmentsAcked << rtt);

  if (m_epochStart != Time::Min() &&
      (Simulator::Now() - m_epochStart) < m_cubicDelta) {
    return;
  }

  if (m_delayMin == Time::Min() || m_delayMin > rtt) {
    m_delayMin = rtt;
  }

  if (m_hystart && tcb->m_cWnd <= tcb->m_ssThresh &&
      tcb->m_cWnd >= m_hystartLowWindow * tcb->m_segmentSize) {
    HystartUpdate(tcb, rtt);
  }
}

void TcpCubic::HystartUpdate(Ptr<TcpSocketState> tcb, const Time &delay) {
  NS_LOG_FUNCTION(this << delay);

  if (!m_found) {
    Time now = Simulator::Now();

    if ((now - m_lastAck) <= m_hystartAckDelta) {
      m_lastAck = now;

      if ((now - m_roundStart) > m_delayMin) {
        if (m_hystartDetect == HybridSSDetectionMode::PACKET_TRAIN ||
            m_hystartDetect == HybridSSDetectionMode::BOTH) {
          m_found = true;
        }
      }
    }

    if (m_sampleCnt < m_hystartMinSamples) {
      if (m_currRtt == Time::Min() || m_currRtt > delay) {
        m_currRtt = delay;
      }

      ++m_sampleCnt;
    } else if (m_currRtt > m_delayMin + HystartDelayThresh(m_delayMin)) {
      if (m_hystartDetect == HybridSSDetectionMode::DELAY ||
          m_hystartDetect == HybridSSDetectionMode::BOTH) {
        m_found = true;
      }
    }

    if (m_found) {
      NS_LOG_DEBUG("Exit from SS, immediately :-)");
      tcb->m_ssThresh = tcb->m_cWnd;
    }
  }
}

Time TcpCubic::HystartDelayThresh(const Time &t) const {
  NS_LOG_FUNCTION(this << t);

  Time ret = t;
  if (t > m_hystartDelayMax) {
    ret = m_hystartDelayMax;
  } else if (t < m_hystartDelayMin) {
    ret = m_hystartDelayMin;
  }

  return ret;
}

uint32_t TcpCubic::GetSsThresh(Ptr<const TcpSocketState> tcb,
                               uint32_t bytesInFlight) {
  NS_LOG_FUNCTION(this << tcb << bytesInFlight);

  uint32_t segCwnd = tcb->GetCwndInSegments();
  NS_LOG_DEBUG("Loss at cWnd=" << segCwnd << " segments in flight="
                               << bytesInFlight / tcb->m_segmentSize);

  if (segCwnd < m_lastMaxCwnd && m_fastConvergence) {
    m_lastMaxCwnd = (segCwnd * (1 + m_beta)) / 2;
  } else {
    m_lastMaxCwnd = segCwnd;
  }

  m_epochStart = Time::Min();

  uint32_t ssThresh = std::max(static_cast<uint32_t>(segCwnd * m_beta), 2U) *
                      tcb->m_segmentSize;

  NS_LOG_DEBUG("SsThresh = " << ssThresh);

  return ssThresh;
}

void TcpCubic::CongestionStateSet(
    Ptr<TcpSocketState> tcb, const TcpSocketState::TcpCongState_t newState) {
  NS_LOG_FUNCTION(this << tcb << newState);

  if (newState == TcpSocketState::CA_LOSS) {
    CubicReset(tcb);
    HystartReset(tcb);
  }
}

void TcpCubic::CubicReset(Ptr<const TcpSocketState> tcb) {
  NS_LOG_FUNCTION(this << tcb);

  m_lastMaxCwnd = 0;
  m_bicOriginPoint = 0;
  m_bicK = 0;
  m_delayMin = Time::Min();
  m_found = false;
}

Ptr<TcpCongestionOps> TcpCubic::Fork() {
  NS_LOG_FUNCTION(this);
  return CopyObject<TcpCubic>(this);
}

} // namespace ns3
