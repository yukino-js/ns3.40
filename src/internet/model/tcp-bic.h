
#ifndef TCPBIC_H
#define TCPBIC_H

#include "tcp-congestion-ops.h"
#include "tcp-recovery-ops.h"

class TcpBicIncrementTest;
class TcpBicDecrementTest;

namespace ns3 {

class TcpBic : public TcpCongestionOps {
public:
  static TypeId GetTypeId();

  TcpBic();

  TcpBic(const TcpBic &sock);

  std::string GetName() const override;
  void IncreaseWindow(Ptr<TcpSocketState> tcb, uint32_t segmentsAcked) override;
  uint32_t GetSsThresh(Ptr<const TcpSocketState> tcb,
                       uint32_t bytesInFlight) override;

  Ptr<TcpCongestionOps> Fork() override;

protected:
  virtual uint32_t Update(Ptr<TcpSocketState> tcb);

private:
  friend class ::TcpBicIncrementTest;
  friend class ::TcpBicDecrementTest;

  bool m_fastConvergence;
  double m_beta;
  uint32_t m_maxIncr;
  uint32_t m_lowWnd;
  uint32_t m_smoothPart;

  uint32_t m_cWndCnt;
  uint32_t m_lastMaxCwnd;
  uint32_t m_lastCwnd;
  Time m_epochStart;
  uint8_t m_b;
};

} // namespace ns3
#endif
