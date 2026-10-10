
#ifndef LTE_RRC_SAP_H
#define LTE_RRC_SAP_H

#include <ns3/ptr.h>
#include <ns3/simulator.h>

#include <list>
#include <stdint.h>

namespace ns3 {

class LteRlcSapUser;
class LtePdcpSapUser;
class LteRlcSapProvider;
class LtePdcpSapProvider;
class Packet;

class LteRrcSap {
public:
  virtual ~LteRrcSap();

  static const uint8_t MaxReportCells = 8;

  struct PlmnIdentityInfo {
    uint32_t plmnIdentity;
  };

  struct CellAccessRelatedInfo {
    PlmnIdentityInfo plmnIdentityInfo;
    uint32_t cellIdentity;
    bool csgIndication;
    uint32_t csgIdentity;
  };

  struct CellSelectionInfo {
    int8_t qRxLevMin;
    int8_t qQualMin;
  };

  struct FreqInfo {
    uint32_t ulCarrierFreq;
    uint16_t ulBandwidth;
  };

  struct RlcConfig {
    enum Direction {
      AM,
      UM_BI_DIRECTIONAL,
      UM_UNI_DIRECTIONAL_UL,
      UM_UNI_DIRECTIONAL_DL
    };

    Direction choice;
  };

  struct LogicalChannelConfig {
    uint8_t priority;
    uint16_t prioritizedBitRateKbps;
    uint16_t bucketSizeDurationMs;
    uint8_t logicalChannelGroup;
  };

  struct SoundingRsUlConfigCommon {
    enum Action { SETUP, RESET };

    Action type;

    uint16_t srsBandwidthConfig;
    uint8_t srsSubframeConfig;
  };

  struct SoundingRsUlConfigDedicated {
    enum Action { SETUP, RESET };

    Action type;

    uint16_t srsBandwidth;
    uint16_t srsConfigIndex;
  };

  struct AntennaInfoDedicated {
    uint8_t transmissionMode;
  };

  struct PdschConfigCommon {
    int8_t referenceSignalPower;
    int8_t pb;
  };

  struct PdschConfigDedicated {
    enum Db { dB_6, dB_4dot77, dB_3, dB_1dot77, dB0, dB1, dB2, dB3 };

    uint8_t pa;
  };

  static double ConvertPdschConfigDedicated2Double(
      PdschConfigDedicated pdschConfigDedicated) {
    double pa = 0;
    switch (pdschConfigDedicated.pa) {
    case PdschConfigDedicated::dB_6:
      pa = -6;
      break;
    case PdschConfigDedicated::dB_4dot77:
      pa = -4.77;
      break;
    case PdschConfigDedicated::dB_3:
      pa = -3;
      break;
    case PdschConfigDedicated::dB_1dot77:
      pa = -1.77;
      break;
    case PdschConfigDedicated::dB0:
      pa = 0;
      break;
    case PdschConfigDedicated::dB1:
      pa = 1;
      break;
    case PdschConfigDedicated::dB2:
      pa = 2;
      break;
    case PdschConfigDedicated::dB3:
      pa = 3;
      break;
    default:
      break;
    }
    return pa;
  }

  struct PhysicalConfigDedicated {
    bool haveSoundingRsUlConfigDedicated;
    SoundingRsUlConfigDedicated soundingRsUlConfigDedicated;
    bool haveAntennaInfoDedicated;
    AntennaInfoDedicated antennaInfo;
    bool havePdschConfigDedicated;
    PdschConfigDedicated pdschConfigDedicated;
  };

  struct SrbToAddMod {
    uint8_t srbIdentity;
    LogicalChannelConfig logicalChannelConfig;
  };

  struct DrbToAddMod {
    uint8_t epsBearerIdentity;
    uint8_t drbIdentity;
    RlcConfig rlcConfig;
    uint8_t logicalChannelIdentity;
    LogicalChannelConfig logicalChannelConfig;
  };

  struct PreambleInfo {
    uint8_t numberOfRaPreambles;
  };

  struct RaSupervisionInfo {
    uint8_t preambleTransMax;
    uint8_t raResponseWindowSize;
  };

  struct TxFailParam {
    uint8_t connEstFailCount{0};
  };

  struct RachConfigCommon {
    PreambleInfo preambleInfo;
    RaSupervisionInfo raSupervisionInfo;
    TxFailParam txFailParam;
  };

  struct RadioResourceConfigCommon {
    RachConfigCommon rachConfigCommon;
  };

  struct RadioResourceConfigCommonSib {
    RachConfigCommon rachConfigCommon;
    PdschConfigCommon pdschConfigCommon;
  };

