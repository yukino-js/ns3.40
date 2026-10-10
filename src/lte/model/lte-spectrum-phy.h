
#ifndef LTE_SPECTRUM_PHY_H
#define LTE_SPECTRUM_PHY_H

#include "ff-mac-common.h"
#include "lte-common.h"
#include "lte-harq-phy.h"
#include "lte-interference.h"

#include <ns3/data-rate.h>
#include <ns3/event-id.h>
#include <ns3/generic-phy.h>
#include <ns3/mobility-model.h>
#include <ns3/net-device.h>
#include <ns3/nstime.h>
#include <ns3/packet-burst.h>
#include <ns3/packet.h>
#include <ns3/random-variable-stream.h>
#include <ns3/spectrum-channel.h>
#include <ns3/spectrum-interference.h>
#include <ns3/spectrum-phy.h>
#include <ns3/spectrum-value.h>

#include <map>

namespace ns3 {

struct TbId_t {
  uint16_t m_rnti;
  uint8_t m_layer;

public:
  TbId_t();
  TbId_t(const uint16_t a, const uint8_t b);

  friend bool operator==(const TbId_t &a, const TbId_t &b);
  friend bool operator<(const TbId_t &a, const TbId_t &b);
};

struct tbInfo_t {
  uint8_t ndi;
  uint16_t size;
  uint8_t mcs;
  std::vector<int> rbBitmap;
  uint8_t harqProcessId;
  uint8_t rv;
  double mi;
  bool downlink;
  bool corrupt;
  bool harqFeedbackSent;
};

typedef std::map<TbId_t, tbInfo_t> expectedTbs_t;

class LteNetDevice;
class AntennaModel;
class LteControlMessage;
struct LteSpectrumSignalParametersDataFrame;
struct LteSpectrumSignalParametersDlCtrlFrame;
struct LteSpectrumSignalParametersUlSrsFrame;

typedef Callback<void> LtePhyRxDataEndErrorCallback;
typedef Callback<void, Ptr<Packet>> LtePhyRxDataEndOkCallback;

typedef Callback<void, std::list<Ptr<LteControlMessage>>>
    LtePhyRxCtrlEndOkCallback;

typedef Callback<void> LtePhyRxCtrlEndErrorCallback;

typedef Callback<void, uint16_t, Ptr<SpectrumValue>> LtePhyRxPssCallback;

typedef Callback<void, DlInfoListElement_s> LtePhyDlHarqFeedbackCallback;

typedef Callback<void, UlInfoListElement_s> LtePhyUlHarqFeedbackCallback;

class LteSpectrumPhy : public SpectrumPhy {
public:
  LteSpectrumPhy();
  ~LteSpectrumPhy() override;

  enum State {
    IDLE,
    TX_DL_CTRL,
    TX_DATA,
    TX_UL_SRS,
    RX_DL_CTRL,
    RX_DATA,
    RX_UL_SRS
  };

  static TypeId GetTypeId();
  void DoDispose() override;

  void SetChannel(Ptr<SpectrumChannel> c) override;
  void SetMobility(Ptr<MobilityModel> m) override;
  void SetDevice(Ptr<NetDevice> d) override;
  Ptr<MobilityModel> GetMobility() const override;
  Ptr<NetDevice> GetDevice() const override;
  Ptr<const SpectrumModel> GetRxSpectrumModel() const override;
  Ptr<Object> GetAntenna() const override;
  void StartRx(Ptr<SpectrumSignalParameters> params) override;
  void StartRxData(Ptr<LteSpectrumSignalParametersDataFrame> params);
  void
  StartRxDlCtrl(Ptr<LteSpectrumSignalParametersDlCtrlFrame> lteDlCtrlRxParams);
  void
  StartRxUlSrs(Ptr<LteSpectrumSignalParametersUlSrsFrame> lteUlSrsRxParams);
  void SetHarqPhyModule(Ptr<LteHarqPhy> harq);

  void SetTxPowerSpectralDensity(Ptr<SpectrumValue> txPsd);

  void SetNoisePowerSpectralDensity(Ptr<const SpectrumValue> noisePsd);

  void Reset();

  void SetAntenna(Ptr<AntennaModel> a);

  bool StartTxDataFrame(Ptr<PacketBurst> pb,
                        std::list<Ptr<LteControlMessage>> ctrlMsgList,
                        Time duration);

  bool StartTxDlCtrlFrame(std::list<Ptr<LteControlMessage>> ctrlMsgList,
                          bool pss);

  bool StartTxUlSrsFrame();

  void SetLtePhyRxDataEndErrorCallback(LtePhyRxDataEndErrorCallback c);

  void SetLtePhyRxDataEndOkCallback(LtePhyRxDataEndOkCallback c);

  void SetLtePhyRxCtrlEndOkCallback(LtePhyRxCtrlEndOkCallback c);

