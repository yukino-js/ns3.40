
#ifndef LTE_UE_PHY_H
#define LTE_UE_PHY_H

#include "ff-mac-common.h"
#include "lte-amc.h"
#include "lte-control-messages.h"
#include "lte-phy.h"
#include "lte-ue-cphy-sap.h"
#include "lte-ue-phy-sap.h"
#include "lte-ue-power-control.h"

#include <ns3/ptr.h>

#include <set>

namespace ns3 {

class PacketBurst;
class LteEnbPhy;
class LteHarqPhy;

class LteUePhy : public LtePhy {
  friend class UeMemberLteUePhySapProvider;
  friend class MemberLteUeCphySapProvider<LteUePhy>;

public:
  enum State { CELL_SEARCH = 0, SYNCHRONIZED, NUM_STATES };

  LteUePhy();

  LteUePhy(Ptr<LteSpectrumPhy> dlPhy, Ptr<LteSpectrumPhy> ulPhy);

  ~LteUePhy() override;

  static TypeId GetTypeId();
  void DoInitialize() override;
  void DoDispose() override;

  LteUePhySapProvider *GetLteUePhySapProvider();

  void SetLteUePhySapUser(LteUePhySapUser *s);

  LteUeCphySapProvider *GetLteUeCphySapProvider();

  void SetLteUeCphySapUser(LteUeCphySapUser *s);

  void SetTxPower(double pow);

  double GetTxPower() const;

  Ptr<LteUePowerControl> GetUplinkPowerControl() const;

  void SetNoiseFigure(double nf);

  double GetNoiseFigure() const;

  uint8_t GetMacChDelay() const;

  Ptr<LteSpectrumPhy> GetDlSpectrumPhy() const;

  Ptr<LteSpectrumPhy> GetUlSpectrumPhy() const;

  Ptr<SpectrumValue> CreateTxPowerSpectralDensity() override;

  void SetSubChannelsForTransmission(std::vector<int> mask);
  std::vector<int> GetSubChannelsForTransmission();

  void SetSubChannelsForReception(std::vector<int> mask);
  std::vector<int> GetSubChannelsForReception();

  Ptr<DlCqiLteControlMessage>
  CreateDlCqiFeedbackMessage(const SpectrumValue &sinr);

  void GenerateCtrlCqiReport(const SpectrumValue &sinr) override;
  void GenerateDataCqiReport(const SpectrumValue &sinr) override;
  virtual void GenerateMixedCqiReport(const SpectrumValue &sinr);
  void ReportInterference(const SpectrumValue &interf) override;
  virtual void ReportDataInterference(const SpectrumValue &interf);
  void ReportRsReceivedPower(const SpectrumValue &power) override;

  virtual void
  ReceiveLteControlMessageList(std::list<Ptr<LteControlMessage>> msgList);
  virtual void ReceivePss(uint16_t cellId, Ptr<SpectrumValue> p);

  void PhyPduReceived(Ptr<Packet> p);

  void SubframeIndication(uint32_t frameNo, uint32_t subframeNo);

  void SendSrs();

  virtual void EnqueueDlHarqFeedback(DlInfoListElement_s mes);

  void SetHarqPhyModule(Ptr<LteHarqPhy> harq);

  State GetState() const;

  typedef void (*StateTracedCallback)(uint16_t cellId, uint16_t rnti,
                                      State oldState, State newState);

  typedef void (*RsrpSinrTracedCallback)(uint16_t cellId, uint16_t rnti,
                                         double rsrp, double sinr,
                                         uint8_t componentCarrierId);

  typedef void (*RsrpRsrqTracedCallback)(uint16_t rnti, uint16_t cellId,
                                         double rsrp, double rsrq,
                                         bool isServingCell,
                                         uint8_t componentCarrierId);

  typedef void (*UlPhyResourceBlocksTracedCallback)(
      uint16_t rnti, const std::vector<int> &rbs);

  typedef void (*PowerSpectralDensityTracedCallback)(uint16_t rnti,
                                                     Ptr<SpectrumValue> psd);

private:
  void SetTxMode1Gain(double gain);
  void SetTxMode2Gain(double gain);
  void SetTxMode3Gain(double gain);
  void SetTxMode4Gain(double gain);
  void SetTxMode5Gain(double gain);
  void SetTxMode6Gain(double gain);
  void SetTxMode7Gain(double gain);
  void SetTxModeGain(uint8_t txMode, double gain);
  void QueueSubChannelsForTransmission(std::vector<int> rbMap);
  void GenerateCqiRsrpRsrq(const SpectrumValue &sinr);
  void ReportUeMeasurements();
  void SetDownlinkCqiPeriodicity(Time cqiPeriodicity);
  void SwitchToState(State s);
  void SetNumQoutEvalSf(uint16_t numSubframes);
  void SetNumQinEvalSf(uint16_t numSubframes);
  uint16_t GetNumQoutEvalSf() const;
  uint16_t GetNumQinEvalSf() const;

