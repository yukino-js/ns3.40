
#ifndef LTE_UE_RRC_H
#define LTE_UE_RRC_H

#include "lte-as-sap.h"
#include "lte-pdcp-sap.h"
#include "lte-rrc-sap.h"
#include "lte-ue-ccm-rrc-sap.h"
#include "lte-ue-cmac-sap.h"
#include "lte-ue-cphy-sap.h"

#include <ns3/object.h>
#include <ns3/packet.h>
#include <ns3/traced-callback.h>

#include <map>
#include <set>
#include <vector>

namespace ns3 {

static const Time UE_MEASUREMENT_REPORT_DELAY = MicroSeconds(1);

class LteRlc;
class LteMacSapProvider;
class LteUeCmacSapUser;
class LteUeCmacSapProvider;
class LteDataRadioBearerInfo;
class LteSignalingRadioBearerInfo;

class LteUeRrc : public Object {
  friend class UeMemberLteUeCmacSapUser;
  friend class UeRrcMemberLteEnbCmacSapUser;
  friend class LtePdcpSpecificLtePdcpSapUser<LteUeRrc>;
  friend class MemberLteAsSapProvider<LteUeRrc>;
  friend class MemberLteUeCphySapUser<LteUeRrc>;
  friend class MemberLteUeRrcSapProvider<LteUeRrc>;
  friend class MemberLteUeCcmRrcSapUser<LteUeRrc>;

public:
  enum State {
    IDLE_START = 0,
    IDLE_CELL_SEARCH,
    IDLE_WAIT_MIB_SIB1,
    IDLE_WAIT_MIB,
    IDLE_WAIT_SIB1,
    IDLE_CAMPED_NORMALLY,
    IDLE_WAIT_SIB2,
    IDLE_RANDOM_ACCESS,
    IDLE_CONNECTING,
    CONNECTED_NORMALLY,
    CONNECTED_HANDOVER,
    CONNECTED_PHY_PROBLEM,
    CONNECTED_REESTABLISHING,
    NUM_STATES
  };

  LteUeRrc();

  ~LteUeRrc() override;

private:
  void DoInitialize() override;
  void DoDispose() override;

public:
  static TypeId GetTypeId();

  void InitializeSap();

  void SetLteUeCphySapProvider(LteUeCphySapProvider *s);
  void SetLteUeCphySapProvider(LteUeCphySapProvider *s, uint8_t index);

  LteUeCphySapUser *GetLteUeCphySapUser();
  LteUeCphySapUser *GetLteUeCphySapUser(uint8_t index);

  void SetLteUeCmacSapProvider(LteUeCmacSapProvider *s);
  void SetLteUeCmacSapProvider(LteUeCmacSapProvider *s, uint8_t index);

  LteUeCmacSapUser *GetLteUeCmacSapUser();
  LteUeCmacSapUser *GetLteUeCmacSapUser(uint8_t index);

  void SetLteUeRrcSapUser(LteUeRrcSapUser *s);

  LteUeRrcSapProvider *GetLteUeRrcSapProvider();

  void SetLteMacSapProvider(LteMacSapProvider *s);

  void SetAsSapUser(LteAsSapUser *s);

  LteAsSapProvider *GetAsSapProvider();

  void SetLteCcmRrcSapProvider(LteUeCcmRrcSapProvider *s);

  LteUeCcmRrcSapUser *GetLteCcmRrcSapUser();

  void SetImsi(uint64_t imsi);

  void StorePreviousCellId(uint16_t cellId);

  uint64_t GetImsi() const;

  uint16_t GetRnti() const;

  uint16_t GetCellId() const;

  bool IsServingCell(uint16_t cellId) const;

  uint8_t GetUlBandwidth() const;

  uint8_t GetDlBandwidth() const;

  uint32_t GetDlEarfcn() const;

  uint32_t GetUlEarfcn() const;

  State GetState() const;

  uint16_t GetPreviousCellId() const;

  void SetUseRlcSm(bool val);

  static const std::string ToString(LteUeRrc::State s);

  typedef void (*CellSelectionTracedCallback)(uint64_t imsi, uint16_t cellId);

  typedef void (*ImsiCidRntiTracedCallback)(uint64_t imsi, uint16_t cellId,
                                            uint16_t rnti);

  typedef void (*MibSibHandoverTracedCallback)(uint64_t imsi, uint16_t cellId,
                                               uint16_t rnti,
                                               uint16_t otherCid);

  typedef void (*StateTracedCallback)(uint64_t imsi, uint16_t cellId,
                                      uint16_t rnti, State oldState,
                                      State newState);

  typedef void (*SCarrierConfiguredTracedCallback)(
      Ptr<LteUeRrc>, std::list<LteRrcSap::SCellToAddMod>);

  typedef void (*PhySyncDetectionTracedCallback)(uint64_t imsi, uint16_t rnti,
                                                 uint16_t cellId,
                                                 std::string type,
                                                 uint16_t count);

