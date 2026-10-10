
#include "ns3/cobalt-queue-disc.h"
#include "ns3/double.h"
#include "ns3/log.h"
#include "ns3/packet.h"
#include "ns3/simulator.h"
#include "ns3/string.h"
#include "ns3/test.h"
#include "ns3/uinteger.h"

using namespace ns3;

class CobaltQueueDiscTestItem : public QueueDiscItem {
public:
  CobaltQueueDiscTestItem(Ptr<Packet> p, const Address &addr, bool ecnCapable);
  ~CobaltQueueDiscTestItem() override;

  CobaltQueueDiscTestItem() = delete;
  CobaltQueueDiscTestItem(const CobaltQueueDiscTestItem &) = delete;
  CobaltQueueDiscTestItem &operator=(const CobaltQueueDiscTestItem &) = delete;

  void AddHeader() override;
  bool Mark() override;

private:
  bool m_ecnCapablePacket;
};

CobaltQueueDiscTestItem::CobaltQueueDiscTestItem(Ptr<Packet> p,
                                                 const Address &addr,
                                                 bool ecnCapable)
    : QueueDiscItem(p, addr, 0), m_ecnCapablePacket(ecnCapable) {}

CobaltQueueDiscTestItem::~CobaltQueueDiscTestItem() {}

void CobaltQueueDiscTestItem::AddHeader() {}

bool CobaltQueueDiscTestItem::Mark() { return m_ecnCapablePacket; }

class CobaltQueueDiscBasicEnqueueDequeue : public TestCase {
public:
  CobaltQueueDiscBasicEnqueueDequeue(QueueSizeUnit mode);
  void DoRun() override;

private:
  QueueSizeUnit m_mode;
};

CobaltQueueDiscBasicEnqueueDequeue::CobaltQueueDiscBasicEnqueueDequeue(
    QueueSizeUnit mode)
    : TestCase("Basic enqueue and dequeue operations, and attribute setting" +
               std::to_string(mode)) {
  m_mode = mode;
}

