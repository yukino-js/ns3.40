
#ifndef WIFI_PHY_OPERATING_CHANNEL_H
#define WIFI_PHY_OPERATING_CHANNEL_H

#include "wifi-phy-band.h"
#include "wifi-standards.h"

#include "ns3/he-ru.h"

#include <set>
#include <tuple>

namespace ns3 {

typedef std::tuple<uint8_t, uint16_t, uint16_t, FrequencyChannelType,
                   WifiPhyBand>
    FrequencyChannelInfo;

class WifiPhyOperatingChannel {
public:
  typedef std::set<FrequencyChannelInfo>::const_iterator ConstIterator;

  WifiPhyOperatingChannel();

  WifiPhyOperatingChannel(ConstIterator it);

  virtual ~WifiPhyOperatingChannel();

  bool operator==(const WifiPhyOperatingChannel &other) const;

  bool operator!=(const WifiPhyOperatingChannel &other) const;

  static const std::set<FrequencyChannelInfo> m_frequencyChannels;

  bool IsSet() const;
  void Set(uint8_t number, uint16_t frequency, uint16_t width,
           WifiStandard standard, WifiPhyBand band);
  void SetDefault(uint16_t width, WifiStandard standard, WifiPhyBand band);

  static uint8_t GetDefaultChannelNumber(uint16_t width, WifiStandard standard,
                                         WifiPhyBand band);

  uint8_t GetNumber() const;
  uint16_t GetFrequency() const;
  uint16_t GetWidth() const;
  WifiPhyBand GetPhyBand() const;
  bool IsOfdm() const;
  bool IsDsss() const;
  bool Is80211p() const;

  uint8_t GetPrimaryChannelIndex(uint16_t primaryChannelWidth) const;

  uint8_t GetSecondaryChannelIndex(uint16_t secondaryChannelWidth) const;

  void SetPrimary20Index(uint8_t index);

  uint16_t GetPrimaryChannelCenterFrequency(uint16_t primaryChannelWidth) const;

  uint16_t
  GetSecondaryChannelCenterFrequency(uint16_t secondaryChannelWidth) const;

  std::set<uint8_t> GetAll20MHzChannelIndicesInPrimary(uint16_t width) const;
  std::set<uint8_t> GetAll20MHzChannelIndicesInSecondary(uint16_t width) const;
  std::set<uint8_t> GetAll20MHzChannelIndicesInSecondary(
      const std::set<uint8_t> &primaryIndices) const;

  static ConstIterator
  FindFirst(uint8_t number, uint16_t frequency, uint16_t width,
            WifiStandard standard, WifiPhyBand band,
            ConstIterator start = m_frequencyChannels.begin());

  uint8_t GetPrimaryChannelNumber(uint16_t primaryChannelWidth,
                                  WifiStandard standard) const;

  std::set<uint8_t> Get20MHzIndicesCoveringRu(HeRu::RuSpec ru,
                                              uint16_t width) const;

private:
  ConstIterator m_channelIt;
  uint8_t m_primary20Index;
};

std::ostream &operator<<(std::ostream &os,
                         const WifiPhyOperatingChannel &channel);

} // namespace ns3

#endif
