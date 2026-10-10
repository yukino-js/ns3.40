
#ifndef TCPLINUXRENO_H
#define TCPLINUXRENO_H

#include "tcp-congestion-ops.h"
#include "tcp-socket-state.h"

namespace ns3 {

class TcpLinuxReno : public TcpCongestionOps {
public:
  static TypeId GetTypeId();

  TcpLinuxReno();

  TcpLinuxReno(const TcpLinuxReno &sock);

  ~TcpLinuxReno() override;

  std::string GetName() const override;

  void IncreaseWindow(Ptr<TcpSocketState> tcb, uint32_t segmentsAcked) override;
  uint32_t GetSsThresh(Ptr<const TcpSocketState> tcb,
                       uint32_t bytesInFlight) override;
  Ptr<TcpCongestionOps> Fork() override;

protected:
  virtual uint32_t SlowStart(Ptr<TcpSocketState> tcb, uint32_t segmentsAcked);
  virtual void CongestionAvoidance(Ptr<TcpSocketState> tcb,
                                   uint32_t segmentsAcked);

private:
  uint32_t m_cWndCnt{0};
};

} // namespace ns3

#endif