void CobaltQueueDiscBasicEnqueueDequeue::DoRun() {
  Ptr<CobaltQueueDisc> queue = CreateObject<CobaltQueueDisc>();

  uint32_t pktSize = 1000;
  uint32_t modeSize = 0;

  Address dest;

  NS_TEST_ASSERT_MSG_EQ(
      queue->SetAttributeFailSafe("Interval", StringValue("50ms")), true,
      "Verify that we can actually set the attribute Interval");
  NS_TEST_ASSERT_MSG_EQ(
      queue->SetAttributeFailSafe("Target", StringValue("4ms")), true,
      "Verify that we can actually set the attribute Target");
  NS_TEST_ASSERT_MSG_EQ(
      queue->SetAttributeFailSafe("BlueThreshold", TimeValue(Time::Max())),
      true, "Disable Blue enhancement");

  if (m_mode == QueueSizeUnit::BYTES) {
    modeSize = pktSize;
  } else if (m_mode == QueueSizeUnit::PACKETS) {
    modeSize = 1;
  }
  NS_TEST_ASSERT_MSG_EQ(
      queue->SetAttributeFailSafe(
          "MaxSize", QueueSizeValue(QueueSize(m_mode, modeSize * 1500))),
      true, "Verify that we can actually set the attribute MaxSize");
  queue->Initialize();

  Ptr<Packet> p1;
  Ptr<Packet> p2;
  Ptr<Packet> p3;
  Ptr<Packet> p4;
  Ptr<Packet> p5;
  Ptr<Packet> p6;
  p1 = Create<Packet>(pktSize);
  p2 = Create<Packet>(pktSize);
  p3 = Create<Packet>(pktSize);
  p4 = Create<Packet>(pktSize);
  p5 = Create<Packet>(pktSize);
  p6 = Create<Packet>(pktSize);

  NS_TEST_ASSERT_MSG_EQ(queue->GetCurrentSize().GetValue(), 0 * modeSize,
                        "There should be no packets in queue");
  queue->Enqueue(Create<CobaltQueueDiscTestItem>(p1, dest, false));
  NS_TEST_ASSERT_MSG_EQ(queue->GetCurrentSize().GetValue(), 1 * modeSize,
                        "There should be one packet in queue");
  queue->Enqueue(Create<CobaltQueueDiscTestItem>(p2, dest, false));
  NS_TEST_ASSERT_MSG_EQ(queue->GetCurrentSize().GetValue(), 2 * modeSize,
                        "There should be two packets in queue");
  queue->Enqueue(Create<CobaltQueueDiscTestItem>(p3, dest, false));
  NS_TEST_ASSERT_MSG_EQ(queue->GetCurrentSize().GetValue(), 3 * modeSize,
                        "There should be three packets in queue");
  queue->Enqueue(Create<CobaltQueueDiscTestItem>(p4, dest, false));
  NS_TEST_ASSERT_MSG_EQ(queue->GetCurrentSize().GetValue(), 4 * modeSize,
                        "There should be four packets in queue");
  queue->Enqueue(Create<CobaltQueueDiscTestItem>(p5, dest, false));
  NS_TEST_ASSERT_MSG_EQ(queue->GetCurrentSize().GetValue(), 5 * modeSize,
                        "There should be five packets in queue");
  queue->Enqueue(Create<CobaltQueueDiscTestItem>(p6, dest, false));
  NS_TEST_ASSERT_MSG_EQ(queue->GetCurrentSize().GetValue(), 6 * modeSize,
                        "There should be six packets in queue");

  NS_TEST_ASSERT_MSG_EQ(
      queue->GetStats().GetNDroppedPackets(CobaltQueueDisc::OVERLIMIT_DROP), 0,
      "There should be no packets being dropped due to full queue");

  Ptr<QueueDiscItem> item;

  item = queue->Dequeue();
  NS_TEST_ASSERT_MSG_NE(item, nullptr, "I want to remove the first packet");
  NS_TEST_ASSERT_MSG_EQ(queue->GetCurrentSize().GetValue(), 5 * modeSize,
                        "There should be five packets in queue");
  NS_TEST_ASSERT_MSG_EQ(item->GetPacket()->GetUid(), p1->GetUid(),
                        "was this the first packet ?");

  item = queue->Dequeue();
  NS_TEST_ASSERT_MSG_NE(item, nullptr, "I want to remove the second packet");
  NS_TEST_ASSERT_MSG_EQ(queue->GetCurrentSize().GetValue(), 4 * modeSize,
                        "There should be four packets in queue");
  NS_TEST_ASSERT_MSG_EQ(item->GetPacket()->GetUid(), p2->GetUid(),
                        "Was this the second packet ?");

  item = queue->Dequeue();
  NS_TEST_ASSERT_MSG_NE(item, nullptr, "I want to remove the third packet");
  NS_TEST_ASSERT_MSG_EQ(queue->GetCurrentSize().GetValue(), 3 * modeSize,
                        "There should be three packets in queue");
  NS_TEST_ASSERT_MSG_EQ(item->GetPacket()->GetUid(), p3->GetUid(),
                        "Was this the third packet ?");

  item = queue->Dequeue();
  NS_TEST_ASSERT_MSG_NE(item, nullptr, "I want to remove the forth packet");
  NS_TEST_ASSERT_MSG_EQ(queue->GetCurrentSize().GetValue(), 2 * modeSize,
                        "There should be two packets in queue");
  NS_TEST_ASSERT_MSG_EQ(item->GetPacket()->GetUid(), p4->GetUid(),
                        "Was this the fourth packet ?");

  item = queue->Dequeue();
  NS_TEST_ASSERT_MSG_NE(item, nullptr, "I want to remove the fifth packet");
  NS_TEST_ASSERT_MSG_EQ(queue->GetCurrentSize().GetValue(), 1 * modeSize,
                        "There should be one packet in queue");
  NS_TEST_ASSERT_MSG_EQ(item->GetPacket()->GetUid(), p5->GetUid(),
                        "Was this the fifth packet ?");

  item = queue->Dequeue();
  NS_TEST_ASSERT_MSG_NE(item, nullptr, "I want to remove the last packet");
  NS_TEST_ASSERT_MSG_EQ(queue->GetCurrentSize().GetValue(), 0 * modeSize,
                        "There should be zero packet in queue");
  NS_TEST_ASSERT_MSG_EQ(item->GetPacket()->GetUid(), p6->GetUid(),
                        "Was this the sixth packet ?");

  item = queue->Dequeue();
  NS_TEST_ASSERT_MSG_EQ(item, nullptr, "There are really no packets in queue");

  NS_TEST_ASSERT_MSG_EQ(
      queue->GetStats().GetNDroppedPackets(
          CobaltQueueDisc::TARGET_EXCEEDED_DROP),
      0, "There should be no packet drops according to Cobalt algorithm");
}

class CobaltQueueDiscDropTest : public TestCase {
public:
  CobaltQueueDiscDropTest();
  void DoRun() override;
  void Enqueue(Ptr<CobaltQueueDisc> queue, uint32_t size, uint32_t nPkt);
  void RunDropTest(QueueSizeUnit mode);
  void EnqueueWithDelay(Ptr<CobaltQueueDisc> queue, uint32_t size,
                        uint32_t nPkt);
};

CobaltQueueDiscDropTest::CobaltQueueDiscDropTest()
    : TestCase("Drop tests verification for both packets and bytes mode") {}

void CobaltQueueDiscDropTest::RunDropTest(QueueSizeUnit mode)

