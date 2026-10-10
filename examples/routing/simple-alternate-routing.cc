

#include "ns3/applications-module.h"
#include "ns3/core-module.h"
#include "ns3/internet-module.h"
#include "ns3/ipv4-global-routing-helper.h"
#include "ns3/network-module.h"
#include "ns3/point-to-point-module.h"

#include <cassert>
#include <fstream>
#include <iostream>
#include <string>

using namespace ns3;

NS_LOG_COMPONENT_DEFINE("SimpleAlternateRoutingExample");

int main(int argc, char *argv[]) {
#if 0
  LogComponentEnable ("GlobalRoutingHelper", LOG_LOGIC);
  LogComponentEnable ("GlobalRouter", LOG_LOGIC);
#endif

  Config::SetDefault("ns3::OnOffApplication::PacketSize", UintegerValue(210));
  Config::SetDefault("ns3::OnOffApplication::DataRate", StringValue("300b/s"));

  CommandLine cmd(__FILE__);
  uint16_t sampleMetric = 1;
  cmd.AddValue("AlternateCost",
               "This metric is used in the example script between n3 and n1 ",
               sampleMetric);

  cmd.Parse(argc, argv);

  NS_LOG_INFO("Create nodes.");
  NodeContainer c;
  c.Create(4);
  NodeContainer n0n2 = NodeContainer(c.Get(0), c.Get(2));
  NodeContainer n1n2 = NodeContainer(c.Get(1), c.Get(2));
  NodeContainer n3n2 = NodeContainer(c.Get(3), c.Get(2));
  NodeContainer n1n3 = NodeContainer(c.Get(1), c.Get(3));

  NS_LOG_INFO("Create channels.");
  PointToPointHelper p2p;
  p2p.SetDeviceAttribute("DataRate", StringValue("5Mbps"));
  p2p.SetChannelAttribute("Delay", StringValue("2ms"));
  NetDeviceContainer d0d2 = p2p.Install(n0n2);

  NetDeviceContainer d1d2 = p2p.Install(n1n2);

  p2p.SetDeviceAttribute("DataRate", StringValue("1500kbps"));
  p2p.SetChannelAttribute("Delay", StringValue("10ms"));
  NetDeviceContainer d3d2 = p2p.Install(n3n2);

  p2p.SetChannelAttribute("Delay", StringValue("100ms"));
  NetDeviceContainer d1d3 = p2p.Install(n1n3);

  InternetStackHelper internet;
  internet.Install(c);

  NS_LOG_INFO("Assign IP Addresses.");
  Ipv4AddressHelper ipv4;
  ipv4.SetBase("10.0.0.0", "255.255.255.0");
  ipv4.Assign(d0d2);

  ipv4.SetBase("10.1.1.0", "255.255.255.0");
  Ipv4InterfaceContainer i1i2 = ipv4.Assign(d1d2);

  ipv4.SetBase("10.2.2.0", "255.255.255.0");
  ipv4.Assign(d3d2);

  ipv4.SetBase("10.3.3.0", "255.255.255.0");
  Ipv4InterfaceContainer i1i3 = ipv4.Assign(d1d3);

  i1i3.SetMetric(0, sampleMetric);
  i1i3.SetMetric(1, sampleMetric);

  Ipv4GlobalRoutingHelper::PopulateRoutingTables();

  NS_LOG_INFO("Create Application.");
  uint16_t port = 9;

  OnOffHelper onoff("ns3::UdpSocketFactory",
                    Address(InetSocketAddress(i1i2.GetAddress(0), port)));
  onoff.SetConstantRate(DataRate("300b/s"));

  ApplicationContainer apps = onoff.Install(c.Get(3));
  apps.Start(Seconds(1.1));
  apps.Stop(Seconds(10.0));

  PacketSinkHelper sink(
      "ns3::UdpSocketFactory",
      Address(InetSocketAddress(Ipv4Address::GetAny(), port)));
  apps = sink.Install(c.Get(1));
  apps.Start(Seconds(1.1));
  apps.Stop(Seconds(10.0));

  AsciiTraceHelper ascii;
  p2p.EnableAsciiAll(ascii.CreateFileStream("simple-alternate-routing.tr"));
  p2p.EnablePcapAll("simple-alternate-routing");

  NS_LOG_INFO("Run Simulation.");
  Simulator::Run();
  Simulator::Destroy();
  NS_LOG_INFO("Done.");

  return 0;
}