  struct RadioResourceConfigDedicated {
    std::list<SrbToAddMod> srbToAddModList;
    std::list<DrbToAddMod> drbToAddModList;
    std::list<uint8_t> drbToReleaseList;
    bool havePhysicalConfigDedicated;
    PhysicalConfigDedicated physicalConfigDedicated;
  };

  struct QuantityConfig {
    uint8_t filterCoefficientRSRP;
    uint8_t filterCoefficientRSRQ;
  };

  struct CellsToAddMod {
    uint8_t cellIndex;
    uint16_t physCellId;
    int8_t cellIndividualOffset;
  };

  struct PhysCellIdRange {
    uint16_t start;
    bool haveRange;
    uint16_t range;
  };

  struct BlackCellsToAddMod {
    uint8_t cellIndex;
    PhysCellIdRange physCellIdRange;
  };

  struct MeasObjectEutra {
    uint32_t carrierFreq;
    uint16_t allowedMeasBandwidth;
    bool presenceAntennaPort1;
    uint8_t neighCellConfig;
    int8_t offsetFreq;
    std::list<uint8_t> cellsToRemoveList;
    std::list<CellsToAddMod> cellsToAddModList;
    std::list<uint8_t> blackCellsToRemoveList;
    std::list<BlackCellsToAddMod> blackCellsToAddModList;
    bool haveCellForWhichToReportCGI;
    uint16_t cellForWhichToReportCGI;
  };

  struct ThresholdEutra {
    enum { THRESHOLD_RSRP, THRESHOLD_RSRQ } choice;

    uint8_t range;
  };

  struct ReportConfigEutra {
    enum { EVENT, PERIODICAL } triggerType;

    enum {
      EVENT_A1,
      EVENT_A2,
      EVENT_A3,
      EVENT_A4,
      EVENT_A5

    } eventId;

    ThresholdEutra threshold1;
    ThresholdEutra threshold2;

    bool reportOnLeave;

    int8_t a3Offset;

    uint8_t hysteresis;

    uint16_t timeToTrigger;

    enum Report { REPORT_STRONGEST_CELLS, REPORT_CGI };

    Report purpose;

    enum { RSRP, RSRQ } triggerQuantity;

    enum { SAME_AS_TRIGGER_QUANTITY, BOTH } reportQuantity;

    uint8_t maxReportCells;

    enum {
      MS120,
      MS240,
      MS480,
      MS640,
      MS1024,
      MS2048,
      MS5120,
      MS10240,
      MIN1,
      MIN6,
      MIN12,
      MIN30,
      MIN60,
      SPARE3,
      SPARE2,
      SPARE1
    } reportInterval;

    uint8_t reportAmount;

    ReportConfigEutra();
  };

  struct MeasObjectToAddMod {
    uint8_t measObjectId;
    MeasObjectEutra measObjectEutra;
  };

  struct ReportConfigToAddMod {
    uint8_t reportConfigId;
    ReportConfigEutra reportConfigEutra;
  };

  struct MeasIdToAddMod {
    uint8_t measId;
    uint8_t measObjectId;
    uint8_t reportConfigId;
  };

  struct MeasGapConfig {
    enum Action { SETUP, RESET };

    Action type;

    enum Gap { GP0, GP1 };

    Gap gapOffsetChoice;

    uint8_t gapOffsetValue;
  };

  struct MobilityStateParameters {
    uint8_t tEvaluation;
    uint8_t tHystNormal;
    uint8_t nCellChangeMedium;
    uint8_t nCellChangeHigh;
  };

  struct SpeedStateScaleFactors {
    uint8_t sfMedium;
    uint8_t sfHigh;
  };

  struct SpeedStatePars {
    enum Action { SETUP, RESET };

    Action type;

    MobilityStateParameters mobilityStateParameters;
    SpeedStateScaleFactors timeToTriggerSf;
  };

  struct MeasConfig {
    std::list<uint8_t> measObjectToRemoveList;
    std::list<MeasObjectToAddMod> measObjectToAddModList;
    std::list<uint8_t> reportConfigToRemoveList;
    std::list<ReportConfigToAddMod> reportConfigToAddModList;
    std::list<uint8_t> measIdToRemoveList;
    std::list<MeasIdToAddMod> measIdToAddModList;
    bool haveQuantityConfig;
    QuantityConfig quantityConfig;
    bool haveMeasGapConfig;
    MeasGapConfig measGapConfig;
    bool haveSmeasure;
    uint8_t sMeasure;
    bool haveSpeedStatePars;
    SpeedStatePars speedStatePars;
  };

