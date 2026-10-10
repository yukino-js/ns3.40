
#ifndef FF_MAC_COMMON_H
#define FF_MAC_COMMON_H

#include <ns3/ptr.h>
#include <ns3/simple-ref-count.h>

#include <vector>

#define MAX_SCHED_CFG_LIST 10
#define MAX_LC_LIST 10

#define MAX_RACH_LIST 30
#define MAX_DL_INFO_LIST 30
#define MAX_BUILD_DATA_LIST 30
#define MAX_BUILD_RAR_LIST 10
#define MAX_BUILD_BC_LIST 3
#define MAX_UL_INFO_LIST 30
#define MAX_DCI_LIST 30
#define MAX_PHICH_LIST 30
#define MAX_TB_LIST 2
#define MAX_RLC_PDU_LIST 30
#define MAX_NR_LCG 4
#define MAX_MBSFN_CONFIG 5
#define MAX_SI_MSG_LIST 32
#define MAX_SI_MSG_SIZE 65535

#define MAX_CQI_LIST 30
#define MAX_UE_SELECTED_SB 6
#define MAX_HL_SB 25
#define MAX_SINR_RB_LIST 100
#define MAX_SR_LIST 30
#define MAX_MAC_CE_LIST 30

namespace ns3 {

enum Result_e { SUCCESS, FAILURE };

enum SetupRelease_e { setup, release };

enum CeBitmap_e { TA, DRX, CR };

enum NormalExtended_e { normal, extended };

struct DlDciListElement_s {
  uint16_t m_rnti{UINT16_MAX};
  uint32_t m_rbBitmap{UINT8_MAX};
  uint8_t m_rbShift{UINT8_MAX};
  uint8_t m_resAlloc{UINT8_MAX};
  std::vector<uint16_t> m_tbsSize;
  std::vector<uint8_t> m_mcs;
  std::vector<uint8_t> m_ndi;
  std::vector<uint8_t> m_rv;
  uint8_t m_cceIndex{UINT8_MAX};
  uint8_t m_aggrLevel{UINT8_MAX};
  uint8_t m_precodingInfo{UINT8_MAX};

  enum Format_e {
    ONE,
    ONE_A,
    ONE_B,
    ONE_C,
    ONE_D,
    TWO,
    TWO_A,
    TWO_B,
    NotValid_Dci_Format
  } m_format{NotValid_Dci_Format};

  uint8_t m_tpc{UINT8_MAX};
  uint8_t m_harqProcess{UINT8_MAX};
  uint8_t m_dai{UINT8_MAX};

  enum VrbFormat_e {
    VRB_DISTRIBUTED,
    VRB_LOCALIZED,
    NotValid_VRB_Format
  } m_vrbFormat{NotValid_VRB_Format};

  bool m_tbSwap{false};
  bool m_spsRelease{false};
  bool m_pdcchOrder{false};
  uint8_t m_preambleIndex{UINT8_MAX};
  uint8_t m_prachMaskIndex{UINT8_MAX};

  enum Ngap_e { GAP1, GAP2, NotValid_Ngap } m_nGap{NotValid_Ngap};

  uint8_t m_tbsIdx{UINT8_MAX};
  uint8_t m_dlPowerOffset{UINT8_MAX};
  uint8_t m_pdcchPowerOffset{UINT8_MAX};
};

struct UlDciListElement_s {
  uint16_t m_rnti{UINT16_MAX};
  uint8_t m_rbStart{UINT8_MAX};
  uint8_t m_rbLen{UINT8_MAX};
  uint16_t m_tbSize{UINT16_MAX};
  uint8_t m_mcs{UINT8_MAX};
  uint8_t m_ndi{UINT8_MAX};
  uint8_t m_cceIndex{UINT8_MAX};
  uint8_t m_aggrLevel{UINT8_MAX};
  uint8_t m_ueTxAntennaSelection{UINT8_MAX};
  bool m_hopping{false};
  uint8_t m_n2Dmrs{UINT8_MAX};
  int8_t m_tpc{INT8_MIN};
  bool m_cqiRequest{false};
  uint8_t m_ulIndex{UINT8_MAX};
  uint8_t m_dai{UINT8_MAX};
  uint8_t m_freqHopping{UINT8_MAX};
  int8_t m_pdcchPowerOffset{INT8_MIN};
};

struct VendorSpecificValue : public SimpleRefCount<VendorSpecificValue> {
  virtual ~VendorSpecificValue();
};

struct VendorSpecificListElement_s {
  uint32_t m_type{UINT32_MAX};
  uint32_t m_length{UINT32_MAX};
  Ptr<VendorSpecificValue> m_value;
};

struct LogicalChannelConfigListElement_s {
  uint8_t m_logicalChannelIdentity{UINT8_MAX};
  uint8_t m_logicalChannelGroup{UINT8_MAX};

