

#include "ns3/applications-module.h"
#include "ns3/core-module.h"
#include "ns3/csma-module.h"
#include "ns3/internet-module.h"
#include "ns3/network-module.h"

#include <fstream>
#include <iostream>

using namespace ns3;

NS_LOG_COMPONENT_DEFINE("CsmaOneSubnetExample");

int main(int argc, char *argv[]) {
#if 0
  LogComponentEnable ("CsmaOneSubnetExample", LOG_LEVEL_INFO);
#endif
  CommandLine cmd(__FILE__);
  cmd.Parse(argc, argv);
  NS_LOG_INFO("Create nodes.");
  NodeContainer nodes;
  nodes.Create(4);

  NS_LOG_INFO("Build Topology");
  CsmaHelper csma;
  csma.SetChannelAttribute("DataRate", DataRateValue(5000000));
  csma.SetChannelAttribute("Delay", TimeValue(MilliSeconds(2)));
  NetDeviceContainer devices = csma.Install(nodes);

  InternetStackHelper internet;
  internet.Install(nodes);

  NS_LOG_INFO("Assign IP Addresses.");
  Ipv4AddressHelper ipv4;
  ipv4.SetBase("10.1.1.0", "255.255.255.0");
  Ipv4InterfaceContainer interfaces = ipv4.Assign(devices);

  NS_LOG_INFO("Create Applications.");
  uint16_t port = 9;

  OnOffHelper onoff("ns3::UdpSocketFactory",
                    Address(InetSocketAddress(interfaces.GetAddress(1), port)));
  onoff.SetConstantRate(DataRate("500kb/s"));

  ApplicationContainer app = onoff.Install(nodes.Get(0));
  app.Start(Seconds(1.0));
  app.Stop(Seconds(10.0));

  PacketSinkHelper sink(
      "ns3::UdpSocketFactory",
      Address(InetSocketAddress(Ipv4Address::GetAny(), port)));
  app = sink.Install(nodes.Get(1));
  app.Start(Seconds(0.0));

  onoff.SetAttribute("Remote", AddressValue(InetSocketAddress(
                                   interfaces.GetAddress(0), port)));
  app = onoff.Install(nodes.Get(3));
  app.Start(Seconds(1.1));
  app.Stop(Seconds(10.0));

  app = sink.Install(nodes.Get(0));
  app.Start(Seconds(0.0));

  NS_LOG_INFO("Configure Tracing.");
  AsciiTraceHelper ascii;
  csma.EnableAsciiAll(ascii.CreateFileStream("csma-one-subnet.tr"));

  csma.EnablePcapAll("csma-one-subnet", false);
  NS_LOG_INFO("Run Simulation.");
  Simulator::Run();
  Simulator::Destroy();
  NS_LOG_INFO("Done.");

  return 0;
}
