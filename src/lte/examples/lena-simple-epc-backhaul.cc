
#include "ns3/applications-module.h"
#include "ns3/config-store-module.h"
#include "ns3/core-module.h"
#include "ns3/internet-module.h"
#include "ns3/lte-module.h"
#include "ns3/mobility-module.h"
#include "ns3/network-module.h"
#include "ns3/point-to-point-module.h"

using namespace ns3;

NS_LOG_COMPONENT_DEFINE("LenaSimpleEpcBackhaul");

int main(int argc, char *argv[]) {
  uint16_t numNodePairs = 2;
  Time simTime = MilliSeconds(1900);
  double distance = 60.0;
  Time interPacketInterval = MilliSeconds(100);
  bool disableDl = false;
  bool disableUl = false;
  bool useHelper = false;

  CommandLine cmd(__FILE__);
  cmd.AddValue("numNodePairs", "Number of eNodeBs + UE pairs", numNodePairs);
  cmd.AddValue("simTime", "Total duration of the simulation", simTime);
  cmd.AddValue("distance", "Distance between eNBs [m]", distance);
  cmd.AddValue("interPacketInterval", "Inter packet interval",
               interPacketInterval);
  cmd.AddValue("disableDl", "Disable downlink data flows", disableDl);
  cmd.AddValue("disableUl", "Disable uplink data flows", disableUl);
  cmd.AddValue("useHelper",
               "Build the backhaul network using the helper or "
               "it is built in the example",
               useHelper);
  cmd.Parse(argc, argv);

  ConfigStore inputConfig;
  inputConfig.ConfigureDefaults();

  cmd.Parse(argc, argv);

  Ptr<LteHelper> lteHelper = CreateObject<LteHelper>();
  Ptr<EpcHelper> epcHelper;
  if (!useHelper) {
    epcHelper = CreateObject<NoBackhaulEpcHelper>();
  } else {
    epcHelper = CreateObject<PointToPointEpcHelper>();
  }
  lteHelper->SetEpcHelper(epcHelper);

  Ptr<Node> pgw = epcHelper->GetPgwNode();

  NodeContainer remoteHostContainer;
  remoteHostContainer.Create(1);
  Ptr<Node> remoteHost = remoteHostContainer.Get(0);
  InternetStackHelper internet;
  internet.Install(remoteHostContainer);

  PointToPointHelper p2ph;
  p2ph.SetDeviceAttribute("DataRate", DataRateValue(DataRate("100Gb/s")));
  p2ph.SetDeviceAttribute("Mtu", UintegerValue(1500));
  p2ph.SetChannelAttribute("Delay", TimeValue(MilliSeconds(10)));
  NetDeviceContainer internetDevices = p2ph.Install(pgw, remoteHost);
  Ipv4AddressHelper ipv4h;
  ipv4h.SetBase("1.0.0.0", "255.0.0.0");
  Ipv4InterfaceContainer internetIpIfaces = ipv4h.Assign(internetDevices);
  Ipv4Address remoteHostAddr = internetIpIfaces.GetAddress(1);

  Ipv4StaticRoutingHelper ipv4RoutingHelper;
  Ptr<Ipv4StaticRouting> remoteHostStaticRouting =
      ipv4RoutingHelper.GetStaticRouting(remoteHost->GetObject<Ipv4>());
  remoteHostStaticRouting->AddNetworkRouteTo(Ipv4Address("7.0.0.0"),
                                             Ipv4Mask("255.0.0.0"), 1);

  NodeContainer ueNodes;
  NodeContainer enbNodes;
  enbNodes.Create(numNodePairs);
  ueNodes.Create(numNodePairs);

  Ptr<ListPositionAllocator> positionAlloc =
      CreateObject<ListPositionAllocator>();
  for (uint16_t i = 0; i < numNodePairs; i++) {
    positionAlloc->Add(Vector(distance * i, 0, 0));
  }
  MobilityHelper mobility;
  mobility.SetMobilityModel("ns3::ConstantPositionMobilityModel");
  mobility.SetPositionAllocator(positionAlloc);
  mobility.Install(enbNodes);
  mobility.Install(ueNodes);

  Ptr<Node> sgw = epcHelper->GetSgwNode();

  Ptr<ListPositionAllocator> positionAlloc2 =
      CreateObject<ListPositionAllocator>();
  positionAlloc2->Add(Vector(0.0, 50.0, 0.0));
  MobilityHelper mobility2;
  mobility2.SetMobilityModel("ns3::ConstantPositionMobilityModel");
  mobility2.SetPositionAllocator(positionAlloc2);
  mobility2.Install(sgw);

  NetDeviceContainer enbLteDevs = lteHelper->InstallEnbDevice(enbNodes);
  NetDeviceContainer ueLteDevs = lteHelper->InstallUeDevice(ueNodes);

  if (!useHelper) {
    Ipv4AddressHelper s1uIpv4AddressHelper;

    s1uIpv4AddressHelper.SetBase("10.0.0.0", "255.255.255.252");

    for (uint16_t i = 0; i < numNodePairs; ++i) {
      Ptr<Node> enb = enbNodes.Get(i);
      std::vector<uint16_t> cellIds(1, i + 1);

      PointToPointHelper p2ph;
      DataRate s1uLinkDataRate = DataRate("10Gb/s");
      uint16_t s1uLinkMtu = 2000;
      Time s1uLinkDelay = Time(0);
      p2ph.SetDeviceAttribute("DataRate", DataRateValue(s1uLinkDataRate));
      p2ph.SetDeviceAttribute("Mtu", UintegerValue(s1uLinkMtu));
      p2ph.SetChannelAttribute("Delay", TimeValue(s1uLinkDelay));
      NetDeviceContainer sgwEnbDevices = p2ph.Install(sgw, enb);

      Ipv4InterfaceContainer sgwEnbIpIfaces =
          s1uIpv4AddressHelper.Assign(sgwEnbDevices);
      s1uIpv4AddressHelper.NewNetwork();

      Ipv4Address sgwS1uAddress = sgwEnbIpIfaces.GetAddress(0);
      Ipv4Address enbS1uAddress = sgwEnbIpIfaces.GetAddress(1);

      epcHelper->AddS1Interface(enb, enbS1uAddress, sgwS1uAddress, cellIds);
    }
  }

  internet.Install(ueNodes);
  Ipv4InterfaceContainer ueIpIface;
  ueIpIface = epcHelper->AssignUeIpv4Address(NetDeviceContainer(ueLteDevs));
  for (uint32_t u = 0; u < ueNodes.GetN(); ++u) {
    Ptr<Node> ueNode = ueNodes.Get(u);
    Ptr<Ipv4StaticRouting> ueStaticRouting =
        ipv4RoutingHelper.GetStaticRouting(ueNode->GetObject<Ipv4>());
    ueStaticRouting->SetDefaultRoute(epcHelper->GetUeDefaultGatewayAddress(),
                                     1);
  }

  for (uint16_t i = 0; i < numNodePairs; i++) {
    lteHelper->Attach(ueLteDevs.Get(i), enbLteDevs.Get(i));
  }

  uint16_t dlPort = 1100;
  uint16_t ulPort = 2000;
  ApplicationContainer clientApps;
  ApplicationContainer serverApps;
  for (uint32_t u = 0; u < ueNodes.GetN(); ++u) {
    if (!disableDl) {
      PacketSinkHelper dlPacketSinkHelper(
          "ns3::UdpSocketFactory",
          InetSocketAddress(Ipv4Address::GetAny(), dlPort));
      serverApps.Add(dlPacketSinkHelper.Install(ueNodes.Get(u)));

      UdpClientHelper dlClient(ueIpIface.GetAddress(u), dlPort);
      dlClient.SetAttribute("Interval", TimeValue(interPacketInterval));
      dlClient.SetAttribute("MaxPackets", UintegerValue(1000000));
      clientApps.Add(dlClient.Install(remoteHost));
    }

    if (!disableUl) {
      ++ulPort;
      PacketSinkHelper ulPacketSinkHelper(
          "ns3::UdpSocketFactory",
          InetSocketAddress(Ipv4Address::GetAny(), ulPort));
      serverApps.Add(ulPacketSinkHelper.Install(remoteHost));

      UdpClientHelper ulClient(remoteHostAddr, ulPort);
      ulClient.SetAttribute("Interval", TimeValue(interPacketInterval));
      ulClient.SetAttribute("MaxPackets", UintegerValue(1000000));
      clientApps.Add(ulClient.Install(ueNodes.Get(u)));
    }
  }

  serverApps.Start(MilliSeconds(500));
  clientApps.Start(MilliSeconds(500));
  lteHelper->EnableTraces();

  Simulator::Stop(simTime);
  Simulator::Run();

  Simulator::Destroy();
  return 0;
}
