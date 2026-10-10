
#include "lte-test-fdtbfq-ff-mac-scheduler.h"

#include "ns3/double.h"
#include "ns3/internet-module.h"
#include "ns3/ipv4-global-routing-helper.h"
#include "ns3/network-module.h"
#include "ns3/packet-sink-helper.h"
#include "ns3/point-to-point-epc-helper.h"
#include "ns3/point-to-point-helper.h"
#include "ns3/radio-bearer-stats-calculator.h"
#include "ns3/string.h"
#include "ns3/udp-client-server-helper.h"
#include <ns3/boolean.h>
#include <ns3/constant-position-mobility-model.h>
#include <ns3/enum.h>
#include <ns3/eps-bearer.h>
#include <ns3/ff-mac-scheduler.h>
#include <ns3/log.h>
#include <ns3/lte-enb-net-device.h>
#include <ns3/lte-enb-phy.h>
#include <ns3/lte-helper.h>
#include <ns3/lte-ue-net-device.h>
#include <ns3/lte-ue-phy.h>
#include <ns3/lte-ue-rrc.h>
#include <ns3/mobility-helper.h>
#include <ns3/net-device-container.h>
#include <ns3/node-container.h>
#include <ns3/object.h>
#include <ns3/packet.h>
#include <ns3/ptr.h>
#include <ns3/simulator.h>
#include <ns3/spectrum-error-model.h>
#include <ns3/spectrum-interference.h>
#include <ns3/test.h>

#include <iostream>
#include <sstream>
#include <string>

using namespace ns3;

NS_LOG_COMPONENT_DEFINE("LenaTestFdTbfqFfMacScheduler");

