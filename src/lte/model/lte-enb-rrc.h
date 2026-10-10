
#ifndef LTE_ENB_RRC_H
#define LTE_ENB_RRC_H

#include "component-carrier.h"
#include "epc-enb-s1-sap.h"
#include "epc-x2-sap.h"
#include "lte-anr-sap.h"
#include "lte-ccm-rrc-sap.h"
#include "lte-enb-cmac-sap.h"
#include "lte-enb-cphy-sap.h"
#include "lte-ffr-rrc-sap.h"
#include "lte-handover-management-sap.h"
#include "lte-mac-sap.h"
#include "lte-pdcp-sap.h"
#include "lte-rrc-sap.h"

#include <ns3/event-id.h>
#include <ns3/nstime.h>
#include <ns3/object.h>
#include <ns3/traced-callback.h>

#include <map>
#include <set>
#include <vector>

namespace ns3 {

class LteRadioBearerInfo;
class LteSignalingRadioBearerInfo;
class LteDataRadioBearerInfo;
class LteEnbRrc;
class Packet;

class UeManager : public Object {
  friend class LtePdcpSpecificLtePdcpSapUser<UeManager>;

public:
  enum State {
    INITIAL_RANDOM_ACCESS = 0,
    CONNECTION_SETUP,
    CONNECTION_REJECTED,
    ATTACH_REQUEST,
    CONNECTED_NORMALLY,
    CONNECTION_RECONFIGURATION,
    CONNECTION_REESTABLISHMENT,
    HANDOVER_PREPARATION,
    HANDOVER_JOINING,
    HANDOVER_PATH_SWITCH,
    HANDOVER_LEAVING,
    NUM_STATES
  };

  UeManager();

  UeManager(Ptr<LteEnbRrc> rrc, uint16_t rnti, State s,
            uint8_t componentCarrierId);

  ~UeManager() override;

protected:
  void DoInitialize() override;
  void DoDispose() override;

public:
  static TypeId GetTypeId();

  void SetSource(uint16_t sourceCellId, uint16_t sourceX2apId);

  void SetImsi(uint64_t imsi);

  void InitialContextSetupRequest();

  void SetupDataRadioBearer(EpsBearer bearer, uint8_t bearerId,
                            uint32_t gtpTeid,
                            Ipv4Address transportLayerAddress);

  void RecordDataRadioBearersToBeStarted();

  void StartDataRadioBearers();

  void ReleaseDataRadioBearer(uint8_t drbid);

  void ScheduleRrcConnectionReconfiguration();

  void PrepareHandover(uint16_t cellId);

  void RecvHandoverRequestAck(EpcX2SapUser::HandoverRequestAckParams params);

  LteRrcSap::RadioResourceConfigDedicated
  GetRadioResourceConfigForHandoverPreparationInfo();

  LteRrcSap::RrcConnectionReconfiguration
  GetRrcConnectionReconfigurationForHandover(uint8_t componentCarrierId);

  void SendData(uint8_t bid, Ptr<Packet> p);

  std::vector<EpcX2Sap::ErabToBeSetupItem> GetErabList();

  void SendUeContextRelease();

  void RecvHandoverPreparationFailure(uint16_t cellId);

  void RecvSnStatusTransfer(EpcX2SapUser::SnStatusTransferParams params);

  void RecvUeContextRelease(EpcX2SapUser::UeContextReleaseParams params);

  void RecvHandoverCancel(EpcX2SapUser::HandoverCancelParams params);

  void CompleteSetupUe(LteEnbRrcSapProvider::CompleteSetupUeParameters params);
  void RecvRrcConnectionRequest(LteRrcSap::RrcConnectionRequest msg);
  void
  RecvRrcConnectionSetupCompleted(LteRrcSap::RrcConnectionSetupCompleted msg);
  void RecvRrcConnectionReconfigurationCompleted(
      LteRrcSap::RrcConnectionReconfigurationCompleted msg);
  void RecvRrcConnectionReestablishmentRequest(
      LteRrcSap::RrcConnectionReestablishmentRequest msg);
  void RecvRrcConnectionReestablishmentComplete(
      LteRrcSap::RrcConnectionReestablishmentComplete msg);
  void RecvMeasurementReport(LteRrcSap::MeasurementReport msg);
  void RecvIdealUeContextRemoveRequest(uint16_t rnti);

  void CmacUeConfigUpdateInd(LteEnbCmacSapUser::UeConfig cmacParams);

  void DoReceivePdcpSdu(LtePdcpSapUser::ReceivePdcpSduParameters params);

  uint16_t GetRnti() const;

  uint64_t GetImsi() const;

