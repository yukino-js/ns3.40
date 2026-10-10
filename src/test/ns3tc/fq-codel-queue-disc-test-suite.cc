
#include "ns3/codel-queue-disc.h"
#include "ns3/fq-codel-queue-disc.h"
#include "ns3/ipv4-address.h"
#include "ns3/ipv4-header.h"
#include "ns3/ipv4-packet-filter.h"
#include "ns3/ipv4-queue-disc-item.h"
#include "ns3/ipv6-header.h"
#include "ns3/ipv6-packet-filter.h"
#include "ns3/ipv6-queue-disc-item.h"
#include "ns3/pointer.h"
#include "ns3/simulator.h"
#include "ns3/string.h"
#include "ns3/tcp-header.h"
#include "ns3/test.h"
#include "ns3/udp-header.h"

using namespace ns3;

static int32_t g_hash;

class Ipv4TestPacketFilter : public Ipv4PacketFilter {
public:
  static TypeId GetTypeId();

  Ipv4TestPacketFilter();
  ~Ipv4TestPacketFilter() override;

private:
  int32_t DoClassify(Ptr<QueueDiscItem> item) const override;

  bool CheckProtocol(Ptr<QueueDiscItem> item) const override;
};

TypeId Ipv4TestPacketFilter::GetTypeId() {
  static TypeId tid = TypeId("ns3::Ipv4TestPacketFilter")
                          .SetParent<Ipv4PacketFilter>()
                          .SetGroupName("Internet")
                          .AddConstructor<Ipv4TestPacketFilter>();
  return tid;
}

Ipv4TestPacketFilter::Ipv4TestPacketFilter() {}

Ipv4TestPacketFilter::~Ipv4TestPacketFilter() {}

int32_t Ipv4TestPacketFilter::DoClassify(Ptr<QueueDiscItem> item) const {
  return g_hash;
}

bool Ipv4TestPacketFilter::CheckProtocol(Ptr<QueueDiscItem> item) const {
  return true;
}

class FqCoDelQueueDiscNoSuitableFilter : public TestCase {
public:
  FqCoDelQueueDiscNoSuitableFilter();
  ~FqCoDelQueueDiscNoSuitableFilter() override;

private:
  void DoRun() override;
};

FqCoDelQueueDiscNoSuitableFilter::FqCoDelQueueDiscNoSuitableFilter()
    : TestCase("Test packets that are not classified by any filter") {}

FqCoDelQueueDiscNoSuitableFilter::~FqCoDelQueueDiscNoSuitableFilter() {}

