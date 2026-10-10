
#include "ns3/boolean.h"
#include "ns3/command-line.h"
#include "ns3/config.h"
#include "ns3/internet-stack-helper.h"
#include "ns3/ipv4-address-helper.h"
#include "ns3/log.h"
#include "ns3/mobility-helper.h"
#include "ns3/packet-sink-helper.h"
#include "ns3/ssid.h"
#include "ns3/string.h"
#include "ns3/udp-client-server-helper.h"
#include "ns3/uinteger.h"
#include "ns3/wifi-mac.h"
#include "ns3/wifi-net-device.h"
#include "ns3/yans-wifi-channel.h"
#include "ns3/yans-wifi-helper.h"

using namespace ns3;

NS_LOG_COMPONENT_DEFINE("SimpleMpduAggregation");

int main(int argc, char *argv[]) {
  uint32_t payloadSize = 1472;
  double simulationTime = 10;
  double distance = 5;
  bool enableRts = false;
  bool enablePcap = false;
  bool verifyResults = false;

  CommandLine cmd(__FILE__);
  cmd.AddValue("payloadSize", "Payload size in bytes", payloadSize);
  cmd.AddValue("enableRts", "Enable or disable RTS/CTS", enableRts);
  cmd.AddValue("simulationTime", "Simulation time in seconds", simulationTime);
  cmd.AddValue("distance",
               "Distance in meters between the station and the access point",
               distance);
  cmd.AddValue("enablePcap", "Enable/disable pcap file generation", enablePcap);
  cmd.AddValue(
      "verifyResults",
      "Enable/disable results verification at the end of the simulation",
      verifyResults);
  cmd.Parse(argc, argv);

  Config::SetDefault("ns3::WifiRemoteStationManager::RtsCtsThreshold",
                     enableRts ? StringValue("0") : StringValue("999999"));

  NodeContainer wifiStaNodes;
  wifiStaNodes.Create(4);
  NodeContainer wifiApNodes;
  wifiApNodes.Create(4);

  YansWifiChannelHelper channel = YansWifiChannelHelper::Default();
  YansWifiPhyHelper phy;
  phy.SetPcapDataLinkType(WifiPhyHelper::DLT_IEEE802_11_RADIO);
  phy.SetChannel(channel.Create());

  WifiHelper wifi;
  wifi.SetStandard(WIFI_STANDARD_80211n);
  wifi.SetRemoteStationManager("ns3::ConstantRateWifiManager", "DataMode",
                               StringValue("HtMcs7"), "ControlMode",
                               StringValue("HtMcs0"));
  WifiMacHelper mac;

  NetDeviceContainer staDeviceA;
  NetDeviceContainer staDeviceB;
  NetDeviceContainer staDeviceC;
  NetDeviceContainer staDeviceD;
  NetDeviceContainer apDeviceA;
  NetDeviceContainer apDeviceB;
  NetDeviceContainer apDeviceC;
  NetDeviceContainer apDeviceD;
  Ssid ssid;

  ssid = Ssid("network-A");
  phy.Set("ChannelSettings", StringValue("{36, 0, BAND_5GHZ, 0}"));
  mac.SetType("ns3::StaWifiMac", "Ssid", SsidValue(ssid));
  staDeviceA = wifi.Install(phy, mac, wifiStaNodes.Get(0));

  mac.SetType("ns3::ApWifiMac", "Ssid", SsidValue(ssid), "EnableBeaconJitter",
              BooleanValue(false));
  apDeviceA = wifi.Install(phy, mac, wifiApNodes.Get(0));

  ssid = Ssid("network-B");
  phy.Set("ChannelSettings", StringValue("{40, 0, BAND_5GHZ, 0}"));
  mac.SetType("ns3::StaWifiMac", "Ssid", SsidValue(ssid));

  staDeviceB = wifi.Install(phy, mac, wifiStaNodes.Get(1));

  Ptr<NetDevice> dev = wifiStaNodes.Get(1)->GetDevice(0);
  Ptr<WifiNetDevice> wifi_dev = DynamicCast<WifiNetDevice>(dev);
  wifi_dev->GetMac()->SetAttribute("BE_MaxAmpduSize", UintegerValue(0));

  mac.SetType("ns3::ApWifiMac", "Ssid", SsidValue(ssid), "EnableBeaconJitter",
              BooleanValue(false));
  apDeviceB = wifi.Install(phy, mac, wifiApNodes.Get(1));

  dev = wifiApNodes.Get(1)->GetDevice(0);
  wifi_dev = DynamicCast<WifiNetDevice>(dev);
  wifi_dev->GetMac()->SetAttribute("BE_MaxAmpduSize", UintegerValue(0));

  ssid = Ssid("network-C");
  phy.Set("ChannelSettings", StringValue("{44, 0, BAND_5GHZ, 0}"));
  mac.SetType("ns3::StaWifiMac", "Ssid", SsidValue(ssid));

  staDeviceC = wifi.Install(phy, mac, wifiStaNodes.Get(2));

  dev = wifiStaNodes.Get(2)->GetDevice(0);
  wifi_dev = DynamicCast<WifiNetDevice>(dev);
  wifi_dev->GetMac()->SetAttribute("BE_MaxAmpduSize", UintegerValue(0));
  wifi_dev->GetMac()->SetAttribute("BE_MaxAmsduSize", UintegerValue(7935));

  mac.SetType("ns3::ApWifiMac", "Ssid", SsidValue(ssid), "EnableBeaconJitter",
              BooleanValue(false));
  apDeviceC = wifi.Install(phy, mac, wifiApNodes.Get(2));

  dev = wifiApNodes.Get(2)->GetDevice(0);
  wifi_dev = DynamicCast<WifiNetDevice>(dev);
  wifi_dev->GetMac()->SetAttribute("BE_MaxAmpduSize", UintegerValue(0));
  wifi_dev->GetMac()->SetAttribute("BE_MaxAmsduSize", UintegerValue(7935));

  ssid = Ssid("network-D");
  phy.Set("ChannelSettings", StringValue("{48, 0, BAND_5GHZ, 0}"));
  mac.SetType("ns3::StaWifiMac", "Ssid", SsidValue(ssid));

  staDeviceD = wifi.Install(phy, mac, wifiStaNodes.Get(3));

  dev = wifiStaNodes.Get(3)->GetDevice(0);
  wifi_dev = DynamicCast<WifiNetDevice>(dev);
  wifi_dev->GetMac()->SetAttribute("BE_MaxAmpduSize", UintegerValue(32768));
  wifi_dev->GetMac()->SetAttribute("BE_MaxAmsduSize", UintegerValue(3839));

  mac.SetType("ns3::ApWifiMac", "Ssid", SsidValue(ssid), "EnableBeaconJitter",
              BooleanValue(false));
  apDeviceD = wifi.Install(phy, mac, wifiApNodes.Get(3));

  dev = wifiApNodes.Get(3)->GetDevice(0);
  wifi_dev = DynamicCast<WifiNetDevice>(dev);
  wifi_dev->GetMac()->SetAttribute("BE_MaxAmpduSize", UintegerValue(32768));
  wifi_dev->GetMac()->SetAttribute("BE_MaxAmsduSize", UintegerValue(3839));

  MobilityHelper mobility;
  Ptr<ListPositionAllocator> positionAlloc =
      CreateObject<ListPositionAllocator>();
  mobility.SetMobilityModel("ns3::ConstantPositionMobilityModel");

  positionAlloc->Add(Vector(0.0, 0.0, 0.0));
  positionAlloc->Add(Vector(10.0, 0.0, 0.0));
  positionAlloc->Add(Vector(20.0, 0.0, 0.0));
  positionAlloc->Add(Vector(30.0, 0.0, 0.0));
  positionAlloc->Add(Vector(distance, 0.0, 0.0));
  positionAlloc->Add(Vector(10 + distance, 0.0, 0.0));
  positionAlloc->Add(Vector(20 + distance, 0.0, 0.0));
  positionAlloc->Add(Vector(30 + distance, 0.0, 0.0));

  mobility.SetPositionAllocator(positionAlloc);
  mobility.Install(wifiApNodes);
  mobility.Install(wifiStaNodes);

  InternetStackHelper stack;
  stack.Install(wifiApNodes);
  stack.Install(wifiStaNodes);

  Ipv4AddressHelper address;
  address.SetBase("192.168.1.0", "255.255.255.0");
  Ipv4InterfaceContainer StaInterfaceA;
  StaInterfaceA = address.Assign(staDeviceA);
  Ipv4InterfaceContainer ApInterfaceA;
  ApInterfaceA = address.Assign(apDeviceA);

  address.SetBase("192.168.2.0", "255.255.255.0");
  Ipv4InterfaceContainer StaInterfaceB;
  StaInterfaceB = address.Assign(staDeviceB);
  Ipv4InterfaceContainer ApInterfaceB;
  ApInterfaceB = address.Assign(apDeviceB);

  address.SetBase("192.168.3.0", "255.255.255.0");
  Ipv4InterfaceContainer StaInterfaceC;
  StaInterfaceC = address.Assign(staDeviceC);
  Ipv4InterfaceContainer ApInterfaceC;
  ApInterfaceC = address.Assign(apDeviceC);

  address.SetBase("192.168.4.0", "255.255.255.0");
  Ipv4InterfaceContainer StaInterfaceD;
  StaInterfaceD = address.Assign(staDeviceD);
  Ipv4InterfaceContainer ApInterfaceD;
  ApInterfaceD = address.Assign(apDeviceD);

  uint16_t port = 9;
  UdpServerHelper serverA(port);
  ApplicationContainer serverAppA = serverA.Install(wifiStaNodes.Get(0));
  serverAppA.Start(Seconds(0.0));
  serverAppA.Stop(Seconds(simulationTime + 1));

  UdpClientHelper clientA(StaInterfaceA.GetAddress(0), port);
  clientA.SetAttribute("MaxPackets", UintegerValue(4294967295U));
  clientA.SetAttribute("Interval", TimeValue(Time("0.0001")));
  clientA.SetAttribute("PacketSize", UintegerValue(payloadSize));

  ApplicationContainer clientAppA = clientA.Install(wifiApNodes.Get(0));
  clientAppA.Start(Seconds(1.0));
  clientAppA.Stop(Seconds(simulationTime + 1));

  UdpServerHelper serverB(port);
  ApplicationContainer serverAppB = serverB.Install(wifiStaNodes.Get(1));
  serverAppB.Start(Seconds(0.0));
  serverAppB.Stop(Seconds(simulationTime + 1));

  UdpClientHelper clientB(StaInterfaceB.GetAddress(0), port);
  clientB.SetAttribute("MaxPackets", UintegerValue(4294967295U));
  clientB.SetAttribute("Interval", TimeValue(Time("0.0001")));
  clientB.SetAttribute("PacketSize", UintegerValue(payloadSize));

  ApplicationContainer clientAppB = clientB.Install(wifiApNodes.Get(1));
  clientAppB.Start(Seconds(1.0));
  clientAppB.Stop(Seconds(simulationTime + 1));

  UdpServerHelper serverC(port);
  ApplicationContainer serverAppC = serverC.Install(wifiStaNodes.Get(2));
  serverAppC.Start(Seconds(0.0));
  serverAppC.Stop(Seconds(simulationTime + 1));

  UdpClientHelper clientC(StaInterfaceC.GetAddress(0), port);
  clientC.SetAttribute("MaxPackets", UintegerValue(4294967295U));
  clientC.SetAttribute("Interval", TimeValue(Time("0.0001")));
  clientC.SetAttribute("PacketSize", UintegerValue(payloadSize));

  ApplicationContainer clientAppC = clientC.Install(wifiApNodes.Get(2));
  clientAppC.Start(Seconds(1.0));
  clientAppC.Stop(Seconds(simulationTime + 1));

  UdpServerHelper serverD(port);
  ApplicationContainer serverAppD = serverD.Install(wifiStaNodes.Get(3));
  serverAppD.Start(Seconds(0.0));
  serverAppD.Stop(Seconds(simulationTime + 1));

  UdpClientHelper clientD(StaInterfaceD.GetAddress(0), port);
  clientD.SetAttribute("MaxPackets", UintegerValue(4294967295U));
  clientD.SetAttribute("Interval", TimeValue(Time("0.0001")));
  clientD.SetAttribute("PacketSize", UintegerValue(payloadSize));

  ApplicationContainer clientAppD = clientD.Install(wifiApNodes.Get(3));
  clientAppD.Start(Seconds(1.0));
  clientAppD.Stop(Seconds(simulationTime + 1));

  if (enablePcap) {
    phy.EnablePcap("AP_A", apDeviceA.Get(0));
    phy.EnablePcap("STA_A", staDeviceA.Get(0));
    phy.EnablePcap("AP_B", apDeviceB.Get(0));
    phy.EnablePcap("STA_B", staDeviceB.Get(0));
    phy.EnablePcap("AP_C", apDeviceC.Get(0));
    phy.EnablePcap("STA_C", staDeviceC.Get(0));
    phy.EnablePcap("AP_D", apDeviceD.Get(0));
    phy.EnablePcap("STA_D", staDeviceD.Get(0));
  }

  Simulator::Stop(Seconds(simulationTime + 1));
  Simulator::Run();

  uint64_t totalPacketsThroughA =
      DynamicCast<UdpServer>(serverAppA.Get(0))->GetReceived();
  uint64_t totalPacketsThroughB =
      DynamicCast<UdpServer>(serverAppB.Get(0))->GetReceived();
  uint64_t totalPacketsThroughC =
      DynamicCast<UdpServer>(serverAppC.Get(0))->GetReceived();
  uint64_t totalPacketsThroughD =
      DynamicCast<UdpServer>(serverAppD.Get(0))->GetReceived();

  Simulator::Destroy();

  double throughput =
      totalPacketsThroughA * payloadSize * 8 / (simulationTime * 1000000.0);
  std::cout << "Throughput with default configuration (A-MPDU aggregation "
               "enabled, 65kB): "
            << throughput << " Mbit/s" << '\n';
  if (verifyResults && (throughput < 59.0 || throughput > 60.0)) {
    NS_LOG_ERROR("Obtained throughput "
                 << throughput << " is not in the expected boundaries!");
    exit(1);
  }

  throughput =
      totalPacketsThroughB * payloadSize * 8 / (simulationTime * 1000000.0);
  std::cout << "Throughput with aggregation disabled: " << throughput
            << " Mbit/s" << '\n';
  if (verifyResults && (throughput < 30 || throughput > 31)) {
    NS_LOG_ERROR("Obtained throughput "
                 << throughput << " is not in the expected boundaries!");
    exit(1);
  }

  throughput =
      totalPacketsThroughC * payloadSize * 8 / (simulationTime * 1000000.0);
  std::cout << "Throughput with A-MPDU disabled and A-MSDU enabled (8kB): "
            << throughput << " Mbit/s" << '\n';
  if (verifyResults && (throughput < 51 || throughput > 52)) {
    NS_LOG_ERROR("Obtained throughput "
                 << throughput << " is not in the expected boundaries!");
    exit(1);
  }

  throughput =
      totalPacketsThroughD * payloadSize * 8 / (simulationTime * 1000000.0);
  std::cout
      << "Throughput with A-MPDU enabled (32kB) and A-MSDU enabled (4kB): "
      << throughput << " Mbit/s" << '\n';
  if (verifyResults && (throughput < 58 || throughput > 59)) {
    NS_LOG_ERROR("Obtained throughput "
                 << throughput << " is not in the expected boundaries!");
    exit(1);
  }

  return 0;
}
