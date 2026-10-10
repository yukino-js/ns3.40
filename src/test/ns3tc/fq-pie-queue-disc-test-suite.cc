
#include "ns3/fq-pie-queue-disc.h"
#include "ns3/ipv4-address.h"
#include "ns3/ipv4-header.h"
#include "ns3/ipv4-packet-filter.h"
#include "ns3/ipv4-queue-disc-item.h"
#include "ns3/ipv6-header.h"
#include "ns3/ipv6-packet-filter.h"
#include "ns3/ipv6-queue-disc-item.h"
#include "ns3/pie-queue-disc.h"
#include "ns3/pointer.h"
#include "ns3/simulator.h"
#include "ns3/string.h"
#include "ns3/tcp-header.h"
#include "ns3/test.h"
#include "ns3/udp-header.h"

using namespace ns3;

static int32_t g_hash;

class Ipv4FqPieTestPacketFilter : public Ipv4PacketFilter {
public:
  static TypeId GetTypeId();

  Ipv4FqPieTestPacketFilter();
  ~Ipv4FqPieTestPacketFilter() override;

private:
  int32_t DoClassify(Ptr<QueueDiscItem> item) const override;

  bool CheckProtocol(Ptr<QueueDiscItem> item) const override;
};

TypeId Ipv4FqPieTestPacketFilter::GetTypeId() {
  static TypeId tid = TypeId("ns3::Ipv4FqPieTestPacketFilter")
                          .SetParent<Ipv4PacketFilter>()
                          .SetGroupName("Internet")
                          .AddConstructor<Ipv4FqPieTestPacketFilter>();
  return tid;
}

Ipv4FqPieTestPacketFilter::Ipv4FqPieTestPacketFilter() {}

Ipv4FqPieTestPacketFilter::~Ipv4FqPieTestPacketFilter() {}

int32_t Ipv4FqPieTestPacketFilter::DoClassify(Ptr<QueueDiscItem> item) const {
  return g_hash;
}

bool Ipv4FqPieTestPacketFilter::CheckProtocol(Ptr<QueueDiscItem> item) const {
  return true;
}

class FqPieQueueDiscNoSuitableFilter : public TestCase {
public:
  FqPieQueueDiscNoSuitableFilter();
  ~FqPieQueueDiscNoSuitableFilter() override;

private:
  void DoRun() override;
};

FqPieQueueDiscNoSuitableFilter::FqPieQueueDiscNoSuitableFilter()
    : TestCase("Test packets that are not classified by any filter") {}

FqPieQueueDiscNoSuitableFilter::~FqPieQueueDiscNoSuitableFilter() {}

void FqPieQueueDiscNoSuitableFilter::DoRun() {
  Ptr<FqPieQueueDisc> queueDisc =
      CreateObjectWithAttributes<FqPieQueueDisc>("MaxSize", StringValue("4p"));
  Ptr<Ipv4FqPieTestPacketFilter> filter =
      CreateObject<Ipv4FqPieTestPacketFilter>();
  queueDisc->AddPacketFilter(filter);

  g_hash = -1;
  queueDisc->SetQuantum(1500);
  queueDisc->Initialize();

  Ptr<Packet> p;
  p = Create<Packet>();
  Ptr<Ipv6QueueDiscItem> item;
  Ipv6Header ipv6Header;
  Address dest;
  item = Create<Ipv6QueueDiscItem>(p, dest, 0, ipv6Header);
  queueDisc->Enqueue(item);
  NS_TEST_ASSERT_MSG_EQ(queueDisc->GetNQueueDiscClasses(), 0,
                        "no flow queue should have been created");

  p = Create<Packet>(reinterpret_cast<const uint8_t *>("hello, world"), 12);
  item = Create<Ipv6QueueDiscItem>(p, dest, 0, ipv6Header);
  queueDisc->Enqueue(item);
  NS_TEST_ASSERT_MSG_EQ(queueDisc->GetNQueueDiscClasses(), 0,
                        "no flow queue should have been created");

  Simulator::Destroy();
}

class FqPieQueueDiscIPFlowsSeparationAndPacketLimit : public TestCase {
public:
  FqPieQueueDiscIPFlowsSeparationAndPacketLimit();
  ~FqPieQueueDiscIPFlowsSeparationAndPacketLimit() override;

private:
  void DoRun() override;
  void AddPacket(Ptr<FqPieQueueDisc> queue, Ipv4Header hdr);
};

FqPieQueueDiscIPFlowsSeparationAndPacketLimit::
    FqPieQueueDiscIPFlowsSeparationAndPacketLimit()
    : TestCase("Test IP flows separation and packet limit") {}

FqPieQueueDiscIPFlowsSeparationAndPacketLimit::
    ~FqPieQueueDiscIPFlowsSeparationAndPacketLimit() {}

void FqPieQueueDiscIPFlowsSeparationAndPacketLimit::AddPacket(
    Ptr<FqPieQueueDisc> queue, Ipv4Header hdr) {
  Ptr<Packet> p = Create<Packet>(100);
  Address dest;
  Ptr<Ipv4QueueDiscItem> item = Create<Ipv4QueueDiscItem>(p, dest, 0, hdr);
  queue->Enqueue(item);
}

