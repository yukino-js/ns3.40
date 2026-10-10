
#include "ns3/cobalt-queue-disc.h"
#include "ns3/fq-cobalt-queue-disc.h"
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

class Ipv4FqCobaltTestPacketFilter : public Ipv4PacketFilter {
public:
  static TypeId GetTypeId();

  Ipv4FqCobaltTestPacketFilter();
  ~Ipv4FqCobaltTestPacketFilter() override;

private:
  int32_t DoClassify(Ptr<QueueDiscItem> item) const override;

  bool CheckProtocol(Ptr<QueueDiscItem> item) const override;
};

TypeId Ipv4FqCobaltTestPacketFilter::GetTypeId() {
  static TypeId tid = TypeId("ns3::Ipv4FqCobaltTestPacketFilter")
                          .SetParent<Ipv4PacketFilter>()
                          .SetGroupName("Internet")
                          .AddConstructor<Ipv4FqCobaltTestPacketFilter>();
  return tid;
}

Ipv4FqCobaltTestPacketFilter::Ipv4FqCobaltTestPacketFilter() {}

Ipv4FqCobaltTestPacketFilter::~Ipv4FqCobaltTestPacketFilter() {}

int32_t
Ipv4FqCobaltTestPacketFilter::DoClassify(Ptr<QueueDiscItem> item) const {
  return g_hash;
}

bool Ipv4FqCobaltTestPacketFilter::CheckProtocol(
    Ptr<QueueDiscItem> item) const {
  return true;
}

class FqCobaltQueueDiscNoSuitableFilter : public TestCase {
public:
  FqCobaltQueueDiscNoSuitableFilter();
  ~FqCobaltQueueDiscNoSuitableFilter() override;

private:
  void DoRun() override;
};

FqCobaltQueueDiscNoSuitableFilter::FqCobaltQueueDiscNoSuitableFilter()
    : TestCase("Test packets that are not classified by any filter") {}

FqCobaltQueueDiscNoSuitableFilter::~FqCobaltQueueDiscNoSuitableFilter() {}

