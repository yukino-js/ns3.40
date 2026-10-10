
#ifndef AIRTIME_METRIC_H
#define AIRTIME_METRIC_H

#include "ns3/mesh-wifi-interface-mac.h"
#include "ns3/wifi-mac-header.h"

namespace ns3 {
namespace dot11s {
class AirtimeLinkMetricCalculator : public Object {
public:
  AirtimeLinkMetricCalculator();
  static TypeId GetTypeId();
  uint32_t CalculateMetric(Mac48Address peerAddress,
                           Ptr<MeshWifiInterfaceMac> mac);

private:
  void SetTestLength(uint16_t testLength);
  void SetHeaderTid(uint8_t tid);

  Ptr<Packet> m_testFrame;
  WifiMacHeader m_testHeader;
};
} // namespace dot11s
} // namespace ns3
#endif
