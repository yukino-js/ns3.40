
#ifndef BLOCK_ACK_AGREEMENT_H
#define BLOCK_ACK_AGREEMENT_H

#include "block-ack-type.h"

#include "ns3/event-id.h"
#include "ns3/mac48-address.h"

namespace ns3 {
class BlockAckAgreement {
  friend class HtFrameExchangeManager;

public:
  BlockAckAgreement(Mac48Address peer, uint8_t tid);
  virtual ~BlockAckAgreement();
  void SetBufferSize(uint16_t bufferSize);
  void SetTimeout(uint16_t timeout);
  void SetStartingSequence(uint16_t seq);
  void SetStartingSequenceControl(uint16_t seq);
  void SetImmediateBlockAck();
  void SetDelayedBlockAck();
  void SetAmsduSupport(bool supported);
  uint8_t GetTid() const;
  Mac48Address GetPeer() const;
  uint16_t GetBufferSize() const;
  uint16_t GetTimeout() const;
  virtual uint16_t GetStartingSequence() const;
  uint16_t GetStartingSequenceControl() const;
  uint16_t GetWinEnd() const;
  bool IsImmediateBlockAck() const;
  bool IsAmsduSupported() const;
  void SetHtSupported(bool htSupported);
  bool IsHtSupported() const;
  BlockAckType GetBlockAckType() const;
  BlockAckReqType GetBlockAckReqType() const;
  static std::size_t GetDistance(uint16_t seqNumber,
                                 uint16_t startingSeqNumber);

protected:
  Mac48Address m_peer;
  uint8_t m_amsduSupported;
  uint8_t m_blockAckPolicy;
  uint8_t m_tid;
  uint16_t m_bufferSize;
  uint16_t m_timeout;
  uint16_t m_startingSeq;
  uint16_t m_winEnd;
  uint8_t m_htSupported;
  mutable EventId m_inactivityEvent;
};

} // namespace ns3

#endif