void FqPieQueueDiscIPFlowsSeparationAndPacketLimit::DoRun() {
  Ptr<FqPieQueueDisc> queueDisc =
      CreateObjectWithAttributes<FqPieQueueDisc>("MaxSize", StringValue("4p"));

  queueDisc->SetQuantum(1500);
  queueDisc->Initialize();

  Ipv4Header hdr;
  hdr.SetPayloadSize(100);
  hdr.SetSource(Ipv4Address("10.10.1.1"));
  hdr.SetDestination(Ipv4Address("10.10.1.2"));
  hdr.SetProtocol(7);

  AddPacket(queueDisc, hdr);
  AddPacket(queueDisc, hdr);
  AddPacket(queueDisc, hdr);
  NS_TEST_ASSERT_MSG_EQ(queueDisc->QueueDisc::GetNPackets(), 3,
                        "unexpected number of packets in the queue disc");
  NS_TEST_ASSERT_MSG_EQ(
      queueDisc->GetQueueDiscClass(0)->GetQueueDisc()->GetNPackets(), 3,
      "unexpected number of packets in the flow queue");

  hdr.SetDestination(Ipv4Address("10.10.1.7"));
  AddPacket(queueDisc, hdr);
  NS_TEST_ASSERT_MSG_EQ(queueDisc->QueueDisc::GetNPackets(), 4,
                        "unexpected number of packets in the queue disc");
  NS_TEST_ASSERT_MSG_EQ(
      queueDisc->GetQueueDiscClass(0)->GetQueueDisc()->GetNPackets(), 3,
      "unexpected number of packets in the flow queue");
  NS_TEST_ASSERT_MSG_EQ(
      queueDisc->GetQueueDiscClass(1)->GetQueueDisc()->GetNPackets(), 1,
      "unexpected number of packets in the flow queue");
  AddPacket(queueDisc, hdr);
  NS_TEST_ASSERT_MSG_EQ(queueDisc->QueueDisc::GetNPackets(), 3,
                        "unexpected number of packets in the queue disc");
  NS_TEST_ASSERT_MSG_EQ(
      queueDisc->GetQueueDiscClass(0)->GetQueueDisc()->GetNPackets(), 1,
      "unexpected number of packets in the flow queue");
  NS_TEST_ASSERT_MSG_EQ(
      queueDisc->GetQueueDiscClass(1)->GetQueueDisc()->GetNPackets(), 2,
      "unexpected number of packets in the flow queue");

  Simulator::Destroy();
}

class FqPieQueueDiscDeficit : public TestCase {
public:
  FqPieQueueDiscDeficit();
  ~FqPieQueueDiscDeficit() override;

private:
  void DoRun() override;
  void AddPacket(Ptr<FqPieQueueDisc> queue, Ipv4Header hdr);
};

FqPieQueueDiscDeficit::FqPieQueueDiscDeficit()
    : TestCase("Test credits and flows status") {}

FqPieQueueDiscDeficit::~FqPieQueueDiscDeficit() {}

void FqPieQueueDiscDeficit::AddPacket(Ptr<FqPieQueueDisc> queue,
                                      Ipv4Header hdr) {
  Ptr<Packet> p = Create<Packet>(100);
  Address dest;
  Ptr<Ipv4QueueDiscItem> item = Create<Ipv4QueueDiscItem>(p, dest, 0, hdr);
  queue->Enqueue(item);
}

