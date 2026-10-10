
#ifndef ADHOC_WIFI_MAC_H
#define ADHOC_WIFI_MAC_H

#include "wifi-mac.h"

namespace ns3 {

class AdhocWifiMac : public WifiMac {
public:
  static TypeId GetTypeId();

  AdhocWifiMac();
  ~AdhocWifiMac() override;

  void SetLinkUpCallback(Callback<void> linkUp) override;
  void Enqueue(Ptr<Packet> packet, Mac48Address to) override;
  bool CanForwardPacketsTo(Mac48Address to) const override;

private:
  void Receive(Ptr<const WifiMpdu> mpdu, uint8_t linkId) override;
};

} // namespace ns3

#endif
