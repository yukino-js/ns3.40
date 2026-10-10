
#include "ns3/codel-queue-disc.h"
#include "ns3/double.h"
#include "ns3/log.h"
#include "ns3/packet.h"
#include "ns3/simulator.h"
#include "ns3/string.h"
#include "ns3/test.h"
#include "ns3/uinteger.h"

using namespace ns3;

#define REC_INV_SQRT_BITS_ns3 (8 * sizeof(uint16_t))
#define REC_INV_SQRT_SHIFT_ns3 (32 - REC_INV_SQRT_BITS_ns3)

static uint16_t _codel_Newton_step(uint16_t rec_inv_sqrt, uint32_t count) {
  uint32_t invsqrt = ((uint32_t)rec_inv_sqrt) << REC_INV_SQRT_SHIFT_ns3;
  uint32_t invsqrt2 = ((uint64_t)invsqrt * invsqrt) >> 32;
  uint64_t val = (3LL << 32) - ((uint64_t)count * invsqrt2);

  val >>= 2;
  val = (val * invsqrt) >> (32 - 2 + 1);
  return static_cast<uint16_t>(val >> REC_INV_SQRT_SHIFT_ns3);
}

static uint32_t _reciprocal_scale(uint32_t val, uint32_t ep_ro) {
  return (uint32_t)(((uint64_t)val * ep_ro) >> 32);
}

class CodelQueueDiscTestItem : public QueueDiscItem {
public:
  CodelQueueDiscTestItem(Ptr<Packet> p, const Address &addr, bool ecnCapable);
  ~CodelQueueDiscTestItem() override;

  CodelQueueDiscTestItem() = delete;
  CodelQueueDiscTestItem(const CodelQueueDiscTestItem &) = delete;
  CodelQueueDiscTestItem &operator=(const CodelQueueDiscTestItem &) = delete;

  void AddHeader() override;
  bool Mark() override;

private:
  bool m_ecnCapablePacket;
};

CodelQueueDiscTestItem::CodelQueueDiscTestItem(Ptr<Packet> p,
                                               const Address &addr,
                                               bool ecnCapable)
    : QueueDiscItem(p, addr, 0), m_ecnCapablePacket(ecnCapable) {}

CodelQueueDiscTestItem::~CodelQueueDiscTestItem() {}

void CodelQueueDiscTestItem::AddHeader() {}

bool CodelQueueDiscTestItem::Mark() { return m_ecnCapablePacket; }

class CoDelQueueDiscBasicEnqueueDequeue : public TestCase {
public:
  CoDelQueueDiscBasicEnqueueDequeue(QueueSizeUnit mode);
  void DoRun() override;

private:
  QueueSizeUnit m_mode;
};

CoDelQueueDiscBasicEnqueueDequeue::CoDelQueueDiscBasicEnqueueDequeue(
    QueueSizeUnit mode)
    : TestCase("Basic enqueue and dequeue operations, and attribute setting") {
  m_mode = mode;
}

void CoDelQueueDiscBasicEnqueueDequeue::DoRun() {
  Ptr<CoDelQueueDisc> queue = CreateObject<CoDelQueueDisc>();

  uint32_t pktSize = 1000;
  uint32_t modeSize = 0;

  Address dest;

  NS_TEST_ASSERT_MSG_EQ(
      queue->SetAttributeFailSafe("MinBytes", UintegerValue(pktSize)), true,
      "Verify that we can actually set the attribute MinBytes");
  NS_TEST_ASSERT_MSG_EQ(
      queue->SetAttributeFailSafe("Interval", StringValue("50ms")), true,
      "Verify that we can actually set the attribute Interval");
  NS_TEST_ASSERT_MSG_EQ(
      queue->SetAttributeFailSafe("Target", StringValue("4ms")), true,
      "Verify that we can actually set the attribute Target");

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
  queue->Enqueue(Create<CodelQueueDiscTestItem>(p1, dest, false));
  NS_TEST_ASSERT_MSG_EQ(queue->GetCurrentSize().GetValue(), 1 * modeSize,
                        "There should be one packet in queue");
  queue->Enqueue(Create<CodelQueueDiscTestItem>(p2, dest, false));
  NS_TEST_ASSERT_MSG_EQ(queue->GetCurrentSize().GetValue(), 2 * modeSize,
                        "There should be two packets in queue");
  queue->Enqueue(Create<CodelQueueDiscTestItem>(p3, dest, false));
  NS_TEST_ASSERT_MSG_EQ(queue->GetCurrentSize().GetValue(), 3 * modeSize,
                        "There should be three packets in queue");
  queue->Enqueue(Create<CodelQueueDiscTestItem>(p4, dest, false));
  NS_TEST_ASSERT_MSG_EQ(queue->GetCurrentSize().GetValue(), 4 * modeSize,
                        "There should be four packets in queue");
  queue->Enqueue(Create<CodelQueueDiscTestItem>(p5, dest, false));
  NS_TEST_ASSERT_MSG_EQ(queue->GetCurrentSize().GetValue(), 5 * modeSize,
                        "There should be five packets in queue");
  queue->Enqueue(Create<CodelQueueDiscTestItem>(p6, dest, false));
  NS_TEST_ASSERT_MSG_EQ(queue->GetCurrentSize().GetValue(), 6 * modeSize,
                        "There should be six packets in queue");

  NS_TEST_ASSERT_MSG_EQ(
      queue->GetStats().GetNDroppedPackets(CoDelQueueDisc::OVERLIMIT_DROP), 0,
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
          CoDelQueueDisc::TARGET_EXCEEDED_DROP),
      0, "There should be no packet drops according to CoDel algorithm");
}

