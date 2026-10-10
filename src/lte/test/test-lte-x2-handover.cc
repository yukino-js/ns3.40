
#include <ns3/core-module.h>
#include <ns3/internet-module.h>
#include <ns3/lte-module.h>
#include <ns3/mobility-module.h>
#include <ns3/network-module.h>
#include <ns3/packet-sink-helper.h>
#include <ns3/packet-sink.h>
#include <ns3/point-to-point-module.h>
#include <ns3/udp-client-server-helper.h>

using namespace ns3;

NS_LOG_COMPONENT_DEFINE("LteX2HandoverTest");

struct HandoverEvent {
  Time startTime;
  uint32_t ueDeviceIndex;
  uint32_t sourceEnbDeviceIndex;
  uint32_t targetEnbDeviceIndex;
};

class LteX2HandoverTestCase : public TestCase {
public:
  LteX2HandoverTestCase(uint32_t nUes, uint32_t nDedicatedBearers,
                        std::list<HandoverEvent> handoverEventList,
                        std::string handoverEventListName,
                        std::string schedulerType, bool admitHo,
                        bool useIdealRrc);

private:
  static std::string BuildNameString(uint32_t nUes, uint32_t nDedicatedBearers,
                                     std::string handoverEventListName,
                                     std::string schedulerType, bool admitHo,
                                     bool useIdealRrc);
  void DoRun() override;
  void CheckConnected(Ptr<NetDevice> ueDevice, Ptr<NetDevice> enbDevice);

  void TeleportUeToMiddle(Ptr<Node> ueNode);

  void TeleportUeNearTargetEnb(Ptr<Node> ueNode, Ptr<Node> enbNode);

  uint32_t m_nUes;
  uint32_t m_nDedicatedBearers;
  std::list<HandoverEvent> m_handoverEventList;
  std::string m_handoverEventListName;
  bool m_epc;
  std::string m_schedulerType;
  bool m_admitHo;
  bool m_useIdealRrc;
  Ptr<LteHelper> m_lteHelper;
  Ptr<PointToPointEpcHelper> m_epcHelper;

  struct BearerData {
    uint32_t bid;
    Ptr<PacketSink> dlSink;
    Ptr<PacketSink> ulSink;
    uint32_t dlOldTotalRx;
    uint32_t ulOldTotalRx;
  };

  struct UeData {
    uint32_t id;
    std::list<BearerData> bearerDataList;
  };

  void SaveStatsAfterHandover(uint32_t ueIndex);
  void CheckStatsAWhileAfterHandover(uint32_t ueIndex);

  std::vector<UeData> m_ueDataVector;

  const Time m_maxHoDuration;
  const Time m_statsDuration;
  const Time m_udpClientInterval;
  const uint32_t m_udpClientPktSize;
};

std::string LteX2HandoverTestCase::BuildNameString(
    uint32_t nUes, uint32_t nDedicatedBearers,
    std::string handoverEventListName, std::string schedulerType, bool admitHo,
    bool useIdealRrc) {
  std::ostringstream oss;
  oss << " nUes=" << nUes << " nDedicatedBearers=" << nDedicatedBearers << " "
      << schedulerType << " admitHo=" << admitHo
      << " hoList: " << handoverEventListName;
  if (useIdealRrc) {
    oss << ", ideal RRC";
  } else {
    oss << ", real RRC";
  }
  return oss.str();
}

LteX2HandoverTestCase::LteX2HandoverTestCase(
    uint32_t nUes, uint32_t nDedicatedBearers,
    std::list<HandoverEvent> handoverEventList,
    std::string handoverEventListName, std::string schedulerType, bool admitHo,
    bool useIdealRrc)
    : TestCase(BuildNameString(nUes, nDedicatedBearers, handoverEventListName,
                               schedulerType, admitHo, useIdealRrc)),
      m_nUes(nUes), m_nDedicatedBearers(nDedicatedBearers),
      m_handoverEventList(handoverEventList),
      m_handoverEventListName(handoverEventListName), m_epc(true),
      m_schedulerType(schedulerType), m_admitHo(admitHo),
      m_useIdealRrc(useIdealRrc), m_maxHoDuration(Seconds(0.1)),
      m_statsDuration(Seconds(0.1)), m_udpClientInterval(Seconds(0.01)),
      m_udpClientPktSize(100)

{}

