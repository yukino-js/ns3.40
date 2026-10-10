
#ifndef LTE_RLC_AM_H
#define LTE_RLC_AM_H

#include "lte-rlc-sequence-number.h"
#include "lte-rlc.h"

#include <ns3/event-id.h>

#include <map>
#include <vector>

namespace ns3 {

class LteRlcAm : public LteRlc {
public:
  LteRlcAm();
  ~LteRlcAm() override;
  static TypeId GetTypeId();
  void DoDispose() override;

  void DoTransmitPdcpPdu(Ptr<Packet> p) override;

  void DoNotifyTxOpportunity(
      LteMacSapUser::TxOpportunityParameters txOpParams) override;
  void DoNotifyHarqDeliveryFailure() override;
  void DoReceivePdu(LteMacSapUser::ReceivePduParameters rxPduParams) override;

private:
  void ExpireReorderingTimer();
  void ExpirePollRetransmitTimer();
  void ExpireRbsTimer();

  void ExpireStatusProhibitTimer();

  bool IsInsideReceivingWindow(SequenceNumber10 seqNumber);

  void ReassembleAndDeliver(Ptr<Packet> packet);

  void DoReportBufferStatus();

private:
  struct TxPdu {
    TxPdu(const Ptr<Packet> &pdu, const Time &time)
        : m_pdu(pdu), m_waitingSince(time) {}

    TxPdu() = delete;

    Ptr<Packet> m_pdu;
    Time m_waitingSince;
  };

  std::vector<TxPdu> m_txonBuffer;

  struct RetxPdu {
    Ptr<Packet> m_pdu;
    uint16_t m_retxCount;
    Time m_waitingSince;
  };

  std::vector<RetxPdu> m_txedBuffer;
  std::vector<RetxPdu> m_retxBuffer;

  uint32_t m_maxTxBufferSize;
  uint32_t m_txonBufferSize;
  uint32_t m_retxBufferSize;
  uint32_t m_txedBufferSize;

  bool m_statusPduRequested;
  uint32_t m_statusPduBufferSize;

  struct PduBuffer {
    SequenceNumber10 m_seqNumber;
    std::list<Ptr<Packet>> m_byteSegments;

    bool m_pduComplete;
  };

  std::map<uint16_t, PduBuffer> m_rxonBuffer;

  Ptr<Packet> m_controlPduBuffer;

  std::list<Ptr<Packet>> m_sdusBuffer;

  SequenceNumber10 m_vtA;
  SequenceNumber10 m_vtMs;
  SequenceNumber10 m_vtS;
  SequenceNumber10 m_pollSn;

  SequenceNumber10 m_vrR;
  SequenceNumber10 m_vrMr;
  SequenceNumber10 m_vrX;
  SequenceNumber10 m_vrMs;
  SequenceNumber10 m_vrH;

  uint32_t m_pduWithoutPoll;
  uint32_t m_byteWithoutPoll;

  uint16_t m_windowSize;

  EventId m_pollRetransmitTimer;
  Time m_pollRetransmitTimerValue;
  EventId m_reorderingTimer;
  Time m_reorderingTimerValue;
  EventId m_statusProhibitTimer;
  Time m_statusProhibitTimerValue;
  EventId m_rbsTimer;
  Time m_rbsTimerValue;

  uint16_t m_maxRetxThreshold;
  uint16_t m_pollPdu;
  uint16_t m_pollByte;

  bool m_txOpportunityForRetxAlwaysBigEnough;
  bool m_pollRetransmitTimerJustExpired;

  enum ReassemblingState_t { NONE = 0, WAITING_S0_FULL = 1, WAITING_SI_SF = 2 };

  ReassemblingState_t m_reassemblingState;
  Ptr<Packet> m_keepS0;

  SequenceNumber10 m_expectedSeqNumber;
};

} // namespace ns3

#endif