  uint8_t GetComponentCarrierId() const;

  uint16_t GetSrsConfigurationIndex() const;

  void SetSrsConfigurationIndex(uint16_t srsConfIndex);

  State GetState() const;

  void
  SetPdschConfigDedicated(LteRrcSap::PdschConfigDedicated pdschConfigDedicated);

  void CancelPendingEvents();

  void SendRrcConnectionRelease();

  EpcX2Sap::HandoverPreparationFailureParams BuildHoPrepFailMsg();

  EpcX2Sap::HandoverCancelParams BuildHoCancelMsg();

  typedef void (*StateTracedCallback)(const uint64_t imsi,
                                      const uint16_t cellId,
                                      const uint16_t rnti, const State oldState,
                                      const State newState);

private:
  uint8_t AddDataRadioBearerInfo(Ptr<LteDataRadioBearerInfo> radioBearerInfo);

  Ptr<LteDataRadioBearerInfo> GetDataRadioBearerInfo(uint8_t drbid);

  void RemoveDataRadioBearerInfo(uint8_t drbid);

  LteRrcSap::RrcConnectionReconfiguration BuildRrcConnectionReconfiguration();

  LteRrcSap::NonCriticalExtensionConfiguration
  BuildNonCriticalExtensionConfigurationCa();

  LteRrcSap::RadioResourceConfigDedicated BuildRadioResourceConfigDedicated();

  uint8_t GetNewRrcTransactionIdentifier();

  uint8_t Lcid2Drbid(uint8_t lcid);

  uint8_t Drbid2Lcid(uint8_t drbid);

  uint8_t Lcid2Bid(uint8_t lcid);

  uint8_t Bid2Lcid(uint8_t bid);

  uint8_t Drbid2Bid(uint8_t drbid);

  uint8_t Bid2Drbid(uint8_t bid);

  void SendPacket(uint8_t bid, Ptr<Packet> p);

  void SwitchToState(State s);

  uint8_t m_lastAllocatedDrbid;

  std::map<uint8_t, Ptr<LteDataRadioBearerInfo>> m_drbMap;

  Ptr<LteSignalingRadioBearerInfo> m_srb0;
  Ptr<LteSignalingRadioBearerInfo> m_srb1;

  uint16_t m_rnti;
  uint64_t m_imsi;
  uint8_t m_componentCarrierId;

  uint8_t m_lastRrcTransactionIdentifier;

  LteRrcSap::PhysicalConfigDedicated m_physicalConfigDedicated;
  Ptr<LteEnbRrc> m_rrc;
  State m_state;

  LtePdcpSapUser *m_drbPdcpSapUser;

  bool m_pendingRrcConnectionReconfiguration;

  TracedCallback<uint64_t, uint16_t, uint16_t, State, State>
      m_stateTransitionTrace;

  TracedCallback<uint64_t, uint16_t, uint16_t, uint8_t> m_drbCreatedTrace;

  uint16_t m_sourceX2apId;
  uint16_t m_targetX2apId;
  uint16_t m_sourceCellId;
  uint16_t m_targetCellId;
  std::list<uint8_t> m_drbsToBeStarted;
  bool m_needPhyMacConfiguration;

  EventId m_connectionRequestTimeout;
  EventId m_connectionSetupTimeout;
  EventId m_connectionRejectedTimeout;
  EventId m_handoverJoiningTimeout;
  EventId m_handoverLeavingTimeout;

  bool m_caSupportConfigured;

  bool m_pendingStartDataRadioBearers;

  std::list<std::pair<uint8_t, Ptr<Packet>>> m_packetBuffer;
};

class LteEnbRrc : public Object {
  friend class EnbRrcMemberLteEnbCmacSapUser;
  friend class MemberLteHandoverManagementSapUser<LteEnbRrc>;
  friend class MemberLteAnrSapUser<LteEnbRrc>;
  friend class MemberLteFfrRrcSapUser<LteEnbRrc>;
  friend class MemberLteEnbRrcSapProvider<LteEnbRrc>;
  friend class MemberEpcEnbS1SapUser<LteEnbRrc>;
  friend class EpcX2SpecificEpcX2SapUser<LteEnbRrc>;
  friend class UeManager;
  friend class MemberLteCcmRrcSapUser<LteEnbRrc>;

public:
  LteEnbRrc();

  ~LteEnbRrc() override;

protected:
  void DoDispose() override;

public:
  static TypeId GetTypeId();

  void SetEpcX2SapProvider(EpcX2SapProvider *s);

  EpcX2SapUser *GetEpcX2SapUser();

  void SetLteEnbCmacSapProvider(LteEnbCmacSapProvider *s);