void LteX2HandoverTestCase::DoRun() {
  NS_LOG_FUNCTION(this << BuildNameString(
                      m_nUes, m_nDedicatedBearers, m_handoverEventListName,
                      m_schedulerType, m_admitHo, m_useIdealRrc));

  uint32_t previousSeed = RngSeedManager::GetSeed();
  uint64_t previousRun = RngSeedManager::GetRun();
  Config::Reset();
  RngSeedManager::SetSeed(1);
  RngSeedManager::SetRun(3);
  Config::SetDefault("ns3::UdpClient::Interval",
                     TimeValue(m_udpClientInterval));
  Config::SetDefault("ns3::UdpClient::MaxPackets", UintegerValue(1000000));
  Config::SetDefault("ns3::UdpClient::PacketSize",
                     UintegerValue(m_udpClientPktSize));

  Config::SetDefault("ns3::LteUePhy::EnableUplinkPowerControl",
                     BooleanValue(false));

  int64_t stream = 1;

  m_lteHelper = CreateObject<LteHelper>();
  m_lteHelper->SetAttribute(
      "PathlossModel", StringValue("ns3::FriisSpectrumPropagationLossModel"));
  m_lteHelper->SetSchedulerType(m_schedulerType);
  m_lteHelper->SetHandoverAlgorithmType("ns3::NoOpHandoverAlgorithm");
  m_lteHelper->SetAttribute("UseIdealRrc", BooleanValue(m_useIdealRrc));

  NodeContainer enbNodes;
  enbNodes.Create(2);
  NodeContainer ueNodes;
  ueNodes.Create(m_nUes);

  if (m_epc) {
    m_epcHelper = CreateObject<PointToPointEpcHelper>();
    m_lteHelper->SetEpcHelper(m_epcHelper);
  }

  Ptr<ListPositionAllocator> positionAlloc =
      CreateObject<ListPositionAllocator>();
  positionAlloc->Add(Vector(-3000, 0, 0));
  positionAlloc->Add(Vector(3000, 0, 0));
  for (uint32_t i = 0; i < m_nUes; i++) {
    positionAlloc->Add(Vector(-3000, 100, 0));
  }
  MobilityHelper mobility;
  mobility.SetPositionAllocator(positionAlloc);
  mobility.SetMobilityModel("ns3::ConstantPositionMobilityModel");
  mobility.Install(enbNodes);
  mobility.Install(ueNodes);

  NetDeviceContainer enbDevices;
  enbDevices = m_lteHelper->InstallEnbDevice(enbNodes);
  stream += m_lteHelper->AssignStreams(enbDevices, stream);
  for (auto it = enbDevices.Begin(); it != enbDevices.End(); ++it) {
    Ptr<LteEnbRrc> enbRrc = (*it)->GetObject<LteEnbNetDevice>()->GetRrc();
    enbRrc->SetAttribute("AdmitHandoverRequest", BooleanValue(m_admitHo));
  }

  NetDeviceContainer ueDevices;
  ueDevices = m_lteHelper->InstallUeDevice(ueNodes);
  stream += m_lteHelper->AssignStreams(ueDevices, stream);

  Ipv4Address remoteHostAddr;
  Ipv4StaticRoutingHelper ipv4RoutingHelper;
  Ipv4InterfaceContainer ueIpIfaces;
  Ptr<Node> remoteHost;
  if (m_epc) {
    NodeContainer remoteHostContainer;
    remoteHostContainer.Create(1);
    remoteHost = remoteHostContainer.Get(0);
    InternetStackHelper internet;
    internet.Install(remoteHostContainer);

    PointToPointHelper p2ph;
    p2ph.SetDeviceAttribute("DataRate", DataRateValue(DataRate("100Gb/s")));
    p2ph.SetDeviceAttribute("Mtu", UintegerValue(1500));
    p2ph.SetChannelAttribute("Delay", TimeValue(Seconds(0.010)));
    Ptr<Node> pgw = m_epcHelper->GetPgwNode();
    NetDeviceContainer internetDevices = p2ph.Install(pgw, remoteHost);
    Ipv4AddressHelper ipv4h;
    ipv4h.SetBase("1.0.0.0", "255.0.0.0");
    Ipv4InterfaceContainer internetIpIfaces = ipv4h.Assign(internetDevices);
    remoteHostAddr = internetIpIfaces.GetAddress(1);

    Ipv4StaticRoutingHelper ipv4RoutingHelper;
    Ptr<Ipv4StaticRouting> remoteHostStaticRouting =
        ipv4RoutingHelper.GetStaticRouting(remoteHost->GetObject<Ipv4>());
    remoteHostStaticRouting->AddNetworkRouteTo(Ipv4Address("7.0.0.0"),
                                               Ipv4Mask("255.0.0.0"), 1);

    internet.Install(ueNodes);
    ueIpIfaces =
        m_epcHelper->AssignUeIpv4Address(NetDeviceContainer(ueDevices));
  }

  m_lteHelper->Attach(ueDevices, enbDevices.Get(0));

  if (m_epc) {

    uint16_t dlPort = 10000;
    uint16_t ulPort = 20000;

    Ptr<UniformRandomVariable> startTimeSeconds =
        CreateObject<UniformRandomVariable>();
    startTimeSeconds->SetAttribute("Min", DoubleValue(0));
    startTimeSeconds->SetAttribute("Max", DoubleValue(0.010));
    startTimeSeconds->SetStream(stream++);

    for (uint32_t u = 0; u < ueNodes.GetN(); ++u) {
      Ptr<Node> ue = ueNodes.Get(u);
      Ptr<Ipv4StaticRouting> ueStaticRouting =
          ipv4RoutingHelper.GetStaticRouting(ue->GetObject<Ipv4>());
      ueStaticRouting->SetDefaultRoute(
          m_epcHelper->GetUeDefaultGatewayAddress(), 1);

      UeData ueData;

      for (uint32_t b = 0; b < m_nDedicatedBearers; ++b) {
        ++dlPort;
        ++ulPort;

        ApplicationContainer clientApps;
        ApplicationContainer serverApps;
        BearerData bearerData = BearerData();

        {
          UdpClientHelper dlClientHelper(ueIpIfaces.GetAddress(u), dlPort);
          clientApps.Add(dlClientHelper.Install(remoteHost));
          PacketSinkHelper dlPacketSinkHelper(
              "ns3::UdpSocketFactory",
              InetSocketAddress(Ipv4Address::GetAny(), dlPort));
          ApplicationContainer sinkContainer = dlPacketSinkHelper.Install(ue);
          bearerData.dlSink = sinkContainer.Get(0)->GetObject<PacketSink>();
          serverApps.Add(sinkContainer);
        }
        {
          UdpClientHelper ulClientHelper(remoteHostAddr, ulPort);
          clientApps.Add(ulClientHelper.Install(ue));
          PacketSinkHelper ulPacketSinkHelper(
              "ns3::UdpSocketFactory",
              InetSocketAddress(Ipv4Address::GetAny(), ulPort));
          ApplicationContainer sinkContainer =
              ulPacketSinkHelper.Install(remoteHost);
          bearerData.ulSink = sinkContainer.Get(0)->GetObject<PacketSink>();
          serverApps.Add(sinkContainer);
        }

        Ptr<EpcTft> tft = Create<EpcTft>();
        {
          EpcTft::PacketFilter dlpf;
          dlpf.localPortStart = dlPort;
          dlpf.localPortEnd = dlPort;
          tft->Add(dlpf);
        }
        {
          EpcTft::PacketFilter ulpf;
          ulpf.remotePortStart = ulPort;
          ulpf.remotePortEnd = ulPort;
          tft->Add(ulpf);
        }

        {
          EpsBearer bearer(EpsBearer::NGBR_VIDEO_TCP_DEFAULT);
          m_lteHelper->ActivateDedicatedEpsBearer(ueDevices.Get(u), bearer,
                                                  tft);
        }
        double d = startTimeSeconds->GetValue();
        Time startTime = Seconds(d);
        serverApps.Start(startTime);
        clientApps.Start(startTime);

        ueData.bearerDataList.push_back(bearerData);
      }

      m_ueDataVector.push_back(ueData);
    }
  } else {
    for (uint32_t u = 0; u < ueDevices.GetN(); ++u) {
      Ptr<NetDevice> ueDev = ueDevices.Get(u);
      for (uint32_t b = 0; b < m_nDedicatedBearers; ++b) {
        EpsBearer::Qci q = EpsBearer::NGBR_VIDEO_TCP_DEFAULT;
        EpsBearer bearer(q);
        m_lteHelper->ActivateDataRadioBearer(ueDev, bearer);
      }
    }
  }

  m_lteHelper->AddX2Interface(enbNodes);

  const Time maxRrcConnectionEstablishmentDuration = Seconds(0.080);
  for (auto it = ueDevices.Begin(); it != ueDevices.End(); ++it) {
    Simulator::Schedule(maxRrcConnectionEstablishmentDuration,
                        &LteX2HandoverTestCase::CheckConnected, this, *it,
                        enbDevices.Get(0));
  }

  Time stopTime = Seconds(0);
  for (auto hoEventIt = m_handoverEventList.begin();
       hoEventIt != m_handoverEventList.end(); ++hoEventIt) {
    Simulator::Schedule(hoEventIt->startTime - MilliSeconds(10),
                        &LteX2HandoverTestCase::TeleportUeToMiddle, this,
                        ueNodes.Get(hoEventIt->ueDeviceIndex));

    Simulator::Schedule(hoEventIt->startTime,
                        &LteX2HandoverTestCase::CheckConnected, this,
                        ueDevices.Get(hoEventIt->ueDeviceIndex),
                        enbDevices.Get(hoEventIt->sourceEnbDeviceIndex));

    m_lteHelper->HandoverRequest(
        hoEventIt->startTime, ueDevices.Get(hoEventIt->ueDeviceIndex),
        enbDevices.Get(hoEventIt->sourceEnbDeviceIndex),
        enbDevices.Get(hoEventIt->targetEnbDeviceIndex));

    Simulator::Schedule(hoEventIt->startTime + MilliSeconds(40),
                        &LteX2HandoverTestCase::TeleportUeNearTargetEnb, this,
                        ueNodes.Get(hoEventIt->ueDeviceIndex),
                        enbNodes.Get(m_admitHo
                                         ? hoEventIt->targetEnbDeviceIndex
                                         : hoEventIt->sourceEnbDeviceIndex));

    Time hoEndTime = hoEventIt->startTime + m_maxHoDuration;
    Simulator::Schedule(hoEndTime, &LteX2HandoverTestCase::CheckConnected, this,
                        ueDevices.Get(hoEventIt->ueDeviceIndex),
                        enbDevices.Get(m_admitHo
                                           ? hoEventIt->targetEnbDeviceIndex
                                           : hoEventIt->sourceEnbDeviceIndex));
    Simulator::Schedule(hoEndTime,
                        &LteX2HandoverTestCase::SaveStatsAfterHandover, this,
                        hoEventIt->ueDeviceIndex);

    Time checkStatsAfterHoTime = hoEndTime + m_statsDuration;
    Simulator::Schedule(checkStatsAfterHoTime,
                        &LteX2HandoverTestCase::CheckStatsAWhileAfterHandover,
                        this, hoEventIt->ueDeviceIndex);
    if (stopTime <= checkStatsAfterHoTime) {
      stopTime = checkStatsAfterHoTime + MilliSeconds(1);
    }
  }

  Simulator::Stop(stopTime);

  Simulator::Run();

  Simulator::Destroy();

  Config::Reset();
  RngSeedManager::SetSeed(previousSeed);
  RngSeedManager::SetRun(previousRun);
}

