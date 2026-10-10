
#include "ns3/applications-module.h"
#include "ns3/config-store-module.h"
#include "ns3/core-module.h"
#include "ns3/internet-module.h"
#include "ns3/lte-module.h"
#include "ns3/mobility-module.h"
#include "ns3/network-module.h"
#include "ns3/point-to-point-module.h"

using namespace ns3;

NS_LOG_COMPONENT_DEFINE("LenaX2HandoverMeasures");

void NotifyConnectionEstablishedUe(std::string context, uint64_t imsi,
                                   uint16_t cellid, uint16_t rnti) {
  std::cout << context << " UE IMSI " << imsi << ": connected to CellId "
            << cellid << " with RNTI " << rnti << std::endl;
}

void NotifyHandoverStartUe(std::string context, uint64_t imsi, uint16_t cellid,
                           uint16_t rnti, uint16_t targetCellId) {
  std::cout << context << " UE IMSI " << imsi
            << ": previously connected to CellId " << cellid << " with RNTI "
            << rnti << ", doing handover to CellId " << targetCellId
            << std::endl;
}

void NotifyHandoverEndOkUe(std::string context, uint64_t imsi, uint16_t cellid,
                           uint16_t rnti) {
  std::cout << context << " UE IMSI " << imsi
            << ": successful handover to CellId " << cellid << " with RNTI "
            << rnti << std::endl;
}

void NotifyConnectionEstablishedEnb(std::string context, uint64_t imsi,
                                    uint16_t cellid, uint16_t rnti) {
  std::cout << context << " eNB CellId " << cellid
            << ": successful connection of UE with IMSI " << imsi << " RNTI "
            << rnti << std::endl;
}

void NotifyHandoverStartEnb(std::string context, uint64_t imsi, uint16_t cellid,
                            uint16_t rnti, uint16_t targetCellId) {
  std::cout << context << " eNB CellId " << cellid
            << ": start handover of UE with IMSI " << imsi << " RNTI " << rnti
            << " to CellId " << targetCellId << std::endl;
}

void NotifyHandoverEndOkEnb(std::string context, uint64_t imsi, uint16_t cellid,
                            uint16_t rnti) {
  std::cout << context << " eNB CellId " << cellid
            << ": completed handover of UE with IMSI " << imsi << " RNTI "
            << rnti << std::endl;
}

