
#include "ns3/applications-module.h"
#include "ns3/core-module.h"
#include "ns3/flow-monitor-helper.h"
#include "ns3/flow-monitor-module.h"
#include "ns3/internet-module.h"
#include "ns3/ipv4-flow-classifier.h"
#include "ns3/mobility-module.h"
#include "ns3/network-module.h"
#include "ns3/point-to-point-module.h"
#include "ns3/ssid.h"
#include "ns3/tcp-bbr.h"
#include "ns3/tcp-socket-factory.h"
#include "ns3/traffic-control-module.h"
#include "ns3/yans-wifi-helper.h"

#include <ostream>

using namespace ns3;

NS_LOG_COMPONENT_DEFINE("main");

int main(int argc, char *argv[]) {
  Config::SetDefault("ns3::TcpL4Protocol::SocketType",
                     TypeIdValue(TcpBbr::GetTypeId()));
  Config::SetDefault("ns3::TcpSocketState::EnablePacing", BooleanValue(true));

  Config::SetDefault("ns3::RedQueueDisc::MaxSize", StringValue("10000p"));

  Time::SetResolution(Time::NS);
  LogComponentEnable("main", LOG_LEVEL_DEBUG);
  LogComponentEnable("TcpSocketBase", LOG_LEVEL_WARN);
  LogComponentEnable("TcpBbr", LOG_LEVEL_WARN);

  NodeContainer wiredNodes;
  wiredNodes.Create(1);
  NodeContainer router;
  router.Create(1);
  NodeContainer wifiStaNodes;
  wifiStaNodes.Create(1);
  PointToPointHelper p2pLeft;

  InternetStackHelper stack;
  stack.Install(wiredNodes);
  stack.Install(router);
  stack.Install(wifiStaNodes);

  p2pLeft.SetDeviceAttribute("DataRate", StringValue("1Gbps"));
  p2pLeft.SetChannelAttribute("Delay", StringValue("100ms"));

  NetDeviceContainer wiredDevicesLeft =
      p2pLeft.Install(wiredNodes.Get(0), router.Get(0));
  TrafficControlHelper tchLeft;
  tchLeft.SetRootQueueDisc("ns3::RedQueueDisc", "MaxSize",
                           StringValue("10000p"));
  tchLeft.Install(wiredDevicesLeft);

  Ptr<RateErrorModel> em = CreateObject<RateErrorModel>();
  em->SetAttribute("ErrorRate", DoubleValue(0.00001));
  em->SetAttribute("ErrorUnit", StringValue("ERROR_UNIT_PACKET"));

  wiredDevicesLeft.Get(0)->SetAttribute("ReceiveErrorModel", PointerValue(em));
  wiredDevicesLeft.Get(1)->SetAttribute("ReceiveErrorModel", PointerValue(em));

  YansWifiChannelHelper channel;
  channel.SetPropagationDelay("ns3::ConstantSpeedPropagationDelayModel",
                              "Speed", DoubleValue(3e8));
  channel.AddPropagationLoss("ns3::RandomPropagationLossModel");
  YansWifiPhyHelper phy;
  phy.SetChannel(channel.Create());
  WifiHelper wifi;
  wifi.SetStandard(WIFI_STANDARD_80211n);

  wifi.SetRemoteStationManager("ns3::ConstantRateWifiManager", "DataMode",
                               StringValue("HtMcs7"), "ControlMode",
                               StringValue("HtMcs0"));

  WifiMacHelper mac;
  Ssid ssid = Ssid("HybridNetwork");

  mac.SetType("ns3::ApWifiMac", "Ssid", SsidValue(ssid));
  NetDeviceContainer apDevices = wifi.Install(phy, mac, router.Get(0));

  mac.SetType("ns3::StaWifiMac", "Ssid", SsidValue(ssid), "ActiveProbing",
              BooleanValue(false));
  NetDeviceContainer wirelessDeviceRight = wifi.Install(phy, mac, wifiStaNodes);

  MobilityHelper mobility;

  mobility.SetMobilityModel("ns3::ConstantPositionMobilityModel");
  mobility.Install(router);
  router.Get(0)->GetObject<MobilityModel>()->SetPosition(Vector(0, 0, 0));

  mobility.SetPositionAllocator(
      "ns3::GridPositionAllocator", "MinX", DoubleValue(40), "MinY",
      DoubleValue(0.0), "DeltaX", DoubleValue(0.0), "DeltaY", DoubleValue(10.0),
      "GridWidth", UintegerValue(1), "LayoutType", StringValue("RowFirst"));
  mobility.SetMobilityModel("ns3::ConstantPositionMobilityModel");
  mobility.Install(wifiStaNodes);

  TrafficControlHelper tchRight;
  tchRight.SetRootQueueDisc("ns3::RedQueueDisc", "MaxSize",
                            StringValue("10000p"));
  tchRight.Install(wirelessDeviceRight);

  Ipv4AddressHelper address;

  address.SetBase("10.1.1.0", "255.255.255.0");
  Ipv4InterfaceContainer p2pInterfaces = address.Assign(wiredDevicesLeft);

  address.SetBase("192.168.1.0", "255.255.255.0");
  Ipv4InterfaceContainer apInterface = address.Assign(apDevices);
  Ipv4InterfaceContainer staInterfaces = address.Assign(wirelessDeviceRight);

  router.Get(0)->GetObject<Ipv4>()->SetAttribute("IpForward",
                                                 BooleanValue(true));
  Ipv4StaticRoutingHelper staticRouting;
  Ptr<Ipv4StaticRouting> staticRouter =
      staticRouting.GetStaticRouting(router.Get(0)->GetObject<Ipv4>());

  staticRouter->AddNetworkRouteTo(Ipv4Address("10.1.1.0"),
                                  Ipv4Mask("255.255.255.0"), 1);

  staticRouter->AddNetworkRouteTo(Ipv4Address("192.168.1.0"),
                                  Ipv4Mask("255.255.255.0"), 2);

  for (uint32_t i = 0; i < wifiStaNodes.GetN(); ++i) {
    Ptr<Ipv4StaticRouting> staStaticRouting =
        staticRouting.GetStaticRouting(wifiStaNodes.Get(i)->GetObject<Ipv4>());
    staStaticRouting->SetDefaultRoute(
        router.Get(0)->GetObject<Ipv4>()->GetAddress(2, 0).GetLocal(), 1);
  }

  Ipv4GlobalRoutingHelper::PopulateRoutingTables();
  PacketSinkHelper sinkHelper("ns3::TcpSocketFactory",
                              InetSocketAddress(Ipv4Address::GetAny(), 9));
  ApplicationContainer sinkApp = sinkHelper.Install(wifiStaNodes.Get(0));
  Ptr<PacketSink> sink = StaticCast<PacketSink>(sinkApp.Get(0));

  OnOffHelper server("ns3::TcpSocketFactory",
                     InetSocketAddress(staInterfaces.GetAddress(0), 9));
  server.SetAttribute("PacketSize", UintegerValue(1472));
  server.SetAttribute("DataRate", StringValue("1Gbps"));
  server.SetAttribute("OnTime",
                      StringValue("ns3::ConstantRandomVariable[Constant=100]"));
  server.SetAttribute("OffTime",
                      StringValue("ns3::ConstantRandomVariable[Constant=0]"));

  ApplicationContainer serverApp = server.Install(wiredNodes.Get(0));

  sinkApp.Start(Seconds(0.0));
  serverApp.Start(Seconds(1.0));
  serverApp.Stop(Seconds(100.0));

  FlowMonitorHelper flowMonitor;
  Ptr<FlowMonitor> monitor = flowMonitor.InstallAll();
  LogComponentEnable("main", LOG_LEVEL_WARN);
  Simulator::Stop(Seconds(103.0));

  auto n0Ipv4 = wiredNodes.Get(0)->GetObject<Ipv4>();
  for (auto i = 0; i < n0Ipv4->GetNInterfaces(); ++i) {
    NS_LOG_UNCOND("[n0] Interface" << i << ": "
                                   << n0Ipv4->GetAddress(i, 0).GetLocal());
  }

  Ptr<Ipv4> routerIpv4 = router.Get(0)->GetObject<Ipv4>();
  for (uint32_t i = 0; i < routerIpv4->GetNInterfaces(); ++i) {
    NS_LOG_UNCOND("[n1] Interface" << i << ": "
                                   << routerIpv4->GetAddress(i, 0).GetLocal());
  }

  auto n2Ipv4 = wifiStaNodes.Get(0)->GetObject<Ipv4>();
  for (auto i = 0; i < n2Ipv4->GetNInterfaces(); ++i) {
    NS_LOG_UNCOND("[n2] Interface" << i << ": "
                                   << n2Ipv4->GetAddress(i, 0).GetLocal());
  }

  Simulator::Run();

  monitor->CheckForLostPackets();
  Ptr<ns3::Ipv4FlowClassifier> classifier =
      DynamicCast<ns3::Ipv4FlowClassifier>(flowMonitor.GetClassifier());
  std::map<FlowId, FlowMonitor::FlowStats> stats = monitor->GetFlowStats();
  for (auto iter = stats.begin(); iter != stats.end(); ++iter) {
    Ipv4FlowClassifier::FiveTuple t = classifier->FindFlow(iter->first);

    NS_LOG_UNCOND("TCP Flow " << iter->first << " Src Addr: " << t.sourceAddress
                              << " Dst Addr: " << t.destinationAddress);
    NS_LOG_UNCOND("Tx Packets Count: " << iter->second.txPackets);
    NS_LOG_UNCOND("Rx Packets Count: " << iter->second.rxPackets);
    NS_LOG_UNCOND("Loss Rate: " << (iter->second.lostPackets /
                                    (double)iter->second.txPackets) *
                                       100
                                << "%");
    NS_LOG_UNCOND("Throughput: "
                  << iter->second.rxBytes * 8.0 /
                         (iter->second.timeLastRxPacket.GetSeconds() -
                          iter->second.timeFirstTxPacket.GetSeconds()) /
                         1e6
                  << "Mbps");
  }
  NS_LOG_UNCOND("Total Rx Bytes Count: " << sink->GetTotalRx());
  Simulator::Destroy();

  return 0;
}