  struct CarrierFreqEutra {
    uint32_t dlCarrierFreq;
    uint32_t ulCarrierFreq;
  };

  struct CarrierBandwidthEutra {
    uint16_t dlBandwidth;
    uint16_t ulBandwidth;
  };

  struct RachConfigDedicated {
    uint8_t raPreambleIndex;
    uint8_t raPrachMaskIndex;
  };

  struct MobilityControlInfo {
    uint16_t targetPhysCellId;
    bool haveCarrierFreq;
    CarrierFreqEutra carrierFreq;
    bool haveCarrierBandwidth;
    CarrierBandwidthEutra carrierBandwidth;
    uint16_t newUeIdentity;
    RadioResourceConfigCommon radioResourceConfigCommon;
    bool haveRachConfigDedicated;
    RachConfigDedicated rachConfigDedicated;
  };

  struct ReestabUeIdentity {
    uint16_t cRnti;
    uint16_t physCellId;
  };

  enum ReestablishmentCause {
    RECONFIGURATION_FAILURE,
    HANDOVER_FAILURE,
    OTHER_FAILURE
  };

  struct MasterInformationBlock {
    uint16_t dlBandwidth;
    uint16_t systemFrameNumber;
  };

  struct SystemInformationBlockType1 {
    CellAccessRelatedInfo cellAccessRelatedInfo;
    CellSelectionInfo cellSelectionInfo;
  };

  struct SystemInformationBlockType2 {
    RadioResourceConfigCommonSib radioResourceConfigCommon;
    FreqInfo freqInfo;
  };

  struct SystemInformation {
    bool haveSib2;
    SystemInformationBlockType2 sib2;
  };

  struct AsConfig {
    MeasConfig sourceMeasConfig;
    RadioResourceConfigDedicated sourceRadioResourceConfig;
    uint16_t sourceUeIdentity;
    MasterInformationBlock sourceMasterInformationBlock;
    SystemInformationBlockType1 sourceSystemInformationBlockType1;
    SystemInformationBlockType2 sourceSystemInformationBlockType2;
    uint32_t sourceDlCarrierFreq;
  };

  struct CgiInfo {
    uint32_t plmnIdentity;
    uint32_t cellIdentity;
    uint16_t trackingAreaCode;
    std::list<uint32_t> plmnIdentityList;
  };

  struct MeasResultPCell {
    uint8_t rsrpResult;
    uint8_t rsrqResult;
  };

  struct MeasResultEutra {
    uint16_t physCellId;
    bool haveCgiInfo;
    CgiInfo cgiInfo;
    bool haveRsrpResult;
    uint8_t rsrpResult;
    bool haveRsrqResult;
    uint8_t rsrqResult;
  };

  struct MeasResultSCell {
    uint8_t rsrpResult;
    uint8_t rsrqResult;
  };

  struct MeasResultBestNeighCell {
    uint16_t physCellId;
    uint8_t rsrpResult;
    uint8_t rsrqResult;
  };

  struct MeasResultServFreq {
    uint16_t servFreqId;
    bool haveMeasResultSCell;
    MeasResultSCell measResultSCell;
    bool haveMeasResultBestNeighCell;
    MeasResultBestNeighCell measResultBestNeighCell;
  };

  struct MeasResults {
    uint8_t measId;
    MeasResultPCell measResultPCell;
    bool haveMeasResultNeighCells;
    std::list<MeasResultEutra> measResultListEutra;
    bool haveMeasResultServFreqList;
    std::list<MeasResultServFreq> measResultServFreqList;
  };

  struct RrcConnectionRequest {
    uint64_t ueIdentity;
  };

  struct RrcConnectionSetup {
    uint8_t rrcTransactionIdentifier;
    RadioResourceConfigDedicated radioResourceConfigDedicated;
  };

  struct RrcConnectionSetupCompleted {
    uint8_t rrcTransactionIdentifier;
  };

  struct CellIdentification {
    uint32_t physCellId;
    uint32_t dlCarrierFreq;
  };

  struct AntennaInfoCommon {
    uint16_t antennaPortsCount;
  };

  struct UlPowerControlCommonSCell {
    uint16_t alpha;
  };

  struct PrachConfigSCell {
    uint16_t index;
  };

  struct NonUlConfiguration {
    uint16_t dlBandwidth;
    AntennaInfoCommon antennaInfoCommon;
    PdschConfigCommon pdschConfigCommon;
  };

  struct UlConfiguration {
    FreqInfo ulFreqInfo;
    UlPowerControlCommonSCell ulPowerControlCommonSCell;
    SoundingRsUlConfigCommon soundingRsUlConfigCommon;
    PrachConfigSCell prachConfigSCell;
  };