class CoDelQueueDiscBasicOverflow : public TestCase {
public:
  CoDelQueueDiscBasicOverflow(QueueSizeUnit mode);
  void DoRun() override;

private:
  void Enqueue(Ptr<CoDelQueueDisc> queue, uint32_t size, uint32_t nPkt);
  QueueSizeUnit m_mode;
};

CoDelQueueDiscBasicOverflow::CoDelQueueDiscBasicOverflow(QueueSizeUnit mode)
    : TestCase("Basic overflow behavior") {
  m_mode = mode;
}

void CoDelQueueDiscBasicOverflow::DoRun() {
  Ptr<CoDelQueueDisc> queue = CreateObject<CoDelQueueDisc>();
  uint32_t pktSize = 1000;
  uint32_t modeSize = 0;

  Address dest;

  if (m_mode == QueueSizeUnit::BYTES) {
    modeSize = pktSize;
  } else if (m_mode == QueueSizeUnit::PACKETS) {
    modeSize = 1;
  }

  Ptr<Packet> p1;
  Ptr<Packet> p2;
  Ptr<Packet> p3;
  p1 = Create<Packet>(pktSize);
  p2 = Create<Packet>(pktSize);
  p3 = Create<Packet>(pktSize);

  NS_TEST_ASSERT_MSG_EQ(
      queue->SetAttributeFailSafe(
          "MaxSize", QueueSizeValue(QueueSize(m_mode, modeSize * 500))),
      true, "Verify that we can actually set the attribute MaxSize");
  NS_TEST_ASSERT_MSG_EQ(
      queue->SetAttributeFailSafe("MinBytes", UintegerValue(pktSize)), true,
      "Verify that we can actually set the attribute MinBytes");

  queue->Initialize();

  Enqueue(queue, pktSize, 500);
  queue->Enqueue(Create<CodelQueueDiscTestItem>(p1, dest, false));
  queue->Enqueue(Create<CodelQueueDiscTestItem>(p2, dest, false));
  queue->Enqueue(Create<CodelQueueDiscTestItem>(p3, dest, false));

  NS_TEST_ASSERT_MSG_EQ(queue->GetCurrentSize().GetValue(), 500 * modeSize,
                        "There should be 500 packets in queue");
  NS_TEST_ASSERT_MSG_EQ(
      queue->GetStats().GetNDroppedPackets(CoDelQueueDisc::OVERLIMIT_DROP), 3,
      "There should be three packets being dropped due to full queue");
}

void CoDelQueueDiscBasicOverflow::Enqueue(Ptr<CoDelQueueDisc> queue,
                                          uint32_t size, uint32_t nPkt) {
  Address dest;
  for (uint32_t i = 0; i < nPkt; i++) {
    queue->Enqueue(
        Create<CodelQueueDiscTestItem>(Create<Packet>(size), dest, false));
  }
}

class CoDelQueueDiscNewtonStepTest : public TestCase {
public:
  CoDelQueueDiscNewtonStepTest();
  void DoRun() override;
};

CoDelQueueDiscNewtonStepTest::CoDelQueueDiscNewtonStepTest()
    : TestCase("NewtonStep arithmetic unit test") {}

void CoDelQueueDiscNewtonStepTest::DoRun() {
  Ptr<CoDelQueueDisc> queue = CreateObject<CoDelQueueDisc>();

  uint16_t result;
  for (uint16_t recInvSqrt = 0xff; recInvSqrt > 0; recInvSqrt /= 2) {
    for (uint32_t count = 1; count < 0xff; count *= 2) {
      result = queue->NewtonStep(recInvSqrt, count);
      NS_TEST_ASSERT_MSG_EQ(
          _codel_Newton_step(recInvSqrt, count), result,
          "ns-3 NewtonStep() fails to match Linux equivalent");
    }
  }
}

class CoDelQueueDiscControlLawTest : public TestCase {
public:
  CoDelQueueDiscControlLawTest();
  void DoRun() override;
  uint32_t _codel_control_law(uint32_t t, uint32_t interval,
                              uint32_t recInvSqrt);
};

CoDelQueueDiscControlLawTest::CoDelQueueDiscControlLawTest()
    : TestCase("ControlLaw arithmetic unit test") {}

uint32_t CoDelQueueDiscControlLawTest::_codel_control_law(uint32_t t,
                                                          uint32_t interval,
                                                          uint32_t recInvSqrt) {
  return t + _reciprocal_scale(interval, recInvSqrt << REC_INV_SQRT_SHIFT_ns3);
}

void CoDelQueueDiscControlLawTest::DoRun() {
  Ptr<CoDelQueueDisc> queue = CreateObject<CoDelQueueDisc>();

  uint32_t interval = queue->Time2CoDel(MilliSeconds(100));

  uint32_t codelTimeVal;
  for (Time timeVal = Seconds(0); timeVal <= Seconds(20);
       timeVal += MilliSeconds(100)) {
    for (uint16_t recInvSqrt = 0xff; recInvSqrt > 0; recInvSqrt /= 2) {
      codelTimeVal = queue->Time2CoDel(timeVal);
      uint32_t ns3Result =
          queue->ControlLaw(codelTimeVal, interval, recInvSqrt);
      uint32_t linuxResult =
          _codel_control_law(codelTimeVal, interval, recInvSqrt);
      NS_TEST_ASSERT_MSG_EQ(
          ns3Result, linuxResult,
          "Linux result for ControlLaw should equal ns-3 result");
    }
  }
}