LenaTestFdTbfqFfMacSchedulerSuite::LenaTestFdTbfqFfMacSchedulerSuite()
    : TestSuite("lte-fdtbfq-ff-mac-scheduler", SYSTEM) {
  NS_LOG_INFO("creating LenaTestFdTbfqFfMacSchedulerSuite");

  bool errorModel = false;

  AddTestCase(new LenaFdTbfqFfMacSchedulerTestCase1(1, 0, 232000, 232000, 200,
                                                    1, errorModel),
              TestCase::EXTENSIVE);
  AddTestCase(new LenaFdTbfqFfMacSchedulerTestCase1(3, 0, 232000, 232000, 200,
                                                    1, errorModel),
              TestCase::EXTENSIVE);
  AddTestCase(new LenaFdTbfqFfMacSchedulerTestCase1(6, 0, 232000, 232000, 200,
                                                    1, errorModel),
              TestCase::EXTENSIVE);

  AddTestCase(new LenaFdTbfqFfMacSchedulerTestCase1(1, 4800, 232000, 232000,
                                                    200, 1, errorModel),
              TestCase::EXTENSIVE);
  AddTestCase(new LenaFdTbfqFfMacSchedulerTestCase1(3, 4800, 232000, 232000,
                                                    200, 1, errorModel),
              TestCase::EXTENSIVE);
  AddTestCase(new LenaFdTbfqFfMacSchedulerTestCase1(6, 4800, 230500, 125000,
                                                    200, 1, errorModel),
              TestCase::EXTENSIVE);

  AddTestCase(new LenaFdTbfqFfMacSchedulerTestCase1(1, 6000, 232000, 232000,
                                                    200, 1, errorModel),
              TestCase::EXTENSIVE);
  AddTestCase(new LenaFdTbfqFfMacSchedulerTestCase1(3, 6000, 232000, 201000,
                                                    200, 1, errorModel),
              TestCase::EXTENSIVE);
  AddTestCase(new LenaFdTbfqFfMacSchedulerTestCase1(6, 6000, 198500, 97000, 200,
                                                    1, errorModel),
              TestCase::EXTENSIVE);

  AddTestCase(new LenaFdTbfqFfMacSchedulerTestCase1(1, 10000, 232000, 232000,
                                                    200, 1, errorModel),
              TestCase::EXTENSIVE);
  AddTestCase(new LenaFdTbfqFfMacSchedulerTestCase1(3, 10000, 232000, 137000,
                                                    200, 1, errorModel),
              TestCase::EXTENSIVE);
  AddTestCase(new LenaFdTbfqFfMacSchedulerTestCase1(6, 10000, 129166, 67000,
                                                    200, 1, errorModel),
              TestCase::EXTENSIVE);

  AddTestCase(new LenaFdTbfqFfMacSchedulerTestCase1(1, 100000, 0, 0, 200, 1,
                                                    errorModel),
              TestCase::QUICK);

  std::vector<double> dist1;
  dist1.push_back(0);
  dist1.push_back(4800);
  dist1.push_back(6000);
  dist1.push_back(10000);
  std::vector<uint16_t> packetSize1;
  packetSize1.push_back(100);
  packetSize1.push_back(100);
  packetSize1.push_back(100);
  packetSize1.push_back(100);
  std::vector<uint32_t> estThrFdTbfqDl1;
  estThrFdTbfqDl1.push_back(132000);
  estThrFdTbfqDl1.push_back(132000);
  estThrFdTbfqDl1.push_back(132000);
  estThrFdTbfqDl1.push_back(132000);
  AddTestCase(new LenaFdTbfqFfMacSchedulerTestCase2(dist1, estThrFdTbfqDl1,
                                                    packetSize1, 1, errorModel),
              TestCase::EXTENSIVE);

  std::vector<double> dist2;
  dist2.push_back(0);
  dist2.push_back(4800);
  dist2.push_back(6000);
  dist2.push_back(10000);
  std::vector<uint16_t> packetSize2;
  packetSize2.push_back(300);
  packetSize2.push_back(300);
  packetSize2.push_back(300);
  packetSize2.push_back(300);
  std::vector<uint32_t> estThrFdTbfqDl2;
  estThrFdTbfqDl2.push_back(302266);
  estThrFdTbfqDl2.push_back(302266);
  estThrFdTbfqDl2.push_back(302266);
  estThrFdTbfqDl2.push_back(302266);
  AddTestCase(new LenaFdTbfqFfMacSchedulerTestCase2(dist2, estThrFdTbfqDl2,
                                                    packetSize2, 1, errorModel),
              TestCase::EXTENSIVE);

  std::vector<double> dist3;
  dist3.push_back(0);
  dist3.push_back(4800);
  dist3.push_back(6000);
  std::vector<uint16_t> packetSize3;
  packetSize3.push_back(100);
  packetSize3.push_back(200);
  packetSize3.push_back(300);
  std::vector<uint32_t> estThrFdTbfqDl3;
  estThrFdTbfqDl3.push_back(132000);
  estThrFdTbfqDl3.push_back(232000);
  estThrFdTbfqDl3.push_back(332000);
  AddTestCase(new LenaFdTbfqFfMacSchedulerTestCase2(dist3, estThrFdTbfqDl3,
                                                    packetSize3, 1, errorModel),
              TestCase::QUICK);
}

static LenaTestFdTbfqFfMacSchedulerSuite lenaTestFdTbfqFfMacSchedulerSuite;

std::string LenaFdTbfqFfMacSchedulerTestCase1::BuildNameString(uint16_t nUser,
                                                               double dist) {
  std::ostringstream oss;
  oss << nUser << " UEs, distance " << dist << " m";
  return oss.str();
}

LenaFdTbfqFfMacSchedulerTestCase1::LenaFdTbfqFfMacSchedulerTestCase1(
    uint16_t nUser, double dist, double thrRefDl, double thrRefUl,
    uint16_t packetSize, uint16_t interval, bool errorModelEnabled)
    : TestCase(BuildNameString(nUser, dist)), m_nUser(nUser), m_dist(dist),
      m_packetSize(packetSize), m_interval(interval), m_thrRefDl(thrRefDl),
      m_thrRefUl(thrRefUl), m_errorModelEnabled(errorModelEnabled) {}

