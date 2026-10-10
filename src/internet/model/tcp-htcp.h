
#ifndef TCP_HTCP_H
#define TCP_HTCP_H

#include "tcp-congestion-ops.h"

namespace ns3 {

class TcpSocketState;

class TcpHtcp : public TcpNewReno {
public:
  static TypeId GetTypeId();
  TcpHtcp();
  TcpHtcp(const TcpHtcp &sock);
  ~TcpHtcp() override;
  std::string GetName() const override;
  Ptr<TcpCongestionOps> Fork() override;
  uint32_t GetSsThresh(Ptr<const TcpSocketState> tcb,
                       uint32_t bytesInFlight) override;

  void PktsAcked(Ptr<TcpSocketState> tcb, uint32_t segmentsAcked,
                 const Time &rtt) override;

protected:
  void CongestionAvoidance(Ptr<TcpSocketState> tcb,
                           uint32_t segmentsAcked) override;

private:
  void UpdateAlpha();

  void UpdateBeta();

  double m_alpha;
  double m_beta;
  double m_defaultBackoff;
  double m_throughputRatio;
  Time m_delta;
  Time m_deltaL;
  Time m_lastCon;
  Time m_minRtt;
  Time m_maxRtt;
  uint32_t m_throughput;
  uint32_t m_lastThroughput;
  uint32_t m_dataSent;
};

} // namespace ns3

#endif
