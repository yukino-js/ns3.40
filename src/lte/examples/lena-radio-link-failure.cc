
#include "ns3/applications-module.h"
#include "ns3/core-module.h"
#include "ns3/internet-module.h"
#include "ns3/lte-module.h"
#include "ns3/mobility-module.h"
#include "ns3/network-module.h"
#include "ns3/point-to-point-module.h"

#include <iomanip>
#include <iostream>
#include <stdio.h>
#include <vector>

using namespace ns3;

NS_LOG_COMPONENT_DEFINE("LenaRadioLinkFailure");

uint16_t counterN310FirsteNB = 0;
Time t310StartTimeFirstEnb = Seconds(0);
uint32_t ByteCounter = 0;
uint32_t oldByteCounter = 0;

void PrintUePosition(uint64_t imsi) {
  for (auto it = NodeList::Begin(); it != NodeList::End(); ++it) {
    Ptr<Node> node = *it;
    int nDevs = node->GetNDevices();
    for (int j = 0; j < nDevs; j++) {
      Ptr<LteUeNetDevice> uedev =
          node->GetDevice(j)->GetObject<LteUeNetDevice>();
      if (uedev) {
        if (imsi == uedev->GetImsi()) {
          Vector pos = node->GetObject<MobilityModel>()->GetPosition();
          std::cout << "IMSI : " << uedev->GetImsi() << " at " << pos.x << ","
                    << pos.y << std::endl;
        }
      }
    }
  }
}

void NotifyConnectionEstablishedUe(std::string context, uint64_t imsi,
                                   uint16_t cellid, uint16_t rnti) {
  std::cout << Simulator::Now().As(Time::S) << " " << context << " UE IMSI "
            << imsi << ": connected to cell id " << cellid << " with RNTI "
            << rnti << std::endl;
}

void NotifyConnectionEstablishedEnb(std::string context, uint64_t imsi,
                                    uint16_t cellId, uint16_t rnti) {
  std::cout << Simulator::Now().As(Time::S) << " " << context << " eNB cell id "
            << cellId << ": successful connection of UE with IMSI " << imsi
            << " RNTI " << rnti << std::endl;
  if (cellId == 1) {
    counterN310FirsteNB = 0;
  }
}

static const std::string g_ueRrcStateName[LteUeRrc::NUM_STATES] = {
    "IDLE_START",
    "IDLE_CELL_SEARCH",
    "IDLE_WAIT_MIB_SIB1",
    "IDLE_WAIT_MIB",
    "IDLE_WAIT_SIB1",
    "IDLE_CAMPED_NORMALLY",
    "IDLE_WAIT_SIB2",
    "IDLE_RANDOM_ACCESS",
    "IDLE_CONNECTING",
    "CONNECTED_NORMALLY",
    "CONNECTED_HANDOVER",
    "CONNECTED_PHY_PROBLEM",
    "CONNECTED_REESTABLISHING",
};

static const std::string &ToString(LteUeRrc::State s) {
  return g_ueRrcStateName[s];
}

void UeStateTransition(uint64_t imsi, uint16_t cellId, uint16_t rnti,
                       LteUeRrc::State oldState, LteUeRrc::State newState) {
  std::cout << Simulator::Now().As(Time::S) << " UE with IMSI " << imsi
            << " RNTI " << rnti << " connected to cell " << cellId
            << " transitions from " << ToString(oldState) << " to "
            << ToString(newState) << std::endl;
}

void EnbRrcTimeout(uint64_t imsi, uint16_t rnti, uint16_t cellId,
                   std::string cause) {
  std::cout << Simulator::Now().As(Time::S) << " IMSI " << imsi << ", RNTI "
            << rnti << ", Cell id " << cellId << ", ENB RRC " << cause
            << std::endl;
}

void NotifyConnectionReleaseAtEnodeB(uint64_t imsi, uint16_t cellId,
                                     uint16_t rnti) {
  std::cout << Simulator::Now() << " IMSI " << imsi << ", RNTI " << rnti
            << ", Cell id " << cellId << ", UE context destroyed at eNodeB"
            << std::endl;
}

void PhySyncDetection(uint16_t n310, uint64_t imsi, uint16_t rnti,
                      uint16_t cellId, std::string type, uint8_t count) {
  std::cout << Simulator::Now().As(Time::S) << " IMSI " << imsi << ", RNTI "
            << rnti << ", Cell id " << cellId << ", " << type
            << ", no of sync indications: " << +count << std::endl;

  if (type == "Notify out of sync" && cellId == 1) {
    ++counterN310FirsteNB;
    if (counterN310FirsteNB == n310) {
      t310StartTimeFirstEnb = Simulator::Now();
    }
    NS_LOG_DEBUG("counterN310FirsteNB = " << counterN310FirsteNB);
  }
}