void FqPieQueueDiscDeficit::DoRun() {
  Ptr<FqPieQueueDisc> queueDisc = CreateObject<FqPieQueueDisc>();

  queueDisc->SetQuantum(90);
  queueDisc->Initialize();

  Ipv4Header hdr;
  hdr.SetPayloadSize(100);
  hdr.SetSource(Ipv4Address("10.10.1.1"));
  hdr.SetDestination(Ipv4Address("10.10.1.2"));
  hdr.SetProtocol(7);

  AddPacket(queueDisc, hdr);
  NS_TEST_ASSERT_MSG_EQ(queueDisc->QueueDisc::GetNPackets(), 1,
                        "unexpected number of packets in the queue disc");
  NS_TEST_ASSERT_MSG_EQ(
      queueDisc->GetQueueDiscClass(0)->GetQueueDisc()->GetNPackets(), 1,
      "unexpected number of packets in the first flow queue");
  Ptr<FqPieFlow> flow1 = StaticCast<FqPieFlow>(queueDisc->GetQueueDiscClass(0));
  NS_TEST_ASSERT_MSG_EQ(flow1->GetDeficit(),
                        static_cast<int32_t>(queueDisc->GetQuantum()),
                        "the deficit of the first flow must equal the quantum");
  NS_TEST_ASSERT_MSG_EQ(flow1->GetStatus(), FqPieFlow::NEW_FLOW,
                        "the first flow must be in the list of new queues");
  queueDisc->Dequeue();
  NS_TEST_ASSERT_MSG_EQ(queueDisc->QueueDisc::GetNPackets(), 0,
                        "unexpected number of packets in the queue disc");
  NS_TEST_ASSERT_MSG_EQ(
      queueDisc->GetQueueDiscClass(0)->GetQueueDisc()->GetNPackets(), 0,
      "unexpected number of packets in the first flow queue");
  NS_TEST_ASSERT_MSG_EQ(flow1->GetDeficit(), -30,
                        "unexpected deficit for the first flow");

  AddPacket(queueDisc, hdr);
  AddPacket(queueDisc, hdr);
  NS_TEST_ASSERT_MSG_EQ(queueDisc->QueueDisc::GetNPackets(), 2,
                        "unexpected number of packets in the queue disc");
  NS_TEST_ASSERT_MSG_EQ(
      queueDisc->GetQueueDiscClass(0)->GetQueueDisc()->GetNPackets(), 2,
      "unexpected number of packets in the first flow queue");
  NS_TEST_ASSERT_MSG_EQ(
      flow1->GetStatus(), FqPieFlow::NEW_FLOW,
      "the first flow must still be in the list of new queues");

  hdr.SetDestination(Ipv4Address("10.10.1.10"));
  AddPacket(queueDisc, hdr);
  AddPacket(queueDisc, hdr);
  NS_TEST_ASSERT_MSG_EQ(queueDisc->QueueDisc::GetNPackets(), 4,
                        "unexpected number of packets in the queue disc");
  NS_TEST_ASSERT_MSG_EQ(
      queueDisc->GetQueueDiscClass(0)->GetQueueDisc()->GetNPackets(), 2,
      "unexpected number of packets in the first flow queue");
  NS_TEST_ASSERT_MSG_EQ(
      queueDisc->GetQueueDiscClass(1)->GetQueueDisc()->GetNPackets(), 2,
      "unexpected number of packets in the second flow queue");
  Ptr<FqPieFlow> flow2 = StaticCast<FqPieFlow>(queueDisc->GetQueueDiscClass(1));
  NS_TEST_ASSERT_MSG_EQ(
      flow2->GetDeficit(), static_cast<int32_t>(queueDisc->GetQuantum()),
      "the deficit of the second flow must equal the quantum");
  NS_TEST_ASSERT_MSG_EQ(flow2->GetStatus(), FqPieFlow::NEW_FLOW,
                        "the second flow must be in the list of new queues");

  queueDisc->Dequeue();
  NS_TEST_ASSERT_MSG_EQ(queueDisc->QueueDisc::GetNPackets(), 3,
                        "unexpected number of packets in the queue disc");
  NS_TEST_ASSERT_MSG_EQ(
      queueDisc->GetQueueDiscClass(0)->GetQueueDisc()->GetNPackets(), 2,
      "unexpected number of packets in the first flow queue");
  NS_TEST_ASSERT_MSG_EQ(
      queueDisc->GetQueueDiscClass(1)->GetQueueDisc()->GetNPackets(), 1,
      "unexpected number of packets in the second flow queue");
  NS_TEST_ASSERT_MSG_EQ(flow1->GetDeficit(), 60,
                        "unexpected deficit for the first flow");
  NS_TEST_ASSERT_MSG_EQ(flow1->GetStatus(), FqPieFlow::OLD_FLOW,
                        "the first flow must be in the list of old queues");
  NS_TEST_ASSERT_MSG_EQ(flow2->GetDeficit(), -30,
                        "unexpected deficit for the second flow");
  NS_TEST_ASSERT_MSG_EQ(flow2->GetStatus(), FqPieFlow::NEW_FLOW,
                        "the second flow must be in the list of new queues");

  queueDisc->Dequeue();
  NS_TEST_ASSERT_MSG_EQ(queueDisc->QueueDisc::GetNPackets(), 2,
                        "unexpected number of packets in the queue disc");
  NS_TEST_ASSERT_MSG_EQ(
      queueDisc->GetQueueDiscClass(0)->GetQueueDisc()->GetNPackets(), 1,
      "unexpected number of packets in the first flow queue");
  NS_TEST_ASSERT_MSG_EQ(
      queueDisc->GetQueueDiscClass(1)->GetQueueDisc()->GetNPackets(), 1,
      "unexpected number of packets in the second flow queue");
  NS_TEST_ASSERT_MSG_EQ(flow1->GetDeficit(), -60,
                        "unexpected deficit for the first flow");
  NS_TEST_ASSERT_MSG_EQ(flow1->GetStatus(), FqPieFlow::OLD_FLOW,
                        "the first flow must be in the list of old queues");
  NS_TEST_ASSERT_MSG_EQ(flow2->GetDeficit(), 60,
                        "unexpected deficit for the second flow");
  NS_TEST_ASSERT_MSG_EQ(flow2->GetStatus(), FqPieFlow::OLD_FLOW,
                        "the second flow must be in the list of new queues");

  queueDisc->Dequeue();
  NS_TEST_ASSERT_MSG_EQ(queueDisc->QueueDisc::GetNPackets(), 1,
                        "unexpected number of packets in the queue disc");
  NS_TEST_ASSERT_MSG_EQ(
      queueDisc->GetQueueDiscClass(0)->GetQueueDisc()->GetNPackets(), 1,
      "unexpected number of packets in the first flow queue");
  NS_TEST_ASSERT_MSG_EQ(
      queueDisc->GetQueueDiscClass(1)->GetQueueDisc()->GetNPackets(), 0,
      "unexpected number of packets in the second flow queue");
  NS_TEST_ASSERT_MSG_EQ(flow1->GetDeficit(), 30,
                        "unexpected deficit for the first flow");
  NS_TEST_ASSERT_MSG_EQ(flow1->GetStatus(), FqPieFlow::OLD_FLOW,
                        "the first flow must be in the list of old queues");
  NS_TEST_ASSERT_MSG_EQ(flow2->GetDeficit(), -60,
                        "unexpected deficit for the second flow");
  NS_TEST_ASSERT_MSG_EQ(flow2->GetStatus(), FqPieFlow::OLD_FLOW,
                        "the second flow must be in the list of new queues");

  queueDisc->Dequeue();
  NS_TEST_ASSERT_MSG_EQ(queueDisc->QueueDisc::GetNPackets(), 0,
                        "unexpected number of packets in the queue disc");
  NS_TEST_ASSERT_MSG_EQ(
      queueDisc->GetQueueDiscClass(0)->GetQueueDisc()->GetNPackets(), 0,
      "unexpected number of packets in the first flow queue");
  NS_TEST_ASSERT_MSG_EQ(
      queueDisc->GetQueueDiscClass(1)->GetQueueDisc()->GetNPackets(), 0,
      "unexpected number of packets in the second flow queue");
  NS_TEST_ASSERT_MSG_EQ(flow1->GetDeficit(), -90,
                        "unexpected deficit for the first flow");
  NS_TEST_ASSERT_MSG_EQ(flow1->GetStatus(), FqPieFlow::OLD_FLOW,
                        "the first flow must be in the list of old queues");
  NS_TEST_ASSERT_MSG_EQ(flow2->GetDeficit(), 30,
                        "unexpected deficit for the second flow");
  NS_TEST_ASSERT_MSG_EQ(flow2->GetStatus(), FqPieFlow::OLD_FLOW,
                        "the second flow must be in the list of new queues");

  queueDisc->Dequeue();
  NS_TEST_ASSERT_MSG_EQ(flow1->GetDeficit(), 90,
                        "unexpected deficit for the first flow");
  NS_TEST_ASSERT_MSG_EQ(flow1->GetStatus(), FqPieFlow::INACTIVE,
                        "the first flow must be inactive");
  NS_TEST_ASSERT_MSG_EQ(flow2->GetDeficit(), 30,
                        "unexpected deficit for the second flow");
  NS_TEST_ASSERT_MSG_EQ(flow2->GetStatus(), FqPieFlow::INACTIVE,
                        "the second flow must be inactive");

  Simulator::Destroy();
}