{
  uint32_t pktSize = 1500;
  uint32_t modeSize = 0;
  Ptr<CobaltQueueDisc> queue = CreateObject<CobaltQueueDisc>();

  if (mode == QueueSizeUnit::BYTES) {
    modeSize = pktSize;
  } else if (mode == QueueSizeUnit::PACKETS) {
    modeSize = 1;
  }

  queue = CreateObject<CobaltQueueDisc>();
  NS_TEST_ASSERT_MSG_EQ(
      queue->SetAttributeFailSafe(
          "MaxSize", QueueSizeValue(QueueSize(mode, modeSize * 100))),
      true, "Verify that we can actually set the attribute MaxSize");
  NS_TEST_ASSERT_MSG_EQ(
      queue->SetAttributeFailSafe("BlueThreshold", TimeValue(Time::Max())),
      true, "Disable Blue enhancement");
  queue->Initialize();

  if (mode == QueueSizeUnit::BYTES) {
    EnqueueWithDelay(queue, pktSize, 200);
  } else {
    EnqueueWithDelay(queue, 1, 200);
  }

  Simulator::Stop(Seconds(8.0));
  Simulator::Run();

  QueueDisc::Stats st = queue->GetStats();

  NS_TEST_ASSERT_MSG_NE(queue->GetPdrop(), 0, "Pdrop should be non-zero");
  NS_TEST_ASSERT_MSG_NE(st.GetNDroppedPackets(CobaltQueueDisc::OVERLIMIT_DROP),
                        0, "Drops due to queue overflow should be non-zero");
}

void CobaltQueueDiscDropTest::EnqueueWithDelay(Ptr<CobaltQueueDisc> queue,
                                               uint32_t size, uint32_t nPkt) {
  Address dest;
  double delay = 0.01;
  for (uint32_t i = 0; i < nPkt; i++) {
    Simulator::Schedule(Time(Seconds((i + 1) * delay)),
                        &CobaltQueueDiscDropTest::Enqueue, this, queue, size,
                        1);
  }
}

void CobaltQueueDiscDropTest::Enqueue(Ptr<CobaltQueueDisc> queue, uint32_t size,
                                      uint32_t nPkt) {
  Address dest;
  for (uint32_t i = 0; i < nPkt; i++) {
    queue->Enqueue(
        Create<CobaltQueueDiscTestItem>(Create<Packet>(size), dest, true));
  }
}

void CobaltQueueDiscDropTest::DoRun() {
  RunDropTest(QueueSizeUnit::PACKETS);
  RunDropTest(QueueSizeUnit::BYTES);
  Simulator::Destroy();
}

class CobaltQueueDiscMarkTest : public TestCase {
public:
  CobaltQueueDiscMarkTest(QueueSizeUnit mode);
  void DoRun() override;

private:
  void Enqueue(Ptr<CobaltQueueDisc> queue, uint32_t size, uint32_t nPkt,
               bool ecnCapable);
  void Dequeue(Ptr<CobaltQueueDisc> queue, uint32_t modeSize,
               uint32_t testCase);
  void DropNextTracer(int64_t oldVal, int64_t newVal);
  QueueSizeUnit m_mode;
  uint32_t m_dropNextCount;
  uint32_t nPacketsBeforeFirstDrop;
  uint32_t nPacketsBeforeFirstMark;
};

CobaltQueueDiscMarkTest::CobaltQueueDiscMarkTest(QueueSizeUnit mode)
    : TestCase("Basic mark operations") {
  m_mode = mode;
  m_dropNextCount = 0;
}

void CobaltQueueDiscMarkTest::DropNextTracer(int64_t, int64_t) {
  m_dropNextCount++;
}