LenaFdTbfqFfMacSchedulerTestCase1::~LenaFdTbfqFfMacSchedulerTestCase1() {}

void LenaFdTbfqFfMacSchedulerTestCase1::DoRun() {
  NS_LOG_FUNCTION(this << GetName());

  if (!m_errorModelEnabled) {
    Config::SetDefault("ns3::LteSpectrumPhy::CtrlErrorModelEnabled",
                       BooleanValue(false));
    Config::SetDefault("ns3::LteSpectrumPhy::DataErrorModelEnabled",
                       BooleanValue(false));
  }

  Config::SetDefault("ns3::LteHelper::UseIdealRrc", BooleanValue(true));
  Config::SetDefault("ns3::MacStatsCalculator::DlOutputFilename",
                     StringValue(CreateTempDirFilename("DlMacStats.txt")));
  Config::SetDefault("ns3::MacStatsCalculator::UlOutputFilename",
                     StringValue(CreateTempDirFilename("UlMacStats.txt")));
  Config::SetDefault("ns3::RadioBearerStatsCalculator::DlRlcOutputFilename",
                     StringValue(CreateTempDirFilename("DlRlcStats.txt")));
  Config::SetDefault("ns3::RadioBearerStatsCalculator::UlRlcOutputFilename",
                     StringValue(CreateTempDirFilename("UlRlcStats.txt")));

  Ptr<LteHelper> lteHelper = CreateObject<LteHelper>();
  Ptr<PointToPointEpcHelper> epcHelper = CreateObject<PointToPointEpcHelper>();
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
  p2ph.SetChannelAttribute("Delay", TimeValue(Seconds(0.001)));
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

  lteHelper->SetAttribute(
      "PathlossModel", StringValue("ns3::FriisSpectrumPropagationLossModel"));

  NodeContainer enbNodes;
  NodeContainer ueNodes;
  enbNodes.Create(1);
  ueNodes.Create(m_nUser);

  MobilityHelper mobility;
  mobility.SetMobilityModel("ns3::ConstantPositionMobilityModel");
  mobility.Install(enbNodes);
  mobility.SetMobilityModel("ns3::ConstantPositionMobilityModel");
  mobility.Install(ueNodes);

  NetDeviceContainer enbDevs;
  NetDeviceContainer ueDevs;
  lteHelper->SetSchedulerType("ns3::FdTbfqFfMacScheduler");
  lteHelper->SetSchedulerAttribute("UlCqiFilter",
                                   EnumValue(FfMacScheduler::SRS_UL_CQI));
  enbDevs = lteHelper->InstallEnbDevice(enbNodes);
  ueDevs = lteHelper->InstallUeDevice(ueNodes);

  Ptr<LteEnbNetDevice> lteEnbDev = enbDevs.Get(0)->GetObject<LteEnbNetDevice>();
  Ptr<LteEnbPhy> enbPhy = lteEnbDev->GetPhy();
  enbPhy->SetAttribute("TxPower", DoubleValue(30.0));
  enbPhy->SetAttribute("NoiseFigure", DoubleValue(5.0));

  for (int i = 0; i < m_nUser; i++) {
    Ptr<ConstantPositionMobilityModel> mm =
        ueNodes.Get(i)->GetObject<ConstantPositionMobilityModel>();
    mm->SetPosition(Vector(m_dist, 0.0, 0.0));
    Ptr<LteUeNetDevice> lteUeDev = ueDevs.Get(i)->GetObject<LteUeNetDevice>();
    Ptr<LteUePhy> uePhy = lteUeDev->GetPhy();
    uePhy->SetAttribute("TxPower", DoubleValue(23.0));
    uePhy->SetAttribute("NoiseFigure", DoubleValue(9.0));
  }

  internet.Install(ueNodes);
  Ipv4InterfaceContainer ueIpIface;
  ueIpIface = epcHelper->AssignUeIpv4Address(NetDeviceContainer(ueDevs));

  for (uint32_t u = 0; u < ueNodes.GetN(); ++u) {
    Ptr<Node> ueNode = ueNodes.Get(u);
    Ptr<Ipv4StaticRouting> ueStaticRouting =
        ipv4RoutingHelper.GetStaticRouting(ueNode->GetObject<Ipv4>());
    ueStaticRouting->SetDefaultRoute(epcHelper->GetUeDefaultGatewayAddress(),
                                     1);
  }

  lteHelper->Attach(ueDevs, enbDevs.Get(0));

  for (uint32_t u = 0; u < ueNodes.GetN(); ++u) {
    Ptr<NetDevice> ueDevice = ueDevs.Get(u);
    GbrQosInformation qos;
    qos.gbrDl = (m_packetSize + 32) * (1000 / m_interval) * 8;
    qos.gbrUl = 0;
    qos.mbrDl = qos.gbrDl;
    qos.mbrUl = 0;

    EpsBearer::Qci q = EpsBearer::GBR_CONV_VOICE;
    EpsBearer bearer(q, qos);
    lteHelper->ActivateDedicatedEpsBearer(ueDevice, bearer, EpcTft::Default());
  }

  uint16_t dlPort = 1234;
  uint16_t ulPort = 2000;
  PacketSinkHelper dlPacketSinkHelper(
      "ns3::UdpSocketFactory",
      InetSocketAddress(Ipv4Address::GetAny(), dlPort));
  ApplicationContainer clientApps;
  ApplicationContainer serverApps;

  for (uint32_t u = 0; u < ueNodes.GetN(); ++u) {
    ++ulPort;
    PacketSinkHelper ulPacketSinkHelper(
        "ns3::UdpSocketFactory",
        InetSocketAddress(Ipv4Address::GetAny(), ulPort));
    serverApps.Add(ulPacketSinkHelper.Install(remoteHost));

    serverApps.Add(dlPacketSinkHelper.Install(ueNodes.Get(u)));

    UdpClientHelper dlClient(ueIpIface.GetAddress(u), dlPort);
    dlClient.SetAttribute("Interval", TimeValue(MilliSeconds(m_interval)));
    dlClient.SetAttribute("MaxPackets", UintegerValue(1000000));
    dlClient.SetAttribute("PacketSize", UintegerValue(m_packetSize));

    UdpClientHelper ulClient(remoteHostAddr, ulPort);
    ulClient.SetAttribute("Interval", TimeValue(MilliSeconds(m_interval)));
    ulClient.SetAttribute("MaxPackets", UintegerValue(1000000));
    ulClient.SetAttribute("PacketSize", UintegerValue(m_packetSize));

    clientApps.Add(dlClient.Install(remoteHost));
    clientApps.Add(ulClient.Install(ueNodes.Get(u)));
  }

  serverApps.Start(Seconds(0.001));
  clientApps.Start(Seconds(0.04));

  double statsStartTime = 0.04;
  double statsDuration = 1;
  double tolerance = 0.1;
  Simulator::Stop(Seconds(statsStartTime + statsDuration - 0.0001));

  lteHelper->EnableRlcTraces();
  lteHelper->EnableMacTraces();
  Ptr<RadioBearerStatsCalculator> rlcStats = lteHelper->GetRlcStats();
  rlcStats->SetAttribute("StartTime", TimeValue(Seconds(statsStartTime)));
  rlcStats->SetAttribute("EpochDuration", TimeValue(Seconds(statsDuration)));

  Simulator::Run();

  NS_LOG_INFO("DL - Test with " << m_nUser << " user(s) at distance "
                                << m_dist);
  std::vector<uint64_t> dlDataRxed;
  for (int i = 0; i < m_nUser; i++) {
    uint64_t imsi = ueDevs.Get(i)->GetObject<LteUeNetDevice>()->GetImsi();
    uint8_t lcId = 4;
    uint64_t data = rlcStats->GetDlRxData(imsi, lcId);
    dlDataRxed.push_back(data);
    NS_LOG_INFO("\tUser " << i << " imsi " << imsi << " bytes rxed "
                          << (double)dlDataRxed.at(i) << "  thr "
                          << (double)dlDataRxed.at(i) / statsDuration << " ref "
                          << m_thrRefDl);
  }

  for (int i = 0; i < m_nUser; i++) {
    NS_TEST_ASSERT_MSG_EQ_TOL((double)dlDataRxed.at(i) / statsDuration,
                              m_thrRefDl, m_thrRefDl * tolerance,
                              " Unfair Throughput!");
  }

  NS_LOG_INFO("UL - Test with " << m_nUser << " user(s) at distance "
                                << m_dist);
  std::vector<uint64_t> ulDataRxed;
  for (int i = 0; i < m_nUser; i++) {
    uint64_t imsi = ueDevs.Get(i)->GetObject<LteUeNetDevice>()->GetImsi();
    uint8_t lcId = 4;
    ulDataRxed.push_back(rlcStats->GetUlRxData(imsi, lcId));
    NS_LOG_INFO("\tUser " << i << " imsi " << imsi << " bytes rxed "
                          << (double)ulDataRxed.at(i) << "  thr "
                          << (double)ulDataRxed.at(i) / statsDuration << " ref "
                          << m_thrRefUl);
  }

  for (int i = 0; i < m_nUser; i++) {
    NS_TEST_ASSERT_MSG_EQ_TOL((double)ulDataRxed.at(i) / statsDuration,
                              m_thrRefUl, m_thrRefUl * tolerance,
                              " Unfair Throughput!");
  }
  Simulator::Destroy();
}

