
#include "ns3/drop-tail-queue.h"
#include "ns3/packet.h"
#include "ns3/queue-disc.h"
#include "ns3/simulator.h"
#include "ns3/test.h"

#include <map>

using namespace ns3;

class QdTestItem : public QueueDiscItem {
public:
  QdTestItem(Ptr<Packet> p, const Address &addr);
  ~QdTestItem() override;
  void AddHeader() override;
  bool Mark() override;
};

QdTestItem::QdTestItem(Ptr<Packet> p, const Address &addr)
    : QueueDiscItem(p, addr, 0) {}

QdTestItem::~QdTestItem() {}

void QdTestItem::AddHeader() {}

bool QdTestItem::Mark() { return false; }

class TestChildQueueDisc : public QueueDisc {
public:
  TestChildQueueDisc();
  ~TestChildQueueDisc() override;
  bool DoEnqueue(Ptr<QueueDiscItem> item) override;
  Ptr<QueueDiscItem> DoDequeue() override;
  bool CheckConfig() override;
  void InitializeParams() override;

  static constexpr const char *BEFORE_ENQUEUE = "Before enqueue";
  static constexpr const char *AFTER_DEQUEUE = "After dequeue";
};

TestChildQueueDisc::TestChildQueueDisc()
    : QueueDisc(QueueDiscSizePolicy::SINGLE_INTERNAL_QUEUE) {}

TestChildQueueDisc::~TestChildQueueDisc() {}

bool TestChildQueueDisc::DoEnqueue(Ptr<QueueDiscItem> item) {
  if (GetNPackets() >= 4) {
    DropBeforeEnqueue(item, BEFORE_ENQUEUE);
    return false;
  }
  return GetInternalQueue(0)->Enqueue(item);
}

Ptr<QueueDiscItem> TestChildQueueDisc::DoDequeue() {
  Ptr<QueueDiscItem> item = GetInternalQueue(0)->Dequeue();

  while (GetNPackets() >= 2) {
    DropAfterDequeue(item, AFTER_DEQUEUE);
    item = GetInternalQueue(0)->Dequeue();
  }
  return item;
}

bool TestChildQueueDisc::CheckConfig() {
  AddInternalQueue(CreateObject<DropTailQueue<QueueDiscItem>>());
  return true;
}

void TestChildQueueDisc::InitializeParams() {}

class TestParentQueueDisc : public QueueDisc {
public:
  TestParentQueueDisc();
  ~TestParentQueueDisc() override;
  bool DoEnqueue(Ptr<QueueDiscItem> item) override;
  Ptr<QueueDiscItem> DoDequeue() override;
  bool CheckConfig() override;
  void InitializeParams() override;
};

TestParentQueueDisc::TestParentQueueDisc()
    : QueueDisc(QueueDiscSizePolicy::SINGLE_CHILD_QUEUE_DISC) {}

TestParentQueueDisc::~TestParentQueueDisc() {}

bool TestParentQueueDisc::DoEnqueue(Ptr<QueueDiscItem> item) {
  return GetQueueDiscClass(0)->GetQueueDisc()->Enqueue(item);
}

Ptr<QueueDiscItem> TestParentQueueDisc::DoDequeue() {
  return GetQueueDiscClass(0)->GetQueueDisc()->Dequeue();
}

bool TestParentQueueDisc::CheckConfig() {
  Ptr<QueueDiscClass> c = CreateObject<QueueDiscClass>();
  c->SetQueueDisc(CreateObject<TestChildQueueDisc>());
  AddQueueDiscClass(c);
  return true;
}

void TestParentQueueDisc::InitializeParams() {}

class TestCounter {
public:
  TestCounter();
  virtual ~TestCounter();

  void ConnectTraces(Ptr<QueueDisc> qd);

private:
  void PacketEnqueued(Ptr<const QueueDiscItem> item);
  void PacketDequeued(Ptr<const QueueDiscItem> item);
  void PacketDbe(Ptr<const QueueDiscItem> item, const char *reason);
  void PacketDad(Ptr<const QueueDiscItem> item, const char *reason);

  uint32_t m_nPackets;
  uint32_t m_nBytes;
  uint32_t m_nDbePackets;
  uint32_t m_nDbeBytes;
  uint32_t m_nDadPackets;
  uint32_t m_nDadBytes;

