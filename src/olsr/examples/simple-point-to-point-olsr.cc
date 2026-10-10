

#include "ns3/applications-module.h"
#include "ns3/core-module.h"
#include "ns3/internet-module.h"
#include "ns3/ipv4-list-routing-helper.h"
#include "ns3/ipv4-static-routing-helper.h"
#include "ns3/network-module.h"
#include "ns3/olsr-helper.h"
#include "ns3/point-to-point-module.h"

#include <cassert>
#include <fstream>
#include <iostream>
#include <string>

using namespace ns3;

NS_LOG_COMPONENT_DEFINE("SimplePointToPointOlsrExample");

int main(int argc, char *argv[]) {
#if 0
  LogComponentEnable ("SimpleGlobalRoutingExample", LOG_LEVEL_INFO);
#endif

  Config::SetDefault("ns3::OnOffApplication::PacketSize", UintegerValue(210));
  Config::SetDefault("ns3::OnOffApplication::DataRate", StringValue("448kb/s"));

  CommandLine cmd(__FILE__);
  cmd.Parse(argc, argv);

  NS_LOG_INFO("Create nodes.");
  NodeContainer c;
  c.Create(5);
  NodeContainer n02 = NodeContainer(c.Get(0), c.Get(2));
  NodeContainer n12 = NodeContainer(c.Get(1), c.Get(2));
  NodeContainer n32 = NodeContainer(c.Get(3), c.Get(2));
  NodeContainer n34 = NodeContainer(c.Get(3), c.Get(4));

  NS_LOG_INFO("Enabling OLSR Routing.");
  OlsrHelper olsr;

  Ipv4StaticRoutingHelper staticRouting;

  Ipv4ListRoutingHelper list;
  list.Add(staticRouting, 0);
  list.Add(olsr, 10);

  InternetStackHelper internet;
  internet.SetRoutingHelper(list);
  internet.Install(c);

  NS_LOG_INFO("Create channels.");
  PointToPointHelper p2p;
  p2p.SetDeviceAttribute("DataRate", StringValue("5Mbps"));
  p2p.SetChannelAttribute("Delay", StringValue("2ms"));
  NetDeviceContainer nd02 = p2p.Install(n02);
  NetDeviceContainer nd12 = p2p.Install(n12);
  p2p.SetDeviceAttribute("DataRate", StringValue("1500kbps"));
  p2p.SetChannelAttribute("Delay", StringValue("10ms"));
  NetDeviceContainer nd32 = p2p.Install(n32);
  NetDeviceContainer nd34 = p2p.Install(n34);

  NS_LOG_INFO("Assign IP Addresses.");
  Ipv4AddressHelper ipv4;
  ipv4.SetBase("10.1.1.0", "255.255.255.0");
  Ipv4InterfaceContainer i02 = ipv4.Assign(nd02);

  ipv4.SetBase("10.1.2.0", "255.255.255.0");
  Ipv4InterfaceContainer i12 = ipv4.Assign(nd12);

  ipv4.SetBase("10.1.3.0", "255.255.255.0");
  Ipv4InterfaceContainer i32 = ipv4.Assign(nd32);

  ipv4.SetBase("10.1.4.0", "255.255.255.0");
  Ipv4InterfaceContainer i34 = ipv4.Assign(nd34);

  NS_LOG_INFO("Create Applications.");
  uint16_t port = 9;

  OnOffHelper onoff1("ns3::UdpSocketFactory",
                     InetSocketAddress(i34.GetAddress(1), port));
  onoff1.SetConstantRate(DataRate("448kb/s"));

  ApplicationContainer onOffApp1 = onoff1.Install(c.Get(0));
  onOffApp1.Start(Seconds(10.0));
  onOffApp1.Stop(Seconds(20.0));

  OnOffHelper onoff2("ns3::UdpSocketFactory",
                     InetSocketAddress(i12.GetAddress(0), port));
  onoff2.SetConstantRate(DataRate("448kb/s"));

  ApplicationContainer onOffApp2 = onoff2.Install(c.Get(3));
  onOffApp2.Start(Seconds(10.1));
  onOffApp2.Stop(Seconds(20.0));

  PacketSinkHelper sink("ns3::UdpSocketFactory",
                        InetSocketAddress(Ipv4Address::GetAny(), port));
  NodeContainer sinks = NodeContainer(c.Get(4), c.Get(1));
  ApplicationContainer sinkApps = sink.Install(sinks);
  sinkApps.Start(Seconds(0.0));
  sinkApps.Stop(Seconds(21.0));

  AsciiTraceHelper ascii;
  p2p.EnableAsciiAll(ascii.CreateFileStream("simple-point-to-point-olsr.tr"));
  p2p.EnablePcapAll("simple-point-to-point-olsr");

  Simulator::Stop(Seconds(30));

  NS_LOG_INFO("Run Simulation.");
  Simulator::Run();
  Simulator::Destroy();
  NS_LOG_INFO("Done.");

  return 0;
}
