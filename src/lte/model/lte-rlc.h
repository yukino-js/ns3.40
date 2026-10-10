
#ifndef LTE_RLC_H
#define LTE_RLC_H

#include "lte-mac-sap.h"
#include "lte-rlc-sap.h"

#include "ns3/nstime.h"
#include "ns3/object.h"
#include "ns3/trace-source-accessor.h"
#include "ns3/traced-value.h"
#include "ns3/uinteger.h"
#include <ns3/packet.h>
#include <ns3/simple-ref-count.h>

namespace ns3 {

class LteRlc : public Object {
  friend class LteRlcSpecificLteMacSapUser;
  friend class LteRlcSpecificLteRlcSapProvider<LteRlc>;

public:
  LteRlc();
  ~LteRlc() override;
  static TypeId GetTypeId();
  void DoDispose() override;

  void SetRnti(uint16_t rnti);

  void SetLcId(uint8_t lcId);

  void SetPacketDelayBudgetMs(uint16_t packetDelayBudget);

  void SetLteRlcSapUser(LteRlcSapUser *s);

  LteRlcSapProvider *GetLteRlcSapProvider();

  void SetLteMacSapProvider(LteMacSapProvider *s);

  LteMacSapUser *GetLteMacSapUser();

  typedef void (*NotifyTxTracedCallback)(uint16_t rnti, uint8_t lcid,
                                         uint32_t bytes);

  typedef void (*ReceiveTracedCallback)(uint16_t rnti, uint8_t lcid,
                                        uint32_t bytes, uint64_t delay);

protected:
  virtual void DoTransmitPdcpPdu(Ptr<Packet> p) = 0;

  LteRlcSapUser *m_rlcSapUser;
  LteRlcSapProvider *m_rlcSapProvider;

  virtual void
  DoNotifyTxOpportunity(LteMacSapUser::TxOpportunityParameters params) = 0;
  virtual void DoNotifyHarqDeliveryFailure() = 0;
  virtual void DoReceivePdu(LteMacSapUser::ReceivePduParameters params) = 0;

  LteMacSapUser *m_macSapUser;
  LteMacSapProvider *m_macSapProvider;

  uint16_t m_rnti;
  uint8_t m_lcid;
  uint16_t m_packetDelayBudgetMs{UINT16_MAX};

  TracedCallback<uint16_t, uint8_t, uint32_t> m_txPdu;
  TracedCallback<uint16_t, uint8_t, uint32_t, uint64_t> m_rxPdu;
  TracedCallback<Ptr<const Packet>> m_txDropTrace;
};

class LteRlcSm : public LteRlc {
public:
  LteRlcSm();
  ~LteRlcSm() override;
  static TypeId GetTypeId();
  void DoInitialize() override;
  void DoDispose() override;

  void DoTransmitPdcpPdu(Ptr<Packet> p) override;
  void DoNotifyTxOpportunity(
      LteMacSapUser::TxOpportunityParameters txOpParams) override;
  void DoNotifyHarqDeliveryFailure() override;
  void DoReceivePdu(LteMacSapUser::ReceivePduParameters rxPduParams) override;

private:
  void ReportBufferStatus();
};

} // namespace ns3

#endif