  typedef void (*ImsiCidRntiCountTracedCallback)(uint64_t imsi, uint16_t cellId,
                                                 uint16_t rnti, uint8_t count);

private:
  void DoReceivePdcpSdu(LtePdcpSapUser::ReceivePdcpSduParameters params);

  void DoSetTemporaryCellRnti(uint16_t rnti);
  void DoNotifyRandomAccessSuccessful();
  void DoNotifyRandomAccessFailed();

  void DoSetCsgWhiteList(uint32_t csgId);
  void DoForceCampedOnEnb(uint16_t cellId, uint32_t dlEarfcn);
  void DoStartCellSelection(uint32_t dlEarfcn);
  void DoConnect();
  void DoSendData(Ptr<Packet> packet, uint8_t bid);
  void DoDisconnect();

  void DoRecvMasterInformationBlock(uint16_t cellId,
                                    LteRrcSap::MasterInformationBlock msg);
  void
  DoRecvSystemInformationBlockType1(uint16_t cellId,
                                    LteRrcSap::SystemInformationBlockType1 msg);
  void
  DoReportUeMeasurements(LteUeCphySapUser::UeMeasurementsParameters params);

  void DoCompleteSetup(LteUeRrcSapProvider::CompleteSetupParameters params);
  void DoRecvSystemInformation(LteRrcSap::SystemInformation msg);
  void DoRecvRrcConnectionSetup(LteRrcSap::RrcConnectionSetup msg);
  void DoRecvRrcConnectionReconfiguration(
      LteRrcSap::RrcConnectionReconfiguration msg);
  void DoRecvRrcConnectionReestablishment(
      LteRrcSap::RrcConnectionReestablishment msg);
  void DoRecvRrcConnectionReestablishmentReject(
      LteRrcSap::RrcConnectionReestablishmentReject msg);
  void DoRecvRrcConnectionRelease(LteRrcSap::RrcConnectionRelease msg);
  void DoRecvRrcConnectionReject(LteRrcSap::RrcConnectionReject msg);

  void DoSetNumberOfComponentCarriers(uint16_t noOfComponentCarriers);

  void SynchronizeToStrongestCell();

  void EvaluateCellForSelection();

  void ApplyMeasConfig(LteRrcSap::MeasConfig mc);

  void SaveUeMeasurements(uint16_t cellId, double rsrp, double rsrq,
                          bool useLayer3Filtering, uint8_t componentCarrierId);

  void MeasurementReportTriggering(uint8_t measId);

  void SendMeasurementReport(uint8_t measId);

  void ApplyRadioResourceConfigDedicated(
      LteRrcSap::RadioResourceConfigDedicated rrcd);
  void ApplyRadioResourceConfigDedicatedSecondaryCarrier(
      LteRrcSap::NonCriticalExtensionConfiguration nonCec);
  void StartConnection();
  void LeaveConnectedMode();
  void DisposeOldSrb1();
  uint8_t Bid2Drbid(uint8_t bid);
  void SwitchToState(State s);

  std::map<uint8_t, uint8_t> m_bid2DrbidMap;

  std::vector<LteUeCphySapUser *> m_cphySapUser;
  std::vector<LteUeCphySapProvider *> m_cphySapProvider;

  std::vector<LteUeCmacSapUser *> m_cmacSapUser;
  std::vector<LteUeCmacSapProvider *> m_cmacSapProvider;

  LteUeRrcSapUser *m_rrcSapUser;
  LteUeRrcSapProvider *m_rrcSapProvider;

  LteMacSapProvider *m_macSapProvider;
  LtePdcpSapUser *m_drbPdcpSapUser;

  LteAsSapProvider *m_asSapProvider;
  LteAsSapUser *m_asSapUser;

  LteUeCcmRrcSapProvider *m_ccmRrcSapProvider;
  LteUeCcmRrcSapUser *m_ccmRrcSapUser;

  State m_state;

  uint64_t m_imsi;
  uint16_t m_rnti;
  uint16_t m_cellId;

  Ptr<LteSignalingRadioBearerInfo> m_srb0;
  Ptr<LteSignalingRadioBearerInfo> m_srb1;
  Ptr<LteSignalingRadioBearerInfo> m_srb1Old;
  std::map<uint8_t, Ptr<LteDataRadioBearerInfo>> m_drbMap;

  bool m_useRlcSm;

  uint8_t m_lastRrcTransactionIdentifier;

  LteRrcSap::PdschConfigDedicated m_pdschConfigDedicated;

  uint16_t m_dlBandwidth;
  uint16_t m_ulBandwidth;

  uint32_t m_dlEarfcn;
  uint32_t m_ulEarfcn;
  std::list<LteRrcSap::SCellToAddMod> m_sCellToAddModList;

