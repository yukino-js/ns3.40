

#include "mpi-test-fixtures.h"

#include "ns3/core-module.h"
#include "ns3/internet-stack-helper.h"
#include "ns3/ipv4-address-helper.h"
#include "ns3/ipv4-global-routing-helper.h"
#include "ns3/ipv4-list-routing-helper.h"
#include "ns3/ipv4-static-routing-helper.h"
#include "ns3/mpi-interface.h"
#include "ns3/network-module.h"
#include "ns3/nix-vector-helper.h"
#include "ns3/on-off-helper.h"
#include "ns3/packet-sink-helper.h"
#include "ns3/packet-sink.h"
#include "ns3/point-to-point-helper.h"

#include <mpi.h>

using namespace ns3;

NS_LOG_COMPONENT_DEFINE("SimpleDistributedMpiComm");

const int NS_COLOR = 1;
const int NOT_NS_COLOR = NS_COLOR + 1;

void ReportRank(int color, MPI_Comm splitComm) {
  int otherId = 0;
  int otherSize = 1;

  MPI_Comm_rank(splitComm, &otherId);
  MPI_Comm_size(splitComm, &otherSize);

  if (color == NS_COLOR) {
    RANK0COUT("ns-3 rank:  ");
  } else {
    RANK0COUT("Other rank: ");
  }

  RANK0COUTAPPEND("in MPI_COMM_WORLD: " << SinkTracer::GetWorldRank() << ":"
                                        << SinkTracer::GetWorldSize()
                                        << ", in splitComm: " << otherId << ":"
                                        << otherSize << std::endl);
}

