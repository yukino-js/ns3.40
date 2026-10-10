
#ifndef LTE_FR_HARD_ALGORITHM_H
#define LTE_FR_HARD_ALGORITHM_H

#include "lte-ffr-algorithm.h"
#include "lte-ffr-rrc-sap.h"
#include "lte-ffr-sap.h"
#include "lte-rrc-sap.h"

namespace ns3 {

class LteFrHardAlgorithm : public LteFfrAlgorithm {
public:
  LteFrHardAlgorithm();

  ~LteFrHardAlgorithm() override;

  static TypeId GetTypeId();

  void SetLteFfrSapUser(LteFfrSapUser *s) override;
  LteFfrSapProvider *GetLteFfrSapProvider() override;

  void SetLteFfrRrcSapUser(LteFfrRrcSapUser *s) override;
  LteFfrRrcSapProvider *GetLteFfrRrcSapProvider() override;

  friend class MemberLteFfrSapProvider<LteFrHardAlgorithm>;
  friend class MemberLteFfrRrcSapProvider<LteFrHardAlgorithm>;

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

  uint8_t m_dlOffset;
  uint8_t m_dlSubBand;

  uint8_t m_ulOffset;
  uint8_t m_ulSubBand;

  std::vector<bool> m_dlRbgMap;
  std::vector<bool> m_ulRbgMap;
};

} // namespace ns3

#endif
