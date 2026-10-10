
#ifndef FF_MAC_CSCHED_SAP_H
#define FF_MAC_CSCHED_SAP_H

#include "ff-mac-common.h"

#include <stdint.h>
#include <vector>

namespace ns3 {

class FfMacCschedSapProvider {
public:
  virtual ~FfMacCschedSapProvider();

  struct CschedCellConfigReqParameters {
    uint8_t m_puschHoppingOffset;

    enum HoppingMode_e { inter, interintra } m_hoppingMode;

    uint8_t m_nSb;

    enum PhichResource_e {
      PHICH_R_ONE_SIXTH,
      PHICH_R_HALF,
      PHICH_R_ONE,
      PHICH_R_TWO
    } m_phichResource;

    NormalExtended_e m_phichDuration;

    uint8_t m_initialNrOfPdcchOfdmSymbols;

    SiConfiguration_s m_siConfiguration;

    uint16_t m_ulBandwidth;
    uint16_t m_dlBandwidth;

    NormalExtended_e m_ulCyclicPrefixLength;
    NormalExtended_e m_dlCyclicPrefixLength;

    uint8_t m_antennaPortsCount;

    enum DuplexMode_e { DM_TDD, DM_FDD } m_duplexMode;

    uint8_t m_subframeAssignment;
    uint8_t m_specialSubframePatterns;
    std::vector<uint8_t> m_mbsfnSubframeConfigRfPeriod;
    std::vector<uint8_t> m_mbsfnSubframeConfigRfOffset;
    std::vector<uint8_t> m_mbsfnSubframeConfigSfAllocation;
    uint8_t m_prachConfigurationIndex;
    uint8_t m_prachFreqOffset;
    uint8_t m_raResponseWindowSize;
    uint8_t m_macContentionResolutionTimer;
    uint8_t m_maxHarqMsg3Tx;
    uint16_t m_n1PucchAn;
    uint8_t m_deltaPucchShift;
    uint8_t m_nrbCqi;
    uint8_t m_ncsAn;
    uint8_t m_srsSubframeConfiguration;
    uint8_t m_srsSubframeOffset;
    uint8_t m_srsBandwidthConfiguration;
    bool m_srsMaxUpPts;

    enum Enable64Qam_e { MOD_16QAM, MOD_64QAM } m_enable64Qam;

    std::vector<VendorSpecificListElement_s> m_vendorSpecificList;
  };

  struct CschedUeConfigReqParameters {
    uint16_t m_rnti;
    bool m_reconfigureFlag;
    bool m_drxConfigPresent;
    DrxConfig_s m_drxConfig;
    uint16_t m_timeAlignmentTimer;

    enum MeasGapConfigPattern_e {
      MGP_GP1,
      MGP_GP2,
      OFF
    } m_measGapConfigPattern;

    uint8_t m_measGapConfigSubframeOffset;
    bool m_spsConfigPresent;
    SpsConfig_s m_spsConfig;
    bool m_srConfigPresent;
    SrConfig_s m_srConfig;
    bool m_cqiConfigPresent;
    CqiConfig_s m_cqiConfig;
    uint8_t m_transmissionMode;
    uint64_t m_ueAggregatedMaximumBitrateUl;
    uint64_t m_ueAggregatedMaximumBitrateDl;
    UeCapabilities_s m_ueCapabilities;

    enum OpenClosedLoop_e {
      noneloop,
      openloop,
      closedloop
    } m_ueTransmitAntennaSelection;

    bool m_ttiBundling;
    uint8_t m_maxHarqTx;
    uint8_t m_betaOffsetAckIndex;
    uint8_t m_betaOffsetRiIndex;
    uint8_t m_betaOffsetCqiIndex;
    bool m_ackNackSrsSimultaneousTransmission;
    bool m_simultaneousAckNackAndCqi;

    enum RepMode_e {
      rm12,
      rm20,
      rm22,
      rm30,
      rm31,
      nonemode
    } m_aperiodicCqiRepMode;

    enum FeedbackMode_e { bundling, multiplexing } m_tddAckNackFeedbackMode;

    uint8_t m_ackNackRepetitionFactor;

    std::vector<VendorSpecificListElement_s> m_vendorSpecificList;
  };

  struct CschedLcConfigReqParameters {
    uint16_t m_rnti;
    bool m_reconfigureFlag;

    std::vector<LogicalChannelConfigListElement_s> m_logicalChannelConfigList;

    std::vector<VendorSpecificListElement_s> m_vendorSpecificList;
  };

  struct CschedLcReleaseReqParameters {
    uint16_t m_rnti;

    std::vector<uint8_t> m_logicalChannelIdentity;

    std::vector<VendorSpecificListElement_s> m_vendorSpecificList;
  };

  struct CschedUeReleaseReqParameters {
    uint16_t m_rnti;

    std::vector<VendorSpecificListElement_s> m_vendorSpecificList;
  };

  virtual void
  CschedCellConfigReq(const CschedCellConfigReqParameters &params) = 0;

