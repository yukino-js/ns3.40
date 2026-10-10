#ifndef TCP_RECOVERY_OPS_H
#define TCP_RECOVERY_OPS_H

#include "ns3/object.h"

namespace ns3 {

class TcpSocketState;

class TcpRecoveryOps : public Object {
public:
  static TypeId GetTypeId();

  TcpRecoveryOps();

  TcpRecoveryOps(const TcpRecoveryOps &other);

  ~TcpRecoveryOps() override;

  virtual std::string GetName() const = 0;

  virtual void EnterRecovery(Ptr<TcpSocketState> tcb, uint32_t dupAckCount,
                             uint32_t unAckDataCount,
                             uint32_t deliveredBytes) = 0;

  virtual void DoRecovery(Ptr<TcpSocketState> tcb, uint32_t deliveredBytes) = 0;

  virtual void ExitRecovery(Ptr<TcpSocketState> tcb) = 0;

  virtual void UpdateBytesSent(uint32_t bytesSent);

  virtual Ptr<TcpRecoveryOps> Fork() = 0;
};

class TcpClassicRecovery : public TcpRecoveryOps {
public:
  static TypeId GetTypeId();

  TcpClassicRecovery();

  TcpClassicRecovery(const TcpClassicRecovery &recovery);

  ~TcpClassicRecovery() override;

  std::string GetName() const override;

  void EnterRecovery(Ptr<TcpSocketState> tcb, uint32_t dupAckCount,
                     uint32_t unAckDataCount, uint32_t deliveredBytes) override;

  void DoRecovery(Ptr<TcpSocketState> tcb, uint32_t deliveredBytes) override;

  void ExitRecovery(Ptr<TcpSocketState> tcb) override;

  Ptr<TcpRecoveryOps> Fork() override;
};

} // namespace ns3

#endif