  struct AntennaInfoUl {
    uint8_t transmissionMode;
  };

  struct PuschConfigDedicatedSCell {
    uint16_t nPuschIdentity;
  };

  struct UlPowerControlDedicatedSCell {
    uint16_t pSrsOffset;
  };

  struct PhysicalConfigDedicatedSCell {
    bool haveNonUlConfiguration;
    bool haveAntennaInfoDedicated;
    AntennaInfoDedicated antennaInfo;
    bool crossCarrierSchedulingConfig;
    bool havePdschConfigDedicated;
    PdschConfigDedicated pdschConfigDedicated;

    bool haveUlConfiguration;
    bool haveAntennaInfoUlDedicated;
    AntennaInfoDedicated antennaInfoUl;
    PuschConfigDedicatedSCell pushConfigDedicatedSCell;
    UlPowerControlDedicatedSCell ulPowerControlDedicatedSCell;
    bool haveSoundingRsUlConfigDedicated;
    SoundingRsUlConfigDedicated soundingRsUlConfigDedicated;
  };

  struct RadioResourceConfigCommonSCell {
    bool haveNonUlConfiguration;
    NonUlConfiguration nonUlConfiguration;
    bool haveUlConfiguration;
    UlConfiguration ulConfiguration;
  };

  struct RadioResourceConfigDedicatedSCell {
    PhysicalConfigDedicatedSCell physicalConfigDedicatedSCell;
  };

  struct SCellToAddMod {
    uint32_t sCellIndex;
    CellIdentification cellIdentification;
    RadioResourceConfigCommonSCell radioResourceConfigCommonSCell;
    bool haveRadioResourceConfigDedicatedSCell;
    RadioResourceConfigDedicatedSCell radioResourceConfigDedicatedSCell;
  };

  struct NonCriticalExtensionConfiguration {
    std::list<SCellToAddMod> sCellToAddModList;
    std::list<uint8_t> sCellToReleaseList;
  };

  struct RrcConnectionReconfiguration {
    uint8_t rrcTransactionIdentifier;
    bool haveMeasConfig;
    MeasConfig measConfig;
    bool haveMobilityControlInfo;
    MobilityControlInfo mobilityControlInfo;
    bool haveRadioResourceConfigDedicated;
    RadioResourceConfigDedicated radioResourceConfigDedicated;
    bool haveNonCriticalExtension;
    NonCriticalExtensionConfiguration nonCriticalExtension;
  };

  struct RrcConnectionReconfigurationCompleted {
    uint8_t rrcTransactionIdentifier;
  };

  struct RrcConnectionReestablishmentRequest {
    ReestabUeIdentity ueIdentity;
    ReestablishmentCause reestablishmentCause;
  };

  struct RrcConnectionReestablishment {
    uint8_t rrcTransactionIdentifier;
    RadioResourceConfigDedicated radioResourceConfigDedicated;
  };

  struct RrcConnectionReestablishmentComplete {
    uint8_t rrcTransactionIdentifier;
  };

  struct RrcConnectionReestablishmentReject {};

  struct RrcConnectionRelease {
    uint8_t rrcTransactionIdentifier;
  };

  struct RrcConnectionReject {
    uint8_t waitTime;
  };

  struct HandoverPreparationInfo {
    AsConfig asConfig;
  };

  struct MeasurementReport {
    MeasResults measResults;
  };
};

class LteUeRrcSapUser : public LteRrcSap {
public:
  struct SetupParameters {
    LteRlcSapProvider *srb0SapProvider;
    LtePdcpSapProvider *srb1SapProvider;
  };

  virtual void Setup(SetupParameters params) = 0;

  virtual void SendRrcConnectionRequest(RrcConnectionRequest msg) = 0;

  virtual void
  SendRrcConnectionSetupCompleted(RrcConnectionSetupCompleted msg) = 0;

  virtual void SendRrcConnectionReconfigurationCompleted(
      RrcConnectionReconfigurationCompleted msg) = 0;

  virtual void SendRrcConnectionReestablishmentRequest(
      RrcConnectionReestablishmentRequest msg) = 0;

  virtual void SendRrcConnectionReestablishmentComplete(
      RrcConnectionReestablishmentComplete msg) = 0;

  virtual void SendMeasurementReport(MeasurementReport msg) = 0;

  virtual void SendIdealUeContextRemoveRequest(uint16_t rnti) = 0;
};

class LteUeRrcSapProvider : public LteRrcSap {
public:
  struct CompleteSetupParameters {
    LteRlcSapUser *srb0SapUser;
    LtePdcpSapUser *srb1SapUser;
  };

