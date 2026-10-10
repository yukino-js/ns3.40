
#ifndef WIFI_STANDARD_H
#define WIFI_STANDARD_H

#include "wifi-phy-band.h"

#include "ns3/abort.h"

#include <list>
#include <map>

namespace ns3 {

enum WifiStandard {
  WIFI_STANDARD_UNSPECIFIED,
  WIFI_STANDARD_80211a,
  WIFI_STANDARD_80211b,
  WIFI_STANDARD_80211g,
  WIFI_STANDARD_80211p,
  WIFI_STANDARD_80211n,
  WIFI_STANDARD_80211ac,
  WIFI_STANDARD_80211ad,
  WIFI_STANDARD_80211ax,
  WIFI_STANDARD_80211be
};

inline std::ostream &operator<<(std::ostream &os, WifiStandard standard) {
  switch (standard) {
  case WIFI_STANDARD_80211a:
    return (os << "802.11a");
  case WIFI_STANDARD_80211b:
    return (os << "802.11b");
  case WIFI_STANDARD_80211g:
    return (os << "802.11g");
  case WIFI_STANDARD_80211p:
    return (os << "802.11p");
  case WIFI_STANDARD_80211n:
    return (os << "802.11n");
  case WIFI_STANDARD_80211ac:
    return (os << "802.11ac");
  case WIFI_STANDARD_80211ad:
    return (os << "802.11ad");
  case WIFI_STANDARD_80211ax:
    return (os << "802.11ax");
  case WIFI_STANDARD_80211be:
    return (os << "802.11be");
  default:
    return (os << "UNSPECIFIED");
  }
}

const std::map<WifiStandard, std::list<WifiPhyBand>> wifiStandards = {
    {WIFI_STANDARD_80211a, {WIFI_PHY_BAND_5GHZ}},
    {WIFI_STANDARD_80211b, {WIFI_PHY_BAND_2_4GHZ}},
    {WIFI_STANDARD_80211g, {WIFI_PHY_BAND_2_4GHZ}},
    {WIFI_STANDARD_80211p, {WIFI_PHY_BAND_5GHZ}},
    {WIFI_STANDARD_80211n, {WIFI_PHY_BAND_2_4GHZ, WIFI_PHY_BAND_5GHZ}},
    {WIFI_STANDARD_80211ac, {WIFI_PHY_BAND_5GHZ}},
    {WIFI_STANDARD_80211ad, {WIFI_PHY_BAND_60GHZ}},
    {WIFI_STANDARD_80211ax,
     {WIFI_PHY_BAND_2_4GHZ, WIFI_PHY_BAND_5GHZ, WIFI_PHY_BAND_6GHZ}},
    {WIFI_STANDARD_80211be,
     {WIFI_PHY_BAND_2_4GHZ, WIFI_PHY_BAND_5GHZ, WIFI_PHY_BAND_6GHZ}},
};

enum FrequencyChannelType : uint8_t {
  WIFI_PHY_DSSS_CHANNEL = 0,
  WIFI_PHY_OFDM_CHANNEL,
  WIFI_PHY_80211p_CHANNEL
};

inline FrequencyChannelType GetFrequencyChannelType(WifiStandard standard) {
  switch (standard) {
  case WIFI_STANDARD_80211b:
    return WIFI_PHY_DSSS_CHANNEL;
  case WIFI_STANDARD_80211p:
    return WIFI_PHY_80211p_CHANNEL;
  default:
    return WIFI_PHY_OFDM_CHANNEL;
  }
}

inline uint16_t GetDefaultChannelWidth(WifiStandard standard,
                                       WifiPhyBand band) {
  switch (standard) {
  case WIFI_STANDARD_80211b:
    return 22;
  case WIFI_STANDARD_80211p:
    return 10;
  case WIFI_STANDARD_80211ac:
    return 80;
  case WIFI_STANDARD_80211ad:
    return 2160;
  case WIFI_STANDARD_80211ax:
  case WIFI_STANDARD_80211be:
    return (band == WIFI_PHY_BAND_2_4GHZ ? 20 : 80);
  default:
    return 20;
  }
}

inline WifiPhyBand GetDefaultPhyBand(WifiStandard standard) {
  switch (standard) {
  case WIFI_STANDARD_80211p:
  case WIFI_STANDARD_80211a:
  case WIFI_STANDARD_80211ac:
  case WIFI_STANDARD_80211ax:
  case WIFI_STANDARD_80211be:
    return WIFI_PHY_BAND_5GHZ;
  case WIFI_STANDARD_80211ad:
    return WIFI_PHY_BAND_60GHZ;
  default:
    return WIFI_PHY_BAND_2_4GHZ;
  }
}

} // namespace ns3

#endif
