
#ifndef SPECTRUM_WIFI_PHY_H
#define SPECTRUM_WIFI_PHY_H

#include "wifi-phy.h"

#include "ns3/antenna-model.h"

#include <map>
#include <optional>

class SpectrumWifiPhyFilterTest;

namespace ns3 {

class SpectrumChannel;
struct SpectrumSignalParameters;
class WifiSpectrumPhyInterface;
struct WifiSpectrumSignalParameters;

using HeRuBands = std::map<WifiSpectrumBandInfo, HeRu::RuSpec>;

class SpectrumWifiPhy : public WifiPhy {
public:
  friend class ::SpectrumWifiPhyFilterTest;

  static TypeId GetTypeId();

  SpectrumWifiPhy();
  ~SpectrumWifiPhy() override;

  void SetDevice(const Ptr<WifiNetDevice> device) override;
  void StartTx(Ptr<const WifiPpdu> ppdu) override;
  Ptr<Channel> GetChannel() const override;
  uint16_t GetGuardBandwidth(uint16_t currentChannelWidth) const override;
  std::tuple<double, double, double> GetTxMaskRejectionParams() const override;
  WifiSpectrumBandInfo GetBand(uint16_t bandWidth,
                               uint8_t bandIndex = 0) override;
  FrequencyRange GetCurrentFrequencyRange() const override;
  WifiSpectrumBandFrequencies ConvertIndicesToFrequencies(
      const WifiSpectrumBandIndices &indices) const override;

  void AddChannel(const Ptr<SpectrumChannel> channel,
                  const FrequencyRange &freqRange = WHOLE_WIFI_SPECTRUM);

  void StartRx(Ptr<SpectrumSignalParameters> rxParams,
               Ptr<const WifiSpectrumPhyInterface> interface);

  void SetAntenna(const Ptr<AntennaModel> antenna);
  Ptr<AntennaModel> GetAntenna() const;

  typedef void (*SignalArrivalCallback)(bool signalType, uint32_t senderNodeId,
                                        double rxPower, Time duration);

  void ConfigureInterface(uint16_t frequency, uint16_t width);

  void Transmit(Ptr<WifiSpectrumSignalParameters> txParams);

  Ptr<const WifiPpdu> GetRxPpduFromTxPpdu(Ptr<const WifiPpdu> ppdu);

  Ptr<WifiSpectrumPhyInterface> GetCurrentInterface() const;

  const std::map<FrequencyRange, Ptr<WifiSpectrumPhyInterface>> &
  GetSpectrumPhyInterfaces() const;

  void SetChannelSwitchedCallback(Callback<void> callback);

protected:
  void DoDispose() override;
  void DoInitialize() override;

  void DoChannelSwitch() override;

  std::map<FrequencyRange, Ptr<WifiSpectrumPhyInterface>>
      m_spectrumPhyInterfaces;

  Ptr<WifiSpectrumPhyInterface> m_currentSpectrumPhyInterface;

private:
  void ResetSpectrumModel(Ptr<WifiSpectrumPhyInterface> spectrumPhyInterface,
                          uint16_t centerFrequency, uint16_t channelWidth);

  void UpdateInterferenceHelperBands(
      Ptr<WifiSpectrumPhyInterface> spectrumPhyInterface);

  HeRuBands GetHeRuBands(Ptr<WifiSpectrumPhyInterface> spectrumPhyInterface,
                         uint16_t guardBandwidth);

  WifiSpectrumBands
  ComputeBands(Ptr<WifiSpectrumPhyInterface> spectrumPhyInterface);

  WifiSpectrumBandInfo
  GetBandForInterface(Ptr<WifiSpectrumPhyInterface> spectrumPhyInterface,
                      uint16_t bandWidth, uint8_t bandIndex = 0);

  WifiSpectrumBandFrequencies ConvertIndicesToFrequenciesForInterface(
      Ptr<WifiSpectrumPhyInterface> spectrumPhyInterface,
      const WifiSpectrumBandIndices &indices) const;

  bool CanStartRx(Ptr<const WifiPpdu> ppdu) const;

  Ptr<WifiSpectrumPhyInterface>
  GetInterfaceCoveringChannelBand(uint16_t frequency, uint16_t width) const;

  void NotifyChannelSwitched();

  Ptr<AntennaModel> m_antenna;

  bool m_disableWifiReception;
  bool m_trackSignalsInactiveInterfaces;

  TracedCallback<bool, uint32_t, double, Time> m_signalCb;

  double m_txMaskInnerBandMinimumRejection;
  double m_txMaskOuterBandMinimumRejection;
  double m_txMaskOuterBandMaximumRejection;

  Callback<void> m_channelSwitchedCallback;
};

} // namespace ns3

#endif