  virtual void CschedUeConfigReq(const CschedUeConfigReqParameters &params) = 0;

  virtual void CschedLcConfigReq(const CschedLcConfigReqParameters &params) = 0;

  virtual void
  CschedLcReleaseReq(const CschedLcReleaseReqParameters &params) = 0;

  virtual void
  CschedUeReleaseReq(const CschedUeReleaseReqParameters &params) = 0;

private:
};

class FfMacCschedSapUser {
public:
  virtual ~FfMacCschedSapUser();

  struct CschedCellConfigCnfParameters {
    Result_e m_result;

    std::vector<VendorSpecificListElement_s> m_vendorSpecificList;
  };

  struct CschedUeConfigCnfParameters {
    uint16_t m_rnti;
    Result_e m_result;

    std::vector<VendorSpecificListElement_s> m_vendorSpecificList;
  };

  struct CschedLcConfigCnfParameters {
    uint16_t m_rnti;
    Result_e m_result;

    std::vector<uint8_t> m_logicalChannelIdentity;

    std::vector<VendorSpecificListElement_s> m_vendorSpecificList;
  };

  struct CschedLcReleaseCnfParameters {
    uint16_t m_rnti;
    Result_e m_result;

    std::vector<uint8_t> m_logicalChannelIdentity;

    std::vector<VendorSpecificListElement_s> m_vendorSpecificList;
  };

  struct CschedUeReleaseCnfParameters {
    uint16_t m_rnti;
    Result_e m_result;

    std::vector<VendorSpecificListElement_s> m_vendorSpecificList;
  };

  struct CschedUeConfigUpdateIndParameters {
    uint16_t m_rnti;
    uint8_t m_transmissionMode;
    bool m_spsConfigPresent;
    SpsConfig_s m_spsConfig;
    bool m_srConfigPresent;
    SrConfig_s m_srConfig;
    bool m_cqiConfigPresent;
    CqiConfig_s m_cqiConfig;

    std::vector<VendorSpecificListElement_s> m_vendorSpecificList;
  };

  struct CschedCellConfigUpdateIndParameters {
    uint8_t m_prbUtilizationDl;
    uint8_t m_prbUtilizationUl;

    std::vector<VendorSpecificListElement_s> m_vendorSpecificList;
  };

  virtual void
  CschedCellConfigCnf(const CschedCellConfigCnfParameters &params) = 0;

  virtual void CschedUeConfigCnf(const CschedUeConfigCnfParameters &params) = 0;

  virtual void CschedLcConfigCnf(const CschedLcConfigCnfParameters &params) = 0;

  virtual void
  CschedLcReleaseCnf(const CschedLcReleaseCnfParameters &params) = 0;

  virtual void
  CschedUeReleaseCnf(const CschedUeReleaseCnfParameters &params) = 0;

  virtual void
  CschedUeConfigUpdateInd(const CschedUeConfigUpdateIndParameters &params) = 0;

  virtual void CschedCellConfigUpdateInd(
      const CschedCellConfigUpdateIndParameters &params) = 0;

private:
};

template <class C>
class MemberCschedSapProvider : public FfMacCschedSapProvider {
public:
  MemberCschedSapProvider(C *scheduler);

  MemberCschedSapProvider() = delete;

  void
  CschedCellConfigReq(const CschedCellConfigReqParameters &params) override;
  void CschedUeConfigReq(const CschedUeConfigReqParameters &params) override;
  void CschedLcConfigReq(const CschedLcConfigReqParameters &params) override;
  void CschedLcReleaseReq(const CschedLcReleaseReqParameters &params) override;
  void CschedUeReleaseReq(const CschedUeReleaseReqParameters &params) override;

private:
  C *m_scheduler;
};

template <class C>
MemberCschedSapProvider<C>::MemberCschedSapProvider(C *scheduler)
    : m_scheduler(scheduler) {}

template <class C>
void MemberCschedSapProvider<C>::CschedCellConfigReq(
    const CschedCellConfigReqParameters &params) {
  m_scheduler->DoCschedCellConfigReq(params);
}

template <class C>
void MemberCschedSapProvider<C>::CschedUeConfigReq(
    const CschedUeConfigReqParameters &params) {
  m_scheduler->DoCschedUeConfigReq(params);
}

template <class C>
void MemberCschedSapProvider<C>::CschedLcConfigReq(
    const CschedLcConfigReqParameters &params) {
  m_scheduler->DoCschedLcConfigReq(params);
}

template <class C>
void MemberCschedSapProvider<C>::CschedLcReleaseReq(
    const CschedLcReleaseReqParameters &params) {
  m_scheduler->DoCschedLcReleaseReq(params);
}

template <class C>
void MemberCschedSapProvider<C>::CschedUeReleaseReq(
    const CschedUeReleaseReqParameters &params) {
  m_scheduler->DoCschedUeReleaseReq(params);
}

} // namespace ns3

#endif
