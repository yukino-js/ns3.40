
#ifndef LTE_FFR_ENHANCED_ALGORITHM_H
#define LTE_FFR_ENHANCED_ALGORITHM_H

#include "lte-ffr-algorithm.h"
#include "lte-ffr-rrc-sap.h"
#include "lte-ffr-sap.h"
#include "lte-rrc-sap.h"

#include <map>

#define NO_SINR -5000

namespace ns3 {

class LteFfrEnhancedAlgorithm : public LteFfrAlgorithm {
public:
  LteFfrEnhancedAlgorithm();
  ~LteFfrEnhancedAlgorithm() override;

  static TypeId GetTypeId();

  void SetLteFfrSapUser(LteFfrSapUser *s) override;
  LteFfrSapProvider *GetLteFfrSapProvider() override;

  void SetLteFfrRrcSapUser(LteFfrRrcSapUser *s) override;
  LteFfrRrcSapProvider *GetLteFfrRrcSapProvider() override;

  friend class MemberLteFfrSapProvider<LteFfrEnhancedAlgorithm>;
  friend class MemberLteFfrRrcSapProvider<LteFfrEnhancedAlgorithm>;

protected:
  void DoInitialize() override;
  void DoDispose() override;

  void Reconfigure() override;

  std::vector<bool> DoGetAvailableDlRbg() override;
  bool DoIsDlRbgAvailableForUe(int i, uint16_t rnti) override;
  std::vector<bool> DoGetAvailableUlRbg() override;
  bool DoIsUlRbgAvailableForUe(int i, uint16_t rnti) override;
  void DoReportDlCqiInfo(
      const FfMacSchedSapProvider::SchedDlCqiInfoReqParameters &params)
      override;
  void DoReportUlCqiInfo(
      const FfMacSchedSapProvider::SchedUlCqiInfoReqParameters &params)
      override;
  void
  DoReportUlCqiInfo(std::map<uint16_t, std::vector<double>> ulCqiMap) override;
  uint8_t DoGetTpc(uint16_t rnti) override;
  uint16_t DoGetMinContinuousUlBandwidth() override;

  void DoReportUeMeas(uint16_t rnti,
                      LteRrcSap::MeasResults measResults) override;
  void DoRecvLoadInformation(EpcX2Sap::LoadInformationParams params) override;

private:
  void SetDownlinkConfiguration(uint16_t cellId, uint8_t bandwidth);
  void SetUplinkConfiguration(uint16_t cellId, uint8_t bandwidth);
  void InitializeDownlinkRbgMaps();
  void InitializeUplinkRbgMaps();

  double EstimateUlSinr(uint16_t rnti, uint16_t rb,
                        std::map<uint16_t, std::vector<double>> ulCqiMap);
  int GetCqiFromSpectralEfficiency(double s);

  LteFfrSapUser *m_ffrSapUser;
  LteFfrSapProvider *m_ffrSapProvider;

  LteFfrRrcSapUser *m_ffrRrcSapUser;
  LteFfrRrcSapProvider *m_ffrRrcSapProvider;

  uint8_t m_dlSubBandOffset;
  uint8_t m_dlReuse3SubBandwidth;
  uint8_t m_dlReuse1SubBandwidth;

  uint8_t m_ulSubBandOffset;
  uint8_t m_ulReuse3SubBandwidth;
  uint8_t m_ulReuse1SubBandwidth;

  std::vector<bool> m_dlRbgMap;
  std::vector<bool> m_ulRbgMap;

  std::vector<bool> m_dlReuse3RbgMap;
  std::vector<bool> m_dlReuse1RbgMap;
  std::vector<bool> m_dlPrimarySegmentRbgMap;
  std::vector<bool> m_dlSecondarySegmentRbgMap;

  std::vector<bool> m_ulReuse3RbgMap;
  std::vector<bool> m_ulReuse1RbgMap;
  std::vector<bool> m_ulPrimarySegmentRbgMap;
  std::vector<bool> m_ulSecondarySegmentRbgMap;

  enum UePosition { AreaUnset, CenterArea, EdgeArea };

  std::map<uint16_t, uint8_t> m_ues;

  uint8_t m_rsrqThreshold;

  uint8_t m_centerAreaPowerOffset;
  uint8_t m_edgeAreaPowerOffset;

  uint8_t m_centerAreaTpc;
  uint8_t m_edgeAreaTpc;

  uint8_t m_dlCqiThreshold;
  std::map<uint16_t, SbMeasResult_s> m_dlCqi;
  std::map<uint16_t, std::vector<bool>> m_dlRbgAvailableforUe;

  uint8_t m_ulCqiThreshold;
  std::map<uint16_t, std::vector<int>> m_ulCqi;
  std::map<uint16_t, std::vector<bool>> m_ulRbAvailableforUe;

  uint8_t m_measId;
};

} // namespace ns3

#endif
