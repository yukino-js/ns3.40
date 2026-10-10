

#include "mpi-test-fixtures.h"

#include "ns3/applications-module.h"
#include "ns3/core-module.h"
#include "ns3/csma-module.h"
#include "ns3/internet-module.h"
#include "ns3/mobility-module.h"
#include "ns3/mpi-module.h"
#include "ns3/network-module.h"
#include "ns3/point-to-point-module.h"
#include "ns3/ssid.h"
#include "ns3/yans-wifi-helper.h"

#include <iomanip>

using namespace ns3;

NS_LOG_COMPONENT_DEFINE("ThirdExampleDistributed");

int main(int argc, char *argv[]) {
  bool verbose = false;
  uint32_t nCsma = 3;
  uint32_t nWifi = 3;
  bool tracing = false;
  bool nullmsg = false;
  bool testing = false;

  CommandLine cmd(__FILE__);
  cmd.AddValue("nCsma", "Number of \"extra\" CSMA nodes/devices", nCsma);
  cmd.AddValue("nWifi", "Number of wifi STA devices", nWifi);
  cmd.AddValue("verbose", "Tell echo applications to log if true", verbose);
  cmd.AddValue("tracing", "Enable pcap tracing", tracing);
  cmd.AddValue("nullmsg", "Enable the use of null-message synchronization",
               nullmsg);
  cmd.AddValue("test", "Enable regression test output", testing);

  cmd.Parse(argc, argv);

  if (nWifi > 18) {
    std::cout << "nWifi should be 18 or less; otherwise grid layout exceeds "
                 "the bounding box"
              << std::endl;
    return 1;
  }

  if (verbose) {
    LogComponentEnable(
        "UdpEchoClientApplication",
        (LogLevel)(LOG_LEVEL_INFO | LOG_PREFIX_NODE | LOG_PREFIX_TIME));
    LogComponentEnable(
        "UdpEchoServerApplication",
        (LogLevel)(LOG_LEVEL_INFO | LOG_PREFIX_NODE | LOG_PREFIX_TIME));
  }

  uint32_t systemId = 0;
  uint32_t systemCount = 1;

  if (nullmsg) {
    GlobalValue::Bind("SimulatorImplementationType",
                      StringValue("ns3::NullMessageSimulatorImpl"));
  } else {
    GlobalValue::Bind("SimulatorImplementationType",
                      StringValue("ns3::DistributedSimulatorImpl"));
  }

  MpiInterface::Enable(&argc, &argv);

  SinkTracer::Init();

  systemId = MpiInterface::GetSystemId();
  systemCount = MpiInterface::GetSize();

  if (systemCount != 2) {
    std::cout << "This simulation requires 2 and only 2 logical processors."
              << std::endl;
    return 1;
  }

  uint32_t systemWifi = 0;

  uint32_t systemCsma = systemCount - 1;

  NodeContainer p2pNodes;
  Ptr<Node> p2pNode1 = CreateObject<Node>(systemWifi);
  Ptr<Node> p2pNode2 = CreateObject<Node>(systemCsma);
  p2pNodes.Add(p2pNode1);
  p2pNodes.Add(p2pNode2);

  PointToPointHelper pointToPoint;
  pointToPoint.SetDeviceAttribute("DataRate", StringValue("5Mbps"));
  pointToPoint.SetChannelAttribute("Delay", StringValue("2ms"));

  NetDeviceContainer p2pDevices;
  p2pDevices = pointToPoint.Install(p2pNodes);

  NodeContainer csmaNodes;
  csmaNodes.Add(p2pNodes.Get(1));
  csmaNodes.Create(nCsma, systemCsma);

  CsmaHelper csma;
  csma.SetChannelAttribute("DataRate", StringValue("100Mbps"));
  csma.SetChannelAttribute("Delay", TimeValue(NanoSeconds(6560)));

  NetDeviceContainer csmaDevices;
  csmaDevices = csma.Install(csmaNodes);

  NodeContainer wifiStaNodes;
  wifiStaNodes.Create(nWifi, systemWifi);
  NodeContainer wifiApNode = p2pNodes.Get(0);

  YansWifiChannelHelper channel = YansWifiChannelHelper::Default();
  YansWifiPhyHelper phy;
  phy.SetChannel(channel.Create());

  WifiMacHelper mac;
  Ssid ssid = Ssid("ns-3-ssid");

  WifiHelper wifi;

  NetDeviceContainer staDevices;
  mac.SetType("ns3::StaWifiMac", "Ssid", SsidValue(ssid), "ActiveProbing",
              BooleanValue(false));
  staDevices = wifi.Install(phy, mac, wifiStaNodes);

  NetDeviceContainer apDevices;
  mac.SetType("ns3::ApWifiMac", "Ssid", SsidValue(ssid));
  apDevices = wifi.Install(phy, mac, wifiApNode);

  MobilityHelper mobility;

  mobility.SetPositionAllocator(
      "ns3::GridPositionAllocator", "MinX", DoubleValue(0.0), "MinY",
      DoubleValue(0.0), "DeltaX", DoubleValue(5.0), "DeltaY", DoubleValue(10.0),
      "GridWidth", UintegerValue(3), "LayoutType", StringValue("RowFirst"));

  mobility.SetMobilityModel("ns3::RandomWalk2dMobilityModel", "Bounds",
                            RectangleValue(Rectangle(-50, 50, -50, 50)));
  mobility.Install(wifiStaNodes);

  mobility.SetMobilityModel("ns3::ConstantPositionMobilityModel");
  mobility.Install(wifiApNode);

  InternetStackHelper stack;
  stack.Install(csmaNodes);
  stack.Install(wifiApNode);
  stack.Install(wifiStaNodes);

  Ipv4AddressHelper address;

  address.SetBase("10.1.1.0", "255.255.255.0");
  Ipv4InterfaceContainer p2pInterfaces;
  p2pInterfaces = address.Assign(p2pDevices);

  address.SetBase("10.1.2.0", "255.255.255.0");
  Ipv4InterfaceContainer csmaInterfaces;
  csmaInterfaces = address.Assign(csmaDevices);

  address.SetBase("10.1.3.0", "255.255.255.0");
  address.Assign(staDevices);
  address.Assign(apDevices);

  if (systemId == systemCsma) {
    UdpEchoServerHelper echoServer(9);

    ApplicationContainer serverApps = echoServer.Install(csmaNodes.Get(nCsma));
    serverApps.Start(Seconds(1.0));
    serverApps.Stop(Seconds(10.0));

    if (testing) {
      serverApps.Get(0)->TraceConnectWithoutContext(
          "RxWithAddresses", MakeCallback(&SinkTracer::SinkTrace));
    }
  }

  if (systemId == systemWifi) {
    UdpEchoClientHelper echoClient(csmaInterfaces.GetAddress(nCsma), 9);
    echoClient.SetAttribute("MaxPackets", UintegerValue(1));
    echoClient.SetAttribute("Interval", TimeValue(Seconds(1.0)));
    echoClient.SetAttribute("PacketSize", UintegerValue(1024));

    ApplicationContainer clientApps =
        echoClient.Install(wifiStaNodes.Get(nWifi - 1));
    clientApps.Start(Seconds(2.0));
    clientApps.Stop(Seconds(10.0));

    if (testing) {
      clientApps.Get(0)->TraceConnectWithoutContext(
          "RxWithAddresses", MakeCallback(&SinkTracer::SinkTrace));
    }
  }

  Ipv4GlobalRoutingHelper::PopulateRoutingTables();

  Simulator::Stop(Seconds(10.0));

  if (tracing) {
    if (systemId == systemCsma) {
      pointToPoint.EnablePcapAll("third-distributed-csma");
      phy.EnablePcap("third-distributed-csma", apDevices.Get(0));
      csma.EnablePcap("third-distributed-csma", csmaDevices.Get(0), true);
    } else {
      pointToPoint.EnablePcapAll("third-distributed-wifi");
      phy.EnablePcap("third-distributed-wifi", apDevices.Get(0));
      csma.EnablePcap("third-distributed-wifi", csmaDevices.Get(0), true);
    }
  }

  Simulator::Run();
  Simulator::Destroy();

  if (testing) {
    SinkTracer::Verify(2);
  }

  MpiInterface::Disable();

  return 0;
}