  virtual void CompleteSetup(CompleteSetupParameters params) = 0;

  virtual void RecvSystemInformation(SystemInformation msg) = 0;

  virtual void RecvRrcConnectionSetup(RrcConnectionSetup msg) = 0;

  virtual void
  RecvRrcConnectionReconfiguration(RrcConnectionReconfiguration msg) = 0;

  virtual void
  RecvRrcConnectionReestablishment(RrcConnectionReestablishment msg) = 0;

  virtual void RecvRrcConnectionReestablishmentReject(
      RrcConnectionReestablishmentReject msg) = 0;

  virtual void RecvRrcConnectionRelease(RrcConnectionRelease msg) = 0;

  virtual void RecvRrcConnectionReject(RrcConnectionReject msg) = 0;
};

class LteEnbRrcSapUser : public LteRrcSap {
public:
  struct SetupUeParameters {
    LteRlcSapProvider *srb0SapProvider;
    LtePdcpSapProvider *srb1SapProvider;
  };

  virtual void SetupUe(uint16_t rnti, SetupUeParameters params) = 0;
  virtual void RemoveUe(uint16_t rnti) = 0;

  virtual void SendSystemInformation(uint16_t cellId,
                                     SystemInformation msg) = 0;

  virtual void SendRrcConnectionSetup(uint16_t rnti,
                                      RrcConnectionSetup msg) = 0;

  virtual void
  SendRrcConnectionReconfiguration(uint16_t rnti,
                                   RrcConnectionReconfiguration msg) = 0;

  virtual void
  SendRrcConnectionReestablishment(uint16_t rnti,
                                   RrcConnectionReestablishment msg) = 0;

  virtual void SendRrcConnectionReestablishmentReject(
      uint16_t rnti, RrcConnectionReestablishmentReject msg) = 0;

  virtual void SendRrcConnectionRelease(uint16_t rnti,
                                        RrcConnectionRelease msg) = 0;

  virtual void SendRrcConnectionReject(uint16_t rnti,
                                       RrcConnectionReject msg) = 0;

  virtual Ptr<Packet>
  EncodeHandoverPreparationInformation(HandoverPreparationInfo msg) = 0;
  virtual HandoverPreparationInfo
  DecodeHandoverPreparationInformation(Ptr<Packet> p) = 0;
  virtual Ptr<Packet>
  EncodeHandoverCommand(RrcConnectionReconfiguration msg) = 0;
  virtual RrcConnectionReconfiguration DecodeHandoverCommand(Ptr<Packet> p) = 0;
};

class LteEnbRrcSapProvider : public LteRrcSap {
public:
  struct CompleteSetupUeParameters {
    LteRlcSapUser *srb0SapUser;
    LtePdcpSapUser *srb1SapUser;
  };

  virtual void CompleteSetupUe(uint16_t rnti,
                               CompleteSetupUeParameters params) = 0;

  virtual void RecvRrcConnectionRequest(uint16_t rnti,
                                        RrcConnectionRequest msg) = 0;

  virtual void
  RecvRrcConnectionSetupCompleted(uint16_t rnti,
                                  RrcConnectionSetupCompleted msg) = 0;

  virtual void RecvRrcConnectionReconfigurationCompleted(
      uint16_t rnti, RrcConnectionReconfigurationCompleted msg) = 0;

  virtual void RecvRrcConnectionReestablishmentRequest(
      uint16_t rnti, RrcConnectionReestablishmentRequest msg) = 0;

  virtual void RecvRrcConnectionReestablishmentComplete(
      uint16_t rnti, RrcConnectionReestablishmentComplete msg) = 0;

  virtual void RecvMeasurementReport(uint16_t rnti, MeasurementReport msg) = 0;