  void SetLteEnbCmacSapProvider(LteEnbCmacSapProvider *s, uint8_t pos);

  LteEnbCmacSapUser *GetLteEnbCmacSapUser();

  LteEnbCmacSapUser *GetLteEnbCmacSapUser(uint8_t pos);

  void SetLteHandoverManagementSapProvider(LteHandoverManagementSapProvider *s);

  LteHandoverManagementSapUser *GetLteHandoverManagementSapUser();

  void SetLteCcmRrcSapProvider(LteCcmRrcSapProvider *s);

  LteCcmRrcSapUser *GetLteCcmRrcSapUser();

  void SetLteAnrSapProvider(LteAnrSapProvider *s);

  LteAnrSapUser *GetLteAnrSapUser();

  void SetLteFfrRrcSapProvider(LteFfrRrcSapProvider *s);
  void SetLteFfrRrcSapProvider(LteFfrRrcSapProvider *s, uint8_t index);

  LteFfrRrcSapUser *GetLteFfrRrcSapUser();
  LteFfrRrcSapUser *GetLteFfrRrcSapUser(uint8_t index);

  void SetLteEnbRrcSapUser(LteEnbRrcSapUser *s);

  LteEnbRrcSapProvider *GetLteEnbRrcSapProvider();

  void SetLteMacSapProvider(LteMacSapProvider *s);

  void SetS1SapProvider(EpcEnbS1SapProvider *s);

  EpcEnbS1SapUser *GetS1SapUser();

  void SetLteEnbCphySapProvider(LteEnbCphySapProvider *s);

  void SetLteEnbCphySapProvider(LteEnbCphySapProvider *s, uint8_t pos);

  LteEnbCphySapUser *GetLteEnbCphySapUser();

  LteEnbCphySapUser *GetLteEnbCphySapUser(uint8_t pos);

  bool HasUeManager(uint16_t rnti) const;

  Ptr<UeManager> GetUeManager(uint16_t rnti);

  std::vector<uint8_t>
  AddUeMeasReportConfig(LteRrcSap::ReportConfigEutra config);

  void
  ConfigureCell(std::map<uint8_t, Ptr<ComponentCarrierBaseStation>> ccPhyConf);

  void ConfigureCarriers(
      std::map<uint8_t, Ptr<ComponentCarrierBaseStation>> ccPhyConf);

  void SetCellId(uint16_t m_cellId);

  void SetCellId(uint16_t m_cellId, uint8_t ccIndex);

  uint8_t CellToComponentCarrierId(uint16_t cellId);

  uint16_t ComponentCarrierToCellId(uint8_t componentCarrierId);

  bool HasCellId(uint16_t cellId) const;

  bool SendData(Ptr<Packet> p);

  void SetForwardUpCallback(Callback<void, Ptr<Packet>> cb);

  void ConnectionRequestTimeout(uint16_t rnti);

  void ConnectionSetupTimeout(uint16_t rnti);

  void ConnectionRejectedTimeout(uint16_t rnti);

  void HandoverJoiningTimeout(uint16_t rnti);

  void HandoverLeavingTimeout(uint16_t rnti);

  void SendHandoverRequest(uint16_t rnti, uint16_t cellId);

  void DoSendReleaseDataRadioBearer(uint64_t imsi, uint16_t rnti,
                                    uint8_t bearerId);

  void SendRrcConnectionRelease();

  enum LteEpsBearerToRlcMapping_t {
    RLC_SM_ALWAYS = 1,
    RLC_UM_ALWAYS = 2,
    RLC_AM_ALWAYS = 3,
    PER_BASED = 4
  };

  typedef void (*NewUeContextTracedCallback)(const uint16_t cellId,
                                             const uint16_t rnti);

  typedef void (*ConnectionHandoverTracedCallback)(const uint64_t imsi,
                                                   const uint16_t cellId,
                                                   const uint16_t rnti);

  typedef void (*HandoverStartTracedCallback)(const uint64_t imsi,
                                              const uint16_t cellId,
                                              const uint16_t rnti,
                                              const uint16_t targetCid);

  typedef void (*ReceiveReportTracedCallback)(
      const uint64_t imsi, const uint16_t cellId, const uint16_t rnti,
      const LteRrcSap::MeasurementReport report);

  typedef void (*TimerExpiryTracedCallback)(const uint64_t imsi,
                                            const uint16_t rnti,
                                            const uint16_t cellId,
                                            const std::string cause);

