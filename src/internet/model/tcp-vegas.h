
#ifndef TCPVEGAS_H
#define TCPVEGAS_H

#include "tcp-congestion-ops.h"

namespace ns3 {

class TcpSocketState;

class TcpVegas : public TcpNewReno {
public:
  static TypeId GetTypeId();

  TcpVegas();

  TcpVegas(const TcpVegas &sock);
  ~TcpVegas() override;

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
  void EnableVegas(Ptr<TcpSocketState> tcb);

  void DisableVegas();

private:
  uint32_t m_alpha;
  uint32_t m_beta;
  uint32_t m_gamma;
  Time m_baseRtt;
  Time m_minRtt;
  uint32_t m_cntRtt;
  bool m_doingVegasNow;
  SequenceNumber32 m_begSndNxt;
};

} // namespace ns3

#endif