void FqCoDelQueueDiscNoSuitableFilter::DoRun() {
  Ptr<FqCoDelQueueDisc> queueDisc =
      CreateObjectWithAttributes<FqCoDelQueueDisc>("MaxSize",
                                                   StringValue("4p"));
  Ptr<Ipv4TestPacketFilter> filter = CreateObject<Ipv4TestPacketFilter>();
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

class FqCoDelQueueDiscIPFlowsSeparationAndPacketLimit : public TestCase {
public:
  FqCoDelQueueDiscIPFlowsSeparationAndPacketLimit();
  ~FqCoDelQueueDiscIPFlowsSeparationAndPacketLimit() override;

private:
  void DoRun() override;
  void AddPacket(Ptr<FqCoDelQueueDisc> queue, Ipv4Header hdr);
};

FqCoDelQueueDiscIPFlowsSeparationAndPacketLimit::
    FqCoDelQueueDiscIPFlowsSeparationAndPacketLimit()
    : TestCase("Test IP flows separation and packet limit") {}

FqCoDelQueueDiscIPFlowsSeparationAndPacketLimit::
    ~FqCoDelQueueDiscIPFlowsSeparationAndPacketLimit() {}

void FqCoDelQueueDiscIPFlowsSeparationAndPacketLimit::AddPacket(
    Ptr<FqCoDelQueueDisc> queue, Ipv4Header hdr) {
  Ptr<Packet> p = Create<Packet>(100);
  Address dest;
  Ptr<Ipv4QueueDiscItem> item = Create<Ipv4QueueDiscItem>(p, dest, 0, hdr);
  queue->Enqueue(item);
}

void FqCoDelQueueDiscIPFlowsSeparationAndPacketLimit::DoRun() {
  Ptr<FqCoDelQueueDisc> queueDisc =
      CreateObjectWithAttributes<FqCoDelQueueDisc>("MaxSize",
                                                   StringValue("4p"));

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

class FqCoDelQueueDiscDeficit : public TestCase {
public:
  FqCoDelQueueDiscDeficit();
  ~FqCoDelQueueDiscDeficit() override;

private:
  void DoRun() override;
  void AddPacket(Ptr<FqCoDelQueueDisc> queue, Ipv4Header hdr);
};

FqCoDelQueueDiscDeficit::FqCoDelQueueDiscDeficit()
    : TestCase("Test credits and flows status") {}

FqCoDelQueueDiscDeficit::~FqCoDelQueueDiscDeficit() {}

void FqCoDelQueueDiscDeficit::AddPacket(Ptr<FqCoDelQueueDisc> queue,
                                        Ipv4Header hdr) {
  Ptr<Packet> p = Create<Packet>(100);
  Address dest;
  Ptr<Ipv4QueueDiscItem> item = Create<Ipv4QueueDiscItem>(p, dest, 0, hdr);
  queue->Enqueue(item);
}

void FqCoDelQueueDiscDeficit::DoRun() {
  Ptr<FqCoDelQueueDisc> queueDisc =
      CreateObjectWithAttributes<FqCoDelQueueDisc>();

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
  Ptr<FqCoDelFlow> flow1 =
      StaticCast<FqCoDelFlow>(queueDisc->GetQueueDiscClass(0));
  NS_TEST_ASSERT_MSG_EQ(flow1->GetDeficit(),
                        static_cast<int32_t>(queueDisc->GetQuantum()),
                        "the deficit of the first flow must equal the quantum");
  NS_TEST_ASSERT_MSG_EQ(flow1->GetStatus(), FqCoDelFlow::NEW_FLOW,
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
      flow1->GetStatus(), FqCoDelFlow::NEW_FLOW,
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
  Ptr<FqCoDelFlow> flow2 =
      StaticCast<FqCoDelFlow>(queueDisc->GetQueueDiscClass(1));
  NS_TEST_ASSERT_MSG_EQ(
      flow2->GetDeficit(), static_cast<int32_t>(queueDisc->GetQuantum()),
      "the deficit of the second flow must equal the quantum");
  NS_TEST_ASSERT_MSG_EQ(flow2->GetStatus(), FqCoDelFlow::NEW_FLOW,
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
  NS_TEST_ASSERT_MSG_EQ(flow1->GetStatus(), FqCoDelFlow::OLD_FLOW,
                        "the first flow must be in the list of old queues");
  NS_TEST_ASSERT_MSG_EQ(flow2->GetDeficit(), -30,
                        "unexpected deficit for the second flow");
  NS_TEST_ASSERT_MSG_EQ(flow2->GetStatus(), FqCoDelFlow::NEW_FLOW,
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
  NS_TEST_ASSERT_MSG_EQ(flow1->GetStatus(), FqCoDelFlow::OLD_FLOW,
                        "the first flow must be in the list of old queues");
  NS_TEST_ASSERT_MSG_EQ(flow2->GetDeficit(), 60,
                        "unexpected deficit for the second flow");
  NS_TEST_ASSERT_MSG_EQ(flow2->GetStatus(), FqCoDelFlow::OLD_FLOW,
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
  NS_TEST_ASSERT_MSG_EQ(flow1->GetStatus(), FqCoDelFlow::OLD_FLOW,
                        "the first flow must be in the list of old queues");
  NS_TEST_ASSERT_MSG_EQ(flow2->GetDeficit(), -60,
                        "unexpected deficit for the second flow");
  NS_TEST_ASSERT_MSG_EQ(flow2->GetStatus(), FqCoDelFlow::OLD_FLOW,
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
  NS_TEST_ASSERT_MSG_EQ(flow1->GetStatus(), FqCoDelFlow::OLD_FLOW,
                        "the first flow must be in the list of old queues");
  NS_TEST_ASSERT_MSG_EQ(flow2->GetDeficit(), 30,
                        "unexpected deficit for the second flow");
  NS_TEST_ASSERT_MSG_EQ(flow2->GetStatus(), FqCoDelFlow::OLD_FLOW,
                        "the second flow must be in the list of new queues");

  queueDisc->Dequeue();
  NS_TEST_ASSERT_MSG_EQ(flow1->GetDeficit(), 90,
                        "unexpected deficit for the first flow");
  NS_TEST_ASSERT_MSG_EQ(flow1->GetStatus(), FqCoDelFlow::INACTIVE,
                        "the first flow must be inactive");
  NS_TEST_ASSERT_MSG_EQ(flow2->GetDeficit(), 30,
                        "unexpected deficit for the second flow");
  NS_TEST_ASSERT_MSG_EQ(flow2->GetStatus(), FqCoDelFlow::INACTIVE,
                        "the second flow must be inactive");

  Simulator::Destroy();
}

class FqCoDelQueueDiscTCPFlowsSeparation : public TestCase {
public:
  FqCoDelQueueDiscTCPFlowsSeparation();
  ~FqCoDelQueueDiscTCPFlowsSeparation() override;

private:
  void DoRun() override;
  void AddPacket(Ptr<FqCoDelQueueDisc> queue, Ipv4Header ipHdr,
                 TcpHeader tcpHdr);
};

FqCoDelQueueDiscTCPFlowsSeparation::FqCoDelQueueDiscTCPFlowsSeparation()
    : TestCase("Test TCP flows separation") {}

FqCoDelQueueDiscTCPFlowsSeparation::~FqCoDelQueueDiscTCPFlowsSeparation() {}

void FqCoDelQueueDiscTCPFlowsSeparation::AddPacket(Ptr<FqCoDelQueueDisc> queue,
                                                   Ipv4Header ipHdr,
                                                   TcpHeader tcpHdr) {
  Ptr<Packet> p = Create<Packet>(100);
  p->AddHeader(tcpHdr);
  Address dest;
  Ptr<Ipv4QueueDiscItem> item = Create<Ipv4QueueDiscItem>(p, dest, 0, ipHdr);
  queue->Enqueue(item);
}

void FqCoDelQueueDiscTCPFlowsSeparation::DoRun() {
  Ptr<FqCoDelQueueDisc> queueDisc =
      CreateObjectWithAttributes<FqCoDelQueueDisc>("MaxSize",
                                                   StringValue("10p"));

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

class FqCoDelQueueDiscUDPFlowsSeparation : public TestCase {
public:
  FqCoDelQueueDiscUDPFlowsSeparation();
  ~FqCoDelQueueDiscUDPFlowsSeparation() override;

private:
  void DoRun() override;
  void AddPacket(Ptr<FqCoDelQueueDisc> queue, Ipv4Header ipHdr,
                 UdpHeader udpHdr);
};

FqCoDelQueueDiscUDPFlowsSeparation::FqCoDelQueueDiscUDPFlowsSeparation()
    : TestCase("Test UDP flows separation") {}

FqCoDelQueueDiscUDPFlowsSeparation::~FqCoDelQueueDiscUDPFlowsSeparation() {}

void FqCoDelQueueDiscUDPFlowsSeparation::AddPacket(Ptr<FqCoDelQueueDisc> queue,
                                                   Ipv4Header ipHdr,
                                                   UdpHeader udpHdr) {
  Ptr<Packet> p = Create<Packet>(100);
  p->AddHeader(udpHdr);
  Address dest;
  Ptr<Ipv4QueueDiscItem> item = Create<Ipv4QueueDiscItem>(p, dest, 0, ipHdr);
  queue->Enqueue(item);
}

void FqCoDelQueueDiscUDPFlowsSeparation::DoRun() {
  Ptr<FqCoDelQueueDisc> queueDisc =
      CreateObjectWithAttributes<FqCoDelQueueDisc>("MaxSize",
                                                   StringValue("10p"));

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

class FqCoDelQueueDiscECNMarking : public TestCase {
public:
  FqCoDelQueueDiscECNMarking();
  ~FqCoDelQueueDiscECNMarking() override;

private:
  void DoRun() override;
  void AddPacket(Ptr<FqCoDelQueueDisc> queue, Ipv4Header hdr, uint32_t nPkt,
                 uint32_t nPktEnqueued, uint32_t nQueueFlows);
  void Dequeue(Ptr<FqCoDelQueueDisc> queue, uint32_t nPkt);
  void DequeueWithDelay(Ptr<FqCoDelQueueDisc> queue, double delay,
                        uint32_t nPkt);
};

FqCoDelQueueDiscECNMarking::FqCoDelQueueDiscECNMarking()
    : TestCase("Test ECN marking") {}

FqCoDelQueueDiscECNMarking::~FqCoDelQueueDiscECNMarking() {}

void FqCoDelQueueDiscECNMarking::AddPacket(Ptr<FqCoDelQueueDisc> queue,
                                           Ipv4Header hdr, uint32_t nPkt,
                                           uint32_t nPktEnqueued,
                                           uint32_t nQueueFlows) {
  Address dest;
  Ptr<Packet> p = Create<Packet>(100);
  for (uint32_t i = 0; i < nPkt; i++) {
    Ptr<Ipv4QueueDiscItem> item = Create<Ipv4QueueDiscItem>(p, dest, 0, hdr);
    queue->Enqueue(item);
  }
  NS_TEST_EXPECT_MSG_EQ(queue->GetNQueueDiscClasses(), nQueueFlows,
                        "unexpected number of flow queues");
  NS_TEST_EXPECT_MSG_EQ(queue->GetNPackets(), nPktEnqueued,
                        "unexpected number of enqueued packets");
}

void FqCoDelQueueDiscECNMarking::Dequeue(Ptr<FqCoDelQueueDisc> queue,
                                         uint32_t nPkt) {
  for (uint32_t i = 0; i < nPkt; i++) {
    Ptr<QueueDiscItem> item = queue->Dequeue();
  }
}

void FqCoDelQueueDiscECNMarking::DequeueWithDelay(Ptr<FqCoDelQueueDisc> queue,
                                                  double delay, uint32_t nPkt) {
  for (uint32_t i = 0; i < nPkt; i++) {
    Simulator::Schedule(Time(Seconds((i + 1) * delay)),
                        &FqCoDelQueueDiscECNMarking::Dequeue, this, queue, 1);
  }
}

void FqCoDelQueueDiscECNMarking::DoRun() {

  Ptr<FqCoDelQueueDisc> queueDisc =
      CreateObjectWithAttributes<FqCoDelQueueDisc>(
          "MaxSize", StringValue("10240p"), "UseEcn", BooleanValue(true),
          "Perturbation", UintegerValue(0));

  queueDisc->SetQuantum(1514);
  queueDisc->Initialize();
  Ipv4Header hdr;
  hdr.SetPayloadSize(100);
  hdr.SetSource(Ipv4Address("10.10.1.1"));
  hdr.SetDestination(Ipv4Address("10.10.1.2"));
  hdr.SetProtocol(7);
  hdr.SetEcn(Ipv4Header::ECN_ECT0);

  Simulator::Schedule(Time(Seconds(0)), &FqCoDelQueueDiscECNMarking::AddPacket,
                      this, queueDisc, hdr, 20, 20, 1);

  hdr.SetDestination(Ipv4Address("10.10.1.10"));
  Simulator::Schedule(Time(Seconds(0)), &FqCoDelQueueDiscECNMarking::AddPacket,
                      this, queueDisc, hdr, 20, 40, 2);

  hdr.SetDestination(Ipv4Address("10.10.1.20"));
  Simulator::Schedule(Time(Seconds(0)), &FqCoDelQueueDiscECNMarking::AddPacket,
                      this, queueDisc, hdr, 20, 60, 3);

  hdr.SetDestination(Ipv4Address("10.10.1.30"));
  hdr.SetEcn(Ipv4Header::ECN_NotECT);
  Simulator::Schedule(Time(Seconds(0)), &FqCoDelQueueDiscECNMarking::AddPacket,
                      this, queueDisc, hdr, 20, 80, 4);

  hdr.SetDestination(Ipv4Address("10.10.1.40"));
  Simulator::Schedule(Time(Seconds(0)), &FqCoDelQueueDiscECNMarking::AddPacket,
                      this, queueDisc, hdr, 20, 100, 5);

  DequeueWithDelay(queueDisc, 0.11, 60);
  Simulator::Run();
  Simulator::Stop(Seconds(8.0));
  Ptr<CoDelQueueDisc> q0 = queueDisc->GetQueueDiscClass(0)
                               ->GetQueueDisc()
                               ->GetObject<CoDelQueueDisc>();
  Ptr<CoDelQueueDisc> q1 = queueDisc->GetQueueDiscClass(1)
                               ->GetQueueDisc()
                               ->GetObject<CoDelQueueDisc>();
  Ptr<CoDelQueueDisc> q2 = queueDisc->GetQueueDiscClass(2)
                               ->GetQueueDisc()
                               ->GetObject<CoDelQueueDisc>();
  Ptr<CoDelQueueDisc> q3 = queueDisc->GetQueueDiscClass(3)
                               ->GetQueueDisc()
                               ->GetObject<CoDelQueueDisc>();
  Ptr<CoDelQueueDisc> q4 = queueDisc->GetQueueDiscClass(4)
                               ->GetQueueDisc()
                               ->GetObject<CoDelQueueDisc>();

  NS_TEST_EXPECT_MSG_NE(
      queueDisc->GetQueueDiscClass(0)->GetQueueDisc()->GetNPackets(), 0,
      "There should be some remaining packets");
  NS_TEST_EXPECT_MSG_NE(
      queueDisc->GetQueueDiscClass(1)->GetQueueDisc()->GetNPackets(), 0,
      "There should be some remaining packets");
  NS_TEST_EXPECT_MSG_NE(
      queueDisc->GetQueueDiscClass(2)->GetQueueDisc()->GetNPackets(), 0,
      "There should be some remaining packets");
  NS_TEST_EXPECT_MSG_NE(
      queueDisc->GetQueueDiscClass(3)->GetQueueDisc()->GetNPackets(), 0,
      "There should be some remaining packets");
  NS_TEST_EXPECT_MSG_NE(
      queueDisc->GetQueueDiscClass(4)->GetQueueDisc()->GetNPackets(), 0,
      "There should be some remaining packets");

  NS_TEST_EXPECT_MSG_EQ(
      q0->GetStats().GetNMarkedPackets(CoDelQueueDisc::TARGET_EXCEEDED_MARK), 6,
      "There should be 6 marked packets"
      "with 20 packets, total bytes in the queue = 120 * 20 = 2400. First "
      "packet dequeues at 110ms which is greater than"
      "test's default target value 5ms. Sojourn time has just gone above "
      "target from below, need to stay above for at"
      "least q->interval before packet can be dropped. Second packet dequeues "
      "at 220ms which is greater than last dequeue"
      "time plus q->interval(test default 100ms) so the packet is marked. "
      "Third packet dequeues at 330ms and the sojourn"
      "time stayed above the target and dropnext value is less than 320 hence "
      "the packet is marked. 4 subsequent packets"
      "are marked as the sojourn time stays above the target. With 8th dequeue "
      "number of bytes in queue = 120 * 12 = 1440"
      "which is less m_minBytes(test's default value 1500 bytes) hence the "
      "packets stop getting marked");
  NS_TEST_EXPECT_MSG_EQ(
      q0->GetStats().GetNDroppedPackets(CoDelQueueDisc::TARGET_EXCEEDED_DROP),
      0, "There should not be any dropped packets");
  NS_TEST_EXPECT_MSG_EQ(
      q1->GetStats().GetNMarkedPackets(CoDelQueueDisc::TARGET_EXCEEDED_MARK), 6,
      "There should be 6 marked packets");
  NS_TEST_EXPECT_MSG_EQ(
      q1->GetStats().GetNDroppedPackets(CoDelQueueDisc::TARGET_EXCEEDED_DROP),
      0, "There should not be any dropped packets");
  NS_TEST_EXPECT_MSG_EQ(
      q2->GetStats().GetNMarkedPackets(CoDelQueueDisc::TARGET_EXCEEDED_MARK), 6,
      "There should be 6 marked packets");
  NS_TEST_EXPECT_MSG_EQ(
      q2->GetStats().GetNDroppedPackets(CoDelQueueDisc::TARGET_EXCEEDED_DROP),
      0, "There should not be any dropped packets");

  NS_TEST_EXPECT_MSG_EQ(
      q3->GetStats().GetNDroppedPackets(CoDelQueueDisc::TARGET_EXCEEDED_DROP),
      4,
      "There should be 4 dropped packets"
      "with 20 packets, total bytes in the queue = 120 * 20 = 2400. First "
      "packet dequeues at "
      "110ms which is greater than"
      "test's default target value 5ms. Sojourn time has just gone above "
      "target from below, need "
      "to stay above for at"
      "least q->interval before packet can be dropped. Second packet dequeues "
      "at 220ms which is "
      "greater than last dequeue"
      "time plus q->interval(test default 100ms) so packet is dropped and next "
      "is dequeued. 4th "
      "packet dequeues at 330ms"
      "and the sojourn time stayed above the target and dropnext value is less "
      "than 320 hence "
      "the packet is dropped and next"
      "packet is dequeued. 6th packet dequeues at 440ms and 2 more packets are "
      "dropped as "
      "dropnext value is increased twice."
      "12 Packets remaining in the queue, total number of bytes int the queue "
      "= 120 * 12 = 1440 "
      "which is less"
      "m_minBytes(test's default value 1500 bytes) hence the packets stop "
      "getting dropped");
  NS_TEST_EXPECT_MSG_EQ(
      q3->GetStats().GetNMarkedPackets(CoDelQueueDisc::TARGET_EXCEEDED_MARK), 0,
      "There should not be any marked packets");
  NS_TEST_EXPECT_MSG_EQ(
      q4->GetStats().GetNDroppedPackets(CoDelQueueDisc::TARGET_EXCEEDED_DROP),
      4, "There should be 4 dropped packets");
  NS_TEST_EXPECT_MSG_EQ(
      q4->GetStats().GetNMarkedPackets(CoDelQueueDisc::TARGET_EXCEEDED_MARK), 0,
      "There should not be any marked packets");
  Ptr<const Ipv4QueueDiscItem> pktQ0 =
      DynamicCast<const Ipv4QueueDiscItem>(q0->Peek());
  NS_TEST_EXPECT_MSG_NE(pktQ0->GetHeader().GetEcn(), Ipv4Header::ECN_NotECT,
                        "flow queue should have ECT0 packets");
  Ptr<const Ipv4QueueDiscItem> pktQ1 =
      DynamicCast<const Ipv4QueueDiscItem>(q1->Peek());
  NS_TEST_EXPECT_MSG_NE(pktQ1->GetHeader().GetEcn(), Ipv4Header::ECN_NotECT,
                        "flow queue should have ECT0 packets");
  Ptr<const Ipv4QueueDiscItem> pktQ2 =
      DynamicCast<const Ipv4QueueDiscItem>(q2->Peek());
  NS_TEST_EXPECT_MSG_NE(pktQ2->GetHeader().GetEcn(), Ipv4Header::ECN_NotECT,
                        "flow queue should have ECT0 packets");

  Simulator::Destroy();

  queueDisc = CreateObjectWithAttributes<FqCoDelQueueDisc>(
      "MaxSize", StringValue("10240p"), "UseEcn", BooleanValue(true),
      "CeThreshold", TimeValue(MilliSeconds(2)));
  queueDisc->SetQuantum(1514);
  queueDisc->Initialize();

  hdr.SetDestination(Ipv4Address("10.10.1.2"));
  hdr.SetEcn(Ipv4Header::ECN_ECT0);
  Simulator::Schedule(Time(Seconds(0)), &FqCoDelQueueDiscECNMarking::AddPacket,
                      this, queueDisc, hdr, 20, 20, 1);

  hdr.SetDestination(Ipv4Address("10.10.1.10"));
  Simulator::Schedule(Time(Seconds(0)), &FqCoDelQueueDiscECNMarking::AddPacket,
                      this, queueDisc, hdr, 20, 40, 2);

  hdr.SetDestination(Ipv4Address("10.10.1.20"));
  Simulator::Schedule(Time(Seconds(0)), &FqCoDelQueueDiscECNMarking::AddPacket,
                      this, queueDisc, hdr, 20, 60, 3);

  hdr.SetDestination(Ipv4Address("10.10.1.30"));
  hdr.SetEcn(Ipv4Header::ECN_NotECT);
  Simulator::Schedule(Time(Seconds(0)), &FqCoDelQueueDiscECNMarking::AddPacket,
                      this, queueDisc, hdr, 20, 80, 4);

  hdr.SetDestination(Ipv4Address("10.10.1.40"));
  Simulator::Schedule(Time(Seconds(0)), &FqCoDelQueueDiscECNMarking::AddPacket,
                      this, queueDisc, hdr, 20, 100, 5);

  DequeueWithDelay(queueDisc, 0.0001, 60);
  Simulator::Run();
  Simulator::Stop(Seconds(8.0));
  q0 = queueDisc->GetQueueDiscClass(0)
           ->GetQueueDisc()
           ->GetObject<CoDelQueueDisc>();
  q1 = queueDisc->GetQueueDiscClass(1)
           ->GetQueueDisc()
           ->GetObject<CoDelQueueDisc>();
  q2 = queueDisc->GetQueueDiscClass(2)
           ->GetQueueDisc()
           ->GetObject<CoDelQueueDisc>();
  q3 = queueDisc->GetQueueDiscClass(3)
           ->GetQueueDisc()
           ->GetObject<CoDelQueueDisc>();
  q4 = queueDisc->GetQueueDiscClass(4)
           ->GetQueueDisc()
           ->GetObject<CoDelQueueDisc>();

  NS_TEST_EXPECT_MSG_NE(
      queueDisc->GetQueueDiscClass(0)->GetQueueDisc()->GetNPackets(), 0,
      "There should be some remaining packets");
  NS_TEST_EXPECT_MSG_NE(
      queueDisc->GetQueueDiscClass(1)->GetQueueDisc()->GetNPackets(), 0,
      "There should be some remaining packets");
  NS_TEST_EXPECT_MSG_NE(
      queueDisc->GetQueueDiscClass(2)->GetQueueDisc()->GetNPackets(), 0,
      "There should be some remaining packets");
  NS_TEST_EXPECT_MSG_NE(
      queueDisc->GetQueueDiscClass(3)->GetQueueDisc()->GetNPackets(), 0,
      "There should be some remaining packets");
  NS_TEST_EXPECT_MSG_NE(
      queueDisc->GetQueueDiscClass(4)->GetQueueDisc()->GetNPackets(), 0,
      "There should be some remaining packets");

  NS_TEST_EXPECT_MSG_EQ(
      q0->GetStats().GetNDroppedPackets(CoDelQueueDisc::TARGET_EXCEEDED_DROP),
      0, "There should not be any dropped packets");
  NS_TEST_EXPECT_MSG_EQ(q0->GetStats().GetNMarkedPackets(
                            CoDelQueueDisc::CE_THRESHOLD_EXCEEDED_MARK),
                        0,
                        "There should not be any marked packets"
                        "with quantum of 1514, 13 packets of size 120 bytes "
                        "can be dequeued. sojourn time of 13th "
                        "packet is 1.3ms which is"
                        "less than CE threshold");
  NS_TEST_EXPECT_MSG_EQ(
      q1->GetStats().GetNDroppedPackets(CoDelQueueDisc::TARGET_EXCEEDED_DROP),
      0, "There should not be any dropped packets");
  NS_TEST_EXPECT_MSG_EQ(q1->GetStats().GetNMarkedPackets(
                            CoDelQueueDisc::CE_THRESHOLD_EXCEEDED_MARK),
                        6,
                        "There should be 6 marked packets"
                        "with quantum of 1514, 13 packets of size 120 bytes "
                        "can be dequeued. sojourn time of 8th "
                        "packet is 2.1ms which is greater"
                        "than CE threshold and subsequent packet also have "
                        "sojourn time more 8th packet hence "
                        "remaining packet are marked.");
  NS_TEST_EXPECT_MSG_EQ(
      q2->GetStats().GetNDroppedPackets(CoDelQueueDisc::TARGET_EXCEEDED_DROP),
      0, "There should not be any dropped packets");
  NS_TEST_EXPECT_MSG_EQ(q2->GetStats().GetNMarkedPackets(
                            CoDelQueueDisc::CE_THRESHOLD_EXCEEDED_MARK),
                        13,
                        "There should be 13 marked packets"
                        "with quantum of 1514, 13 packets of size 120 bytes "
                        "can be dequeued and all of them have "
                        "sojourn time more than CE threshold");

  NS_TEST_EXPECT_MSG_EQ(q3->GetStats().GetNMarkedPackets(
                            CoDelQueueDisc::CE_THRESHOLD_EXCEEDED_MARK),
                        0, "There should not be any marked packets");
  NS_TEST_EXPECT_MSG_EQ(
      q3->GetStats().GetNDroppedPackets(CoDelQueueDisc::TARGET_EXCEEDED_DROP),
      0, "There should not be any dropped packets");
  NS_TEST_EXPECT_MSG_EQ(q4->GetStats().GetNMarkedPackets(
                            CoDelQueueDisc::CE_THRESHOLD_EXCEEDED_MARK),
                        0, "There should not be any marked packets");
  NS_TEST_EXPECT_MSG_EQ(
      q4->GetStats().GetNDroppedPackets(CoDelQueueDisc::TARGET_EXCEEDED_DROP),
      0, "There should not be any dropped packets");

  pktQ0 = DynamicCast<const Ipv4QueueDiscItem>(q0->Peek());
  NS_TEST_EXPECT_MSG_NE(pktQ0->GetHeader().GetEcn(), Ipv4Header::ECN_NotECT,
                        "flow queue should have ECT0 packets");
  pktQ1 = DynamicCast<const Ipv4QueueDiscItem>(q1->Peek());
  NS_TEST_EXPECT_MSG_NE(pktQ1->GetHeader().GetEcn(), Ipv4Header::ECN_NotECT,
                        "flow queue should have ECT0 packets");
  pktQ2 = DynamicCast<const Ipv4QueueDiscItem>(q2->Peek());
  NS_TEST_EXPECT_MSG_NE(pktQ2->GetHeader().GetEcn(), Ipv4Header::ECN_NotECT,
                        "flow queue should have ECT0 packets");

  Simulator::Destroy();

  queueDisc = CreateObjectWithAttributes<FqCoDelQueueDisc>(
      "MaxSize", StringValue("10240p"), "UseEcn", BooleanValue(true),
      "CeThreshold", TimeValue(MilliSeconds(2)));
  queueDisc->SetQuantum(1514);
  queueDisc->Initialize();

  hdr.SetDestination(Ipv4Address("10.10.1.2"));
  hdr.SetEcn(Ipv4Header::ECN_ECT0);
  Simulator::Schedule(Time(Seconds(0)), &FqCoDelQueueDiscECNMarking::AddPacket,
                      this, queueDisc, hdr, 20, 20, 1);

  hdr.SetDestination(Ipv4Address("10.10.1.10"));
  Simulator::Schedule(Time(Seconds(0)), &FqCoDelQueueDiscECNMarking::AddPacket,
                      this, queueDisc, hdr, 20, 40, 2);

  hdr.SetDestination(Ipv4Address("10.10.1.20"));
  Simulator::Schedule(Time(Seconds(0)), &FqCoDelQueueDiscECNMarking::AddPacket,
                      this, queueDisc, hdr, 20, 60, 3);

  hdr.SetDestination(Ipv4Address("10.10.1.30"));
  hdr.SetEcn(Ipv4Header::ECN_NotECT);
  Simulator::Schedule(Time(Seconds(0)), &FqCoDelQueueDiscECNMarking::AddPacket,
                      this, queueDisc, hdr, 20, 80, 4);

  hdr.SetDestination(Ipv4Address("10.10.1.40"));
  Simulator::Schedule(Time(Seconds(0)), &FqCoDelQueueDiscECNMarking::AddPacket,
                      this, queueDisc, hdr, 20, 100, 5);

  DequeueWithDelay(queueDisc, 0.110, 60);
  Simulator::Run();
  Simulator::Stop(Seconds(8.0));
  q0 = queueDisc->GetQueueDiscClass(0)
           ->GetQueueDisc()
           ->GetObject<CoDelQueueDisc>();
  q1 = queueDisc->GetQueueDiscClass(1)
           ->GetQueueDisc()
           ->GetObject<CoDelQueueDisc>();
  q2 = queueDisc->GetQueueDiscClass(2)
           ->GetQueueDisc()
           ->GetObject<CoDelQueueDisc>();
  q3 = queueDisc->GetQueueDiscClass(3)
           ->GetQueueDisc()
           ->GetObject<CoDelQueueDisc>();
  q4 = queueDisc->GetQueueDiscClass(4)
           ->GetQueueDisc()
           ->GetObject<CoDelQueueDisc>();

  NS_TEST_EXPECT_MSG_NE(
      queueDisc->GetQueueDiscClass(0)->GetQueueDisc()->GetNPackets(), 0,
      "There should be some remaining packets");
  NS_TEST_EXPECT_MSG_NE(
      queueDisc->GetQueueDiscClass(1)->GetQueueDisc()->GetNPackets(), 0,
      "There should be some remaining packets");
  NS_TEST_EXPECT_MSG_NE(
      queueDisc->GetQueueDiscClass(2)->GetQueueDisc()->GetNPackets(), 0,
      "There should be some remaining packets");
  NS_TEST_EXPECT_MSG_NE(
      queueDisc->GetQueueDiscClass(3)->GetQueueDisc()->GetNPackets(), 0,
      "There should be some remaining packets");
  NS_TEST_EXPECT_MSG_NE(
      queueDisc->GetQueueDiscClass(4)->GetQueueDisc()->GetNPackets(), 0,
      "There should be some remaining packets");

  NS_TEST_EXPECT_MSG_EQ(
      q0->GetStats().GetNDroppedPackets(CoDelQueueDisc::TARGET_EXCEEDED_DROP),
      0, "There should not be any dropped packets");
  NS_TEST_EXPECT_MSG_EQ(q0->GetStats().GetNMarkedPackets(
                            CoDelQueueDisc::CE_THRESHOLD_EXCEEDED_MARK) +
                            q0->GetStats().GetNMarkedPackets(
                                CoDelQueueDisc::TARGET_EXCEEDED_MARK),
                        20 - q0->GetNPackets(),
                        "Number of CE threshold"
                        " exceeded marks plus Number of Target exceeded marks "
                        "should be equal to total number of "
                        "packets dequeued");
  NS_TEST_EXPECT_MSG_EQ(
      q1->GetStats().GetNDroppedPackets(CoDelQueueDisc::TARGET_EXCEEDED_DROP),
      0, "There should not be any dropped packets");
  NS_TEST_EXPECT_MSG_EQ(q1->GetStats().GetNMarkedPackets(
                            CoDelQueueDisc::CE_THRESHOLD_EXCEEDED_MARK) +
                            q1->GetStats().GetNMarkedPackets(
                                CoDelQueueDisc::TARGET_EXCEEDED_MARK),
                        20 - q1->GetNPackets(),
                        "Number of CE threshold"
                        " exceeded marks plus Number of Target exceeded marks "
                        "should be equal to total number of "
                        "packets dequeued");
  NS_TEST_EXPECT_MSG_EQ(
      q2->GetStats().GetNDroppedPackets(CoDelQueueDisc::TARGET_EXCEEDED_DROP),
      0, "There should not be any dropped packets");
  NS_TEST_EXPECT_MSG_EQ(q2->GetStats().GetNMarkedPackets(
                            CoDelQueueDisc::CE_THRESHOLD_EXCEEDED_MARK) +
                            q2->GetStats().GetNMarkedPackets(
                                CoDelQueueDisc::TARGET_EXCEEDED_MARK),
                        20 - q2->GetNPackets(),
                        "Number of CE threshold"
                        " exceeded marks plus Number of Target exceeded marks "
                        "should be equal to total number of "
                        "packets dequeued");

  NS_TEST_EXPECT_MSG_EQ(q3->GetStats().GetNMarkedPackets(
                            CoDelQueueDisc::CE_THRESHOLD_EXCEEDED_MARK),
                        0, "There should not be any marked packets");
  NS_TEST_EXPECT_MSG_EQ(
      q3->GetStats().GetNDroppedPackets(CoDelQueueDisc::TARGET_EXCEEDED_DROP),
      4,
      "There should be 4 dropped packets"
      " As queue delay is same as in test case 1, number of dropped packets "
      "should also be same");
  NS_TEST_EXPECT_MSG_EQ(q4->GetStats().GetNMarkedPackets(
                            CoDelQueueDisc::CE_THRESHOLD_EXCEEDED_MARK),
                        0, "There should not be any marked packets");
  NS_TEST_EXPECT_MSG_EQ(
      q4->GetStats().GetNDroppedPackets(CoDelQueueDisc::TARGET_EXCEEDED_DROP),
      4, "There should be 4 dropped packets");

  pktQ0 = DynamicCast<const Ipv4QueueDiscItem>(q0->Peek());
  NS_TEST_EXPECT_MSG_NE(pktQ0->GetHeader().GetEcn(), Ipv4Header::ECN_NotECT,
                        "flow queue should have ECT0 packets");
  pktQ1 = DynamicCast<const Ipv4QueueDiscItem>(q1->Peek());
  NS_TEST_EXPECT_MSG_NE(pktQ1->GetHeader().GetEcn(), Ipv4Header::ECN_NotECT,
                        "flow queue should have ECT0 packets");
  pktQ2 = DynamicCast<const Ipv4QueueDiscItem>(q2->Peek());
  NS_TEST_EXPECT_MSG_NE(pktQ2->GetHeader().GetEcn(), Ipv4Header::ECN_NotECT,
                        "flow queue should have ECT0 packets");

  Simulator::Destroy();
}

class FqCoDelQueueDiscSetLinearProbing : public TestCase {
public:
  FqCoDelQueueDiscSetLinearProbing();
  ~FqCoDelQueueDiscSetLinearProbing() override;

private:
  void DoRun() override;
  void AddPacket(Ptr<FqCoDelQueueDisc> queue, Ipv4Header hdr);
};

FqCoDelQueueDiscSetLinearProbing::FqCoDelQueueDiscSetLinearProbing()
    : TestCase("Test credits and flows status") {}

FqCoDelQueueDiscSetLinearProbing::~FqCoDelQueueDiscSetLinearProbing() {}

void FqCoDelQueueDiscSetLinearProbing::AddPacket(Ptr<FqCoDelQueueDisc> queue,
                                                 Ipv4Header hdr) {
  Ptr<Packet> p = Create<Packet>(100);
  Address dest;
  Ptr<Ipv4QueueDiscItem> item = Create<Ipv4QueueDiscItem>(p, dest, 0, hdr);
  queue->Enqueue(item);
}

void FqCoDelQueueDiscSetLinearProbing::DoRun() {
  Ptr<FqCoDelQueueDisc> queueDisc =
      CreateObjectWithAttributes<FqCoDelQueueDisc>("EnableSetAssociativeHash",
                                                   BooleanValue(true));
  queueDisc->SetQuantum(90);
  queueDisc->Initialize();

  Ptr<Ipv4TestPacketFilter> filter = CreateObject<Ipv4TestPacketFilter>();
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

class FqCoDelQueueDiscL4sMode : public TestCase {
public:
  FqCoDelQueueDiscL4sMode();
  ~FqCoDelQueueDiscL4sMode() override;

private:
  void DoRun() override;

  void AddPacket(Ptr<FqCoDelQueueDisc> queue, Ipv4Header hdr, uint32_t nPkt);

  void AddPacketWithDelay(Ptr<FqCoDelQueueDisc> queue, Ipv4Header hdr,
                          double delay, uint32_t nPkt);

  void Dequeue(Ptr<FqCoDelQueueDisc> queue, uint32_t nPkt);
  void DequeueWithDelay(Ptr<FqCoDelQueueDisc> queue, double delay,
                        uint32_t nPkt);
};

FqCoDelQueueDiscL4sMode::FqCoDelQueueDiscL4sMode()
    : TestCase("Test L4S mode") {}

FqCoDelQueueDiscL4sMode::~FqCoDelQueueDiscL4sMode() {}

void FqCoDelQueueDiscL4sMode::AddPacket(Ptr<FqCoDelQueueDisc> queue,
                                        Ipv4Header hdr, uint32_t nPkt) {
  Address dest;
  Ptr<Packet> p = Create<Packet>(100);
  for (uint32_t i = 0; i < nPkt; i++) {
    Ptr<Ipv4QueueDiscItem> item = Create<Ipv4QueueDiscItem>(p, dest, 0, hdr);
    queue->Enqueue(item);
  }
}

void FqCoDelQueueDiscL4sMode::AddPacketWithDelay(Ptr<FqCoDelQueueDisc> queue,
                                                 Ipv4Header hdr, double delay,
                                                 uint32_t nPkt) {
  for (uint32_t i = 0; i < nPkt; i++) {
    Simulator::Schedule(Time(Seconds((i + 1) * delay)),
                        &FqCoDelQueueDiscL4sMode::AddPacket, this, queue, hdr,
                        1);
  }
}

void FqCoDelQueueDiscL4sMode::Dequeue(Ptr<FqCoDelQueueDisc> queue,
                                      uint32_t nPkt) {
  for (uint32_t i = 0; i < nPkt; i++) {
    Ptr<QueueDiscItem> item = queue->Dequeue();
  }
}

void FqCoDelQueueDiscL4sMode::DequeueWithDelay(Ptr<FqCoDelQueueDisc> queue,
                                               double delay, uint32_t nPkt) {
  for (uint32_t i = 0; i < nPkt; i++) {
    Simulator::Schedule(Time(Seconds((i + 1) * delay)),
                        &FqCoDelQueueDiscL4sMode::Dequeue, this, queue, 1);
  }
}

void FqCoDelQueueDiscL4sMode::DoRun() {

  Ptr<FqCoDelQueueDisc> queueDisc =
      CreateObjectWithAttributes<FqCoDelQueueDisc>(
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
                      &FqCoDelQueueDiscL4sMode::AddPacketWithDelay, this,
                      queueDisc, hdr, delay, 70);

  hdr.SetEcn(Ipv4Header::ECN_ECT0);
  hdr.SetDestination(Ipv4Address("10.10.1.10"));
  Simulator::Schedule(Time(Seconds(0)),
                      &FqCoDelQueueDiscL4sMode::AddPacketWithDelay, this,
                      queueDisc, hdr, delay, 70);

  delay = 0.001;
  DequeueWithDelay(queueDisc, delay, 140);
  Simulator::Run();
  Simulator::Stop(Seconds(8.0));
  Ptr<CoDelQueueDisc> q0 = queueDisc->GetQueueDiscClass(0)
                               ->GetQueueDisc()
                               ->GetObject<CoDelQueueDisc>();
  Ptr<CoDelQueueDisc> q1 = queueDisc->GetQueueDiscClass(1)
                               ->GetQueueDisc()
                               ->GetObject<CoDelQueueDisc>();

  NS_TEST_EXPECT_MSG_EQ(q0->GetStats().GetNMarkedPackets(
                            CoDelQueueDisc::CE_THRESHOLD_EXCEEDED_MARK),
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
      q0->GetStats().GetNDroppedPackets(CoDelQueueDisc::TARGET_EXCEEDED_DROP),
      0, "There should not be any dropped packets");
  NS_TEST_EXPECT_MSG_EQ(
      q0->GetStats().GetNMarkedPackets(CoDelQueueDisc::TARGET_EXCEEDED_MARK), 0,
      "There should not be any marked packets");
  NS_TEST_EXPECT_MSG_EQ(
      q1->GetStats().GetNMarkedPackets(CoDelQueueDisc::TARGET_EXCEEDED_MARK), 1,
      "There should be 1 marked packets");
  NS_TEST_EXPECT_MSG_EQ(
      q1->GetStats().GetNDroppedPackets(CoDelQueueDisc::TARGET_EXCEEDED_DROP),
      0, "There should not be any dropped packets");

  Simulator::Destroy();

  queueDisc = CreateObjectWithAttributes<FqCoDelQueueDisc>(
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
  Simulator::Schedule(Time(Seconds(0.0005)),
                      &FqCoDelQueueDiscL4sMode::AddPacket, this, queueDisc, hdr,
                      1);
  Simulator::Schedule(Time(Seconds(0.0005)),
                      &FqCoDelQueueDiscL4sMode::AddPacketWithDelay, this,
                      queueDisc, hdr, delay, 69);

  hdr.SetEcn(Ipv4Header::ECN_ECT0);
  Simulator::Schedule(Time(Seconds(0)),
                      &FqCoDelQueueDiscL4sMode::AddPacketWithDelay, this,
                      queueDisc, hdr, delay, 70);

  DequeueWithDelay(queueDisc, delay, 140);
  Simulator::Run();
  Simulator::Stop(Seconds(8.0));
  q0 = queueDisc->GetQueueDiscClass(0)
           ->GetQueueDisc()
           ->GetObject<CoDelQueueDisc>();

  NS_TEST_EXPECT_MSG_EQ(q0->GetStats().GetNMarkedPackets(
                            CoDelQueueDisc::CE_THRESHOLD_EXCEEDED_MARK),
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
      q0->GetStats().GetNDroppedPackets(CoDelQueueDisc::TARGET_EXCEEDED_DROP),
      0, "There should not be any dropped packets");
  NS_TEST_EXPECT_MSG_EQ(
      q0->GetStats().GetNMarkedPackets(CoDelQueueDisc::TARGET_EXCEEDED_MARK), 1,
      "There should be 1 marked packets");

  Simulator::Destroy();
}

class FqCoDelQueueDiscTestSuite : public TestSuite {
public:
  FqCoDelQueueDiscTestSuite();
};

FqCoDelQueueDiscTestSuite::FqCoDelQueueDiscTestSuite()
    : TestSuite("fq-codel-queue-disc", UNIT) {
  AddTestCase(new FqCoDelQueueDiscNoSuitableFilter, TestCase::QUICK);
  AddTestCase(new FqCoDelQueueDiscIPFlowsSeparationAndPacketLimit,
              TestCase::QUICK);
  AddTestCase(new FqCoDelQueueDiscDeficit, TestCase::QUICK);
  AddTestCase(new FqCoDelQueueDiscTCPFlowsSeparation, TestCase::QUICK);
  AddTestCase(new FqCoDelQueueDiscUDPFlowsSeparation, TestCase::QUICK);
  AddTestCase(new FqCoDelQueueDiscECNMarking, TestCase::QUICK);
  AddTestCase(new FqCoDelQueueDiscSetLinearProbing, TestCase::QUICK);
  AddTestCase(new FqCoDelQueueDiscL4sMode, TestCase::QUICK);
}

static FqCoDelQueueDiscTestSuite g_fqCoDelQueueDiscTestSuite;
