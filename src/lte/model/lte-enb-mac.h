
#ifndef LTE_ENB_MAC_H
#define LTE_ENB_MAC_H

#include "ff-mac-csched-sap.h"
#include "ff-mac-sched-sap.h"
#include "lte-ccm-mac-sap.h"
#include "lte-common.h"
#include "lte-enb-cmac-sap.h"
#include "lte-enb-phy-sap.h"
#include "lte-mac-sap.h"

#include <ns3/nstime.h>
#include <ns3/packet-burst.h>
#include <ns3/packet.h>
#include <ns3/trace-source-accessor.h>
#include <ns3/traced-value.h>

#include <map>
#include <vector>

namespace ns3 {

class DlCqiLteControlMessage;
class UlCqiLteControlMessage;
class PdcchMapLteControlMessage;

typedef std::vector<std::vector<Ptr<PacketBurst>>> DlHarqProcessesBuffer_t;

class LteEnbMac : public Object {
  friend class EnbMacMemberLteEnbCmacSapProvider;
  friend class EnbMacMemberLteMacSapProvider<LteEnbMac>;
  friend class EnbMacMemberFfMacSchedSapUser;
  friend class EnbMacMemberFfMacCschedSapUser;
  friend class EnbMacMemberLteEnbPhySapUser;
  friend class MemberLteCcmMacSapProvider<LteEnbMac>;

public:
  static TypeId GetTypeId();

  LteEnbMac();
  ~LteEnbMac() override;
  void DoDispose() override;

  void SetComponentCarrierId(uint8_t index);
  void SetFfMacSchedSapProvider(FfMacSchedSapProvider *s);
  FfMacSchedSapUser *GetFfMacSchedSapUser();
  void SetFfMacCschedSapProvider(FfMacCschedSapProvider *s);
  FfMacCschedSapUser *GetFfMacCschedSapUser();

  void SetLteMacSapUser(LteMacSapUser *s);
  LteMacSapProvider *GetLteMacSapProvider();
  void SetLteEnbCmacSapUser(LteEnbCmacSapUser *s);
  LteEnbCmacSapProvider *GetLteEnbCmacSapProvider();

  LteEnbPhySapUser *GetLteEnbPhySapUser();

  void SetLteEnbPhySapProvider(LteEnbPhySapProvider *s);

  LteCcmMacSapProvider *GetLteCcmMacSapProvider();

  void SetLteCcmMacSapUser(LteCcmMacSapUser *s);

  typedef void (*DlSchedulingTracedCallback)(
      const uint32_t frame, const uint32_t subframe, const uint16_t rnti,
      const uint8_t mcs0, const uint16_t tbs0Size, const uint8_t mcs1,
      const uint16_t tbs1Size, const uint8_t ccId);

  typedef void (*UlSchedulingTracedCallback)(const uint32_t frame,
                                             const uint32_t subframe,
                                             const uint16_t rnti,
                                             const uint8_t mcs,
                                             const uint16_t tbsSize);

private:
  void ReceiveDlCqiLteControlMessage(Ptr<DlCqiLteControlMessage> msg);

  void DoReceiveLteControlMessage(Ptr<LteControlMessage> msg);

  void ReceiveBsrMessage(MacCeListElement_s bsr);

  void DoUlCqiReport(FfMacSchedSapProvider::SchedUlCqiInfoReqParameters ulcqi);

  void DoConfigureMac(uint16_t ulBandwidth, uint16_t dlBandwidth);
  void DoAddUe(uint16_t rnti);
  void DoRemoveUe(uint16_t rnti);
  void DoAddLc(LteEnbCmacSapProvider::LcInfo lcinfo, LteMacSapUser *msu);
  void DoReconfigureLc(LteEnbCmacSapProvider::LcInfo lcinfo);
  void DoReleaseLc(uint16_t rnti, uint8_t lcid);
  void DoUeUpdateConfigurationReq(LteEnbCmacSapProvider::UeConfig params);
  LteEnbCmacSapProvider::RachConfig DoGetRachConfig() const;
  LteEnbCmacSapProvider::AllocateNcRaPreambleReturnValue
  DoAllocateNcRaPreamble(uint16_t rnti);