  typedef void (*HandoverFailureTracedCallback)(const uint64_t imsi,
                                                const uint16_t rnti,
                                                const uint16_t cellId);

private:
  void
  DoCompleteSetupUe(uint16_t rnti,
                    LteEnbRrcSapProvider::CompleteSetupUeParameters params);
  void DoRecvRrcConnectionRequest(uint16_t rnti,
                                  LteRrcSap::RrcConnectionRequest msg);
  void
  DoRecvRrcConnectionSetupCompleted(uint16_t rnti,
                                    LteRrcSap::RrcConnectionSetupCompleted msg);
  void DoRecvRrcConnectionReconfigurationCompleted(
      uint16_t rnti, LteRrcSap::RrcConnectionReconfigurationCompleted msg);
  void DoRecvRrcConnectionReestablishmentRequest(
      uint16_t rnti, LteRrcSap::RrcConnectionReestablishmentRequest msg);
  void DoRecvRrcConnectionReestablishmentComplete(
      uint16_t rnti, LteRrcSap::RrcConnectionReestablishmentComplete msg);
  void DoRecvMeasurementReport(uint16_t rnti, LteRrcSap::MeasurementReport msg);
  void DoRecvIdealUeContextRemoveRequest(uint16_t rnti);

  void DoInitialContextSetupRequest(
      EpcEnbS1SapUser::InitialContextSetupRequestParameters params);
  void DoDataRadioBearerSetupRequest(
      EpcEnbS1SapUser::DataRadioBearerSetupRequestParameters params);
  void DoPathSwitchRequestAcknowledge(
      EpcEnbS1SapUser::PathSwitchRequestAcknowledgeParameters params);

  void DoRecvHandoverRequest(EpcX2SapUser::HandoverRequestParams params);
  void DoRecvHandoverRequestAck(EpcX2SapUser::HandoverRequestAckParams params);
  void DoRecvHandoverPreparationFailure(
      EpcX2SapUser::HandoverPreparationFailureParams params);
  void DoRecvSnStatusTransfer(EpcX2SapUser::SnStatusTransferParams params);
  void DoRecvUeContextRelease(EpcX2SapUser::UeContextReleaseParams params);
  void DoRecvLoadInformation(EpcX2SapUser::LoadInformationParams params);
  void
  DoRecvResourceStatusUpdate(EpcX2SapUser::ResourceStatusUpdateParams params);
  void DoRecvUeData(EpcX2SapUser::UeDataParams params);
  void DoRecvHandoverCancel(EpcX2SapUser::HandoverCancelParams params);

  uint16_t DoAllocateTemporaryCellRnti(uint8_t componentCarrierId);
  void DoNotifyLcConfigResult(uint16_t rnti, uint8_t lcid, bool success);
  void DoRrcConfigurationUpdateInd(LteEnbCmacSapUser::UeConfig params);

  std::vector<uint8_t>
  DoAddUeMeasReportConfigForHandover(LteRrcSap::ReportConfigEutra reportConfig);
  uint8_t DoAddUeMeasReportConfigForComponentCarrier(
      LteRrcSap::ReportConfigEutra reportConfig);
  void DoSetNumberOfComponentCarriers(uint16_t numberOfComponentCarriers);

  void DoTriggerHandover(uint16_t rnti, uint16_t targetCellId);

  uint8_t
  DoAddUeMeasReportConfigForAnr(LteRrcSap::ReportConfigEutra reportConfig);

  uint8_t
  DoAddUeMeasReportConfigForFfr(LteRrcSap::ReportConfigEutra reportConfig);
  void DoSetPdschConfigDedicated(uint16_t rnti,
                                 LteRrcSap::PdschConfigDedicated pa);
  void DoSendLoadInformation(EpcX2Sap::LoadInformationParams params);

  uint16_t AddUe(UeManager::State state, uint8_t componentCarrierId);

  void RemoveUe(uint16_t rnti);

  TypeId GetRlcType(EpsBearer bearer);

  bool IsRandomAccessCompleted(uint16_t rnti);

public:
  void AddX2Neighbour(uint16_t cellId);

  void SetSrsPeriodicity(uint32_t p);

  uint32_t GetSrsPeriodicity() const;

  void SetCsgId(uint32_t csgId, bool csgIndication);

private:
  uint16_t GetNewSrsConfigurationIndex();

  void RemoveSrsConfigurationIndex(uint16_t srcCi);

  bool IsMaxSrsReached();

  uint8_t GetLogicalChannelGroup(EpsBearer bearer);

  uint8_t GetLogicalChannelPriority(EpsBearer bearer);

  void SendSystemInformation();

  Callback<void, Ptr<Packet>> m_forwardUpCallback;

  EpcX2SapUser *m_x2SapUser;
  EpcX2SapProvider *m_x2SapProvider;

