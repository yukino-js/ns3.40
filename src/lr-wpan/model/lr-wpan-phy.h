#ifndef LR_WPAN_PHY_H
#define LR_WPAN_PHY_H

#include "lr-wpan-interference-helper.h"

#include <ns3/event-id.h>
#include <ns3/spectrum-phy.h>
#include <ns3/traced-callback.h>
#include <ns3/traced-value.h>

namespace ns3 {

class Packet;
class SpectrumValue;
class LrWpanErrorModel;
struct LrWpanSpectrumSignalParameters;
class MobilityModel;
class SpectrumChannel;
class SpectrumModel;
class AntennaModel;
class NetDevice;
class UniformRandomVariable;
class ErrorModel;

struct LrWpanEdPower {
  double averagePower;
  Time lastUpdate;
  Time measurementLength;
};

struct LrWpanPhyDataAndSymbolRates {
  double bitRate;
  double symbolRate;
};

struct LrWpanPhyPpduHeaderSymbolNumber {
  double shrPreamble;
  double shrSfd;
  double phr;
};

enum LrWpanPhyOption {
  IEEE_802_15_4_868MHZ_BPSK,
  IEEE_802_15_4_915MHZ_BPSK,
  IEEE_802_15_4_950MHZ_BPSK,
  IEEE_802_15_4_868MHZ_ASK,
  IEEE_802_15_4_915MHZ_ASK,
  IEEE_802_15_4_780MHZ_OQPSK,
  IEEE_802_15_4_868MHZ_OQPSK,
  IEEE_802_15_4_915MHZ_OQPSK,
  IEEE_802_15_4_2_4GHZ_OQPSK,
  IEEE_802_15_4_INVALID_PHY_OPTION
};

enum LrWpanPhyEnumeration {
  IEEE_802_15_4_PHY_BUSY = 0x00,
  IEEE_802_15_4_PHY_BUSY_RX = 0x01,
  IEEE_802_15_4_PHY_BUSY_TX = 0x02,
  IEEE_802_15_4_PHY_FORCE_TRX_OFF = 0x03,
  IEEE_802_15_4_PHY_IDLE = 0x04,
  IEEE_802_15_4_PHY_INVALID_PARAMETER = 0x05,
  IEEE_802_15_4_PHY_RX_ON = 0x06,
  IEEE_802_15_4_PHY_SUCCESS = 0x07,
  IEEE_802_15_4_PHY_TRX_OFF = 0x08,
  IEEE_802_15_4_PHY_TX_ON = 0x09,
  IEEE_802_15_4_PHY_UNSUPPORTED_ATTRIBUTE = 0xa,
  IEEE_802_15_4_PHY_READ_ONLY = 0xb,
  IEEE_802_15_4_PHY_UNSPECIFIED = 0xc
};

namespace TracedValueCallback {
typedef void (*LrWpanPhyEnumeration)(LrWpanPhyEnumeration oldValue,
                                     LrWpanPhyEnumeration newValue);
}

enum LrWpanPibAttributeIdentifier {
  phyCurrentChannel = 0x00,
  phyChannelsSupported = 0x01,
  phyTransmitPower = 0x02,
  phyCCAMode = 0x03,
  phyCurrentPage = 0x04,
  phyMaxFrameDuration = 0x05,
  phySHRDuration = 0x06,
  phySymbolsPerOctet = 0x07
};

struct LrWpanPhyPibAttributes : public SimpleRefCount<LrWpanPhyPibAttributes> {
  uint8_t phyCurrentChannel;
  uint32_t phyChannelsSupported[32];
  uint8_t phyTransmitPower;
  uint8_t phyCCAMode;
  uint8_t phyCurrentPage;
  uint32_t phyMaxFrameDuration;
  uint32_t phySHRDuration;
  double phySymbolsPerOctet;
};

typedef Callback<void, uint32_t, Ptr<Packet>, uint8_t> PdDataIndicationCallback;

typedef Callback<void, LrWpanPhyEnumeration> PdDataConfirmCallback;

typedef Callback<void, LrWpanPhyEnumeration> PlmeCcaConfirmCallback;

typedef Callback<void, LrWpanPhyEnumeration, uint8_t> PlmeEdConfirmCallback;

typedef Callback<void, LrWpanPhyEnumeration, LrWpanPibAttributeIdentifier,
                 Ptr<LrWpanPhyPibAttributes>>
    PlmeGetAttributeConfirmCallback;

typedef Callback<void, LrWpanPhyEnumeration> PlmeSetTRXStateConfirmCallback;

typedef Callback<void, LrWpanPhyEnumeration, LrWpanPibAttributeIdentifier>
    PlmeSetAttributeConfirmCallback;

class LrWpanPhy : public SpectrumPhy {
public:
  static TypeId GetTypeId();

  LrWpanPhy();
  ~LrWpanPhy() override;

  void SetMobility(Ptr<MobilityModel> m) override;
  Ptr<MobilityModel> GetMobility() const override;
  void SetChannel(Ptr<SpectrumChannel> c) override;

  Ptr<SpectrumChannel> GetChannel();
  void SetDevice(Ptr<NetDevice> d) override;
  Ptr<NetDevice> GetDevice() const override;

  void SetAntenna(Ptr<AntennaModel> a);
  Ptr<Object> GetAntenna() const override;
  Ptr<const SpectrumModel> GetRxSpectrumModel() const override;

  void SetTxPowerSpectralDensity(Ptr<SpectrumValue> txPsd);

  void SetNoisePowerSpectralDensity(Ptr<const SpectrumValue> noisePsd);

