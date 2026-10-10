
#ifndef TCP_DCTCP_H
#define TCP_DCTCP_H

#include "tcp-congestion-ops.h"
#include "tcp-linux-reno.h"

#include "ns3/traced-callback.h"

namespace ns3 {

class TcpDctcp : public TcpLinuxReno {
public:
  static TypeId GetTypeId();

  TcpDctcp();

  TcpDctcp(const TcpDctcp &sock);

  ~TcpDctcp() override;

  std::string GetName() const override;

  void Init(Ptr<TcpSocketState> tcb) override;

  typedef void (*CongestionEstimateTracedCallback)(uint32_t bytesAcked,
                                                   uint32_t bytesMarked,
                                                   double alpha);

  uint32_t GetSsThresh(Ptr<const TcpSocketState> tcb,
                       uint32_t bytesInFlight) override;
  Ptr<TcpCongestionOps> Fork() override;
  void PktsAcked(Ptr<TcpSocketState> tcb, uint32_t segmentsAcked,
                 const Time &rtt) override;
  void CwndEvent(Ptr<TcpSocketState> tcb,
                 const TcpSocketState::TcpCAEvent_t event) override;

private:
  void CeState0to1(Ptr<TcpSocketState> tcb);

  void CeState1to0(Ptr<TcpSocketState> tcb);

  void UpdateAckReserved(Ptr<TcpSocketState> tcb,
                         const TcpSocketState::TcpCAEvent_t event);

  void Reset(Ptr<TcpSocketState> tcb);

  void InitializeDctcpAlpha(double alpha);

  uint32_t m_ackedBytesEcn;
  uint32_t m_ackedBytesTotal;
  SequenceNumber32 m_priorRcvNxt;
  bool m_priorRcvNxtFlag;
  double m_alpha;
  SequenceNumber32 m_nextSeq;
  bool m_nextSeqFlag;
  bool m_ceState;
  bool m_delayedAckReserved;
  double m_g;
  bool m_useEct0;
  bool m_initialized;
  TracedCallback<uint32_t, uint32_t, double> m_traceCongestionEstimate;
};

} // namespace ns3

#endif
