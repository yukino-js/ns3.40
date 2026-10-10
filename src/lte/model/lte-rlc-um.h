
#ifndef LTE_RLC_UM_H
#define LTE_RLC_UM_H

#include "lte-rlc-sequence-number.h"
#include "lte-rlc.h"

#include <ns3/event-id.h>

#include <map>

namespace ns3 {

class LteRlcUm : public LteRlc {
public:
  LteRlcUm();
  ~LteRlcUm() override;
  static TypeId GetTypeId();
  void DoDispose() override;

  void DoTransmitPdcpPdu(Ptr<Packet> p) override;

  void DoNotifyTxOpportunity(
      LteMacSapUser::TxOpportunityParameters txOpParams) override;
  void DoNotifyHarqDeliveryFailure() override;
  void DoReceivePdu(LteMacSapUser::ReceivePduParameters rxPduParams) override;

private:
  void ExpireReorderingTimer();
  void ExpireRbsTimer();

  bool IsInsideReorderingWindow(SequenceNumber10 seqNumber);

  void ReassembleOutsideWindow();
  void ReassembleSnInterval(SequenceNumber10 lowSeqNumber,
                            SequenceNumber10 highSeqNumber);

  void ReassembleAndDeliver(Ptr<Packet> packet);

  void DoReportBufferStatus();

private:
  uint32_t m_maxTxBufferSize;
  uint32_t m_txBufferSize;

  struct TxPdu {
    TxPdu(const Ptr<Packet> &pdu, const Time &time)
        : m_pdu(pdu), m_waitingSince(time) {}

    TxPdu() = delete;

    Ptr<Packet> m_pdu;
    Time m_waitingSince;
  };

  std::vector<TxPdu> m_txBuffer;
  std::map<uint16_t, Ptr<Packet>> m_rxBuffer;
  std::vector<Ptr<Packet>> m_reasBuffer;

  std::list<Ptr<Packet>> m_sdusBuffer;

  SequenceNumber10 m_sequenceNumber;

  SequenceNumber10 m_vrUr;
  SequenceNumber10 m_vrUx;
  SequenceNumber10 m_vrUh;

  uint16_t m_windowSize;

  Time m_reorderingTimerValue;
  EventId m_reorderingTimer;
  EventId m_rbsTimer;
  bool m_enablePdcpDiscarding{false};
  uint32_t m_discardTimerMs{0};

  enum ReassemblingState_t { NONE = 0, WAITING_S0_FULL = 1, WAITING_SI_SF = 2 };

  ReassemblingState_t m_reassemblingState;
  Ptr<Packet> m_keepS0;

  SequenceNumber10 m_expectedSeqNumber;
};

} // namespace ns3

#endif