void CobaltQueueDiscMarkTest::DoRun() {

  Ptr<CobaltQueueDisc> queue = CreateObject<CobaltQueueDisc>();
  uint32_t pktSize = 1000;
  uint32_t modeSize = 0;
  nPacketsBeforeFirstDrop = 0;
  nPacketsBeforeFirstMark = 0;

  if (m_mode == QueueSizeUnit::BYTES) {
    modeSize = pktSize;
  } else if (m_mode == QueueSizeUnit::PACKETS) {
    modeSize = 1;
  }

  NS_TEST_ASSERT_MSG_EQ(
      queue->SetAttributeFailSafe(
          "MaxSize", QueueSizeValue(QueueSize(m_mode, modeSize * 500))),
      true, "Verify that we can actually set the attribute MaxSize");
  NS_TEST_ASSERT_MSG_EQ(
      queue->SetAttributeFailSafe("UseEcn", BooleanValue(false)), true,
      "Verify that we can actually set the attribute UseEcn");
  NS_TEST_ASSERT_MSG_EQ(
      queue->SetAttributeFailSafe("BlueThreshold", TimeValue(Time::Max())),
      true, "Disable Blue enhancement");
  queue->Initialize();

  Enqueue(queue, pktSize, 20, false);
  NS_TEST_ASSERT_MSG_EQ(queue->GetCurrentSize().GetValue(), 20 * modeSize,
                        "There should be 20 packets in queue.");

  Time waitUntilFirstDequeue = 2 * queue->GetTarget();
  Simulator::Schedule(waitUntilFirstDequeue, &CobaltQueueDiscMarkTest::Dequeue,
                      this, queue, modeSize, 1);

  Time waitUntilSecondDequeue =
      waitUntilFirstDequeue + 2 * queue->GetInterval();
  Simulator::Schedule(waitUntilSecondDequeue, &CobaltQueueDiscMarkTest::Dequeue,
                      this, queue, modeSize, 1);

  Simulator::Run();
  Simulator::Destroy();

  queue = CreateObject<CobaltQueueDisc>();
  NS_TEST_ASSERT_MSG_EQ(
      queue->SetAttributeFailSafe(
          "MaxSize", QueueSizeValue(QueueSize(m_mode, modeSize * 500))),
      true, "Verify that we can actually set the attribute MaxSize");
  NS_TEST_ASSERT_MSG_EQ(
      queue->SetAttributeFailSafe("UseEcn", BooleanValue(true)), true,
      "Verify that we can actually set the attribute UseEcn");
  NS_TEST_ASSERT_MSG_EQ(
      queue->SetAttributeFailSafe("BlueThreshold", TimeValue(Time::Max())),
      true, "Disable Blue enhancement");
  queue->Initialize();

  Enqueue(queue, pktSize, 20, true);
  NS_TEST_ASSERT_MSG_EQ(queue->GetCurrentSize().GetValue(), 20 * modeSize,
                        "There should be 20 packets in queue.");

  Simulator::Schedule(waitUntilFirstDequeue, &CobaltQueueDiscMarkTest::Dequeue,
                      this, queue, modeSize, 2);

  Simulator::Schedule(waitUntilSecondDequeue, &CobaltQueueDiscMarkTest::Dequeue,
                      this, queue, modeSize, 2);

  Simulator::Schedule(waitUntilSecondDequeue, &CobaltQueueDiscMarkTest::Dequeue,
                      this, queue, modeSize, 2);

  Simulator::Schedule(waitUntilSecondDequeue * 2,
                      &CobaltQueueDiscMarkTest::Dequeue, this, queue, modeSize,
                      2);

  Simulator::Run();
  Simulator::Destroy();

  queue = CreateObject<CobaltQueueDisc>();
  NS_TEST_ASSERT_MSG_EQ(
      queue->SetAttributeFailSafe(
          "MaxSize", QueueSizeValue(QueueSize(m_mode, modeSize * 500))),
      true, "Verify that we can actually set the attribute MaxSize");
  NS_TEST_ASSERT_MSG_EQ(
      queue->SetAttributeFailSafe("UseEcn", BooleanValue(true)), true,
      "Verify that we can actually set the attribute UseEcn");
  NS_TEST_ASSERT_MSG_EQ(
      queue->SetAttributeFailSafe("BlueThreshold", TimeValue(Time::Max())),
      true, "Disable Blue enhancement");
  queue->Initialize();

  Enqueue(queue, pktSize, 3, true);
  Enqueue(queue, pktSize, 17, false);
  NS_TEST_ASSERT_MSG_EQ(queue->GetCurrentSize().GetValue(), 20 * modeSize,
                        "There should be 20 packets in queue.");

  Simulator::Schedule(waitUntilFirstDequeue, &CobaltQueueDiscMarkTest::Dequeue,
                      this, queue, modeSize, 3);

  Simulator::Schedule(waitUntilSecondDequeue, &CobaltQueueDiscMarkTest::Dequeue,
                      this, queue, modeSize, 3);

  Simulator::Schedule(waitUntilSecondDequeue, &CobaltQueueDiscMarkTest::Dequeue,
                      this, queue, modeSize, 3);

  Simulator::Schedule(waitUntilSecondDequeue * 2,
                      &CobaltQueueDiscMarkTest::Dequeue, this, queue, modeSize,
                      3);

  Simulator::Run();
  Simulator::Destroy();
}

void CobaltQueueDiscMarkTest::Enqueue(Ptr<CobaltQueueDisc> queue, uint32_t size,
                                      uint32_t nPkt, bool ecnCapable) {
  Address dest;
  for (uint32_t i = 0; i < nPkt; i++) {
    queue->Enqueue(Create<CobaltQueueDiscTestItem>(Create<Packet>(size), dest,
                                                   ecnCapable));
  }
}