void RadioLinkFailure(Time t310, uint64_t imsi, uint16_t cellId,
                      uint16_t rnti) {
  std::cout << Simulator::Now() << " IMSI " << imsi << ", RNTI " << rnti
            << ", Cell id " << cellId << ", radio link failure detected"
            << std::endl
            << std::endl;

  PrintUePosition(imsi);

  if (cellId == 1) {
    NS_ABORT_MSG_IF((Simulator::Now() - t310StartTimeFirstEnb) != t310,
                    "T310 timer expired at wrong time");
  }
}

void NotifyRandomAccessErrorUe(uint64_t imsi, uint16_t cellId, uint16_t rnti) {
  std::cout << Simulator::Now().As(Time::S) << " IMSI " << imsi << ", RNTI "
            << rnti << ", Cell id " << cellId << ", UE RRC Random access Failed"
            << std::endl;
}

void NotifyConnectionTimeoutUe(uint64_t imsi, uint16_t cellId, uint16_t rnti,
                               uint8_t connEstFailCount) {
  std::cout << Simulator::Now().As(Time::S) << " IMSI " << imsi << ", RNTI "
            << rnti << ", Cell id " << cellId << ", T300 expiration counter "
            << (uint16_t)connEstFailCount << ", UE RRC Connection timeout"
            << std::endl;
}

void NotifyRaResponseTimeoutUe(uint64_t imsi, bool contention,
                               uint8_t preambleTxCounter,
                               uint8_t maxPreambleTxLimit) {
  std::cout << Simulator::Now().As(Time::S) << " IMSI " << imsi
            << ", Contention flag " << contention << ", preamble Tx Counter "
            << (uint16_t)preambleTxCounter << ", Max Preamble Tx Limit "
            << (uint16_t)maxPreambleTxLimit << ", UE RA response timeout"
            << std::endl;
}

void ReceivePacket(Ptr<const Packet> packet, const Address &) {
  ByteCounter += packet->GetSize();
}

void Throughput(bool firstWrite, Time binSize, std::string fileName) {
  std::ofstream output;

  if (firstWrite) {
    output.open(fileName, std::ofstream::out);
    firstWrite = false;
  } else {
    output.open(fileName, std::ofstream::app);
  }

  double throughput =
      (ByteCounter - oldByteCounter) * 8 / binSize.GetSeconds() / 1024 / 1024;
  output << Simulator::Now().As(Time::S) << " " << throughput << std::endl;
  oldByteCounter = ByteCounter;
  Simulator::Schedule(binSize, &Throughput, firstWrite, binSize, fileName);
}

