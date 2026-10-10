#ifndef TCP_PRR_RECOVERY_H
#define TCP_PRR_RECOVERY_H

#include "tcp-recovery-ops.h"

namespace ns3 {

class TcpPrrRecovery : public TcpClassicRecovery {
public:
  static TypeId GetTypeId();

  TcpPrrRecovery();

  TcpPrrRecovery(const TcpPrrRecovery &sock);

  ~TcpPrrRecovery() override;

  enum ReductionBound_t { CRB, SSRB };

  std::string GetName() const override;

  void EnterRecovery(Ptr<TcpSocketState> tcb, uint32_t dupAckCount,
                     uint32_t unAckDataCount, uint32_t deliveredBytes) override;

  void DoRecovery(Ptr<TcpSocketState> tcb, uint32_t deliveredBytes) override;

  void ExitRecovery(Ptr<TcpSocketState> tcb) override;

  void UpdateBytesSent(uint32_t bytesSent) override;

  Ptr<TcpRecoveryOps> Fork() override;

private:
  uint32_t m_prrDelivered{0};
  uint32_t m_prrOut{0};
  uint32_t m_recoveryFlightSize{0};
  ReductionBound_t m_reductionBoundMode{SSRB};
};
} // namespace ns3

#endif
