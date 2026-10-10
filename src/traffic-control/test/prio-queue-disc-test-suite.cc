
#include "ns3/fifo-queue-disc.h"
#include "ns3/log.h"
#include "ns3/packet-filter.h"
#include "ns3/packet.h"
#include "ns3/prio-queue-disc.h"
#include "ns3/simulator.h"
#include "ns3/socket.h"
#include "ns3/string.h"
#include "ns3/test.h"

#include <array>
#include <queue>

using namespace ns3;

class PrioQueueDiscTestItem : public QueueDiscItem {
public:
  PrioQueueDiscTestItem(Ptr<Packet> p, const Address &addr, uint8_t priority);
  void AddHeader() override;
  bool Mark() override;
};

PrioQueueDiscTestItem::PrioQueueDiscTestItem(Ptr<Packet> p, const Address &addr,
                                             uint8_t priority)
    : QueueDiscItem(p, addr, 0) {
  SocketPriorityTag priorityTag;
  priorityTag.SetPriority(priority);
  p->ReplacePacketTag(priorityTag);
}

void PrioQueueDiscTestItem::AddHeader() {}

bool PrioQueueDiscTestItem::Mark() { return false; }

class PrioQueueDiscTestFilter : public PacketFilter {
public:
  PrioQueueDiscTestFilter(bool cls);
  ~PrioQueueDiscTestFilter() override;
  void SetReturnValue(int32_t ret);

private:
  bool CheckProtocol(Ptr<QueueDiscItem> item) const override;
  int32_t DoClassify(Ptr<QueueDiscItem> item) const override;

  bool m_cls;
  int32_t m_ret;
};

PrioQueueDiscTestFilter::PrioQueueDiscTestFilter(bool cls)
    : m_cls(cls), m_ret(0) {}

PrioQueueDiscTestFilter::~PrioQueueDiscTestFilter() {}

void PrioQueueDiscTestFilter::SetReturnValue(int32_t ret) { m_ret = ret; }

bool PrioQueueDiscTestFilter::CheckProtocol(Ptr<QueueDiscItem> item) const {
  return m_cls;
}

int32_t PrioQueueDiscTestFilter::DoClassify(Ptr<QueueDiscItem> item) const {
  return m_ret;
}

class PrioQueueDiscTestCase : public TestCase {
public:
  PrioQueueDiscTestCase();
  void DoRun() override;
};

PrioQueueDiscTestCase::PrioQueueDiscTestCase()
    : TestCase("Sanity check on the prio queue disc implementation") {}