class CoDelQueueDiscBasicDrop : public TestCase {
public:
  CoDelQueueDiscBasicDrop(QueueSizeUnit mode);
  void DoRun() override;

private:
  void Enqueue(Ptr<CoDelQueueDisc> queue, uint32_t size, uint32_t nPkt);
  void Dequeue(Ptr<CoDelQueueDisc> queue, uint32_t modeSize);
  void DropNextTracer(uint32_t oldVal, uint32_t newVal);
  QueueSizeUnit m_mode;
  uint32_t m_dropNextCount;
};

CoDelQueueDiscBasicDrop::CoDelQueueDiscBasicDrop(QueueSizeUnit mode)
    : TestCase("Basic drop operations") {
  m_mode = mode;
  m_dropNextCount = 0;
}

void CoDelQueueDiscBasicDrop::DropNextTracer(uint32_t, uint32_t) {
  m_dropNextCount++;
}

void CoDelQueueDiscBasicDrop::DoRun() {
  Ptr<CoDelQueueDisc> queue = CreateObject<CoDelQueueDisc>();
  uint32_t pktSize = 1000;
  uint32_t modeSize = 0;

  if (m_mode == QueueSizeUnit::BYTES) {
    modeSize = pktSize;
  } else if (m_mode == QueueSizeUnit::PACKETS) {
    modeSize = 1;
  }

  NS_TEST_ASSERT_MSG_EQ(
      queue->SetAttributeFailSafe(
          "MaxSize", QueueSizeValue(QueueSize(m_mode, modeSize * 500))),
      true, "Verify that we can actually set the attribute MaxSize");

  queue->Initialize();

  Enqueue(queue, pktSize, 20);
  NS_TEST_ASSERT_MSG_EQ(queue->GetCurrentSize().GetValue(), 20 * modeSize,
                        "There should be 20 packets in queue.");

  Time waitUntilFirstDequeue = 2 * queue->GetTarget();
  Simulator::Schedule(waitUntilFirstDequeue, &CoDelQueueDiscBasicDrop::Dequeue,
                      this, queue, modeSize);

  Time waitUntilSecondDequeue =
      waitUntilFirstDequeue + 2 * queue->GetInterval();
  Simulator::Schedule(waitUntilSecondDequeue, &CoDelQueueDiscBasicDrop::Dequeue,
                      this, queue, modeSize);

  Simulator::Schedule(waitUntilSecondDequeue, &CoDelQueueDiscBasicDrop::Dequeue,
                      this, queue, modeSize);

  Simulator::Schedule(waitUntilSecondDequeue * 2,
                      &CoDelQueueDiscBasicDrop::Dequeue, this, queue, modeSize);

  Simulator::Run();
  Simulator::Destroy();
}

void CoDelQueueDiscBasicDrop::Enqueue(Ptr<CoDelQueueDisc> queue, uint32_t size,
                                      uint32_t nPkt) {
  Address dest;
  for (uint32_t i = 0; i < nPkt; i++) {
    queue->Enqueue(
        Create<CodelQueueDiscTestItem>(Create<Packet>(size), dest, false));
  }
}

void CoDelQueueDiscBasicDrop::Dequeue(Ptr<CoDelQueueDisc> queue,
                                      uint32_t modeSize) {
  uint32_t initialDropCount = queue->GetStats().GetNDroppedPackets(
      CoDelQueueDisc::TARGET_EXCEEDED_DROP);
  uint32_t initialQSize = queue->GetCurrentSize().GetValue();
  uint32_t initialDropNext = queue->GetDropNext();
  Time currentTime = Simulator::Now();
  uint32_t currentDropCount = 0;

  if (initialDropCount > 0 &&
      currentTime.GetMicroSeconds() >= initialDropNext) {
    queue->TraceConnectWithoutContext(
        "DropNext",
        MakeCallback(&CoDelQueueDiscBasicDrop::DropNextTracer, this));
  }

  if (initialQSize != 0) {
    Ptr<QueueDiscItem> item = queue->Dequeue();
    if (initialDropCount == 0 && currentTime > queue->GetTarget()) {
      if (currentTime < queue->GetInterval()) {
        currentDropCount = queue->GetStats().GetNDroppedPackets(
            CoDelQueueDisc::TARGET_EXCEEDED_DROP);
        NS_TEST_EXPECT_MSG_EQ(
            currentDropCount, 0,
            "We are not in dropping state."
            "Sojourn time has just gone above target from below."
            "Hence, there should be no packet drops");
        NS_TEST_EXPECT_MSG_EQ(queue->GetCurrentSize().GetValue(),
                              initialQSize - modeSize,
                              "There should be 1 packet dequeued.");
      } else if (currentTime >= queue->GetInterval()) {
        currentDropCount = queue->GetStats().GetNDroppedPackets(
            CoDelQueueDisc::TARGET_EXCEEDED_DROP);
        NS_TEST_EXPECT_MSG_EQ(
            queue->GetCurrentSize().GetValue(), initialQSize - 2 * modeSize,
            "Sojourn time has been above target for at least interval."
            "We enter the dropping state, perform initial packet drop, "
            "and dequeue the next."
            "So there should be 2 more packets dequeued.");
        NS_TEST_EXPECT_MSG_EQ(currentDropCount, 1,
                              "There should be 1 packet drop");
      }
    } else if (initialDropCount > 0) {
      if (currentTime.GetMicroSeconds() < initialDropNext) {
        currentDropCount = queue->GetStats().GetNDroppedPackets(
            CoDelQueueDisc::TARGET_EXCEEDED_DROP);
        NS_TEST_EXPECT_MSG_EQ(queue->GetCurrentSize().GetValue(),
                              initialQSize - modeSize,
                              "We are in dropping state."
                              "Sojourn is still above target."
                              "However, it's not time for next drop."
                              "So there should be only 1 more packet dequeued");

        NS_TEST_EXPECT_MSG_EQ(
            currentDropCount, 1,
            "There should still be only 1 packet drop from the last dequeue");
      } else if (currentTime.GetMicroSeconds() >= initialDropNext) {
        currentDropCount = queue->GetStats().GetNDroppedPackets(
            CoDelQueueDisc::TARGET_EXCEEDED_DROP);
        NS_TEST_EXPECT_MSG_EQ(
            queue->GetCurrentSize().GetValue(),
            initialQSize - (m_dropNextCount + 1) * modeSize,
            "We are in dropping state."
            "It's time for next drop."
            "The number of packets dequeued equals to the number of "
            "times m_dropNext is updated plus initial dequeue");
        NS_TEST_EXPECT_MSG_EQ(
            currentDropCount, 1 + m_dropNextCount,
            "The number of drops equals to the number of times "
            "m_dropNext is updated plus 1 from last dequeue");
      }
    }
  }
}

