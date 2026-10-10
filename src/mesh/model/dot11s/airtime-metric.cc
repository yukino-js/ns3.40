
#include "airtime-metric.h"

#include "ns3/wifi-phy.h"

namespace ns3 {
namespace dot11s {
NS_OBJECT_ENSURE_REGISTERED(AirtimeLinkMetricCalculator);

TypeId AirtimeLinkMetricCalculator::GetTypeId() {
  static TypeId tid =
      TypeId("ns3::dot11s::AirtimeLinkMetricCalculator")
          .SetParent<Object>()
          .SetGroupName("Mesh")
          .AddConstructor<AirtimeLinkMetricCalculator>()
          .AddAttribute(
              "TestLength",
              "Number of bytes in test frame (a constant 1024 in the standard)",
              UintegerValue(1024),
              MakeUintegerAccessor(&AirtimeLinkMetricCalculator::SetTestLength),
              MakeUintegerChecker<uint16_t>(1))
          .AddAttribute(
              "Dot11MetricTid", "TID used to calculate metric (data rate)",
              UintegerValue(0),
              MakeUintegerAccessor(&AirtimeLinkMetricCalculator::SetHeaderTid),
              MakeUintegerChecker<uint8_t>(0));
  return tid;
}

AirtimeLinkMetricCalculator::AirtimeLinkMetricCalculator() {}

void AirtimeLinkMetricCalculator::SetHeaderTid(uint8_t tid) {
  m_testHeader.SetType(WIFI_MAC_DATA);
  m_testHeader.SetDsFrom();
  m_testHeader.SetDsTo();
  m_testHeader.SetQosTid(tid);
}

void AirtimeLinkMetricCalculator::SetTestLength(uint16_t testLength) {
  m_testFrame = Create<Packet>(testLength + 6 + 36);
}

uint32_t
AirtimeLinkMetricCalculator::CalculateMetric(Mac48Address peerAddress,
                                             Ptr<MeshWifiInterfaceMac> mac) {
  NS_ASSERT(!peerAddress.IsGroup());
  WifiMode mode =
      mac->GetWifiRemoteStationManager()
          ->GetDataTxVector(m_testHeader, mac->GetWifiPhy()->GetChannelWidth())
          .GetMode();
  double failAvg = mac->GetWifiRemoteStationManager()
                       ->GetInfo(peerAddress)
                       .GetFrameErrorRate();
  if (failAvg == 1) {
    return (uint32_t)0xffffffff;
  }
  NS_ASSERT(failAvg < 1.0);
  WifiTxVector txVector;
  txVector.SetMode(mode);
  txVector.SetPreambleType(WIFI_PREAMBLE_LONG);
  uint32_t metric = (uint32_t)((double)(2 * mac->GetWifiPhy()->GetSifs() +
                                        2 * mac->GetWifiPhy()->GetSlot() +
                                        mac->GetWifiPhy()->GetAckTxTime() +
                                        mac->GetWifiPhy()->CalculateTxDuration(
                                            m_testFrame->GetSize(), txVector,
                                            mac->GetWifiPhy()->GetPhyBand()))
                                   .GetMicroSeconds() /
                               (10.24 * (1.0 - failAvg)));
  return metric;
}
} // namespace dot11s
} // namespace ns3
