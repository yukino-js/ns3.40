
#ifndef WIFI_PHY_STATE_H
#define WIFI_PHY_STATE_H

#include "ns3/fatal-error.h"

enum WifiPhyState { IDLE, CCA_BUSY, TX, RX, SWITCHING, SLEEP, OFF };

inline std::ostream &operator<<(std::ostream &os, WifiPhyState state) {
  switch (state) {
  case IDLE:
    return (os << "IDLE");
  case CCA_BUSY:
    return (os << "CCA_BUSY");
  case TX:
    return (os << "TX");
  case RX:
    return (os << "RX");
  case SWITCHING:
    return (os << "SWITCHING");
  case SLEEP:
    return (os << "SLEEP");
  case OFF:
    return (os << "OFF");
  default:
    NS_FATAL_ERROR("Invalid state");
    return (os << "INVALID");
  }
}

#endif
