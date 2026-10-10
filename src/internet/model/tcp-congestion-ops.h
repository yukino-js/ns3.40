#ifndef TCPCONGESTIONOPS_H
#define TCPCONGESTIONOPS_H

#include "tcp-rate-ops.h"
#include "tcp-socket-state.h"

namespace ns3 {

class TcpCongestionOps : public Object {
public:
  static TypeId GetTypeId();

  TcpCongestionOps();

  TcpCongestionOps(const TcpCongestionOps &other);

  ~TcpCongestionOps() override;

  virtual std::string GetName() const = 0;

  virtual void Init(Ptr<TcpSocketState> tcb [[maybe_unused]]) {}

  virtual uint32_t GetSsThresh(Ptr<const TcpSocketState> tcb,
                               uint32_t bytesInFlight) = 0;

  virtual void IncreaseWindow(Ptr<TcpSocketState> tcb, uint32_t segmentsAcked);

  virtual void PktsAcked(Ptr<TcpSocketState> tcb, uint32_t segmentsAcked,
                         const Time &rtt);

  virtual void
  CongestionStateSet(Ptr<TcpSocketState> tcb,
                     const TcpSocketState::TcpCongState_t newState);

  virtual void CwndEvent(Ptr<TcpSocketState> tcb,
                         const TcpSocketState::TcpCAEvent_t event);

  virtual bool HasCongControl() const;

  virtual void CongControl(Ptr<TcpSocketState> tcb,
                           const TcpRateOps::TcpRateConnection &rc,
                           const TcpRateOps::TcpRateSample &rs);

  virtual Ptr<TcpCongestionOps> Fork() = 0;
};

class TcpNewReno : public TcpCongestionOps {
public:
  static TypeId GetTypeId();

  TcpNewReno();

  TcpNewReno(const TcpNewReno &sock);

  ~TcpNewReno() override;

  std::string GetName() const override;

  void IncreaseWindow(Ptr<TcpSocketState> tcb, uint32_t segmentsAcked) override;
  uint32_t GetSsThresh(Ptr<const TcpSocketState> tcb,
                       uint32_t bytesInFlight) override;
  Ptr<TcpCongestionOps> Fork() override;

protected:
  virtual uint32_t SlowStart(Ptr<TcpSocketState> tcb, uint32_t segmentsAcked);
  virtual void CongestionAvoidance(Ptr<TcpSocketState> tcb,
                                   uint32_t segmentsAcked);
};

} // namespace ns3

#endif