void LteX2HandoverTestCase::CheckConnected(Ptr<NetDevice> ueDevice,
                                           Ptr<NetDevice> enbDevice) {
  Ptr<LteUeNetDevice> ueLteDevice = ueDevice->GetObject<LteUeNetDevice>();
  Ptr<LteUeRrc> ueRrc = ueLteDevice->GetRrc();
  NS_TEST_ASSERT_MSG_EQ(ueRrc->GetState(), LteUeRrc::CONNECTED_NORMALLY,
                        "Wrong LteUeRrc state!");

  Ptr<LteEnbNetDevice> enbLteDevice = enbDevice->GetObject<LteEnbNetDevice>();
  Ptr<LteEnbRrc> enbRrc = enbLteDevice->GetRrc();
  uint16_t rnti = ueRrc->GetRnti();
  Ptr<UeManager> ueManager = enbRrc->GetUeManager(rnti);
  NS_TEST_ASSERT_MSG_NE(ueManager, nullptr,
                        "RNTI " << rnti << " not found in eNB");

  UeManager::State ueManagerState = ueManager->GetState();
  NS_TEST_ASSERT_MSG_EQ(ueManagerState, UeManager::CONNECTED_NORMALLY,
                        "Wrong UeManager state!");
  NS_ASSERT_MSG(ueManagerState == UeManager::CONNECTED_NORMALLY,
                "Wrong UeManager state!");

  uint16_t ueCellId = ueRrc->GetCellId();
  uint16_t enbCellId = enbLteDevice->GetCellId();
  uint8_t ueDlBandwidth = ueRrc->GetDlBandwidth();
  uint8_t enbDlBandwidth = enbLteDevice->GetDlBandwidth();
  uint8_t ueUlBandwidth = ueRrc->GetUlBandwidth();
  uint8_t enbUlBandwidth = enbLteDevice->GetUlBandwidth();
  uint8_t ueDlEarfcn = ueRrc->GetDlEarfcn();
  uint8_t enbDlEarfcn = enbLteDevice->GetDlEarfcn();
  uint8_t ueUlEarfcn = ueRrc->GetUlEarfcn();
  uint8_t enbUlEarfcn = enbLteDevice->GetUlEarfcn();
  uint64_t ueImsi = ueLteDevice->GetImsi();
  uint64_t enbImsi = ueManager->GetImsi();

  NS_TEST_ASSERT_MSG_EQ(ueImsi, enbImsi, "inconsistent IMSI");
  NS_TEST_ASSERT_MSG_EQ(ueCellId, enbCellId, "inconsistent CellId");
  NS_TEST_ASSERT_MSG_EQ(ueDlBandwidth, enbDlBandwidth,
                        "inconsistent DlBandwidth");
  NS_TEST_ASSERT_MSG_EQ(ueUlBandwidth, enbUlBandwidth,
                        "inconsistent UlBandwidth");
  NS_TEST_ASSERT_MSG_EQ(ueDlEarfcn, enbDlEarfcn, "inconsistent DlEarfcn");
  NS_TEST_ASSERT_MSG_EQ(ueUlEarfcn, enbUlEarfcn, "inconsistent UlEarfcn");

  ObjectMapValue enbDataRadioBearerMapValue;
  ueManager->GetAttribute("DataRadioBearerMap", enbDataRadioBearerMapValue);
  NS_TEST_ASSERT_MSG_EQ(enbDataRadioBearerMapValue.GetN(),
                        m_nDedicatedBearers + 1, "wrong num bearers at eNB");

  ObjectMapValue ueDataRadioBearerMapValue;
  ueRrc->GetAttribute("DataRadioBearerMap", ueDataRadioBearerMapValue);
  NS_TEST_ASSERT_MSG_EQ(ueDataRadioBearerMapValue.GetN(),
                        m_nDedicatedBearers + 1, "wrong num bearers at UE");

  auto enbBearerIt = enbDataRadioBearerMapValue.Begin();
  auto ueBearerIt = ueDataRadioBearerMapValue.Begin();
  while (enbBearerIt != enbDataRadioBearerMapValue.End() &&
         ueBearerIt != ueDataRadioBearerMapValue.End()) {
    Ptr<LteDataRadioBearerInfo> enbDrbInfo =
        enbBearerIt->second->GetObject<LteDataRadioBearerInfo>();
    Ptr<LteDataRadioBearerInfo> ueDrbInfo =
        ueBearerIt->second->GetObject<LteDataRadioBearerInfo>();
    NS_TEST_ASSERT_MSG_EQ((uint32_t)enbDrbInfo->m_epsBearerIdentity,
                          (uint32_t)ueDrbInfo->m_epsBearerIdentity,
                          "epsBearerIdentity differs");
    NS_TEST_ASSERT_MSG_EQ((uint32_t)enbDrbInfo->m_drbIdentity,
                          (uint32_t)ueDrbInfo->m_drbIdentity,
                          "drbIdentity differs");
    NS_TEST_ASSERT_MSG_EQ((uint32_t)enbDrbInfo->m_logicalChannelIdentity,
                          (uint32_t)ueDrbInfo->m_logicalChannelIdentity,
                          "logicalChannelIdentity differs");

    ++enbBearerIt;
    ++ueBearerIt;
  }
  NS_ASSERT_MSG(enbBearerIt == enbDataRadioBearerMapValue.End(),
                "too many bearers at eNB");
  NS_ASSERT_MSG(ueBearerIt == ueDataRadioBearerMapValue.End(),
                "too many bearers at UE");
}

