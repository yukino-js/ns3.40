

#include "ns3/applications-module.h"
#include "ns3/click-internet-stack-helper.h"
#include "ns3/core-module.h"
#include "ns3/internet-module.h"
#include "ns3/log.h"
#include "ns3/mobility-helper.h"
#include "ns3/network-module.h"
#include "ns3/wifi-module.h"

using namespace ns3;

void ReceivePacket(Ptr<Socket> socket) {
  NS_LOG_UNCOND("Received one packet!");
}

int main(int argc, char *argv[]) {
  double rss = -80;
  std::string clickConfigFolder = "src/click/examples";

  CommandLine cmd(__FILE__);
  cmd.AddValue("clickConfigFolder", "Base folder for click configuration files",
               clickConfigFolder);
  cmd.Parse(argc, argv);

  NodeContainer wifiNodes;
  wifiNodes.Create(2);

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
                                 DoubleValue(rss));
  wifiPhy.SetChannel(wifiChannel.Create());

  WifiMacHelper wifiMac;
  wifi.SetRemoteStationManager("ns3::ConstantRateWifiManager", "DataMode",
                               StringValue(phyMode), "ControlMode",
                               StringValue(phyMode));
  wifiMac.SetType("ns3::AdhocWifiMac");
  NetDeviceContainer wifiDevices = wifi.Install(wifiPhy, wifiMac, wifiNodes);

  MobilityHelper mobility;
  Ptr<ListPositionAllocator> positionAlloc =
      CreateObject<ListPositionAllocator>();
  positionAlloc->Add(Vector(0.0, 0.0, 0.0));
  positionAlloc->Add(Vector(5.0, 0.0, 0.0));
  mobility.SetPositionAllocator(positionAlloc);
  mobility.SetMobilityModel("ns3::ConstantPositionMobilityModel");
  mobility.Install(wifiNodes);

  InternetStackHelper internet;
  internet.Install(wifiNodes.Get(1));

  ClickInternetStackHelper clickinternet;
  clickinternet.SetClickFile(wifiNodes.Get(0),
                             clickConfigFolder +
                                 "/nsclick-wifi-single-interface.click");
  clickinternet.SetRoutingTableElement(wifiNodes.Get(0), "rt");
  clickinternet.Install(wifiNodes.Get(0));

  Ipv4AddressHelper ipv4;
  ipv4.SetBase("172.16.1.0", "255.255.255.0");
  ipv4.Assign(wifiDevices);

  Address LocalAddress(InetSocketAddress(Ipv4Address::GetAny(), 50000));
  PacketSinkHelper packetSinkHelper("ns3::TcpSocketFactory", LocalAddress);
  ApplicationContainer recvapp = packetSinkHelper.Install(wifiNodes.Get(1));
  recvapp.Start(Seconds(5.0));
  recvapp.Stop(Seconds(10.0));

  OnOffHelper onOffHelper("ns3::TcpSocketFactory", Address());
  onOffHelper.SetAttribute(
      "OnTime", StringValue("ns3::ConstantRandomVariable[Constant=1]"));
  onOffHelper.SetAttribute(
      "OffTime", StringValue("ns3::ConstantRandomVariable[Constant=0]"));

  ApplicationContainer appcont;

  AddressValue remoteAddress(
      InetSocketAddress(Ipv4Address("172.16.1.2"), 50000));
  onOffHelper.SetAttribute("Remote", remoteAddress);
  appcont.Add(onOffHelper.Install(wifiNodes.Get(0)));

  appcont.Start(Seconds(5.0));
  appcont.Stop(Seconds(10.0));

  wifiPhy.EnablePcap("nsclick-raw-wlan", wifiDevices);

  Simulator::Stop(Seconds(20.0));
  Simulator::Run();

  Simulator::Destroy();

  return 0;
}
