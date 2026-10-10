
#include "ns3/double.h"
#include "ns3/fifo-queue-disc.h"
#include "ns3/log.h"
#include "ns3/object-factory.h"
#include "ns3/packet.h"
#include "ns3/queue.h"
#include "ns3/simulator.h"
#include "ns3/string.h"
#include "ns3/test.h"
#include "ns3/uinteger.h"

#include <vector>

using namespace ns3;

class FifoQueueDiscTestItem : public QueueDiscItem {
public:
  FifoQueueDiscTestItem(Ptr<Packet> p, const Address &addr);
  ~FifoQueueDiscTestItem() override;

  FifoQueueDiscTestItem() = delete;
  FifoQueueDiscTestItem(const FifoQueueDiscTestItem &) = delete;
  FifoQueueDiscTestItem &operator=(const FifoQueueDiscTestItem &) = delete;

  void AddHeader() override;
  bool Mark() override;
};

FifoQueueDiscTestItem::FifoQueueDiscTestItem(Ptr<Packet> p, const Address &addr)
    : QueueDiscItem(p, addr, 0) {}

FifoQueueDiscTestItem::~FifoQueueDiscTestItem() {}

void FifoQueueDiscTestItem::AddHeader() {}

bool FifoQueueDiscTestItem::Mark() { return false; }

class FifoQueueDiscTestCase : public TestCase {
public:
  FifoQueueDiscTestCase();
  void DoRun() override;

private:
  void RunFifoTest(QueueSizeUnit mode);
  void DoRunFifoTest(Ptr<FifoQueueDisc> q, uint32_t qSize, uint32_t pktSize);
};

FifoQueueDiscTestCase::FifoQueueDiscTestCase()
    : TestCase("Sanity check on the fifo queue disc implementation") {}

void FifoQueueDiscTestCase::DoRunFifoTest(Ptr<FifoQueueDisc> q, uint32_t qSize,
                                          uint32_t pktSize) {
  std::vector<uint64_t> uids;
  Ptr<Packet> p;
  Ptr<QueueDiscItem> item;
  Address dest;
  uint32_t modeSize =
      (q->GetMaxSize().GetUnit() == QueueSizeUnit::PACKETS ? 1 : pktSize);
  uint32_t numPackets = qSize / modeSize;

  NS_TEST_ASSERT_MSG_EQ(q->GetCurrentSize().GetValue(), 0,
                        "The queue disc should be empty");

  for (uint32_t i = 1; i <= numPackets; i++) {
    p = Create<Packet>(pktSize);
    uids.push_back(p->GetUid());
    q->Enqueue(Create<FifoQueueDiscTestItem>(p, dest));
    NS_TEST_ASSERT_MSG_EQ(q->GetCurrentSize().GetValue(), i * modeSize,
                          "There should be " << i << " packet(s) in there");
  }

  NS_TEST_ASSERT_MSG_EQ(q->Enqueue(Create<FifoQueueDiscTestItem>(p, dest)),
                        false, "There should be no room for another packet");

  for (uint32_t i = 1; i <= numPackets; i++) {
    item = q->Dequeue();
    NS_TEST_ASSERT_MSG_NE(item, nullptr, "A packet should have been dequeued");
    NS_TEST_ASSERT_MSG_EQ(
        q->GetCurrentSize().GetValue(), (numPackets - i) * modeSize,
        "There should be " << numPackets - i << " packet(s) in there");
    NS_TEST_ASSERT_MSG_EQ(item->GetPacket()->GetUid(), uids[i - 1],
                          "was this the right packet?");
  }

  item = q->Dequeue();
  NS_TEST_ASSERT_MSG_EQ(item, nullptr, "There are really no packets in there");
}

