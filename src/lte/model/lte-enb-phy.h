
#ifndef ENB_LTE_PHY_H
#define ENB_LTE_PHY_H

#include "lte-control-messages.h"
#include "lte-enb-cphy-sap.h"
#include "lte-enb-phy-sap.h"
#include "lte-harq-phy.h"
#include "lte-phy.h"

#include <map>
#include <set>

namespace ns3 {

class PacketBurst;
class LteNetDevice;
class LteUePhy;

class LteEnbPhy : public LtePhy {
  friend class EnbMemberLteEnbPhySapProvider;
  friend class MemberLteEnbCphySapProvider<LteEnbPhy>;

public:
  LteEnbPhy();

  LteEnbPhy(Ptr<LteSpectrumPhy> dlPhy, Ptr<LteSpectrumPhy> ulPhy);

  ~LteEnbPhy() override;

  static TypeId GetTypeId();
  void DoInitialize() override;
  void DoDispose() override;

  LteEnbPhySapProvider *GetLteEnbPhySapProvider();

  void SetLteEnbPhySapUser(LteEnbPhySapUser *s);

  LteEnbCphySapProvider *GetLteEnbCphySapProvider();

  void SetLteEnbCphySapUser(LteEnbCphySapUser *s);

  void SetTxPower(double pow);

  double GetTxPower() const;

  int8_t DoGetReferenceSignalPower() const;

  void SetNoiseFigure(double pow);

  double GetNoiseFigure() const;

  void SetMacChDelay(uint8_t delay);

  uint8_t GetMacChDelay() const;

  Ptr<LteSpectrumPhy> GetDlSpectrumPhy() const;

  Ptr<LteSpectrumPhy> GetUlSpectrumPhy() const;

  void SetDownlinkSubChannels(std::vector<int> mask);

  void SetDownlinkSubChannelsWithPowerAllocation(std::vector<int> mask);
  std::vector<int> GetDownlinkSubChannels();

  void GeneratePowerAllocationMap(uint16_t rnti, int rbId);

  Ptr<SpectrumValue> CreateTxPowerSpectralDensity() override;

  virtual Ptr<SpectrumValue> CreateTxPowerSpectralDensityWithPowerAllocation();

  void CalcChannelQualityForUe(std::vector<double> sinr,
                               Ptr<LteSpectrumPhy> ue);

  virtual void ReceiveLteControlMessage(Ptr<LteControlMessage> msg);

  FfMacSchedSapProvider::SchedUlCqiInfoReqParameters
  CreatePuschCqiReport(const SpectrumValue &sinr);

  FfMacSchedSapProvider::SchedUlCqiInfoReqParameters
  CreateSrsCqiReport(const SpectrumValue &sinr);

  void SendControlChannels(std::list<Ptr<LteControlMessage>> ctrlMsgList);

  void SendDataChannels(Ptr<PacketBurst> pb);

  void QueueUlDci(UlDciLteControlMessage m);

  std::list<UlDciLteControlMessage> DequeueUlDci();

  void StartFrame();
  void StartSubFrame();
  void EndSubFrame();
  void EndFrame();

  void PhyPduReceived(Ptr<Packet> p);

  virtual void
  ReceiveLteControlMessageList(std::list<Ptr<LteControlMessage>> msgList);

  void GenerateCtrlCqiReport(const SpectrumValue &sinr) override;
  void GenerateDataCqiReport(const SpectrumValue &sinr) override;
  void ReportInterference(const SpectrumValue &interf) override;
  void ReportRsReceivedPower(const SpectrumValue &power) override;

  virtual void ReportUlHarqFeedback(UlInfoListElement_s mes);

  void SetHarqPhyModule(Ptr<LteHarqPhy> harq);

  typedef void (*ReportUeSinrTracedCallback)(uint16_t cellId, uint16_t rnti,
                                             double sinrLinear,
                                             uint8_t componentCarrierId);

  typedef void (*ReportInterferenceTracedCallback)(
      uint16_t cellId, Ptr<SpectrumValue> spectrumValue);

private:
  void DoSetBandwidth(uint16_t ulBandwidth, uint16_t dlBandwidth);
  void DoSetEarfcn(uint32_t dlEarfcn, uint32_t ulEarfcn);
  void DoAddUe(uint16_t rnti);
  void DoRemoveUe(uint16_t rnti);
  void DoSetPa(uint16_t rnti, double pa);
  void DoSetTransmissionMode(uint16_t rnti, uint8_t txMode);
  void DoSetSrsConfigurationIndex(uint16_t rnti, uint16_t srcCi);
  void DoSetMasterInformationBlock(LteRrcSap::MasterInformationBlock mib);
  void
  DoSetSystemInformationBlockType1(LteRrcSap::SystemInformationBlockType1 sib1);

  void DoSendMacPdu(Ptr<Packet> p) override;
  void DoSendLteControlMessage(Ptr<LteControlMessage> msg);
  uint8_t DoGetMacChTtiDelay();

  bool AddUePhy(uint16_t rnti);
  bool DeleteUePhy(uint16_t rnti);

  void CreateSrsReport(uint16_t rnti, double srs);

  std::set<uint16_t> m_ueAttached;

  std::map<uint16_t, double> m_paMap;

  std::map<int, double> m_dlPowerAllocationMap;

  std::vector<int> m_listOfDownlinkSubchannel;

  std::vector<int> m_dlDataRbMap;

  std::vector<std::list<UlDciLteControlMessage>> m_ulDciQueue;

  LteEnbPhySapProvider *m_enbPhySapProvider;
  LteEnbPhySapUser *m_enbPhySapUser;

  LteEnbCphySapProvider *m_enbCphySapProvider;
  LteEnbCphySapUser *m_enbCphySapUser;

  uint32_t m_nrFrames;
  uint32_t m_nrSubFrames;

  uint16_t m_srsPeriodicity;
  Time m_srsStartTime;
  std::map<uint16_t, uint16_t> m_srsCounter;
  std::vector<uint16_t> m_srsUeOffset;
  uint16_t m_currentSrsOffset;

  LteRrcSap::MasterInformationBlock m_mib;
  LteRrcSap::SystemInformationBlockType1 m_sib1;

  Ptr<LteHarqPhy> m_harqPhyModule;

  TracedCallback<uint16_t, uint16_t, double, uint8_t> m_reportUeSinr;
  uint16_t m_srsSamplePeriod;
  std::map<uint16_t, uint16_t> m_srsSampleCounterMap;

  TracedCallback<uint16_t, Ptr<SpectrumValue>> m_reportInterferenceTrace;
  uint16_t m_interferenceSamplePeriod;
  uint16_t m_interferenceSampleCounter;

  TracedCallback<PhyTransmissionStatParameters> m_dlPhyTransmission;
};

} // namespace ns3

#endif
