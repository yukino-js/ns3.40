
#include "ns3/boolean.h"
#include "ns3/double.h"
#include "ns3/internet-stack-helper.h"
#include "ns3/ipv4-address-helper.h"
#include "ns3/mobility-helper.h"
#include "ns3/on-off-helper.h"
#include "ns3/packet-sink-helper.h"
#include "ns3/packet-sink.h"
#include "ns3/ssid.h"
#include "ns3/string.h"
#include "ns3/test.h"
#include "ns3/uinteger.h"
#include "ns3/yans-wifi-helper.h"

using namespace ns3;

class WifiMsduAggregatorThroughputTest : public TestCase {
public:
  WifiMsduAggregatorThroughputTest();
  void DoRun() override;

private:
  bool m_writeResults;
};

WifiMsduAggregatorThroughputTest::WifiMsduAggregatorThroughputTest()
    : TestCase("MsduAggregator throughput test"), m_writeResults(false) {}

void WifiMsduAggregatorThroughputTest::DoRun() {
  WifiHelper wifi;
  WifiMacHelper wifiMac;
  YansWifiPhyHelper wifiPhy;
  YansWifiChannelHelper wifiChannel = YansWifiChannelHelper::Default();
  wifiPhy.SetChannel(wifiChannel.Create());

  Ssid ssid = Ssid("wifi-amsdu-throughput");
  std::string phyMode("DsssRate1Mbps");
  wifi.SetStandard(WIFI_STANDARD_80211b);
  wifi.SetRemoteStationManager("ns3::ConstantRateWifiManager", "DataMode",
                               StringValue(phyMode), "ControlMode",
                               StringValue(phyMode));

  NodeContainer ap;
  ap.Create(1);
  wifiMac.SetType("ns3::ApWifiMac", "QosSupported", BooleanValue(true), "Ssid",
                  SsidValue(ssid), "BeaconGeneration", BooleanValue(true),
                  "BeaconInterval", TimeValue(MicroSeconds(102400)),
                  "BE_MaxAmsduSize", UintegerValue(4000));

  NetDeviceContainer apDev = wifi.Install(wifiPhy, wifiMac, ap);

  NodeContainer sta;
  sta.Create(1);
  wifiMac.SetType("ns3::StaWifiMac", "QosSupported", BooleanValue(true), "Ssid",
                  SsidValue(ssid), "ActiveProbing", BooleanValue(false));
  NetDeviceContainer staDev = wifi.Install(wifiPhy, wifiMac, sta);

  MobilityHelper mobility;
  mobility.SetMobilityModel("ns3::ConstantPositionMobilityModel");
  mobility.SetPositionAllocator(
      "ns3::GridPositionAllocator", "MinX", DoubleValue(0.0), "MinY",
      DoubleValue(0.0), "DeltaX", DoubleValue(5.0), "DeltaY", DoubleValue(10.0),
      "GridWidth", UintegerValue(2), "LayoutType", StringValue("RowFirst"));
  mobility.Install(sta);
  mobility.Install(ap);

  InternetStackHelper stack;
  stack.Install(ap);
  stack.Install(sta);

  Ipv4AddressHelper address;
  address.SetBase("192.168.0.0", "255.255.255.0");
  Ipv4InterfaceContainer staNodeInterface;
  Ipv4InterfaceContainer apNodeInterface;
  staNodeInterface = address.Assign(staDev);
  apNodeInterface = address.Assign(apDev);

  uint16_t udpPort = 50000;

  PacketSinkHelper packetSink(
      "ns3::UdpSocketFactory",
      InetSocketAddress(Ipv4Address::GetAny(), udpPort));
  ApplicationContainer sinkApp = packetSink.Install(sta.Get(0));
  sinkApp.Start(Seconds(0));
  sinkApp.Stop(Seconds(9.0));

  OnOffHelper onoff("ns3::UdpSocketFactory",
                    InetSocketAddress(staNodeInterface.GetAddress(0), udpPort));
  onoff.SetAttribute("PacketSize", UintegerValue(100));
  onoff.SetConstantRate(DataRate("1Mbps"));
  ApplicationContainer sourceApp = onoff.Install(ap.Get(0));
  sourceApp.Start(Seconds(1.0));
  sourceApp.Stop(Seconds(9.0));

  if (m_writeResults) {
    wifiPhy.SetPcapDataLinkType(WifiPhyHelper::DLT_IEEE802_11_RADIO);
    wifiPhy.EnablePcap("wifi-amsdu-throughput", sta.Get(0)->GetId(), 0);
  }

  Simulator::Stop(Seconds(10.0));
  Simulator::Run();
  Simulator::Destroy();

  uint32_t totalOctetsThrough =
      DynamicCast<PacketSink>(sinkApp.Get(0))->GetTotalRx();

  NS_TEST_ASSERT_MSG_GT(totalOctetsThrough, 600000,
                        "A-MSDU test fails for low throughput of "
                            << totalOctetsThrough << " octets");
}

class WifiMsduAggregatorTestSuite : public TestSuite {
public:
  WifiMsduAggregatorTestSuite();
};

WifiMsduAggregatorTestSuite::WifiMsduAggregatorTestSuite()
    : TestSuite("wifi-msdu-aggregator", SYSTEM) {
  AddTestCase(new WifiMsduAggregatorThroughputTest, TestCase::QUICK);
}

static WifiMsduAggregatorTestSuite wifiMsduAggregatorTestSuite;