  void DoTransmitPdu(LteMacSapProvider::TransmitPduParameters params);
  void
  DoReportBufferStatus(LteMacSapProvider::ReportBufferStatusParameters params);

  void DoCschedCellConfigCnf(
      FfMacCschedSapUser::CschedCellConfigCnfParameters params);
  void
  DoCschedUeConfigCnf(FfMacCschedSapUser::CschedUeConfigCnfParameters params);
  void
  DoCschedLcConfigCnf(FfMacCschedSapUser::CschedLcConfigCnfParameters params);
  void
  DoCschedLcReleaseCnf(FfMacCschedSapUser::CschedLcReleaseCnfParameters params);
  void
  DoCschedUeReleaseCnf(FfMacCschedSapUser::CschedUeReleaseCnfParameters params);
  void DoCschedUeConfigUpdateInd(
      FfMacCschedSapUser::CschedUeConfigUpdateIndParameters params);
  void DoCschedCellConfigUpdateInd(
      FfMacCschedSapUser::CschedCellConfigUpdateIndParameters params);

  void DoSchedDlConfigInd(FfMacSchedSapUser::SchedDlConfigIndParameters ind);
  void DoSchedUlConfigInd(FfMacSchedSapUser::SchedUlConfigIndParameters params);

  void DoSubframeIndication(uint32_t frameNo, uint32_t subframeNo);
  void DoReceiveRachPreamble(uint8_t prachId);

  void DoReportMacCeToScheduler(MacCeListElement_s bsr);

  void DoReportSrToScheduler(uint16_t rnti [[maybe_unused]]) {}

public:
  void DoReceivePhyPdu(Ptr<Packet> p);

private:
  void DoUlInfoListElementHarqFeedback(UlInfoListElement_s params);
  void DoDlInfoListElementHarqFeedback(DlInfoListElement_s params);

  std::map<uint16_t, std::map<uint8_t, LteMacSapUser *>> m_rlcAttached;

  std::vector<CqiListElement_s> m_dlCqiReceived;
  std::vector<FfMacSchedSapProvider::SchedUlCqiInfoReqParameters>
      m_ulCqiReceived;
  std::vector<MacCeListElement_s> m_ulCeReceived;

  std::vector<DlInfoListElement_s> m_dlInfoListReceived;

  std::vector<UlInfoListElement_s> m_ulInfoListReceived;

  LteMacSapProvider *m_macSapProvider;
  LteEnbCmacSapProvider *m_cmacSapProvider;
  LteMacSapUser *m_macSapUser;
  LteEnbCmacSapUser *m_cmacSapUser;

  FfMacSchedSapProvider *m_schedSapProvider;
  FfMacCschedSapProvider *m_cschedSapProvider;
  FfMacSchedSapUser *m_schedSapUser;
  FfMacCschedSapUser *m_cschedSapUser;

  LteEnbPhySapProvider *m_enbPhySapProvider;
  LteEnbPhySapUser *m_enbPhySapUser;

  LteCcmMacSapProvider *m_ccmMacSapProvider;
  LteCcmMacSapUser *m_ccmMacSapUser;
  uint32_t m_frameNo;
  uint32_t m_subframeNo;
  TracedCallback<DlSchedulingCallbackInfo> m_dlScheduling;

  TracedCallback<uint32_t, uint32_t, uint16_t, uint8_t, uint16_t, uint8_t>
      m_ulScheduling;

  uint8_t m_macChTtiDelay;

  std::map<uint16_t, DlHarqProcessesBuffer_t> m_miDlHarqProcessesPackets;

  uint8_t m_numberOfRaPreambles;
  uint8_t m_preambleTransMax;
  uint8_t m_raResponseWindowSize;
  uint8_t m_connEstFailCount;

  struct NcRaPreambleInfo {
    uint16_t rnti;
    Time expiryTime;
  };

  std::map<uint8_t, NcRaPreambleInfo> m_allocatedNcRaPreambleMap;

  std::map<uint8_t, uint32_t> m_receivedRachPreambleCount;

  std::map<uint16_t, uint32_t> m_rapIdRntiMap;

  uint8_t m_componentCarrierId;
};

} // namespace ns3

#endif