void PrioQueueDiscTestCase::DoRun() {
  Ptr<PrioQueueDisc> qdisc;
  Ptr<QueueDiscItem> item;
  std::string priomap("0 1 2 3 0 1 2 3 0 1 2 3 0 1 2 3");
  Address dest;
  std::array<std::queue<uint64_t>, 4> uids;

  qdisc = CreateObject<PrioQueueDisc>();

  for (uint8_t i = 0; i < 4; i++) {
    Ptr<FifoQueueDisc> child = CreateObject<FifoQueueDisc>();
    child->Initialize();
    Ptr<QueueDiscClass> c = CreateObject<QueueDiscClass>();
    c->SetQueueDisc(child);
    qdisc->AddQueueDiscClass(c);
  }
  qdisc->Initialize();

  NS_TEST_ASSERT_MSG_EQ(qdisc->GetNQueueDiscClasses(), 4,
                        "Verify that the queue disc has 4 child queue discs");

  NS_TEST_ASSERT_MSG_EQ(
      qdisc->SetAttributeFailSafe("Priomap", StringValue(priomap)), true,
      "Verify that we can actually set the attribute Priomap");

  StringValue sv;
  NS_TEST_ASSERT_MSG_EQ(
      qdisc->GetAttributeFailSafe("Priomap", sv), true,
      "Verify that we can actually get the attribute Priomap");

  NS_TEST_ASSERT_MSG_EQ(sv.Get(), priomap,
                        "Verify that the priomap has been correctly set");

  for (uint16_t i = 0; i < 4; i++) {
    NS_TEST_ASSERT_MSG_EQ(
        qdisc->GetQueueDiscClass(i)->GetQueueDisc()->GetNPackets(), 0,
        "There should be no packets in the child queue disc " << i);

    item = Create<PrioQueueDiscTestItem>(Create<Packet>(100), dest, i);
    qdisc->Enqueue(item);
    uids[i].push(item->GetPacket()->GetUid());

    NS_TEST_ASSERT_MSG_EQ(
        qdisc->GetQueueDiscClass(i)->GetQueueDisc()->GetNPackets(), 1,
        "There should be one packet in the child queue disc " << i);
  }

  Ptr<PrioQueueDiscTestFilter> pf1 =
      CreateObject<PrioQueueDiscTestFilter>(false);
  qdisc->AddPacketFilter(pf1);

  for (uint16_t i = 0; i < 4; i++) {
    NS_TEST_ASSERT_MSG_EQ(
        qdisc->GetQueueDiscClass(i)->GetQueueDisc()->GetNPackets(), 1,
        "There should be one packet in the child queue disc " << i);

    item = Create<PrioQueueDiscTestItem>(Create<Packet>(100), dest, i + 4);
    qdisc->Enqueue(item);
    uids[i].push(item->GetPacket()->GetUid());

    NS_TEST_ASSERT_MSG_EQ(
        qdisc->GetQueueDiscClass(i)->GetQueueDisc()->GetNPackets(), 2,
        "There should be two packets in the child queue disc " << i);
  }

  Ptr<PrioQueueDiscTestFilter> pf2 =
      CreateObject<PrioQueueDiscTestFilter>(true);
  qdisc->AddPacketFilter(pf2);

  for (uint16_t i = 0; i < 4; i++) {
    pf2->SetReturnValue(i);
    NS_TEST_ASSERT_MSG_EQ(
        qdisc->GetQueueDiscClass(i)->GetQueueDisc()->GetNPackets(), 2,
        "There should be two packets in the child queue disc " << i);

    item = Create<PrioQueueDiscTestItem>(Create<Packet>(100), dest, 0);
    qdisc->Enqueue(item);
    uids[i].push(item->GetPacket()->GetUid());

    NS_TEST_ASSERT_MSG_EQ(
        qdisc->GetQueueDiscClass(i)->GetQueueDisc()->GetNPackets(), 3,
        "There should be three packets in the child queue disc " << i);
  }

  for (uint16_t i = 0; i < 4; i++) {
    pf2->SetReturnValue(4 + i);
    NS_TEST_ASSERT_MSG_EQ(qdisc->GetBandForPriority(0), 0,
                          "The band for priority 0 must be band 0");
    NS_TEST_ASSERT_MSG_EQ(
        qdisc->GetQueueDiscClass(0)->GetQueueDisc()->GetNPackets(), i + 3U,
        "There should be " << i + 3 << " packets in the child queue disc "
                           << qdisc->GetBandForPriority(0));

    item = Create<PrioQueueDiscTestItem>(Create<Packet>(100), dest, 1);
    qdisc->Enqueue(item);
    uids[0].push(item->GetPacket()->GetUid());

    NS_TEST_ASSERT_MSG_EQ(
        qdisc->GetQueueDiscClass(0)->GetQueueDisc()->GetNPackets(), i + 4U,
        "There should be " << i + 4 << " packets in the child queue disc "
                           << qdisc->GetBandForPriority(0));
  }

  while ((item = qdisc->Dequeue())) {
    for (uint16_t i = 0; i < 4; i++) {
      if (uids[i].empty()) {
        NS_TEST_ASSERT_MSG_EQ(
            qdisc->GetQueueDiscClass(i)->GetQueueDisc()->GetNPackets(), 0,
            "Band " << i << " should be empty");
        continue;
      }
      NS_TEST_ASSERT_MSG_EQ(uids[i].front(), item->GetPacket()->GetUid(),
                            "The dequeued packet is not the one we expected");
      uids[i].pop();
      break;
    }
  }

  Simulator::Destroy();
}

static class PrioQueueDiscTestSuite : public TestSuite {
public:
  PrioQueueDiscTestSuite() : TestSuite("prio-queue-disc", UNIT) {
    AddTestCase(new PrioQueueDiscTestCase(), TestCase::QUICK);
  }
} g_prioQueueTestSuite;
