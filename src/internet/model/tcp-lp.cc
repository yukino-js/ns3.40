
#include "tcp-lp.h"

#include "ns3/log.h"
#include "ns3/simulator.h"

namespace ns3 {

NS_LOG_COMPONENT_DEFINE("TcpLp");
NS_OBJECT_ENSURE_REGISTERED(TcpLp);

TypeId TcpLp::GetTypeId() {
  static TypeId tid = TypeId("ns3::TcpLp")
                          .SetParent<TcpNewReno>()
                          .AddConstructor<TcpLp>()
                          .SetGroupName("Internet");
  return tid;
}

TcpLp::TcpLp()
    : TcpNewReno(), m_flag(0), m_sOwd(0), m_owdMin(0xffffffff), m_owdMax(0),
      m_owdMaxRsv(0), m_lastDrop(Time(0)), m_inference(Time(0)) {
  NS_LOG_FUNCTION(this);
}

TcpLp::TcpLp(const TcpLp &sock)
    : TcpNewReno(sock), m_flag(sock.m_flag), m_sOwd(sock.m_sOwd),
      m_owdMin(sock.m_owdMin), m_owdMax(sock.m_owdMax),
      m_owdMaxRsv(sock.m_owdMaxRsv), m_lastDrop(sock.m_lastDrop),
      m_inference(sock.m_inference) {
  NS_LOG_FUNCTION(this);
}

TcpLp::~TcpLp() { NS_LOG_FUNCTION(this); }

Ptr<TcpCongestionOps> TcpLp::Fork() { return CopyObject<TcpLp>(this); }

void TcpLp::CongestionAvoidance(Ptr<TcpSocketState> tcb,
                                uint32_t segmentsAcked) {
  NS_LOG_FUNCTION(this << tcb << segmentsAcked);

  if (!(m_flag & LP_WITHIN_INF)) {
    TcpNewReno::CongestionAvoidance(tcb, segmentsAcked);
  }
}

uint32_t TcpLp::OwdCalculator(Ptr<TcpSocketState> tcb) {
  NS_LOG_FUNCTION(this << tcb);

  int64_t owd = 0;

  owd = tcb->m_rcvTimestampValue - tcb->m_rcvTimestampEchoReply;

  if (owd < 0) {
    owd = -owd;
  }
  if (owd > 0) {
    m_flag |= LP_VALID_OWD;
  } else {
    m_flag &= ~LP_VALID_OWD;
  }
  return owd;
}

void TcpLp::RttSample(Ptr<TcpSocketState> tcb) {
  NS_LOG_FUNCTION(this << tcb);

  uint32_t mowd = OwdCalculator(tcb);

  if (!(m_flag & LP_VALID_OWD)) {
    return;
  }

  if (mowd < m_owdMin) {
    m_owdMin = mowd;
  }

  if (mowd > m_owdMax) {
    if (mowd > m_owdMaxRsv) {
      if (m_owdMaxRsv == 0) {
        m_owdMax = mowd;
      } else {
        m_owdMax = m_owdMaxRsv;
      }
      m_owdMaxRsv = mowd;
    } else {
      m_owdMax = mowd;
    }
  }

  if (m_sOwd != 0) {
    mowd -= m_sOwd >> 3;
    m_sOwd += mowd;
  } else {
    m_sOwd = mowd << 3;
  }
}

void TcpLp::PktsAcked(Ptr<TcpSocketState> tcb, uint32_t segmentsAcked,
                      const Time &rtt) {
  NS_LOG_FUNCTION(this << tcb << segmentsAcked << rtt);

  if (!rtt.IsZero()) {
    RttSample(tcb);
  }

  Time timestamp = Simulator::Now();
  if (timestamp.GetMilliSeconds() > tcb->m_rcvTimestampEchoReply) {
    m_inference = 3 * (timestamp - MilliSeconds(tcb->m_rcvTimestampEchoReply));
  }

  if (!m_lastDrop.IsZero() && (timestamp - m_lastDrop < m_inference)) {
    m_flag |= LP_WITHIN_INF;
  } else {
    m_flag &= ~LP_WITHIN_INF;
  }

  if (m_sOwd >> 3 <= m_owdMin + 15 * (m_owdMax - m_owdMin) / 100) {
    m_flag |= LP_WITHIN_THR;
  } else {
    m_flag &= ~LP_WITHIN_THR;
  }

  if (m_flag & LP_WITHIN_THR) {
    return;
  }

  m_owdMin = m_sOwd >> 3;
  m_owdMax = m_sOwd >> 2;
  m_owdMaxRsv = m_sOwd >> 2;

  if (m_flag & LP_WITHIN_INF) {
    tcb->m_cWnd = 1U * tcb->m_segmentSize;
  }

  else {
    tcb->m_cWnd = std::max(tcb->m_cWnd.Get() >> 1U, 1U * tcb->m_segmentSize);
  }

  m_lastDrop = timestamp;
}

std::string TcpLp::GetName() const { return "TcpLp"; }
} // namespace ns3