std::string
LenaFdTbfqFfMacSchedulerTestCase2::BuildNameString(uint16_t nUser,
                                                   std::vector<double> dist) {
  std::ostringstream oss;
  oss << "distances (m) = [ ";
  for (auto it = dist.begin(); it != dist.end(); ++it) {
    oss << *it << " ";
  }
  oss << "]";
  return oss.str();
}

LenaFdTbfqFfMacSchedulerTestCase2::LenaFdTbfqFfMacSchedulerTestCase2(
    std::vector<double> dist, std::vector<uint32_t> estThrFdTbfqDl,
    std::vector<uint16_t> packetSize, uint16_t interval, bool errorModelEnabled)
    : TestCase(BuildNameString(dist.size(), dist)), m_nUser(dist.size()),
      m_dist(dist), m_packetSize(packetSize), m_interval(interval),
      m_estThrFdTbfqDl(estThrFdTbfqDl), m_errorModelEnabled(errorModelEnabled) {
}

LenaFdTbfqFfMacSchedulerTestCase2::~LenaFdTbfqFfMacSchedulerTestCase2() {}

void LenaFdTbfqFfMacSchedulerTestCase2::DoRun() {
  if (!m_errorModelEnabled) {
    Config::SetDefault("ns3::LteSpectrumPhy::CtrlErrorModelEnabled",
                       BooleanValue(false));
    Config::SetDefault("ns3::LteSpectrumPhy::DataErrorModelEnabled",
                       BooleanValue(false));
  }

  Config::SetDefault("ns3::LteHelper::UseIdealRrc", BooleanValue(true));
  Config::SetDefault("ns3::MacStatsCalculator::DlOutputFilename",
                     StringValue(CreateTempDirFilename("DlMacStats.txt")));
  Config::SetDefault("ns3::MacStatsCalculator::UlOutputFilename",
                     StringValue(CreateTempDirFilename("UlMacStats.txt")));
  Config::SetDefault("ns3::RadioBearerStatsCalculator::DlRlcOutputFilename",
                     StringValue(CreateTempDirFilename("DlRlcStats.txt")));
  Config::SetDefault("ns3::RadioBearerStatsCalculator::UlRlcOutputFilename",
                     StringValue(CreateTempDirFilename("UlRlcStats.txt")));

  Ptr<LteHelper> lteHelper = CreateObject<LteHelper>();
  Ptr<PointToPointEpcHelper> epcHelper = CreateObject<PointToPointEpcHelper>();
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
  p2ph.SetChannelAttribute("Delay", TimeValue(Seconds(0.001)));
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

  lteHelper->SetAttribute(
      "PathlossModel", StringValue("ns3::FriisSpectrumPropagationLossModel"));

  NodeContainer enbNodes;
  NodeContainer ueNodes;
  enbNodes.Create(1);
  ueNodes.Create(m_nUser);

  MobilityHelper mobility;
  mobility.SetMobilityModel("ns3::ConstantPositionMobilityModel");
  mobility.Install(enbNodes);
  mobility.SetMobilityModel("ns3::ConstantPositionMobilityModel");
  mobility.Install(ueNodes);

  NetDeviceContainer enbDevs;
  NetDeviceContainer ueDevs;
  lteHelper->SetSchedulerType("ns3::FdTbfqFfMacScheduler");
  lteHelper->SetSchedulerAttribute("UlCqiFilter",
                                   EnumValue(FfMacScheduler::SRS_UL_CQI));
  enbDevs = lteHelper->InstallEnbDevice(enbNodes);
  ueDevs = lteHelper->InstallUeDevice(ueNodes);

  Ptr<LteEnbNetDevice> lteEnbDev = enbDevs.Get(0)->GetObject<LteEnbNetDevice>();
  Ptr<LteEnbPhy> enbPhy = lteEnbDev->GetPhy();
  enbPhy->SetAttribute("TxPower", DoubleValue(30.0));
  enbPhy->SetAttribute("NoiseFigure", DoubleValue(5.0));

  for (int i = 0; i < m_nUser; i++) {
    Ptr<ConstantPositionMobilityModel> mm =
        ueNodes.Get(i)->GetObject<ConstantPositionMobilityModel>();
    mm->SetPosition(Vector(m_dist.at(i), 0.0, 0.0));
    Ptr<LteUeNetDevice> lteUeDev = ueDevs.Get(i)->GetObject<LteUeNetDevice>();
    Ptr<LteUePhy> uePhy = lteUeDev->GetPhy();
    uePhy->SetAttribute("TxPower", DoubleValue(23.0));
    uePhy->SetAttribute("NoiseFigure", DoubleValue(9.0));
  }

  internet.Install(ueNodes);
  Ipv4InterfaceContainer ueIpIface;
  ueIpIface = epcHelper->AssignUeIpv4Address(NetDeviceContainer(ueDevs));

  for (uint32_t u = 0; u < ueNodes.GetN(); ++u) {
    Ptr<Node> ueNode = ueNodes.Get(u);
    Ptr<Ipv4StaticRouting> ueStaticRouting =
        ipv4RoutingHelper.GetStaticRouting(ueNode->GetObject<Ipv4>());
    ueStaticRouting->SetDefaultRoute(epcHelper->GetUeDefaultGatewayAddress(),
                                     1);
  }

  lteHelper->Attach(ueDevs, enbDevs.Get(0));

  uint16_t mbrDl = 0;
  for (uint32_t u = 0; u < ueNodes.GetN(); ++u) {
    mbrDl = mbrDl + m_packetSize.at(u);
  }
  mbrDl = mbrDl / ueNodes.GetN();

  for (uint32_t u = 0; u < ueNodes.GetN(); ++u) {
    Ptr<NetDevice> ueDevice = ueDevs.Get(u);
    GbrQosInformation qos;
    qos.gbrDl = (mbrDl + 32) * (1000 / m_interval) * 8;
    qos.gbrUl = 0;
    qos.mbrDl = qos.gbrDl;
    qos.mbrUl = 0;

    EpsBearer::Qci q = EpsBearer::GBR_CONV_VOICE;
    EpsBearer bearer(q, qos);
    lteHelper->ActivateDedicatedEpsBearer(ueDevice, bearer, EpcTft::Default());
  }

  uint16_t dlPort = 1234;
  uint16_t ulPort = 2000;
  PacketSinkHelper dlPacketSinkHelper(
      "ns3::UdpSocketFactory",
      InetSocketAddress(Ipv4Address::GetAny(), dlPort));
  ApplicationContainer clientApps;
  ApplicationContainer serverApps;

  for (uint32_t u = 0; u < ueNodes.GetN(); ++u) {
    ++ulPort;
    PacketSinkHelper ulPacketSinkHelper(
        "ns3::UdpSocketFactory",
        InetSocketAddress(Ipv4Address::GetAny(), ulPort));
    serverApps.Add(ulPacketSinkHelper.Install(remoteHost));
    serverApps.Add(dlPacketSinkHelper.Install(ueNodes.Get(u)));

    UdpClientHelper dlClient(ueIpIface.GetAddress(u), dlPort);
    dlClient.SetAttribute("Interval", TimeValue(MilliSeconds(m_interval)));
    dlClient.SetAttribute("MaxPackets", UintegerValue(1000000));
    dlClient.SetAttribute("PacketSize", UintegerValue(m_packetSize.at(u)));

    UdpClientHelper ulClient(remoteHostAddr, ulPort);
    ulClient.SetAttribute("Interval", TimeValue(MilliSeconds(m_interval)));
    ulClient.SetAttribute("MaxPackets", UintegerValue(1000000));
    ulClient.SetAttribute("PacketSize", UintegerValue(m_packetSize.at(u)));

    clientApps.Add(dlClient.Install(remoteHost));
    clientApps.Add(ulClient.Install(ueNodes.Get(u)));
  }

  serverApps.Start(Seconds(0.001));
  clientApps.Start(Seconds(0.001));

  double statsStartTime = 0.001;
  double statsDuration = 1.0;
  double tolerance = 0.1;
  Simulator::Stop(Seconds(statsStartTime + statsDuration - 0.0001));

  lteHelper->EnableRlcTraces();
  Ptr<RadioBearerStatsCalculator> rlcStats = lteHelper->GetRlcStats();
  rlcStats->SetAttribute("StartTime", TimeValue(Seconds(statsStartTime)));
  rlcStats->SetAttribute("EpochDuration", TimeValue(Seconds(statsDuration)));

  Simulator::Run();

  NS_LOG_INFO("DL - Test with " << m_nUser << " user(s)");
  std::vector<uint64_t> dlDataRxed;
  for (int i = 0; i < m_nUser; i++) {
    uint64_t imsi = ueDevs.Get(i)->GetObject<LteUeNetDevice>()->GetImsi();
    uint8_t lcId = 4;
    dlDataRxed.push_back(rlcStats->GetDlRxData(imsi, lcId));
    NS_LOG_INFO("\tUser " << i << " dist " << m_dist.at(i) << " imsi " << imsi
                          << " bytes rxed " << (double)dlDataRxed.at(i)
                          << "  thr "
                          << (double)dlDataRxed.at(i) / statsDuration << " ref "
                          << m_nUser);
  }

  for (int i = 0; i < m_nUser; i++) {
    NS_TEST_ASSERT_MSG_EQ_TOL(
        (double)dlDataRxed.at(i) / statsDuration, m_estThrFdTbfqDl.at(i),
        m_estThrFdTbfqDl.at(i) * tolerance, " Unfair Throughput!");
  }

  Simulator::Destroy();
}