  enum Direction_e { DIR_UL, DIR_DL, DIR_BOTH, NotValid } m_direction{NotValid};

  enum QosBearerType_e {
    QBT_NON_GBR,
    QBT_GBR,
    QBT_DGBR,
    NotValid_QosBearerType
  } m_qosBearerType{NotValid_QosBearerType};

  uint8_t m_qci{UINT8_MAX};
  uint64_t m_eRabMaximulBitrateUl{UINT64_MAX};
  uint64_t m_eRabMaximulBitrateDl{UINT64_MAX};
  uint64_t m_eRabGuaranteedBitrateUl{UINT64_MAX};
  uint64_t m_eRabGuaranteedBitrateDl{UINT64_MAX};
};

struct RachListElement_s {
  uint16_t m_rnti{UINT16_MAX};
  uint16_t m_estimatedSize{UINT16_MAX};
};

struct PhichListElement_s {
  uint16_t m_rnti{UINT16_MAX};

  enum Phich_e { ACK, NACK, NotValid } m_phich{NotValid};
};

struct RlcPduListElement_s {
  uint8_t m_logicalChannelIdentity{UINT8_MAX};
  uint16_t m_size{UINT16_MAX};
};

struct BuildDataListElement_s {
  uint16_t m_rnti{UINT16_MAX};
  struct DlDciListElement_s m_dci;
  std::vector<CeBitmap_e> m_ceBitmap;
  std::vector<std::vector<struct RlcPduListElement_s>> m_rlcPduList;
};

struct UlGrant_s {
  uint16_t m_rnti{UINT16_MAX};
  uint8_t m_rbStart{UINT8_MAX};
  uint8_t m_rbLen{UINT8_MAX};
  uint16_t m_tbSize{UINT16_MAX};
  uint8_t m_mcs{UINT8_MAX};
  bool m_hopping{false};
  int8_t m_tpc{INT8_MIN};
  bool m_cqiRequest{false};
  bool m_ulDelay{false};
};

struct BuildRarListElement_s {
  uint16_t m_rnti{UINT16_MAX};
  UlGrant_s m_grant;
  struct DlDciListElement_s m_dci;
};

struct BuildBroadcastListElement_s {
  enum Type_e { BCCH, PCCH, NotValid } m_type{NotValid};

  uint8_t m_index{UINT8_MAX};
  struct DlDciListElement_s m_dci;
};

struct UlInfoListElement_s {
  uint16_t m_rnti{UINT16_MAX};
  std::vector<uint16_t> m_ulReception;

  enum ReceptionStatus_e { Ok, NotOk, NotValid } m_receptionStatus{NotValid};

  uint8_t m_tpc{UINT8_MAX};
};

struct SrListElement_s {
  uint16_t m_rnti{UINT16_MAX};
};

struct MacCeValue_u {
  uint8_t m_phr{UINT8_MAX};
  uint8_t m_crnti{UINT8_MAX};
  std::vector<uint8_t> m_bufferStatus;
};

struct MacCeListElement_s {
  uint16_t m_rnti{UINT16_MAX};

