
#include "ns3/applications-module.h"
#include "ns3/core-module.h"
#include "ns3/epc-helper.h"
#include "ns3/internet-module.h"
#include "ns3/ipv4-global-routing-helper.h"
#include "ns3/lte-helper.h"
#include "ns3/lte-module.h"
#include "ns3/mobility-module.h"
#include "ns3/network-module.h"
#include "ns3/point-to-point-helper.h"
#include <ns3/config-store-module.h>

using namespace ns3;

NS_LOG_COMPONENT_DEFINE("EpcFirstExample");

int main(int argc, char *argv[]) {
  uint16_t nEnbs = 1;
  uint16_t nUesPerEnb = 1;
  double simTime = 10.1;
  double distance = 1000.0;
  double interPacketInterval = 1000;

  CommandLine cmd(__FILE__);
  cmd.AddValue("nEnbs", "Number of eNBs", nEnbs);
  cmd.AddValue("nUesPerEnb", "Number of UEs per eNB", nUesPerEnb);
  cmd.AddValue("simTime", "Total duration of the simulation [s])", simTime);
  cmd.AddValue("distance", "Distance between eNBs [m]", distance);
  cmd.AddValue("interPacketInterval", "Inter packet interval [ms])",
               interPacketInterval);
  cmd.Parse(argc, argv);

  Config::SetDefault("ns3::LteSpectrumPhy::CtrlErrorModelEnabled",
                     BooleanValue(false));
  Config::SetDefault("ns3::LteSpectrumPhy::DataErrorModelEnabled",
                     BooleanValue(false));

  GlobalValue::Bind("ChecksumEnabled", BooleanValue(true));

  ConfigStore inputConfig;
  inputConfig.ConfigureDefaults();

  cmd.Parse(argc, argv);

  Ptr<LteHelper> lteHelper = CreateObject<LteHelper>();
  Ptr<EmuEpcHelper> epcHelper = CreateObject<EmuEpcHelper>();
  lteHelper->SetEpcHelper(epcHelper);
  epcHelper->Initialize();

  Ptr<Node> pgw = epcHelper->GetPgwNode();

  NodeContainer remoteHostContainer;
  remoteHostContainer.Create(1);
  Ptr<Node> remoteHost = remoteHostContainer.Get(0);
  InternetStackHelper internet;
  internet.Install(remoteHostContainer);

  PointToPointHelper p2ph;
  p2ph.SetDeviceAttribute("DataRate", DataRateValue(DataRate("100Gb/s")));
  p2ph.SetDeviceAttribute("Mtu", UintegerValue(1500));
  p2ph.SetChannelAttribute("Delay", TimeValue(Seconds(0.010)));
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
  enbNodes.Create(nEnbs);
  ueNodes.Create(nEnbs * nUesPerEnb);

  Ptr<ListPositionAllocator> positionAlloc =
      CreateObject<ListPositionAllocator>();
  for (uint16_t i = 0; i < nEnbs; i++) {
    positionAlloc->Add(Vector(distance * i, 0, 0));
  }
  MobilityHelper mobility;
  mobility.SetMobilityModel("ns3::ConstantPositionMobilityModel");
  mobility.SetPositionAllocator(positionAlloc);
  mobility.Install(enbNodes);
  mobility.Install(ueNodes);

  NetDeviceContainer enbLteDevs = lteHelper->InstallEnbDevice(enbNodes);
  NetDeviceContainer ueLteDevs = lteHelper->InstallUeDevice(ueNodes);

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

  lteHelper->Attach(ueLteDevs);

  Ptr<UniformRandomVariable> startTimeSeconds =
      CreateObject<UniformRandomVariable>();
  startTimeSeconds->SetAttribute("Min", DoubleValue(0));
  startTimeSeconds->SetAttribute("Max",
                                 DoubleValue(interPacketInterval / 1000.0));

  uint16_t dlPort = 1234;
  uint16_t ulPort = 2000;
  for (uint32_t u = 0; u < ueNodes.GetN(); ++u) {
    ++ulPort;
    ApplicationContainer clientApps;
    ApplicationContainer serverApps;

    PacketSinkHelper dlPacketSinkHelper(
        "ns3::UdpSocketFactory",
        InetSocketAddress(Ipv4Address::GetAny(), dlPort));
    PacketSinkHelper ulPacketSinkHelper(
        "ns3::UdpSocketFactory",
        InetSocketAddress(Ipv4Address::GetAny(), ulPort));
    serverApps.Add(dlPacketSinkHelper.Install(ueNodes.Get(u)));
    serverApps.Add(ulPacketSinkHelper.Install(remoteHost));

    UdpClientHelper dlClient(ueIpIface.GetAddress(u), dlPort);
    dlClient.SetAttribute("Interval",
                          TimeValue(MilliSeconds(interPacketInterval)));
    dlClient.SetAttribute("MaxPackets", UintegerValue(1000000));

    UdpClientHelper ulClient(remoteHostAddr, ulPort);
    ulClient.SetAttribute("Interval",
                          TimeValue(MilliSeconds(interPacketInterval)));
    ulClient.SetAttribute("MaxPackets", UintegerValue(1000000));

    clientApps.Add(dlClient.Install(remoteHost));
    clientApps.Add(ulClient.Install(ueNodes.Get(u)));

    serverApps.Start(Seconds(startTimeSeconds->GetValue()));
    clientApps.Start(Seconds(startTimeSeconds->GetValue()));
  }

  Simulator::Stop(Seconds(simTime));
  Simulator::Run();

  Simulator::Destroy();
  return 0;
}
