

#include "ns3/address.h"
#include "ns3/callback.h"
#include "ns3/double.h"
#include "ns3/error-model.h"
#include "ns3/mac48-address.h"
#include "ns3/node.h"
#include "ns3/packet.h"
#include "ns3/pointer.h"
#include "ns3/queue.h"
#include "ns3/rng-seed-manager.h"
#include "ns3/simple-channel.h"
#include "ns3/simple-net-device.h"
#include "ns3/simulator.h"
#include "ns3/string.h"
#include "ns3/test.h"

using namespace ns3;

static void SendPacket(int num, Ptr<NetDevice> device, Address &addr) {
  for (int i = 0; i < num; i++) {
    Ptr<Packet> pkt = Create<Packet>(1000);
    device->Send(pkt, addr, 0);
  }
}

static void BuildSimpleTopology(Ptr<Node> a, Ptr<Node> b,
                                Ptr<SimpleNetDevice> input,
                                Ptr<SimpleNetDevice> output,
                                Ptr<SimpleChannel> channel) {
  ObjectFactory queueFactory;
  queueFactory.SetTypeId("ns3::DropTailQueue<Packet>");
  queueFactory.Set("MaxSize", StringValue("100000p"));
  Ptr<Queue<Packet>> queueA = queueFactory.Create<Queue<Packet>>();
  Ptr<Queue<Packet>> queueB = queueFactory.Create<Queue<Packet>>();

  input->SetQueue(queueA);
  output->SetQueue(queueB);
  a->AddDevice(input);
  b->AddDevice(output);
  input->SetAddress(Mac48Address::Allocate());
  input->SetChannel(channel);
  input->SetNode(a);
  output->SetChannel(channel);
  output->SetNode(b);
  output->SetAddress(Mac48Address::Allocate());
}

class ErrorModelSimple : public TestCase {
public:
  ErrorModelSimple();
  ~ErrorModelSimple() override;

private:
  void DoRun() override;
  bool Receive(Ptr<NetDevice> nd, Ptr<const Packet> p, uint16_t protocol,
               const Address &addr);
  void DropEvent(Ptr<const Packet> p);

  uint32_t m_count;
  uint32_t m_drops;
};

ErrorModelSimple::ErrorModelSimple()
    : TestCase("ErrorModel and PhyRxDrop trace for SimpleNetDevice"),
      m_count(0), m_drops(0) {}

ErrorModelSimple::~ErrorModelSimple() {}

bool ErrorModelSimple::Receive(Ptr<NetDevice> nd, Ptr<const Packet> p,
                               uint16_t protocol, const Address &addr) {
  m_count++;
  return true;
}

void ErrorModelSimple::DropEvent(Ptr<const Packet> p) { m_drops++; }

void ErrorModelSimple::DoRun() {
  RngSeedManager::SetSeed(7);
  RngSeedManager::SetRun(2);

  Ptr<Node> a = CreateObject<Node>();
  Ptr<Node> b = CreateObject<Node>();

  Ptr<SimpleNetDevice> input = CreateObject<SimpleNetDevice>();
  Ptr<SimpleNetDevice> output = CreateObject<SimpleNetDevice>();
  Ptr<SimpleChannel> channel = CreateObject<SimpleChannel>();
  BuildSimpleTopology(a, b, input, output, channel);

  output->SetReceiveCallback(MakeCallback(&ErrorModelSimple::Receive, this));
  Ptr<UniformRandomVariable> uv = CreateObject<UniformRandomVariable>();
  uv->SetStream(50);

  Ptr<RateErrorModel> em = CreateObject<RateErrorModel>();
  em->SetRandomVariable(uv);
  em->SetAttribute("ErrorRate", DoubleValue(0.001));
  em->SetAttribute("ErrorUnit", StringValue("ERROR_UNIT_PACKET"));

  output->SetAttribute("ReceiveErrorModel", PointerValue(em));
  output->TraceConnectWithoutContext(
      "PhyRxDrop", MakeCallback(&ErrorModelSimple::DropEvent, this));

  Simulator::Schedule(Seconds(0), &SendPacket, 10000, input,
                      output->GetAddress());

  Simulator::Run();
  Simulator::Destroy();

  NS_TEST_ASSERT_MSG_EQ(m_count, 9991, "Wrong number of receptions.");
  NS_TEST_ASSERT_MSG_EQ(m_drops, 9, "Wrong number of drops.");
}

class BurstErrorModelSimple : public TestCase {
public:
  BurstErrorModelSimple();
  ~BurstErrorModelSimple() override;

private:
  void DoRun() override;
  bool Receive(Ptr<NetDevice> nd, Ptr<const Packet> p, uint16_t protocol,
               const Address &addr);
  void DropEvent(Ptr<const Packet> p);

  uint32_t m_count;
  uint32_t m_drops;
};

BurstErrorModelSimple::BurstErrorModelSimple()
    : TestCase("ErrorModel and PhyRxDrop trace for SimpleNetDevice"),
      m_count(0), m_drops(0) {}

BurstErrorModelSimple::~BurstErrorModelSimple() {}

bool BurstErrorModelSimple::Receive(Ptr<NetDevice> nd, Ptr<const Packet> p,
                                    uint16_t protocol, const Address &addr) {
  m_count++;
  return true;
}

void BurstErrorModelSimple::DropEvent(Ptr<const Packet> p) { m_drops++; }

void BurstErrorModelSimple::DoRun() {
  RngSeedManager::SetSeed(5);
  RngSeedManager::SetRun(8);

  Ptr<Node> a = CreateObject<Node>();
  Ptr<Node> b = CreateObject<Node>();

  Ptr<SimpleNetDevice> input = CreateObject<SimpleNetDevice>();
  Ptr<SimpleNetDevice> output = CreateObject<SimpleNetDevice>();
  Ptr<SimpleChannel> channel = CreateObject<SimpleChannel>();
  BuildSimpleTopology(a, b, input, output, channel);

  output->SetReceiveCallback(
      MakeCallback(&BurstErrorModelSimple::Receive, this));
  Ptr<UniformRandomVariable> uv = CreateObject<UniformRandomVariable>();
  uv->SetStream(50);

  Ptr<BurstErrorModel> em = CreateObject<BurstErrorModel>();
  em->SetRandomVariable(uv);
  em->SetAttribute("ErrorRate", DoubleValue(0.01));

  em->AssignStreams(51);

  output->SetAttribute("ReceiveErrorModel", PointerValue(em));
  output->TraceConnectWithoutContext(
      "PhyRxDrop", MakeCallback(&BurstErrorModelSimple::DropEvent, this));

  Simulator::Schedule(Seconds(0), &SendPacket, 10000, input,
                      output->GetAddress());

  Simulator::Run();
  Simulator::Destroy();

  NS_TEST_ASSERT_MSG_EQ(m_count, 9740, "Wrong number of receptions.");
  NS_TEST_ASSERT_MSG_EQ(m_drops, 260, "Wrong number of drops.");
}

class ErrorModelTestSuite : public TestSuite {
public:
  ErrorModelTestSuite();
};

ErrorModelTestSuite::ErrorModelTestSuite() : TestSuite("error-model", UNIT) {
  AddTestCase(new ErrorModelSimple, TestCase::QUICK);
  AddTestCase(new BurstErrorModelSimple, TestCase::QUICK);
}

static ErrorModelTestSuite errorModelTestSuite;
