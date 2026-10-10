
#ifndef SPECTRUM_WIFI_HELPER_H
#define SPECTRUM_WIFI_HELPER_H

#include "wifi-helper.h"

#include <map>
#include <set>

namespace ns3 {

class SpectrumChannel;
class SpectrumWifiPhy;

class SpectrumWifiPhyHelper : public WifiPhyHelper {
public:
  SpectrumWifiPhyHelper(uint8_t nLinks = 1);

  void SetChannel(const Ptr<SpectrumChannel> channel);
  void SetChannel(const std::string &channelName);

  void AddChannel(const Ptr<SpectrumChannel> channel,
                  const FrequencyRange &freqRange = WHOLE_WIFI_SPECTRUM);
  void AddChannel(const std::string &channelName,
                  const FrequencyRange &freqRange = WHOLE_WIFI_SPECTRUM);

  void AddPhyToFreqRangeMapping(uint8_t linkId,
                                const FrequencyRange &freqRange);

  void ResetPhyToFreqRangeMapping();

private:
  std::vector<Ptr<WifiPhy>> Create(Ptr<Node> node,
                                   Ptr<WifiNetDevice> device) const override;

  void InstallPhyInterfaces(uint8_t linkId, Ptr<SpectrumWifiPhy> phy) const;

  void SpectrumChannelSwitched(Ptr<SpectrumWifiPhy> phy) const;

  void AddWifiBandwidthFilter(Ptr<SpectrumChannel> channel);

  std::map<FrequencyRange, Ptr<SpectrumChannel>> m_channels;
  std::map<uint8_t, std::set<FrequencyRange>> m_interfacesMap;
};

} // namespace ns3

#endif
