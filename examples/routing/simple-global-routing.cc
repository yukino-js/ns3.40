

#include "ns3/applications-module.h"
#include "ns3/core-module.h"
#include "ns3/flow-monitor-helper.h"
#include "ns3/internet-module.h"
#include "ns3/ipv4-global-routing-helper.h"
#include "ns3/network-module.h"
#include "ns3/point-to-point-module.h"

#include <cassert>
#include <fstream>
#include <iostream>
#include <string>

using namespace ns3;

NS_LOG_COMPONENT_DEFINE("SimpleGlobalRoutingExample");

int main(int argc, char *argv[]) {
#if 0
  LogComponentEnable ("SimpleGlobalRoutingExample", LOG_LEVEL_INFO);
#endif

  Config::SetDefault("ns3::OnOffApplication::PacketSize", UintegerValue(210));
  Config::SetDefault("ns3::OnOffApplication::DataRate", StringValue("448kb/s"));

  CommandLine cmd(__FILE__);
  bool enableFlowMonitor = false;
  cmd.AddValue("EnableMonitor", "Enable Flow Monitor", enableFlowMonitor);
  cmd.Parse(argc, argv);

  NS_LOG_INFO("Create nodes.");
  NodeContainer c;
  c.Create(4);
  NodeContainer n0n2 = NodeContainer(c.Get(0), c.Get(2));
  NodeContainer n1n2 = NodeContainer(c.Get(1), c.Get(2));
  NodeContainer n3n2 = NodeContainer(c.Get(3), c.Get(2));

  InternetStackHelper internet;
  internet.Install(c);

  NS_LOG_INFO("Create channels.");
  PointToPointHelper p2p;
  p2p.SetDeviceAttribute("DataRate", StringValue("5Mbps"));
  p2p.SetChannelAttribute("Delay", StringValue("2ms"));
  NetDeviceContainer d0d2 = p2p.Install(n0n2);

  NetDeviceContainer d1d2 = p2p.Install(n1n2);

  p2p.SetDeviceAttribute("DataRate", StringValue("1500kbps"));
  p2p.SetChannelAttribute("Delay", StringValue("10ms"));
  NetDeviceContainer d3d2 = p2p.Install(n3n2);

  NS_LOG_INFO("Assign IP Addresses.");
  Ipv4AddressHelper ipv4;
  ipv4.SetBase("10.1.1.0", "255.255.255.0");
  Ipv4InterfaceContainer i0i2 = ipv4.Assign(d0d2);

  ipv4.SetBase("10.1.2.0", "255.255.255.0");
  Ipv4InterfaceContainer i1i2 = ipv4.Assign(d1d2);

  ipv4.SetBase("10.1.3.0", "255.255.255.0");
  Ipv4InterfaceContainer i3i2 = ipv4.Assign(d3d2);

  Ipv4GlobalRoutingHelper::PopulateRoutingTables();

  NS_LOG_INFO("Create Applications.");
  uint16_t port = 9;
  OnOffHelper onoff("ns3::UdpSocketFactory",
                    Address(InetSocketAddress(i3i2.GetAddress(0), port)));
  onoff.SetConstantRate(DataRate("448kb/s"));
  ApplicationContainer apps = onoff.Install(c.Get(0));
  apps.Start(Seconds(1.0));
  apps.Stop(Seconds(10.0));

  PacketSinkHelper sink(
      "ns3::UdpSocketFactory",
      Address(InetSocketAddress(Ipv4Address::GetAny(), port)));
  apps = sink.Install(c.Get(3));
  apps.Start(Seconds(1.0));
  apps.Stop(Seconds(10.0));

  onoff.SetAttribute("Remote",
                     AddressValue(InetSocketAddress(i1i2.GetAddress(0), port)));
  apps = onoff.Install(c.Get(3));
  apps.Start(Seconds(1.1));
  apps.Stop(Seconds(10.0));

  apps = sink.Install(c.Get(1));
  apps.Start(Seconds(1.1));
  apps.Stop(Seconds(10.0));

  AsciiTraceHelper ascii;
  p2p.EnableAsciiAll(ascii.CreateFileStream("simple-global-routing.tr"));
  p2p.EnablePcapAll("simple-global-routing");

  FlowMonitorHelper flowmonHelper;
  if (enableFlowMonitor) {
    flowmonHelper.InstallAll();
  }

  NS_LOG_INFO("Run Simulation.");
  Simulator::Stop(Seconds(11));
  Simulator::Run();
  NS_LOG_INFO("Done.");

  if (enableFlowMonitor) {
    flowmonHelper.SerializeToXmlFile("simple-global-routing.flowmon", false,
                                     false);
  }

  Simulator::Destroy();
  return 0;
}
