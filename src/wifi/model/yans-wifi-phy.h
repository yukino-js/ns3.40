
#ifndef YANS_WIFI_PHY_H
#define YANS_WIFI_PHY_H

#include "wifi-phy.h"

namespace ns3 {

class YansWifiChannel;

class YansWifiPhy : public WifiPhy {
public:
  static TypeId GetTypeId();

  YansWifiPhy();
  ~YansWifiPhy() override;

  void SetInterferenceHelper(const Ptr<InterferenceHelper> helper) override;
  void StartTx(Ptr<const WifiPpdu> ppdu) override;
  Ptr<Channel> GetChannel() const override;
  uint16_t GetGuardBandwidth(uint16_t currentChannelWidth) const override;
  std::tuple<double, double, double> GetTxMaskRejectionParams() const override;
  WifiSpectrumBandInfo GetBand(uint16_t bandWidth,
                               uint8_t bandIndex = 0) override;
  FrequencyRange GetCurrentFrequencyRange() const override;
  WifiSpectrumBandFrequencies ConvertIndicesToFrequencies(
      const WifiSpectrumBandIndices &indices) const override;

  void SetChannel(const Ptr<YansWifiChannel> channel);

protected:
  void DoDispose() override;

private:
  Ptr<YansWifiChannel> m_channel;
};

} // namespace ns3

#endif
