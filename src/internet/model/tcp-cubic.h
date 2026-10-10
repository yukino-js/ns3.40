
#ifndef TCPCUBIC_H
#define TCPCUBIC_H

#include "tcp-congestion-ops.h"
#include "tcp-socket-base.h"

namespace ns3 {

class TcpCubic : public TcpCongestionOps {
public:
  enum HybridSSDetectionMode {
    PACKET_TRAIN = 1,
    DELAY = 2,
    BOTH = 3,
  };

  static TypeId GetTypeId();

  TcpCubic();

  TcpCubic(const TcpCubic &sock);

  std::string GetName() const override;
  void PktsAcked(Ptr<TcpSocketState> tcb, uint32_t segmentsAcked,
                 const Time &rtt) override;
  void IncreaseWindow(Ptr<TcpSocketState> tcb, uint32_t segmentsAcked) override;
  uint32_t GetSsThresh(Ptr<const TcpSocketState> tcb,
                       uint32_t bytesInFlight) override;
  void
  CongestionStateSet(Ptr<TcpSocketState> tcb,
                     const TcpSocketState::TcpCongState_t newState) override;

  Ptr<TcpCongestionOps> Fork() override;

private:
  bool m_fastConvergence;
  double m_beta;

  bool m_hystart;
  HybridSSDetectionMode m_hystartDetect;
  uint32_t m_hystartLowWindow;
  Time m_hystartAckDelta;
  Time m_hystartDelayMin;
  Time m_hystartDelayMax;
  uint8_t m_hystartMinSamples;

  uint32_t m_initialCwnd;
  uint8_t m_cntClamp;

  double m_c;

  uint32_t m_cWndCnt;
  uint32_t m_lastMaxCwnd;
  uint32_t m_bicOriginPoint;
  double m_bicK;
  Time m_delayMin;
  Time m_epochStart;
  bool m_found;
  Time m_roundStart;
  SequenceNumber32 m_endSeq;
  Time m_lastAck;
  Time m_cubicDelta;
  Time m_currRtt;
  uint32_t m_sampleCnt;

private:
  void HystartReset(Ptr<const TcpSocketState> tcb);

  void CubicReset(Ptr<const TcpSocketState> tcb);

  uint32_t Update(Ptr<TcpSocketState> tcb);

  void HystartUpdate(Ptr<TcpSocketState> tcb, const Time &delay);

  Time HystartDelayThresh(const Time &t) const;
};

} // namespace ns3

#endif
