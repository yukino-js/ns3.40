

#include "ns3/applications-module.h"
#include "ns3/core-module.h"
#include "ns3/csma-module.h"
#include "ns3/internet-module.h"
#include "ns3/network-module.h"

#include <cassert>
#include <fstream>
#include <iostream>
#include <string>

using namespace ns3;

NS_LOG_COMPONENT_DEFINE("CsmaBroadcastExample");

int main(int argc, char *argv[]) {
#if 0
  LogComponentEnable ("CsmaBroadcastExample", LOG_LEVEL_INFO);
#endif
  LogComponentEnable("CsmaBroadcastExample", LOG_PREFIX_TIME);

  CommandLine cmd(__FILE__);
  cmd.Parse(argc, argv);

  NS_LOG_INFO("Create nodes.");
  NodeContainer c;
  c.Create(3);
  NodeContainer c0 = NodeContainer(c.Get(0), c.Get(1));
  NodeContainer c1 = NodeContainer(c.Get(0), c.Get(2));

  NS_LOG_INFO("Build Topology.");
  CsmaHelper csma;
  csma.SetChannelAttribute("DataRate", DataRateValue(DataRate(5000000)));
  csma.SetChannelAttribute("Delay", TimeValue(MilliSeconds(2)));

  NetDeviceContainer n0 = csma.Install(c0);
  NetDeviceContainer n1 = csma.Install(c1);

  InternetStackHelper internet;
  internet.Install(c);

  NS_LOG_INFO("Assign IP Addresses.");
  Ipv4AddressHelper ipv4;
  ipv4.SetBase("10.1.0.0", "255.255.255.0");
  ipv4.Assign(n0);
  ipv4.SetBase("192.168.1.0", "255.255.255.0");
  ipv4.Assign(n1);

  uint16_t port = 9;

  NS_LOG_INFO("Create Applications.");
  OnOffHelper onoff(
      "ns3::UdpSocketFactory",
      Address(InetSocketAddress(Ipv4Address("255.255.255.255"), port)));
  onoff.SetConstantRate(DataRate("500kb/s"));

  ApplicationContainer app = onoff.Install(c0.Get(0));
  app.Start(Seconds(1.0));
  app.Stop(Seconds(10.0));

  PacketSinkHelper sink(
      "ns3::UdpSocketFactory",
      Address(InetSocketAddress(Ipv4Address::GetAny(), port)));
  app = sink.Install(c0.Get(1));
  app.Add(sink.Install(c1.Get(1)));
  app.Start(Seconds(1.0));
  app.Stop(Seconds(10.0));

  AsciiTraceHelper ascii;
  csma.EnableAsciiAll(ascii.CreateFileStream("csma-broadcast.tr"));

  csma.EnablePcapAll("csma-broadcast", false);

  NS_LOG_INFO("Run Simulation.");
  Simulator::Run();
  Simulator::Destroy();
  NS_LOG_INFO("Done.");

  return 0;
}
