#ifndef ORIGINATOR_BLOCK_ACK_AGREEMENT_H
#define ORIGINATOR_BLOCK_ACK_AGREEMENT_H

#include "block-ack-agreement.h"
#include "block-ack-window.h"

class OriginatorBlockAckWindowTest;

namespace ns3 {

class WifiMpdu;

class OriginatorBlockAckAgreement : public BlockAckAgreement {
  friend class BlockAckManager;
  friend class ::OriginatorBlockAckWindowTest;

public:
  OriginatorBlockAckAgreement(Mac48Address recipient, uint8_t tid);
  ~OriginatorBlockAckAgreement() override;

  enum State { PENDING, ESTABLISHED, NO_REPLY, RESET, REJECTED };

  void SetState(State state);
  bool IsPending() const;
  bool IsEstablished() const;
  bool IsNoReply() const;
  bool IsReset() const;
  bool IsRejected() const;

  uint16_t GetStartingSequence() const override;

  std::size_t GetDistance(uint16_t seqNumber) const;

  void InitTxWindow();

  void NotifyTransmittedMpdu(Ptr<const WifiMpdu> mpdu);
  void NotifyAckedMpdu(Ptr<const WifiMpdu> mpdu);
  void NotifyDiscardedMpdu(Ptr<const WifiMpdu> mpdu);

private:
  void AdvanceTxWindow();

  State m_state;
  BlockAckWindow m_txWindow;
};

} // namespace ns3

#endif