class FqPieQueueDiscTCPFlowsSeparation : public TestCase {
public:
  FqPieQueueDiscTCPFlowsSeparation();
  ~FqPieQueueDiscTCPFlowsSeparation() override;

private:
  void DoRun() override;
  void AddPacket(Ptr<FqPieQueueDisc> queue, Ipv4Header ipHdr, TcpHeader tcpHdr);
};

FqPieQueueDiscTCPFlowsSeparation::FqPieQueueDiscTCPFlowsSeparation()
    : TestCase("Test TCP flows separation") {}

FqPieQueueDiscTCPFlowsSeparation::~FqPieQueueDiscTCPFlowsSeparation() {}

void FqPieQueueDiscTCPFlowsSeparation::AddPacket(Ptr<FqPieQueueDisc> queue,
                                                 Ipv4Header ipHdr,
                                                 TcpHeader tcpHdr) {
  Ptr<Packet> p = Create<Packet>(100);
  p->AddHeader(tcpHdr);
  Address dest;
  Ptr<Ipv4QueueDiscItem> item = Create<Ipv4QueueDiscItem>(p, dest, 0, ipHdr);
  queue->Enqueue(item);
}

void FqPieQueueDiscTCPFlowsSeparation::DoRun() {
  Ptr<FqPieQueueDisc> queueDisc =
      CreateObjectWithAttributes<FqPieQueueDisc>("MaxSize", StringValue("10p"));

  queueDisc->SetQuantum(1500);
  queueDisc->Initialize();

  Ipv4Header hdr;
  hdr.SetPayloadSize(100);
  hdr.SetSource(Ipv4Address("10.10.1.1"));
  hdr.SetDestination(Ipv4Address("10.10.1.2"));
  hdr.SetProtocol(6);

  TcpHeader tcpHdr;
  tcpHdr.SetSourcePort(7);
  tcpHdr.SetDestinationPort(27);

  AddPacket(queueDisc, hdr, tcpHdr);
  AddPacket(queueDisc, hdr, tcpHdr);
  AddPacket(queueDisc, hdr, tcpHdr);
  NS_TEST_ASSERT_MSG_EQ(queueDisc->QueueDisc::GetNPackets(), 3,
                        "unexpected number of packets in the queue disc");
  NS_TEST_ASSERT_MSG_EQ(
      queueDisc->GetQueueDiscClass(0)->GetQueueDisc()->GetNPackets(), 3,
      "unexpected number of packets in the first flow queue");

  tcpHdr.SetSourcePort(8);
  AddPacket(queueDisc, hdr, tcpHdr);
  NS_TEST_ASSERT_MSG_EQ(queueDisc->QueueDisc::GetNPackets(), 4,
                        "unexpected number of packets in the queue disc");
  NS_TEST_ASSERT_MSG_EQ(
      queueDisc->GetQueueDiscClass(0)->GetQueueDisc()->GetNPackets(), 3,
      "unexpected number of packets in the first flow queue");
  NS_TEST_ASSERT_MSG_EQ(
      queueDisc->GetQueueDiscClass(1)->GetQueueDisc()->GetNPackets(), 1,
      "unexpected number of packets in the second flow queue");

  tcpHdr.SetDestinationPort(28);
  AddPacket(queueDisc, hdr, tcpHdr);
  NS_TEST_ASSERT_MSG_EQ(queueDisc->QueueDisc::GetNPackets(), 5,
                        "unexpected number of packets in the queue disc");
  NS_TEST_ASSERT_MSG_EQ(
      queueDisc->GetQueueDiscClass(0)->GetQueueDisc()->GetNPackets(), 3,
      "unexpected number of packets in the first flow queue");
  NS_TEST_ASSERT_MSG_EQ(
      queueDisc->GetQueueDiscClass(1)->GetQueueDisc()->GetNPackets(), 1,
      "unexpected number of packets in the second flow queue");
  NS_TEST_ASSERT_MSG_EQ(
      queueDisc->GetQueueDiscClass(2)->GetQueueDisc()->GetNPackets(), 1,
      "unexpected number of packets in the third flow queue");

  tcpHdr.SetSourcePort(7);
  AddPacket(queueDisc, hdr, tcpHdr);
  AddPacket(queueDisc, hdr, tcpHdr);
  NS_TEST_ASSERT_MSG_EQ(queueDisc->QueueDisc::GetNPackets(), 7,
                        "unexpected number of packets in the queue disc");
  NS_TEST_ASSERT_MSG_EQ(
      queueDisc->GetQueueDiscClass(0)->GetQueueDisc()->GetNPackets(), 3,
      "unexpected number of packets in the first flow queue");
  NS_TEST_ASSERT_MSG_EQ(
      queueDisc->GetQueueDiscClass(1)->GetQueueDisc()->GetNPackets(), 1,
      "unexpected number of packets in the second flow queue");
  NS_TEST_ASSERT_MSG_EQ(
      queueDisc->GetQueueDiscClass(2)->GetQueueDisc()->GetNPackets(), 1,
      "unexpected number of packets in the third flow queue");
  NS_TEST_ASSERT_MSG_EQ(
      queueDisc->GetQueueDiscClass(3)->GetQueueDisc()->GetNPackets(), 2,
      "unexpected number of packets in the third flow queue");

  Simulator::Destroy();
}

