
#include "lte-test-deactivate-bearer.h"

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
#include <ns3/rng-seed-manager.h>
#include <ns3/simulator.h>
#include <ns3/spectrum-error-model.h>
#include <ns3/spectrum-interference.h>
#include <ns3/test.h>

#include <iostream>
#include <sstream>
#include <string>

NS_LOG_COMPONENT_DEFINE("LenaTestDeactivateBearer");

namespace ns3 {

LenaTestBearerDeactivateSuite::LenaTestBearerDeactivateSuite()
    : TestSuite("lte-test-deactivate-bearer", SYSTEM) {
  NS_LOG_INFO("creating LenaTestPssFfMacSchedulerSuite");

  bool errorModel = false;

  std::vector<uint16_t> dist_1;

  dist_1.push_back(0);
  dist_1.push_back(0);
  dist_1.push_back(0);

  std::vector<uint16_t> packetSize_1;

  packetSize_1.push_back(100);
  packetSize_1.push_back(100);
  packetSize_1.push_back(100);

  std::vector<uint32_t> estThrPssDl_1;

  estThrPssDl_1.push_back(132000);
  estThrPssDl_1.push_back(132000);
  estThrPssDl_1.push_back(132000);

  AddTestCase(new LenaDeactivateBearerTestCase(
                  dist_1, estThrPssDl_1, packetSize_1, 1, errorModel, true),
              TestCase::QUICK);
}

static LenaTestBearerDeactivateSuite lenaTestBearerDeactivateSuite;

std::string
LenaDeactivateBearerTestCase::BuildNameString(uint16_t nUser,
                                              std::vector<uint16_t> dist) {
  std::ostringstream oss;
  oss << "distances (m) = [ ";
  for (auto it = dist.begin(); it != dist.end(); ++it) {
    oss << *it << " ";
  }
  oss << "]";
  return oss.str();
}

LenaDeactivateBearerTestCase::LenaDeactivateBearerTestCase(
    std::vector<uint16_t> dist, std::vector<uint32_t> estThrPssDl,
    std::vector<uint16_t> packetSize, uint16_t interval, bool errorModelEnabled,
    bool useIdealRrc)
    : TestCase(BuildNameString(dist.size(), dist)), m_nUser(dist.size()),
      m_dist(dist), m_packetSize(packetSize), m_interval(interval),
      m_estThrPssDl(estThrPssDl), m_errorModelEnabled(errorModelEnabled) {}

LenaDeactivateBearerTestCase::~LenaDeactivateBearerTestCase() {}

void LenaDeactivateBearerTestCase::DoRun() {
  uint32_t originalSeed = RngSeedManager::GetSeed();
  uint32_t originalRun = RngSeedManager::GetRun();
  RngSeedManager::SetSeed(1);
  RngSeedManager::SetRun(1);

  if (!m_errorModelEnabled) {
    Config::SetDefault("ns3::LteSpectrumPhy::CtrlErrorModelEnabled",
                       BooleanValue(false));
    Config::SetDefault("ns3::LteSpectrumPhy::DataErrorModelEnabled",
                       BooleanValue(false));
  }

  Config::SetDefault("ns3::LteHelper::UseIdealRrc", BooleanValue(true));
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
  int64_t stream = 1;

  lteHelper->SetSchedulerType("ns3::PssFfMacScheduler");
  enbDevs = lteHelper->InstallEnbDevice(enbNodes);
  stream += lteHelper->AssignStreams(enbDevs, stream);

  ueDevs = lteHelper->InstallUeDevice(ueNodes);
  stream += lteHelper->AssignStreams(ueDevs, stream);

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

  for (uint32_t u = 0; u < ueNodes.GetN(); ++u) {
    Ptr<NetDevice> ueDevice = ueDevs.Get(u);
    GbrQosInformation qos;
    qos.gbrDl = (m_packetSize.at(u) + 32) * (1000 / m_interval) * 8;
    qos.gbrUl = (m_packetSize.at(u) + 32) * (1000 / m_interval) * 8;
    qos.mbrDl = qos.gbrDl;
    qos.mbrUl = qos.gbrUl;

    EpsBearer::Qci q = EpsBearer::GBR_CONV_VOICE;
    EpsBearer bearer(q, qos);
    bearer.arp.priorityLevel = 15 - (u + 1);
    bearer.arp.preemptionCapability = true;
    bearer.arp.preemptionVulnerability = true;
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
    serverApps.Add(dlPacketSinkHelper.Install(ueNodes.Get(u)));
    PacketSinkHelper ulPacketSinkHelper(
        "ns3::UdpSocketFactory",
        InetSocketAddress(Ipv4Address::GetAny(), ulPort));
    serverApps.Add(ulPacketSinkHelper.Install(remoteHost));

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

  serverApps.Start(Seconds(0.030));
  clientApps.Start(Seconds(0.030));

  double statsStartTime = 0.04;
  double statsDuration = 1.0;
  double tolerance = 0.1;

  lteHelper->EnableRlcTraces();
  Ptr<RadioBearerStatsCalculator> rlcStats = lteHelper->GetRlcStats();
  rlcStats->SetAttribute("StartTime", TimeValue(Seconds(statsStartTime)));
  rlcStats->SetAttribute("EpochDuration", TimeValue(Seconds(statsDuration)));

  Ptr<NetDevice> ueDevice = ueDevs.Get(0);
  Ptr<NetDevice> enbDevice = enbDevs.Get(0);

  Time deActivateTime(Seconds(1.5));
  Simulator::Schedule(deActivateTime, &LteHelper::DeActivateDedicatedEpsBearer,
                      lteHelper, ueDevice, enbDevice, 2);

  Simulator::Stop(Seconds(3.0));

  Simulator::Run();

  NS_LOG_INFO("DL - Test with " << m_nUser << " user(s)");
  std::vector<uint64_t> dlDataRxed;
  std::vector<uint64_t> dlDataTxed;
  for (int i = 0; i < m_nUser; i++) {
    uint64_t imsi = ueDevs.Get(i)->GetObject<LteUeNetDevice>()->GetImsi();
    uint8_t lcId = 4;
    dlDataRxed.push_back(rlcStats->GetDlRxData(imsi, lcId));
    dlDataTxed.push_back(rlcStats->GetDlTxData(imsi, lcId));
    NS_LOG_INFO("\tUser " << i << " dist " << m_dist.at(i) << " imsi " << imsi
                          << " bytes rxed " << (double)dlDataRxed.at(i)
                          << "  thr "
                          << (double)dlDataRxed.at(i) / statsDuration << " ref "
                          << m_estThrPssDl.at(i));
    NS_LOG_INFO("\tUser " << i << " imsi " << imsi << " bytes txed "
                          << (double)dlDataTxed.at(i) << "  thr "
                          << (double)dlDataTxed.at(i) / statsDuration);
  }

  for (int i = 0; i < m_nUser; i++) {
    uint64_t imsi = ueDevs.Get(i)->GetObject<LteUeNetDevice>()->GetImsi();

    if (imsi == 1) {
      NS_TEST_ASSERT_MSG_EQ((double)dlDataTxed.at(i), 0,
                            "Invalid LCID in Statistics ");
    } else {
      NS_TEST_ASSERT_MSG_EQ_TOL(
          (double)dlDataTxed.at(i) / statsDuration, m_estThrPssDl.at(i),
          m_estThrPssDl.at(i) * tolerance, " Unfair Throughput!");
    }
  }

  Simulator::Destroy();

  RngSeedManager::SetSeed(originalSeed);
  RngSeedManager::SetRun(originalRun);
}
} // namespace ns3