void FifoQueueDiscTestCase::RunFifoTest(QueueSizeUnit mode) {
  Ptr<FifoQueueDisc> queue;
  uint32_t numPackets = 10;
  uint32_t pktSize = 1000;
  uint32_t modeSize = (mode == QueueSizeUnit::PACKETS ? 1 : pktSize);

  queue = CreateObject<FifoQueueDisc>();

  NS_TEST_ASSERT_MSG_EQ(queue->GetNInternalQueues(), 0,
                        "Verify that the queue disc has no internal queue");

  NS_TEST_ASSERT_MSG_EQ(
      queue->SetAttributeFailSafe(
          "MaxSize", QueueSizeValue(QueueSize(mode, numPackets * modeSize))),
      true, "Verify that we can actually set the attribute MaxSize");

  queue->Initialize();

  DoRunFifoTest(queue, numPackets * modeSize, pktSize);

  queue = CreateObject<FifoQueueDisc>();

  NS_TEST_ASSERT_MSG_EQ(queue->GetNInternalQueues(), 0,
                        "Verify that the queue disc has no internal queue");

  queue->Initialize();

  NS_TEST_ASSERT_MSG_EQ(
      queue->SetAttributeFailSafe(
          "MaxSize", QueueSizeValue(QueueSize(mode, numPackets * modeSize))),
      true, "Verify that we can actually set the attribute MaxSize");

  DoRunFifoTest(queue, numPackets * modeSize, pktSize);

  queue = CreateObject<FifoQueueDisc>();

  NS_TEST_ASSERT_MSG_EQ(queue->GetNInternalQueues(), 0,
                        "Verify that the queue disc has no internal queue");

  ObjectFactory factory;
  factory.SetTypeId("ns3::DropTailQueue<QueueDiscItem>");
  if (mode == QueueSizeUnit::PACKETS) {
    factory.Set("MaxSize",
                QueueSizeValue(QueueSize(QueueSizeUnit::PACKETS, numPackets)));
  } else {
    factory.Set("MaxSize", QueueSizeValue(QueueSize(QueueSizeUnit::BYTES,
                                                    numPackets * pktSize)));
  }
  queue->AddInternalQueue(factory.Create<QueueDisc::InternalQueue>());

  queue->Initialize();

  DoRunFifoTest(queue, numPackets * modeSize, pktSize);

  queue = CreateObject<FifoQueueDisc>();

  NS_TEST_ASSERT_MSG_EQ(queue->GetNInternalQueues(), 0,
                        "Verify that the queue disc has no internal queue");

  queue->Initialize();

  NS_TEST_ASSERT_MSG_EQ(queue->GetNInternalQueues(), 1,
                        "Verify that the queue disc got an internal queue");

  Ptr<QueueDisc::InternalQueue> iq = queue->GetInternalQueue(0);

  if (mode == QueueSizeUnit::PACKETS) {
    NS_TEST_ASSERT_MSG_EQ(
        iq->SetAttributeFailSafe(
            "MaxSize",
            QueueSizeValue(QueueSize(QueueSizeUnit::PACKETS, numPackets))),
        true,
        "Verify that we can actually set the attribute MaxSize on the internal "
        "queue");
  } else {
    NS_TEST_ASSERT_MSG_EQ(
        iq->SetAttributeFailSafe(
            "MaxSize", QueueSizeValue(QueueSize(QueueSizeUnit::BYTES,
                                                numPackets * pktSize))),
        true,
        "Verify that we can actually set the attribute MaxSize on the internal "
        "queue");
  }

  DoRunFifoTest(queue, numPackets * modeSize, pktSize);
}

void FifoQueueDiscTestCase::DoRun() {
  RunFifoTest(QueueSizeUnit::PACKETS);
  RunFifoTest(QueueSizeUnit::BYTES);
  Simulator::Destroy();
}

static class FifoQueueDiscTestSuite : public TestSuite {
public:
  FifoQueueDiscTestSuite() : TestSuite("fifo-queue-disc", UNIT) {
    AddTestCase(new FifoQueueDiscTestCase(), TestCase::QUICK);
  }
} g_fifoQueueTestSuite;
