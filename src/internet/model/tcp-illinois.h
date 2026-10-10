
#ifndef TCPILLINOIS_H
#define TCPILLINOIS_H

#include "tcp-congestion-ops.h"

namespace ns3 {

class TcpSocketState;

class TcpIllinois : public TcpNewReno {
public:
  static TypeId GetTypeId();

  TcpIllinois();

  TcpIllinois(const TcpIllinois &sock);
  ~TcpIllinois() override;

  std::string GetName() const override;

  uint32_t GetSsThresh(Ptr<const TcpSocketState> tcb,
                       uint32_t bytesInFlight) override;

  Ptr<TcpCongestionOps> Fork() override;

  void
  CongestionStateSet(Ptr<TcpSocketState> tcb,
                     const TcpSocketState::TcpCongState_t newState) override;

  void IncreaseWindow(Ptr<TcpSocketState> tcb, uint32_t segmentsAcked) override;

  void PktsAcked(Ptr<TcpSocketState> tcb, uint32_t segmentsAcked,
                 const Time &rtt) override;

protected:
private:
  void RecalcParam(uint32_t cWnd);

  void CalculateAlpha(double da, double dm);

  void CalculateBeta(double da, double dm);

  Time CalculateAvgDelay() const;

  Time CalculateMaxDelay() const;

  void Reset(const SequenceNumber32 &nextTxSequence);

private:
  Time m_sumRtt;
  uint32_t m_cntRtt;
  Time m_baseRtt;
  Time m_maxRtt;
  SequenceNumber32 m_endSeq;
  bool m_rttAbove;
  uint8_t m_rttLow;
  double m_alphaMin;
  double m_alphaMax;
  double m_alphaBase;
  double m_alpha;
  double m_betaMin;
  double m_betaMax;
  double m_betaBase;
  double m_beta;
  uint32_t m_winThresh;
  uint32_t m_theta;
  uint32_t m_ackCnt;
};

} // namespace ns3

#endif
