#include "tcp-congestion-ops.h"

#include "ns3/log.h"

namespace ns3 {

NS_LOG_COMPONENT_DEFINE("TcpCongestionOps");

NS_OBJECT_ENSURE_REGISTERED(TcpCongestionOps);

TypeId TcpCongestionOps::GetTypeId() {
  static TypeId tid = TypeId("ns3::TcpCongestionOps")
                          .SetParent<Object>()
                          .SetGroupName("Internet");
  return tid;
}

TcpCongestionOps::TcpCongestionOps() : Object() {}

TcpCongestionOps::TcpCongestionOps(const TcpCongestionOps &other)
    : Object(other) {}

TcpCongestionOps::~TcpCongestionOps() {}

void TcpCongestionOps::IncreaseWindow(Ptr<TcpSocketState> tcb,
                                      uint32_t segmentsAcked) {
  NS_LOG_FUNCTION(this << tcb << segmentsAcked);
}

void TcpCongestionOps::PktsAcked(Ptr<TcpSocketState> tcb,
                                 uint32_t segmentsAcked, const Time &rtt) {
  NS_LOG_FUNCTION(this << tcb << segmentsAcked << rtt);
}

void TcpCongestionOps::CongestionStateSet(
    Ptr<TcpSocketState> tcb, const TcpSocketState::TcpCongState_t newState) {
  NS_LOG_FUNCTION(this << tcb << newState);
}

void TcpCongestionOps::CwndEvent(Ptr<TcpSocketState> tcb,
                                 const TcpSocketState::TcpCAEvent_t event) {
  NS_LOG_FUNCTION(this << tcb << event);
}

bool TcpCongestionOps::HasCongControl() const { return false; }

void TcpCongestionOps::CongControl(Ptr<TcpSocketState> tcb,
                                   const TcpRateOps::TcpRateConnection &,
                                   const TcpRateOps::TcpRateSample &) {
  NS_LOG_FUNCTION(this << tcb);
}

NS_OBJECT_ENSURE_REGISTERED(TcpNewReno);

TypeId TcpNewReno::GetTypeId() {
  static TypeId tid = TypeId("ns3::TcpNewReno")
                          .SetParent<TcpCongestionOps>()
                          .SetGroupName("Internet")
                          .AddConstructor<TcpNewReno>();
  return tid;
}

TcpNewReno::TcpNewReno() : TcpCongestionOps() { NS_LOG_FUNCTION(this); }

TcpNewReno::TcpNewReno(const TcpNewReno &sock) : TcpCongestionOps(sock) {
  NS_LOG_FUNCTION(this);
}

TcpNewReno::~TcpNewReno() {}

uint32_t TcpNewReno::SlowStart(Ptr<TcpSocketState> tcb,
                               uint32_t segmentsAcked) {
  NS_LOG_FUNCTION(this << tcb << segmentsAcked);

  if (segmentsAcked >= 1) {
    tcb->m_cWnd += tcb->m_segmentSize;
    NS_LOG_INFO("In SlowStart, updated to cwnd " << tcb->m_cWnd << " ssthresh "
                                                 << tcb->m_ssThresh);
    return segmentsAcked - 1;
  }

  return 0;
}

void TcpNewReno::CongestionAvoidance(Ptr<TcpSocketState> tcb,
                                     uint32_t segmentsAcked) {
  NS_LOG_FUNCTION(this << tcb << segmentsAcked);

  if (segmentsAcked > 0) {
    double adder =
        static_cast<double>(tcb->m_segmentSize * tcb->m_segmentSize) /
        tcb->m_cWnd.Get();
    adder = std::max(1.0, adder);
    tcb->m_cWnd += static_cast<uint32_t>(adder);
    NS_LOG_INFO("In CongAvoid, updated to cwnd " << tcb->m_cWnd << " ssthresh "
                                                 << tcb->m_ssThresh);
  }
}

void TcpNewReno::IncreaseWindow(Ptr<TcpSocketState> tcb,
                                uint32_t segmentsAcked) {
  NS_LOG_FUNCTION(this << tcb << segmentsAcked);

  if (tcb->m_cWnd < tcb->m_ssThresh) {
    segmentsAcked = SlowStart(tcb, segmentsAcked);
  }

  if (tcb->m_cWnd >= tcb->m_ssThresh) {
    CongestionAvoidance(tcb, segmentsAcked);
  }
}

std::string TcpNewReno::GetName() const { return "TcpNewReno"; }

uint32_t TcpNewReno::GetSsThresh(Ptr<const TcpSocketState> state,
                                 uint32_t bytesInFlight) {
  NS_LOG_FUNCTION(this << state << bytesInFlight);

  return std::max(2 * state->m_segmentSize, bytesInFlight / 2);
}

Ptr<TcpCongestionOps> TcpNewReno::Fork() {
  return CopyObject<TcpNewReno>(this);
}

} // namespace ns3