class CoDelQueueDiscBasicMark : public TestCase {
public:
  CoDelQueueDiscBasicMark(QueueSizeUnit mode);
  void DoRun() override;

private:
  void Enqueue(Ptr<CoDelQueueDisc> queue, uint32_t size, uint32_t nPkt,
               bool ecnCapable);
  void Dequeue(Ptr<CoDelQueueDisc> queue, uint32_t modeSize, uint32_t testCase);
  void DropNextTracer(uint32_t oldVal, uint32_t newVal);
  QueueSizeUnit m_mode;
  uint32_t m_dropNextCount;
  uint32_t nPacketsBeforeFirstDrop;
  uint32_t nPacketsBeforeFirstMark;
};

CoDelQueueDiscBasicMark::CoDelQueueDiscBasicMark(QueueSizeUnit mode)
    : TestCase("Basic mark operations") {
  m_mode = mode;
  m_dropNextCount = 0;
}

void CoDelQueueDiscBasicMark::DropNextTracer(uint32_t, uint32_t) {
  m_dropNextCount++;
}

void CoDelQueueDiscBasicMark::DoRun() {

  Ptr<CoDelQueueDisc> queue = CreateObject<CoDelQueueDisc>();
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
      queue->SetAttributeFailSafe("UseEcn", BooleanValue(true)), true,
      "Verify that we can actually set the attribute UseEcn");

  queue->Initialize();

  Enqueue(queue, pktSize, 20, false);
  NS_TEST_ASSERT_MSG_EQ(queue->GetCurrentSize().GetValue(), 20 * modeSize,
                        "There should be 20 packets in queue.");

  Time waitUntilFirstDequeue = 2 * queue->GetTarget();
  Simulator::Schedule(waitUntilFirstDequeue, &CoDelQueueDiscBasicMark::Dequeue,
                      this, queue, modeSize, 1);

  Time waitUntilSecondDequeue =
      waitUntilFirstDequeue + 2 * queue->GetInterval();
  Simulator::Schedule(waitUntilSecondDequeue, &CoDelQueueDiscBasicMark::Dequeue,
                      this, queue, modeSize, 1);

  Simulator::Run();
  Simulator::Destroy();

  queue = CreateObject<CoDelQueueDisc>();
  NS_TEST_ASSERT_MSG_EQ(
      queue->SetAttributeFailSafe(
          "MaxSize", QueueSizeValue(QueueSize(m_mode, modeSize * 500))),
      true, "Verify that we can actually set the attribute MaxSize");
  NS_TEST_ASSERT_MSG_EQ(
      queue->SetAttributeFailSafe("UseEcn", BooleanValue(true)), true,
      "Verify that we can actually set the attribute UseEcn");

  queue->Initialize();

  Enqueue(queue, pktSize, 20, true);
  NS_TEST_ASSERT_MSG_EQ(queue->GetCurrentSize().GetValue(), 20 * modeSize,
                        "There should be 20 packets in queue.");

  Simulator::Schedule(waitUntilFirstDequeue, &CoDelQueueDiscBasicMark::Dequeue,
                      this, queue, modeSize, 2);

  Simulator::Schedule(waitUntilSecondDequeue, &CoDelQueueDiscBasicMark::Dequeue,
                      this, queue, modeSize, 2);

  Simulator::Schedule(waitUntilSecondDequeue, &CoDelQueueDiscBasicMark::Dequeue,
                      this, queue, modeSize, 2);

  Simulator::Schedule(waitUntilSecondDequeue * 2,
                      &CoDelQueueDiscBasicMark::Dequeue, this, queue, modeSize,
                      2);

  Simulator::Run();
  Simulator::Destroy();

  queue = CreateObject<CoDelQueueDisc>();
  NS_TEST_ASSERT_MSG_EQ(
      queue->SetAttributeFailSafe(
          "MaxSize", QueueSizeValue(QueueSize(m_mode, modeSize * 500))),
      true, "Verify that we can actually set the attribute MaxSize");
  NS_TEST_ASSERT_MSG_EQ(
      queue->SetAttributeFailSafe("UseEcn", BooleanValue(true)), true,
      "Verify that we can actually set the attribute UseEcn");
  NS_TEST_ASSERT_MSG_EQ(
      queue->SetAttributeFailSafe("CeThreshold", TimeValue(MilliSeconds(2))),
      true, "Verify that we can actually set the attribute CeThreshold");

  queue->Initialize();

  Enqueue(queue, pktSize, 3, true);
  Enqueue(queue, pktSize, 17, false);
  NS_TEST_ASSERT_MSG_EQ(queue->GetCurrentSize().GetValue(), 20 * modeSize,
                        "There should be 20 packets in queue.");

  Simulator::Schedule(waitUntilFirstDequeue, &CoDelQueueDiscBasicMark::Dequeue,
                      this, queue, modeSize, 3);

  Simulator::Schedule(waitUntilSecondDequeue, &CoDelQueueDiscBasicMark::Dequeue,
                      this, queue, modeSize, 3);

  Simulator::Schedule(waitUntilSecondDequeue, &CoDelQueueDiscBasicMark::Dequeue,
                      this, queue, modeSize, 3);

  Simulator::Schedule(waitUntilSecondDequeue * 2,
                      &CoDelQueueDiscBasicMark::Dequeue, this, queue, modeSize,
                      3);

  Simulator::Run();
  Simulator::Destroy();

  queue = CreateObject<CoDelQueueDisc>();
  NS_TEST_ASSERT_MSG_EQ(
      queue->SetAttributeFailSafe(
          "MaxSize", QueueSizeValue(QueueSize(m_mode, modeSize * 500))),
      true, "Verify that we can actually set the attribute MaxSize");
  NS_TEST_ASSERT_MSG_EQ(
      queue->SetAttributeFailSafe("UseEcn", BooleanValue(true)), true,
      "Verify that we can actually set the attribute UseEcn");
  NS_TEST_ASSERT_MSG_EQ(
      queue->SetAttributeFailSafe("CeThreshold", TimeValue(MilliSeconds(2))),
      true, "Verify that we can actually set the attribute CeThreshold");

  queue->Initialize();

  Enqueue(queue, pktSize, 20, true);
  NS_TEST_ASSERT_MSG_EQ(queue->GetCurrentSize().GetValue(), 20 * modeSize,
                        "There should be 20 packets in queue.");

  Simulator::Schedule(MilliSeconds(1), &CoDelQueueDiscBasicMark::Dequeue, this,
                      queue, modeSize, 4);

  Simulator::Schedule(MilliSeconds(3), &CoDelQueueDiscBasicMark::Dequeue, this,
                      queue, modeSize, 4);

  Simulator::Schedule(waitUntilFirstDequeue, &CoDelQueueDiscBasicMark::Dequeue,
                      this, queue, modeSize, 4);

  Simulator::Schedule(waitUntilSecondDequeue, &CoDelQueueDiscBasicMark::Dequeue,
                      this, queue, modeSize, 4);

  Simulator::Run();
  Simulator::Destroy();
}

