
#include "ns3/command-line.h"
#include "ns3/internet-stack-helper.h"
#include "ns3/ipv4-address-helper.h"
#include "ns3/log.h"
#include "ns3/mobility-helper.h"
#include "ns3/on-off-helper.h"
#include "ns3/pointer.h"
#include "ns3/qos-txop.h"
#include "ns3/ssid.h"
#include "ns3/string.h"
#include "ns3/udp-client-server-helper.h"
#include "ns3/wifi-mac.h"
#include "ns3/wifi-net-device.h"
#include "ns3/yans-wifi-channel.h"
#include "ns3/yans-wifi-helper.h"

using namespace ns3;

NS_LOG_COMPONENT_DEFINE("80211eTxop");

struct TxopDurationTracer {
  void Trace(Time startTime, Time duration, uint8_t linkId);
  Time m_max{Seconds(0)};
};

void TxopDurationTracer::Trace(Time startTime, Time duration, uint8_t linkId) {
  if (duration > m_max) {
    m_max = duration;
  }
}

int main(int argc, char *argv[]) {
  uint32_t payloadSize = 1472;
  double simulationTime = 10;
  double distance = 5;
  bool enablePcap = false;
  bool verifyResults = false;
  Time txopLimit = MicroSeconds(4096);

  CommandLine cmd(__FILE__);
  cmd.AddValue("payloadSize", "Payload size in bytes", payloadSize);
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

  NodeContainer wifiStaNodes;
  wifiStaNodes.Create(4);
  NodeContainer wifiApNodes;
  wifiApNodes.Create(4);

  YansWifiChannelHelper channel = YansWifiChannelHelper::Default();
  YansWifiPhyHelper phy;
  phy.SetPcapDataLinkType(WifiPhyHelper::DLT_IEEE802_11_RADIO);
  phy.SetChannel(channel.Create());

  WifiHelper wifi;
  wifi.SetStandard(WIFI_STANDARD_80211a);
  wifi.SetRemoteStationManager("ns3::IdealWifiManager");
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
  phy.Set("ChannelSettings", StringValue("{36, 20, BAND_5GHZ, 0}"));
  mac.SetType("ns3::StaWifiMac", "QosSupported", BooleanValue(true), "Ssid",
              SsidValue(ssid));
  staDeviceA = wifi.Install(phy, mac, wifiStaNodes.Get(0));

  mac.SetType("ns3::ApWifiMac", "QosSupported", BooleanValue(true), "Ssid",
              SsidValue(ssid), "EnableBeaconJitter", BooleanValue(false));
  apDeviceA = wifi.Install(phy, mac, wifiApNodes.Get(0));

  ssid = Ssid("network-B");
  phy.Set("ChannelSettings", StringValue("{40, 20, BAND_5GHZ, 0}"));
  mac.SetType("ns3::StaWifiMac", "QosSupported", BooleanValue(true), "Ssid",
              SsidValue(ssid));

  staDeviceB = wifi.Install(phy, mac, wifiStaNodes.Get(1));

  mac.SetType("ns3::ApWifiMac", "QosSupported", BooleanValue(true), "Ssid",
              SsidValue(ssid), "EnableBeaconJitter", BooleanValue(false));
  apDeviceB = wifi.Install(phy, mac, wifiApNodes.Get(1));

  Ptr<NetDevice> dev = wifiApNodes.Get(1)->GetDevice(0);
  Ptr<WifiNetDevice> wifi_dev = DynamicCast<WifiNetDevice>(dev);
  Ptr<WifiMac> wifi_mac = wifi_dev->GetMac();
  PointerValue ptr;
  Ptr<QosTxop> edca;
  wifi_mac->GetAttribute("BE_Txop", ptr);
  edca = ptr.Get<QosTxop>();
  edca->SetTxopLimit(txopLimit);

  dev = wifiStaNodes.Get(1)->GetDevice(0);
  wifi_dev = DynamicCast<WifiNetDevice>(dev);
  wifi_mac = wifi_dev->GetMac();
  wifi_mac->GetAttribute("BE_Txop", ptr);
  edca = ptr.Get<QosTxop>();
  TxopDurationTracer beTxopTracer;
  edca->TraceConnectWithoutContext(
      "TxopTrace", MakeCallback(&TxopDurationTracer::Trace, &beTxopTracer));

  ssid = Ssid("network-C");
  phy.Set("ChannelSettings", StringValue("{44, 20, BAND_5GHZ, 0}"));
  mac.SetType("ns3::StaWifiMac", "QosSupported", BooleanValue(true), "Ssid",
              SsidValue(ssid));

  staDeviceC = wifi.Install(phy, mac, wifiStaNodes.Get(2));

  mac.SetType("ns3::ApWifiMac", "QosSupported", BooleanValue(true), "Ssid",
              SsidValue(ssid), "EnableBeaconJitter", BooleanValue(false));
  apDeviceC = wifi.Install(phy, mac, wifiApNodes.Get(2));

  dev = wifiStaNodes.Get(2)->GetDevice(0);
  wifi_dev = DynamicCast<WifiNetDevice>(dev);
  wifi_mac = wifi_dev->GetMac();
  wifi_mac->GetAttribute("VI_Txop", ptr);
  edca = ptr.Get<QosTxop>();
  TxopDurationTracer viTxopTracer;
  edca->TraceConnectWithoutContext(
      "TxopTrace", MakeCallback(&TxopDurationTracer::Trace, &viTxopTracer));

  ssid = Ssid("network-D");
  phy.Set("ChannelSettings", StringValue("{48, 20, BAND_5GHZ, 0}"));
  mac.SetType("ns3::StaWifiMac", "QosSupported", BooleanValue(true), "Ssid",
              SsidValue(ssid));

  staDeviceD = wifi.Install(phy, mac, wifiStaNodes.Get(3));

  mac.SetType("ns3::ApWifiMac", "QosSupported", BooleanValue(true), "Ssid",
              SsidValue(ssid), "EnableBeaconJitter", BooleanValue(false));
  apDeviceD = wifi.Install(phy, mac, wifiApNodes.Get(3));

  dev = wifiApNodes.Get(3)->GetDevice(0);
  wifi_dev = DynamicCast<WifiNetDevice>(dev);
  wifi_mac = wifi_dev->GetMac();
  wifi_mac->GetAttribute("VI_Txop", ptr);
  edca = ptr.Get<QosTxop>();
  edca->SetTxopLimit(MicroSeconds(0));

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

  uint16_t port = 5001;
  UdpServerHelper serverA(port);
  ApplicationContainer serverAppA = serverA.Install(wifiApNodes.Get(0));
  serverAppA.Start(Seconds(0.0));
  serverAppA.Stop(Seconds(simulationTime + 1));

  InetSocketAddress destA(ApInterfaceA.GetAddress(0), port);
  destA.SetTos(0x70);

  OnOffHelper clientA("ns3::UdpSocketFactory", destA);
  clientA.SetAttribute("OnTime",
                       StringValue("ns3::ConstantRandomVariable[Constant=1]"));
  clientA.SetAttribute("OffTime",
                       StringValue("ns3::ConstantRandomVariable[Constant=0]"));
  clientA.SetAttribute("DataRate", StringValue("100000kb/s"));
  clientA.SetAttribute("PacketSize", UintegerValue(payloadSize));

  ApplicationContainer clientAppA = clientA.Install(wifiStaNodes.Get(0));
  clientAppA.Start(Seconds(1.0));
  clientAppA.Stop(Seconds(simulationTime + 1));

  UdpServerHelper serverB(port);
  ApplicationContainer serverAppB = serverB.Install(wifiApNodes.Get(1));
  serverAppB.Start(Seconds(0.0));
  serverAppB.Stop(Seconds(simulationTime + 1));

  InetSocketAddress destB(ApInterfaceB.GetAddress(0), port);
  destB.SetTos(0x70);

  OnOffHelper clientB("ns3::UdpSocketFactory", destB);
  clientB.SetAttribute("OnTime",
                       StringValue("ns3::ConstantRandomVariable[Constant=1]"));
  clientB.SetAttribute("OffTime",
                       StringValue("ns3::ConstantRandomVariable[Constant=0]"));
  clientB.SetAttribute("DataRate", StringValue("100000kb/s"));
  clientB.SetAttribute("PacketSize", UintegerValue(payloadSize));

  ApplicationContainer clientAppB = clientB.Install(wifiStaNodes.Get(1));
  clientAppB.Start(Seconds(1.0));
  clientAppB.Stop(Seconds(simulationTime + 1));

  UdpServerHelper serverC(port);
  ApplicationContainer serverAppC = serverC.Install(wifiApNodes.Get(2));
  serverAppC.Start(Seconds(0.0));
  serverAppC.Stop(Seconds(simulationTime + 1));

  InetSocketAddress destC(ApInterfaceC.GetAddress(0), port);
  destC.SetTos(0xb8);

  OnOffHelper clientC("ns3::UdpSocketFactory", destC);
  clientC.SetAttribute("OnTime",
                       StringValue("ns3::ConstantRandomVariable[Constant=1]"));
  clientC.SetAttribute("OffTime",
                       StringValue("ns3::ConstantRandomVariable[Constant=0]"));
  clientC.SetAttribute("DataRate", StringValue("100000kb/s"));
  clientC.SetAttribute("PacketSize", UintegerValue(payloadSize));

  ApplicationContainer clientAppC = clientC.Install(wifiStaNodes.Get(2));
  clientAppC.Start(Seconds(1.0));
  clientAppC.Stop(Seconds(simulationTime + 1));

  UdpServerHelper serverD(port);
  ApplicationContainer serverAppD = serverD.Install(wifiApNodes.Get(3));
  serverAppD.Start(Seconds(0.0));
  serverAppD.Stop(Seconds(simulationTime + 1));

  InetSocketAddress destD(ApInterfaceD.GetAddress(0), port);
  destD.SetTos(0xb8);

  OnOffHelper clientD("ns3::UdpSocketFactory", destD);
  clientD.SetAttribute("OnTime",
                       StringValue("ns3::ConstantRandomVariable[Constant=1]"));
  clientD.SetAttribute("OffTime",
                       StringValue("ns3::ConstantRandomVariable[Constant=0]"));
  clientD.SetAttribute("DataRate", StringValue("100000kb/s"));
  clientD.SetAttribute("PacketSize", UintegerValue(payloadSize));

  ApplicationContainer clientAppD = clientD.Install(wifiStaNodes.Get(3));
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
  std::cout << "AC_BE with default TXOP limit (0ms): " << '\n'
            << "  Throughput = " << throughput << " Mbit/s" << '\n';
  if (verifyResults && (throughput < 28 || throughput > 29)) {
    NS_LOG_ERROR("Obtained throughput "
                 << throughput << " is not in the expected boundaries!");
    exit(1);
  }

  throughput =
      totalPacketsThroughB * payloadSize * 8 / (simulationTime * 1000000.0);
  std::cout << "AC_BE with non-default TXOP limit (4.096ms): " << '\n'
            << "  Throughput = " << throughput << " Mbit/s" << '\n';
  if (verifyResults && (throughput < 36.5 || throughput > 37)) {
    NS_LOG_ERROR("Obtained throughput "
                 << throughput << " is not in the expected boundaries!");
    exit(1);
  }
  std::cout << "  Maximum TXOP duration = "
            << beTxopTracer.m_max.GetMicroSeconds() << " us" << '\n';
  if (verifyResults && (beTxopTracer.m_max < MicroSeconds(3008) ||
                        beTxopTracer.m_max > txopLimit)) {
    NS_LOG_ERROR("Maximum TXOP duration "
                 << beTxopTracer.m_max
                 << " is not in the expected boundaries!");
    exit(1);
  }

  throughput =
      totalPacketsThroughC * payloadSize * 8 / (simulationTime * 1000000.0);
  std::cout << "AC_VI with default TXOP limit (4.096ms): " << '\n'
            << "  Throughput = " << throughput << " Mbit/s" << '\n';
  if (verifyResults && (throughput < 36.5 || throughput > 37.5)) {
    NS_LOG_ERROR("Obtained throughput "
                 << throughput << " is not in the expected boundaries!");
    exit(1);
  }
  std::cout << "  Maximum TXOP duration = "
            << viTxopTracer.m_max.GetMicroSeconds() << " us" << '\n';
  if (verifyResults && (viTxopTracer.m_max < MicroSeconds(3008) ||
                        viTxopTracer.m_max > txopLimit)) {
    NS_LOG_ERROR("Maximum TXOP duration "
                 << viTxopTracer.m_max
                 << " is not in the expected boundaries!");
    exit(1);
  }

  throughput =
      totalPacketsThroughD * payloadSize * 8 / (simulationTime * 1000000.0);
  std::cout << "AC_VI with non-default TXOP limit (0ms): " << '\n'
            << "  Throughput = " << throughput << " Mbit/s" << '\n';
  if (verifyResults && (throughput < 31.5 || throughput > 32.5)) {
    NS_LOG_ERROR("Obtained throughput "
                 << throughput << " is not in the expected boundaries!");
    exit(1);
  }

  return 0;
}
