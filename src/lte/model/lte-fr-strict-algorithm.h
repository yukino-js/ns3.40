
#ifndef LTE_FR_STRICT_ALGORITHM_H
#define LTE_FR_STRICT_ALGORITHM_H

#include "lte-ffr-algorithm.h"
#include "lte-ffr-rrc-sap.h"
#include "lte-ffr-sap.h"
#include "lte-rrc-sap.h"

#include <map>

namespace ns3 {

class LteFrStrictAlgorithm : public LteFfrAlgorithm {
public:
  LteFrStrictAlgorithm();

  ~LteFrStrictAlgorithm() override;

  static TypeId GetTypeId();

  void SetLteFfrSapUser(LteFfrSapUser *s) override;
  LteFfrSapProvider *GetLteFfrSapProvider() override;

  void SetLteFfrRrcSapUser(LteFfrRrcSapUser *s) override;
  LteFfrRrcSapProvider *GetLteFfrRrcSapProvider() override;

  friend class MemberLteFfrSapProvider<LteFrStrictAlgorithm>;
  friend class MemberLteFfrRrcSapProvider<LteFrStrictAlgorithm>;

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

  LteFfrSapUser *m_ffrSapUser;
  LteFfrSapProvider *m_ffrSapProvider;

  LteFfrRrcSapUser *m_ffrRrcSapUser;
  LteFfrRrcSapProvider *m_ffrRrcSapProvider;

  uint8_t m_dlCommonSubBandwidth;
  uint8_t m_dlEdgeSubBandOffset;
  uint8_t m_dlEdgeSubBandwidth;

  uint8_t m_ulCommonSubBandwidth;
  uint8_t m_ulEdgeSubBandOffset;
  uint8_t m_ulEdgeSubBandwidth;

  std::vector<bool> m_dlRbgMap;
  std::vector<bool> m_ulRbgMap;

  std::vector<bool> m_dlEdgeRbgMap;
  std::vector<bool> m_ulEdgeRbgMap;

  enum SubBand { AreaUnset, CellCenter, CellEdge };

  std::map<uint16_t, uint8_t> m_ues;
  std::vector<uint16_t> m_edgeUes;

  uint8_t m_edgeSubBandThreshold;

  uint8_t m_centerAreaPowerOffset;
  uint8_t m_edgeAreaPowerOffset;

  uint8_t m_centerAreaTpc;
  uint8_t m_edgeAreaTpc;

  uint8_t m_measId;
};

} // namespace ns3

#endif