class FqPieQueueDiscUDPFlowsSeparation : public TestCase {
public:
  FqPieQueueDiscUDPFlowsSeparation();
  ~FqPieQueueDiscUDPFlowsSeparation() override;

private:
  void DoRun() override;
  void AddPacket(Ptr<FqPieQueueDisc> queue, Ipv4Header ipHdr, UdpHeader udpHdr);
};

FqPieQueueDiscUDPFlowsSeparation::FqPieQueueDiscUDPFlowsSeparation()
    : TestCase("Test UDP flows separation") {}

FqPieQueueDiscUDPFlowsSeparation::~FqPieQueueDiscUDPFlowsSeparation() {}

void FqPieQueueDiscUDPFlowsSeparation::AddPacket(Ptr<FqPieQueueDisc> queue,
                                                 Ipv4Header ipHdr,
                                                 UdpHeader udpHdr) {
  Ptr<Packet> p = Create<Packet>(100);
  p->AddHeader(udpHdr);
  Address dest;
  Ptr<Ipv4QueueDiscItem> item = Create<Ipv4QueueDiscItem>(p, dest, 0, ipHdr);
  queue->Enqueue(item);
}

void FqPieQueueDiscUDPFlowsSeparation::DoRun() {
  Ptr<FqPieQueueDisc> queueDisc =
      CreateObjectWithAttributes<FqPieQueueDisc>("MaxSize", StringValue("10p"));

  queueDisc->SetQuantum(1500);
  queueDisc->Initialize();

  Ipv4Header hdr;
  hdr.SetPayloadSize(100);
  hdr.SetSource(Ipv4Address("10.10.1.1"));
  hdr.SetDestination(Ipv4Address("10.10.1.2"));
  hdr.SetProtocol(17);

  UdpHeader udpHdr;
  udpHdr.SetSourcePort(7);
  udpHdr.SetDestinationPort(27);

  AddPacket(queueDisc, hdr, udpHdr);
  AddPacket(queueDisc, hdr, udpHdr);
  AddPacket(queueDisc, hdr, udpHdr);
  NS_TEST_ASSERT_MSG_EQ(queueDisc->QueueDisc::GetNPackets(), 3,
                        "unexpected number of packets in the queue disc");
  NS_TEST_ASSERT_MSG_EQ(
      queueDisc->GetQueueDiscClass(0)->GetQueueDisc()->GetNPackets(), 3,
      "unexpected number of packets in the first flow queue");

  udpHdr.SetSourcePort(8);
  AddPacket(queueDisc, hdr, udpHdr);
  NS_TEST_ASSERT_MSG_EQ(queueDisc->QueueDisc::GetNPackets(), 4,
                        "unexpected number of packets in the queue disc");
  NS_TEST_ASSERT_MSG_EQ(
      queueDisc->GetQueueDiscClass(0)->GetQueueDisc()->GetNPackets(), 3,
      "unexpected number of packets in the first flow queue");
  NS_TEST_ASSERT_MSG_EQ(
      queueDisc->GetQueueDiscClass(1)->GetQueueDisc()->GetNPackets(), 1,
      "unexpected number of packets in the second flow queue");

  udpHdr.SetDestinationPort(28);
  AddPacket(queueDisc, hdr, udpHdr);
  NS_TEST_ASSERT_MSG_EQ(queueDisc->QueueDisc::GetNPackets(), 5,
                        "unexpected number of packets in the queue disc");
  NS_TEST_ASSERT_MSG_EQ(
      queueDisc->GetQueueDiscClass(0)->GetQueueDisc()->GetNPackets(), 3,
      "unexpected number of packets in the first flow queue");
  NS_TEST_ASSERT_MSG_EQ(
      queueDisc->GetQueueDiscClass(1)->GetQueueDisc()->GetNPackets(), 1,
      "unexpected number of packets in the second flow queue");
  NS_TEST_ASSERT_MSG_EQ(
      queueDisc->GetQueueDiscClass(2)->GetQueueDisc()->GetNPackets(), 1,
      "unexpected number of packets in the third flow queue");

  udpHdr.SetSourcePort(7);
  AddPacket(queueDisc, hdr, udpHdr);
  AddPacket(queueDisc, hdr, udpHdr);
  NS_TEST_ASSERT_MSG_EQ(queueDisc->QueueDisc::GetNPackets(), 7,
                        "unexpected number of packets in the queue disc");
  NS_TEST_ASSERT_MSG_EQ(
      queueDisc->GetQueueDiscClass(0)->GetQueueDisc()->GetNPackets(), 3,
      "unexpected number of packets in the first flow queue");
  NS_TEST_ASSERT_MSG_EQ(
      queueDisc->GetQueueDiscClass(1)->GetQueueDisc()->GetNPackets(), 1,
      "unexpected number of packets in the second flow queue");
  NS_TEST_ASSERT_MSG_EQ(
      queueDisc->GetQueueDiscClass(2)->GetQueueDisc()->GetNPackets(), 1,
      "unexpected number of packets in the third flow queue");
  NS_TEST_ASSERT_MSG_EQ(
      queueDisc->GetQueueDiscClass(3)->GetQueueDisc()->GetNPackets(), 2,
      "unexpected number of packets in the third flow queue");

  Simulator::Destroy();
}

