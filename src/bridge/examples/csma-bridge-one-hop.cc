

#include "ns3/applications-module.h"
#include "ns3/bridge-module.h"
#include "ns3/core-module.h"
#include "ns3/csma-module.h"
#include "ns3/internet-module.h"
#include "ns3/network-module.h"

#include <fstream>
#include <iostream>

using namespace ns3;

NS_LOG_COMPONENT_DEFINE("CsmaBridgeOneHopExample");

int main(int argc, char *argv[]) {
#if 0
  LogComponentEnable ("CsmaBridgeOneHopExample", LOG_LEVEL_INFO);
#endif

  CommandLine cmd(__FILE__);
  cmd.Parse(argc, argv);

  NS_LOG_INFO("Create nodes.");

  Ptr<Node> n0 = CreateObject<Node>();
  Ptr<Node> n1 = CreateObject<Node>();
  Ptr<Node> n2 = CreateObject<Node>();
  Ptr<Node> n3 = CreateObject<Node>();
  Ptr<Node> n4 = CreateObject<Node>();

  Ptr<Node> bridge1 = CreateObject<Node>();
  Ptr<Node> bridge2 = CreateObject<Node>();

  NS_LOG_INFO("Build Topology");
  CsmaHelper csma;
  csma.SetChannelAttribute("DataRate", DataRateValue(5000000));
  csma.SetChannelAttribute("Delay", TimeValue(MilliSeconds(2)));

  NetDeviceContainer topLanDevices;
  NetDeviceContainer topBridgeDevices;

  NodeContainer topLan(n2, n0, n1);

  for (int i = 0; i < 3; i++) {
    NetDeviceContainer link =
        csma.Install(NodeContainer(topLan.Get(i), bridge1));
    topLanDevices.Add(link.Get(0));
    topBridgeDevices.Add(link.Get(1));
  }

  BridgeHelper bridge;
  bridge.Install(bridge1, topBridgeDevices);

  NodeContainer routerNodes(n0, n1, n2, n3, n4);
  InternetStackHelper internet;
  internet.Install(routerNodes);

  NetDeviceContainer bottomLanDevices;
  NetDeviceContainer bottomBridgeDevices;
  NodeContainer bottomLan(n2, n3, n4);
  for (int i = 0; i < 3; i++) {
    NetDeviceContainer link =
        csma.Install(NodeContainer(bottomLan.Get(i), bridge2));
    bottomLanDevices.Add(link.Get(0));
    bottomBridgeDevices.Add(link.Get(1));
  }
  bridge.Install(bridge2, bottomBridgeDevices);

  NS_LOG_INFO("Assign IP Addresses.");
  Ipv4AddressHelper ipv4;
  ipv4.SetBase("10.1.1.0", "255.255.255.0");
  ipv4.Assign(topLanDevices);
  ipv4.SetBase("10.1.2.0", "255.255.255.0");
  ipv4.Assign(bottomLanDevices);

  Ipv4GlobalRoutingHelper::PopulateRoutingTables();

  NS_LOG_INFO("Create Applications.");
  uint16_t port = 9;

  OnOffHelper onoff("ns3::UdpSocketFactory",
                    Address(InetSocketAddress(Ipv4Address("10.1.1.3"), port)));
  onoff.SetConstantRate(DataRate("500kb/s"));

  ApplicationContainer app = onoff.Install(n0);
  app.Start(Seconds(1.0));
  app.Stop(Seconds(10.0));

  PacketSinkHelper sink(
      "ns3::UdpSocketFactory",
      Address(InetSocketAddress(Ipv4Address::GetAny(), port)));
  ApplicationContainer sink1 = sink.Install(n1);
  sink1.Start(Seconds(1.0));
  sink1.Stop(Seconds(10.0));

  onoff.SetAttribute(
      "Remote", AddressValue(InetSocketAddress(Ipv4Address("10.1.1.2"), port)));
  ApplicationContainer app2 = onoff.Install(n3);
  app2.Start(Seconds(1.1));
  app2.Stop(Seconds(10.0));

  ApplicationContainer sink2 = sink.Install(n0);
  sink2.Start(Seconds(1.1));
  sink2.Stop(Seconds(10.0));

  NS_LOG_INFO("Configure Tracing.");

  AsciiTraceHelper ascii;
  csma.EnableAsciiAll(ascii.CreateFileStream("csma-bridge-one-hop.tr"));

  csma.EnablePcapAll("csma-bridge-one-hop", false);

  NS_LOG_INFO("Run Simulation.");
  Simulator::Run();
  Simulator::Destroy();
  NS_LOG_INFO("Done.");

  return 0;
}