  friend class QueueDiscTracesTestCase;
};

TestCounter::TestCounter()
    : m_nPackets(0), m_nBytes(0), m_nDbePackets(0), m_nDbeBytes(0),
      m_nDadPackets(0), m_nDadBytes(0) {}

TestCounter::~TestCounter() {}

void TestCounter::PacketEnqueued(Ptr<const QueueDiscItem> item) {
  m_nPackets++;
  m_nBytes += item->GetSize();
}

void TestCounter::PacketDequeued(Ptr<const QueueDiscItem> item) {
  m_nPackets--;
  m_nBytes -= item->GetSize();
}

void TestCounter::PacketDbe(Ptr<const QueueDiscItem> item, const char *reason) {
  m_nDbePackets++;
  m_nDbeBytes += item->GetSize();
}

void TestCounter::PacketDad(Ptr<const QueueDiscItem> item, const char *reason) {
  m_nDadPackets++;
  m_nDadBytes += item->GetSize();
}

void TestCounter::ConnectTraces(Ptr<QueueDisc> qd) {
  qd->TraceConnectWithoutContext(
      "Enqueue", MakeCallback(&TestCounter::PacketEnqueued, this));
  qd->TraceConnectWithoutContext(
      "Dequeue", MakeCallback(&TestCounter::PacketDequeued, this));
  qd->TraceConnectWithoutContext("DropBeforeEnqueue",
                                 MakeCallback(&TestCounter::PacketDbe, this));
  qd->TraceConnectWithoutContext("DropAfterDequeue",
                                 MakeCallback(&TestCounter::PacketDad, this));
}

class QueueDiscTracesTestCase : public TestCase {
public:
  QueueDiscTracesTestCase();
  void DoRun() override;

  void CheckQueued(Ptr<QueueDisc> qd, uint32_t nPackets, uint32_t nBytes);
  void CheckDroppedBeforeEnqueue(Ptr<QueueDisc> qd, uint32_t nDbePackets,
                                 uint32_t nDbeBytes);
  void CheckDroppedAfterDequeue(Ptr<QueueDisc> qd, uint32_t nDadPackets,
                                uint32_t nDadBytes);

private:
  std::map<Ptr<QueueDisc>, TestCounter> m_counter;
};

QueueDiscTracesTestCase::QueueDiscTracesTestCase()
    : TestCase("Sanity check on the queue disc traces and statistics") {}

void QueueDiscTracesTestCase::CheckQueued(Ptr<QueueDisc> qd, uint32_t nPackets,
                                          uint32_t nBytes) {
  NS_TEST_ASSERT_MSG_EQ(
      qd->GetNPackets(), nPackets,
      "Verify that the number of queued packets is computed correctly");
  NS_TEST_ASSERT_MSG_EQ(
      m_counter[qd].m_nPackets, nPackets,
      "Verify that the number of queued packets is computed correctly");

  NS_TEST_ASSERT_MSG_EQ(
      qd->GetNBytes(), nBytes,
      "Verify that the number of queued bytes is computed correctly");
  NS_TEST_ASSERT_MSG_EQ(
      m_counter[qd].m_nBytes, nBytes,
      "Verify that the number of queued bytes is computed correctly");
}

void QueueDiscTracesTestCase::CheckDroppedBeforeEnqueue(Ptr<QueueDisc> qd,
                                                        uint32_t nDbePackets,
                                                        uint32_t nDbeBytes) {
  QueueDisc::Stats stats = qd->GetStats();

  NS_TEST_ASSERT_MSG_EQ(stats.nTotalDroppedPacketsBeforeEnqueue, nDbePackets,
                        "Verify that the number of packets dropped before "
                        "enqueue is computed correctly");
  NS_TEST_ASSERT_MSG_EQ(m_counter[qd].m_nDbePackets, nDbePackets,
                        "Verify that the number of packets dropped before "
                        "enqueue is computed correctly");

  NS_TEST_ASSERT_MSG_EQ(stats.nTotalDroppedBytesBeforeEnqueue, nDbeBytes,
                        "Verify that the number of bytes dropped before "
                        "enqueue is computed correctly");
  NS_TEST_ASSERT_MSG_EQ(m_counter[qd].m_nDbeBytes, nDbeBytes,
                        "Verify that the number of bytes dropped before "
                        "enqueue is computed correctly");
}