class FqPieQueueDiscSetLinearProbing : public TestCase {
public:
  FqPieQueueDiscSetLinearProbing();
  ~FqPieQueueDiscSetLinearProbing() override;

private:
  void DoRun() override;
  void AddPacket(Ptr<FqPieQueueDisc> queue, Ipv4Header hdr);
};

FqPieQueueDiscSetLinearProbing::FqPieQueueDiscSetLinearProbing()
    : TestCase("Test credits and flows status") {}

FqPieQueueDiscSetLinearProbing::~FqPieQueueDiscSetLinearProbing() {}

void FqPieQueueDiscSetLinearProbing::AddPacket(Ptr<FqPieQueueDisc> queue,
                                               Ipv4Header hdr) {
  Ptr<Packet> p = Create<Packet>(100);
  Address dest;
  Ptr<Ipv4QueueDiscItem> item = Create<Ipv4QueueDiscItem>(p, dest, 0, hdr);
  queue->Enqueue(item);
}

void FqPieQueueDiscSetLinearProbing::DoRun() {
  Ptr<FqPieQueueDisc> queueDisc = CreateObjectWithAttributes<FqPieQueueDisc>(
      "EnableSetAssociativeHash", BooleanValue(true));
  queueDisc->SetQuantum(90);
  queueDisc->Initialize();

  Ptr<Ipv4FqPieTestPacketFilter> filter =
      CreateObject<Ipv4FqPieTestPacketFilter>();
  queueDisc->AddPacketFilter(filter);

  Ipv4Header hdr;
  hdr.SetPayloadSize(100);
  hdr.SetSource(Ipv4Address("10.10.1.1"));
  hdr.SetDestination(Ipv4Address("10.10.1.2"));
  hdr.SetProtocol(7);

  g_hash = 0;
  AddPacket(queueDisc, hdr);
  g_hash = 1;
  AddPacket(queueDisc, hdr);
  AddPacket(queueDisc, hdr);
  g_hash = 2;
  AddPacket(queueDisc, hdr);
  g_hash = 3;
  AddPacket(queueDisc, hdr);
  g_hash = 4;
  AddPacket(queueDisc, hdr);
  AddPacket(queueDisc, hdr);
  g_hash = 5;
  AddPacket(queueDisc, hdr);
  g_hash = 6;
  AddPacket(queueDisc, hdr);
  g_hash = 7;
  AddPacket(queueDisc, hdr);
  g_hash = 1024;
  AddPacket(queueDisc, hdr);

  NS_TEST_ASSERT_MSG_EQ(queueDisc->QueueDisc::GetNPackets(), 11,
                        "unexpected number of packets in the queue disc");
  NS_TEST_ASSERT_MSG_EQ(
      queueDisc->GetQueueDiscClass(0)->GetQueueDisc()->GetNPackets(), 2,
      "unexpected number of packets in the first flow queue of set one");
  NS_TEST_ASSERT_MSG_EQ(
      queueDisc->GetQueueDiscClass(1)->GetQueueDisc()->GetNPackets(), 2,
      "unexpected number of packets in the second flow queue of set one");
  NS_TEST_ASSERT_MSG_EQ(
      queueDisc->GetQueueDiscClass(2)->GetQueueDisc()->GetNPackets(), 1,
      "unexpected number of packets in the third flow queue of set one");
  NS_TEST_ASSERT_MSG_EQ(
      queueDisc->GetQueueDiscClass(3)->GetQueueDisc()->GetNPackets(), 1,
      "unexpected number of packets in the fourth flow queue of set one");
  NS_TEST_ASSERT_MSG_EQ(
      queueDisc->GetQueueDiscClass(4)->GetQueueDisc()->GetNPackets(), 2,
      "unexpected number of packets in the fifth flow queue of set one");
  NS_TEST_ASSERT_MSG_EQ(
      queueDisc->GetQueueDiscClass(5)->GetQueueDisc()->GetNPackets(), 1,
      "unexpected number of packets in the sixth flow queue of set one");
  NS_TEST_ASSERT_MSG_EQ(
      queueDisc->GetQueueDiscClass(6)->GetQueueDisc()->GetNPackets(), 1,
      "unexpected number of packets in the seventh flow queue of set one");
  NS_TEST_ASSERT_MSG_EQ(
      queueDisc->GetQueueDiscClass(7)->GetQueueDisc()->GetNPackets(), 1,
      "unexpected number of packets in the eighth flow queue of set one");
  g_hash = 1025;
  AddPacket(queueDisc, hdr);
  NS_TEST_ASSERT_MSG_EQ(
      queueDisc->GetQueueDiscClass(0)->GetQueueDisc()->GetNPackets(), 3,
      "unexpected number of packets in the first flow of set one");
  g_hash = 10;
  AddPacket(queueDisc, hdr);
  NS_TEST_ASSERT_MSG_EQ(
      queueDisc->GetQueueDiscClass(8)->GetQueueDisc()->GetNPackets(), 1,
      "unexpected number of packets in the first flow of set two");
  Simulator::Destroy();
}

