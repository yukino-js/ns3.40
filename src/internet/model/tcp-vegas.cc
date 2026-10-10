
#include "tcp-vegas.h"

#include "tcp-socket-state.h"

#include "ns3/log.h"

namespace ns3 {

NS_LOG_COMPONENT_DEFINE("TcpVegas");
NS_OBJECT_ENSURE_REGISTERED(TcpVegas);

TypeId TcpVegas::GetTypeId() {
  static TypeId tid =
      TypeId("ns3::TcpVegas")
          .SetParent<TcpNewReno>()
          .AddConstructor<TcpVegas>()
          .SetGroupName("Internet")
          .AddAttribute("Alpha", "Lower bound of packets in network",
                        UintegerValue(2),
                        MakeUintegerAccessor(&TcpVegas::m_alpha),
                        MakeUintegerChecker<uint32_t>())
          .AddAttribute("Beta", "Upper bound of packets in network",
                        UintegerValue(4),
                        MakeUintegerAccessor(&TcpVegas::m_beta),
                        MakeUintegerChecker<uint32_t>())
          .AddAttribute("Gamma", "Limit on increase", UintegerValue(1),
                        MakeUintegerAccessor(&TcpVegas::m_gamma),
                        MakeUintegerChecker<uint32_t>());
  return tid;
}

TcpVegas::TcpVegas()
    : TcpNewReno(), m_alpha(2), m_beta(4), m_gamma(1), m_baseRtt(Time::Max()),
      m_minRtt(Time::Max()), m_cntRtt(0), m_doingVegasNow(true),
      m_begSndNxt(0) {
  NS_LOG_FUNCTION(this);
}

TcpVegas::TcpVegas(const TcpVegas &sock)
    : TcpNewReno(sock), m_alpha(sock.m_alpha), m_beta(sock.m_beta),
      m_gamma(sock.m_gamma), m_baseRtt(sock.m_baseRtt), m_minRtt(sock.m_minRtt),
      m_cntRtt(sock.m_cntRtt), m_doingVegasNow(true), m_begSndNxt(0) {
  NS_LOG_FUNCTION(this);
}

TcpVegas::~TcpVegas() { NS_LOG_FUNCTION(this); }

Ptr<TcpCongestionOps> TcpVegas::Fork() { return CopyObject<TcpVegas>(this); }

void TcpVegas::PktsAcked(Ptr<TcpSocketState> tcb, uint32_t segmentsAcked,
                         const Time &rtt) {
  NS_LOG_FUNCTION(this << tcb << segmentsAcked << rtt);

  if (rtt.IsZero()) {
    return;
  }

  m_minRtt = std::min(m_minRtt, rtt);
  NS_LOG_DEBUG("Updated m_minRtt = " << m_minRtt);

  m_baseRtt = std::min(m_baseRtt, rtt);
  NS_LOG_DEBUG("Updated m_baseRtt = " << m_baseRtt);

  m_cntRtt++;
  NS_LOG_DEBUG("Updated m_cntRtt = " << m_cntRtt);
}

void TcpVegas::EnableVegas(Ptr<TcpSocketState> tcb) {
  NS_LOG_FUNCTION(this << tcb);

  m_doingVegasNow = true;
  m_begSndNxt = tcb->m_nextTxSequence;
  m_cntRtt = 0;
  m_minRtt = Time::Max();
}

void TcpVegas::DisableVegas() {
  NS_LOG_FUNCTION(this);

  m_doingVegasNow = false;
}

void TcpVegas::CongestionStateSet(
    Ptr<TcpSocketState> tcb, const TcpSocketState::TcpCongState_t newState) {
  NS_LOG_FUNCTION(this << tcb << newState);
  if (newState == TcpSocketState::CA_OPEN) {
    EnableVegas(tcb);
  } else {
    DisableVegas();
  }
}

void TcpVegas::IncreaseWindow(Ptr<TcpSocketState> tcb, uint32_t segmentsAcked) {
  NS_LOG_FUNCTION(this << tcb << segmentsAcked);

  if (!m_doingVegasNow) {
    NS_LOG_LOGIC("Vegas is not turned on, we follow NewReno algorithm.");
    TcpNewReno::IncreaseWindow(tcb, segmentsAcked);
    return;
  }

  if (tcb->m_lastAckedSeq >= m_begSndNxt) {

    NS_LOG_LOGIC("A Vegas cycle has finished, we adjust cwnd once per RTT.");

    m_begSndNxt = tcb->m_nextTxSequence;

    if (m_cntRtt <= 2) {
      NS_LOG_LOGIC("We do not have enough RTT samples to do Vegas, so we "
                   "behave like NewReno.");
      TcpNewReno::IncreaseWindow(tcb, segmentsAcked);
    } else {
      NS_LOG_LOGIC("We have enough RTT samples to perform Vegas calculations");
      uint32_t diff;
      uint32_t targetCwnd;
      uint32_t segCwnd = tcb->GetCwndInSegments();

      double tmp = m_baseRtt.GetSeconds() / m_minRtt.GetSeconds();
      targetCwnd = static_cast<uint32_t>(segCwnd * tmp);
      NS_LOG_DEBUG("Calculated targetCwnd = " << targetCwnd);
      NS_ASSERT(segCwnd >= targetCwnd);

      diff = segCwnd - targetCwnd;
      NS_LOG_DEBUG("Calculated diff = " << diff);

      if (diff > m_gamma && (tcb->m_cWnd < tcb->m_ssThresh)) {
        NS_LOG_LOGIC("We are going too fast. We need to slow down and "
                     "change to linear increase/decrease mode.");
        segCwnd = std::min(segCwnd, targetCwnd + 1);
        tcb->m_cWnd = segCwnd * tcb->m_segmentSize;
        tcb->m_ssThresh = GetSsThresh(tcb, 0);
        NS_LOG_DEBUG("Updated cwnd = " << tcb->m_cWnd
                                       << " ssthresh=" << tcb->m_ssThresh);
      } else if (tcb->m_cWnd < tcb->m_ssThresh) {
        NS_LOG_LOGIC("We are in slow start and diff < m_gamma, so we "
                     "follow NewReno slow start");
        TcpNewReno::SlowStart(tcb, segmentsAcked);
      } else {
        NS_LOG_LOGIC("We are in linear increase/decrease mode");
        if (diff > m_beta) {
          NS_LOG_LOGIC(
              "We are going too fast, so we slow down by decrementing cwnd");
          segCwnd--;
          tcb->m_cWnd = segCwnd * tcb->m_segmentSize;
          tcb->m_ssThresh = GetSsThresh(tcb, 0);
          NS_LOG_DEBUG("Updated cwnd = " << tcb->m_cWnd
                                         << " ssthresh=" << tcb->m_ssThresh);
        } else if (diff < m_alpha) {
          NS_LOG_LOGIC(
              "We are going too slow, so we speed up by incrementing cwnd");
          segCwnd++;
          tcb->m_cWnd = segCwnd * tcb->m_segmentSize;
          NS_LOG_DEBUG("Updated cwnd = " << tcb->m_cWnd
                                         << " ssthresh=" << tcb->m_ssThresh);
        } else {
          NS_LOG_LOGIC("We are sending at the right speed");
        }
      }
      tcb->m_ssThresh = std::max(tcb->m_ssThresh, 3 * tcb->m_cWnd / 4);
      NS_LOG_DEBUG("Updated ssThresh = " << tcb->m_ssThresh);
    }

    m_cntRtt = 0;
    m_minRtt = Time::Max();
  } else if (tcb->m_cWnd < tcb->m_ssThresh) {
    TcpNewReno::SlowStart(tcb, segmentsAcked);
  }
}

std::string TcpVegas::GetName() const { return "TcpVegas"; }

uint32_t TcpVegas::GetSsThresh(Ptr<const TcpSocketState> tcb,
                               uint32_t bytesInFlight) {
  NS_LOG_FUNCTION(this << tcb << bytesInFlight);
  return std::max(
      std::min(tcb->m_ssThresh.Get(), tcb->m_cWnd.Get() - tcb->m_segmentSize),
      2 * tcb->m_segmentSize);
}

} // namespace ns3
