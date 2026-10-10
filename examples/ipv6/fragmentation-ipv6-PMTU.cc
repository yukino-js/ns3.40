

#include "ns3/applications-module.h"
#include "ns3/core-module.h"
#include "ns3/csma-module.h"
#include "ns3/internet-module.h"
#include "ns3/ipv6-routing-table-entry.h"
#include "ns3/ipv6-static-routing-helper.h"
#include "ns3/network-module.h"
#include "ns3/point-to-point-module.h"

#include <fstream>

using namespace ns3;

NS_LOG_COMPONENT_DEFINE("FragmentationIpv6PmtuExample");

int main(int argc, char **argv) {
  bool verbose = false;

  CommandLine cmd(__FILE__);
  cmd.AddValue("verbose", "turn on log components", verbose);
  cmd.Parse(argc, argv);

  if (verbose) {
    LogComponentEnable("Ipv6L3Protocol", LOG_LEVEL_ALL);
    LogComponentEnable("Icmpv6L4Protocol", LOG_LEVEL_ALL);
    LogComponentEnable("Ipv6StaticRouting", LOG_LEVEL_ALL);
    LogComponentEnable("Ipv6Interface", LOG_LEVEL_ALL);
    LogComponentEnable("UdpEchoClientApplication", LOG_LEVEL_ALL);
    LogComponentEnable("UdpEchoServerApplication", LOG_LEVEL_ALL);
  }
  LogComponentEnable("UdpEchoClientApplication", LOG_LEVEL_INFO);

  NS_LOG_INFO("Create nodes.");
  Ptr<Node> n0 = CreateObject<Node>();
  Ptr<Node> r1 = CreateObject<Node>();
  Ptr<Node> n1 = CreateObject<Node>();
  Ptr<Node> r2 = CreateObject<Node>();
  Ptr<Node> n2 = CreateObject<Node>();

  NodeContainer net1(n0, r1);
  NodeContainer net2(r1, n1, r2);
  NodeContainer net3(r2, n2);
  NodeContainer all(n0, r1, n1, r2, n2);

  NS_LOG_INFO("Create IPv6 Internet Stack");
  InternetStackHelper internetv6;
  internetv6.Install(all);

  NS_LOG_INFO("Create channels.");
  CsmaHelper csma;
  csma.SetChannelAttribute("DataRate", DataRateValue(5000000));
  csma.SetChannelAttribute("Delay", TimeValue(MilliSeconds(2)));
  csma.SetDeviceAttribute("Mtu", UintegerValue(2000));
  NetDeviceContainer d2 = csma.Install(net2);

  csma.SetDeviceAttribute("Mtu", UintegerValue(5000));
  NetDeviceContainer d1 = csma.Install(net1);

  PointToPointHelper pointToPoint;
  pointToPoint.SetDeviceAttribute("DataRate", DataRateValue(5000000));
  pointToPoint.SetChannelAttribute("Delay", StringValue("2ms"));
  pointToPoint.SetDeviceAttribute("Mtu", UintegerValue(1500));
  NetDeviceContainer d3 = pointToPoint.Install(net3);

  NS_LOG_INFO("Create networks and assign IPv6 Addresses.");
  Ipv6AddressHelper ipv6;

  ipv6.SetBase(Ipv6Address("2001:1::"), Ipv6Prefix(64));
  Ipv6InterfaceContainer i1 = ipv6.Assign(d1);
  i1.SetForwarding(1, true);
  i1.SetDefaultRouteInAllNodes(1);

  ipv6.SetBase(Ipv6Address("2001:2::"), Ipv6Prefix(64));
  Ipv6InterfaceContainer i2 = ipv6.Assign(d2);
  i2.SetForwarding(0, true);
  i2.SetDefaultRouteInAllNodes(0);
  i2.SetForwarding(1, true);
  i2.SetDefaultRouteInAllNodes(0);
  i2.SetForwarding(2, true);
  i2.SetDefaultRouteInAllNodes(2);

  ipv6.SetBase(Ipv6Address("2001:3::"), Ipv6Prefix(64));
  Ipv6InterfaceContainer i3 = ipv6.Assign(d3);
  i3.SetForwarding(0, true);
  i3.SetDefaultRouteInAllNodes(0);

  Ptr<OutputStreamWrapper> routingStream =
      Create<OutputStreamWrapper>(&std::cout);
  Ipv6RoutingHelper::PrintRoutingTableAt(Seconds(0), r1, routingStream);

  UdpEchoServerHelper echoServer(42);
  ApplicationContainer serverApps = echoServer.Install(n2);
  serverApps.Start(Seconds(0.0));
  serverApps.Stop(Seconds(30.0));

  uint32_t maxPacketCount = 5;

  uint32_t packetSizeN1 = 1600;
  UdpEchoClientHelper echoClient(i3.GetAddress(1, 1), 42);
  echoClient.SetAttribute("PacketSize", UintegerValue(packetSizeN1));
  echoClient.SetAttribute("MaxPackets", UintegerValue(maxPacketCount));
  ApplicationContainer clientAppsN1 = echoClient.Install(n1);
  clientAppsN1.Start(Seconds(2.0));
  clientAppsN1.Stop(Seconds(10.0));

  uint32_t packetSizeN2 = 4000;
  echoClient.SetAttribute("PacketSize", UintegerValue(packetSizeN2));
  echoClient.SetAttribute("MaxPackets", UintegerValue(maxPacketCount));
  ApplicationContainer clientAppsN2 = echoClient.Install(n1);
  clientAppsN2.Start(Seconds(11.0));
  clientAppsN2.Stop(Seconds(20.0));

  AsciiTraceHelper ascii;
  csma.EnableAsciiAll(ascii.CreateFileStream("fragmentation-ipv6-PMTU.tr"));
  csma.EnablePcapAll(std::string("fragmentation-ipv6-PMTU"), true);
  pointToPoint.EnablePcapAll(std::string("fragmentation-ipv6-PMTU"), true);

  NS_LOG_INFO("Run Simulation.");
  Simulator::Run();
  Simulator::Destroy();
  NS_LOG_INFO("Done.");

  return 0;
}