void CoDelQueueDiscBasicMark::Enqueue(Ptr<CoDelQueueDisc> queue, uint32_t size,
                                      uint32_t nPkt, bool ecnCapable) {
  Address dest;
  for (uint32_t i = 0; i < nPkt; i++) {
    queue->Enqueue(
        Create<CodelQueueDiscTestItem>(Create<Packet>(size), dest, ecnCapable));
  }
}

void CoDelQueueDiscBasicMark::Dequeue(Ptr<CoDelQueueDisc> queue,
                                      uint32_t modeSize, uint32_t testCase) {
  uint32_t initialTargetMarkCount =
      queue->GetStats().GetNMarkedPackets(CoDelQueueDisc::TARGET_EXCEEDED_MARK);
  uint32_t initialCeThreshMarkCount = queue->GetStats().GetNMarkedPackets(
      CoDelQueueDisc::CE_THRESHOLD_EXCEEDED_MARK);
  uint32_t initialQSize = queue->GetCurrentSize().GetValue();
  uint32_t initialDropNext = queue->GetDropNext();
  Time currentTime = Simulator::Now();
  uint32_t currentDropCount = 0;
  uint32_t currentTargetMarkCount = 0;
  uint32_t currentCeThreshMarkCount = 0;

  if (initialTargetMarkCount > 0 &&
      currentTime.GetMicroSeconds() >= initialDropNext && testCase == 3) {
    queue->TraceConnectWithoutContext(
        "DropNext",
        MakeCallback(&CoDelQueueDiscBasicMark::DropNextTracer, this));
  }

  if (initialQSize != 0) {
    Ptr<QueueDiscItem> item = queue->Dequeue();
    if (testCase == 1) {
      currentDropCount = queue->GetStats().GetNDroppedPackets(
          CoDelQueueDisc::TARGET_EXCEEDED_DROP);
      if (currentDropCount == 1) {
        nPacketsBeforeFirstDrop = initialQSize;
      }
    } else if (testCase == 2) {
      if (initialTargetMarkCount == 0 && currentTime > queue->GetTarget()) {
        if (currentTime < queue->GetInterval()) {
          currentDropCount = queue->GetStats().GetNDroppedPackets(
              CoDelQueueDisc::TARGET_EXCEEDED_DROP);
          currentTargetMarkCount = queue->GetStats().GetNMarkedPackets(
              CoDelQueueDisc::TARGET_EXCEEDED_MARK);
          currentCeThreshMarkCount = queue->GetStats().GetNMarkedPackets(
              CoDelQueueDisc::CE_THRESHOLD_EXCEEDED_MARK);
          NS_TEST_ASSERT_MSG_EQ(queue->GetCurrentSize().GetValue(),
                                initialQSize - modeSize,
                                "There should be 1 packet dequeued.");
          NS_TEST_ASSERT_MSG_EQ(currentDropCount, 0,
                                "There should not be any packet drops");
          NS_TEST_ASSERT_MSG_EQ(
              currentTargetMarkCount, 0,
              "We are not in dropping state."
              "Sojourn time has just gone above target from below."
              "Hence, there should be no target exceeded marked packets");
          NS_TEST_ASSERT_MSG_EQ(currentCeThreshMarkCount, 0,
                                "Marking due to CE threshold is disabled"
                                "Hence, there should not be any CE threshold "
                                "exceeded marked packet");
        } else if (currentTime >= queue->GetInterval()) {
          currentDropCount = queue->GetStats().GetNDroppedPackets(
              CoDelQueueDisc::TARGET_EXCEEDED_DROP);
          currentTargetMarkCount = queue->GetStats().GetNMarkedPackets(
              CoDelQueueDisc::TARGET_EXCEEDED_MARK);
          nPacketsBeforeFirstMark = initialQSize;
          currentCeThreshMarkCount = queue->GetStats().GetNMarkedPackets(
              CoDelQueueDisc::CE_THRESHOLD_EXCEEDED_MARK);
          NS_TEST_ASSERT_MSG_EQ(
              queue->GetCurrentSize().GetValue(), initialQSize - modeSize,
              "Sojourn time has been above target for at least interval."
              "We enter the dropping state and perform initial packet marking"
              "So there should be only 1 more packet dequeued.");
          NS_TEST_ASSERT_MSG_EQ(currentDropCount, 0,
                                "There should not be any packet drops");
          NS_TEST_ASSERT_MSG_EQ(
              currentTargetMarkCount, 1,
              "There should be 1 target exceeded marked packet");
          NS_TEST_ASSERT_MSG_EQ(
              currentCeThreshMarkCount, 0,
              "There should not be any CE threshold exceeded marked packet");
        }
      } else if (initialTargetMarkCount > 0) {
        if (currentTime.GetMicroSeconds() < initialDropNext) {
          currentDropCount = queue->GetStats().GetNDroppedPackets(
              CoDelQueueDisc::TARGET_EXCEEDED_DROP);
          currentTargetMarkCount = queue->GetStats().GetNMarkedPackets(
              CoDelQueueDisc::TARGET_EXCEEDED_MARK);
          currentCeThreshMarkCount = queue->GetStats().GetNMarkedPackets(
              CoDelQueueDisc::CE_THRESHOLD_EXCEEDED_MARK);
          NS_TEST_ASSERT_MSG_EQ(
              queue->GetCurrentSize().GetValue(), initialQSize - modeSize,
              "We are in dropping state."
              "Sojourn is still above target."
              "However, it's not time for next target exceeded mark."
              "So there should be only 1 more packet dequeued");
          NS_TEST_ASSERT_MSG_EQ(currentDropCount, 0,
                                "There should not be any packet drops");
          NS_TEST_ASSERT_MSG_EQ(
              currentTargetMarkCount, 1,
              "There should still be only 1 target exceeded marked "
              "packet from the last dequeue");
          NS_TEST_ASSERT_MSG_EQ(
              currentCeThreshMarkCount, 0,
              "There should not be any CE threshold exceeded marked packet");
        } else if (currentTime.GetMicroSeconds() >= initialDropNext) {
          currentDropCount = queue->GetStats().GetNDroppedPackets(
              CoDelQueueDisc::TARGET_EXCEEDED_DROP);
          currentTargetMarkCount = queue->GetStats().GetNMarkedPackets(
              CoDelQueueDisc::TARGET_EXCEEDED_MARK);
          currentCeThreshMarkCount = queue->GetStats().GetNMarkedPackets(
              CoDelQueueDisc::CE_THRESHOLD_EXCEEDED_MARK);
          NS_TEST_ASSERT_MSG_EQ(
              queue->GetCurrentSize().GetValue(), initialQSize - modeSize,
              "We are in dropping state."
              "It's time for packet to be marked"
              "So there should be only 1 more packet dequeued");
          NS_TEST_ASSERT_MSG_EQ(currentDropCount, 0,
                                "There should not be any packet drops");
          NS_TEST_ASSERT_MSG_EQ(currentTargetMarkCount, 2,
                                "There should 2 target exceeded marked packet");
          NS_TEST_ASSERT_MSG_EQ(
              nPacketsBeforeFirstDrop, nPacketsBeforeFirstMark,
              "Number of packets in the queue before drop should be equal"
              "to number of packets in the queue before first mark as the "
              "behavior "
              "until packet N should be the same.");
          NS_TEST_ASSERT_MSG_EQ(
              currentCeThreshMarkCount, 0,
              "There should not be any CE threshold exceeded marked packet");
        }
      }
    } else if (testCase == 3) {
      if (initialTargetMarkCount == 0 && currentTime > queue->GetTarget()) {
        if (currentTime < queue->GetInterval()) {
          currentDropCount = queue->GetStats().GetNDroppedPackets(
              CoDelQueueDisc::TARGET_EXCEEDED_DROP);
          currentTargetMarkCount = queue->GetStats().GetNMarkedPackets(
              CoDelQueueDisc::TARGET_EXCEEDED_MARK);
          currentCeThreshMarkCount = queue->GetStats().GetNMarkedPackets(
              CoDelQueueDisc::CE_THRESHOLD_EXCEEDED_MARK);
          NS_TEST_ASSERT_MSG_EQ(queue->GetCurrentSize().GetValue(),
                                initialQSize - modeSize,
                                "There should be 1 packet dequeued.");
          NS_TEST_ASSERT_MSG_EQ(currentDropCount, 0,
                                "There should not be any packet drops");
          NS_TEST_ASSERT_MSG_EQ(
              currentTargetMarkCount, 0,
              "We are not in dropping state."
              "Sojourn time has just gone above target from below."
              "Hence, there should be no target exceeded marked packets");
          NS_TEST_ASSERT_MSG_EQ(
              currentCeThreshMarkCount, 1,
              "Sojourn time has gone above CE threshold."
              "Hence, there should be 1 CE threshold exceeded marked packet");
        } else if (currentTime >= queue->GetInterval()) {
          currentDropCount = queue->GetStats().GetNDroppedPackets(
              CoDelQueueDisc::TARGET_EXCEEDED_DROP);
          currentTargetMarkCount = queue->GetStats().GetNMarkedPackets(
              CoDelQueueDisc::TARGET_EXCEEDED_MARK);
          currentCeThreshMarkCount = queue->GetStats().GetNMarkedPackets(
              CoDelQueueDisc::CE_THRESHOLD_EXCEEDED_MARK);
          NS_TEST_ASSERT_MSG_EQ(
              queue->GetCurrentSize().GetValue(), initialQSize - modeSize,
              "Sojourn time has been above target for at least interval."
              "We enter the dropping state and perform initial packet marking"
              "So there should be only 1 more packet dequeued.");
          NS_TEST_ASSERT_MSG_EQ(currentDropCount, 0,
                                "There should not be any packet drops");
          NS_TEST_ASSERT_MSG_EQ(
              currentTargetMarkCount, 1,
              "There should be 1 target exceeded marked packet");
          NS_TEST_ASSERT_MSG_EQ(
              currentCeThreshMarkCount, 1,
              "There should be 1 CE threshold exceeded marked packets");
        }
      } else if (initialTargetMarkCount > 0) {
        if (currentTime.GetMicroSeconds() < initialDropNext) {
          currentDropCount = queue->GetStats().GetNDroppedPackets(
              CoDelQueueDisc::TARGET_EXCEEDED_DROP);
          currentTargetMarkCount = queue->GetStats().GetNMarkedPackets(
              CoDelQueueDisc::TARGET_EXCEEDED_MARK);
          currentCeThreshMarkCount = queue->GetStats().GetNMarkedPackets(
              CoDelQueueDisc::CE_THRESHOLD_EXCEEDED_MARK);
          NS_TEST_ASSERT_MSG_EQ(
              queue->GetCurrentSize().GetValue(), initialQSize - modeSize,
              "We are in dropping state."
              "Sojourn is still above target."
              "However, it's not time for next target exceeded mark."
              "So there should be only 1 more packet dequeued");
          NS_TEST_ASSERT_MSG_EQ(currentDropCount, 0,
                                "There should not be any packet drops");
          NS_TEST_ASSERT_MSG_EQ(
              currentTargetMarkCount, 1,
              "There should still be only 1 target exceeded marked "
              "packet from the last dequeue");
          NS_TEST_ASSERT_MSG_EQ(
              currentCeThreshMarkCount, 2,
              "There should be 2 CE threshold exceeded marked packets");
        } else if (currentTime.GetMicroSeconds() >= initialDropNext) {
          currentDropCount = queue->GetStats().GetNDroppedPackets(
              CoDelQueueDisc::TARGET_EXCEEDED_DROP);
          currentTargetMarkCount = queue->GetStats().GetNMarkedPackets(
              CoDelQueueDisc::TARGET_EXCEEDED_MARK);
          currentCeThreshMarkCount = queue->GetStats().GetNMarkedPackets(
              CoDelQueueDisc::CE_THRESHOLD_EXCEEDED_MARK);
          NS_TEST_ASSERT_MSG_EQ(
              queue->GetCurrentSize().GetValue(),
              initialQSize - (m_dropNextCount + 1) * modeSize,
              "We are in dropping state."
              "It's time for packet to be dropped as packets are not ecnCapable"
              "The number of packets dequeued equals to the number of times "
              "m_dropNext "
              "is updated plus initial dequeue");
          NS_TEST_ASSERT_MSG_EQ(currentDropCount, m_dropNextCount,
                                "The number of drops equals to the number of "
                                "times m_dropNext is updated");
          NS_TEST_ASSERT_MSG_EQ(
              currentTargetMarkCount, 1,
              "There should still be only 1 target exceeded marked packet");
          NS_TEST_ASSERT_MSG_EQ(
              currentCeThreshMarkCount, 2,
              "There should still be 2 CE threshold exceeded marked "
              "packet as packets are not ecnCapable");
        }
      }
    } else if (testCase == 4) {
      if (currentTime < queue->GetTarget()) {
        if (initialCeThreshMarkCount == 0 && currentTime < MilliSeconds(2)) {
          currentDropCount = queue->GetStats().GetNDroppedPackets(
              CoDelQueueDisc::TARGET_EXCEEDED_DROP);
          currentCeThreshMarkCount = queue->GetStats().GetNMarkedPackets(
              CoDelQueueDisc::CE_THRESHOLD_EXCEEDED_MARK);
          NS_TEST_ASSERT_MSG_EQ(queue->GetCurrentSize().GetValue(),
                                initialQSize - modeSize,
                                "There should be 1 packet dequeued.");
          NS_TEST_ASSERT_MSG_EQ(currentDropCount, 0,
                                "There should not be any packet drops");
          NS_TEST_ASSERT_MSG_EQ(currentCeThreshMarkCount, 0,
                                "Sojourn time has not gone above CE threshold."
                                "Hence, there should not be any CE threshold "
                                "exceeded marked packet");
        } else {
          currentDropCount = queue->GetStats().GetNDroppedPackets(
              CoDelQueueDisc::TARGET_EXCEEDED_DROP);
          currentCeThreshMarkCount = queue->GetStats().GetNMarkedPackets(
              CoDelQueueDisc::CE_THRESHOLD_EXCEEDED_MARK);
          NS_TEST_ASSERT_MSG_EQ(queue->GetCurrentSize().GetValue(),
                                initialQSize - modeSize,
                                "There should be only 1 more packet dequeued.");
          NS_TEST_ASSERT_MSG_EQ(currentDropCount, 0,
                                "There should not be any packet drops");
          NS_TEST_ASSERT_MSG_EQ(
              currentCeThreshMarkCount, 1,
              "Sojourn time has gone above CE threshold."
              "There should be 1 CE threshold exceeded marked packet");
        }
      } else if (initialCeThreshMarkCount > 0 &&
                 currentTime < queue->GetInterval()) {
        if (initialCeThreshMarkCount < 2) {
          currentDropCount = queue->GetStats().GetNDroppedPackets(
              CoDelQueueDisc::TARGET_EXCEEDED_DROP);
          currentCeThreshMarkCount = queue->GetStats().GetNMarkedPackets(
              CoDelQueueDisc::CE_THRESHOLD_EXCEEDED_MARK);
          NS_TEST_ASSERT_MSG_EQ(queue->GetCurrentSize().GetValue(),
                                initialQSize - modeSize,
                                "There should be only 1 more packet dequeued.");
          NS_TEST_ASSERT_MSG_EQ(currentDropCount, 0,
                                "There should not be any packet drops");
          NS_TEST_ASSERT_MSG_EQ(
              currentCeThreshMarkCount, 2,
              "There should be 2 CE threshold exceeded marked packets");
        } else {
          currentDropCount = queue->GetStats().GetNDroppedPackets(
              CoDelQueueDisc::TARGET_EXCEEDED_DROP);
          currentCeThreshMarkCount = queue->GetStats().GetNMarkedPackets(
              CoDelQueueDisc::CE_THRESHOLD_EXCEEDED_MARK);
          NS_TEST_ASSERT_MSG_EQ(queue->GetCurrentSize().GetValue(),
                                initialQSize - modeSize,
                                "There should be only 1 more packet dequeued.");
          NS_TEST_ASSERT_MSG_EQ(currentDropCount, 0,
                                "There should not be any packet drops");
          NS_TEST_ASSERT_MSG_EQ(
              currentCeThreshMarkCount, 3,
              "There should be 3 CE threshold exceeded marked packet");
        }
      }
    }
  }
}

