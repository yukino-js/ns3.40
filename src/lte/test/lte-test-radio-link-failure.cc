
#include "lte-test-radio-link-failure.h"

#include "ns3/config-store-module.h"
#include "ns3/config-store.h"
#include "ns3/core-module.h"
#include "ns3/internet-module.h"
#include "ns3/lte-module.h"
#include "ns3/mobility-module.h"
#include "ns3/network-module.h"
#include "ns3/packet-sink-helper.h"
#include "ns3/point-to-point-module.h"
#include "ns3/udp-client-server-helper.h"

#include <iomanip>
#include <iostream>
#include <stdio.h>
#include <vector>

using namespace ns3;

NS_LOG_COMPONENT_DEFINE("LteRadioLinkFailureTest");

LteRadioLinkFailureTestSuite::LteRadioLinkFailureTestSuite()
    : TestSuite("lte-radio-link-failure", SYSTEM) {
  std::vector<Vector> uePositionList;
  std::vector<Vector> enbPositionList;
  std::vector<Time> checkConnectedList;
  Vector ueJumpAwayPosition;

  uePositionList.emplace_back(10, 0, 0);
  enbPositionList.emplace_back(0, 0, 0);
  ueJumpAwayPosition = Vector(7000.0, 0.0, 0.0);
  checkConnectedList.push_back(Seconds(0.3));
  checkConnectedList.push_back(Seconds(1));

  AddTestCase(new LteRadioLinkFailureTestCase(
                  1, 1, Seconds(2), true, uePositionList, enbPositionList,
                  ueJumpAwayPosition, checkConnectedList),
              TestCase::QUICK);

  AddTestCase(new LteRadioLinkFailureTestCase(
                  1, 1, Seconds(2), false, uePositionList, enbPositionList,
                  ueJumpAwayPosition, checkConnectedList),
              TestCase::QUICK);

  enbPositionList.emplace_back(7020, 0, 0);

  AddTestCase(new LteRadioLinkFailureTestCase(
                  2, 1, Seconds(2), true, uePositionList, enbPositionList,
                  ueJumpAwayPosition, checkConnectedList),
              TestCase::QUICK);

  AddTestCase(new LteRadioLinkFailureTestCase(
                  2, 1, Seconds(2), false, uePositionList, enbPositionList,
                  ueJumpAwayPosition, checkConnectedList),
              TestCase::QUICK);
}

static LteRadioLinkFailureTestSuite g_lteRadioLinkFailureTestSuite;

std::string LteRadioLinkFailureTestCase::BuildNameString(uint32_t numEnbs,
                                                         uint32_t numUes,
                                                         bool isIdealRrc) {
  std::ostringstream oss;
  std::string rrcProtocol;
  if (isIdealRrc) {
    rrcProtocol = "RRC Ideal";
  } else {
    rrcProtocol = "RRC Real";
  }
  oss << numEnbs << " eNBs, " << numUes << " UEs, " << rrcProtocol
      << " Protocol";
  return oss.str();
}

LteRadioLinkFailureTestCase::LteRadioLinkFailureTestCase(
    uint32_t numEnbs, uint32_t numUes, Time simTime, bool isIdealRrc,
    std::vector<Vector> uePositionList, std::vector<Vector> enbPositionList,
    Vector ueJumpAwayPosition, std::vector<Time> checkConnectedList)
    : TestCase(BuildNameString(numEnbs, numUes, isIdealRrc)),
      m_numEnbs(numEnbs), m_numUes(numUes), m_simTime(simTime),
      m_isIdealRrc(isIdealRrc), m_uePositionList(uePositionList),
      m_enbPositionList(enbPositionList),
      m_checkConnectedList(checkConnectedList),
      m_ueJumpAwayPosition(ueJumpAwayPosition) {
  NS_LOG_FUNCTION(this << GetName());
  m_lastState = LteUeRrc::NUM_STATES;
  m_radioLinkFailureDetected = false;
  m_numOfInSyncIndications = 0;
  m_numOfOutOfSyncIndications = 0;
}