void FqCobaltQueueDiscNoSuitableFilter::DoRun() {
  Ptr<FqCobaltQueueDisc> queueDisc =
      CreateObjectWithAttributes<FqCobaltQueueDisc>("MaxSize",
                                                    StringValue("4p"));
  Ptr<Ipv4FqCobaltTestPacketFilter> filter =
      CreateObject<Ipv4FqCobaltTestPacketFilter>();
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

class FqCobaltQueueDiscIPFlowsSeparationAndPacketLimit : public TestCase {
public:
  FqCobaltQueueDiscIPFlowsSeparationAndPacketLimit();
  ~FqCobaltQueueDiscIPFlowsSeparationAndPacketLimit() override;

private:
  void DoRun() override;
  void AddPacket(Ptr<FqCobaltQueueDisc> queue, Ipv4Header hdr);
};

FqCobaltQueueDiscIPFlowsSeparationAndPacketLimit::
    FqCobaltQueueDiscIPFlowsSeparationAndPacketLimit()
    : TestCase("Test IP flows separation and packet limit") {}

FqCobaltQueueDiscIPFlowsSeparationAndPacketLimit::
    ~FqCobaltQueueDiscIPFlowsSeparationAndPacketLimit() {}

void FqCobaltQueueDiscIPFlowsSeparationAndPacketLimit::AddPacket(
    Ptr<FqCobaltQueueDisc> queue, Ipv4Header hdr) {
  Ptr<Packet> p = Create<Packet>(100);
  Address dest;
  Ptr<Ipv4QueueDiscItem> item = Create<Ipv4QueueDiscItem>(p, dest, 0, hdr);
  queue->Enqueue(item);
}

void FqCobaltQueueDiscIPFlowsSeparationAndPacketLimit::DoRun() {
  Ptr<FqCobaltQueueDisc> queueDisc =
      CreateObjectWithAttributes<FqCobaltQueueDisc>("MaxSize",
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

class FqCobaltQueueDiscDeficit : public TestCase {
public:
  FqCobaltQueueDiscDeficit();
  ~FqCobaltQueueDiscDeficit() override;

private:
  void DoRun() override;
  void AddPacket(Ptr<FqCobaltQueueDisc> queue, Ipv4Header hdr);
};

FqCobaltQueueDiscDeficit::FqCobaltQueueDiscDeficit()
    : TestCase("Test credits and flows status") {}

FqCobaltQueueDiscDeficit::~FqCobaltQueueDiscDeficit() {}

void FqCobaltQueueDiscDeficit::AddPacket(Ptr<FqCobaltQueueDisc> queue,
                                         Ipv4Header hdr) {
  Ptr<Packet> p = Create<Packet>(100);
  Address dest;
  Ptr<Ipv4QueueDiscItem> item = Create<Ipv4QueueDiscItem>(p, dest, 0, hdr);
  queue->Enqueue(item);
}

void FqCobaltQueueDiscDeficit::DoRun() {
  Ptr<FqCobaltQueueDisc> queueDisc =
      CreateObjectWithAttributes<FqCobaltQueueDisc>();

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
  Ptr<FqCobaltFlow> flow1 =
      StaticCast<FqCobaltFlow>(queueDisc->GetQueueDiscClass(0));
  NS_TEST_ASSERT_MSG_EQ(flow1->GetDeficit(),
                        static_cast<int32_t>(queueDisc->GetQuantum()),
                        "the deficit of the first flow must equal the quantum");
  NS_TEST_ASSERT_MSG_EQ(flow1->GetStatus(), FqCobaltFlow::NEW_FLOW,
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
      flow1->GetStatus(), FqCobaltFlow::NEW_FLOW,
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
  Ptr<FqCobaltFlow> flow2 =
      StaticCast<FqCobaltFlow>(queueDisc->GetQueueDiscClass(1));
  NS_TEST_ASSERT_MSG_EQ(
      flow2->GetDeficit(), static_cast<int32_t>(queueDisc->GetQuantum()),
      "the deficit of the second flow must equal the quantum");
  NS_TEST_ASSERT_MSG_EQ(flow2->GetStatus(), FqCobaltFlow::NEW_FLOW,
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
  NS_TEST_ASSERT_MSG_EQ(flow1->GetStatus(), FqCobaltFlow::OLD_FLOW,
                        "the first flow must be in the list of old queues");
  NS_TEST_ASSERT_MSG_EQ(flow2->GetDeficit(), -30,
                        "unexpected deficit for the second flow");
  NS_TEST_ASSERT_MSG_EQ(flow2->GetStatus(), FqCobaltFlow::NEW_FLOW,
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
  NS_TEST_ASSERT_MSG_EQ(flow1->GetStatus(), FqCobaltFlow::OLD_FLOW,
                        "the first flow must be in the list of old queues");
  NS_TEST_ASSERT_MSG_EQ(flow2->GetDeficit(), 60,
                        "unexpected deficit for the second flow");
  NS_TEST_ASSERT_MSG_EQ(flow2->GetStatus(), FqCobaltFlow::OLD_FLOW,
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
  NS_TEST_ASSERT_MSG_EQ(flow1->GetStatus(), FqCobaltFlow::OLD_FLOW,
                        "the first flow must be in the list of old queues");
  NS_TEST_ASSERT_MSG_EQ(flow2->GetDeficit(), -60,
                        "unexpected deficit for the second flow");
  NS_TEST_ASSERT_MSG_EQ(flow2->GetStatus(), FqCobaltFlow::OLD_FLOW,
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
  NS_TEST_ASSERT_MSG_EQ(flow1->GetStatus(), FqCobaltFlow::OLD_FLOW,
                        "the first flow must be in the list of old queues");
  NS_TEST_ASSERT_MSG_EQ(flow2->GetDeficit(), 30,
                        "unexpected deficit for the second flow");
  NS_TEST_ASSERT_MSG_EQ(flow2->GetStatus(), FqCobaltFlow::OLD_FLOW,
                        "the second flow must be in the list of new queues");

  queueDisc->Dequeue();
  NS_TEST_ASSERT_MSG_EQ(flow1->GetDeficit(), 90,
                        "unexpected deficit for the first flow");
  NS_TEST_ASSERT_MSG_EQ(flow1->GetStatus(), FqCobaltFlow::INACTIVE,
                        "the first flow must be inactive");
  NS_TEST_ASSERT_MSG_EQ(flow2->GetDeficit(), 30,
                        "unexpected deficit for the second flow");
  NS_TEST_ASSERT_MSG_EQ(flow2->GetStatus(), FqCobaltFlow::INACTIVE,
                        "the second flow must be inactive");

  Simulator::Destroy();
}

class FqCobaltQueueDiscTCPFlowsSeparation : public TestCase {
public:
  FqCobaltQueueDiscTCPFlowsSeparation();
  ~FqCobaltQueueDiscTCPFlowsSeparation() override;

private:
  void DoRun() override;
  void AddPacket(Ptr<FqCobaltQueueDisc> queue, Ipv4Header ipHdr,
                 TcpHeader tcpHdr);
};

FqCobaltQueueDiscTCPFlowsSeparation::FqCobaltQueueDiscTCPFlowsSeparation()
    : TestCase("Test TCP flows separation") {}

FqCobaltQueueDiscTCPFlowsSeparation::~FqCobaltQueueDiscTCPFlowsSeparation() {}

void FqCobaltQueueDiscTCPFlowsSeparation::AddPacket(
    Ptr<FqCobaltQueueDisc> queue, Ipv4Header ipHdr, TcpHeader tcpHdr) {
  Ptr<Packet> p = Create<Packet>(100);
  p->AddHeader(tcpHdr);
  Address dest;
  Ptr<Ipv4QueueDiscItem> item = Create<Ipv4QueueDiscItem>(p, dest, 0, ipHdr);
  queue->Enqueue(item);
}

void FqCobaltQueueDiscTCPFlowsSeparation::DoRun() {
  Ptr<FqCobaltQueueDisc> queueDisc =
      CreateObjectWithAttributes<FqCobaltQueueDisc>("MaxSize",
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

class FqCobaltQueueDiscUDPFlowsSeparation : public TestCase {
public:
  FqCobaltQueueDiscUDPFlowsSeparation();
  ~FqCobaltQueueDiscUDPFlowsSeparation() override;

private:
  void DoRun() override;
  void AddPacket(Ptr<FqCobaltQueueDisc> queue, Ipv4Header ipHdr,
                 UdpHeader udpHdr);
};

FqCobaltQueueDiscUDPFlowsSeparation::FqCobaltQueueDiscUDPFlowsSeparation()
    : TestCase("Test UDP flows separation") {}

FqCobaltQueueDiscUDPFlowsSeparation::~FqCobaltQueueDiscUDPFlowsSeparation() {}

void FqCobaltQueueDiscUDPFlowsSeparation::AddPacket(
    Ptr<FqCobaltQueueDisc> queue, Ipv4Header ipHdr, UdpHeader udpHdr) {
  Ptr<Packet> p = Create<Packet>(100);
  p->AddHeader(udpHdr);
  Address dest;
  Ptr<Ipv4QueueDiscItem> item = Create<Ipv4QueueDiscItem>(p, dest, 0, ipHdr);
  queue->Enqueue(item);
}

void FqCobaltQueueDiscUDPFlowsSeparation::DoRun() {
  Ptr<FqCobaltQueueDisc> queueDisc =
      CreateObjectWithAttributes<FqCobaltQueueDisc>("MaxSize",
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

class FqCobaltQueueDiscEcnMarking : public TestCase {
public:
  FqCobaltQueueDiscEcnMarking();
  ~FqCobaltQueueDiscEcnMarking() override;

private:
  void DoRun() override;
  void AddPacket(Ptr<FqCobaltQueueDisc> queue, Ipv4Header hdr, uint32_t nPkt,
                 uint32_t nPktEnqueued, uint32_t nQueueFlows);
  void Dequeue(Ptr<FqCobaltQueueDisc> queue, uint32_t nPkt);
  void DequeueWithDelay(Ptr<FqCobaltQueueDisc> queue, double delay,
                        uint32_t nPkt);
  void DropNextTracer(int64_t oldVal, int64_t newVal);
  uint32_t m_dropNextCount;
};

FqCobaltQueueDiscEcnMarking::FqCobaltQueueDiscEcnMarking()
    : TestCase("Test ECN marking") {
  m_dropNextCount = 0;
}

FqCobaltQueueDiscEcnMarking::~FqCobaltQueueDiscEcnMarking() {}

void FqCobaltQueueDiscEcnMarking::AddPacket(Ptr<FqCobaltQueueDisc> queue,
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

void FqCobaltQueueDiscEcnMarking::Dequeue(Ptr<FqCobaltQueueDisc> queue,
                                          uint32_t nPkt) {
  Ptr<CobaltQueueDisc> q3 =
      queue->GetQueueDiscClass(3)->GetQueueDisc()->GetObject<CobaltQueueDisc>();

  if (q3->GetNPackets() == 19) {
    q3->TraceConnectWithoutContext(
        "DropNext",
        MakeCallback(&FqCobaltQueueDiscEcnMarking::DropNextTracer, this));
  }

  for (uint32_t i = 0; i < nPkt; i++) {
    Ptr<QueueDiscItem> item = queue->Dequeue();
  }
}

void FqCobaltQueueDiscEcnMarking::DequeueWithDelay(Ptr<FqCobaltQueueDisc> queue,
                                                   double delay,
                                                   uint32_t nPkt) {
  for (uint32_t i = 0; i < nPkt; i++) {
    Simulator::Schedule(Time(Seconds((i + 1) * delay)),
                        &FqCobaltQueueDiscEcnMarking::Dequeue, this, queue, 1);
  }
}

void FqCobaltQueueDiscEcnMarking::DropNextTracer(int64_t, int64_t) {
  m_dropNextCount++;
}

void FqCobaltQueueDiscEcnMarking::DoRun() {

  Ptr<FqCobaltQueueDisc> queueDisc =
      CreateObjectWithAttributes<FqCobaltQueueDisc>(
          "MaxSize", StringValue("10240p"), "UseEcn", BooleanValue(true),
          "Perturbation", UintegerValue(0), "BlueThreshold",
          TimeValue(Time::Max()));

  queueDisc->SetQuantum(1514);
  queueDisc->Initialize();
  Ipv4Header hdr;
  hdr.SetPayloadSize(100);
  hdr.SetSource(Ipv4Address("10.10.1.1"));
  hdr.SetDestination(Ipv4Address("10.10.1.2"));
  hdr.SetProtocol(7);
  hdr.SetEcn(Ipv4Header::ECN_ECT0);

  Simulator::Schedule(Time(Seconds(0)), &FqCobaltQueueDiscEcnMarking::AddPacket,
                      this, queueDisc, hdr, 20, 20, 1);

  hdr.SetDestination(Ipv4Address("10.10.1.10"));
  Simulator::Schedule(Time(Seconds(0)), &FqCobaltQueueDiscEcnMarking::AddPacket,
                      this, queueDisc, hdr, 20, 40, 2);

  hdr.SetDestination(Ipv4Address("10.10.1.20"));
  Simulator::Schedule(Time(Seconds(0)), &FqCobaltQueueDiscEcnMarking::AddPacket,
                      this, queueDisc, hdr, 20, 60, 3);

  hdr.SetDestination(Ipv4Address("10.10.1.30"));
  hdr.SetEcn(Ipv4Header::ECN_NotECT);
  Simulator::Schedule(Time(Seconds(0)), &FqCobaltQueueDiscEcnMarking::AddPacket,
                      this, queueDisc, hdr, 20, 80, 4);

  hdr.SetDestination(Ipv4Address("10.10.1.40"));
  Simulator::Schedule(Time(Seconds(0)), &FqCobaltQueueDiscEcnMarking::AddPacket,
                      this, queueDisc, hdr, 20, 100, 5);

  DequeueWithDelay(queueDisc, 0.11, 60);
  Simulator::Run();
  Simulator::Stop(Seconds(8.0));
  Ptr<CobaltQueueDisc> q0 = queueDisc->GetQueueDiscClass(0)
                                ->GetQueueDisc()
                                ->GetObject<CobaltQueueDisc>();
  Ptr<CobaltQueueDisc> q1 = queueDisc->GetQueueDiscClass(1)
                                ->GetQueueDisc()
                                ->GetObject<CobaltQueueDisc>();
  Ptr<CobaltQueueDisc> q2 = queueDisc->GetQueueDiscClass(2)
                                ->GetQueueDisc()
                                ->GetObject<CobaltQueueDisc>();
  Ptr<CobaltQueueDisc> q3 = queueDisc->GetQueueDiscClass(3)
                                ->GetQueueDisc()
                                ->GetObject<CobaltQueueDisc>();
  Ptr<CobaltQueueDisc> q4 = queueDisc->GetQueueDiscClass(4)
                                ->GetQueueDisc()
                                ->GetObject<CobaltQueueDisc>();

  NS_TEST_EXPECT_MSG_EQ(
      q0->GetStats().GetNMarkedPackets(CobaltQueueDisc::FORCED_MARK), 19,
      "There should be 19 marked packets."
      "As there is no CoDel minBytes parameter so all the packets apart from "
      "the first one gets marked. As q3 and q4 have"
      "NotEct packets and the queue delay is much higher than 5ms so the queue "
      "gets empty pretty quickly so more"
      "packets from q0 can be dequeued.");
  NS_TEST_EXPECT_MSG_EQ(
      q0->GetStats().GetNDroppedPackets(CobaltQueueDisc::TARGET_EXCEEDED_DROP),
      0, "There should not be any dropped packets");
  NS_TEST_EXPECT_MSG_EQ(
      q1->GetStats().GetNMarkedPackets(CobaltQueueDisc::FORCED_MARK), 16,
      "There should be 16 marked packets"
      "As there is no CoDel minBytes parameter so all the packets apart from "
      "the first one until no more packets are dequeued"
      "are marked.");
  NS_TEST_EXPECT_MSG_EQ(
      q1->GetStats().GetNDroppedPackets(CobaltQueueDisc::TARGET_EXCEEDED_DROP),
      0, "There should not be any dropped packets");
  NS_TEST_EXPECT_MSG_EQ(
      q2->GetStats().GetNMarkedPackets(CobaltQueueDisc::FORCED_MARK), 12,
      "There should be 12 marked packets"
      "Each packet size is 120 bytes and the quantum is 1500 bytes so in the "
      "first turn (1514/120 = 12.61) 13 packets are"
      "dequeued and apart from the first one, all the packets are marked.");
  NS_TEST_EXPECT_MSG_EQ(
      q2->GetStats().GetNDroppedPackets(CobaltQueueDisc::TARGET_EXCEEDED_DROP),
      0, "There should not be any dropped packets");

  NS_TEST_EXPECT_MSG_EQ(
      q3->GetStats().GetNDroppedPackets(CobaltQueueDisc::TARGET_EXCEEDED_DROP),
      m_dropNextCount,
      "The number of drops should"
      "be equal to the number of times m_dropNext is updated");
  NS_TEST_EXPECT_MSG_EQ(
      q3->GetStats().GetNMarkedPackets(CobaltQueueDisc::FORCED_MARK), 0,
      "There should not be any marked packets");
  NS_TEST_EXPECT_MSG_EQ(
      q4->GetStats().GetNDroppedPackets(CobaltQueueDisc::TARGET_EXCEEDED_DROP),
      m_dropNextCount,
      "The number of drops should"
      "be equal to the number of times m_dropNext is updated");
  NS_TEST_EXPECT_MSG_EQ(
      q4->GetStats().GetNMarkedPackets(CobaltQueueDisc::FORCED_MARK), 0,
      "There should not be any marked packets");

  Simulator::Destroy();

  queueDisc = CreateObjectWithAttributes<FqCobaltQueueDisc>(
      "MaxSize", StringValue("10240p"), "UseEcn", BooleanValue(true),
      "CeThreshold", TimeValue(MilliSeconds(2)));
  queueDisc->SetQuantum(1514);
  queueDisc->Initialize();

  hdr.SetDestination(Ipv4Address("10.10.1.2"));
  hdr.SetEcn(Ipv4Header::ECN_ECT0);
  Simulator::Schedule(Time(Seconds(0)), &FqCobaltQueueDiscEcnMarking::AddPacket,
                      this, queueDisc, hdr, 20, 20, 1);

  hdr.SetDestination(Ipv4Address("10.10.1.10"));
  Simulator::Schedule(Time(Seconds(0)), &FqCobaltQueueDiscEcnMarking::AddPacket,
                      this, queueDisc, hdr, 20, 40, 2);

  hdr.SetDestination(Ipv4Address("10.10.1.20"));
  Simulator::Schedule(Time(Seconds(0)), &FqCobaltQueueDiscEcnMarking::AddPacket,
                      this, queueDisc, hdr, 20, 60, 3);

  hdr.SetDestination(Ipv4Address("10.10.1.30"));
  hdr.SetEcn(Ipv4Header::ECN_NotECT);
  Simulator::Schedule(Time(Seconds(0)), &FqCobaltQueueDiscEcnMarking::AddPacket,
                      this, queueDisc, hdr, 20, 80, 4);

  hdr.SetDestination(Ipv4Address("10.10.1.40"));
  Simulator::Schedule(Time(Seconds(0)), &FqCobaltQueueDiscEcnMarking::AddPacket,
                      this, queueDisc, hdr, 20, 100, 5);

  DequeueWithDelay(queueDisc, 0.0001, 60);
  Simulator::Run();
  Simulator::Stop(Seconds(8.0));
  q0 = queueDisc->GetQueueDiscClass(0)
           ->GetQueueDisc()
           ->GetObject<CobaltQueueDisc>();
  q1 = queueDisc->GetQueueDiscClass(1)
           ->GetQueueDisc()
           ->GetObject<CobaltQueueDisc>();
  q2 = queueDisc->GetQueueDiscClass(2)
           ->GetQueueDisc()
           ->GetObject<CobaltQueueDisc>();
  q3 = queueDisc->GetQueueDiscClass(3)
           ->GetQueueDisc()
           ->GetObject<CobaltQueueDisc>();
  q4 = queueDisc->GetQueueDiscClass(4)
           ->GetQueueDisc()
           ->GetObject<CobaltQueueDisc>();

  NS_TEST_EXPECT_MSG_EQ(
      q0->GetStats().GetNDroppedPackets(CobaltQueueDisc::TARGET_EXCEEDED_DROP),
      0, "There should not be any dropped packets");
  NS_TEST_EXPECT_MSG_EQ(q0->GetStats().GetNMarkedPackets(
                            CobaltQueueDisc::CE_THRESHOLD_EXCEEDED_MARK),
                        0,
                        "There should not be any marked packets"
                        "with quantum of 1514, 13 packets of size 120 bytes "
                        "can be dequeued. sojourn time of 13th "
                        "packet is 1.3ms which is"
                        "less than CE threshold");
  NS_TEST_EXPECT_MSG_EQ(
      q1->GetStats().GetNDroppedPackets(CobaltQueueDisc::TARGET_EXCEEDED_DROP),
      0, "There should not be any dropped packets");
  NS_TEST_EXPECT_MSG_EQ(q1->GetStats().GetNMarkedPackets(
                            CobaltQueueDisc::CE_THRESHOLD_EXCEEDED_MARK),
                        6,
                        "There should be 6 marked packets"
                        "with quantum of 1514, 13 packets of size 120 bytes "
                        "can be dequeued. sojourn time of 8th "
                        "packet is 2.1ms which is greater"
                        "than CE threshold and subsequent packet also have "
                        "sojourn time more 8th packet hence "
                        "remaining packet are marked.");
  NS_TEST_EXPECT_MSG_EQ(
      q2->GetStats().GetNDroppedPackets(CobaltQueueDisc::TARGET_EXCEEDED_DROP),
      0, "There should not be any dropped packets");
  NS_TEST_EXPECT_MSG_EQ(q2->GetStats().GetNMarkedPackets(
                            CobaltQueueDisc::CE_THRESHOLD_EXCEEDED_MARK),
                        13,
                        "There should be 13 marked packets"
                        "with quantum of 1514, 13 packets of size 120 bytes "
                        "can be dequeued and all of them have "
                        "sojourn time more than CE threshold");

  NS_TEST_EXPECT_MSG_EQ(q3->GetStats().GetNMarkedPackets(
                            CobaltQueueDisc::CE_THRESHOLD_EXCEEDED_MARK),
                        0, "There should not be any marked packets");
  NS_TEST_EXPECT_MSG_EQ(
      q3->GetStats().GetNDroppedPackets(CobaltQueueDisc::TARGET_EXCEEDED_DROP),
      0, "There should not be any dropped packets");
  NS_TEST_EXPECT_MSG_EQ(q4->GetStats().GetNMarkedPackets(
                            CobaltQueueDisc::CE_THRESHOLD_EXCEEDED_MARK),
                        0, "There should not be any marked packets");
  NS_TEST_EXPECT_MSG_EQ(
      q4->GetStats().GetNDroppedPackets(CobaltQueueDisc::TARGET_EXCEEDED_DROP),
      1,
      "There should 1 dropped packet. As the queue"
      "delay for the first dequeue is greater than the target (5ms), Cobalt "
      "overloads the m_dropNext field as an activity timeout"
      "and dropNext is to set to the current Time value so on the next dequeue "
      "a packet is dropped.");

  Simulator::Destroy();

  queueDisc = CreateObjectWithAttributes<FqCobaltQueueDisc>(
      "MaxSize", StringValue("10240p"), "UseEcn", BooleanValue(true),
      "CeThreshold", TimeValue(MilliSeconds(2)), "BlueThreshold",
      TimeValue(Time::Max()));
  queueDisc->SetQuantum(1514);
  queueDisc->Initialize();

  hdr.SetDestination(Ipv4Address("10.10.1.2"));
  hdr.SetEcn(Ipv4Header::ECN_ECT0);
  Simulator::Schedule(Time(Seconds(0)), &FqCobaltQueueDiscEcnMarking::AddPacket,
                      this, queueDisc, hdr, 20, 20, 1);

  hdr.SetDestination(Ipv4Address("10.10.1.10"));
  Simulator::Schedule(Time(Seconds(0)), &FqCobaltQueueDiscEcnMarking::AddPacket,
                      this, queueDisc, hdr, 20, 40, 2);

  hdr.SetDestination(Ipv4Address("10.10.1.20"));
  Simulator::Schedule(Time(Seconds(0)), &FqCobaltQueueDiscEcnMarking::AddPacket,
                      this, queueDisc, hdr, 20, 60, 3);

  hdr.SetDestination(Ipv4Address("10.10.1.30"));
  hdr.SetEcn(Ipv4Header::ECN_NotECT);
  Simulator::Schedule(Time(Seconds(0)), &FqCobaltQueueDiscEcnMarking::AddPacket,
                      this, queueDisc, hdr, 20, 80, 4);

  hdr.SetDestination(Ipv4Address("10.10.1.40"));
  Simulator::Schedule(Time(Seconds(0)), &FqCobaltQueueDiscEcnMarking::AddPacket,
                      this, queueDisc, hdr, 20, 100, 5);

  m_dropNextCount = 0;

  DequeueWithDelay(queueDisc, 0.110, 60);
  Simulator::Run();
  Simulator::Stop(Seconds(8.0));
  q0 = queueDisc->GetQueueDiscClass(0)
           ->GetQueueDisc()
           ->GetObject<CobaltQueueDisc>();
  q1 = queueDisc->GetQueueDiscClass(1)
           ->GetQueueDisc()
           ->GetObject<CobaltQueueDisc>();
  q2 = queueDisc->GetQueueDiscClass(2)
           ->GetQueueDisc()
           ->GetObject<CobaltQueueDisc>();
  q3 = queueDisc->GetQueueDiscClass(3)
           ->GetQueueDisc()
           ->GetObject<CobaltQueueDisc>();
  q4 = queueDisc->GetQueueDiscClass(4)
           ->GetQueueDisc()
           ->GetObject<CobaltQueueDisc>();

  NS_TEST_EXPECT_MSG_EQ(
      q0->GetStats().GetNDroppedPackets(CobaltQueueDisc::TARGET_EXCEEDED_DROP),
      0, "There should not be any dropped packets");
  NS_TEST_EXPECT_MSG_EQ(
      q0->GetStats().GetNMarkedPackets(
          CobaltQueueDisc::CE_THRESHOLD_EXCEEDED_MARK) +
          q0->GetStats().GetNMarkedPackets(CobaltQueueDisc::FORCED_MARK),
      20 - q0->GetNPackets(),
      "Number of CE threshold"
      " exceeded marks plus Number of Target exceeded marks should be equal to "
      "total number of "
      "packets dequeued");
  NS_TEST_EXPECT_MSG_EQ(
      q1->GetStats().GetNDroppedPackets(CobaltQueueDisc::TARGET_EXCEEDED_DROP),
      0, "There should not be any dropped packets");
  NS_TEST_EXPECT_MSG_EQ(
      q1->GetStats().GetNMarkedPackets(
          CobaltQueueDisc::CE_THRESHOLD_EXCEEDED_MARK) +
          q1->GetStats().GetNMarkedPackets(CobaltQueueDisc::FORCED_MARK),
      20 - q1->GetNPackets(),
      "Number of CE threshold"
      " exceeded marks plus Number of Target exceeded marks should be equal to "
      "total number of "
      "packets dequeued");
  NS_TEST_EXPECT_MSG_EQ(
      q2->GetStats().GetNDroppedPackets(CobaltQueueDisc::TARGET_EXCEEDED_DROP),
      0, "There should not be any dropped packets");
  NS_TEST_EXPECT_MSG_EQ(
      q2->GetStats().GetNMarkedPackets(
          CobaltQueueDisc::CE_THRESHOLD_EXCEEDED_MARK) +
          q2->GetStats().GetNMarkedPackets(CobaltQueueDisc::FORCED_MARK),
      20 - q2->GetNPackets(),
      "Number of CE threshold"
      " exceeded marks plus Number of Target exceeded marks should be equal to "
      "total number of "
      "packets dequeued");

  NS_TEST_EXPECT_MSG_EQ(q3->GetStats().GetNMarkedPackets(
                            CobaltQueueDisc::CE_THRESHOLD_EXCEEDED_MARK),
                        0, "There should not be any marked packets");
  NS_TEST_EXPECT_MSG_EQ(
      q3->GetStats().GetNDroppedPackets(CobaltQueueDisc::TARGET_EXCEEDED_DROP),
      m_dropNextCount,
      "The number of drops should"
      "be equal to the number of times m_dropNext is updated");
  NS_TEST_EXPECT_MSG_EQ(q4->GetStats().GetNMarkedPackets(
                            CobaltQueueDisc::CE_THRESHOLD_EXCEEDED_MARK),
                        0, "There should not be any marked packets");
  NS_TEST_EXPECT_MSG_EQ(
      q4->GetStats().GetNDroppedPackets(CobaltQueueDisc::TARGET_EXCEEDED_DROP),
      m_dropNextCount,
      "The number of drops should"
      "be equal to the number of times m_dropNext is updated");

  Simulator::Destroy();
}

class FqCobaltQueueDiscSetLinearProbing : public TestCase {
public:
  FqCobaltQueueDiscSetLinearProbing();
  ~FqCobaltQueueDiscSetLinearProbing() override;

private:
  void DoRun() override;
  void AddPacket(Ptr<FqCobaltQueueDisc> queue, Ipv4Header hdr);
};

FqCobaltQueueDiscSetLinearProbing::FqCobaltQueueDiscSetLinearProbing()
    : TestCase("Test credits and flows status") {}

FqCobaltQueueDiscSetLinearProbing::~FqCobaltQueueDiscSetLinearProbing() {}

void FqCobaltQueueDiscSetLinearProbing::AddPacket(Ptr<FqCobaltQueueDisc> queue,
                                                  Ipv4Header hdr) {
  Ptr<Packet> p = Create<Packet>(100);
  Address dest;
  Ptr<Ipv4QueueDiscItem> item = Create<Ipv4QueueDiscItem>(p, dest, 0, hdr);
  queue->Enqueue(item);
}

void FqCobaltQueueDiscSetLinearProbing::DoRun() {
  Ptr<FqCobaltQueueDisc> queueDisc =
      CreateObjectWithAttributes<FqCobaltQueueDisc>("EnableSetAssociativeHash",
                                                    BooleanValue(true));
  queueDisc->SetQuantum(90);
  queueDisc->Initialize();

  Ptr<Ipv4FqCobaltTestPacketFilter> filter =
      CreateObject<Ipv4FqCobaltTestPacketFilter>();
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

class FqCobaltQueueDiscL4sMode : public TestCase {
public:
  FqCobaltQueueDiscL4sMode();
  ~FqCobaltQueueDiscL4sMode() override;

private:
  void DoRun() override;
  void AddPacket(Ptr<FqCobaltQueueDisc> queue, Ipv4Header hdr, uint32_t nPkt);
  void AddPacketWithDelay(Ptr<FqCobaltQueueDisc> queue, Ipv4Header hdr,
                          double delay, uint32_t nPkt);
  void Dequeue(Ptr<FqCobaltQueueDisc> queue, uint32_t nPkt);
  void DequeueWithDelay(Ptr<FqCobaltQueueDisc> queue, double delay,
                        uint32_t nPkt);
};

FqCobaltQueueDiscL4sMode::FqCobaltQueueDiscL4sMode()
    : TestCase("Test L4S mode") {}

FqCobaltQueueDiscL4sMode::~FqCobaltQueueDiscL4sMode() {}

void FqCobaltQueueDiscL4sMode::AddPacket(Ptr<FqCobaltQueueDisc> queue,
                                         Ipv4Header hdr, uint32_t nPkt) {
  Address dest;
  Ptr<Packet> p = Create<Packet>(100);
  for (uint32_t i = 0; i < nPkt; i++) {
    Ptr<Ipv4QueueDiscItem> item = Create<Ipv4QueueDiscItem>(p, dest, 0, hdr);
    queue->Enqueue(item);
  }
}

void FqCobaltQueueDiscL4sMode::AddPacketWithDelay(Ptr<FqCobaltQueueDisc> queue,
                                                  Ipv4Header hdr, double delay,
                                                  uint32_t nPkt) {
  for (uint32_t i = 0; i < nPkt; i++) {
    Simulator::Schedule(Time(Seconds((i + 1) * delay)),
                        &FqCobaltQueueDiscL4sMode::AddPacket, this, queue, hdr,
                        1);
  }
}

void FqCobaltQueueDiscL4sMode::Dequeue(Ptr<FqCobaltQueueDisc> queue,
                                       uint32_t nPkt) {
  for (uint32_t i = 0; i < nPkt; i++) {
    Ptr<QueueDiscItem> item = queue->Dequeue();
  }
}

void FqCobaltQueueDiscL4sMode::DequeueWithDelay(Ptr<FqCobaltQueueDisc> queue,
                                                double delay, uint32_t nPkt) {
  for (uint32_t i = 0; i < nPkt; i++) {
    Simulator::Schedule(Time(Seconds((i + 1) * delay)),
                        &FqCobaltQueueDiscL4sMode::Dequeue, this, queue, 1);
  }
}

void FqCobaltQueueDiscL4sMode::DoRun() {

  Ptr<FqCobaltQueueDisc> queueDisc =
      CreateObjectWithAttributes<FqCobaltQueueDisc>(
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
                      &FqCobaltQueueDiscL4sMode::AddPacketWithDelay, this,
                      queueDisc, hdr, delay, 70);

  hdr.SetEcn(Ipv4Header::ECN_ECT0);
  hdr.SetDestination(Ipv4Address("10.10.1.10"));
  Simulator::Schedule(Time(Seconds(0)),
                      &FqCobaltQueueDiscL4sMode::AddPacketWithDelay, this,
                      queueDisc, hdr, delay, 70);

  delay = 0.001;
  DequeueWithDelay(queueDisc, delay, 140);
  Simulator::Run();
  Simulator::Stop(Seconds(8.0));
  Ptr<CobaltQueueDisc> q0 = queueDisc->GetQueueDiscClass(0)
                                ->GetQueueDisc()
                                ->GetObject<CobaltQueueDisc>();
  Ptr<CobaltQueueDisc> q1 = queueDisc->GetQueueDiscClass(1)
                                ->GetQueueDisc()
                                ->GetObject<CobaltQueueDisc>();

  NS_TEST_EXPECT_MSG_EQ(q0->GetStats().GetNMarkedPackets(
                            CobaltQueueDisc::CE_THRESHOLD_EXCEEDED_MARK),
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
      q0->GetStats().GetNDroppedPackets(CobaltQueueDisc::TARGET_EXCEEDED_DROP),
      0, "There should not be any dropped packets");
  NS_TEST_EXPECT_MSG_EQ(
      q0->GetStats().GetNMarkedPackets(CobaltQueueDisc::FORCED_MARK), 0,
      "There should not be any marked packets");
  NS_TEST_EXPECT_MSG_EQ(
      q1->GetStats().GetNMarkedPackets(CobaltQueueDisc::FORCED_MARK), 2,
      "There should be 2 marked packets. Packets are dequeued"
      "from q0 first, which leads to delay greater than 5ms for the first "
      "dequeue from q1. Because of inactivity (started with high queue delay)"
      "Cobalt keeps drop_next as now and the next packet is marked. With "
      "second dequeue count increases to 2, drop_next becomes now plus around"
      "70ms which is less than the running time(140), and as the queue delay "
      "is persistently higher than 5ms, second packet is marked.");
  NS_TEST_EXPECT_MSG_EQ(
      q1->GetStats().GetNDroppedPackets(CobaltQueueDisc::TARGET_EXCEEDED_DROP),
      0, "There should not be any dropped packets");

  Simulator::Destroy();

  queueDisc = CreateObjectWithAttributes<FqCobaltQueueDisc>(
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
                      &FqCobaltQueueDiscL4sMode::AddPacket, this, queueDisc,
                      hdr, 1);
  Simulator::Schedule(Time(Seconds(0.0005)),
                      &FqCobaltQueueDiscL4sMode::AddPacketWithDelay, this,
                      queueDisc, hdr, delay, 69);

  hdr.SetEcn(Ipv4Header::ECN_ECT0);
  Simulator::Schedule(Time(Seconds(0)),
                      &FqCobaltQueueDiscL4sMode::AddPacketWithDelay, this,
                      queueDisc, hdr, delay, 70);

  DequeueWithDelay(queueDisc, delay, 140);
  Simulator::Run();
  Simulator::Stop(Seconds(8.0));
  q0 = queueDisc->GetQueueDiscClass(0)
           ->GetQueueDisc()
           ->GetObject<CobaltQueueDisc>();
  q0 = queueDisc->GetQueueDiscClass(0)
           ->GetQueueDisc()
           ->GetObject<CobaltQueueDisc>();

  NS_TEST_EXPECT_MSG_EQ(q0->GetStats().GetNMarkedPackets(
                            CobaltQueueDisc::CE_THRESHOLD_EXCEEDED_MARK),
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
      q0->GetStats().GetNDroppedPackets(CobaltQueueDisc::TARGET_EXCEEDED_DROP),
      0, "There should not be any dropped packets");
  NS_TEST_EXPECT_MSG_EQ(
      q0->GetStats().GetNMarkedPackets(CobaltQueueDisc::FORCED_MARK), 1,
      "There should be 1 marked packets");

  Simulator::Destroy();
}

class FqCobaltQueueDiscTestSuite : public TestSuite {
public:
  FqCobaltQueueDiscTestSuite();
};

FqCobaltQueueDiscTestSuite::FqCobaltQueueDiscTestSuite()
    : TestSuite("fq-cobalt-queue-disc", UNIT) {
  AddTestCase(new FqCobaltQueueDiscNoSuitableFilter, TestCase::QUICK);
  AddTestCase(new FqCobaltQueueDiscIPFlowsSeparationAndPacketLimit,
              TestCase::QUICK);
  AddTestCase(new FqCobaltQueueDiscDeficit, TestCase::QUICK);
  AddTestCase(new FqCobaltQueueDiscTCPFlowsSeparation, TestCase::QUICK);
  AddTestCase(new FqCobaltQueueDiscUDPFlowsSeparation, TestCase::QUICK);
  AddTestCase(new FqCobaltQueueDiscEcnMarking, TestCase::QUICK);
  AddTestCase(new FqCobaltQueueDiscSetLinearProbing, TestCase::QUICK);
  AddTestCase(new FqCobaltQueueDiscL4sMode, TestCase::QUICK);
}

static FqCobaltQueueDiscTestSuite g_fqCobaltQueueDiscTestSuite;
