

#include "mpi-test-fixtures.h"

#include "ns3/core-module.h"
#include "ns3/internet-stack-helper.h"
#include "ns3/ipv4-address-helper.h"
#include "ns3/ipv4-global-routing-helper.h"
#include "ns3/mpi-interface.h"
#include "ns3/mtp-interface.h"
#include "ns3/network-module.h"
#include "ns3/nix-vector-helper.h"
#include "ns3/on-off-helper.h"
#include "ns3/packet-sink-helper.h"
#include "ns3/point-to-point-helper.h"

#include <iomanip>
#include <mpi.h>

using namespace ns3;

NS_LOG_COMPONENT_DEFINE("SimpleHybrid");

int main(int argc, char *argv[]) {
  LogComponentEnable("LogicalProcess", LOG_LEVEL_INFO);

  bool nix = true;
  bool tracing = false;
  bool testing = false;
  bool verbose = false;

  CommandLine cmd(__FILE__);
  cmd.AddValue("nix", "Enable the use of nix-vector or global routing", nix);
  cmd.AddValue("tracing", "Enable pcap tracing", tracing);
  cmd.AddValue("verbose", "verbose output", verbose);
  cmd.AddValue("test", "Enable regression test output", testing);
  cmd.Parse(argc, argv);

  MtpInterface::Enable();
  MpiInterface::Enable(&argc, &argv);

  SinkTracer::Init();

  if (verbose) {
    LogComponentEnable(
        "PacketSink",
        (LogLevel)(LOG_LEVEL_INFO | LOG_PREFIX_NODE | LOG_PREFIX_TIME));
  }

  uint32_t systemId = MpiInterface::GetSystemId();
  uint32_t systemCount = MpiInterface::GetSize();

  if (systemCount != 2) {
    std::cout << "This simulation requires 2 and only 2 logical processors."
              << std::endl;
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
  if (nix) {
    Ipv4NixVectorHelper nixRouting;
    stack.SetRoutingHelper(nixRouting);
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

  if (tracing == true) {
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
      sinkApp.Add(sinkHelper.Install(rightLeafNodes.Get(i)));
      if (testing) {
        sinkApp.Get(i)->TraceConnectWithoutContext(
            "RxWithAddresses", MakeCallback(&SinkTracer::SinkTrace));
      }
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

  Simulator::Stop(Seconds(5));
  Simulator::Run();
  Simulator::Destroy();

  if (testing) {
    SinkTracer::Verify(4);
  }

  MpiInterface::Disable();
  return 0;
}
