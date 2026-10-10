
#include "ns3/applications-module.h"
#include "ns3/core-module.h"
#include "ns3/internet-module.h"
#include "ns3/ipv4-list-routing-helper.h"
#include "ns3/ipv4-static-routing-helper.h"
#include "ns3/ipv6-list-routing-helper.h"
#include "ns3/ipv6-static-routing-helper.h"
#include "ns3/network-module.h"
#include "ns3/nix-vector-helper.h"
#include "ns3/point-to-point-module.h"

using namespace ns3;

NS_LOG_COMPONENT_DEFINE("NixSimpleExample");

int main(int argc, char *argv[]) {
  bool useIpv6 = false;

  CommandLine cmd(__FILE__);
  cmd.AddValue("useIPv6", "Use IPv6 instead of IPv4", useIpv6);
  cmd.Parse(argc, argv);

  LogComponentEnable("UdpEchoClientApplication", LOG_LEVEL_INFO);
  LogComponentEnable("UdpEchoServerApplication", LOG_LEVEL_INFO);

  NodeContainer nodes;
  nodes.Create(4);

  PointToPointHelper pointToPoint;
  pointToPoint.SetDeviceAttribute("DataRate", StringValue("5Mbps"));
  pointToPoint.SetChannelAttribute("Delay", StringValue("2ms"));

  NetDeviceContainer devices01;
  NetDeviceContainer devices12;
  NetDeviceContainer devices23;
  NetDeviceContainer devices02;
  devices01 = pointToPoint.Install(NodeContainer(nodes.Get(0), nodes.Get(1)));
  devices12 = pointToPoint.Install(NodeContainer(nodes.Get(1), nodes.Get(2)));
  devices23 = pointToPoint.Install(NodeContainer(nodes.Get(2), nodes.Get(3)));
  devices02 = pointToPoint.Install(NodeContainer(nodes.Get(0), nodes.Get(2)));

  Address udpServerAddress;

  if (!useIpv6) {
    Ipv4NixVectorHelper nixRouting;
    InternetStackHelper stack;
    stack.SetRoutingHelper(nixRouting);
    stack.Install(nodes);

    Ipv4AddressHelper address1;
    address1.SetBase("10.1.1.0", "255.255.255.0");
    Ipv4AddressHelper address2;
    address2.SetBase("10.1.2.0", "255.255.255.0");
    Ipv4AddressHelper address3;
    address3.SetBase("10.1.3.0", "255.255.255.0");
    Ipv4AddressHelper address4;
    address4.SetBase("10.1.4.0", "255.255.255.0");

    Ipv4InterfaceContainer interfaces01 = address1.Assign(devices01);
    Ipv4InterfaceContainer interfaces12 = address2.Assign(devices12);
    Ipv4InterfaceContainer interfaces23 = address3.Assign(devices23);
    Ipv4InterfaceContainer interfaces02 = address4.Assign(devices02);

    udpServerAddress = interfaces23.GetAddress(1);

    Ptr<OutputStreamWrapper> routingStream =
        Create<OutputStreamWrapper>("nix-simple-ipv4.routes", std::ios::out);
    nixRouting.PrintRoutingPathAt(Seconds(3), nodes.Get(0),
                                  interfaces23.GetAddress(1), routingStream);
    nixRouting.PrintRoutingPathAt(Seconds(5), nodes.Get(1),
                                  interfaces23.GetAddress(1), routingStream);
    nixRouting.PrintRoutingPathAt(Seconds(6), nodes.Get(2),
                                  interfaces01.GetAddress(0), routingStream);
    nixRouting.PrintRoutingPathAt(Seconds(7), nodes.Get(1),
                                  interfaces01.GetAddress(1), routingStream);
    Ipv4NixVectorHelper::PrintRoutingTableAllAt(Seconds(8), routingStream);
  } else {
    Ipv6NixVectorHelper nixRouting;
    InternetStackHelper stack;
    stack.SetRoutingHelper(nixRouting);
    stack.Install(nodes);

    Ipv6AddressHelper address1;
    address1.SetBase(Ipv6Address("2001:1::"), Ipv6Prefix(64));
    Ipv6AddressHelper address2;
    address2.SetBase(Ipv6Address("2001:2::"), Ipv6Prefix(64));
    Ipv6AddressHelper address3;
    address3.SetBase(Ipv6Address("2001:3::"), Ipv6Prefix(64));
    Ipv6AddressHelper address4;
    address4.SetBase(Ipv6Address("2001:4::"), Ipv6Prefix(64));

    Ipv6InterfaceContainer interfaces01 = address1.Assign(devices01);
    Ipv6InterfaceContainer interfaces12 = address2.Assign(devices12);
    Ipv6InterfaceContainer interfaces23 = address3.Assign(devices23);
    Ipv6InterfaceContainer interfaces02 = address4.Assign(devices02);

    udpServerAddress = interfaces23.GetAddress(1, 1);

    Ptr<OutputStreamWrapper> routingStream =
        Create<OutputStreamWrapper>("nix-simple-ipv6.routes", std::ios::out);
    nixRouting.PrintRoutingPathAt(Seconds(3), nodes.Get(0),
                                  interfaces23.GetAddress(1, 1), routingStream);
    nixRouting.PrintRoutingPathAt(Seconds(5), nodes.Get(1),
                                  interfaces23.GetAddress(1, 1), routingStream);
    nixRouting.PrintRoutingPathAt(Seconds(6), nodes.Get(2),
                                  interfaces01.GetAddress(0, 1), routingStream);
    nixRouting.PrintRoutingPathAt(Seconds(7), nodes.Get(1),
                                  interfaces01.GetAddress(1, 1), routingStream);
    Ipv6NixVectorHelper::PrintRoutingTableAllAt(Seconds(8), routingStream);
  }

  UdpEchoServerHelper echoServer(9);

  ApplicationContainer serverApps = echoServer.Install(nodes.Get(3));
  serverApps.Start(Seconds(1.0));
  serverApps.Stop(Seconds(10.0));

  UdpEchoClientHelper echoClient(udpServerAddress, 9);
  echoClient.SetAttribute("MaxPackets", UintegerValue(1));
  echoClient.SetAttribute("Interval", TimeValue(Seconds(1.)));
  echoClient.SetAttribute("PacketSize", UintegerValue(1024));

  ApplicationContainer clientApps = echoClient.Install(nodes.Get(0));
  clientApps.Start(Seconds(2.0));
  clientApps.Stop(Seconds(10.0));

  Simulator::Run();
  Simulator::Destroy();
  return 0;
}
