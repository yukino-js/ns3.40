
#ifndef LTE_FFR_SIMPLE_H
#define LTE_FFR_SIMPLE_H

#include <ns3/lte-ffr-algorithm.h>
#include <ns3/lte-ffr-rrc-sap.h>
#include <ns3/lte-ffr-sap.h>
#include <ns3/lte-rrc-sap.h>
#include <ns3/traced-callback.h>

#include <map>

namespace ns3 {

class LteFfrSimple : public LteFfrAlgorithm {
public:
  LteFfrSimple();

  ~LteFfrSimple() override;

  static TypeId GetTypeId();

  void ChangePdschConfigDedicated(bool change);
  void
  SetPdschConfigDedicated(LteRrcSap::PdschConfigDedicated pdschConfigDedicated);

  void SetTpc(uint32_t tpc, uint32_t num, bool acculumatedMode);

  void SetLteFfrSapUser(LteFfrSapUser *s) override;
  LteFfrSapProvider *GetLteFfrSapProvider() override;

  void SetLteFfrRrcSapUser(LteFfrRrcSapUser *s) override;
  LteFfrRrcSapProvider *GetLteFfrRrcSapProvider() override;

  friend class MemberLteFfrSapProvider<LteFfrSimple>;
  friend class MemberLteFfrRrcSapProvider<LteFfrSimple>;

  typedef void (*PdschTracedCallback)(uint16_t rnti, uint8_t pdschPa);

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
  void UpdatePdschConfigDedicated();

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

  std::map<uint16_t, LteRrcSap::PdschConfigDedicated> m_ues;

  uint8_t m_measId;

  bool m_changePdschConfigDedicated;

  LteRrcSap::PdschConfigDedicated m_pdschConfigDedicated;

  TracedCallback<uint16_t, uint8_t> m_changePdschConfigDedicatedTrace;

  uint32_t m_tpc;
  uint32_t m_tpcNum;
  bool m_accumulatedMode;
};

} // namespace ns3

#endif
