
#ifndef LTE_RLC_TM_H
#define LTE_RLC_TM_H

#include "lte-rlc.h"

#include <ns3/event-id.h>

#include <map>

namespace ns3 {

class LteRlcTm : public LteRlc {
public:
  LteRlcTm();
  ~LteRlcTm() override;
  static TypeId GetTypeId();
  void DoDispose() override;

  void DoTransmitPdcpPdu(Ptr<Packet> p) override;

  void DoNotifyTxOpportunity(
      LteMacSapUser::TxOpportunityParameters txOpParams) override;
  void DoNotifyHarqDeliveryFailure() override;
  void DoReceivePdu(LteMacSapUser::ReceivePduParameters rxPduParams) override;

private:
  void ExpireRbsTimer();
  void DoReportBufferStatus();

private:
  struct TxPdu {
    TxPdu(const Ptr<Packet> &pdu, const Time &time)
        : m_pdu(pdu), m_waitingSince(time) {}

    TxPdu() = delete;

    Ptr<Packet> m_pdu;
    Time m_waitingSince;
  };

  std::vector<TxPdu> m_txBuffer;

  uint32_t m_maxTxBufferSize;
  uint32_t m_txBufferSize;

  EventId m_rbsTimer;
};

} // namespace ns3

#endif