  void SetLtePhyRxCtrlEndErrorCallback(LtePhyRxCtrlEndErrorCallback c);

  void SetLtePhyRxPssCallback(LtePhyRxPssCallback c);

  void SetLtePhyDlHarqFeedbackCallback(LtePhyDlHarqFeedbackCallback c);

  void SetLtePhyUlHarqFeedbackCallback(LtePhyUlHarqFeedbackCallback c);

  void SetState(State newState);

  void SetCellId(uint16_t cellId);

  void SetComponentCarrierId(uint8_t componentCarrierId);

  void AddRsPowerChunkProcessor(Ptr<LteChunkProcessor> p);

  void AddDataPowerChunkProcessor(Ptr<LteChunkProcessor> p);

  void AddDataSinrChunkProcessor(Ptr<LteChunkProcessor> p);

  void AddInterferenceCtrlChunkProcessor(Ptr<LteChunkProcessor> p);

  void AddInterferenceDataChunkProcessor(Ptr<LteChunkProcessor> p);

  void AddCtrlSinrChunkProcessor(Ptr<LteChunkProcessor> p);

  void AddExpectedTb(uint16_t rnti, uint8_t ndi, uint16_t size, uint8_t mcs,
                     std::vector<int> map, uint8_t layer, uint8_t harqId,
                     uint8_t rv, bool downlink);
  void RemoveExpectedTb(uint16_t rnti);

  void UpdateSinrPerceived(const SpectrumValue &sinr);

  void SetTransmissionMode(uint8_t txMode);

  Ptr<SpectrumChannel> GetChannel();

  friend class LteUePhy;

  int64_t AssignStreams(int64_t stream);

private:
  void ChangeState(State newState);
  void EndTxData();
  void EndTxDlCtrl();
  void EndTxUlSrs();
  void EndRxData();
  void EndRxDlCtrl();
  void EndRxUlSrs();

  void SetTxModeGain(uint8_t txMode, double gain);

  Ptr<MobilityModel> m_mobility;
  Ptr<AntennaModel> m_antenna;
  Ptr<NetDevice> m_device;

  Ptr<SpectrumChannel> m_channel;

  Ptr<const SpectrumModel> m_rxSpectrumModel;
  Ptr<SpectrumValue> m_txPsd;
  Ptr<PacketBurst> m_txPacketBurst;
  std::list<Ptr<PacketBurst>> m_rxPacketBurstList;

  std::list<Ptr<LteControlMessage>> m_txControlMessageList;
  std::list<Ptr<LteControlMessage>> m_rxControlMessageList;

  State m_state;
  Time m_firstRxStart;
  Time m_firstRxDuration;

  TracedCallback<Ptr<const PacketBurst>> m_phyTxStartTrace;
  TracedCallback<Ptr<const PacketBurst>> m_phyTxEndTrace;
  TracedCallback<Ptr<const PacketBurst>> m_phyRxStartTrace;
  TracedCallback<Ptr<const Packet>> m_phyRxEndOkTrace;
  TracedCallback<Ptr<const Packet>> m_phyRxEndErrorTrace;

  LtePhyRxDataEndErrorCallback m_ltePhyRxDataEndErrorCallback;
  LtePhyRxDataEndOkCallback m_ltePhyRxDataEndOkCallback;

  LtePhyRxCtrlEndOkCallback m_ltePhyRxCtrlEndOkCallback;
  LtePhyRxCtrlEndErrorCallback m_ltePhyRxCtrlEndErrorCallback;
  LtePhyRxPssCallback m_ltePhyRxPssCallback;

  Ptr<LteInterference> m_interferenceData;
  Ptr<LteInterference> m_interferenceCtrl;

  uint16_t m_cellId;

  uint8_t m_componentCarrierId;
  expectedTbs_t m_expectedTbs;
  SpectrumValue m_sinrPerceived;

  Ptr<UniformRandomVariable> m_random;
  bool m_dataErrorModelEnabled;
  bool m_ctrlErrorModelEnabled;

  uint8_t m_transmissionMode;
  uint8_t m_layersNum;
  std::vector<double> m_txModeGain;

  Ptr<LteHarqPhy> m_harqPhyModule;
  LtePhyDlHarqFeedbackCallback m_ltePhyDlHarqFeedbackCallback;
  LtePhyUlHarqFeedbackCallback m_ltePhyUlHarqFeedbackCallback;

  TracedCallback<PhyReceptionStatParameters> m_dlPhyReception;

  TracedCallback<PhyReceptionStatParameters> m_ulPhyReception;

  EventId m_endTxEvent;
  EventId m_endRxDataEvent;
  EventId m_endRxDlCtrlEvent;
  EventId m_endRxUlSrsEvent;
};

} // namespace ns3

#endif
