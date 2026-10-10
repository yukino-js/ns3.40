
#ifndef TCPYEAH_H
#define TCPYEAH_H

#include "tcp-recovery-ops.h"
#include "tcp-scalable.h"

namespace ns3 {

class TcpYeah : public TcpNewReno {
public:
  static TypeId GetTypeId();

  TcpYeah();

  TcpYeah(const TcpYeah &sock);
  ~TcpYeah() override;

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
  void EnableYeah(const SequenceNumber32 &nextTxSequence);

  void DisableYeah();

private:
  uint32_t m_alpha;
  uint32_t m_gamma;
  uint32_t m_delta;
  uint32_t m_epsilon;
  uint32_t m_phy;
  uint32_t m_rho;
  uint32_t m_zeta;

  uint32_t m_stcpAiFactor;
  Ptr<TcpScalable> m_stcp;
  Time m_baseRtt;
  Time m_minRtt;
  uint32_t m_cntRtt;
  bool m_doingYeahNow;
  SequenceNumber32 m_begSndNxt;
  uint32_t m_lastQ;
  uint32_t m_doingRenoNow;
  uint32_t m_renoCount;
  uint32_t m_fastCount;
};

} // namespace ns3

#endif
