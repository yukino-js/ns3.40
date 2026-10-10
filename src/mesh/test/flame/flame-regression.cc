#include "flame-regression.h"

#include "ns3/abort.h"
#include "ns3/double.h"
#include "ns3/internet-stack-helper.h"
#include "ns3/ipv4-address-helper.h"
#include "ns3/mesh-helper.h"
#include "ns3/mobility-helper.h"
#include "ns3/mobility-model.h"
#include "ns3/pcap-test.h"
#include "ns3/random-variable-stream.h"
#include "ns3/rng-seed-manager.h"
#include "ns3/simulator.h"
#include "ns3/string.h"
#include "ns3/uinteger.h"
#include "ns3/yans-wifi-helper.h"

#include <sstream>

using namespace ns3;

const char *const PREFIX = "flame-regression-test";

FlameRegressionTest::FlameRegressionTest()
    : TestCase("FLAME regression test"), m_nodes(nullptr), m_time(Seconds(10)),
      m_sentPktsCounter(0) {}

FlameRegressionTest::~FlameRegressionTest() { delete m_nodes; }

void FlameRegressionTest::DoRun() {
  RngSeedManager::SetSeed(1);
  RngSeedManager::SetRun(1);
  CreateNodes();
  CreateDevices();
  InstallApplications();

  Simulator::Stop(m_time);
  Simulator::Run();
  Simulator::Destroy();

  CheckResults();

  delete m_nodes, m_nodes = nullptr;
}

void FlameRegressionTest::CreateNodes() {
  m_nodes = new NodeContainer;
  m_nodes->Create(3);
  MobilityHelper mobility;
  mobility.SetPositionAllocator(
      "ns3::GridPositionAllocator", "MinX", DoubleValue(0.0), "MinY",
      DoubleValue(0.0), "DeltaX", DoubleValue(120), "DeltaY", DoubleValue(0),
      "GridWidth", UintegerValue(3), "LayoutType", StringValue("RowFirst"));
  mobility.SetMobilityModel("ns3::ConstantPositionMobilityModel");
  mobility.Install(*m_nodes);
}

void FlameRegressionTest::CreateDevices() {
  int64_t streamsUsed = 0;
  YansWifiPhyHelper wifiPhy;
  wifiPhy.SetErrorRateModel("ns3::YansErrorRateModel");
  YansWifiChannelHelper wifiChannel = YansWifiChannelHelper::Default();
  Ptr<YansWifiChannel> chan = wifiChannel.Create();
  wifiPhy.SetChannel(chan);
  wifiPhy.DisablePreambleDetectionModel();

  MeshHelper mesh = MeshHelper::Default();
  mesh.SetStackInstaller("ns3::FlameStack");
  mesh.SetMacType("RandomStart", TimeValue(Seconds(0.1)));
  mesh.SetNumberOfInterfaces(1);
  NetDeviceContainer meshDevices = mesh.Install(wifiPhy, *m_nodes);
  streamsUsed += mesh.AssignStreams(meshDevices, streamsUsed);
  NS_TEST_ASSERT_MSG_EQ(streamsUsed, (meshDevices.GetN() * 8),
                        "Stream assignment unexpected value");
  streamsUsed += wifiChannel.AssignStreams(chan, streamsUsed);
  NS_TEST_ASSERT_MSG_EQ(streamsUsed, (meshDevices.GetN() * 8),
                        "Stream assignment unexpected value");
  InternetStackHelper internetStack;
  internetStack.Install(*m_nodes);
  streamsUsed += internetStack.AssignStreams(*m_nodes, streamsUsed);
  Ipv4AddressHelper address;
  address.SetBase("10.1.1.0", "255.255.255.0");
  m_interfaces = address.Assign(meshDevices);
  wifiPhy.EnablePcapAll(CreateTempDirFilename(PREFIX));
}

void FlameRegressionTest::InstallApplications() {
  m_clientSocket = Socket::CreateSocket(
      m_nodes->Get(2), TypeId::LookupByName("ns3::UdpSocketFactory"));
  m_clientSocket->Bind();
  m_clientSocket->Connect(InetSocketAddress(m_interfaces.GetAddress(0), 9));
  m_clientSocket->SetRecvCallback(
      MakeCallback(&FlameRegressionTest::HandleReadClient, this));
  Simulator::ScheduleWithContext(m_clientSocket->GetNode()->GetId(),
                                 Seconds(1.0), &FlameRegressionTest::SendData,
                                 this, m_clientSocket);

  m_serverSocket = Socket::CreateSocket(
      m_nodes->Get(0), TypeId::LookupByName("ns3::UdpSocketFactory"));
  m_serverSocket->Bind(InetSocketAddress(Ipv4Address::GetAny(), 9));
  m_serverSocket->SetRecvCallback(
      MakeCallback(&FlameRegressionTest::HandleReadServer, this));
}

void FlameRegressionTest::CheckResults() {
  for (int i = 0; i < 3; ++i) {
    NS_PCAP_TEST_EXPECT_EQ(PREFIX << "-" << i << "-1.pcap");
  }
}

void FlameRegressionTest::SendData(Ptr<Socket> socket) {
  if ((Simulator::Now() < m_time) && (m_sentPktsCounter < 300)) {
    socket->Send(Create<Packet>(20));
    m_sentPktsCounter++;
    Simulator::ScheduleWithContext(socket->GetNode()->GetId(), Seconds(1.1),
                                   &FlameRegressionTest::SendData, this,
                                   socket);
  }
}

void FlameRegressionTest::HandleReadServer(Ptr<Socket> socket) {
  Ptr<Packet> packet;
  Address from;
  while ((packet = socket->RecvFrom(from))) {
    packet->RemoveAllPacketTags();
    packet->RemoveAllByteTags();

    socket->SendTo(packet, 0, from);
  }
}

void FlameRegressionTest::HandleReadClient(Ptr<Socket> socket) {
  Ptr<Packet> packet;
  Address from;
  while ((packet = socket->RecvFrom(from))) {
  }
}
