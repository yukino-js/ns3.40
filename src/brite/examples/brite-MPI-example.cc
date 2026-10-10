
#include "Brite.h"

#include "ns3/applications-module.h"
#include "ns3/brite-module.h"
#include "ns3/core-module.h"
#include "ns3/internet-module.h"
#include "ns3/mobility-module.h"
#include "ns3/mpi-interface.h"
#include "ns3/network-module.h"
#include "ns3/nix-vector-helper.h"
#include "ns3/point-to-point-module.h"

#include <fstream>
#include <iostream>
#include <string>

#ifdef NS3_MPI
#include <mpi.h>
#endif

using namespace ns3;

NS_LOG_COMPONENT_DEFINE("BriteMPITest");

int main(int argc, char *argv[]) {
#ifdef NS3_MPI
  MpiInterface::Enable(&argc, &argv);
  GlobalValue::Bind("SimulatorImplementationType",
                    StringValue("ns3::DistributedSimulatorImpl"));

  LogComponentEnable("BriteMPITest", LOG_LEVEL_ALL);
  LogComponentEnable("TcpSocketBase", LOG_LEVEL_INFO);

  uint32_t systemId = MpiInterface::GetSystemId();
  uint32_t systemCount = MpiInterface::GetSize();

  NS_ASSERT_MSG(systemCount == 2,
                "This demonstration requires 2 and only 2 logical processors.");

  std::string confFile =
      "src/brite/examples/conf_files/TD_ASBarabasi_RTWaxman.conf";
  bool tracing = false;
  bool nix = false;

  CommandLine cmd(__FILE__);
  cmd.AddValue("confFile", "BRITE conf file", confFile);
  cmd.AddValue("tracing", "Enable or disable ascii tracing", tracing);
  cmd.AddValue("nix", "Enable or disable nix-vector routing", nix);

  cmd.Parse(argc, argv);

  BriteTopologyHelper bth(confFile);

  PointToPointHelper p2p;

  InternetStackHelper stack;

  if (nix) {
    Ipv4NixVectorHelper nixRouting;
    stack.SetRoutingHelper(nixRouting);
  }

  Ipv4AddressHelper address;
  address.SetBase("10.0.0.0", "255.255.255.252");

  bth.BuildBriteTopology(stack, systemCount);
  bth.AssignIpv4Addresses(address);

  NS_LOG_LOGIC("Number of AS created " << bth.GetNAs());

  uint16_t port = 5001;

  NodeContainer client;
  NodeContainer server;

  NS_LOG_LOGIC("AS 0 has been assigned to system "
               << bth.GetSystemNumberForAs(0));
  NS_LOG_LOGIC("As 1 has been assigned to system "
               << bth.GetSystemNumberForAs(1));

  client.Add(CreateObject<Node>(0));
  stack.Install(client);
  int numLeafNodesInAsZero = bth.GetNLeafNodesForAs(0);
  client.Add(bth.GetLeafNodeForAs(0, numLeafNodesInAsZero - 1));

  server.Add(CreateObject<Node>(1));
  stack.Install(server);
  int numLeafNodesInAsOne = bth.GetNLeafNodesForAs(1);
  server.Add(bth.GetLeafNodeForAs(1, numLeafNodesInAsOne - 1));

  p2p.SetDeviceAttribute("DataRate", StringValue("5Mbps"));
  p2p.SetChannelAttribute("Delay", StringValue("2ms"));

  NetDeviceContainer p2pClientDevices;
  NetDeviceContainer p2pServerDevices;

  p2pClientDevices = p2p.Install(client);
  p2pServerDevices = p2p.Install(server);

  address.SetBase("10.1.0.0", "255.255.0.0");
  Ipv4InterfaceContainer clientInterfaces;
  clientInterfaces = address.Assign(p2pClientDevices);

  address.SetBase("10.2.0.0", "255.255.0.0");
  Ipv4InterfaceContainer serverInterfaces;
  serverInterfaces = address.Assign(p2pServerDevices);

  if (!nix) {
    Ipv4GlobalRoutingHelper::PopulateRoutingTables();
  }

  ApplicationContainer sinkApps;

  if (systemId == 1) {
    Address sinkLocalAddress(InetSocketAddress(Ipv4Address::GetAny(), port));
    PacketSinkHelper packetSinkHelper("ns3::TcpSocketFactory",
                                      sinkLocalAddress);
    sinkApps.Add(packetSinkHelper.Install(server.Get(0)));
    sinkApps.Start(Seconds(0.0));
    sinkApps.Stop(Seconds(10.0));
  }

  if (systemId == 0) {
    OnOffHelper clientHelper("ns3::TcpSocketFactory", Address());
    clientHelper.SetAttribute(
        "OnTime", StringValue("ns3::ConstantRandomVariable[Constant=1]"));
    clientHelper.SetAttribute(
        "OffTime", StringValue("ns3::ConstantRandomVariable[Constant=0]"));

    ApplicationContainer clientApps;
    AddressValue remoteAddress(
        InetSocketAddress(serverInterfaces.GetAddress(0), port));
    clientHelper.SetAttribute("Remote", remoteAddress);
    clientApps.Add(clientHelper.Install(client.Get(0)));
    clientApps.Start(Seconds(1.0));
    clientApps.Stop(Seconds(9.0));
  }

  if (!nix) {
    Ipv4GlobalRoutingHelper::PopulateRoutingTables();
  }

  if (tracing) {
    AsciiTraceHelper ascii;
    p2p.EnableAsciiAll(ascii.CreateFileStream("briteLeaves.tr"));
  }

  Simulator::Stop(Seconds(200.0));
  Simulator::Run();
  Simulator::Destroy();

  if (systemId == 1) {
    Ptr<PacketSink> sink1 = DynamicCast<PacketSink>(sinkApps.Get(0));
    NS_LOG_DEBUG("Total Bytes Received: " << sink1->GetTotalRx());
  }

  MpiInterface::Disable();

  return 0;

#else
  NS_FATAL_ERROR("Can't use distributed simulator without MPI compiled in");
#endif
}
