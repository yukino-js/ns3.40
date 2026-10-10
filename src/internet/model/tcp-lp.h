
#ifndef TCPLP_H
#define TCPLP_H

#include "tcp-congestion-ops.h"

#include "ns3/traced-value.h"

namespace ns3 {

class TcpSocketState;

class TcpLp : public TcpNewReno {
public:
  static TypeId GetTypeId();

  TcpLp();

  TcpLp(const TcpLp &sock);

  ~TcpLp() override;

  void PktsAcked(Ptr<TcpSocketState> tcb, uint32_t segmentsAcked,
                 const Time &rtt) override;

  std::string GetName() const override;

  Ptr<TcpCongestionOps> Fork() override;

protected:
  void CongestionAvoidance(Ptr<TcpSocketState> tcb,
                           uint32_t segmentsAcked) override;

private:
  enum State {
    LP_VALID_OWD = (1 << 1),
    LP_WITHIN_THR = (1 << 3),
    LP_WITHIN_INF = (1 << 4),
  };

  uint32_t m_flag;
  uint32_t m_sOwd;
  uint32_t m_owdMin;
  uint32_t m_owdMax;
  uint32_t m_owdMaxRsv;
  Time m_lastDrop;
  Time m_inference;

private:
  uint32_t OwdCalculator(Ptr<TcpSocketState> tcb);

  void RttSample(Ptr<TcpSocketState> tcb);
};

} // namespace ns3

#endif