  enum MacCeType_e { BSR, PHR, CRNTI, NotValid } m_macCeType{NotValid};
  struct MacCeValue_u m_macCeValue;
};

struct DrxConfig_s {
  uint8_t m_onDurationTimer{UINT8_MAX};
  uint16_t m_drxInactivityTimer{UINT16_MAX};
  uint16_t m_drxRetransmissionTimer{UINT16_MAX};
  uint16_t m_longDrxCycle{UINT16_MAX};
  uint16_t m_longDrxCycleStartOffset{UINT16_MAX};
  uint16_t m_shortDrxCycle{UINT16_MAX};
  uint8_t m_drxShortCycleTimer{UINT8_MAX};
};

struct SpsConfig_s {
  uint16_t m_semiPersistSchedIntervalUl{UINT16_MAX};
  uint16_t m_semiPersistSchedIntervalDl{UINT16_MAX};
  uint8_t m_numberOfConfSpsProcesses{UINT8_MAX};
  uint8_t m_n1PucchAnPersistentListSize{UINT8_MAX};
  std::vector<uint16_t> m_n1PucchAnPersistentList;
  uint8_t m_implicitReleaseAfter{UINT8_MAX};
};

struct SrConfig_s {
  enum SetupRelease_e m_action { setup };

  uint8_t m_schedInterval{UINT8_MAX};
  uint8_t m_dsrTransMax{UINT8_MAX};
};

struct CqiConfig_s {
  enum SetupRelease_e m_action { setup };

  uint16_t m_cqiSchedInterval{UINT16_MAX};
  uint8_t m_riSchedInterval{UINT8_MAX};
};

struct UeCapabilities_s {
  bool m_halfDuplex{false};
  bool m_intraSfHopping{false};
  bool m_type2Sb1{false};
  uint8_t m_ueCategory{UINT8_MAX};
  bool m_resAllocType1{false};
};

struct SiMessageListElement_s {
  uint16_t m_periodicity{UINT16_MAX};
  uint16_t m_length{UINT16_MAX};
};

struct SiConfiguration_s {
  uint16_t m_sfn{UINT16_MAX};
  uint16_t m_sib1Length{UINT16_MAX};
  uint8_t m_siWindowLength{UINT8_MAX};
  std::vector<struct SiMessageListElement_s> m_siMessageList;
};

struct DlInfoListElement_s {
  uint16_t m_rnti{UINT16_MAX};
  uint8_t m_harqProcessId{UINT8_MAX};

  enum HarqStatus_e { ACK, NACK, DTX };

  std::vector<HarqStatus_e> m_harqStatus;
};

struct BwPart_s {
  uint8_t m_bwPartIndex{UINT8_MAX};
  uint8_t m_sb{UINT8_MAX};
  uint8_t m_cqi{UINT8_MAX};
};

struct HigherLayerSelected_s {
  uint8_t m_sbPmi{UINT8_MAX};
  std::vector<uint8_t> m_sbCqi;
};

struct UeSelected_s {
  std::vector<uint8_t> m_sbList;
  uint8_t m_sbPmi{UINT8_MAX};
  std::vector<uint8_t> m_sbCqi;
};

struct SbMeasResult_s {
  struct UeSelected_s m_ueSelected;
  std::vector<struct HigherLayerSelected_s> m_higherLayerSelected;
  struct BwPart_s m_bwPart;
};

struct CqiListElement_s {
  uint16_t m_rnti{UINT16_MAX};
  uint8_t m_ri{UINT8_MAX};

  enum CqiType_e {
    P10,
    P11,
    P20,
    P21,
    A12,
    A22,
    A20,
    A30,
    A31,
    NotValid
  } m_cqiType{NotValid};

  std::vector<uint8_t> m_wbCqi;
  uint8_t m_wbPmi{UINT8_MAX};

  struct SbMeasResult_s m_sbMeasResult;
};

struct UlCqi_s {
  std::vector<uint16_t> m_sinr;

  enum Type_e {
    SRS,
    PUSCH,
    PUCCH_1,
    PUCCH_2,
    PRACH,
    NotValid
  } m_type{NotValid};
};

struct PagingInfoListElement_s {
  uint8_t m_pagingIndex{UINT8_MAX};
  uint16_t m_pagingMessageSize{UINT16_MAX};
  uint8_t m_pagingSubframe{UINT8_MAX};
};

} // namespace ns3

#endif