void CobaltQueueDiscMarkTest::Dequeue(Ptr<CobaltQueueDisc> queue,
                                      uint32_t modeSize, uint32_t testCase) {
  uint32_t initialMarkCount =
      queue->GetStats().GetNMarkedPackets(CobaltQueueDisc::FORCED_MARK);
  uint32_t initialQSize = queue->GetCurrentSize().GetValue();
  uint32_t initialDropNext = queue->GetDropNext();
  Time currentTime = Simulator::Now();
  uint32_t currentDropCount = 0;
  uint32_t currentMarkCount = 0;

  if (initialMarkCount > 0 && currentTime.GetNanoSeconds() > initialDropNext &&
      testCase == 3) {
    queue->TraceConnectWithoutContext(
        "DropNext",
        MakeCallback(&CobaltQueueDiscMarkTest::DropNextTracer, this));
  }

  if (initialQSize != 0) {
    Ptr<QueueDiscItem> item = queue->Dequeue();
    if (testCase == 1) {
      currentDropCount = queue->GetStats().GetNDroppedPackets(
          CobaltQueueDisc::TARGET_EXCEEDED_DROP);
      if (currentDropCount != 0) {
        nPacketsBeforeFirstDrop = initialQSize;
      }
    }
    if (testCase == 2) {
      if (initialMarkCount == 0 && currentTime > queue->GetTarget()) {
        if (currentTime < queue->GetInterval()) {
          currentDropCount = queue->GetStats().GetNDroppedPackets(
              CobaltQueueDisc::TARGET_EXCEEDED_DROP);
          currentMarkCount =
              queue->GetStats().GetNMarkedPackets(CobaltQueueDisc::FORCED_MARK);
          NS_TEST_EXPECT_MSG_EQ(queue->GetCurrentSize().GetValue(),
                                initialQSize - modeSize,
                                "There should be 1 packet dequeued.");
          NS_TEST_EXPECT_MSG_EQ(currentDropCount, 0,
                                "There should not be any packet drops");
          NS_TEST_EXPECT_MSG_EQ(
              currentMarkCount, 0,
              "We are not in dropping state."
              "Sojourn time has just gone above target from below."
              "Hence, there should be no marked packets");
        } else if (currentTime >= queue->GetInterval()) {
          nPacketsBeforeFirstMark = initialQSize;
          currentDropCount = queue->GetStats().GetNDroppedPackets(
              CobaltQueueDisc::TARGET_EXCEEDED_DROP);
          currentMarkCount =
              queue->GetStats().GetNMarkedPackets(CobaltQueueDisc::FORCED_MARK);
          NS_TEST_EXPECT_MSG_EQ(
              queue->GetCurrentSize().GetValue(), initialQSize - modeSize,
              "Sojourn time has been above target for at least interval."
              "We enter the dropping state and perform initial packet marking"
              "So there should be only 1 more packet dequeued.");
          NS_TEST_EXPECT_MSG_EQ(currentDropCount, 0,
                                "There should not be any packet drops");
          NS_TEST_EXPECT_MSG_EQ(currentMarkCount, 1,
                                "There should be 1 marked packet");
        }
      } else if (initialMarkCount > 0) {
        if (currentTime.GetNanoSeconds() <= initialDropNext) {
          currentDropCount = queue->GetStats().GetNDroppedPackets(
              CobaltQueueDisc::TARGET_EXCEEDED_DROP);
          currentMarkCount =
              queue->GetStats().GetNMarkedPackets(CobaltQueueDisc::FORCED_MARK);
          NS_TEST_EXPECT_MSG_EQ(queue->GetCurrentSize().GetValue(),
                                initialQSize - modeSize,
                                "We are in dropping state."
                                "Sojourn is still above target."
                                "There should be only 1 more packet dequeued");
          NS_TEST_EXPECT_MSG_EQ(currentDropCount, 0,
                                "There should not be any packet drops");
          NS_TEST_EXPECT_MSG_EQ(currentMarkCount, 2,
                                "There should be 2 marked packet as."
                                "current dropnext is equal to current time.");
        } else if (currentTime.GetNanoSeconds() > initialDropNext) {
          currentDropCount = queue->GetStats().GetNDroppedPackets(
              CobaltQueueDisc::TARGET_EXCEEDED_DROP);
          currentMarkCount =
              queue->GetStats().GetNMarkedPackets(CobaltQueueDisc::FORCED_MARK);
          NS_TEST_EXPECT_MSG_EQ(
              queue->GetCurrentSize().GetValue(), initialQSize - modeSize,
              "We are in dropping state."
              "It's time for packet to be marked"
              "So there should be only 1 more packet dequeued");
          NS_TEST_EXPECT_MSG_EQ(currentDropCount, 0,
                                "There should not be any packet drops");
          NS_TEST_EXPECT_MSG_EQ(currentMarkCount, 3,
                                "There should be 3 marked packet");
          NS_TEST_EXPECT_MSG_EQ(
              nPacketsBeforeFirstDrop, nPacketsBeforeFirstMark,
              "Number of packets in the queue before drop should be equal"
              "to number of packets in the queue before first mark as the "
              "behavior until "
              "packet N should be the same.");
        }
      }
    } else if (testCase == 3) {
      if (initialMarkCount == 0 && currentTime > queue->GetTarget()) {
        if (currentTime < queue->GetInterval()) {
          currentDropCount = queue->GetStats().GetNDroppedPackets(
              CobaltQueueDisc::TARGET_EXCEEDED_DROP);
          currentMarkCount =
              queue->GetStats().GetNMarkedPackets(CobaltQueueDisc::FORCED_MARK);
          NS_TEST_EXPECT_MSG_EQ(queue->GetCurrentSize().GetValue(),
                                initialQSize - modeSize,
                                "There should be 1 packet dequeued.");
          NS_TEST_EXPECT_MSG_EQ(currentDropCount, 0,
                                "There should not be any packet drops");
          NS_TEST_EXPECT_MSG_EQ(
              currentMarkCount, 0,
              "We are not in dropping state."
              "Sojourn time has just gone above target from below."
              "Hence, there should be no marked packets");
        } else if (currentTime >= queue->GetInterval()) {
          currentDropCount = queue->GetStats().GetNDroppedPackets(
              CobaltQueueDisc::TARGET_EXCEEDED_DROP);
          currentMarkCount =
              queue->GetStats().GetNMarkedPackets(CobaltQueueDisc::FORCED_MARK);
          NS_TEST_EXPECT_MSG_EQ(
              queue->GetCurrentSize().GetValue(), initialQSize - modeSize,
              "Sojourn time has been above target for at least interval."
              "We enter the dropping state and perform initial packet marking"
              "So there should be only 1 more packet dequeued.");
          NS_TEST_EXPECT_MSG_EQ(currentDropCount, 0,
                                "There should not be any packet drops");
          NS_TEST_EXPECT_MSG_EQ(currentMarkCount, 1,
                                "There should be 1 marked packet");
        }
      } else if (initialMarkCount > 0) {
        if (currentTime.GetNanoSeconds() <= initialDropNext) {
          currentDropCount = queue->GetStats().GetNDroppedPackets(
              CobaltQueueDisc::TARGET_EXCEEDED_DROP);
          currentMarkCount =
              queue->GetStats().GetNMarkedPackets(CobaltQueueDisc::FORCED_MARK);
          NS_TEST_EXPECT_MSG_EQ(
              queue->GetCurrentSize().GetValue(), initialQSize - modeSize,
              "We are in dropping state."
              "Sojourn is still above target."
              "So there should be only 1 more packet dequeued");
          NS_TEST_EXPECT_MSG_EQ(currentDropCount, 0,
                                "There should not be any packet drops");
          NS_TEST_EXPECT_MSG_EQ(currentMarkCount, 2,
                                "There should be 2 marked packet"
                                "as dropnext is equal to current time");
        } else if (currentTime.GetNanoSeconds() > initialDropNext) {
          currentDropCount = queue->GetStats().GetNDroppedPackets(
              CobaltQueueDisc::TARGET_EXCEEDED_DROP);
          currentMarkCount =
              queue->GetStats().GetNMarkedPackets(CobaltQueueDisc::FORCED_MARK);
          NS_TEST_EXPECT_MSG_EQ(
              queue->GetCurrentSize().GetValue(),
              initialQSize - (m_dropNextCount + 1) * modeSize,
              "We are in dropping state."
              "It's time for packet to be dropped as packets are not ecnCapable"
              "The number of packets dequeued equals to the number of times "
              "m_dropNext "
              "is updated plus initial dequeue");
          NS_TEST_EXPECT_MSG_EQ(currentDropCount, m_dropNextCount,
                                "The number of drops equals to the number of "
                                "times m_dropNext is updated");
          NS_TEST_EXPECT_MSG_EQ(currentMarkCount, 2,
                                "There should still be only 2 marked packet");
        }
      }
    }
  }
}