  virtual void RecvIdealUeContextRemoveRequest(uint16_t rnti) = 0;
};

template <class C> class MemberLteUeRrcSapUser : public LteUeRrcSapUser {
public:
  MemberLteUeRrcSapUser(C *owner);

  MemberLteUeRrcSapUser() = delete;

  void Setup(SetupParameters params) override;
  void SendRrcConnectionRequest(RrcConnectionRequest msg) override;
  void
  SendRrcConnectionSetupCompleted(RrcConnectionSetupCompleted msg) override;
  void SendRrcConnectionReconfigurationCompleted(
      RrcConnectionReconfigurationCompleted msg) override;
  void SendRrcConnectionReestablishmentRequest(
      RrcConnectionReestablishmentRequest msg) override;
  void SendRrcConnectionReestablishmentComplete(
      RrcConnectionReestablishmentComplete msg) override;
  void SendMeasurementReport(MeasurementReport msg) override;
  void SendIdealUeContextRemoveRequest(uint16_t rnti) override;

private:
  C *m_owner;
};

template <class C>
MemberLteUeRrcSapUser<C>::MemberLteUeRrcSapUser(C *owner) : m_owner(owner) {}

template <class C>
void MemberLteUeRrcSapUser<C>::Setup(SetupParameters params) {
  m_owner->DoSetup(params);
}

template <class C>
void MemberLteUeRrcSapUser<C>::SendRrcConnectionRequest(
    RrcConnectionRequest msg) {
  m_owner->DoSendRrcConnectionRequest(msg);
}

template <class C>
void MemberLteUeRrcSapUser<C>::SendRrcConnectionSetupCompleted(
    RrcConnectionSetupCompleted msg) {
  m_owner->DoSendRrcConnectionSetupCompleted(msg);
}

template <class C>
void MemberLteUeRrcSapUser<C>::SendRrcConnectionReconfigurationCompleted(
    RrcConnectionReconfigurationCompleted msg) {
  m_owner->DoSendRrcConnectionReconfigurationCompleted(msg);
}

template <class C>
void MemberLteUeRrcSapUser<C>::SendRrcConnectionReestablishmentRequest(
    RrcConnectionReestablishmentRequest msg) {
  m_owner->DoSendRrcConnectionReestablishmentRequest(msg);
}

template <class C>
void MemberLteUeRrcSapUser<C>::SendRrcConnectionReestablishmentComplete(
    RrcConnectionReestablishmentComplete msg) {
  m_owner->DoSendRrcConnectionReestablishmentComplete(msg);
}

template <class C>
void MemberLteUeRrcSapUser<C>::SendMeasurementReport(MeasurementReport msg) {
  m_owner->DoSendMeasurementReport(msg);
}

template <class C>
void MemberLteUeRrcSapUser<C>::SendIdealUeContextRemoveRequest(uint16_t rnti) {
  m_owner->DoSendIdealUeContextRemoveRequest(rnti);
}

template <class C>
class MemberLteUeRrcSapProvider : public LteUeRrcSapProvider {
public:
  MemberLteUeRrcSapProvider(C *owner);

  MemberLteUeRrcSapProvider() = delete;

