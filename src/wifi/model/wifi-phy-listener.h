
#ifndef WIFI_PHY_LISTENER_H
#define WIFI_PHY_LISTENER_H

#include "wifi-phy-common.h"

#include <vector>

namespace ns3 {

class WifiPhyListener {
public:
  virtual ~WifiPhyListener() {}

  virtual void NotifyRxStart(Time duration) = 0;
  virtual void NotifyRxEndOk() = 0;
  virtual void NotifyRxEndError() = 0;
  virtual void NotifyTxStart(Time duration, double txPowerDbm) = 0;
  virtual void
  NotifyCcaBusyStart(Time duration, WifiChannelListType channelType,
                     const std::vector<Time> &per20MhzDurations) = 0;
  virtual void NotifySwitchingStart(Time duration) = 0;
  virtual void NotifySleep() = 0;
  virtual void NotifyOff() = 0;
  virtual void NotifyWakeup() = 0;
  virtual void NotifyOn() = 0;
};

} // namespace ns3

#endif