void QueueDiscTracesTestCase::CheckDroppedAfterDequeue(Ptr<QueueDisc> qd,
                                                       uint32_t nDadPackets,
                                                       uint32_t nDadBytes) {
  QueueDisc::Stats stats = qd->GetStats();

  NS_TEST_ASSERT_MSG_EQ(stats.nTotalDroppedPacketsAfterDequeue, nDadPackets,
                        "Verify that the number of packets dropped after "
                        "dequeue is computed correctly");
  NS_TEST_ASSERT_MSG_EQ(m_counter[qd].m_nDadPackets, nDadPackets,
                        "Verify that the number of packets dropped after "
                        "dequeue is computed correctly");

  NS_TEST_ASSERT_MSG_EQ(stats.nTotalDroppedBytesAfterDequeue, nDadBytes,
                        "Verify that the number of bytes dropped after dequeue "
                        "is computed correctly");
  NS_TEST_ASSERT_MSG_EQ(m_counter[qd].m_nDadBytes, nDadBytes,
                        "Verify that the number of bytes dropped after dequeue "
                        "is computed correctly");
}

void QueueDiscTracesTestCase::DoRun() {
  Address dest;
  uint32_t pktSizeUnit = 100;
  Ptr<const QueueDiscItem> item;

  Ptr<QueueDisc> root = CreateObject<TestParentQueueDisc>();
  root->Initialize();

  Ptr<QueueDisc> child = root->GetQueueDiscClass(0)->GetQueueDisc();

  NS_TEST_ASSERT_MSG_NE(child, nullptr,
                        "The child queue disc has not been created");

  m_counter.emplace(root, TestCounter());
  m_counter.emplace(child, TestCounter());

  m_counter[root].ConnectTraces(root);
  m_counter[child].ConnectTraces(child);

  for (uint16_t i = 1; i <= 4; i++) {
    root->Enqueue(Create<QdTestItem>(Create<Packet>(pktSizeUnit * i), dest));

    CheckQueued(root, i, pktSizeUnit * i * (i + 1) / 2);
    CheckDroppedBeforeEnqueue(root, 0, 0);
    CheckDroppedAfterDequeue(root, 0, 0);

    CheckQueued(child, i, pktSizeUnit * i * (i + 1) / 2);
    CheckDroppedBeforeEnqueue(child, 0, 0);
    CheckDroppedAfterDequeue(child, 0, 0);
  }

  root->Enqueue(Create<QdTestItem>(Create<Packet>(pktSizeUnit * 5), dest));

  CheckQueued(root, 4, pktSizeUnit * 10);
  CheckDroppedBeforeEnqueue(root, 1, pktSizeUnit * 5);
  CheckDroppedAfterDequeue(root, 0, 0);

  CheckQueued(child, 4, pktSizeUnit * 10);
  CheckDroppedBeforeEnqueue(child, 1, pktSizeUnit * 5);
  CheckDroppedAfterDequeue(child, 0, 0);

  item = root->Peek();

  NS_TEST_ASSERT_MSG_NE(item, nullptr, "A packet must have been returned");
  NS_TEST_ASSERT_MSG_EQ(item->GetSize(), pktSizeUnit * 3,
                        "The peeked packet has not the expected size");

  CheckQueued(root, 2, pktSizeUnit * 7);
  CheckDroppedBeforeEnqueue(root, 1, pktSizeUnit * 5);
  CheckDroppedAfterDequeue(root, 2, pktSizeUnit * 3);

  CheckQueued(child, 1, pktSizeUnit * 4);
  CheckDroppedBeforeEnqueue(child, 1, pktSizeUnit * 5);
  CheckDroppedAfterDequeue(child, 2, pktSizeUnit * 3);

  item = root->Peek();

  NS_TEST_ASSERT_MSG_NE(item, nullptr, "A packet must have been returned");
  NS_TEST_ASSERT_MSG_EQ(item->GetSize(), pktSizeUnit * 3,
                        "The peeked packet has not the expected size");

  CheckQueued(root, 2, pktSizeUnit * 7);
  CheckDroppedBeforeEnqueue(root, 1, pktSizeUnit * 5);
  CheckDroppedAfterDequeue(root, 2, pktSizeUnit * 3);

  CheckQueued(child, 1, pktSizeUnit * 4);
  CheckDroppedBeforeEnqueue(child, 1, pktSizeUnit * 5);
  CheckDroppedAfterDequeue(child, 2, pktSizeUnit * 3);

  item = root->Dequeue();

  NS_TEST_ASSERT_MSG_NE(item, nullptr, "A packet must have been returned");
  NS_TEST_ASSERT_MSG_EQ(item->GetSize(), pktSizeUnit * 3,
                        "The dequeued packet has not the expected size");

  CheckQueued(root, 1, pktSizeUnit * 4);
  CheckDroppedBeforeEnqueue(root, 1, pktSizeUnit * 5);
  CheckDroppedAfterDequeue(root, 2, pktSizeUnit * 3);

  CheckQueued(child, 1, pktSizeUnit * 4);
  CheckDroppedBeforeEnqueue(child, 1, pktSizeUnit * 5);
  CheckDroppedAfterDequeue(child, 2, pktSizeUnit * 3);

  item = root->Dequeue();

  NS_TEST_ASSERT_MSG_NE(item, nullptr, "A packet must have been returned");
  NS_TEST_ASSERT_MSG_EQ(item->GetSize(), pktSizeUnit * 4,
                        "The dequeued packet has not the expected size");

  CheckQueued(root, 0, 0);
  CheckDroppedBeforeEnqueue(root, 1, pktSizeUnit * 5);
  CheckDroppedAfterDequeue(root, 2, pktSizeUnit * 3);

  CheckQueued(child, 0, 0);
  CheckDroppedBeforeEnqueue(child, 1, pktSizeUnit * 5);
  CheckDroppedAfterDequeue(child, 2, pktSizeUnit * 3);

  item = root->Peek();

  NS_TEST_ASSERT_MSG_EQ(item, nullptr, "No packet must have been returned");

  CheckQueued(root, 0, 0);
  CheckDroppedBeforeEnqueue(root, 1, pktSizeUnit * 5);
  CheckDroppedAfterDequeue(root, 2, pktSizeUnit * 3);

  CheckQueued(child, 0, 0);
  CheckDroppedBeforeEnqueue(child, 1, pktSizeUnit * 5);
  CheckDroppedAfterDequeue(child, 2, pktSizeUnit * 3);

  root->Enqueue(Create<QdTestItem>(Create<Packet>(pktSizeUnit), dest));

  CheckQueued(root, 1, pktSizeUnit);
  CheckDroppedBeforeEnqueue(root, 1, pktSizeUnit * 5);
  CheckDroppedAfterDequeue(root, 2, pktSizeUnit * 3);

  CheckQueued(child, 1, pktSizeUnit);
  CheckDroppedBeforeEnqueue(child, 1, pktSizeUnit * 5);
  CheckDroppedAfterDequeue(child, 2, pktSizeUnit * 3);

  item = root->Dequeue();

  NS_TEST_ASSERT_MSG_NE(item, nullptr, "A packet must have been returned");
  NS_TEST_ASSERT_MSG_EQ(item->GetSize(), pktSizeUnit,
                        "The dequeued packet has not the expected size");

  CheckQueued(root, 0, 0);
  CheckDroppedBeforeEnqueue(root, 1, pktSizeUnit * 5);
  CheckDroppedAfterDequeue(root, 2, pktSizeUnit * 3);

  CheckQueued(child, 0, 0);
  CheckDroppedBeforeEnqueue(child, 1, pktSizeUnit * 5);
  CheckDroppedAfterDequeue(child, 2, pktSizeUnit * 3);

  Simulator::Destroy();
}

static class QueueDiscTracesTestSuite : public TestSuite {
public:
  QueueDiscTracesTestSuite() : TestSuite("queue-disc-traces", UNIT) {
    AddTestCase(new QueueDiscTracesTestCase(), TestCase::QUICK);
  }
} g_queueDiscTracesTestSuite;
