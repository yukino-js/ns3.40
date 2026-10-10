
#ifndef TCPVENO_H
#define TCPVENO_H

#include "tcp-congestion-ops.h"

namespace ns3 {

class TcpSocketState;

class TcpVeno : public TcpNewReno {
public:
  static TypeId GetTypeId();

  TcpVeno();

  TcpVeno(const TcpVeno &sock);
  ~TcpVeno() override;

  std::string GetName() const override;

  void PktsAcked(Ptr<TcpSocketState> tcb, uint32_t segmentsAcked,
                 const Time &rtt) override;

  void
  CongestionStateSet(Ptr<TcpSocketState> tcb,
                     const TcpSocketState::TcpCongState_t newState) override;

  void IncreaseWindow(Ptr<TcpSocketState> tcb, uint32_t segmentsAcked) override;

  uint32_t GetSsThresh(Ptr<const TcpSocketState> tcb,
                       uint32_t bytesInFlight) override;

  Ptr<TcpCongestionOps> Fork() override;

protected:
private:
  void EnableVeno();

  void DisableVeno();

private:
  Time m_baseRtt;
  Time m_minRtt;
  uint32_t m_cntRtt;
  bool m_doingVenoNow;
  uint32_t m_diff;
  bool m_inc;
  uint32_t m_ackCnt;
  uint32_t m_beta;
};

} // namespace ns3

#endif