int main(int argc, char *argv[]) {

  uint16_t numberOfUes = 1;
  uint16_t numberOfEnbs = 2;
  uint16_t numBearersPerUe = 0;
  double distance = 500.0;
  double yForUe = 500.0;
  double speed = 20;
  double simTime = (double)(numberOfEnbs + 1) * distance / speed;
  double enbTxPowerDbm = 46.0;

  Config::SetDefault("ns3::UdpClient::Interval", TimeValue(MilliSeconds(10)));
  Config::SetDefault("ns3::UdpClient::MaxPackets", UintegerValue(1000000));
  Config::SetDefault("ns3::LteHelper::UseIdealRrc", BooleanValue(true));

  CommandLine cmd(__FILE__);
  cmd.AddValue("simTime", "Total duration of the simulation (in seconds)",
               simTime);
  cmd.AddValue("speed", "Speed of the UE (default = 20 m/s)", speed);
  cmd.AddValue("enbTxPowerDbm", "TX power [dBm] used by HeNBs (default = 46.0)",
               enbTxPowerDbm);

  cmd.Parse(argc, argv);

  Ptr<LteHelper> lteHelper = CreateObject<LteHelper>();
  Ptr<PointToPointEpcHelper> epcHelper = CreateObject<PointToPointEpcHelper>();
  lteHelper->SetEpcHelper(epcHelper);
  lteHelper->SetSchedulerType("ns3::RrFfMacScheduler");

  lteHelper->SetHandoverAlgorithmType("ns3::A2A4RsrqHandoverAlgorithm");
  lteHelper->SetHandoverAlgorithmAttribute("ServingCellThreshold",
                                           UintegerValue(30));
  lteHelper->SetHandoverAlgorithmAttribute("NeighbourCellOffset",
                                           UintegerValue(1));

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

  Ptr<ListPositionAllocator> enbPositionAlloc =
      CreateObject<ListPositionAllocator>();
  for (uint16_t i = 0; i < numberOfEnbs; i++) {
    Vector enbPosition(distance * (i + 1), distance, 0);
    enbPositionAlloc->Add(enbPosition);
  }
  MobilityHelper enbMobility;
  enbMobility.SetMobilityModel("ns3::ConstantPositionMobilityModel");
  enbMobility.SetPositionAllocator(enbPositionAlloc);
  enbMobility.Install(enbNodes);

  MobilityHelper ueMobility;
  ueMobility.SetMobilityModel("ns3::ConstantVelocityMobilityModel");
  ueMobility.Install(ueNodes);
  ueNodes.Get(0)->GetObject<MobilityModel>()->SetPosition(Vector(0, yForUe, 0));
  ueNodes.Get(0)->GetObject<ConstantVelocityMobilityModel>()->SetVelocity(
      Vector(speed, 0, 0));

  Config::SetDefault("ns3::LteEnbPhy::TxPower", DoubleValue(enbTxPowerDbm));
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
  startTimeSeconds->SetAttribute("Min", DoubleValue(0));
  startTimeSeconds->SetAttribute("Max", DoubleValue(0.010));

  for (uint32_t u = 0; u < numberOfUes; ++u) {
    Ptr<Node> ue = ueNodes.Get(u);
    Ptr<Ipv4StaticRouting> ueStaticRouting =
        ipv4RoutingHelper.GetStaticRouting(ue->GetObject<Ipv4>());
    ueStaticRouting->SetDefaultRoute(epcHelper->GetUeDefaultGatewayAddress(),
                                     1);

    for (uint32_t b = 0; b < numBearersPerUe; ++b) {
      ++dlPort;
      ++ulPort;

      ApplicationContainer clientApps;
      ApplicationContainer serverApps;

      NS_LOG_LOGIC("installing UDP DL app for UE " << u);
      UdpClientHelper dlClientHelper(ueIpIfaces.GetAddress(u), dlPort);
      clientApps.Add(dlClientHelper.Install(remoteHost));
      PacketSinkHelper dlPacketSinkHelper(
          "ns3::UdpSocketFactory",
          InetSocketAddress(Ipv4Address::GetAny(), dlPort));
      serverApps.Add(dlPacketSinkHelper.Install(ue));

      NS_LOG_LOGIC("installing UDP UL app for UE " << u);
      UdpClientHelper ulClientHelper(remoteHostAddr, ulPort);
      clientApps.Add(ulClientHelper.Install(ue));
      PacketSinkHelper ulPacketSinkHelper(
          "ns3::UdpSocketFactory",
          InetSocketAddress(Ipv4Address::GetAny(), ulPort));
      serverApps.Add(ulPacketSinkHelper.Install(remoteHost));

      Ptr<EpcTft> tft = Create<EpcTft>();
      EpcTft::PacketFilter dlpf;
      dlpf.localPortStart = dlPort;
      dlpf.localPortEnd = dlPort;
      tft->Add(dlpf);
      EpcTft::PacketFilter ulpf;
      ulpf.remotePortStart = ulPort;
      ulpf.remotePortEnd = ulPort;
      tft->Add(ulpf);
      EpsBearer bearer(EpsBearer::NGBR_VIDEO_TCP_DEFAULT);
      lteHelper->ActivateDedicatedEpsBearer(ueLteDevs.Get(u), bearer, tft);

      Time startTime = Seconds(startTimeSeconds->GetValue());
      serverApps.Start(startTime);
      clientApps.Start(startTime);
    }
  }

  lteHelper->AddX2Interface(enbNodes);

  lteHelper->EnablePhyTraces();
  lteHelper->EnableMacTraces();
  lteHelper->EnableRlcTraces();
  lteHelper->EnablePdcpTraces();
  Ptr<RadioBearerStatsCalculator> rlcStats = lteHelper->GetRlcStats();
  rlcStats->SetAttribute("EpochDuration", TimeValue(Seconds(1.0)));
  Ptr<RadioBearerStatsCalculator> pdcpStats = lteHelper->GetPdcpStats();
  pdcpStats->SetAttribute("EpochDuration", TimeValue(Seconds(1.0)));

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

  Simulator::Stop(Seconds(simTime));
  Simulator::Run();

  Simulator::Destroy();
  return 0;
}
