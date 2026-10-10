
#include "ns3/applications-module.h"
#include "ns3/core-module.h"
#include "ns3/internet-module.h"
#include "ns3/ipv4-list-routing-helper.h"
#include "ns3/ipv4-static-routing-helper.h"
#include "ns3/network-module.h"
#include "ns3/nix-vector-helper.h"
#include "ns3/point-to-point-module.h"

using namespace ns3;

NS_LOG_COMPONENT_DEFINE("NixSimpleMultiAddressExample");

int main(int argc, char *argv[]) {
  CommandLine cmd(__FILE__);
  cmd.Parse(argc, argv);

  LogComponentEnable("UdpEchoClientApplication", LOG_LEVEL_INFO);
  LogComponentEnable("UdpEchoServerApplication", LOG_LEVEL_INFO);

  NodeContainer nodes12;
  nodes12.Create(2);

  NodeContainer nodes23;
  nodes23.Add(nodes12.Get(1));
  nodes23.Create(1);

  NodeContainer nodes34;
  nodes34.Add(nodes23.Get(1));
  nodes34.Create(1);

  PointToPointHelper pointToPoint;
  pointToPoint.SetDeviceAttribute("DataRate", StringValue("5Mbps"));
  pointToPoint.SetChannelAttribute("Delay", StringValue("2ms"));

  NodeContainer allNodes =
      NodeContainer(nodes12, nodes23.Get(1), nodes34.Get(1));

  Ipv4NixVectorHelper nixRouting;
  InternetStackHelper stack;
  stack.SetRoutingHelper(nixRouting);
  stack.Install(allNodes);

  NetDeviceContainer devices12;
  NetDeviceContainer devices23;
  NetDeviceContainer devices34;
  devices12 = pointToPoint.Install(nodes12);
  devices23 = pointToPoint.Install(nodes23);
  devices34 = pointToPoint.Install(nodes34);

  Ipv4AddressHelper address1;
  address1.SetBase("10.1.1.0", "255.255.255.0");
  Ipv4AddressHelper address2;
  address2.SetBase("10.1.2.0", "255.255.255.0");
  Ipv4AddressHelper address3;
  address3.SetBase("10.1.3.0", "255.255.255.0");
  Ipv4AddressHelper address4;
  address4.SetBase("10.2.1.0", "255.255.255.0");
  Ipv4AddressHelper address5;
  address5.SetBase("10.2.3.0", "255.255.255.0");

  address1.Assign(devices12);
  address2.Assign(devices23);
  Ipv4InterfaceContainer interfaces34 = address3.Assign(devices34);
  Ipv4InterfaceContainer interfaces12 = address4.Assign(devices12);

  UdpEchoServerHelper echoServer(9);

  ApplicationContainer serverApps = echoServer.Install(nodes34.Get(1));
  serverApps.Start(Seconds(1.0));
  serverApps.Stop(Seconds(10.0));

  UdpEchoClientHelper echoClient(interfaces34.GetAddress(1), 9);
  echoClient.SetAttribute("MaxPackets", UintegerValue(1));
  echoClient.SetAttribute("Interval", TimeValue(Seconds(1.)));
  echoClient.SetAttribute("PacketSize", UintegerValue(1024));

  ApplicationContainer clientApps = echoClient.Install(nodes12.Get(0));
  clientApps.Start(Seconds(2.0));
  clientApps.Stop(Seconds(10.0));

  Ptr<OutputStreamWrapper> routingStream = Create<OutputStreamWrapper>(
      "nix-simple-multi-address.routes", std::ios::out);
  nixRouting.PrintRoutingPathAt(Seconds(3), nodes12.Get(0),
                                interfaces12.GetAddress(1, 1), routingStream);
  Ipv4NixVectorHelper::PrintRoutingTableAllAt(Seconds(4), routingStream);
  Simulator::Schedule(Seconds(5), &Ipv4AddressHelper::Assign, &address5,
                      devices34);

  Ipv4NixVectorHelper::PrintRoutingTableAllAt(Seconds(6), routingStream);

  nixRouting.PrintRoutingPathAt(Seconds(7), nodes12.Get(0),
                                Ipv4Address("10.2.3.2"), routingStream);

  Simulator::Run();
  Simulator::Destroy();
  return 0;
}