void LteX2HandoverTestCase::TeleportUeToMiddle(Ptr<Node> ueNode) {
  Ptr<MobilityModel> ueMobility = ueNode->GetObject<MobilityModel>();
  ueMobility->SetPosition(Vector(0.0, 0.0, 0.0));
}

void LteX2HandoverTestCase::TeleportUeNearTargetEnb(Ptr<Node> ueNode,
                                                    Ptr<Node> enbNode) {
  Ptr<MobilityModel> enbMobility = enbNode->GetObject<MobilityModel>();
  Vector pos = enbMobility->GetPosition();

  Ptr<MobilityModel> ueMobility = ueNode->GetObject<MobilityModel>();
  ueMobility->SetPosition(pos + Vector(0.0, 100.0, 0.0));
}

void LteX2HandoverTestCase::SaveStatsAfterHandover(uint32_t ueIndex) {
  for (auto it = m_ueDataVector.at(ueIndex).bearerDataList.begin();
       it != m_ueDataVector.at(ueIndex).bearerDataList.end(); ++it) {
    it->dlOldTotalRx = it->dlSink->GetTotalRx();
    it->ulOldTotalRx = it->ulSink->GetTotalRx();
  }
}

void LteX2HandoverTestCase::CheckStatsAWhileAfterHandover(uint32_t ueIndex) {
  uint32_t b = 1;
  for (auto it = m_ueDataVector.at(ueIndex).bearerDataList.begin();
       it != m_ueDataVector.at(ueIndex).bearerDataList.end(); ++it) {
    uint32_t dlRx = it->dlSink->GetTotalRx() - it->dlOldTotalRx;
    uint32_t ulRx = it->ulSink->GetTotalRx() - it->ulOldTotalRx;
    uint32_t expectedBytes =
        m_udpClientPktSize *
        (m_statsDuration / m_udpClientInterval).GetDouble();

    NS_TEST_ASSERT_MSG_EQ(dlRx, expectedBytes,
                          "too few RX bytes in DL, ue=" << ueIndex
                                                        << ", b=" << b);
    NS_TEST_ASSERT_MSG_EQ(ulRx, expectedBytes,
                          "too few RX bytes in UL, ue=" << ueIndex
                                                        << ", b=" << b);
    ++b;
  }
}

