

#include "ns3/applications-module.h"
#include "ns3/bridge-module.h"
#include "ns3/core-module.h"
#include "ns3/csma-module.h"
#include "ns3/internet-module.h"
#include "ns3/network-module.h"

#include <fstream>
#include <iostream>

using namespace ns3;

NS_LOG_COMPONENT_DEFINE("CsmaBridgeExample");

int main(int argc, char *argv[]) {
#if 0
  LogComponentEnable ("CsmaBridgeExample", LOG_LEVEL_INFO);
#endif

  CommandLine cmd(__FILE__);
  cmd.Parse(argc, argv);

  NS_LOG_INFO("Create nodes.");
  NodeContainer terminals;
  terminals.Create(4);

  NodeContainer csmaSwitch;
  csmaSwitch.Create(1);

  NS_LOG_INFO("Build Topology");
  CsmaHelper csma;
  csma.SetChannelAttribute("DataRate", DataRateValue(5000000));
  csma.SetChannelAttribute("Delay", TimeValue(MilliSeconds(2)));

  NetDeviceContainer terminalDevices;
  NetDeviceContainer switchDevices;

  for (int i = 0; i < 4; i++) {
    NetDeviceContainer link =
        csma.Install(NodeContainer(terminals.Get(i), csmaSwitch));
    terminalDevices.Add(link.Get(0));
    switchDevices.Add(link.Get(1));
  }

  Ptr<Node> switchNode = csmaSwitch.Get(0);
  BridgeHelper bridge;
  bridge.Install(switchNode, switchDevices);

  InternetStackHelper internet;
  internet.Install(terminals);

  NS_LOG_INFO("Assign IP Addresses.");
  Ipv4AddressHelper ipv4;
  ipv4.SetBase("10.1.1.0", "255.255.255.0");
  ipv4.Assign(terminalDevices);

  NS_LOG_INFO("Create Applications.");
  uint16_t port = 9;

  OnOffHelper onoff("ns3::UdpSocketFactory",
                    Address(InetSocketAddress(Ipv4Address("10.1.1.2"), port)));
  onoff.SetConstantRate(DataRate("500kb/s"));

  ApplicationContainer app = onoff.Install(terminals.Get(0));
  app.Start(Seconds(1.0));
  app.Stop(Seconds(10.0));

  PacketSinkHelper sink(
      "ns3::UdpSocketFactory",
      Address(InetSocketAddress(Ipv4Address::GetAny(), port)));
  app = sink.Install(terminals.Get(1));
  app.Start(Seconds(0.0));

  onoff.SetAttribute(
      "Remote", AddressValue(InetSocketAddress(Ipv4Address("10.1.1.1"), port)));
  app = onoff.Install(terminals.Get(3));
  app.Start(Seconds(1.1));
  app.Stop(Seconds(10.0));

  app = sink.Install(terminals.Get(0));
  app.Start(Seconds(0.0));

  NS_LOG_INFO("Configure Tracing.");

  AsciiTraceHelper ascii;
  csma.EnableAsciiAll(ascii.CreateFileStream("csma-bridge.tr"));

  csma.EnablePcapAll("csma-bridge", false);

  NS_LOG_INFO("Run Simulation.");
  Simulator::Run();
  Simulator::Destroy();
  NS_LOG_INFO("Done.");

  return 0;
}