int main(int argc, char *argv[]) {
  Time simTime = Seconds(25);
  uint16_t numberOfEnbs = 1;
  double interSiteDistance = 1200;
  uint16_t n311 = 1;
  uint16_t n310 = 1;
  Time t310 = Seconds(1);
  bool useIdealRrc = true;
  bool enableCtrlErrorModel = true;
  bool enableDataErrorModel = true;
  bool enableNsLogs = false;

  CommandLine cmd(__FILE__);
  cmd.AddValue("simTime", "Total duration of the simulation (in seconds)",
               simTime);
  cmd.AddValue("numberOfEnbs", "Number of eNBs", numberOfEnbs);
  cmd.AddValue("n311", "Number of in-synch indication", n311);
  cmd.AddValue("n310", "Number of out-of-synch indication", n310);
  cmd.AddValue("t310",
               "Timer for detecting the Radio link failure (in seconds)", t310);
  cmd.AddValue("interSiteDistance", "Inter-site distance in meter",
               interSiteDistance);
  cmd.AddValue("useIdealRrc", "Use ideal RRC protocol", useIdealRrc);
  cmd.AddValue("enableCtrlErrorModel", "Enable control error model",
               enableCtrlErrorModel);
  cmd.AddValue("enableDataErrorModel", "Enable data error model",
               enableDataErrorModel);
  cmd.AddValue("enableNsLogs", "Enable ns-3 logging (debug builds)",
               enableNsLogs);
  cmd.Parse(argc, argv);

  if (enableNsLogs) {
    auto logLevel = (LogLevel)(LOG_PREFIX_FUNC | LOG_PREFIX_NODE |
                               LOG_PREFIX_TIME | LOG_LEVEL_ALL);
    LogComponentEnable("LteUeRrc", logLevel);
    LogComponentEnable("LteUeMac", logLevel);
    LogComponentEnable("LteUePhy", logLevel);

    LogComponentEnable("LteEnbRrc", logLevel);
    LogComponentEnable("LteEnbMac", logLevel);
    LogComponentEnable("LteEnbPhy", logLevel);

    LogComponentEnable("LenaRadioLinkFailure", logLevel);
  }

  uint16_t numberOfUes = 1;
  uint16_t numBearersPerUe = 1;
  double eNodeB_txPower = 43;

  Config::SetDefault("ns3::LteHelper::UseIdealRrc", BooleanValue(useIdealRrc));
  Config::SetDefault("ns3::LteSpectrumPhy::CtrlErrorModelEnabled",
                     BooleanValue(enableCtrlErrorModel));
  Config::SetDefault("ns3::LteSpectrumPhy::DataErrorModelEnabled",
                     BooleanValue(enableDataErrorModel));

  Config::SetDefault("ns3::LteRlcUm::MaxTxBufferSize",
                     UintegerValue(60 * 1024));

  Ptr<LteHelper> lteHelper = CreateObject<LteHelper>();
  Ptr<PointToPointEpcHelper> epcHelper = CreateObject<PointToPointEpcHelper>();
  lteHelper->SetEpcHelper(epcHelper);

  lteHelper->SetPathlossModelType(
      TypeId::LookupByName("ns3::LogDistancePropagationLossModel"));
  lteHelper->SetPathlossModelAttribute("Exponent", DoubleValue(3.9));
  lteHelper->SetPathlossModelAttribute("ReferenceLoss", DoubleValue(38.57));
  lteHelper->SetPathlossModelAttribute("ReferenceDistance", DoubleValue(1));

  Config::SetDefault("ns3::LteEnbPhy::TxPower", DoubleValue(eNodeB_txPower));
  Config::SetDefault("ns3::LteUePhy::TxPower", DoubleValue(23));
  Config::SetDefault("ns3::LteUePhy::NoiseFigure", DoubleValue(7));
  Config::SetDefault("ns3::LteEnbPhy::NoiseFigure", DoubleValue(2));
  Config::SetDefault("ns3::LteUePhy::EnableUplinkPowerControl",
                     BooleanValue(true));
  Config::SetDefault("ns3::LteUePowerControl::ClosedLoop", BooleanValue(true));
  Config::SetDefault("ns3::LteUePowerControl::AccumulationEnabled",
                     BooleanValue(true));

  lteHelper->SetEnbDeviceAttribute("DlEarfcn", UintegerValue(100));
  lteHelper->SetEnbDeviceAttribute("UlEarfcn", UintegerValue(18100));
  lteHelper->SetEnbDeviceAttribute("DlBandwidth", UintegerValue(25));
  lteHelper->SetEnbDeviceAttribute("UlBandwidth", UintegerValue(25));

  lteHelper->SetSchedulerType("ns3::PfFfMacScheduler");
  Config::SetDefault("ns3::LteAmc::AmcModel", EnumValue(LteAmc::PiroEW2010));
  Config::SetDefault("ns3::LteAmc::Ber", DoubleValue(0.01));
  Config::SetDefault("ns3::PfFfMacScheduler::HarqEnabled", BooleanValue(true));

  Config::SetDefault("ns3::FfMacScheduler::UlCqiFilter",
                     EnumValue(FfMacScheduler::SRS_UL_CQI));

  Config::SetDefault("ns3::LteUeRrc::N310", UintegerValue(n310));
  Config::SetDefault("ns3::LteUeRrc::N311", UintegerValue(n311));
  Config::SetDefault("ns3::LteUeRrc::T310", TimeValue(t310));

  NS_LOG_INFO("Create the internet");
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

  NS_LOG_INFO("Create eNodeB and UE nodes");
  NodeContainer enbNodes;
  NodeContainer ueNodes;
  enbNodes.Create(numberOfEnbs);
  ueNodes.Create(numberOfUes);

  NS_LOG_INFO("Assign mobility");
  Ptr<ListPositionAllocator> positionAllocEnb =
      CreateObject<ListPositionAllocator>();

  for (uint16_t i = 0; i < numberOfEnbs; i++) {
    positionAllocEnb->Add(Vector(interSiteDistance * i, 0, 0));
  }
  MobilityHelper mobility;
  mobility.SetMobilityModel("ns3::ConstantPositionMobilityModel");
  mobility.SetPositionAllocator(positionAllocEnb);
  mobility.Install(enbNodes);

  Ptr<ListPositionAllocator> positionAllocUe =
      CreateObject<ListPositionAllocator>();

  for (int i = 0; i < numberOfUes; i++) {
    positionAllocUe->Add(Vector(200, 0, 0));
  }

  mobility.SetPositionAllocator(positionAllocUe);
  mobility.SetMobilityModel("ns3::ConstantVelocityMobilityModel");
  mobility.Install(ueNodes);

  for (int i = 0; i < numberOfUes; i++) {
    ueNodes.Get(i)->GetObject<ConstantVelocityMobilityModel>()->SetVelocity(
        Vector(30, 0.0, 0.0));
  }

  NS_LOG_INFO(
      "Install LTE Devices in eNB and UEs and fix random number stream");
  NetDeviceContainer enbDevs;
  NetDeviceContainer ueDevs;

  int64_t randomStream = 1;

  enbDevs = lteHelper->InstallEnbDevice(enbNodes);
  randomStream += lteHelper->AssignStreams(enbDevs, randomStream);
  ueDevs = lteHelper->InstallUeDevice(ueNodes);
  randomStream += lteHelper->AssignStreams(ueDevs, randomStream);

  NS_LOG_INFO("Install the IP stack on the UEs");
  internet.Install(ueNodes);
  Ipv4InterfaceContainer ueIpIfaces;
  ueIpIfaces = epcHelper->AssignUeIpv4Address(NetDeviceContainer(ueDevs));

  NS_LOG_INFO("Attach a UE to a eNB");
  lteHelper->Attach(ueDevs);

  NS_LOG_INFO("Install and start applications on UEs and remote host");
  uint16_t dlPort = 10000;
  uint16_t ulPort = 20000;

  DataRateValue dataRateValue = DataRate("18.6Mbps");

  uint64_t bitRate = dataRateValue.Get().GetBitRate();

  uint32_t packetSize = 1024;

  NS_LOG_DEBUG("bit rate " << bitRate);

  double interPacketInterval = static_cast<double>(packetSize * 8) / bitRate;

  Time udpInterval = Seconds(interPacketInterval);

  NS_LOG_DEBUG("UDP will use application interval " << udpInterval.As(Time::S)
                                                    << " sec");

  for (uint32_t u = 0; u < numberOfUes; ++u) {
    Ptr<Node> ue = ueNodes.Get(u);
    Ptr<Ipv4StaticRouting> ueStaticRouting =
        ipv4RoutingHelper.GetStaticRouting(ue->GetObject<Ipv4>());
    ueStaticRouting->SetDefaultRoute(epcHelper->GetUeDefaultGatewayAddress(),
                                     1);

    for (uint32_t b = 0; b < numBearersPerUe; ++b) {
      ApplicationContainer ulClientApps;
      ApplicationContainer ulServerApps;
      ApplicationContainer dlClientApps;
      ApplicationContainer dlServerApps;

      ++dlPort;
      ++ulPort;

      NS_LOG_LOGIC("installing UDP DL app for UE " << u + 1);
      UdpClientHelper dlClientHelper(ueIpIfaces.GetAddress(u), dlPort);
      dlClientHelper.SetAttribute("Interval", TimeValue(udpInterval));
      dlClientHelper.SetAttribute("PacketSize", UintegerValue(packetSize));
      dlClientHelper.SetAttribute("MaxPackets", UintegerValue(1000000));
      dlClientApps.Add(dlClientHelper.Install(remoteHost));

      PacketSinkHelper dlPacketSinkHelper(
          "ns3::UdpSocketFactory",
          InetSocketAddress(Ipv4Address::GetAny(), dlPort));
      dlServerApps.Add(dlPacketSinkHelper.Install(ue));

      NS_LOG_LOGIC("installing UDP UL app for UE " << u + 1);
      UdpClientHelper ulClientHelper(remoteHostAddr, ulPort);
      ulClientHelper.SetAttribute("Interval", TimeValue(udpInterval));
      dlClientHelper.SetAttribute("PacketSize", UintegerValue(packetSize));
      ulClientHelper.SetAttribute("MaxPackets", UintegerValue(1000000));
      ulClientApps.Add(ulClientHelper.Install(ue));

      PacketSinkHelper ulPacketSinkHelper(
          "ns3::UdpSocketFactory",
          InetSocketAddress(Ipv4Address::GetAny(), ulPort));
      ulServerApps.Add(ulPacketSinkHelper.Install(remoteHost));

      Ptr<EpcTft> tft = Create<EpcTft>();
      EpcTft::PacketFilter dlpf;
      dlpf.localPortStart = dlPort;
      dlpf.localPortEnd = dlPort;
      tft->Add(dlpf);
      EpcTft::PacketFilter ulpf;
      ulpf.remotePortStart = ulPort;
      ulpf.remotePortEnd = ulPort;
      tft->Add(ulpf);
      EpsBearer bearer(EpsBearer::NGBR_IMS);
      lteHelper->ActivateDedicatedEpsBearer(ueDevs.Get(u), bearer, tft);

      dlServerApps.Start(Seconds(0.27));
      dlClientApps.Start(Seconds(0.27));
      ulServerApps.Start(Seconds(0.27));
      ulClientApps.Start(Seconds(0.27));
    }
  }
  NS_LOG_INFO("Enable Lte traces and connect custom trace sinks");

  lteHelper->EnableTraces();
  Ptr<RadioBearerStatsCalculator> rlcStats = lteHelper->GetRlcStats();
  rlcStats->SetAttribute("EpochDuration", TimeValue(Seconds(0.05)));
  Ptr<RadioBearerStatsCalculator> pdcpStats = lteHelper->GetPdcpStats();
  pdcpStats->SetAttribute("EpochDuration", TimeValue(Seconds(0.05)));

  Config::Connect("/NodeList/*/DeviceList/*/LteEnbRrc/ConnectionEstablished",
                  MakeCallback(&NotifyConnectionEstablishedEnb));
  Config::Connect("/NodeList/*/DeviceList/*/LteUeRrc/ConnectionEstablished",
                  MakeCallback(&NotifyConnectionEstablishedUe));
  Config::ConnectWithoutContext(
      "/NodeList/*/DeviceList/*/LteUeRrc/StateTransition",
      MakeCallback(&UeStateTransition));
  Config::ConnectWithoutContext(
      "/NodeList/*/DeviceList/*/LteUeRrc/PhySyncDetection",
      MakeBoundCallback(&PhySyncDetection, n310));
  Config::ConnectWithoutContext(
      "/NodeList/*/DeviceList/*/LteUeRrc/RadioLinkFailure",
      MakeBoundCallback(&RadioLinkFailure, t310));
  Config::ConnectWithoutContext(
      "/NodeList/*/DeviceList/*/LteEnbRrc/NotifyConnectionRelease",
      MakeCallback(&NotifyConnectionReleaseAtEnodeB));
  Config::ConnectWithoutContext("/NodeList/*/DeviceList/*/LteEnbRrc/RrcTimeout",
                                MakeCallback(&EnbRrcTimeout));
  Config::ConnectWithoutContext(
      "/NodeList/*/DeviceList/*/LteUeRrc/RandomAccessError",
      MakeCallback(&NotifyRandomAccessErrorUe));
  Config::ConnectWithoutContext(
      "/NodeList/*/DeviceList/*/LteUeRrc/ConnectionTimeout",
      MakeCallback(&NotifyConnectionTimeoutUe));
  Config::ConnectWithoutContext(
      "/NodeList/*/DeviceList/*/$ns3::LteUeNetDevice/"
      "ComponentCarrierMapUe/*/LteUeMac/RaResponseTimeout",
      MakeCallback(&NotifyRaResponseTimeoutUe));

  std::ostringstream oss;
  oss << "/NodeList/" << ueNodes.Get(0)->GetId()
      << "/ApplicationList/0/$ns3::PacketSink/Rx";
  Config::ConnectWithoutContext(oss.str(), MakeCallback(&ReceivePacket));

  bool firstWrite = true;
  std::string rrcType = useIdealRrc ? "ideal_rrc" : "real_rrc";
  std::string fileName =
      "rlf_dl_thrput_" + std::to_string(enbNodes.GetN()) + "_eNB_" + rrcType;
  Time binSize = Seconds(0.2);
  Simulator::Schedule(Seconds(0.47), &Throughput, firstWrite, binSize,
                      fileName);

  NS_LOG_INFO("Starting simulation...");

  Simulator::Stop(simTime);

  Simulator::Run();

  NS_ABORT_MSG_IF(counterN310FirsteNB != n310,
                  "UE RRC should receive "
                      << n310
                      << " out-of-sync indications in Cell 1."
                         " Total received = "
                      << counterN310FirsteNB);

  Simulator::Destroy();

  return 0;
}