static class CoDelQueueDiscTestSuite : public TestSuite {
public:
  CoDelQueueDiscTestSuite() : TestSuite("codel-queue-disc", UNIT) {
    AddTestCase(new CoDelQueueDiscBasicEnqueueDequeue(QueueSizeUnit::PACKETS),
                TestCase::QUICK);
    AddTestCase(new CoDelQueueDiscBasicEnqueueDequeue(QueueSizeUnit::BYTES),
                TestCase::QUICK);
    AddTestCase(new CoDelQueueDiscBasicOverflow(QueueSizeUnit::PACKETS),
                TestCase::QUICK);
    AddTestCase(new CoDelQueueDiscBasicOverflow(QueueSizeUnit::BYTES),
                TestCase::QUICK);
    AddTestCase(new CoDelQueueDiscNewtonStepTest(), TestCase::QUICK);
    AddTestCase(new CoDelQueueDiscControlLawTest(), TestCase::QUICK);
    AddTestCase(new CoDelQueueDiscBasicDrop(QueueSizeUnit::PACKETS),
                TestCase::QUICK);
    AddTestCase(new CoDelQueueDiscBasicDrop(QueueSizeUnit::BYTES),
                TestCase::QUICK);
    AddTestCase(new CoDelQueueDiscBasicMark(QueueSizeUnit::PACKETS),
                TestCase::QUICK);
    AddTestCase(new CoDelQueueDiscBasicMark(QueueSizeUnit::BYTES),
                TestCase::QUICK);
  }
} g_coDelQueueTestSuite;
