

#include "ns3/applications-module.h"
#include "ns3/click-internet-stack-helper.h"
#include "ns3/core-module.h"
#include "ns3/internet-module.h"
#include "ns3/ipv4-click-routing.h"
#include "ns3/mobility-module.h"
#include "ns3/network-module.h"
#include "ns3/wifi-module.h"

#include <fstream>

using namespace ns3;

NS_LOG_COMPONENT_DEFINE("NsclickUdpClientServerWifi");

void ReadArp(Ptr<Ipv4ClickRouting> clickRouter) {
  NS_LOG_INFO(clickRouter->ReadHandler("wifi/arpquerier", "table"));
  NS_LOG_INFO(clickRouter->ReadHandler("wifi/arpquerier", "stats"));
}

void WriteArp(Ptr<Ipv4ClickRouting> clickRouter) {
  NS_LOG_INFO(clickRouter->WriteHandler("wifi/arpquerier", "insert",
                                        "172.16.1.2 00:00:00:00:00:02"));
}

int main(int argc, char *argv[]) {
  std::string clickConfigFolder = "src/click/examples";

  CommandLine cmd(__FILE__);
  cmd.AddValue("clickConfigFolder", "Base folder for click configuration files",
               clickConfigFolder);
  cmd.Parse(argc, argv);

  LogComponentEnable("NsclickUdpClientServerWifi", LOG_LEVEL_INFO);

  NS_LOG_INFO("Create nodes.");
  NodeContainer n;
  n.Create(4);

  NS_LOG_INFO("Create channels.");
  std::string phyMode("DsssRate1Mbps");

  Config::SetDefault("ns3::WifiRemoteStationManager::FragmentationThreshold",
                     StringValue("2200"));
  Config::SetDefault("ns3::WifiRemoteStationManager::RtsCtsThreshold",
                     StringValue("2200"));
  Config::SetDefault("ns3::WifiRemoteStationManager::NonUnicastMode",
                     StringValue(phyMode));

  WifiHelper wifi;
  wifi.SetStandard(WIFI_STANDARD_80211b);

  YansWifiPhyHelper wifiPhy;
  wifiPhy.Set("RxGain", DoubleValue(0));
  wifiPhy.SetPcapDataLinkType(WifiPhyHelper::DLT_IEEE802_11_RADIO);

  YansWifiChannelHelper wifiChannel;
  wifiChannel.SetPropagationDelay("ns3::ConstantSpeedPropagationDelayModel");
  wifiChannel.AddPropagationLoss("ns3::FixedRssLossModel", "Rss",
                                 DoubleValue(-80));
  wifiPhy.SetChannel(wifiChannel.Create());

  WifiMacHelper wifiMac;
  wifi.SetRemoteStationManager("ns3::ConstantRateWifiManager", "DataMode",
                               StringValue(phyMode), "ControlMode",
                               StringValue(phyMode));
  wifiMac.SetType("ns3::AdhocWifiMac");
  NetDeviceContainer d = wifi.Install(wifiPhy, wifiMac, n);

  MobilityHelper mobility;
  Ptr<ListPositionAllocator> positionAlloc =
      CreateObject<ListPositionAllocator>();
  positionAlloc->Add(Vector(0.0, 0.0, 0.0));
  positionAlloc->Add(Vector(10.0, 0.0, 0.0));
  positionAlloc->Add(Vector(20.0, 0.0, 0.0));
  positionAlloc->Add(Vector(0.0, 10.0, 0.0));
  mobility.SetPositionAllocator(positionAlloc);
  mobility.SetMobilityModel("ns3::ConstantPositionMobilityModel");
  mobility.Install(n);

  ClickInternetStackHelper clickinternet;
  clickinternet.SetClickFile(
      n.Get(0), clickConfigFolder + "/nsclick-wifi-single-interface.click");
  clickinternet.SetClickFile(
      n.Get(1), clickConfigFolder + "/nsclick-wifi-single-interface.click");
  clickinternet.SetClickFile(
      n.Get(2), clickConfigFolder + "/nsclick-wifi-single-interface.click");

  clickinternet.SetClickFile(
      n.Get(3),
      clickConfigFolder + "/nsclick-wifi-single-interface-promisc.click");
  clickinternet.SetRoutingTableElement(n, "rt");
  clickinternet.Install(n);
  Ipv4AddressHelper ipv4;
  NS_LOG_INFO("Assign IP Addresses.");
  ipv4.SetBase("172.16.1.0", "255.255.255.0");
  Ipv4InterfaceContainer i = ipv4.Assign(d);

  NS_LOG_INFO("Create Applications.");
  uint16_t port = 4000;
  UdpServerHelper server(port);
  ApplicationContainer apps = server.Install(n.Get(1));
  apps.Start(Seconds(1.0));
  apps.Stop(Seconds(10.0));

  uint32_t MaxPacketSize = 1024;
  Time interPacketInterval = Seconds(0.5);
  uint32_t maxPacketCount = 320;
  UdpClientHelper client(i.GetAddress(1), port);
  client.SetAttribute("MaxPackets", UintegerValue(maxPacketCount));
  client.SetAttribute("Interval", TimeValue(interPacketInterval));
  client.SetAttribute("PacketSize", UintegerValue(MaxPacketSize));
  apps = client.Install(NodeContainer(n.Get(0), n.Get(2)));
  apps.Start(Seconds(2.0));
  apps.Stop(Seconds(10.0));

  wifiPhy.EnablePcap("nsclick-udp-client-server-wifi", d);

  Simulator::Schedule(Seconds(0.5), &ReadArp,
                      n.Get(2)->GetObject<Ipv4ClickRouting>());
  Simulator::Schedule(Seconds(0.6), &WriteArp,
                      n.Get(2)->GetObject<Ipv4ClickRouting>());
  Simulator::Schedule(Seconds(0.7), &ReadArp,
                      n.Get(2)->GetObject<Ipv4ClickRouting>());

  NS_LOG_INFO("Run Simulation.");
  Simulator::Stop(Seconds(20.0));
  Simulator::Run();
  Simulator::Destroy();
  NS_LOG_INFO("Done.");

  return 0;
}
