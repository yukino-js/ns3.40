
#ifndef WIFI_DEFAULT_ASSOC_MANAGER_H
#define WIFI_DEFAULT_ASSOC_MANAGER_H

#include "wifi-assoc-manager.h"

namespace ns3 {

class StaWifiMac;

class WifiDefaultAssocManager : public WifiAssocManager {
public:
  static TypeId GetTypeId();
  WifiDefaultAssocManager();
  ~WifiDefaultAssocManager() override;

  void NotifyChannelSwitched(uint8_t linkId) override;
  bool Compare(const StaWifiMac::ApInfo &lhs,
               const StaWifiMac::ApInfo &rhs) const override;

protected:
  void DoDispose() override;
  bool CanBeInserted(const StaWifiMac::ApInfo &apInfo) const override;
  bool CanBeReturned(const StaWifiMac::ApInfo &apInfo) const override;

  void EndScanning();

private:
  void DoStartScanning() override;

  void ChannelSwitchTimeout(uint8_t linkId);

  EventId m_waitBeaconEvent;
  EventId m_probeRequestEvent;
  Time m_channelSwitchTimeout;

  struct ChannelSwitchInfo {
    EventId timer;
    Mac48Address apLinkAddress;
    Mac48Address apMldAddress;
  };

  std::vector<ChannelSwitchInfo> m_channelSwitchInfo;
};

} // namespace ns3

#endif
