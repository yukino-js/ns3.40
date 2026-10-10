
#include "tcp-veno.h"

#include "tcp-socket-state.h"

#include "ns3/log.h"

namespace ns3 {

NS_LOG_COMPONENT_DEFINE("TcpVeno");
NS_OBJECT_ENSURE_REGISTERED(TcpVeno);

TypeId TcpVeno::GetTypeId() {
  static TypeId tid =
      TypeId("ns3::TcpVeno")
          .SetParent<TcpNewReno>()
          .AddConstructor<TcpVeno>()
          .SetGroupName("Internet")
          .AddAttribute("Beta", "Threshold for congestion detection",
                        UintegerValue(3),
                        MakeUintegerAccessor(&TcpVeno::m_beta),
                        MakeUintegerChecker<uint32_t>());
  return tid;
}

TcpVeno::TcpVeno()
    : TcpNewReno(), m_baseRtt(Time::Max()), m_minRtt(Time::Max()), m_cntRtt(0),
      m_doingVenoNow(true), m_diff(0), m_inc(true), m_ackCnt(0), m_beta(6) {
  NS_LOG_FUNCTION(this);
}

TcpVeno::TcpVeno(const TcpVeno &sock)
    : TcpNewReno(sock), m_baseRtt(sock.m_baseRtt), m_minRtt(sock.m_minRtt),
      m_cntRtt(sock.m_cntRtt), m_doingVenoNow(true), m_diff(0), m_inc(true),
      m_ackCnt(sock.m_ackCnt), m_beta(sock.m_beta) {
  NS_LOG_FUNCTION(this);
}

TcpVeno::~TcpVeno() { NS_LOG_FUNCTION(this); }

Ptr<TcpCongestionOps> TcpVeno::Fork() { return CopyObject<TcpVeno>(this); }

void TcpVeno::PktsAcked(Ptr<TcpSocketState> tcb, uint32_t segmentsAcked,
                        const Time &rtt) {
  NS_LOG_FUNCTION(this << tcb << segmentsAcked << rtt);

  if (rtt.IsZero()) {
    return;
  }

  m_minRtt = std::min(m_minRtt, rtt);
  NS_LOG_DEBUG("Updated m_minRtt= " << m_minRtt);

  m_baseRtt = std::min(m_baseRtt, rtt);
  NS_LOG_DEBUG("Updated m_baseRtt= " << m_baseRtt);

  m_cntRtt++;
  NS_LOG_DEBUG("Updated m_cntRtt= " << m_cntRtt);
}

void TcpVeno::EnableVeno() {
  NS_LOG_FUNCTION(this);

  m_doingVenoNow = true;
  m_minRtt = Time::Max();
}

void TcpVeno::DisableVeno() {
  NS_LOG_FUNCTION(this);

  m_doingVenoNow = false;
}

void TcpVeno::CongestionStateSet(
    Ptr<TcpSocketState> tcb, const TcpSocketState::TcpCongState_t newState) {
  NS_LOG_FUNCTION(this << tcb << newState);
  if (newState == TcpSocketState::CA_OPEN) {
    EnableVeno();
    NS_LOG_LOGIC("Veno is now on.");
  } else {
    DisableVeno();
    NS_LOG_LOGIC("Veno is turned off.");
  }
}

void TcpVeno::IncreaseWindow(Ptr<TcpSocketState> tcb, uint32_t segmentsAcked) {
  NS_LOG_FUNCTION(this << tcb << segmentsAcked);

  uint32_t targetCwnd;
  uint32_t segCwnd = tcb->GetCwndInSegments();

  double tmp = m_baseRtt.GetSeconds() / m_minRtt.GetSeconds();
  targetCwnd = static_cast<uint32_t>(segCwnd * tmp);
  NS_LOG_DEBUG("Calculated targetCwnd = " << targetCwnd);
  NS_ASSERT(segCwnd >= targetCwnd);

  m_diff = segCwnd - targetCwnd;
  NS_LOG_DEBUG("Calculated m_diff = " << m_diff);

  if (!m_doingVenoNow) {
    NS_LOG_LOGIC("Veno is not turned on, we follow NewReno algorithm.");
    TcpNewReno::IncreaseWindow(tcb, segmentsAcked);
    return;
  }

  if (m_cntRtt <= 2) {
    NS_LOG_LOGIC("We do not have enough RTT samples to perform Veno "
                 "calculations, we behave like NewReno.");
    TcpNewReno::IncreaseWindow(tcb, segmentsAcked);
  } else {
    NS_LOG_LOGIC("We have enough RTT samples to perform Veno calculations.");

    if (tcb->m_cWnd < tcb->m_ssThresh) {
      NS_LOG_LOGIC("We are in slow start, behave like NewReno.");
      TcpNewReno::SlowStart(tcb, segmentsAcked);
    } else {
      NS_LOG_LOGIC("We are in congestion avoidance, execute Veno additive "
                   "increase algo.");

      if (m_diff < m_beta) {
        NS_LOG_LOGIC("Available bandwidth not fully utilized, increase "
                     "cwnd by 1 every RTT");
        TcpNewReno::CongestionAvoidance(tcb, segmentsAcked);
      } else {
        NS_LOG_LOGIC("Available bandwidth fully utilized, increase cwnd "
                     "by 1 every other RTT");
        if (m_inc) {
          TcpNewReno::CongestionAvoidance(tcb, segmentsAcked);
          m_inc = false;
        } else {
          m_inc = true;
        }
      }
    }
  }

  m_cntRtt = 0;
  m_minRtt = Time::Max();
}

std::string TcpVeno::GetName() const { return "TcpVeno"; }

uint32_t TcpVeno::GetSsThresh(Ptr<const TcpSocketState> tcb,
                              uint32_t bytesInFlight) {
  NS_LOG_FUNCTION(this << tcb << bytesInFlight);

  if (m_diff < m_beta) {
    NS_LOG_LOGIC("Random loss is most likely to have occurred, "
                 "cwnd is reduced by 1/5");
    static double tmp = 4.0 / 5.0;
    return std::max(static_cast<uint32_t>(bytesInFlight * tmp),
                    2 * tcb->m_segmentSize);
  } else {
    NS_LOG_LOGIC("Congestive loss is most likely to have occurred, "
                 "cwnd is halved");
    return TcpNewReno::GetSsThresh(tcb, bytesInFlight);
  }
}

} // namespace ns3