  void DoReset();
  void DoStartCellSearch(uint32_t dlEarfcn);
  void DoSynchronizeWithEnb(uint16_t cellId);
  void DoSynchronizeWithEnb(uint16_t cellId, uint32_t dlEarfcn);
  uint16_t DoGetCellId();
  uint32_t DoGetDlEarfcn();
  void DoSetDlBandwidth(uint16_t dlBandwidth);
  void DoConfigureUplink(uint32_t ulEarfcn, uint16_t ulBandwidth);
  void DoConfigureReferenceSignalPower(int8_t referenceSignalPower);
  void DoSetRnti(uint16_t rnti);
  void DoSetTransmissionMode(uint8_t txMode);
  void DoSetSrsConfigurationIndex(uint16_t srcCi);
  void DoSetPa(double pa);
  void DoResetPhyAfterRlf();
  void DoResetRlfParams();

  void DoStartInSyncDetection();

  void RlfDetection(double sinrdB);
  void InitializeRlfParams();
  void DoSetImsi(uint64_t imsi);
  void DoSetRsrpFilterCoefficient(uint8_t rsrpFilterCoefficient);
  double ComputeAvgSinr(const SpectrumValue &sinr);

  void DoSendMacPdu(Ptr<Packet> p) override;
  virtual void DoSendLteControlMessage(Ptr<LteControlMessage> msg);
  virtual void DoSendRachPreamble(uint32_t prachId, uint32_t raRnti);
  virtual void DoNotifyConnectionSuccessful();

  std::vector<int> m_subChannelsForTransmission;
  std::vector<int> m_subChannelsForReception;

  std::vector<std::vector<int>> m_subChannelsForTransmissionQueue;

  Ptr<LteAmc> m_amc;

  bool m_enableUplinkPowerControl;
  Ptr<LteUePowerControl> m_powerControl;

  Time m_p10CqiPeriodicity;
  Time m_p10CqiLast;

  Time m_a30CqiPeriodicity;
  Time m_a30CqiLast;

  LteUePhySapProvider *m_uePhySapProvider;
  LteUePhySapUser *m_uePhySapUser;

  LteUeCphySapProvider *m_ueCphySapProvider;
  LteUeCphySapUser *m_ueCphySapUser;

  uint16_t m_rnti;

  uint8_t m_transmissionMode;
  std::vector<double> m_txModeGain;

  uint16_t m_srsPeriodicity;
  uint16_t m_srsSubframeOffset;
  bool m_srsConfigured;
  Time m_srsStartTime;

  double m_paLinear;

  bool m_dlConfigured;
  bool m_ulConfigured;

  State m_state;
  TracedCallback<uint16_t, uint16_t, State, State> m_stateTransitionTrace;

  uint8_t m_subframeNo;

  bool m_rsReceivedPowerUpdated;
  SpectrumValue m_rsReceivedPower;

  bool m_rsInterferencePowerUpdated;
  SpectrumValue m_rsInterferencePower;

  bool m_dataInterferencePowerUpdated;
  SpectrumValue m_dataInterferencePower;

  bool m_pssReceived;

  struct PssElement {
    uint16_t cellId;
    double pssPsdSum;
    uint16_t nRB;
  };

  std::list<PssElement> m_pssList;

  double m_pssReceptionThreshold;

  struct UeMeasurementsElement {
    double rsrpSum;
    uint8_t rsrpNum;
    double rsrqSum;
    uint8_t rsrqNum;
  };

  std::map<uint16_t, UeMeasurementsElement> m_ueMeasurementsMap;
  Time m_ueMeasurementsFilterPeriod;
  Time m_ueMeasurementsFilterLast;

  Ptr<LteHarqPhy> m_harqPhyModule;

  uint32_t m_raPreambleId;
  uint32_t m_raRnti;

  TracedCallback<uint16_t, uint16_t, double, double, uint8_t>
      m_reportCurrentCellRsrpSinrTrace;
  uint16_t m_rsrpSinrSamplePeriod;
  uint16_t m_rsrpSinrSampleCounter;

  TracedCallback<uint16_t, uint16_t, double, double, bool, uint8_t>
      m_reportUeMeasurements;

  EventId m_sendSrsEvent;

  TracedCallback<PhyTransmissionStatParameters> m_ulPhyTransmission;

  TracedCallback<uint16_t, const std::vector<int> &>
      m_reportUlPhyResourceBlocks;

  TracedCallback<uint16_t, Ptr<SpectrumValue>> m_reportPowerSpectralDensity;

  Ptr<SpectrumValue> m_noisePsd;

  bool m_isConnected;
  double m_qIn;

  double m_qOut;

  uint16_t m_numOfQoutEvalSf;
  uint16_t m_numOfQinEvalSf;

  bool m_downlinkInSync;
  uint16_t m_numOfSubframes;
  uint16_t m_numOfFrames;
  double m_sinrDbFrame;
  SpectrumValue m_ctrlSinrForRlf;
  uint64_t m_imsi;
  bool m_enableRlfDetection;
};

} // namespace ns3

#endif