  std::vector<LteEnbCmacSapUser *> m_cmacSapUser;
  std::vector<LteEnbCmacSapProvider *> m_cmacSapProvider;

  LteHandoverManagementSapUser *m_handoverManagementSapUser;
  LteHandoverManagementSapProvider *m_handoverManagementSapProvider;

  LteCcmRrcSapUser *m_ccmRrcSapUser;
  LteCcmRrcSapProvider *m_ccmRrcSapProvider;

  LteAnrSapUser *m_anrSapUser;
  LteAnrSapProvider *m_anrSapProvider;

  std::vector<LteFfrRrcSapUser *> m_ffrRrcSapUser;
  std::vector<LteFfrRrcSapProvider *> m_ffrRrcSapProvider;

  LteEnbRrcSapUser *m_rrcSapUser;
  LteEnbRrcSapProvider *m_rrcSapProvider;

  LteMacSapProvider *m_macSapProvider;

  EpcEnbS1SapProvider *m_s1SapProvider;
  EpcEnbS1SapUser *m_s1SapUser;

  std::vector<LteEnbCphySapUser *> m_cphySapUser;
  std::vector<LteEnbCphySapProvider *> m_cphySapProvider;

  bool m_configured;
  uint32_t m_dlEarfcn;
  uint32_t m_ulEarfcn;
  uint16_t m_dlBandwidth;
  uint16_t m_ulBandwidth;
  uint16_t m_lastAllocatedRnti;

  std::vector<LteRrcSap::SystemInformationBlockType1> m_sib1;

  std::map<uint16_t, Ptr<UeManager>> m_ueMap;

  LteRrcSap::MeasConfig m_ueMeasConfig;

  std::set<uint8_t> m_handoverMeasIds;
  std::set<uint8_t> m_anrMeasIds;
  std::set<uint8_t> m_ffrMeasIds;
  std::set<uint8_t> m_componentCarrierMeasIds;

  struct X2uTeidInfo {
    uint16_t rnti;
    uint8_t drbid;
  };

  std::map<uint32_t, X2uTeidInfo> m_x2uTeidInfoMap;

  uint8_t m_defaultTransmissionMode;
  LteEpsBearerToRlcMapping_t m_epsBearerToRlcMapping;
  Time m_systemInformationPeriodicity;
  uint16_t m_srsCurrentPeriodicityId;
  std::set<uint16_t> m_ueSrsConfigurationIndexSet;
  uint16_t m_lastAllocatedConfigurationIndex;
  bool m_reconfigureUes;

  int8_t m_qRxLevMin;
  bool m_admitHandoverRequest;
  bool m_admitRrcConnectionRequest;
  uint8_t m_rsrpFilterCoefficient;
  uint8_t m_rsrqFilterCoefficient;
  Time m_connectionRequestTimeoutDuration;
  Time m_connectionSetupTimeoutDuration;
  Time m_connectionRejectedTimeoutDuration;
  Time m_handoverJoiningTimeoutDuration;
  Time m_handoverLeavingTimeoutDuration;

  TracedCallback<uint16_t, uint16_t> m_newUeContextTrace;
  TracedCallback<uint64_t, uint16_t, uint16_t> m_connectionEstablishedTrace;
  TracedCallback<uint64_t, uint16_t, uint16_t> m_connectionReconfigurationTrace;
  TracedCallback<uint64_t, uint16_t, uint16_t, uint16_t> m_handoverStartTrace;
  TracedCallback<uint64_t, uint16_t, uint16_t> m_handoverEndOkTrace;
  TracedCallback<uint64_t, uint16_t, uint16_t, LteRrcSap::MeasurementReport>
      m_recvMeasurementReportTrace;
  TracedCallback<uint64_t, uint16_t, uint16_t> m_connectionReleaseTrace;
  TracedCallback<uint64_t, uint16_t, uint16_t, std::string> m_rrcTimeoutTrace;
  TracedCallback<uint64_t, uint16_t, uint16_t> m_handoverFailureNoPreambleTrace;
  TracedCallback<uint64_t, uint16_t, uint16_t> m_handoverFailureMaxRachTrace;
  TracedCallback<uint64_t, uint16_t, uint16_t> m_handoverFailureLeavingTrace;
  TracedCallback<uint64_t, uint16_t, uint16_t> m_handoverFailureJoiningTrace;

  uint16_t m_numberOfComponentCarriers;

  bool m_carriersConfigured;

  std::map<uint8_t, Ptr<ComponentCarrierBaseStation>> m_componentCarrierPhyConf;
};

} // namespace ns3

#endif
