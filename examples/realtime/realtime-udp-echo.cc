

#include "ns3/applications-module.h"
#include "ns3/core-module.h"
#include "ns3/csma-module.h"
#include "ns3/internet-module.h"

#include <fstream>

using namespace ns3;

NS_LOG_COMPONENT_DEFINE("RealtimeUdpEchoExample");

int main(int argc, char *argv[]) {
  CommandLine cmd(__FILE__);
  cmd.Parse(argc, argv);

  GlobalValue::Bind("SimulatorImplementationType",
                    StringValue("ns3::RealtimeSimulatorImpl"));

  NS_LOG_INFO("Create nodes.");
  NodeContainer n;
  n.Create(4);

  InternetStackHelper internet;
  internet.Install(n);

  NS_LOG_INFO("Create channels.");
  CsmaHelper csma;
  csma.SetChannelAttribute("DataRate", DataRateValue(DataRate(5000000)));
  csma.SetChannelAttribute("Delay", TimeValue(MilliSeconds(2)));
  csma.SetDeviceAttribute("Mtu", UintegerValue(1400));
  NetDeviceContainer d = csma.Install(n);

  NS_LOG_INFO("Assign IP Addresses.");
  Ipv4AddressHelper ipv4;
  ipv4.SetBase("10.1.1.0", "255.255.255.0");
  Ipv4InterfaceContainer i = ipv4.Assign(d);

  NS_LOG_INFO("Create Applications.");

  uint16_t port = 9;
  UdpEchoServerHelper server(port);
  ApplicationContainer apps = server.Install(n.Get(1));
  apps.Start(Seconds(1.0));
  apps.Stop(Seconds(10.0));

  uint32_t packetSize = 1024;
  uint32_t maxPacketCount = 500;
  Time interPacketInterval = Seconds(0.01);
  UdpEchoClientHelper client(i.GetAddress(1), port);
  client.SetAttribute("MaxPackets", UintegerValue(maxPacketCount));
  client.SetAttribute("Interval", TimeValue(interPacketInterval));
  client.SetAttribute("PacketSize", UintegerValue(packetSize));
  apps = client.Install(n.Get(0));
  apps.Start(Seconds(2.0));
  apps.Stop(Seconds(10.0));

  AsciiTraceHelper ascii;
  csma.EnableAsciiAll(ascii.CreateFileStream("realtime-udp-echo.tr"));
  csma.EnablePcapAll("realtime-udp-echo", false);

  Simulator::Stop(Seconds(11.0));
  NS_LOG_INFO("Run Simulation.");
  Simulator::Run();
  Simulator::Destroy();
  NS_LOG_INFO("Done.");

  return 0;
}
