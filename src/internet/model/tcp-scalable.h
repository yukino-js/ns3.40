
#ifndef TCPSCALABLE_H
#define TCPSCALABLE_H

#include "tcp-congestion-ops.h"

namespace ns3 {

class TcpSocketState;

class TcpScalable : public TcpNewReno {
public:
  static TypeId GetTypeId();

  TcpScalable();

  TcpScalable(const TcpScalable &sock);
  ~TcpScalable() override;

  std::string GetName() const override;

  uint32_t GetSsThresh(Ptr<const TcpSocketState> tcb,
                       uint32_t bytesInFlight) override;

  Ptr<TcpCongestionOps> Fork() override;

protected:
  void CongestionAvoidance(Ptr<TcpSocketState> tcb,
                           uint32_t segmentsAcked) override;

private:
  uint32_t m_ackCnt;
  uint32_t m_aiFactor;
  double m_mdFactor;
};

} // namespace ns3

#endif