  Ptr<const SpectrumValue> GetNoisePowerSpectralDensity();

  void SetPhyOption(LrWpanPhyOption phyOption);

  void SetRxSensitivity(double dbmSensitivity);

  double GetRxSensitivity();

  void StartRx(Ptr<SpectrumSignalParameters> params) override;

  void PdDataRequest(const uint32_t psduLength, Ptr<Packet> p);

  void PlmeCcaRequest();

  void CcaCancel();

  void PlmeEdRequest();

  void PlmeGetAttributeRequest(LrWpanPibAttributeIdentifier id);

  void PlmeSetTRXStateRequest(LrWpanPhyEnumeration state);

  void PlmeSetAttributeRequest(LrWpanPibAttributeIdentifier id,
                               Ptr<LrWpanPhyPibAttributes> attribute);

  void SetPdDataIndicationCallback(PdDataIndicationCallback c);

  void SetPdDataConfirmCallback(PdDataConfirmCallback c);

  void SetPlmeCcaConfirmCallback(PlmeCcaConfirmCallback c);

  void SetPlmeEdConfirmCallback(PlmeEdConfirmCallback c);

  void SetPlmeGetAttributeConfirmCallback(PlmeGetAttributeConfirmCallback c);

  void SetPlmeSetTRXStateConfirmCallback(PlmeSetTRXStateConfirmCallback c);

  void SetPlmeSetAttributeConfirmCallback(PlmeSetAttributeConfirmCallback c);

  uint8_t GetCurrentPage() const;

  uint8_t GetCurrentChannelNum() const;

  double GetDataOrSymbolRate(bool isData);

  void SetErrorModel(Ptr<LrWpanErrorModel> e);

  Ptr<LrWpanErrorModel> GetErrorModel() const;

  void SetPostReceptionErrorModel(const Ptr<ErrorModel> em);

  uint64_t GetPhySHRDuration() const;

  double GetPhySymbolsPerOctet() const;

  double GetCurrentSignalPsd();

  int64_t AssignStreams(int64_t stream);

  typedef void (*StateTracedCallback)(Time time, LrWpanPhyEnumeration oldState,
                                      LrWpanPhyEnumeration newState);

private:
  typedef std::pair<Ptr<Packet>, bool> PacketAndStatus;

  void DoInitialize() override;
  void DoDispose() override;

  void ChangeTrxState(LrWpanPhyEnumeration newState);

  LrWpanPhyOption GetMyPhyOption();

  void EndTx();

  void CheckInterference();

  void EndRx(Ptr<SpectrumSignalParameters> params);

  void CancelEd(LrWpanPhyEnumeration state);

  void EndEd();

  void EndCca();

  void EndSetTRXState();

  Time CalculateTxTime(Ptr<const Packet> packet);

  Time GetPpduHeaderTxTime();

  bool ChannelSupported(uint8_t channel);

  bool PageSupported(uint8_t page);

  bool PhyIsBusy() const;

  TracedCallback<Ptr<const Packet>> m_phyTxBeginTrace;

  TracedCallback<Ptr<const Packet>> m_phyTxEndTrace;

  TracedCallback<Ptr<const Packet>> m_phyTxDropTrace;

  TracedCallback<Ptr<const Packet>> m_phyRxBeginTrace;

  TracedCallback<Ptr<const Packet>, double> m_phyRxEndTrace;

  TracedCallback<Ptr<const Packet>> m_phyRxDropTrace;

  TracedCallback<Time, LrWpanPhyEnumeration, LrWpanPhyEnumeration>
      m_trxStateLogger;

  int8_t GetNominalTxPowerFromPib(uint8_t phyTransmitPower);

  double WToDbm(double watt);

  double DbmToW(double dbm);

  Ptr<MobilityModel> m_mobility;

  Ptr<NetDevice> m_device;

  Ptr<SpectrumChannel> m_channel;

  Ptr<AntennaModel> m_antenna;

  Ptr<SpectrumValue> m_txPsd;

  Ptr<const SpectrumValue> m_noise;

  Ptr<LrWpanErrorModel> m_errorModel;

  LrWpanPhyPibAttributes m_phyPIBAttributes;

  TracedValue<LrWpanPhyEnumeration> m_trxState;

  LrWpanPhyEnumeration m_trxStatePending;

  PdDataIndicationCallback m_pdDataIndicationCallback;

  PdDataConfirmCallback m_pdDataConfirmCallback;

  PlmeCcaConfirmCallback m_plmeCcaConfirmCallback;

  PlmeEdConfirmCallback m_plmeEdConfirmCallback;

  PlmeGetAttributeConfirmCallback m_plmeGetAttributeConfirmCallback;

  PlmeSetTRXStateConfirmCallback m_plmeSetTRXStateConfirmCallback;

  PlmeSetAttributeConfirmCallback m_plmeSetAttributeConfirmCallback;

  LrWpanPhyOption m_phyOption;

  LrWpanEdPower m_edPower;

  double m_ccaPeakPower;

  double m_rxSensitivity;

  bool m_isRxCanceled;

  Ptr<LrWpanInterferenceHelper> m_signal;

  Time m_rxLastUpdate;

  std::pair<Ptr<LrWpanSpectrumSignalParameters>, bool> m_currentRxPacket;

  PacketAndStatus m_currentTxPacket;

  EventId m_ccaRequest;

  EventId m_edRequest;

  EventId m_setTRXState;

  EventId m_pdDataRequest;

  Ptr<UniformRandomVariable> m_random;

  Ptr<ErrorModel> m_postReceptionErrorModel;
};

} // namespace ns3

#endif