class LteX2HandoverTestSuite : public TestSuite {
public:
  LteX2HandoverTestSuite();
};

LteX2HandoverTestSuite::LteX2HandoverTestSuite()
    : TestSuite("lte-x2-handover", SYSTEM) {

  HandoverEvent ue1fwd;
  ue1fwd.startTime = MilliSeconds(100);
  ue1fwd.ueDeviceIndex = 0;
  ue1fwd.sourceEnbDeviceIndex = 0;
  ue1fwd.targetEnbDeviceIndex = 1;

  HandoverEvent ue1bwd;
  ue1bwd.startTime = MilliSeconds(400);
  ue1bwd.ueDeviceIndex = 0;
  ue1bwd.sourceEnbDeviceIndex = 1;
  ue1bwd.targetEnbDeviceIndex = 0;

  HandoverEvent ue1fwdagain;
  ue1fwdagain.startTime = MilliSeconds(700);
  ue1fwdagain.ueDeviceIndex = 0;
  ue1fwdagain.sourceEnbDeviceIndex = 0;
  ue1fwdagain.targetEnbDeviceIndex = 1;

  HandoverEvent ue2fwd;
  ue2fwd.startTime = MilliSeconds(110);
  ue2fwd.ueDeviceIndex = 1;
  ue2fwd.sourceEnbDeviceIndex = 0;
  ue2fwd.targetEnbDeviceIndex = 1;

  HandoverEvent ue2bwd;
  ue2bwd.startTime = MilliSeconds(350);
  ue2bwd.ueDeviceIndex = 1;
  ue2bwd.sourceEnbDeviceIndex = 1;
  ue2bwd.targetEnbDeviceIndex = 0;

  std::string handoverEventList0name("none");
  std::list<HandoverEvent> handoverEventList0;

  std::string handoverEventList1name("1 fwd");
  const std::list<HandoverEvent> handoverEventList1{
      ue1fwd,
  };

  std::string handoverEventList2name("1 fwd & bwd");
  const std::list<HandoverEvent> handoverEventList2{
      ue1fwd,
      ue1bwd,
  };

  std::string handoverEventList3name("1 fwd & bwd & fwd");
  const std::list<HandoverEvent> handoverEventList3{
      ue1fwd,
      ue1bwd,
      ue1fwdagain,
  };

  std::string handoverEventList4name("1+2 fwd");
  const std::list<HandoverEvent> handoverEventList4{
      ue1fwd,
      ue2fwd,
  };

  std::string handoverEventList5name("1+2 fwd & bwd");
  const std::list<HandoverEvent> handoverEventList5{
      ue1fwd,
      ue1bwd,
      ue2fwd,
      ue2bwd,
  };

  std::vector<std::string> schedulers{
      "ns3::RrFfMacScheduler",
      "ns3::PfFfMacScheduler",
  };

  for (auto schedIt = schedulers.begin(); schedIt != schedulers.end();
       ++schedIt) {
    for (auto useIdealRrc : {true, false}) {
      AddTestCase(new LteX2HandoverTestCase(1, 0, handoverEventList0,
                                            handoverEventList0name, *schedIt,
                                            true, useIdealRrc),
                  TestCase::EXTENSIVE);
      AddTestCase(new LteX2HandoverTestCase(2, 0, handoverEventList0,
                                            handoverEventList0name, *schedIt,
                                            true, useIdealRrc),
                  TestCase::EXTENSIVE);
      AddTestCase(new LteX2HandoverTestCase(1, 5, handoverEventList0,
                                            handoverEventList0name, *schedIt,
                                            true, useIdealRrc),
                  TestCase::EXTENSIVE);
      AddTestCase(new LteX2HandoverTestCase(2, 5, handoverEventList0,
                                            handoverEventList0name, *schedIt,
                                            true, useIdealRrc),
                  TestCase::EXTENSIVE);
      AddTestCase(new LteX2HandoverTestCase(1, 0, handoverEventList1,
                                            handoverEventList1name, *schedIt,
                                            true, useIdealRrc),
                  TestCase::EXTENSIVE);
      AddTestCase(new LteX2HandoverTestCase(1, 1, handoverEventList1,
                                            handoverEventList1name, *schedIt,
                                            true, useIdealRrc),
                  TestCase::EXTENSIVE);
      AddTestCase(new LteX2HandoverTestCase(1, 2, handoverEventList1,
                                            handoverEventList1name, *schedIt,
                                            true, useIdealRrc),
                  TestCase::EXTENSIVE);
      AddTestCase(new LteX2HandoverTestCase(1, 0, handoverEventList1,
                                            handoverEventList1name, *schedIt,
                                            false, useIdealRrc),
                  TestCase::EXTENSIVE);
      AddTestCase(new LteX2HandoverTestCase(1, 1, handoverEventList1,
                                            handoverEventList1name, *schedIt,
                                            false, useIdealRrc),
                  TestCase::EXTENSIVE);
      AddTestCase(new LteX2HandoverTestCase(1, 2, handoverEventList1,
                                            handoverEventList1name, *schedIt,
                                            false, useIdealRrc),
                  TestCase::EXTENSIVE);
      AddTestCase(new LteX2HandoverTestCase(2, 0, handoverEventList1,
                                            handoverEventList1name, *schedIt,
                                            true, useIdealRrc),
                  TestCase::EXTENSIVE);
      AddTestCase(new LteX2HandoverTestCase(2, 1, handoverEventList1,
                                            handoverEventList1name, *schedIt,
                                            true, useIdealRrc),
                  TestCase::EXTENSIVE);
      AddTestCase(new LteX2HandoverTestCase(2, 2, handoverEventList1,
                                            handoverEventList1name, *schedIt,
                                            true, useIdealRrc),
                  TestCase::EXTENSIVE);
      AddTestCase(new LteX2HandoverTestCase(2, 0, handoverEventList1,
                                            handoverEventList1name, *schedIt,
                                            false, useIdealRrc),
                  TestCase::EXTENSIVE);
      AddTestCase(new LteX2HandoverTestCase(2, 1, handoverEventList1,
                                            handoverEventList1name, *schedIt,
                                            false, useIdealRrc),
                  TestCase::EXTENSIVE);
      AddTestCase(new LteX2HandoverTestCase(2, 2, handoverEventList1,
                                            handoverEventList1name, *schedIt,
                                            false, useIdealRrc),
                  TestCase::EXTENSIVE);
      AddTestCase(new LteX2HandoverTestCase(1, 0, handoverEventList2,
                                            handoverEventList2name, *schedIt,
                                            true, useIdealRrc),
                  TestCase::EXTENSIVE);
      AddTestCase(new LteX2HandoverTestCase(1, 1, handoverEventList2,
                                            handoverEventList2name, *schedIt,
                                            true, useIdealRrc),
                  TestCase::EXTENSIVE);
      AddTestCase(new LteX2HandoverTestCase(1, 2, handoverEventList2,
                                            handoverEventList2name, *schedIt,
                                            true, useIdealRrc),
                  TestCase::EXTENSIVE);
      AddTestCase(new LteX2HandoverTestCase(1, 0, handoverEventList3,
                                            handoverEventList3name, *schedIt,
                                            true, useIdealRrc),
                  TestCase::EXTENSIVE);
      AddTestCase(new LteX2HandoverTestCase(1, 1, handoverEventList3,
                                            handoverEventList3name, *schedIt,
                                            true, useIdealRrc),
                  TestCase::EXTENSIVE);
      AddTestCase(new LteX2HandoverTestCase(1, 2, handoverEventList3,
                                            handoverEventList3name, *schedIt,
                                            true, useIdealRrc),
                  TestCase::EXTENSIVE);
      AddTestCase(new LteX2HandoverTestCase(2, 0, handoverEventList3,
                                            handoverEventList3name, *schedIt,
                                            true, useIdealRrc),
                  TestCase::EXTENSIVE);
      AddTestCase(new LteX2HandoverTestCase(2, 1, handoverEventList3,
                                            handoverEventList3name, *schedIt,
                                            true, useIdealRrc),
                  TestCase::EXTENSIVE);
      AddTestCase(new LteX2HandoverTestCase(2, 2, handoverEventList3,
                                            handoverEventList3name, *schedIt,
                                            true, useIdealRrc),
                  TestCase::QUICK);
      AddTestCase(new LteX2HandoverTestCase(2, 0, handoverEventList4,
                                            handoverEventList4name, *schedIt,
                                            true, useIdealRrc),
                  TestCase::EXTENSIVE);
      AddTestCase(new LteX2HandoverTestCase(2, 1, handoverEventList4,
                                            handoverEventList4name, *schedIt,
                                            true, useIdealRrc),
                  TestCase::EXTENSIVE);
      AddTestCase(new LteX2HandoverTestCase(2, 2, handoverEventList4,
                                            handoverEventList4name, *schedIt,
                                            true, useIdealRrc),
                  TestCase::EXTENSIVE);
      AddTestCase(new LteX2HandoverTestCase(2, 0, handoverEventList5,
                                            handoverEventList5name, *schedIt,
                                            true, useIdealRrc),
                  TestCase::EXTENSIVE);
      AddTestCase(new LteX2HandoverTestCase(2, 1, handoverEventList5,
                                            handoverEventList5name, *schedIt,
                                            true, useIdealRrc),
                  TestCase::EXTENSIVE);
      AddTestCase(new LteX2HandoverTestCase(2, 2, handoverEventList5,
                                            handoverEventList5name, *schedIt,
                                            true, useIdealRrc),
                  TestCase::EXTENSIVE);
      AddTestCase(new LteX2HandoverTestCase(3, 0, handoverEventList3,
                                            handoverEventList3name, *schedIt,
                                            true, useIdealRrc),
                  TestCase::EXTENSIVE);
      AddTestCase(new LteX2HandoverTestCase(3, 1, handoverEventList3,
                                            handoverEventList3name, *schedIt,
                                            true, useIdealRrc),
                  TestCase::EXTENSIVE);
      AddTestCase(new LteX2HandoverTestCase(3, 2, handoverEventList3,
                                            handoverEventList3name, *schedIt,
                                            true, useIdealRrc),
                  TestCase::EXTENSIVE);
      AddTestCase(new LteX2HandoverTestCase(3, 0, handoverEventList4,
                                            handoverEventList4name, *schedIt,
                                            true, useIdealRrc),
                  TestCase::EXTENSIVE);
      AddTestCase(new LteX2HandoverTestCase(3, 1, handoverEventList4,
                                            handoverEventList4name, *schedIt,
                                            true, useIdealRrc),
                  TestCase::EXTENSIVE);
      AddTestCase(new LteX2HandoverTestCase(3, 2, handoverEventList4,
                                            handoverEventList4name, *schedIt,
                                            true, useIdealRrc),
                  TestCase::EXTENSIVE);
      AddTestCase(new LteX2HandoverTestCase(3, 0, handoverEventList5,
                                            handoverEventList5name, *schedIt,
                                            true, useIdealRrc),
                  TestCase::EXTENSIVE);
      AddTestCase(new LteX2HandoverTestCase(3, 1, handoverEventList5,
                                            handoverEventList5name, *schedIt,
                                            true, useIdealRrc),
                  TestCase::EXTENSIVE);
      AddTestCase(new LteX2HandoverTestCase(3, 2, handoverEventList5,
                                            handoverEventList5name, *schedIt,
                                            true, useIdealRrc),
                  TestCase::QUICK);
    }
  }
}

static LteX2HandoverTestSuite g_lteX2HandoverTestSuiteInstance;
