
#include "ns3/command-line.h"
#include "ns3/config.h"
#include "ns3/double.h"
#include "ns3/internet-stack-helper.h"
#include "ns3/ipv4-address-helper.h"
#include "ns3/ipv4-global-routing-helper.h"
#include "ns3/log.h"
#include "ns3/mobility-helper.h"
#include "ns3/mobility-model.h"
#include "ns3/ssid.h"
#include "ns3/string.h"
#include "ns3/udp-client-server-helper.h"
#include "ns3/uinteger.h"
#include "ns3/yans-wifi-channel.h"
#include "ns3/yans-wifi-helper.h"

using namespace ns3;

NS_LOG_COMPONENT_DEFINE("wifi-timing-attributes");

int main(int argc, char *argv[]) {
  uint32_t slot = 9;
  uint32_t sifs = 10;
  uint32_t pifs = 19;
  double simulationTime = 10;

  CommandLine cmd(__FILE__);
  cmd.AddValue("slot", "Slot time in microseconds", slot);
  cmd.AddValue("sifs", "SIFS duration in microseconds", sifs);
  cmd.AddValue("pifs", "PIFS duration in microseconds", pifs);
  cmd.AddValue("simulationTime", "Simulation time in seconds", simulationTime);
  cmd.Parse(argc, argv);

  Config::SetDefault("ns3::LogDistancePropagationLossModel::ReferenceLoss",
                     DoubleValue(40.046));

  NodeContainer wifiStaNode;
  wifiStaNode.Create(1);
  NodeContainer wifiApNode;
  wifiApNode.Create(1);

  YansWifiChannelHelper channel = YansWifiChannelHelper::Default();
  YansWifiPhyHelper phy;
  phy.SetChannel(channel.Create());

  WifiHelper wifi;
  wifi.SetStandard(WIFI_STANDARD_80211n);
  wifi.SetRemoteStationManager("ns3::ConstantRateWifiManager", "DataMode",
                               StringValue("HtMcs7"), "ControlMode",
                               StringValue("HtMcs0"));
  WifiMacHelper mac;

  Ssid ssid = Ssid("ns3-wifi");
  mac.SetType("ns3::StaWifiMac", "Ssid", SsidValue(ssid));

  NetDeviceContainer staDevice;
  staDevice = wifi.Install(phy, mac, wifiStaNode);

  mac.SetType("ns3::ApWifiMac", "Ssid", SsidValue(ssid));

  NetDeviceContainer apDevice;
  apDevice = wifi.Install(phy, mac, wifiApNode);

  Config::Set("/NodeList/*/DeviceList/*/$ns3::WifiNetDevice/Phy/Slot",
              TimeValue(MicroSeconds(slot)));
  Config::Set("/NodeList/*/DeviceList/*/$ns3::WifiNetDevice/Phy/Sifs",
              TimeValue(MicroSeconds(sifs)));
  Config::Set("/NodeList/*/DeviceList/*/$ns3::WifiNetDevice/Phy/Pifs",
              TimeValue(MicroSeconds(pifs)));

  MobilityHelper mobility;
  Ptr<ListPositionAllocator> positionAlloc =
      CreateObject<ListPositionAllocator>();

  positionAlloc->Add(Vector(0.0, 0.0, 0.0));
  positionAlloc->Add(Vector(1.0, 0.0, 0.0));
  mobility.SetPositionAllocator(positionAlloc);

  mobility.SetMobilityModel("ns3::ConstantPositionMobilityModel");

  mobility.Install(wifiApNode);
  mobility.Install(wifiStaNode);

  InternetStackHelper stack;
  stack.Install(wifiApNode);
  stack.Install(wifiStaNode);

  Ipv4AddressHelper address;

  address.SetBase("192.168.1.0", "255.255.255.0");
  Ipv4InterfaceContainer staNodeInterface;
  Ipv4InterfaceContainer apNodeInterface;

  staNodeInterface = address.Assign(staDevice);
  apNodeInterface = address.Assign(apDevice);

  uint16_t port = 9;
  UdpServerHelper server(port);
  ApplicationContainer serverApp = server.Install(wifiStaNode.Get(0));
  serverApp.Start(Seconds(0.0));
  serverApp.Stop(Seconds(simulationTime + 1));

  UdpClientHelper client(staNodeInterface.GetAddress(0), port);
  client.SetAttribute("MaxPackets", UintegerValue(4294967295U));
  client.SetAttribute("Interval", TimeValue(Time("0.0001")));
  client.SetAttribute("PacketSize", UintegerValue(1472));

  ApplicationContainer clientApp = client.Install(wifiApNode.Get(0));
  clientApp.Start(Seconds(1.0));
  clientApp.Stop(Seconds(simulationTime + 1));

  Ipv4GlobalRoutingHelper::PopulateRoutingTables();

  Simulator::Stop(Seconds(simulationTime + 1));
  Simulator::Run();

  uint64_t totalPacketsThrough =
      DynamicCast<UdpServer>(serverApp.Get(0))->GetReceived();
  double throughput =
      totalPacketsThrough * 1472 * 8 / (simulationTime * 1000000.0);
  std::cout << "Throughput: " << throughput << " Mbit/s" << std::endl;

  Simulator::Destroy();
  return 0;
}
