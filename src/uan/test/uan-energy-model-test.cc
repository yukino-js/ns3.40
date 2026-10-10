
#include "ns3/acoustic-modem-energy-model-helper.h"
#include "ns3/acoustic-modem-energy-model.h"
#include "ns3/basic-energy-source-helper.h"
#include "ns3/constant-position-mobility-model.h"
#include "ns3/log.h"
#include "ns3/node.h"
#include "ns3/packet.h"
#include "ns3/simple-device-energy-model.h"
#include "ns3/simulator.h"
#include "ns3/test.h"
#include "ns3/uan-channel.h"
#include "ns3/uan-header-common.h"
#include "ns3/uan-helper.h"
#include "ns3/uan-net-device.h"
#include "ns3/uan-noise-model-default.h"
#include "ns3/uan-phy.h"
#include "ns3/uan-prop-model-ideal.h"

using namespace ns3;

NS_LOG_COMPONENT_DEFINE("UanEnergyModelTestSuite");

class AcousticModemEnergyTestCase : public TestCase {
public:
  AcousticModemEnergyTestCase();
  ~AcousticModemEnergyTestCase() override;

  bool RxPacket(Ptr<NetDevice> dev, Ptr<const Packet> pkt, uint16_t mode,
                const Address &sender);
  void SendOnePacket(Ptr<Node> node);

  void DoRun() override;

  double m_simTime;
  uint32_t m_bytesRx;
  uint32_t m_sentPackets;
  uint32_t m_packetSize;
  Ptr<Node> m_node;
  Ptr<Node> m_gateway;
};

AcousticModemEnergyTestCase::AcousticModemEnergyTestCase()
    : TestCase("Acoustic Modem energy model test case"), m_simTime(25),
      m_bytesRx(0), m_sentPackets(0), m_packetSize(17) {}

AcousticModemEnergyTestCase::~AcousticModemEnergyTestCase() {
  m_node = nullptr;
  m_gateway = nullptr;
}

void AcousticModemEnergyTestCase::SendOnePacket(Ptr<Node> node) {
  Ptr<Packet> pkt = Create<Packet>(m_packetSize);
  Ptr<UanNetDevice> dev = node->GetDevice(0)->GetObject<UanNetDevice>();
  dev->Send(pkt, dev->GetBroadcast(), 0);
  ++m_sentPackets;

  Simulator::Schedule(Seconds(10), &AcousticModemEnergyTestCase::SendOnePacket,
                      this, node);
}

bool AcousticModemEnergyTestCase::RxPacket(Ptr<NetDevice>,
                                           Ptr<const Packet> pkt, uint16_t,
                                           const Address &) {
  m_bytesRx += pkt->GetSize();

  return true;
}

void AcousticModemEnergyTestCase::DoRun() {
  m_node = CreateObject<Node>();

  Ptr<UanChannel> channel = CreateObject<UanChannel>();
  Ptr<UanNoiseModelDefault> noise = CreateObject<UanNoiseModelDefault>();
  channel->SetPropagationModel(CreateObject<UanPropModelIdeal>());
  channel->SetNoiseModel(noise);

  UanHelper uan;
  Ptr<UanNetDevice> devNode = uan.Install(m_node, channel);

  uint32_t datarate = devNode->GetPhy()->GetMode(0).GetDataRateBps();
  UanHeaderCommon hd;
  double packetDuration =
      (m_packetSize + hd.GetSerializedSize()) * 8.0 / (double)datarate;

  BasicEnergySourceHelper eh;
  eh.Set("BasicEnergySourceInitialEnergyJ", DoubleValue(10000000.0));
  eh.Install(m_node);

  Ptr<ConstantPositionMobilityModel> mobility =
      CreateObject<ConstantPositionMobilityModel>();
  mobility->SetPosition(Vector(0, 0, -500));
  m_node->AggregateObject(mobility);

  AcousticModemEnergyModelHelper modemHelper;
  Ptr<EnergySource> source = m_node->GetObject<EnergySourceContainer>()->Get(0);
  DeviceEnergyModelContainer cont = modemHelper.Install(devNode, source);

  Simulator::ScheduleNow(&AcousticModemEnergyTestCase::SendOnePacket, this,
                         m_node);

  m_gateway = CreateObject<Node>();

  Ptr<UanNetDevice> devGateway = uan.Install(m_gateway, channel);

  eh.Set("BasicEnergySourceInitialEnergyJ", DoubleValue(10000000.0));
  eh.Install(m_gateway);

  Ptr<ConstantPositionMobilityModel> mobility2 =
      CreateObject<ConstantPositionMobilityModel>();
  mobility2->SetPosition(Vector(0, 0, 0));
  m_gateway->AggregateObject(mobility2);

  Ptr<EnergySource> source2 =
      m_gateway->GetObject<EnergySourceContainer>()->Get(0);
  DeviceEnergyModelContainer cont2 = modemHelper.Install(devGateway, source2);

  Ptr<NetDevice> dev = m_gateway->GetDevice(0);
  dev->SetReceiveCallback(
      MakeCallback(&AcousticModemEnergyTestCase::RxPacket, this));

  Simulator::Stop(Seconds(m_simTime));
  Simulator::Run();

  uint32_t receivedPackets = m_bytesRx / m_packetSize;
  Ptr<EnergySource> src1 =
      m_gateway->GetObject<EnergySourceContainer>()->Get(0);
  double consumed1 = src1->GetInitialEnergy() - src1->GetRemainingEnergy();
  double computed1 =
      cont2.Get(0)->GetObject<AcousticModemEnergyModel>()->GetRxPowerW() *
          packetDuration * receivedPackets +
      cont2.Get(0)->GetObject<AcousticModemEnergyModel>()->GetIdlePowerW() *
          (m_simTime - packetDuration * receivedPackets);

  NS_TEST_ASSERT_MSG_EQ_TOL(consumed1, computed1, 1.0e-5,
                            "Incorrect gateway consumed energy!");

  Ptr<EnergySource> src2 = m_node->GetObject<EnergySourceContainer>()->Get(0);
  double consumed2 = src2->GetInitialEnergy() - src2->GetRemainingEnergy();
  double computed2 =
      cont.Get(0)->GetObject<AcousticModemEnergyModel>()->GetTxPowerW() *
          packetDuration * m_sentPackets +
      cont.Get(0)->GetObject<AcousticModemEnergyModel>()->GetIdlePowerW() *
          (m_simTime - packetDuration * m_sentPackets);

  NS_TEST_ASSERT_MSG_EQ_TOL(consumed2, computed2, 1.0e-5,
                            "Incorrect node consumed energy!");

  Simulator::Destroy();
}