class FqPieQueueDiscL4sMode : public TestCase {
public:
  FqPieQueueDiscL4sMode();
  ~FqPieQueueDiscL4sMode() override;

private:
  void DoRun() override;
  void AddPacket(Ptr<FqPieQueueDisc> queue, Ipv4Header hdr, uint32_t nPkt);
  void AddPacketWithDelay(Ptr<FqPieQueueDisc> queue, Ipv4Header hdr,
                          double delay, uint32_t nPkt);
  void Dequeue(Ptr<FqPieQueueDisc> queue, uint32_t nPkt);
  void DequeueWithDelay(Ptr<FqPieQueueDisc> queue, double delay, uint32_t nPkt);
};

FqPieQueueDiscL4sMode::FqPieQueueDiscL4sMode() : TestCase("Test L4S mode") {}

FqPieQueueDiscL4sMode::~FqPieQueueDiscL4sMode() {}

void FqPieQueueDiscL4sMode::AddPacket(Ptr<FqPieQueueDisc> queue, Ipv4Header hdr,
                                      uint32_t nPkt) {
  Address dest;
  Ptr<Packet> p = Create<Packet>(100);
  for (uint32_t i = 0; i < nPkt; i++) {
    Ptr<Ipv4QueueDiscItem> item = Create<Ipv4QueueDiscItem>(p, dest, 0, hdr);
    queue->Enqueue(item);
  }
}

void FqPieQueueDiscL4sMode::AddPacketWithDelay(Ptr<FqPieQueueDisc> queue,
                                               Ipv4Header hdr, double delay,
                                               uint32_t nPkt) {
  for (uint32_t i = 0; i < nPkt; i++) {
    Simulator::Schedule(Time(Seconds((i + 1) * delay)),
                        &FqPieQueueDiscL4sMode::AddPacket, this, queue, hdr, 1);
  }
}

void FqPieQueueDiscL4sMode::Dequeue(Ptr<FqPieQueueDisc> queue, uint32_t nPkt) {
  for (uint32_t i = 0; i < nPkt; i++) {
    Ptr<QueueDiscItem> item = queue->Dequeue();
  }
}

void FqPieQueueDiscL4sMode::DequeueWithDelay(Ptr<FqPieQueueDisc> queue,
                                             double delay, uint32_t nPkt) {
  for (uint32_t i = 0; i < nPkt; i++) {
    Simulator::Schedule(Time(Seconds((i + 1) * delay)),
                        &FqPieQueueDiscL4sMode::Dequeue, this, queue, 1);
  }
}