class CobaltQueueDiscCeThresholdTest : public TestCase {
public:
  CobaltQueueDiscCeThresholdTest(QueueSizeUnit mode);
  ~CobaltQueueDiscCeThresholdTest() override;

private:
  void DoRun() override;
  void Enqueue(Ptr<CobaltQueueDisc> queue, uint32_t size, uint32_t nPkt);
  void EnqueueWithDelay(Ptr<CobaltQueueDisc> queue, uint32_t size,
                        uint32_t nPkt, Time delay);
  void Dequeue(Ptr<CobaltQueueDisc> queue, uint32_t modeSize);
  void DequeueWithDelay(Ptr<CobaltQueueDisc> queue, uint32_t modeSize,
                        uint32_t nPkt, Time delay);
  QueueSizeUnit m_mode;
};

CobaltQueueDiscCeThresholdTest::CobaltQueueDiscCeThresholdTest(
    QueueSizeUnit mode)
    : TestCase("Test CE Threshold marking") {
  m_mode = mode;
}

CobaltQueueDiscCeThresholdTest::~CobaltQueueDiscCeThresholdTest() {}

void CobaltQueueDiscCeThresholdTest::Enqueue(Ptr<CobaltQueueDisc> queue,
                                             uint32_t size, uint32_t nPkt) {
  Address dest;
  for (uint32_t i = 0; i < nPkt; i++) {
    queue->Enqueue(
        Create<CobaltQueueDiscTestItem>(Create<Packet>(size), dest, true));
  }
}

void CobaltQueueDiscCeThresholdTest::EnqueueWithDelay(
    Ptr<CobaltQueueDisc> queue, uint32_t size, uint32_t nPkt, Time delay) {
  for (uint32_t i = 0; i < nPkt; i++) {
    Simulator::Schedule(Time(Seconds((i + 1) * delay.GetSeconds())),
                        &CobaltQueueDiscCeThresholdTest::Enqueue, this, queue,
                        size, 1);
  }
}

