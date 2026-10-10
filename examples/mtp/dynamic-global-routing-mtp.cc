

#include "ns3/applications-module.h"
#include "ns3/core-module.h"
#include "ns3/csma-module.h"
#include "ns3/internet-module.h"
#include "ns3/ipv4-global-routing-helper.h"
#include "ns3/mtp-module.h"
#include "ns3/network-module.h"
#include "ns3/point-to-point-module.h"

#include <cassert>
#include <fstream>
#include <iostream>
#include <string>

using namespace ns3;

NS_LOG_COMPONENT_DEFINE("DynamicGlobalRoutingExample");

int main(int argc, char *argv[]) {
  MtpInterface::Enable();
  Config::SetDefault("ns3::Ipv4GlobalRouting::RespondToInterfaceEvents",
                     BooleanValue(true));

  CommandLine cmd(__FILE__);
  cmd.Parse(argc, argv);

  NS_LOG_INFO("Create nodes.");
  NodeContainer c;
  c.Create(7);
  NodeContainer n0n2 = NodeContainer(c.Get(0), c.Get(2));
  NodeContainer n1n2 = NodeContainer(c.Get(1), c.Get(2));
  NodeContainer n5n6 = NodeContainer(c.Get(5), c.Get(6));
  NodeContainer n1n6 = NodeContainer(c.Get(1), c.Get(6));
  NodeContainer n2345 = NodeContainer(c.Get(2), c.Get(3), c.Get(4), c.Get(5));

  InternetStackHelper internet;
  internet.Install(c);

  NS_LOG_INFO("Create channels.");
  PointToPointHelper p2p;
  p2p.SetDeviceAttribute("DataRate", StringValue("5Mbps"));
  p2p.SetChannelAttribute("Delay", StringValue("2ms"));
  NetDeviceContainer d0d2 = p2p.Install(n0n2);
  NetDeviceContainer d1d6 = p2p.Install(n1n6);

  NetDeviceContainer d1d2 = p2p.Install(n1n2);

  p2p.SetDeviceAttribute("DataRate", StringValue("1500kbps"));
  p2p.SetChannelAttribute("Delay", StringValue("10ms"));
  NetDeviceContainer d5d6 = p2p.Install(n5n6);

  CsmaHelper csma;
  csma.SetChannelAttribute("DataRate", StringValue("5Mbps"));
  csma.SetChannelAttribute("Delay", StringValue("2ms"));
  NetDeviceContainer d2345 = csma.Install(n2345);

  NS_LOG_INFO("Assign IP Addresses.");
  Ipv4AddressHelper ipv4;
  ipv4.SetBase("10.1.1.0", "255.255.255.0");
  ipv4.Assign(d0d2);

  ipv4.SetBase("10.1.2.0", "255.255.255.0");
  ipv4.Assign(d1d2);

  ipv4.SetBase("10.1.3.0", "255.255.255.0");
  Ipv4InterfaceContainer i5i6 = ipv4.Assign(d5d6);

  ipv4.SetBase("10.250.1.0", "255.255.255.0");
  ipv4.Assign(d2345);

  ipv4.SetBase("172.16.1.0", "255.255.255.0");
  Ipv4InterfaceContainer i1i6 = ipv4.Assign(d1d6);

  Ipv4GlobalRoutingHelper::PopulateRoutingTables();

  NS_LOG_INFO("Create Applications.");
  uint16_t port = 9;
  OnOffHelper onoff("ns3::UdpSocketFactory",
                    InetSocketAddress(i5i6.GetAddress(1), port));
  onoff.SetConstantRate(DataRate("2kbps"));
  onoff.SetAttribute("PacketSize", UintegerValue(50));

  ApplicationContainer apps = onoff.Install(c.Get(1));
  apps.Start(Seconds(1.0));
  apps.Stop(Seconds(10.0));

  OnOffHelper onoff2("ns3::UdpSocketFactory",
                     InetSocketAddress(i1i6.GetAddress(1), port));
  onoff2.SetAttribute("OnTime",
                      StringValue("ns3::ConstantRandomVariable[Constant=1]"));
  onoff2.SetAttribute("OffTime",
                      StringValue("ns3::ConstantRandomVariable[Constant=0]"));
  onoff2.SetAttribute("DataRate", StringValue("2kbps"));
  onoff2.SetAttribute("PacketSize", UintegerValue(50));

  ApplicationContainer apps2 = onoff2.Install(c.Get(1));
  apps2.Start(Seconds(11.0));
  apps2.Stop(Seconds(16.0));

  PacketSinkHelper sink(
      "ns3::UdpSocketFactory",
      Address(InetSocketAddress(Ipv4Address::GetAny(), port)));
  apps = sink.Install(c.Get(6));
  apps.Start(Seconds(1.0));
  apps.Stop(Seconds(10.0));

  PacketSinkHelper sink2(
      "ns3::UdpSocketFactory",
      Address(InetSocketAddress(Ipv4Address::GetAny(), port)));
  apps2 = sink2.Install(c.Get(6));
  apps2.Start(Seconds(11.0));
  apps2.Stop(Seconds(16.0));

  AsciiTraceHelper ascii;
  Ptr<OutputStreamWrapper> stream =
      ascii.CreateFileStream("dynamic-global-routing.tr");
  p2p.EnableAsciiAll(stream);
  csma.EnableAsciiAll(stream);
  internet.EnableAsciiIpv4All(stream);

  p2p.EnablePcapAll("dynamic-global-routing");
  csma.EnablePcapAll("dynamic-global-routing", false);

  Ptr<Node> n1 = c.Get(1);
  Ptr<Ipv4> ipv41 = n1->GetObject<Ipv4>();
  uint32_t ipv4ifIndex1 = 2;

  Simulator::Schedule(Seconds(2), &Ipv4::SetDown, ipv41, ipv4ifIndex1);
  Simulator::Schedule(Seconds(4), &Ipv4::SetUp, ipv41, ipv4ifIndex1);

  Ptr<Node> n6 = c.Get(6);
  Ptr<Ipv4> ipv46 = n6->GetObject<Ipv4>();
  uint32_t ipv4ifIndex6 = 2;
  Simulator::Schedule(Seconds(6), &Ipv4::SetDown, ipv46, ipv4ifIndex6);
  Simulator::Schedule(Seconds(8), &Ipv4::SetUp, ipv46, ipv4ifIndex6);

  Simulator::Schedule(Seconds(12), &Ipv4::SetDown, ipv41, ipv4ifIndex1);
  Simulator::Schedule(Seconds(14), &Ipv4::SetUp, ipv41, ipv4ifIndex1);

  Ptr<OutputStreamWrapper> routingStream = Create<OutputStreamWrapper>(
      "dynamic-global-routing.routes", std::ios::out);
  Ipv4RoutingHelper::PrintRoutingTableAllAt(Seconds(12), routingStream);

  NS_LOG_INFO("Run Simulation.");
  Simulator::Run();
  Simulator::Destroy();
  NS_LOG_INFO("Done.");

  return 0;
}