void FqPieQueueDiscL4sMode::DoRun() {

  Ptr<FqPieQueueDisc> queueDisc = CreateObjectWithAttributes<FqPieQueueDisc>(
      "MaxSize", StringValue("10240p"), "UseEcn", BooleanValue(true),
      "Perturbation", UintegerValue(0), "UseL4s", BooleanValue(true),
      "CeThreshold", TimeValue(MilliSeconds(2)));

  queueDisc->SetQuantum(1514);
  queueDisc->Initialize();
  Ipv4Header hdr;
  hdr.SetPayloadSize(100);
  hdr.SetSource(Ipv4Address("10.10.1.1"));
  hdr.SetDestination(Ipv4Address("10.10.1.2"));
  hdr.SetProtocol(7);
  hdr.SetEcn(Ipv4Header::ECN_ECT1);

  double delay = 0.0005;
  Simulator::Schedule(Time(Seconds(0)),
                      &FqPieQueueDiscL4sMode::AddPacketWithDelay, this,
                      queueDisc, hdr, delay, 70);

  hdr.SetEcn(Ipv4Header::ECN_ECT0);
  hdr.SetDestination(Ipv4Address("10.10.1.10"));
  Simulator::Schedule(Time(Seconds(0)),
                      &FqPieQueueDiscL4sMode::AddPacketWithDelay, this,
                      queueDisc, hdr, delay, 70);

  delay = 0.001;
  DequeueWithDelay(queueDisc, delay, 140);
  Simulator::Stop(Seconds(10.0));
  Simulator::Run();

  Ptr<PieQueueDisc> q0 = queueDisc->GetQueueDiscClass(0)
                             ->GetQueueDisc()
                             ->GetObject<PieQueueDisc>();
  Ptr<PieQueueDisc> q1 = queueDisc->GetQueueDiscClass(1)
                             ->GetQueueDisc()
                             ->GetObject<PieQueueDisc>();

  NS_TEST_EXPECT_MSG_EQ(q0->GetStats().GetNMarkedPackets(
                            PieQueueDisc::CE_THRESHOLD_EXCEEDED_MARK),
                        66,
                        "There should be 66 marked packets"
                        "4th packet is enqueued at 2ms and dequeued at 4ms "
                        "hence the delay of 2ms which not "
                        "greater than CE threshold"
                        "5th packet is enqueued at 2.5ms and dequeued at 5ms "
                        "hence the delay of 2.5ms and "
                        "subsequent packet also do have delay"
                        "greater than CE threshold so all the packets after "
                        "4th packet are marked");
  NS_TEST_EXPECT_MSG_EQ(
      q0->GetStats().GetNDroppedPackets(PieQueueDisc::UNFORCED_DROP), 0,
      "Queue delay is less than max burst allowance so"
      "There should not be any dropped packets");
  NS_TEST_EXPECT_MSG_EQ(
      q0->GetStats().GetNMarkedPackets(PieQueueDisc::UNFORCED_MARK), 0,
      "There should not be any marked packets");
  NS_TEST_EXPECT_MSG_EQ(
      q1->GetStats().GetNMarkedPackets(PieQueueDisc::UNFORCED_MARK), 0,
      "There should not be marked packets.");
  NS_TEST_EXPECT_MSG_EQ(
      q1->GetStats().GetNDroppedPackets(PieQueueDisc::UNFORCED_DROP), 0,
      "There should not be any dropped packets");

  Simulator::Destroy();

  queueDisc = CreateObjectWithAttributes<FqPieQueueDisc>(
      "MaxSize", StringValue("10240p"), "UseEcn", BooleanValue(true),
      "Perturbation", UintegerValue(0), "UseL4s", BooleanValue(true),
      "CeThreshold", TimeValue(MilliSeconds(2)));

  queueDisc->SetQuantum(1514);
  queueDisc->Initialize();
  hdr.SetPayloadSize(100);
  hdr.SetSource(Ipv4Address("10.10.1.1"));
  hdr.SetDestination(Ipv4Address("10.10.1.2"));
  hdr.SetProtocol(7);
  hdr.SetEcn(Ipv4Header::ECN_ECT1);

  delay = 0.001;
  Simulator::Schedule(Time(Seconds(0.0005)), &FqPieQueueDiscL4sMode::AddPacket,
                      this, queueDisc, hdr, 1);
  Simulator::Schedule(Time(Seconds(0.0005)),
                      &FqPieQueueDiscL4sMode::AddPacketWithDelay, this,
                      queueDisc, hdr, delay, 69);

  hdr.SetEcn(Ipv4Header::ECN_ECT0);
  Simulator::Schedule(Time(Seconds(0)),
                      &FqPieQueueDiscL4sMode::AddPacketWithDelay, this,
                      queueDisc, hdr, delay, 70);

  DequeueWithDelay(queueDisc, delay, 140);
  Simulator::Stop(Seconds(1.0));
  Simulator::Run();
  q0 = queueDisc->GetQueueDiscClass(0)
           ->GetQueueDisc()
           ->GetObject<PieQueueDisc>();
  q0 = queueDisc->GetQueueDiscClass(0)
           ->GetQueueDisc()
           ->GetObject<PieQueueDisc>();

  NS_TEST_EXPECT_MSG_EQ(q0->GetStats().GetNMarkedPackets(
                            PieQueueDisc::CE_THRESHOLD_EXCEEDED_MARK),
                        68,
                        "There should be 68 marked packets"
                        "2nd ECT1 packet is enqueued at 1.5ms and dequeued at "
                        "3ms hence the delay of 1.5ms which "
                        "not greater than CE threshold"
                        "3rd packet is enqueued at 2.5ms and dequeued at 5ms "
                        "hence the delay of 2.5ms and "
                        "subsequent packet also do have delay"
                        "greater than CE threshold so all the packets after "
                        "2nd packet are marked");
  NS_TEST_EXPECT_MSG_EQ(
      q0->GetStats().GetNDroppedPackets(PieQueueDisc::UNFORCED_DROP), 0,
      "Queue delay is less than max burst allowance so"
      "There should not be any dropped packets");
  NS_TEST_EXPECT_MSG_EQ(
      q0->GetStats().GetNMarkedPackets(PieQueueDisc::UNFORCED_MARK), 0,
      "There should not be any marked packets");

  Simulator::Destroy();
}

class FqPieQueueDiscTestSuite : public TestSuite {
public:
  FqPieQueueDiscTestSuite();
};

FqPieQueueDiscTestSuite::FqPieQueueDiscTestSuite()
    : TestSuite("fq-pie-queue-disc", UNIT) {
  AddTestCase(new FqPieQueueDiscNoSuitableFilter, TestCase::QUICK);
  AddTestCase(new FqPieQueueDiscIPFlowsSeparationAndPacketLimit,
              TestCase::QUICK);
  AddTestCase(new FqPieQueueDiscDeficit, TestCase::QUICK);
  AddTestCase(new FqPieQueueDiscTCPFlowsSeparation, TestCase::QUICK);
  AddTestCase(new FqPieQueueDiscUDPFlowsSeparation, TestCase::QUICK);
  AddTestCase(new FqPieQueueDiscSetLinearProbing, TestCase::QUICK);
  AddTestCase(new FqPieQueueDiscL4sMode, TestCase::QUICK);
}

static FqPieQueueDiscTestSuite g_fqPieQueueDiscTestSuite;
