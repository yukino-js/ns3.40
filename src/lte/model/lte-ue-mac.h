
#ifndef LTE_UE_MAC_ENTITY_H
#define LTE_UE_MAC_ENTITY_H

#include "ff-mac-common.h"
#include "lte-mac-sap.h"
#include "lte-ue-cmac-sap.h"
#include "lte-ue-phy-sap.h"

#include <ns3/event-id.h>
#include <ns3/nstime.h>
#include <ns3/packet-burst.h>
#include <ns3/packet.h>
#include <ns3/traced-callback.h>

#include <map>
#include <vector>

namespace ns3 {

class UniformRandomVariable;

class LteUeMac : public Object {
  friend class UeMemberLteUeCmacSapProvider;
  friend class UeMemberLteMacSapProvider;
  friend class UeMemberLteUePhySapUser;

public:
  static TypeId GetTypeId();

  LteUeMac();
  ~LteUeMac() override;
  void DoDispose() override;

  typedef void (*RaResponseTimeoutTracedCallback)(uint64_t imsi,
                                                  bool contention,
                                                  uint8_t preambleTxCounter,
                                                  uint8_t maxPreambleTxLimit);

  LteMacSapProvider *GetLteMacSapProvider();
  void SetLteUeCmacSapUser(LteUeCmacSapUser *s);
  LteUeCmacSapProvider *GetLteUeCmacSapProvider();

  void SetComponentCarrierId(uint8_t index);

  LteUePhySapUser *GetLteUePhySapUser();

  void SetLteUePhySapProvider(LteUePhySapProvider *s);

  void DoSubframeIndication(uint32_t frameNo, uint32_t subframeNo);

  int64_t AssignStreams(int64_t stream);

private:
  void DoTransmitPdu(LteMacSapProvider::TransmitPduParameters params);
  void
  DoReportBufferStatus(LteMacSapProvider::ReportBufferStatusParameters params);

  void DoConfigureRach(LteUeCmacSapProvider::RachConfig rc);
  void DoStartContentionBasedRandomAccessProcedure();
  void DoSetRnti(uint16_t rnti);
  void DoStartNonContentionBasedRandomAccessProcedure(uint16_t rnti,
                                                      uint8_t rapId,
                                                      uint8_t prachMask);
  void DoAddLc(uint8_t lcId,
               LteUeCmacSapProvider::LogicalChannelConfig lcConfig,
               LteMacSapUser *msu);
  void DoRemoveLc(uint8_t lcId);
  void DoReset();
  void DoNotifyConnectionSuccessful();
  void DoSetImsi(uint64_t imsi);

  void DoReceivePhyPdu(Ptr<Packet> p);
  void DoReceiveLteControlMessage(Ptr<LteControlMessage> msg);

  void RandomlySelectAndSendRaPreamble();
  void SendRaPreamble(bool contention);
  void StartWaitingForRaResponse();
  void RecvRaResponse(BuildRarListElement_s raResponse);
  void RaResponseTimeout(bool contention);
  void SendReportBufferStatus();
  void RefreshHarqProcessesPacketBuffer();

  uint8_t m_componentCarrierId;

private:
  struct LcInfo {
    LteUeCmacSapProvider::LogicalChannelConfig lcConfig;
    LteMacSapUser *macSapUser;
  };

  std::map<uint8_t, LcInfo> m_lcInfoMap;

  LteMacSapProvider *m_macSapProvider;

  LteUeCmacSapUser *m_cmacSapUser;
  LteUeCmacSapProvider *m_cmacSapProvider;

  LteUePhySapProvider *m_uePhySapProvider;
  LteUePhySapUser *m_uePhySapUser;

  std::map<uint8_t, LteMacSapProvider::ReportBufferStatusParameters>
      m_ulBsrReceived;

  Time m_bsrPeriodicity;
  Time m_bsrLast;

  bool m_freshUlBsr;

  uint8_t m_harqProcessId;
  std::vector<Ptr<PacketBurst>> m_miUlHarqProcessesPacket;
  std::vector<uint8_t> m_miUlHarqProcessesPacketTimer;

  uint16_t m_rnti;
  uint16_t m_imsi;

  bool m_rachConfigured;
  LteUeCmacSapProvider::RachConfig m_rachConfig;
  uint8_t m_raPreambleId;
  uint8_t m_preambleTransmissionCounter;
  uint16_t m_backoffParameter;
  EventId m_noRaResponseReceivedEvent;
  Ptr<UniformRandomVariable> m_raPreambleUniformVariable;

  uint32_t m_frameNo;
  uint32_t m_subframeNo;
  uint8_t m_raRnti;
  bool m_waitingForRaResponse;

  TracedCallback<uint64_t, bool, uint8_t, uint8_t> m_raResponseTimeoutTrace;
};

} // namespace ns3

#endif
