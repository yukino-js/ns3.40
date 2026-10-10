
#ifndef WIFI_SPECTRUM_PHY_INTERFACE_H
#define WIFI_SPECTRUM_PHY_INTERFACE_H

#include "spectrum-wifi-phy.h"

#include "ns3/he-phy.h"
#include "ns3/spectrum-phy.h"

namespace ns3 {

class SpectrumWifiPhy;

class WifiSpectrumPhyInterface : public SpectrumPhy {
public:
  static TypeId GetTypeId();
  WifiSpectrumPhyInterface(FrequencyRange freqRange);
  void SetSpectrumWifiPhy(const Ptr<SpectrumWifiPhy> phy);

  Ptr<const SpectrumWifiPhy> GetSpectrumWifiPhy() const;

  Ptr<NetDevice> GetDevice() const override;
  void SetDevice(const Ptr<NetDevice> d) override;
  void SetMobility(const Ptr<MobilityModel> m) override;
  Ptr<MobilityModel> GetMobility() const override;
  void SetChannel(const Ptr<SpectrumChannel> c) override;
  Ptr<const SpectrumModel> GetRxSpectrumModel() const override;
  Ptr<Object> GetAntenna() const override;
  void StartRx(Ptr<SpectrumSignalParameters> params) override;

  Ptr<SpectrumChannel> GetChannel() const;

  const FrequencyRange &GetFrequencyRange() const;

  uint16_t GetCenterFrequency() const;

  uint16_t GetChannelWidth() const;

  void StartTx(Ptr<SpectrumSignalParameters> params);

  void SetRxSpectrumModel(uint32_t centerFrequency, uint16_t channelWidth,
                          uint32_t bandBandwidth, uint16_t guardBandwidth);

  void SetBands(WifiSpectrumBands &&bands);
  const WifiSpectrumBands &GetBands() const;

  void SetHeRuBands(HeRuBands &&heRuBands);
  const HeRuBands &GetHeRuBands() const;

private:
  void DoDispose() override;

  FrequencyRange m_frequencyRange;
  Ptr<SpectrumWifiPhy> m_spectrumWifiPhy;
  Ptr<NetDevice> m_netDevice;
  Ptr<SpectrumChannel> m_channel;
  uint16_t m_centerFrequency;
  uint16_t m_channelWidth;
  Ptr<const SpectrumModel> m_rxSpectrumModel;

  WifiSpectrumBands m_bands;
  HeRuBands m_heRuBands;
};

} // namespace ns3

#endif
