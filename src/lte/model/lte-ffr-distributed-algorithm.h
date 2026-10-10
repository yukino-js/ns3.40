
#ifndef LTE_FFR_DISTRIBUTED_ALGORITHM_H
#define LTE_FFR_DISTRIBUTED_ALGORITHM_H

#include "lte-ffr-algorithm.h"
#include "lte-ffr-rrc-sap.h"
#include "lte-ffr-sap.h"
#include "lte-rrc-sap.h"

namespace ns3 {

class LteFfrDistributedAlgorithm : public LteFfrAlgorithm {
public:
  LteFfrDistributedAlgorithm();
  ~LteFfrDistributedAlgorithm() override;

  static TypeId GetTypeId();

  void SetLteFfrSapUser(LteFfrSapUser *s) override;
  LteFfrSapProvider *GetLteFfrSapProvider() override;

  void SetLteFfrRrcSapUser(LteFfrRrcSapUser *s) override;
  LteFfrRrcSapProvider *GetLteFfrRrcSapProvider() override;

  friend class MemberLteFfrSapProvider<LteFfrDistributedAlgorithm>;
  friend class MemberLteFfrRrcSapProvider<LteFfrDistributedAlgorithm>;

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

  void UpdateNeighbourMeasurements(uint16_t rnti, uint16_t cellId, uint8_t rsrp,
                                   uint8_t rsrq);

  void Calculate();
  void SendLoadInformation(uint16_t targetCellId);

  LteFfrSapUser *m_ffrSapUser;
  LteFfrSapProvider *m_ffrSapProvider;

  LteFfrRrcSapUser *m_ffrRrcSapUser;
  LteFfrRrcSapProvider *m_ffrRrcSapProvider;

  std::vector<bool> m_dlRbgMap;
  std::vector<bool> m_ulRbgMap;

  uint8_t m_edgeRbNum;
  std::vector<bool> m_dlEdgeRbgMap;
  std::vector<bool> m_ulEdgeRbgMap;

  enum UePosition { AreaUnset, CenterArea, EdgeArea };

  std::map<uint16_t, uint8_t> m_ues;

  uint8_t m_edgeSubBandRsrqThreshold;

  uint8_t m_centerPowerOffset;
  uint8_t m_edgePowerOffset;

  uint8_t m_centerAreaTpc;
  uint8_t m_edgeAreaTpc;

  Time m_calculationInterval;
  EventId m_calculationEvent;

  uint8_t m_rsrqMeasId;
  uint8_t m_rsrpMeasId;

  class UeMeasure : public SimpleRefCount<UeMeasure> {
  public:
    uint16_t m_cellId;
    uint8_t m_rsrp;
    uint8_t m_rsrq;
  };

  typedef std::map<uint16_t, Ptr<UeMeasure>> MeasurementRow_t;
  typedef std::map<uint16_t, MeasurementRow_t> MeasurementTable_t;
  MeasurementTable_t m_ueMeasures;

  std::vector<uint16_t> m_neighborCell;

  uint8_t m_rsrpDifferenceThreshold;

  std::map<uint16_t, uint32_t> m_cellWeightMap;

  std::map<uint16_t, std::vector<bool>> m_rntp;
};

} // namespace ns3

#endif