  TracedCallback<uint64_t, uint16_t, uint16_t, uint16_t> m_mibReceivedTrace;
  TracedCallback<uint64_t, uint16_t, uint16_t, uint16_t> m_sib1ReceivedTrace;
  TracedCallback<uint64_t, uint16_t, uint16_t> m_sib2ReceivedTrace;
  TracedCallback<uint64_t, uint16_t, uint16_t, State, State>
      m_stateTransitionTrace;
  TracedCallback<uint64_t, uint16_t> m_initialCellSelectionEndOkTrace;
  TracedCallback<uint64_t, uint16_t> m_initialCellSelectionEndErrorTrace;
  TracedCallback<uint64_t, uint16_t, uint16_t> m_randomAccessSuccessfulTrace;
  TracedCallback<uint64_t, uint16_t, uint16_t> m_randomAccessErrorTrace;
  TracedCallback<uint64_t, uint16_t, uint16_t> m_connectionEstablishedTrace;
  TracedCallback<uint64_t, uint16_t, uint16_t, uint8_t>
      m_connectionTimeoutTrace;
  TracedCallback<uint64_t, uint16_t, uint16_t> m_connectionReconfigurationTrace;
  TracedCallback<uint64_t, uint16_t, uint16_t, uint16_t> m_handoverStartTrace;
  TracedCallback<uint64_t, uint16_t, uint16_t> m_handoverEndOkTrace;
  TracedCallback<uint64_t, uint16_t, uint16_t> m_handoverEndErrorTrace;
  TracedCallback<Ptr<LteUeRrc>, std::list<LteRrcSap::SCellToAddMod>>
      m_sCarrierConfiguredTrace;
  TracedCallback<uint64_t, uint16_t, uint16_t> m_srb1CreatedTrace;
  TracedCallback<uint64_t, uint16_t, uint16_t, uint8_t> m_drbCreatedTrace;
  TracedCallback<uint64_t, uint16_t, uint16_t, std::string, uint8_t>
      m_phySyncDetectionTrace;
  TracedCallback<uint64_t, uint16_t, uint16_t> m_radioLinkFailureTrace;

  bool m_connectionPending;
  bool m_hasReceivedMib;
  bool m_hasReceivedSib1;
  bool m_hasReceivedSib2;

  LteRrcSap::SystemInformationBlockType1 m_lastSib1;

  std::set<uint16_t> m_acceptableCell;

  uint32_t m_csgWhiteList;

  struct VarMeasConfig {
    std::map<uint8_t, LteRrcSap::MeasIdToAddMod> measIdList;
    std::map<uint8_t, LteRrcSap::MeasObjectToAddMod> measObjectList;
    std::map<uint8_t, LteRrcSap::ReportConfigToAddMod> reportConfigList;
    LteRrcSap::QuantityConfig quantityConfig;
    double aRsrp;
    double aRsrq;
  };

  VarMeasConfig m_varMeasConfig;

  struct VarMeasReport {
    uint8_t measId;
    std::set<uint16_t> cellsTriggeredList;
    uint32_t numberOfReportsSent;
    EventId periodicReportTimer;
  };

  std::map<uint8_t, VarMeasReport> m_varMeasReportList;

  typedef std::list<uint16_t> ConcernedCells_t;

  void VarMeasReportListAdd(uint8_t measId, ConcernedCells_t enteringCells);

  void VarMeasReportListErase(uint8_t measId, ConcernedCells_t leavingCells,
                              bool reportOnLeave);

  void VarMeasReportListClear(uint8_t measId);

  struct MeasValues {
    double rsrp;
    double rsrq;
    uint32_t carrierFreq;
  };

  std::map<uint16_t, MeasValues> m_storedMeasValues;

  std::map<uint16_t, std::map<uint8_t, MeasValues>>
      m_storedMeasValuesPerCarrier;

  std::map<uint16_t, MeasValues> m_storedScellMeasValues;

  struct PendingTrigger_t {
    uint8_t measId;
    ConcernedCells_t concernedCells;
    EventId timer;
  };

  std::map<uint8_t, std::list<PendingTrigger_t>> m_enteringTriggerQueue;

  std::map<uint8_t, std::list<PendingTrigger_t>> m_leavingTriggerQueue;

  void CancelEnteringTrigger(uint8_t measId);

  void CancelEnteringTrigger(uint8_t measId, uint16_t cellId);

  void CancelLeavingTrigger(uint8_t measId);

  void CancelLeavingTrigger(uint8_t measId, uint16_t cellId);

  Time m_t300;

  EventId m_connectionTimeout;

  void ConnectionTimeout();

  Time m_t310;

  uint8_t m_n310;

  uint8_t m_n311;

  EventId m_radioLinkFailureDetected;

  uint8_t m_noOfSyncIndications;

  bool m_leaveConnectedMode;

  uint16_t m_previousCellId;

  uint8_t m_connEstFailCountLimit;

  uint8_t m_connEstFailCount;
  void RadioLinkFailureDetected();

  void DoNotifyInSync();

  void DoNotifyOutOfSync();

  void DoResetSyncIndicationCounter();

  void ResetRlfParams();

public:
  uint16_t m_numberOfComponentCarriers;
};

} // namespace ns3

#endif