int main(int argc, char *argv[]) {
  bool nix = true;
  bool nullmsg = false;
  bool tracing = false;
  bool init = false;
  bool verbose = false;
  bool testing = false;

  CommandLine cmd(__FILE__);
  cmd.AddValue("nix", "Enable the use of nix-vector or global routing", nix);
  cmd.AddValue("nullmsg",
               "Enable the use of null-message synchronization (instead of "
               "granted time window)",
               nullmsg);
  cmd.AddValue("tracing", "Enable pcap tracing", tracing);
  cmd.AddValue("init", "ns-3 should initialize MPI by calling MPI_Init", init);
  cmd.AddValue("verbose", "verbose output", verbose);
  cmd.AddValue("test", "Enable regression test output", testing);
  cmd.Parse(argc, argv);

  if (nullmsg) {
    GlobalValue::Bind("SimulatorImplementationType",
                      StringValue("ns3::NullMessageSimulatorImpl"));
  } else {
    GlobalValue::Bind("SimulatorImplementationType",
                      StringValue("ns3::DistributedSimulatorImpl"));
  }

  if (init) {
    MPI_Init(&argc, &argv);
  } else {
    MpiInterface::Enable(&argc, &argv);
  }

  SinkTracer::Init();

  auto worldSize = SinkTracer::GetWorldSize();
  auto worldRank = SinkTracer::GetWorldRank();

  if ((!init) && (worldSize != 2)) {
    RANK0COUT("This simulation requires exactly 2 logical processors if --init "
              "is not set."
              << std::endl);
    return 1;
  }

  if (worldSize < 2) {
    RANK0COUT("This simulation requires 2  or more logical processors."
              << std::endl);
    return 1;
  }

  bool freeComm = false;
  MPI_Comm splitComm = MPI_COMM_WORLD;
  std::string ns3Ranks;
  int color = MPI_UNDEFINED;

  if (worldSize == 2) {
    std::stringstream ss;
    color = NS_COLOR;
    ss << "MPI_COMM_WORLD (" << worldSize << " ranks)";
    ns3Ranks = ss.str();
    splitComm = MPI_COMM_WORLD;
    freeComm = false;
  } else {

    if (worldRank == 1 || worldRank == 2) {
      color = NS_COLOR;
    } else {
      color = NOT_NS_COLOR;
    }
    std::stringstream ss;
    ss << "Split [1-2] (out of " << worldSize << " ranks) from MPI_COMM_WORLD";
    ns3Ranks = ss.str();

    MPI_Comm_split(MPI_COMM_WORLD, color, worldRank, &splitComm);
    freeComm = true;
  }

  if (init) {
    MpiInterface::Enable(splitComm);
  }

  RANK0COUT(cmd.GetName() << "\n");
  RANK0COUT("\n");
  RANK0COUT("Configuration:\n");
  RANK0COUT("Routing:           " << (nix ? "nix-vector" : "global") << "\n");
  RANK0COUT("Synchronization:   "
            << (nullmsg ? "null-message" : "granted time window (YAWNS)")
            << "\n");
  RANK0COUT("MPI_Init called:   "
            << (init ? "explicitly by this program"
                     : "implicitly by ns3::MpiInterface::Enable()")
            << "\n");
  RANK0COUT("ns-3 Communicator: " << ns3Ranks << "\n");
  RANK0COUT("PCAP tracing:      " << (tracing ? "" : "not") << " enabled\n");
  RANK0COUT("\n");
  RANK0COUT("Rank assignments:" << std::endl);

  if (worldRank == 0) {
    ReportRank(color, splitComm);
  }

  if (verbose) {
    int token;

    if (worldRank == 0) {
      token = 1;
    } else {
      MPI_Recv(&token, 1, MPI_INT, worldRank - 1, 0, MPI_COMM_WORLD,
               MPI_STATUS_IGNORE);
      ReportRank(color, splitComm);
    }

    MPI_Send(&token, 1, MPI_INT, (worldRank + 1) % worldSize, 0,
             MPI_COMM_WORLD);

    if (worldRank == 0) {
      MPI_Recv(&token, 1, MPI_INT, worldSize - 1, 0, MPI_COMM_WORLD,
               MPI_STATUS_IGNORE);
    }
  }

  RANK0COUT(std::endl);

  if (color != NS_COLOR) {

    MpiInterface::Disable();

    if (init) {
      MPI_Finalize();
    }

    return 0;
  }

  if (verbose) {
    LogComponentEnable("PacketSink", LOG_LEVEL_INFO);
  }

  uint32_t systemId = MpiInterface::GetSystemId();
  uint32_t systemCount = MpiInterface::GetSize();

  if (systemCount < 2) {
    RANK0COUT("This simulation requires at least 2 logical processors."
              << std::endl);
    return 1;
  }

  Config::SetDefault("ns3::OnOffApplication::PacketSize", UintegerValue(512));
  Config::SetDefault("ns3::OnOffApplication::DataRate", StringValue("1Mbps"));
  Config::SetDefault("ns3::OnOffApplication::MaxBytes", UintegerValue(512));

  NodeContainer leftLeafNodes;
  leftLeafNodes.Create(4, 0);

  NodeContainer routerNodes;
  Ptr<Node> routerNode1 = CreateObject<Node>(0);
  Ptr<Node> routerNode2 = CreateObject<Node>(1);
  routerNodes.Add(routerNode1);
  routerNodes.Add(routerNode2);

  NodeContainer rightLeafNodes;
  rightLeafNodes.Create(4, 1);

  PointToPointHelper routerLink;
  routerLink.SetDeviceAttribute("DataRate", StringValue("5Mbps"));
  routerLink.SetChannelAttribute("Delay", StringValue("5ms"));

  PointToPointHelper leafLink;
  leafLink.SetDeviceAttribute("DataRate", StringValue("1Mbps"));
  leafLink.SetChannelAttribute("Delay", StringValue("2ms"));

  NetDeviceContainer routerDevices;
  routerDevices = routerLink.Install(routerNodes);

  NetDeviceContainer leftRouterDevices;
  NetDeviceContainer leftLeafDevices;
  for (uint32_t i = 0; i < 4; ++i) {
    NetDeviceContainer temp =
        leafLink.Install(leftLeafNodes.Get(i), routerNodes.Get(0));
    leftLeafDevices.Add(temp.Get(0));
    leftRouterDevices.Add(temp.Get(1));
  }

  NetDeviceContainer rightRouterDevices;
  NetDeviceContainer rightLeafDevices;
  for (uint32_t i = 0; i < 4; ++i) {
    NetDeviceContainer temp =
        leafLink.Install(rightLeafNodes.Get(i), routerNodes.Get(1));
    rightLeafDevices.Add(temp.Get(0));
    rightRouterDevices.Add(temp.Get(1));
  }

  InternetStackHelper stack;
  Ipv4NixVectorHelper nixRouting;
  Ipv4StaticRoutingHelper staticRouting;

  Ipv4ListRoutingHelper list;
  list.Add(staticRouting, 0);
  list.Add(nixRouting, 10);

  if (nix) {
    stack.SetRoutingHelper(list);
  }

  stack.InstallAll();

  Ipv4InterfaceContainer routerInterfaces;
  Ipv4InterfaceContainer leftLeafInterfaces;
  Ipv4InterfaceContainer leftRouterInterfaces;
  Ipv4InterfaceContainer rightLeafInterfaces;
  Ipv4InterfaceContainer rightRouterInterfaces;

  Ipv4AddressHelper leftAddress;
  leftAddress.SetBase("10.1.1.0", "255.255.255.0");

  Ipv4AddressHelper routerAddress;
  routerAddress.SetBase("10.2.1.0", "255.255.255.0");

  Ipv4AddressHelper rightAddress;
  rightAddress.SetBase("10.3.1.0", "255.255.255.0");

  routerInterfaces = routerAddress.Assign(routerDevices);

  for (uint32_t i = 0; i < 4; ++i) {
    NetDeviceContainer ndc;
    ndc.Add(leftLeafDevices.Get(i));
    ndc.Add(leftRouterDevices.Get(i));
    Ipv4InterfaceContainer ifc = leftAddress.Assign(ndc);
    leftLeafInterfaces.Add(ifc.Get(0));
    leftRouterInterfaces.Add(ifc.Get(1));
    leftAddress.NewNetwork();
  }

  for (uint32_t i = 0; i < 4; ++i) {
    NetDeviceContainer ndc;
    ndc.Add(rightLeafDevices.Get(i));
    ndc.Add(rightRouterDevices.Get(i));
    Ipv4InterfaceContainer ifc = rightAddress.Assign(ndc);
    rightLeafInterfaces.Add(ifc.Get(0));
    rightRouterInterfaces.Add(ifc.Get(1));
    rightAddress.NewNetwork();
  }

  if (!nix) {
    Ipv4GlobalRoutingHelper::PopulateRoutingTables();
  }

  if (tracing) {
    if (systemId == 0) {
      routerLink.EnablePcap("router-left", routerDevices, true);
      leafLink.EnablePcap("leaf-left", leftLeafDevices, true);
    }

    if (systemId == 1) {
      routerLink.EnablePcap("router-right", routerDevices, true);
      leafLink.EnablePcap("leaf-right", rightLeafDevices, true);
    }
  }

  uint16_t port = 50000;
  if (systemId == 1) {
    Address sinkLocalAddress(InetSocketAddress(Ipv4Address::GetAny(), port));
    PacketSinkHelper sinkHelper("ns3::UdpSocketFactory", sinkLocalAddress);
    ApplicationContainer sinkApp;
    for (uint32_t i = 0; i < 4; ++i) {
      auto apps = sinkHelper.Install(rightLeafNodes.Get(i));
      auto sink = DynamicCast<PacketSink>(apps.Get(0));
      NS_ASSERT_MSG(sink, "Couldn't get PacketSink application.");
      if (testing) {
        sink->TraceConnectWithoutContext("RxWithAddresses",
                                         MakeCallback(&SinkTracer::SinkTrace));
      }
      sinkApp.Add(apps);
    }
    sinkApp.Start(Seconds(1.0));
    sinkApp.Stop(Seconds(5));
  }

  if (systemId == 0) {
    OnOffHelper clientHelper("ns3::UdpSocketFactory", Address());
    clientHelper.SetAttribute(
        "OnTime", StringValue("ns3::ConstantRandomVariable[Constant=1]"));
    clientHelper.SetAttribute(
        "OffTime", StringValue("ns3::ConstantRandomVariable[Constant=0]"));

    ApplicationContainer clientApps;
    for (uint32_t i = 0; i < 4; ++i) {
      AddressValue remoteAddress(
          InetSocketAddress(rightLeafInterfaces.GetAddress(i), port));
      clientHelper.SetAttribute("Remote", remoteAddress);
      clientApps.Add(clientHelper.Install(leftLeafNodes.Get(i)));
    }
    clientApps.Start(Seconds(1.0));
    clientApps.Stop(Seconds(5));
  }

  RANK0COUT(std::endl);

  Simulator::Stop(Seconds(5));
  Simulator::Run();
  Simulator::Destroy();

  if (freeComm) {
    MPI_Comm_free(&splitComm);
  }

  if (testing) {
    SinkTracer::Verify(4);
  }

  MpiInterface::Disable();

  if (init) {
    MPI_Finalize();
  }

  return 0;
}