class AcousticModemEnergyDepletionTestCase : public TestCase {
public:
  AcousticModemEnergyDepletionTestCase();
  ~AcousticModemEnergyDepletionTestCase() override;

  void DepletionHandler();
  void SendOnePacket(Ptr<Node> node);

  void DoRun() override;

  double m_simTime;
  uint32_t m_callbackCount;
  uint32_t m_packetSize;
  Ptr<Node> m_node;
};

AcousticModemEnergyDepletionTestCase::AcousticModemEnergyDepletionTestCase()
    : TestCase("Acoustic Modem energy depletion test case"), m_simTime(25),
      m_callbackCount(0), m_packetSize(17) {}

AcousticModemEnergyDepletionTestCase::~AcousticModemEnergyDepletionTestCase() {
  m_node = nullptr;
}

void AcousticModemEnergyDepletionTestCase::DepletionHandler() {
  m_callbackCount++;
}

void AcousticModemEnergyDepletionTestCase::SendOnePacket(Ptr<Node> node) {
  Ptr<Packet> pkt = Create<Packet>(m_packetSize);
  Ptr<UanNetDevice> dev = node->GetDevice(0)->GetObject<UanNetDevice>();
  dev->Send(pkt, dev->GetBroadcast(), 0);

  Simulator::Schedule(Seconds(10),
                      &AcousticModemEnergyDepletionTestCase::SendOnePacket,
                      this, node);
}

void AcousticModemEnergyDepletionTestCase::DoRun() {
  m_node = CreateObject<Node>();

  Ptr<UanChannel> channel = CreateObject<UanChannel>();
  Ptr<UanNoiseModelDefault> noise = CreateObject<UanNoiseModelDefault>();
  channel->SetPropagationModel(CreateObject<UanPropModelIdeal>());
  channel->SetNoiseModel(noise);

  UanHelper uan;
  Ptr<UanNetDevice> devNode = uan.Install(m_node, channel);

  BasicEnergySourceHelper eh;
  eh.Set("BasicEnergySourceInitialEnergyJ", DoubleValue(0.0));
  eh.Install(m_node);

  Ptr<ConstantPositionMobilityModel> mobility =
      CreateObject<ConstantPositionMobilityModel>();
  mobility->SetPosition(Vector(0, 0, 0));
  m_node->AggregateObject(mobility);

  AcousticModemEnergyModelHelper modemHelper;
  Ptr<EnergySource> source = m_node->GetObject<EnergySourceContainer>()->Get(0);
  AcousticModemEnergyModel::AcousticModemEnergyDepletionCallback callback =
      MakeCallback(&AcousticModemEnergyDepletionTestCase::DepletionHandler,
                   this);
  modemHelper.SetDepletionCallback(callback);
  DeviceEnergyModelContainer cont = modemHelper.Install(devNode, source);

  Simulator::ScheduleNow(&AcousticModemEnergyDepletionTestCase::SendOnePacket,
                         this, m_node);

  Simulator::Stop(Seconds(m_simTime));
  Simulator::Run();
  Simulator::Destroy();

  NS_TEST_ASSERT_MSG_EQ(m_callbackCount, 1, "Callback not invoked");
}

class UanEnergyModelTestSuite : public TestSuite {
public:
  UanEnergyModelTestSuite();
};

UanEnergyModelTestSuite::UanEnergyModelTestSuite()
    : TestSuite("uan-energy-model", UNIT) {
  AddTestCase(new AcousticModemEnergyTestCase, TestCase::QUICK);
  AddTestCase(new AcousticModemEnergyDepletionTestCase, TestCase::QUICK);
}

static UanEnergyModelTestSuite g_uanEnergyModelTestSuite;
