
#include "aodv-regression.h"

#include "bug-772.h"

#include "ns3/abort.h"
#include "ns3/aodv-helper.h"
#include "ns3/boolean.h"
#include "ns3/config.h"
#include "ns3/double.h"
#include "ns3/icmpv4.h"
#include "ns3/internet-stack-helper.h"
#include "ns3/ipv4-address-helper.h"
#include "ns3/mobility-helper.h"
#include "ns3/mobility-model.h"
#include "ns3/pcap-file.h"
#include "ns3/pcap-test.h"
#include "ns3/rng-seed-manager.h"
#include "ns3/simulator.h"
#include "ns3/string.h"
#include "ns3/uinteger.h"
#include "ns3/yans-wifi-helper.h"

#include <sstream>

using namespace ns3;

class AodvRegressionTestSuite : public TestSuite {
public:
  AodvRegressionTestSuite() : TestSuite("routing-aodv-regression", SYSTEM) {
    SetDataDir(NS_TEST_SOURCEDIR);
    AddTestCase(new ChainRegressionTest("aodv-chain-regression-test"),
                TestCase::QUICK);
    AddTestCase(
        new ChainRegressionTest("bug-606-test", Seconds(10), 3, Seconds(1)),
        TestCase::QUICK);
    AddTestCase(new Bug772ChainTest("udp-chain-test", "ns3::UdpSocketFactory",
                                    Seconds(3), 10),
                TestCase::QUICK);
  }
} g_aodvRegressionTestSuite;

ChainRegressionTest::ChainRegressionTest(const char *const prefix, Time t,
                                         uint32_t size, Time arpAliveTimeout)
    : TestCase("AODV chain regression test"), m_nodes(nullptr),
      m_prefix(prefix), m_time(t), m_size(size), m_step(120),
      m_arpAliveTimeout(arpAliveTimeout), m_seq(0) {}

ChainRegressionTest::~ChainRegressionTest() { delete m_nodes; }

void ChainRegressionTest::SendPing() {
  if (Simulator::Now() >= m_time) {
    return;
  }

  Ptr<Packet> p = Create<Packet>();
  Icmpv4Echo echo;
  echo.SetSequenceNumber(m_seq);
  m_seq++;
  echo.SetIdentifier(0);

  Ptr<Packet> dataPacket = Create<Packet>(56);
  echo.SetData(dataPacket);
  p->AddHeader(echo);
  Icmpv4Header header;
  header.SetType(Icmpv4Header::ICMPV4_ECHO);
  header.SetCode(0);
  if (Node::ChecksumEnabled()) {
    header.EnableChecksum();
  }
  p->AddHeader(header);
  m_socket->Send(p, 0);
  Simulator::Schedule(Seconds(1), &ChainRegressionTest::SendPing, this);
}

void ChainRegressionTest::DoRun() {
  RngSeedManager::SetSeed(12345);
  RngSeedManager::SetRun(7);
  Config::SetDefault("ns3::ArpCache::AliveTimeout",
                     TimeValue(m_arpAliveTimeout));

  CreateNodes();
  CreateDevices();

  Ptr<Node> node = m_nodes->Get(m_size / 2);
  Ptr<MobilityModel> mob = node->GetObject<MobilityModel>();
  Simulator::Schedule(Time(m_time / 3), &MobilityModel::SetPosition, mob,
                      Vector(1e5, 1e5, 1e5));

  Simulator::Stop(m_time);
  Simulator::Run();
  Simulator::Destroy();

  CheckResults();

  delete m_nodes, m_nodes = nullptr;
}

void ChainRegressionTest::CreateNodes() {
  m_nodes = new NodeContainer;
  m_nodes->Create(m_size);
  MobilityHelper mobility;
  mobility.SetPositionAllocator(
      "ns3::GridPositionAllocator", "MinX", DoubleValue(0.0), "MinY",
      DoubleValue(0.0), "DeltaX", DoubleValue(m_step), "DeltaY", DoubleValue(0),
      "GridWidth", UintegerValue(m_size), "LayoutType",
      StringValue("RowFirst"));
  mobility.SetMobilityModel("ns3::ConstantPositionMobilityModel");
  mobility.Install(*m_nodes);
}

void ChainRegressionTest::CreateDevices() {
  int64_t streamsUsed = 0;
  WifiMacHelper wifiMac;
  wifiMac.SetType("ns3::AdhocWifiMac");
  YansWifiPhyHelper wifiPhy;
  wifiPhy.DisablePreambleDetectionModel();
  YansWifiChannelHelper wifiChannel = YansWifiChannelHelper::Default();
  Ptr<YansWifiChannel> chan = wifiChannel.Create();
  wifiPhy.SetChannel(chan);

  wifiPhy.SetErrorRateModel("ns3::YansErrorRateModel");
  WifiHelper wifi;
  wifi.SetStandard(WIFI_STANDARD_80211a);
  wifi.SetRemoteStationManager("ns3::ConstantRateWifiManager", "DataMode",
                               StringValue("OfdmRate6Mbps"), "RtsCtsThreshold",
                               StringValue("2200"));
  NetDeviceContainer devices = wifi.Install(wifiPhy, wifiMac, *m_nodes);

  streamsUsed += wifi.AssignStreams(devices, streamsUsed);
  NS_TEST_ASSERT_MSG_EQ(streamsUsed, (devices.GetN() * 2),
                        "Stream assignment mismatch");
  streamsUsed += wifiChannel.AssignStreams(chan, streamsUsed);
  NS_TEST_ASSERT_MSG_EQ(streamsUsed, (devices.GetN() * 2),
                        "Stream assignment mismatch");

  AodvHelper aodv;
  InternetStackHelper internetStack;
  internetStack.SetRoutingHelper(aodv);
  internetStack.Install(*m_nodes);
  streamsUsed += internetStack.AssignStreams(*m_nodes, streamsUsed);
  NS_TEST_ASSERT_MSG_EQ(streamsUsed, (devices.GetN() * 5) + m_size,
                        "Stream assignment mismatch");
  streamsUsed += aodv.AssignStreams(*m_nodes, streamsUsed);
  NS_TEST_ASSERT_MSG_EQ(streamsUsed, ((devices.GetN() * 5) + (2 * m_size)),
                        "Stream assignment mismatch");

  Ipv4AddressHelper address;
  address.SetBase("10.1.1.0", "255.255.255.0");
  Ipv4InterfaceContainer interfaces = address.Assign(devices);

  m_socket = Socket::CreateSocket(
      m_nodes->Get(0), TypeId::LookupByName("ns3::Ipv4RawSocketFactory"));
  m_socket->SetAttribute("Protocol", UintegerValue(1));
  InetSocketAddress src = InetSocketAddress(Ipv4Address::GetAny(), 0);
  m_socket->Bind(src);
  InetSocketAddress dst =
      InetSocketAddress(interfaces.GetAddress(m_size - 1), 0);
  m_socket->Connect(dst);

  SendPing();

  wifiPhy.EnablePcapAll(CreateTempDirFilename(m_prefix));
}

void ChainRegressionTest::CheckResults() {
  for (uint32_t i = 0; i < m_size; ++i) {
    NS_PCAP_TEST_EXPECT_EQ(m_prefix << "-" << i << "-0.pcap");
  }
}
