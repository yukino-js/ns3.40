
#include "ns3/applications-module.h"
#include "ns3/config-store-module.h"
#include "ns3/core-module.h"
#include "ns3/internet-module.h"
#include "ns3/lte-module.h"
#include "ns3/mobility-module.h"
#include "ns3/network-module.h"
#include "ns3/point-to-point-module.h"

using namespace ns3;

NS_LOG_COMPONENT_DEFINE("LenaX2HandoverExample");

void NotifyConnectionEstablishedUe(std::string context, uint64_t imsi,
                                   uint16_t cellid, uint16_t rnti) {
  std::cout << Simulator::Now().As(Time::S) << " " << context << " UE IMSI "
            << imsi << ": connected to CellId " << cellid << " with RNTI "
            << rnti << std::endl;
}

void NotifyHandoverStartUe(std::string context, uint64_t imsi, uint16_t cellid,
                           uint16_t rnti, uint16_t targetCellId) {
  std::cout << Simulator::Now().As(Time::S) << " " << context << " UE IMSI "
            << imsi << ": previously connected to CellId " << cellid
            << " with RNTI " << rnti << ", doing handover to CellId "
            << targetCellId << std::endl;
}

void NotifyHandoverEndOkUe(std::string context, uint64_t imsi, uint16_t cellid,
                           uint16_t rnti) {
  std::cout << Simulator::Now().As(Time::S) << " " << context << " UE IMSI "
            << imsi << ": successful handover to CellId " << cellid
            << " with RNTI " << rnti << std::endl;
}

void NotifyConnectionEstablishedEnb(std::string context, uint64_t imsi,
                                    uint16_t cellid, uint16_t rnti) {
  std::cout << Simulator::Now().As(Time::S) << " " << context << " eNB CellId "
            << cellid << ": successful connection of UE with IMSI " << imsi
            << " RNTI " << rnti << std::endl;
}

void NotifyHandoverStartEnb(std::string context, uint64_t imsi, uint16_t cellid,
                            uint16_t rnti, uint16_t targetCellId) {
  std::cout << Simulator::Now().As(Time::S) << " " << context << " eNB CellId "
            << cellid << ": start handover of UE with IMSI " << imsi << " RNTI "
            << rnti << " to CellId " << targetCellId << std::endl;
}

void NotifyHandoverEndOkEnb(std::string context, uint64_t imsi, uint16_t cellid,
                            uint16_t rnti) {
  std::cout << Simulator::Now().As(Time::S) << " " << context << " eNB CellId "
            << cellid << ": completed handover of UE with IMSI " << imsi
            << " RNTI " << rnti << std::endl;
}

void NotifyHandoverFailure(std::string context, uint64_t imsi, uint16_t cellid,
                           uint16_t rnti) {
  std::cout << Simulator::Now().As(Time::S) << " " << context << " eNB CellId "
            << cellid << " IMSI " << imsi << " RNTI " << rnti
            << " handover failure" << std::endl;
}