LteRadioLinkFailureTestCase::~LteRadioLinkFailureTestCase() {
  NS_LOG_FUNCTION(this << GetName());
}

void LteRadioLinkFailureTestCase::DoRun() {

  Config::SetDefault("ns3::MacStatsCalculator::DlOutputFilename",
                     StringValue(CreateTempDirFilename("DlMacStats.txt")));
  Config::SetDefault("ns3::MacStatsCalculator::UlOutputFilename",
                     StringValue(CreateTempDirFilename("UlMacStats.txt")));
  Config::SetDefault("ns3::RadioBearerStatsCalculator::DlRlcOutputFilename",
                     StringValue(CreateTempDirFilename("DlRlcStats.txt")));
  Config::SetDefault("ns3::RadioBearerStatsCalculator::UlRlcOutputFilename",
                     StringValue(CreateTempDirFilename("UlRlcStats.txt")));
  Config::SetDefault("ns3::RadioBearerStatsCalculator::DlPdcpOutputFilename",
                     StringValue(CreateTempDirFilename("DlPdcpStats.txt")));
  Config::SetDefault("ns3::RadioBearerStatsCalculator::UlPdcpOutputFilename",
                     StringValue(CreateTempDirFilename("UlPdcpStats.txt")));
  Config::SetDefault("ns3::PhyStatsCalculator::DlRsrpSinrFilename",
                     StringValue(CreateTempDirFilename("DlRsrpSinrStats.txt")));
  Config::SetDefault("ns3::PhyStatsCalculator::UlSinrFilename",
                     StringValue(CreateTempDirFilename("UlSinrStats.txt")));
  Config::SetDefault(
      "ns3::PhyStatsCalculator::UlInterferenceFilename",
      StringValue(CreateTempDirFilename("UlInterferenceStats.txt")));
  Config::SetDefault("ns3::PhyRxStatsCalculator::DlRxOutputFilename",
                     StringValue(CreateTempDirFilename("DlRxPhyStats.txt")));
  Config::SetDefault("ns3::PhyRxStatsCalculator::UlRxOutputFilename",
                     StringValue(CreateTempDirFilename("UlRxPhyStats.txt")));
  Config::SetDefault("ns3::PhyTxStatsCalculator::DlTxOutputFilename",
                     StringValue(CreateTempDirFilename("DlTxPhyStats.txt")));
  Config::SetDefault("ns3::PhyTxStatsCalculator::UlTxOutputFilename",
                     StringValue(CreateTempDirFilename("UlTxPhyStats.txt")));

  NS_LOG_FUNCTION(this << GetName());
  uint16_t numBearersPerUe = 1;
  Time simTime = m_simTime;
  double eNodeB_txPower = 43;

  Config::SetDefault("ns3::LteHelper::UseIdealRrc", BooleanValue(m_isIdealRrc));

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

  Config::SetDefault("ns3::LteUeRrc::N310", UintegerValue(1));
  Config::SetDefault("ns3::LteUeRrc::N311", UintegerValue(1));
  Config::SetDefault("ns3::LteUeRrc::T310", TimeValue(Seconds(1)));

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

  NodeContainer enbNodes;
  NodeContainer ueNodes;
  enbNodes.Create(m_numEnbs);
  ueNodes.Create(m_numUes);

  Ptr<ListPositionAllocator> positionAllocEnb =
      CreateObject<ListPositionAllocator>();

  for (auto enbPosIt = m_enbPositionList.begin();
       enbPosIt != m_enbPositionList.end(); ++enbPosIt) {
    positionAllocEnb->Add(*enbPosIt);
  }
  MobilityHelper mobility;
  mobility.SetMobilityModel("ns3::ConstantPositionMobilityModel");
  mobility.SetPositionAllocator(positionAllocEnb);
  mobility.Install(enbNodes);

  Ptr<ListPositionAllocator> positionAllocUe =
      CreateObject<ListPositionAllocator>();

  for (auto uePosIt = m_uePositionList.begin();
       uePosIt != m_uePositionList.end(); ++uePosIt) {
    positionAllocUe->Add(*uePosIt);
  }

  mobility.SetPositionAllocator(positionAllocUe);
  mobility.SetMobilityModel("ns3::ConstantPositionMobilityModel");
  mobility.Install(ueNodes);
  m_ueMobility = ueNodes.Get(0)->GetObject<MobilityModel>();

  NetDeviceContainer enbDevs;
  NetDeviceContainer ueDevs;

  int64_t randomStream = 1;
  enbDevs = lteHelper->InstallEnbDevice(enbNodes);
  randomStream += lteHelper->AssignStreams(enbDevs, randomStream);
  ueDevs = lteHelper->InstallUeDevice(ueNodes);
  randomStream += lteHelper->AssignStreams(ueDevs, randomStream);

  internet.Install(ueNodes);
  Ipv4InterfaceContainer ueIpIfaces;
  ueIpIfaces = epcHelper->AssignUeIpv4Address(NetDeviceContainer(ueDevs));

  lteHelper->Attach(ueDevs);

  uint16_t dlPort = 10000;
  uint16_t ulPort = 20000;

  DataRateValue dataRateValue = DataRate("18.6Mbps");
  uint64_t bitRate = dataRateValue.Get().GetBitRate();
  uint32_t packetSize = 1024;
  NS_LOG_DEBUG("bit rate " << bitRate);
  double interPacketInterval = static_cast<double>(packetSize * 8) / bitRate;
  Time udpInterval = Seconds(interPacketInterval);

  NS_LOG_DEBUG("UDP will use application interval " << udpInterval.As(Time::S));

  for (uint32_t u = 0; u < m_numUes; ++u) {
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
      dlClientHelper.SetAttribute("MaxPackets", UintegerValue(1000000));
      dlClientApps.Add(dlClientHelper.Install(remoteHost));

      PacketSinkHelper dlPacketSinkHelper(
          "ns3::UdpSocketFactory",
          InetSocketAddress(Ipv4Address::GetAny(), dlPort));
      dlServerApps.Add(dlPacketSinkHelper.Install(ue));

      NS_LOG_LOGIC("installing UDP UL app for UE " << u + 1);
      UdpClientHelper ulClientHelper(remoteHostAddr, ulPort);
      ulClientHelper.SetAttribute("Interval", TimeValue(udpInterval));
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

  lteHelper->EnableTraces();

  for (uint32_t u = 0; u < m_numUes; ++u) {
    Simulator::Schedule(m_checkConnectedList.at(u),
                        &LteRadioLinkFailureTestCase::CheckConnected, this,
                        ueDevs.Get(u), enbDevs);
  }

  Simulator::Schedule(Seconds(0.4), &LteRadioLinkFailureTestCase::JumpAway,
                      this, m_ueJumpAwayPosition);

  Config::Connect(
      "/NodeList/*/DeviceList/*/LteEnbRrc/ConnectionEstablished",
      MakeCallback(
          &LteRadioLinkFailureTestCase::ConnectionEstablishedEnbCallback,
          this));
  Config::Connect(
      "/NodeList/*/DeviceList/*/LteUeRrc/ConnectionEstablished",
      MakeCallback(
          &LteRadioLinkFailureTestCase::ConnectionEstablishedUeCallback, this));
  Config::Connect(
      "/NodeList/*/DeviceList/*/LteUeRrc/StateTransition",
      MakeCallback(&LteRadioLinkFailureTestCase::UeStateTransitionCallback,
                   this));
  Config::Connect(
      "/NodeList/*/DeviceList/*/LteEnbRrc/NotifyConnectionRelease",
      MakeCallback(&LteRadioLinkFailureTestCase::ConnectionReleaseAtEnbCallback,
                   this));
  Config::Connect(
      "/NodeList/*/DeviceList/*/LteUeRrc/PhySyncDetection",
      MakeCallback(&LteRadioLinkFailureTestCase::PhySyncDetectionCallback,
                   this));
  Config::Connect(
      "/NodeList/*/DeviceList/*/LteUeRrc/RadioLinkFailure",
      MakeCallback(&LteRadioLinkFailureTestCase::RadioLinkFailureCallback,
                   this));

  Simulator::Stop(simTime);

  Simulator::Run();
  for (uint32_t u = 0; u < m_numUes; ++u) {
    NS_TEST_ASSERT_MSG_EQ(m_radioLinkFailureDetected, true,
                          "Error, UE transitions to idle state for other than "
                          "radio link failure");
    CheckIdle(ueDevs.Get(u), enbDevs);
  }
  Simulator::Destroy();
}

void LteRadioLinkFailureTestCase::JumpAway(Vector UeJumpAwayPosition) {
  NS_LOG_FUNCTION(this);

  m_ueMobility->SetPosition(UeJumpAwayPosition);
}

void LteRadioLinkFailureTestCase::CheckConnected(
    Ptr<NetDevice> ueDevice, NetDeviceContainer enbDevices) {
  NS_LOG_FUNCTION(ueDevice);

  Ptr<LteUeNetDevice> ueLteDevice = ueDevice->GetObject<LteUeNetDevice>();
  Ptr<LteUeRrc> ueRrc = ueLteDevice->GetRrc();
  NS_TEST_ASSERT_MSG_EQ(ueRrc->GetState(), LteUeRrc::CONNECTED_NORMALLY,
                        "Wrong LteUeRrc state!");
  uint16_t cellId = ueRrc->GetCellId();

  Ptr<LteEnbNetDevice> enbLteDevice;

  for (auto enbDevIt = enbDevices.Begin(); enbDevIt != enbDevices.End();
       ++enbDevIt) {
    if (((*enbDevIt)->GetObject<LteEnbNetDevice>())->HasCellId(cellId)) {
      enbLteDevice = (*enbDevIt)->GetObject<LteEnbNetDevice>();
    }
  }

  NS_TEST_ASSERT_MSG_NE(enbLteDevice, nullptr, "LTE eNB device not found");
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
  NS_TEST_ASSERT_MSG_EQ(enbDataRadioBearerMapValue.GetN(), 1 + 1,
                        "wrong num bearers at eNB");

  ObjectMapValue ueDataRadioBearerMapValue;
  ueRrc->GetAttribute("DataRadioBearerMap", ueDataRadioBearerMapValue);
  NS_TEST_ASSERT_MSG_EQ(ueDataRadioBearerMapValue.GetN(), 1 + 1,
                        "wrong num bearers at UE");

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

void LteRadioLinkFailureTestCase::CheckIdle(Ptr<NetDevice> ueDevice,
                                            NetDeviceContainer enbDevices) {
  NS_LOG_FUNCTION(ueDevice);

  Ptr<LteUeNetDevice> ueLteDevice = ueDevice->GetObject<LteUeNetDevice>();
  Ptr<LteUeRrc> ueRrc = ueLteDevice->GetRrc();
  uint16_t rnti = ueRrc->GetRnti();
  uint32_t numEnbDevices = enbDevices.GetN();
  bool ueManagerFound = false;

  switch (numEnbDevices) {
  case 1:
    NS_TEST_ASSERT_MSG_EQ(ueRrc->GetState(), LteUeRrc::IDLE_CELL_SEARCH,
                          "Wrong LteUeRrc state!");
    ueManagerFound = CheckUeExistAtEnb(rnti, enbDevices.Get(0));
    NS_TEST_ASSERT_MSG_EQ(ueManagerFound, false,
                          "Unexpected RNTI with value " << rnti
                                                        << " found in eNB");
    break;
  case 2:
    NS_TEST_ASSERT_MSG_EQ(ueRrc->GetState(), LteUeRrc::CONNECTED_NORMALLY,
                          "Wrong LteUeRrc state!");
    ueManagerFound = CheckUeExistAtEnb(rnti, enbDevices.Get(1));
    NS_TEST_ASSERT_MSG_EQ(ueManagerFound, true,
                          "RNTI " << rnti << " is not attached to the eNB");
    break;
  default:
    NS_FATAL_ERROR(
        "The RRC state of the UE in more then 2 eNB scenario is not defined. "
        "Consider creating more cases");
    break;
  }
}

bool LteRadioLinkFailureTestCase::CheckUeExistAtEnb(uint16_t rnti,
                                                    Ptr<NetDevice> enbDevice) {
  NS_LOG_FUNCTION(this << rnti);
  Ptr<LteEnbNetDevice> enbLteDevice = DynamicCast<LteEnbNetDevice>(enbDevice);
  NS_ABORT_MSG_IF(!enbLteDevice, "LTE eNB device not found");
  Ptr<LteEnbRrc> enbRrc = enbLteDevice->GetRrc();
  bool ueManagerFound = enbRrc->HasUeManager(rnti);
  return ueManagerFound;
}

void LteRadioLinkFailureTestCase::UeStateTransitionCallback(
    std::string context, uint64_t imsi, uint16_t cellId, uint16_t rnti,
    LteUeRrc::State oldState, LteUeRrc::State newState) {
  NS_LOG_FUNCTION(this << imsi << cellId << rnti << oldState << newState);
  m_lastState = newState;
}

void LteRadioLinkFailureTestCase::ConnectionEstablishedEnbCallback(
    std::string context, uint64_t imsi, uint16_t cellId, uint16_t rnti) {
  NS_LOG_FUNCTION(this << imsi << cellId << rnti);
}

void LteRadioLinkFailureTestCase::ConnectionEstablishedUeCallback(
    std::string context, uint64_t imsi, uint16_t cellId, uint16_t rnti) {
  NS_LOG_FUNCTION(this << imsi << cellId << rnti);
  NS_TEST_ASSERT_MSG_EQ(
      m_numOfOutOfSyncIndications, 0,
      "radio link failure detection should start only in RRC CONNECTED state");
  NS_TEST_ASSERT_MSG_EQ(
      m_numOfInSyncIndications, 0,
      "radio link failure detection should start only in RRC CONNECTED state");
}

void LteRadioLinkFailureTestCase::ConnectionReleaseAtEnbCallback(
    std::string context, uint64_t imsi, uint16_t cellId, uint16_t rnti) {
  NS_LOG_FUNCTION(this << imsi << cellId << rnti);
}

void LteRadioLinkFailureTestCase::PhySyncDetectionCallback(
    std::string context, uint64_t imsi, uint16_t rnti, uint16_t cellId,
    std::string type, uint8_t count) {
  NS_LOG_FUNCTION(this << imsi << cellId << rnti);
  if (type == "Notify out of sync") {
    m_numOfOutOfSyncIndications = count;
  } else if (type == "Notify in sync") {
    m_numOfInSyncIndications = count;
  }
}

void LteRadioLinkFailureTestCase::RadioLinkFailureCallback(std::string context,
                                                           uint64_t imsi,
                                                           uint16_t cellId,
                                                           uint16_t rnti) {
  NS_LOG_FUNCTION(this << imsi << cellId << rnti);
  NS_LOG_DEBUG("RLF at " << Simulator::Now());
  m_radioLinkFailureDetected = true;
  NS_TEST_ASSERT_MSG_EQ(m_numOfOutOfSyncIndications, 1,
                        "wrong number of out-of-sync indications detected, "
                        "check configured value for N310");
  NS_TEST_ASSERT_MSG_LT(m_numOfInSyncIndications, 1,
                        "wrong number of out-of-sync indications detected, "
                        "check configured value for N311");
  m_numOfOutOfSyncIndications = 0;
}
