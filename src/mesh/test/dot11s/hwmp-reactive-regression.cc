
#include "hwmp-reactive-regression.h"

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

const char *const PREFIX = "hwmp-reactive-regression-test";

HwmpReactiveRegressionTest::HwmpReactiveRegressionTest()
    : TestCase("HWMP on-demand regression test"), m_nodes(nullptr),
      m_time(Seconds(10)), m_sentPktsCounter(0) {}

HwmpReactiveRegressionTest::~HwmpReactiveRegressionTest() { delete m_nodes; }

void HwmpReactiveRegressionTest::DoRun() {
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

void HwmpReactiveRegressionTest::CreateNodes() {
  m_nodes = new NodeContainer;
  m_nodes->Create(6);
  MobilityHelper mobility;
  Ptr<ListPositionAllocator> positionAlloc =
      CreateObject<ListPositionAllocator>();
  positionAlloc->Add(Vector(0, 0, 0));
  positionAlloc->Add(Vector(0, 120, 0));
  positionAlloc->Add(Vector(0, 240, 0));
  positionAlloc->Add(Vector(0, 360, 0));
  positionAlloc->Add(Vector(0, 480, 0));
  positionAlloc->Add(Vector(0, 600, 0));
  mobility.SetPositionAllocator(positionAlloc);
  mobility.SetMobilityModel("ns3::ConstantPositionMobilityModel");
  mobility.Install(*m_nodes);
  Simulator::Schedule(Seconds(5.0), &HwmpReactiveRegressionTest::ResetPosition,
                      this);
}

void HwmpReactiveRegressionTest::InstallApplications() {
  m_clientSocket = Socket::CreateSocket(
      m_nodes->Get(5), TypeId::LookupByName("ns3::UdpSocketFactory"));
  m_clientSocket->Bind();
  m_clientSocket->Connect(InetSocketAddress(m_interfaces.GetAddress(0), 9));
  m_clientSocket->SetRecvCallback(
      MakeCallback(&HwmpReactiveRegressionTest::HandleReadClient, this));
  Simulator::ScheduleWithContext(
      m_clientSocket->GetNode()->GetId(), Seconds(2.0),
      &HwmpReactiveRegressionTest::SendData, this, m_clientSocket);

  m_serverSocket = Socket::CreateSocket(
      m_nodes->Get(0), TypeId::LookupByName("ns3::UdpSocketFactory"));
  m_serverSocket->Bind(InetSocketAddress(Ipv4Address::GetAny(), 9));
  m_serverSocket->SetRecvCallback(
      MakeCallback(&HwmpReactiveRegressionTest::HandleReadServer, this));
}

void HwmpReactiveRegressionTest::CreateDevices() {
  int64_t streamsUsed = 0;
  YansWifiPhyHelper wifiPhy;
  wifiPhy.SetErrorRateModel("ns3::YansErrorRateModel");
  YansWifiChannelHelper wifiChannel = YansWifiChannelHelper::Default();
  Ptr<YansWifiChannel> chan = wifiChannel.Create();
  wifiPhy.SetChannel(chan);
  wifiPhy.DisablePreambleDetectionModel();

  MeshHelper mesh = MeshHelper::Default();
  mesh.SetStackInstaller("ns3::Dot11sStack");
  mesh.SetMacType("RandomStart", TimeValue(Seconds(0.1)));
  mesh.SetNumberOfInterfaces(1);
  NetDeviceContainer meshDevices = mesh.Install(wifiPhy, *m_nodes);
  streamsUsed += mesh.AssignStreams(meshDevices, streamsUsed);
  NS_TEST_EXPECT_MSG_EQ(streamsUsed, (meshDevices.GetN() * 10),
                        "Stream assignment mismatch");
  streamsUsed += wifiChannel.AssignStreams(chan, streamsUsed);
  NS_TEST_EXPECT_MSG_EQ(streamsUsed, (meshDevices.GetN() * 10),
                        "Stream assignment mismatch");

  InternetStackHelper internetStack;
  internetStack.Install(*m_nodes);
  streamsUsed += internetStack.AssignStreams(*m_nodes, streamsUsed);
  Ipv4AddressHelper address;
  address.SetBase("10.1.1.0", "255.255.255.0");
  m_interfaces = address.Assign(meshDevices);
  wifiPhy.EnablePcapAll(CreateTempDirFilename(PREFIX));
}

void HwmpReactiveRegressionTest::CheckResults() {
  for (int i = 0; i < 6; ++i) {
    NS_PCAP_TEST_EXPECT_EQ(PREFIX << "-" << i << "-1.pcap");
  }
}

void HwmpReactiveRegressionTest::ResetPosition() {
  Ptr<Object> object = m_nodes->Get(3);
  Ptr<MobilityModel> model = object->GetObject<MobilityModel>();
  if (!model) {
    return;
  }
  model->SetPosition(Vector(9000, 0, 0));
}

void HwmpReactiveRegressionTest::SendData(Ptr<Socket> socket) {
  if ((Simulator::Now() < m_time) && (m_sentPktsCounter < 300)) {
    socket->Send(Create<Packet>(20));
    m_sentPktsCounter++;
    Simulator::ScheduleWithContext(socket->GetNode()->GetId(), Seconds(0.5),
                                   &HwmpReactiveRegressionTest::SendData, this,
                                   socket);
  }
}

void HwmpReactiveRegressionTest::HandleReadServer(Ptr<Socket> socket) {
  Ptr<Packet> packet;
  Address from;
  while ((packet = socket->RecvFrom(from))) {
    packet->RemoveAllPacketTags();
    packet->RemoveAllByteTags();

    socket->SendTo(packet, 0, from);
  }
}

void HwmpReactiveRegressionTest::HandleReadClient(Ptr<Socket> socket) {
  Ptr<Packet> packet;
  Address from;
  while ((packet = socket->RecvFrom(from))) {
  }
}