void CobaltQueueDiscCeThresholdTest::Dequeue(Ptr<CobaltQueueDisc> queue,
                                             uint32_t modeSize) {
  Ptr<QueueDiscItem> item = queue->Dequeue();

  if (Simulator::Now() > MilliSeconds(11) &&
      Simulator::Now() < MilliSeconds(28)) {
    NS_TEST_EXPECT_MSG_EQ(queue->GetStats().GetNMarkedPackets(
                              CobaltQueueDisc::CE_THRESHOLD_EXCEEDED_MARK),
                          1,
                          "There should be only 1 packet"
                          "mark, the delay between the enqueueing of the "
                          "packets decreased after the"
                          "1st mark (packet enqueued at 11ms) and increased "
                          "for the packet enqueued after 20.6ms."
                          "Queue delay remains below or equal to 1ms for the "
                          "packet enqueued before 28ms");
  }
  if (Simulator::Now() > MilliSeconds(31)) {
    NS_TEST_EXPECT_MSG_EQ(queue->GetStats().GetNMarkedPackets(
                              CobaltQueueDisc::CE_THRESHOLD_EXCEEDED_MARK),
                          3,
                          "There should be 3 packet"
                          "marks, the delay between the enqueueing of the "
                          "packets decreased after 1st mark"
                          "(packet enqueued at 11ms) and increased for the "
                          "packet enqueued after 20.6ms."
                          "Queue delay remains below 1ms for the packets "
                          "enqueued before 28ms and increases"
                          "for the packets enqueued after 28ms.");
  }
}

void CobaltQueueDiscCeThresholdTest::DequeueWithDelay(
    Ptr<CobaltQueueDisc> queue, uint32_t modeSize, uint32_t nPkt, Time delay) {
  for (uint32_t i = 0; i < nPkt; i++) {
    Simulator::Schedule(Time(Seconds((i + 1) * delay.GetSeconds())),
                        &CobaltQueueDiscCeThresholdTest::Dequeue, this, queue,
                        modeSize);
  }
}

void CobaltQueueDiscCeThresholdTest::DoRun() {
  Ptr<CobaltQueueDisc> queue = CreateObject<CobaltQueueDisc>();
  uint32_t pktSize = 1000;
  uint32_t modeSize = 0;

  if (m_mode == QueueSizeUnit::BYTES) {
    modeSize = pktSize;
  } else if (m_mode == QueueSizeUnit::PACKETS) {
    modeSize = 1;
  }

  NS_TEST_ASSERT_MSG_EQ(
      queue->SetAttributeFailSafe("UseEcn", BooleanValue(true)), true,
      "Verify that we can actually set the attribute UseEcn");
  NS_TEST_ASSERT_MSG_EQ(
      queue->SetAttributeFailSafe("CeThreshold", TimeValue(MilliSeconds(1))),
      true, "Verify that we can actually set the attribute UseEcn");
  NS_TEST_ASSERT_MSG_EQ(
      queue->SetAttributeFailSafe("BlueThreshold", TimeValue(Time::Max())),
      true, "Disable Blue enhancement");
  queue->Initialize();

  EnqueueWithDelay(queue, pktSize, 11, MilliSeconds(1));

  Time dequeueInterval = MicroSeconds(1100);
  DequeueWithDelay(queue, modeSize, 11, dequeueInterval);

  Time waitUntilFirstMark = MilliSeconds(11);
  Simulator::Schedule(waitUntilFirstMark,
                      &CobaltQueueDiscCeThresholdTest::EnqueueWithDelay, this,
                      queue, pktSize, 9, MicroSeconds(1200));

  Simulator::Schedule(waitUntilFirstMark,
                      &CobaltQueueDiscCeThresholdTest::DequeueWithDelay, this,
                      queue, modeSize, 9, dequeueInterval);

  Time waitUntilDecreasingEnqueueDelay = waitUntilFirstMark + MilliSeconds(9);
  Simulator::Schedule(waitUntilDecreasingEnqueueDelay,
                      &CobaltQueueDiscCeThresholdTest::EnqueueWithDelay, this,
                      queue, pktSize, 10, MilliSeconds(1));

  Simulator::Schedule(waitUntilFirstMark,
                      &CobaltQueueDiscCeThresholdTest::DequeueWithDelay, this,
                      queue, modeSize, 10, dequeueInterval);

  Simulator::Run();
  Simulator::Destroy();
}

class CobaltQueueDiscEnhancedBlueTest : public TestCase {
public:
  CobaltQueueDiscEnhancedBlueTest(QueueSizeUnit mode);
  ~CobaltQueueDiscEnhancedBlueTest() override;

private:
  void DoRun() override;
  void Enqueue(Ptr<CobaltQueueDisc> queue, uint32_t size, uint32_t nPkt);
  void Dequeue(Ptr<CobaltQueueDisc> queue);
  void DequeueWithDelay(Ptr<CobaltQueueDisc> queue, uint32_t nPkt, Time delay);
  QueueSizeUnit m_mode;
};