int main(int argc, char *argv[]) {

  uint16_t numberOfUes = 1;
  uint16_t numberOfEnbs = 2;
  uint16_t numBearersPerUe = 2;
  Time simTime = MilliSeconds(490);
  double distance = 100.0;
  bool disableDl = false;
  bool disableUl = false;

  Config::SetDefault("ns3::UdpClient::Interval", TimeValue(MilliSeconds(10)));
  Config::SetDefault("ns3::UdpClient::MaxPackets", UintegerValue(1000000));
  Config::SetDefault("ns3::LteHelper::UseIdealRrc", BooleanValue(false));

  CommandLine cmd(__FILE__);
  cmd.AddValue("numberOfUes", "Number of UEs", numberOfUes);
  cmd.AddValue("numberOfEnbs", "Number of eNodeBs", numberOfEnbs);
  cmd.AddValue("simTime", "Total duration of the simulation", simTime);
  cmd.AddValue("disableDl", "Disable downlink data flows", disableDl);
  cmd.AddValue("disableUl", "Disable uplink data flows", disableUl);
  cmd.Parse(argc, argv);

  Ptr<LteHelper> lteHelper = CreateObject<LteHelper>();
  Ptr<PointToPointEpcHelper> epcHelper = CreateObject<PointToPointEpcHelper>();
  lteHelper->SetEpcHelper(epcHelper);
  lteHelper->SetSchedulerType("ns3::RrFfMacScheduler");
  lteHelper->SetHandoverAlgorithmType("ns3::NoOpHandoverAlgorithm");

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
  enbNodes.Create(numberOfEnbs);
  ueNodes.Create(numberOfUes);

  Ptr<ListPositionAllocator> positionAlloc =
      CreateObject<ListPositionAllocator>();
  for (uint16_t i = 0; i < numberOfEnbs; i++) {
    positionAlloc->Add(Vector(distance * 2 * i - distance, 0, 0));
  }
  for (uint16_t i = 0; i < numberOfUes; i++) {
    positionAlloc->Add(Vector(0, 0, 0));
  }
  MobilityHelper mobility;
  mobility.SetMobilityModel("ns3::ConstantPositionMobilityModel");
  mobility.SetPositionAllocator(positionAlloc);
  mobility.Install(enbNodes);
  mobility.Install(ueNodes);

  NetDeviceContainer enbLteDevs = lteHelper->InstallEnbDevice(enbNodes);
  NetDeviceContainer ueLteDevs = lteHelper->InstallUeDevice(ueNodes);

  internet.Install(ueNodes);
  Ipv4InterfaceContainer ueIpIfaces;
  ueIpIfaces = epcHelper->AssignUeIpv4Address(NetDeviceContainer(ueLteDevs));

  for (uint16_t i = 0; i < numberOfUes; i++) {
    lteHelper->Attach(ueLteDevs.Get(i), enbLteDevs.Get(0));
  }

  NS_LOG_LOGIC("setting up applications");

  uint16_t dlPort = 10000;
  uint16_t ulPort = 20000;

  Ptr<UniformRandomVariable> startTimeSeconds =
      CreateObject<UniformRandomVariable>();
  startTimeSeconds->SetAttribute("Min", DoubleValue(0.05));
  startTimeSeconds->SetAttribute("Max", DoubleValue(0.06));

  for (uint32_t u = 0; u < numberOfUes; ++u) {
    Ptr<Node> ue = ueNodes.Get(u);
    Ptr<Ipv4StaticRouting> ueStaticRouting =
        ipv4RoutingHelper.GetStaticRouting(ue->GetObject<Ipv4>());
    ueStaticRouting->SetDefaultRoute(epcHelper->GetUeDefaultGatewayAddress(),
                                     1);

    for (uint32_t b = 0; b < numBearersPerUe; ++b) {
      ApplicationContainer clientApps;
      ApplicationContainer serverApps;
      Ptr<EpcTft> tft = Create<EpcTft>();

      if (!disableDl) {
        ++dlPort;

        NS_LOG_LOGIC("installing UDP DL app for UE " << u);
        UdpClientHelper dlClientHelper(ueIpIfaces.GetAddress(u), dlPort);
        clientApps.Add(dlClientHelper.Install(remoteHost));
        PacketSinkHelper dlPacketSinkHelper(
            "ns3::UdpSocketFactory",
            InetSocketAddress(Ipv4Address::GetAny(), dlPort));
        serverApps.Add(dlPacketSinkHelper.Install(ue));

        EpcTft::PacketFilter dlpf;
        dlpf.localPortStart = dlPort;
        dlpf.localPortEnd = dlPort;
        tft->Add(dlpf);
      }

      if (!disableUl) {
        ++ulPort;

        NS_LOG_LOGIC("installing UDP UL app for UE " << u);
        UdpClientHelper ulClientHelper(remoteHostAddr, ulPort);
        clientApps.Add(ulClientHelper.Install(ue));
        PacketSinkHelper ulPacketSinkHelper(
            "ns3::UdpSocketFactory",
            InetSocketAddress(Ipv4Address::GetAny(), ulPort));
        serverApps.Add(ulPacketSinkHelper.Install(remoteHost));

        EpcTft::PacketFilter ulpf;
        ulpf.remotePortStart = ulPort;
        ulpf.remotePortEnd = ulPort;
        tft->Add(ulpf);
      }

      EpsBearer bearer(EpsBearer::NGBR_VIDEO_TCP_DEFAULT);
      lteHelper->ActivateDedicatedEpsBearer(ueLteDevs.Get(u), bearer, tft);

      Time startTime = Seconds(startTimeSeconds->GetValue());
      serverApps.Start(startTime);
      clientApps.Start(startTime);
      clientApps.Stop(simTime);
    }
  }

  lteHelper->AddX2Interface(enbNodes);

  lteHelper->HandoverRequest(MilliSeconds(300), ueLteDevs.Get(0),
                             enbLteDevs.Get(0), enbLteDevs.Get(1));

  lteHelper->EnablePhyTraces();
  lteHelper->EnableMacTraces();
  lteHelper->EnableRlcTraces();
  lteHelper->EnablePdcpTraces();
  Ptr<RadioBearerStatsCalculator> rlcStats = lteHelper->GetRlcStats();
  rlcStats->SetAttribute("EpochDuration", TimeValue(Seconds(0.05)));
  Ptr<RadioBearerStatsCalculator> pdcpStats = lteHelper->GetPdcpStats();
  pdcpStats->SetAttribute("EpochDuration", TimeValue(Seconds(0.05)));

  Config::Connect("/NodeList/*/DeviceList/*/LteEnbRrc/ConnectionEstablished",
                  MakeCallback(&NotifyConnectionEstablishedEnb));
  Config::Connect("/NodeList/*/DeviceList/*/LteUeRrc/ConnectionEstablished",
                  MakeCallback(&NotifyConnectionEstablishedUe));
  Config::Connect("/NodeList/*/DeviceList/*/LteEnbRrc/HandoverStart",
                  MakeCallback(&NotifyHandoverStartEnb));
  Config::Connect("/NodeList/*/DeviceList/*/LteUeRrc/HandoverStart",
                  MakeCallback(&NotifyHandoverStartUe));
  Config::Connect("/NodeList/*/DeviceList/*/LteEnbRrc/HandoverEndOk",
                  MakeCallback(&NotifyHandoverEndOkEnb));
  Config::Connect("/NodeList/*/DeviceList/*/LteUeRrc/HandoverEndOk",
                  MakeCallback(&NotifyHandoverEndOkUe));

  Config::Connect(
      "/NodeList/*/DeviceList/*/LteEnbRrc/HandoverFailureNoPreamble",
      MakeCallback(&NotifyHandoverFailure));
  Config::Connect("/NodeList/*/DeviceList/*/LteEnbRrc/HandoverFailureMaxRach",
                  MakeCallback(&NotifyHandoverFailure));
  Config::Connect("/NodeList/*/DeviceList/*/LteEnbRrc/HandoverFailureLeaving",
                  MakeCallback(&NotifyHandoverFailure));
  Config::Connect("/NodeList/*/DeviceList/*/LteEnbRrc/HandoverFailureJoining",
                  MakeCallback(&NotifyHandoverFailure));

  Simulator::Stop(simTime + MilliSeconds(20));
  Simulator::Run();

  Simulator::Destroy();
  return 0;
}