  void CompleteSetup(CompleteSetupParameters params) override;
  void RecvSystemInformation(SystemInformation msg) override;
  void RecvRrcConnectionSetup(RrcConnectionSetup msg) override;
  void
  RecvRrcConnectionReconfiguration(RrcConnectionReconfiguration msg) override;
  void
  RecvRrcConnectionReestablishment(RrcConnectionReestablishment msg) override;
  void RecvRrcConnectionReestablishmentReject(
      RrcConnectionReestablishmentReject msg) override;
  void RecvRrcConnectionRelease(RrcConnectionRelease msg) override;
  void RecvRrcConnectionReject(RrcConnectionReject msg) override;

private:
  C *m_owner;
};

template <class C>
MemberLteUeRrcSapProvider<C>::MemberLteUeRrcSapProvider(C *owner)
    : m_owner(owner) {}

template <class C>
void MemberLteUeRrcSapProvider<C>::CompleteSetup(
    CompleteSetupParameters params) {
  m_owner->DoCompleteSetup(params);
}

template <class C>
void MemberLteUeRrcSapProvider<C>::RecvSystemInformation(
    SystemInformation msg) {
  Simulator::ScheduleNow(&C::DoRecvSystemInformation, m_owner, msg);
}

template <class C>
void MemberLteUeRrcSapProvider<C>::RecvRrcConnectionSetup(
    RrcConnectionSetup msg) {
  Simulator::ScheduleNow(&C::DoRecvRrcConnectionSetup, m_owner, msg);
}

template <class C>
void MemberLteUeRrcSapProvider<C>::RecvRrcConnectionReconfiguration(
    RrcConnectionReconfiguration msg) {
  Simulator::ScheduleNow(&C::DoRecvRrcConnectionReconfiguration, m_owner, msg);
}

template <class C>
void MemberLteUeRrcSapProvider<C>::RecvRrcConnectionReestablishment(
    RrcConnectionReestablishment msg) {
  Simulator::ScheduleNow(&C::DoRecvRrcConnectionReestablishment, m_owner, msg);
}

template <class C>
void MemberLteUeRrcSapProvider<C>::RecvRrcConnectionReestablishmentReject(
    RrcConnectionReestablishmentReject msg) {
  Simulator::ScheduleNow(&C::DoRecvRrcConnectionReestablishmentReject, m_owner,
                         msg);
}

template <class C>
void MemberLteUeRrcSapProvider<C>::RecvRrcConnectionRelease(
    RrcConnectionRelease msg) {
  Simulator::ScheduleNow(&C::DoRecvRrcConnectionRelease, m_owner, msg);
}

template <class C>
void MemberLteUeRrcSapProvider<C>::RecvRrcConnectionReject(
    RrcConnectionReject msg) {
  Simulator::ScheduleNow(&C::DoRecvRrcConnectionReject, m_owner, msg);
}

template <class C> class MemberLteEnbRrcSapUser : public LteEnbRrcSapUser {
public:
  MemberLteEnbRrcSapUser(C *owner);

  MemberLteEnbRrcSapUser() = delete;

  void SetupUe(uint16_t rnti, SetupUeParameters params) override;
  void RemoveUe(uint16_t rnti) override;
  void SendSystemInformation(uint16_t cellId, SystemInformation msg) override;
  void SendRrcConnectionSetup(uint16_t rnti, RrcConnectionSetup msg) override;
  void
  SendRrcConnectionReconfiguration(uint16_t rnti,
                                   RrcConnectionReconfiguration msg) override;
  void
  SendRrcConnectionReestablishment(uint16_t rnti,
                                   RrcConnectionReestablishment msg) override;
  void SendRrcConnectionReestablishmentReject(
      uint16_t rnti, RrcConnectionReestablishmentReject msg) override;
  void SendRrcConnectionRelease(uint16_t rnti,
                                RrcConnectionRelease msg) override;
  void SendRrcConnectionReject(uint16_t rnti, RrcConnectionReject msg) override;
  Ptr<Packet>
  EncodeHandoverPreparationInformation(HandoverPreparationInfo msg) override;
  HandoverPreparationInfo
  DecodeHandoverPreparationInformation(Ptr<Packet> p) override;
  Ptr<Packet> EncodeHandoverCommand(RrcConnectionReconfiguration msg) override;
  RrcConnectionReconfiguration DecodeHandoverCommand(Ptr<Packet> p) override;

private:
  C *m_owner;
};

template <class C>
MemberLteEnbRrcSapUser<C>::MemberLteEnbRrcSapUser(C *owner) : m_owner(owner) {}

template <class C>
void MemberLteEnbRrcSapUser<C>::SetupUe(uint16_t rnti,
                                        SetupUeParameters params) {
  m_owner->DoSetupUe(rnti, params);
}

template <class C> void MemberLteEnbRrcSapUser<C>::RemoveUe(uint16_t rnti) {
  m_owner->DoRemoveUe(rnti);
}

template <class C>
void MemberLteEnbRrcSapUser<C>::SendSystemInformation(uint16_t cellId,
                                                      SystemInformation msg) {
  m_owner->DoSendSystemInformation(cellId, msg);
}

template <class C>
void MemberLteEnbRrcSapUser<C>::SendRrcConnectionSetup(uint16_t rnti,
                                                       RrcConnectionSetup msg) {
  m_owner->DoSendRrcConnectionSetup(rnti, msg);
}

template <class C>
void MemberLteEnbRrcSapUser<C>::SendRrcConnectionReconfiguration(
    uint16_t rnti, RrcConnectionReconfiguration msg) {
  m_owner->DoSendRrcConnectionReconfiguration(rnti, msg);
}

template <class C>
void MemberLteEnbRrcSapUser<C>::SendRrcConnectionReestablishment(
    uint16_t rnti, RrcConnectionReestablishment msg) {
  m_owner->DoSendRrcConnectionReestablishment(rnti, msg);
}

template <class C>
void MemberLteEnbRrcSapUser<C>::SendRrcConnectionReestablishmentReject(
    uint16_t rnti, RrcConnectionReestablishmentReject msg) {
  m_owner->DoSendRrcConnectionReestablishmentReject(rnti, msg);
}

template <class C>
void MemberLteEnbRrcSapUser<C>::SendRrcConnectionRelease(
    uint16_t rnti, RrcConnectionRelease msg) {
  m_owner->DoSendRrcConnectionRelease(rnti, msg);
}

template <class C>
void MemberLteEnbRrcSapUser<C>::SendRrcConnectionReject(
    uint16_t rnti, RrcConnectionReject msg) {
  m_owner->DoSendRrcConnectionReject(rnti, msg);
}

template <class C>
Ptr<Packet> MemberLteEnbRrcSapUser<C>::EncodeHandoverPreparationInformation(
    HandoverPreparationInfo msg) {
  return m_owner->DoEncodeHandoverPreparationInformation(msg);
}

template <class C>
LteRrcSap::HandoverPreparationInfo
MemberLteEnbRrcSapUser<C>::DecodeHandoverPreparationInformation(Ptr<Packet> p) {
  return m_owner->DoDecodeHandoverPreparationInformation(p);
}

template <class C>
Ptr<Packet> MemberLteEnbRrcSapUser<C>::EncodeHandoverCommand(
    RrcConnectionReconfiguration msg) {
  return m_owner->DoEncodeHandoverCommand(msg);
}

template <class C>
LteRrcSap::RrcConnectionReconfiguration
MemberLteEnbRrcSapUser<C>::DecodeHandoverCommand(Ptr<Packet> p) {
  return m_owner->DoDecodeHandoverCommand(p);
}

template <class C>
class MemberLteEnbRrcSapProvider : public LteEnbRrcSapProvider {
public:
  MemberLteEnbRrcSapProvider(C *owner);

