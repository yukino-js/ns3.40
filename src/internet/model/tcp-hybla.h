#ifndef TCPHYBLA_H
#define TCPHYBLA_H

#include "tcp-congestion-ops.h"

#include "ns3/traced-value.h"

namespace ns3 {

class TcpSocketState;

class TcpHybla : public TcpNewReno {
public:
  static TypeId GetTypeId();

  TcpHybla();

  TcpHybla(const TcpHybla &sock);

  ~TcpHybla() override;

  void PktsAcked(Ptr<TcpSocketState> tcb, uint32_t segmentsAcked,
                 const Time &rtt) override;
  std::string GetName() const override;
  Ptr<TcpCongestionOps> Fork() override;

protected:
  uint32_t SlowStart(Ptr<TcpSocketState> tcb, uint32_t segmentsAcked) override;
  void CongestionAvoidance(Ptr<TcpSocketState> tcb,
                           uint32_t segmentsAcked) override;

private:
  TracedValue<double> m_rho;
  Time m_rRtt;
  double m_cWndCnt;

private:
  void RecalcParam(const Ptr<TcpSocketState> &tcb);
};

} // namespace ns3

#endif
