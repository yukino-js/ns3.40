

#include "ns3/applications-module.h"
#include "ns3/core-module.h"
#include "ns3/csma-module.h"
#include "ns3/internet-module.h"

#include <fstream>

using namespace ns3;

NS_LOG_COMPONENT_DEFINE("UdpEchoExample");

int main(int argc, char *argv[]) {
#if 0
  LogComponentEnable ("UdpEchoExample", LOG_LEVEL_INFO);
  LogComponentEnable ("UdpEchoClientApplication", LOG_LEVEL_ALL);
  LogComponentEnable ("UdpEchoServerApplication", LOG_LEVEL_ALL);
#endif
  bool useV6 = false;
  Address serverAddress;

  CommandLine cmd(__FILE__);
  cmd.AddValue("useIpv6", "Use Ipv6", useV6);
  cmd.Parse(argc, argv);
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
  if (!useV6) {
    Ipv4AddressHelper ipv4;
    ipv4.SetBase("10.1.1.0", "255.255.255.0");
    Ipv4InterfaceContainer i = ipv4.Assign(d);
    serverAddress = Address(i.GetAddress(1));
  } else {
    Ipv6AddressHelper ipv6;
    ipv6.SetBase("2001:0000:f00d:cafe::", Ipv6Prefix(64));
    Ipv6InterfaceContainer i6 = ipv6.Assign(d);
    serverAddress = Address(i6.GetAddress(1, 1));
  }

  NS_LOG_INFO("Create Applications.");
  uint16_t port = 9;
  UdpEchoServerHelper server(port);
  ApplicationContainer apps = server.Install(n.Get(1));
  apps.Start(Seconds(1.0));
  apps.Stop(Seconds(10.0));

  uint32_t packetSize = 1024;
  uint32_t maxPacketCount = 1;
  Time interPacketInterval = Seconds(1.);
  UdpEchoClientHelper client(serverAddress, port);
  client.SetAttribute("MaxPackets", UintegerValue(maxPacketCount));
  client.SetAttribute("Interval", TimeValue(interPacketInterval));
  client.SetAttribute("PacketSize", UintegerValue(packetSize));
  apps = client.Install(n.Get(0));
  apps.Start(Seconds(2.0));
  apps.Stop(Seconds(10.0));

#if 0
  client.SetFill (apps.Get (0), "Hello World");

  client.SetFill (apps.Get (0), 0xa5, 1024);

  uint8_t fill[] = { 0, 1, 2, 3, 4, 5, 6};
  client.SetFill (apps.Get (0), fill, sizeof(fill), 1024);
#endif

  AsciiTraceHelper ascii;
  csma.EnableAsciiAll(ascii.CreateFileStream("udp-echo.tr"));
  csma.EnablePcapAll("udp-echo", false);

  NS_LOG_INFO("Run Simulation.");
  Simulator::Run();
  Simulator::Destroy();
  NS_LOG_INFO("Done.");

  return 0;
}
