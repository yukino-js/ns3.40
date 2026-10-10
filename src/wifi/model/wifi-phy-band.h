
#ifndef WIFI_PHY_BAND_H
#define WIFI_PHY_BAND_H

#include <iostream>

namespace ns3 {

enum WifiPhyBand {
  WIFI_PHY_BAND_2_4GHZ,
  WIFI_PHY_BAND_5GHZ,
  WIFI_PHY_BAND_6GHZ,
  WIFI_PHY_BAND_60GHZ,
  WIFI_PHY_BAND_UNSPECIFIED
};

inline std::ostream &operator<<(std::ostream &os, WifiPhyBand band) {
  switch (band) {
  case WIFI_PHY_BAND_2_4GHZ:
    return (os << "2.4GHz");
  case WIFI_PHY_BAND_5GHZ:
    return (os << "5GHz");
  case WIFI_PHY_BAND_6GHZ:
    return (os << "6GHz");
  case WIFI_PHY_BAND_60GHZ:
    return (os << "60GHz");
  default:
    return (os << "INVALID");
  }
}

} // namespace ns3

#endif
