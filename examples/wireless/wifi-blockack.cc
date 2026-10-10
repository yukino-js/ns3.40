

#include "ns3/boolean.h"
#include "ns3/command-line.h"
#include "ns3/double.h"
#include "ns3/internet-stack-helper.h"
#include "ns3/ipv4-address-helper.h"
#include "ns3/ipv4-global-routing-helper.h"
#include "ns3/log.h"
#include "ns3/mobility-helper.h"
#include "ns3/mobility-model.h"
#include "ns3/on-off-helper.h"
#include "ns3/rectangle.h"
#include "ns3/ssid.h"
#include "ns3/string.h"
#include "ns3/uinteger.h"
#include "ns3/yans-wifi-channel.h"
#include "ns3/yans-wifi-helper.h"

using namespace ns3;

NS_LOG_COMPONENT_DEFINE("Test-block-ack");

int main(int argc, char *argv[]) {
  CommandLine cmd(__FILE__);
  cmd.Parse(argc, argv);

  LogComponentEnable("QosTxop", LOG_LEVEL_DEBUG);
  LogComponentEnable("BlockAckManager", LOG_LEVEL_INFO);

  Ptr<Node> sta = CreateObject<Node>();
  Ptr<Node> ap = CreateObject<Node>();

  YansWifiChannelHelper channel = YansWifiChannelHelper::Default();
  YansWifiPhyHelper phy;
  phy.SetPcapDataLinkType(WifiPhyHelper::DLT_IEEE802_11_RADIO);
  phy.SetChannel(channel.Create());

  WifiHelper wifi;
  wifi.SetStandard(WIFI_STANDARD_80211n);
  WifiMacHelper mac;
  wifi.SetRemoteStationManager("ns3::IdealWifiManager",
                               "FragmentationThreshold", UintegerValue(2500));

  Ssid ssid("My-network");

  mac.SetType("ns3::StaWifiMac", "QosSupported", BooleanValue(true), "Ssid",
              SsidValue(ssid), "BE_BlockAckThreshold", UintegerValue(2),
              "BE_BlockAckInactivityTimeout", UintegerValue(3));
  NetDeviceContainer staDevice = wifi.Install(phy, mac, sta);

  mac.SetType("ns3::ApWifiMac", "QosSupported", BooleanValue(true), "Ssid",
              SsidValue(ssid), "BE_BlockAckThreshold", UintegerValue(0));
  NetDeviceContainer apDevice = wifi.Install(phy, mac, ap);

  MobilityHelper mobility;

  mobility.SetPositionAllocator(
      "ns3::GridPositionAllocator", "MinX", DoubleValue(0.0), "MinY",
      DoubleValue(0.0), "DeltaX", DoubleValue(5.0), "DeltaY", DoubleValue(10.0),
      "GridWidth", UintegerValue(3), "LayoutType", StringValue("RowFirst"));

  mobility.SetMobilityModel("ns3::RandomWalk2dMobilityModel", "Bounds",
                            RectangleValue(Rectangle(-50, 50, -50, 50)));
  mobility.Install(sta);

  mobility.SetMobilityModel("ns3::ConstantPositionMobilityModel");
  mobility.Install(ap);

  InternetStackHelper stack;
  stack.Install(sta);
  stack.Install(ap);

  Ipv4AddressHelper address;

  address.SetBase("192.168.1.0", "255.255.255.0");
  Ipv4InterfaceContainer staIf;
  Ipv4InterfaceContainer apIf;
  staIf = address.Assign(staDevice);
  apIf = address.Assign(apDevice);

  uint16_t port = 9;
  DataRate dataRate("1Mb/s");
  OnOffHelper onOff("ns3::UdpSocketFactory",
                    Address(InetSocketAddress(apIf.GetAddress(0), port)));
  onOff.SetAttribute("DataRate", DataRateValue(dataRate));
  onOff.SetAttribute("OnTime",
                     StringValue("ns3::ConstantRandomVariable[Constant=0.01]"));
  onOff.SetAttribute("OffTime",
                     StringValue("ns3::ConstantRandomVariable[Constant=8]"));
  onOff.SetAttribute("PacketSize", UintegerValue(50));

  ApplicationContainer staApps = onOff.Install(sta);

  staApps.Start(Seconds(1.0));
  staApps.Stop(Seconds(10.0));

  Ipv4GlobalRoutingHelper::PopulateRoutingTables();

  Simulator::Stop(Seconds(10.0));

  phy.EnablePcap("test-blockack", ap->GetId(), 0);
  Simulator::Run();
  Simulator::Destroy();

  return 0;
}
