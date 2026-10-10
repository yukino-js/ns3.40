
#ifndef TCPHIGHSPEED_H
#define TCPHIGHSPEED_H

#include "tcp-congestion-ops.h"

namespace ns3 {

class TcpSocketState;

class TcpHighSpeed : public TcpNewReno {
public:
  static TypeId GetTypeId();

  TcpHighSpeed();

  TcpHighSpeed(const TcpHighSpeed &sock);
  ~TcpHighSpeed() override;

  std::string GetName() const override;

  uint32_t GetSsThresh(Ptr<const TcpSocketState> tcb,
                       uint32_t bytesInFlight) override;

  Ptr<TcpCongestionOps> Fork() override;

  static uint32_t TableLookupA(uint32_t w);

  static double TableLookupB(uint32_t w);

protected:
  void CongestionAvoidance(Ptr<TcpSocketState> tcb,
                           uint32_t segmentsAcked) override;

private:
  uint32_t m_ackCnt;
};

} // namespace ns3

#endif