CobaltQueueDiscEnhancedBlueTest::CobaltQueueDiscEnhancedBlueTest(
    QueueSizeUnit mode)
    : TestCase(
          "Enhanced Blue tests verification for both packets and bytes mode") {
  m_mode = mode;
}

CobaltQueueDiscEnhancedBlueTest::~CobaltQueueDiscEnhancedBlueTest() {}

void CobaltQueueDiscEnhancedBlueTest::DoRun()

{
  uint32_t pktSize = 1500;
  uint32_t modeSize = 0;
  Ptr<CobaltQueueDisc> queue = CreateObject<CobaltQueueDisc>();

  if (m_mode == QueueSizeUnit::BYTES) {
    modeSize = pktSize;
  } else if (m_mode == QueueSizeUnit::PACKETS) {
    modeSize = 1;
  }
  queue->Initialize();
  queue->AssignStreams(1);
  Enqueue(queue, modeSize, 200);
  DequeueWithDelay(queue, 100, MilliSeconds(10));

  Simulator::Stop(Seconds(8.0));
  Simulator::Run();

  QueueDisc::Stats st = queue->GetStats();

  NS_TEST_ASSERT_MSG_EQ(
      queue->GetPdrop(), 0.234375,
      "Pdrop should be increased by 1/256 for every packet whose sojourn time "
      "is above 400ms."
      " From the 41st dequeue until the last one, sojourn time is above 400ms, "
      "so 60 packets "
      "have sojourn time above 400ms"
      "hence Pdrop should be increased 60*(1/256) which is 0.234375");
  NS_TEST_ASSERT_MSG_EQ(
      st.GetNDroppedPackets(CobaltQueueDisc::TARGET_EXCEEDED_DROP), 49,
      "There should a fixed number of drops (49 here)");
  Simulator::Destroy();

  queue = CreateObject<CobaltQueueDisc>();
  NS_TEST_ASSERT_MSG_EQ(
      queue->SetAttributeFailSafe("UseEcn", BooleanValue(true)), true,
      "Verify that we can actually set the attribute UseEcn");
  NS_TEST_ASSERT_MSG_EQ(
      queue->SetAttributeFailSafe("BlueThreshold", TimeValue(Time::Max())),
      true, "Disable Blue enhancement");
  queue->Initialize();
  Enqueue(queue, modeSize, 200);
  DequeueWithDelay(queue, 100, MilliSeconds(10));

  Simulator::Stop(Seconds(8.0));
  Simulator::Run();

  st = queue->GetStats();

  NS_TEST_ASSERT_MSG_EQ(queue->GetPdrop(), 0, "Pdrop should be zero");
  NS_TEST_ASSERT_MSG_EQ(
      st.GetNDroppedPackets(CobaltQueueDisc::TARGET_EXCEEDED_DROP), 0,
      "There should not any dropped packets");
  Simulator::Destroy();
}

void CobaltQueueDiscEnhancedBlueTest::Enqueue(Ptr<CobaltQueueDisc> queue,
                                              uint32_t size, uint32_t nPkt) {
  Address dest;
  for (uint32_t i = 0; i < nPkt; i++) {
    queue->Enqueue(
        Create<CobaltQueueDiscTestItem>(Create<Packet>(size), dest, true));
  }
}

void CobaltQueueDiscEnhancedBlueTest::Dequeue(Ptr<CobaltQueueDisc> queue) {
  Ptr<QueueDiscItem> item = queue->Dequeue();
}

void CobaltQueueDiscEnhancedBlueTest::DequeueWithDelay(
    Ptr<CobaltQueueDisc> queue, uint32_t nPkt, Time delay) {
  for (uint32_t i = 0; i < nPkt; i++) {
    Simulator::Schedule(Time(Seconds((i + 1) * delay.GetSeconds())),
                        &CobaltQueueDiscEnhancedBlueTest::Dequeue, this, queue);
  }
}

static class CobaltQueueDiscTestSuite : public TestSuite {
public:
  CobaltQueueDiscTestSuite() : TestSuite("cobalt-queue-disc", UNIT) {
    AddTestCase(new CobaltQueueDiscBasicEnqueueDequeue(PACKETS),
                TestCase::QUICK);
    AddTestCase(new CobaltQueueDiscBasicEnqueueDequeue(BYTES), TestCase::QUICK);
    AddTestCase(new CobaltQueueDiscDropTest(), TestCase::QUICK);
    AddTestCase(new CobaltQueueDiscMarkTest(PACKETS), TestCase::QUICK);
    AddTestCase(new CobaltQueueDiscMarkTest(BYTES), TestCase::QUICK);
    AddTestCase(new CobaltQueueDiscCeThresholdTest(PACKETS), TestCase::QUICK);
    AddTestCase(new CobaltQueueDiscCeThresholdTest(BYTES), TestCase::QUICK);
    AddTestCase(new CobaltQueueDiscEnhancedBlueTest(PACKETS), TestCase::QUICK);
    AddTestCase(new CobaltQueueDiscEnhancedBlueTest(BYTES), TestCase::QUICK);
  }
} g_cobaltQueueTestSuite;