  MemberLteEnbRrcSapProvider() = delete;

  void CompleteSetupUe(uint16_t rnti,
                       CompleteSetupUeParameters params) override;
  void RecvRrcConnectionRequest(uint16_t rnti,
                                RrcConnectionRequest msg) override;
  void
  RecvRrcConnectionSetupCompleted(uint16_t rnti,
                                  RrcConnectionSetupCompleted msg) override;
  void RecvRrcConnectionReconfigurationCompleted(
      uint16_t rnti, RrcConnectionReconfigurationCompleted msg) override;
  void RecvRrcConnectionReestablishmentRequest(
      uint16_t rnti, RrcConnectionReestablishmentRequest msg) override;
  void RecvRrcConnectionReestablishmentComplete(
      uint16_t rnti, RrcConnectionReestablishmentComplete msg) override;
  void RecvMeasurementReport(uint16_t rnti, MeasurementReport msg) override;
  void RecvIdealUeContextRemoveRequest(uint16_t rnti) override;

private:
  C *m_owner;
};

template <class C>
MemberLteEnbRrcSapProvider<C>::MemberLteEnbRrcSapProvider(C *owner)
    : m_owner(owner) {}

template <class C>
void MemberLteEnbRrcSapProvider<C>::CompleteSetupUe(
    uint16_t rnti, CompleteSetupUeParameters params) {
  m_owner->DoCompleteSetupUe(rnti, params);
}

template <class C>
void MemberLteEnbRrcSapProvider<C>::RecvRrcConnectionRequest(
    uint16_t rnti, RrcConnectionRequest msg) {
  Simulator::ScheduleNow(&C::DoRecvRrcConnectionRequest, m_owner, rnti, msg);
}

template <class C>
void MemberLteEnbRrcSapProvider<C>::RecvRrcConnectionSetupCompleted(
    uint16_t rnti, RrcConnectionSetupCompleted msg) {
  Simulator::ScheduleNow(&C::DoRecvRrcConnectionSetupCompleted, m_owner, rnti,
                         msg);
}

template <class C>
void MemberLteEnbRrcSapProvider<C>::RecvRrcConnectionReconfigurationCompleted(
    uint16_t rnti, RrcConnectionReconfigurationCompleted msg) {
  Simulator::ScheduleNow(&C::DoRecvRrcConnectionReconfigurationCompleted,
                         m_owner, rnti, msg);
}

template <class C>
void MemberLteEnbRrcSapProvider<C>::RecvRrcConnectionReestablishmentRequest(
    uint16_t rnti, RrcConnectionReestablishmentRequest msg) {
  Simulator::ScheduleNow(&C::DoRecvRrcConnectionReestablishmentRequest, m_owner,
                         rnti, msg);
}

template <class C>
void MemberLteEnbRrcSapProvider<C>::RecvRrcConnectionReestablishmentComplete(
    uint16_t rnti, RrcConnectionReestablishmentComplete msg) {
  Simulator::ScheduleNow(&C::DoRecvRrcConnectionReestablishmentComplete,
                         m_owner, rnti, msg);
}

template <class C>
void MemberLteEnbRrcSapProvider<C>::RecvMeasurementReport(
    uint16_t rnti, MeasurementReport msg) {
  Simulator::ScheduleNow(&C::DoRecvMeasurementReport, m_owner, rnti, msg);
}

template <class C>
void MemberLteEnbRrcSapProvider<C>::RecvIdealUeContextRemoveRequest(
    uint16_t rnti) {
  Simulator::ScheduleNow(&C::DoRecvIdealUeContextRemoveRequest, m_owner, rnti);
}

} // namespace ns3

#endif
