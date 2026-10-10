
#include "ns3/double.h"
#include "ns3/log.h"
#include "ns3/packet.h"
#include "ns3/pie-queue-disc.h"
#include "ns3/simulator.h"
#include "ns3/string.h"
#include "ns3/test.h"
#include "ns3/uinteger.h"

using namespace ns3;

class PieQueueDiscTestItem : public QueueDiscItem {
public:
  PieQueueDiscTestItem(Ptr<Packet> p, const Address &addr, bool ecnCapable);

  PieQueueDiscTestItem() = delete;
  PieQueueDiscTestItem(const PieQueueDiscTestItem &) = delete;
  PieQueueDiscTestItem &operator=(const PieQueueDiscTestItem &) = delete;

  void AddHeader() override;
  bool Mark() override;

  double m_maxDropProbDiff = 0.0;
  double m_prevDropProb = 0.0;
  bool m_checkProb = false;

  double m_maxDropProb = 0.0;
  bool m_ecnCapable = false;

  bool m_checkAccuProb = false;
  bool m_constAccuProb = false;
  bool m_checkMaxAccuProb = false;
  double m_accuProbError = 0.0;
  double m_prevAccuProb = 0.0;
  double m_setAccuProb = 0.0;
  uint32_t m_expectedDrops = 0;

private:
  bool m_ecnCapablePacket;
};

PieQueueDiscTestItem::PieQueueDiscTestItem(Ptr<Packet> p, const Address &addr,
                                           bool ecnCapable)
    : QueueDiscItem(p, addr, 0), m_ecnCapablePacket(ecnCapable) {}

void PieQueueDiscTestItem::AddHeader() {}

bool PieQueueDiscTestItem::Mark() { return m_ecnCapablePacket; }

class PieQueueDiscTestCase : public TestCase {
public:
  PieQueueDiscTestCase();
  void DoRun() override;

private:
  void Enqueue(Ptr<PieQueueDisc> queue, uint32_t size, uint32_t nPkt,
               Ptr<PieQueueDiscTestItem> testAttributes);
  void EnqueueWithDelay(Ptr<PieQueueDisc> queue, uint32_t size, uint32_t nPkt,
                        Ptr<PieQueueDiscTestItem> testAttributes);
  void Dequeue(Ptr<PieQueueDisc> queue, uint32_t nPkt);
  void DequeueWithDelay(Ptr<PieQueueDisc> queue, double delay, uint32_t nPkt);
  void RunPieTest(QueueSizeUnit mode);
  void CheckDropProb(Ptr<PieQueueDisc> queue,
                     Ptr<PieQueueDiscTestItem> testAttributes);
  void CheckAccuProb(Ptr<PieQueueDisc> queue,
                     Ptr<PieQueueDiscTestItem> testAttributes);
  void CheckMaxAccuProb(Ptr<PieQueueDisc> queue,
                        Ptr<PieQueueDiscTestItem> testAttributes);
};

PieQueueDiscTestCase::PieQueueDiscTestCase()
    : TestCase("Sanity check on the pie queue disc implementation") {}

void PieQueueDiscTestCase::RunPieTest(QueueSizeUnit mode) {
  uint32_t pktSize = 0;

  uint32_t modeSize = 1;

  uint32_t qSize = 300;
  Ptr<PieQueueDisc> queue = CreateObject<PieQueueDisc>();

  Address dest;
  Ptr<PieQueueDiscTestItem> testAttributes =
      Create<PieQueueDiscTestItem>(Create<Packet>(pktSize), dest, false);

  if (mode == QueueSizeUnit::BYTES) {
    pktSize = 1000;
    modeSize = pktSize;
    qSize = qSize * modeSize;
  }

  NS_TEST_ASSERT_MSG_EQ(
      queue->SetAttributeFailSafe("MaxSize",
                                  QueueSizeValue(QueueSize(mode, qSize))),
      true, "Verify that we can actually set the attribute MaxSize");

  Ptr<Packet> p1;
  Ptr<Packet> p2;
  Ptr<Packet> p3;
  Ptr<Packet> p4;
  Ptr<Packet> p5;
  Ptr<Packet> p6;
  Ptr<Packet> p7;
  Ptr<Packet> p8;
  p1 = Create<Packet>(pktSize);
  p2 = Create<Packet>(pktSize);
  p3 = Create<Packet>(pktSize);
  p4 = Create<Packet>(pktSize);
  p5 = Create<Packet>(pktSize);
  p6 = Create<Packet>(pktSize);
  p7 = Create<Packet>(pktSize);
  p8 = Create<Packet>(pktSize);

  queue->Initialize();
  NS_TEST_ASSERT_MSG_EQ(queue->GetCurrentSize().GetValue(), 0 * modeSize,
                        "There should be no packets in there");
  queue->Enqueue(Create<PieQueueDiscTestItem>(p1, dest, false));
  NS_TEST_ASSERT_MSG_EQ(queue->GetCurrentSize().GetValue(), 1 * modeSize,
                        "There should be one packet in there");
  queue->Enqueue(Create<PieQueueDiscTestItem>(p2, dest, false));
  NS_TEST_ASSERT_MSG_EQ(queue->GetCurrentSize().GetValue(), 2 * modeSize,
                        "There should be two packets in there");
  queue->Enqueue(Create<PieQueueDiscTestItem>(p3, dest, false));
  queue->Enqueue(Create<PieQueueDiscTestItem>(p4, dest, false));
  queue->Enqueue(Create<PieQueueDiscTestItem>(p5, dest, false));
  queue->Enqueue(Create<PieQueueDiscTestItem>(p6, dest, false));
  queue->Enqueue(Create<PieQueueDiscTestItem>(p7, dest, false));
  queue->Enqueue(Create<PieQueueDiscTestItem>(p8, dest, false));
  NS_TEST_ASSERT_MSG_EQ(queue->GetCurrentSize().GetValue(), 8 * modeSize,
                        "There should be eight packets in there");

  Ptr<QueueDiscItem> item;

  item = queue->Dequeue();
  NS_TEST_ASSERT_MSG_NE(item, nullptr, "I want to remove the first packet");
  NS_TEST_ASSERT_MSG_EQ(queue->GetCurrentSize().GetValue(), 7 * modeSize,
                        "There should be seven packets in there");
  NS_TEST_ASSERT_MSG_EQ(item->GetPacket()->GetUid(), p1->GetUid(),
                        "was this the first packet ?");

  item = queue->Dequeue();
  NS_TEST_ASSERT_MSG_NE(item, nullptr, "I want to remove the second packet");
  NS_TEST_ASSERT_MSG_EQ(queue->GetCurrentSize().GetValue(), 6 * modeSize,
                        "There should be six packet in there");
  NS_TEST_ASSERT_MSG_EQ(item->GetPacket()->GetUid(), p2->GetUid(),
                        "Was this the second packet ?");

  item = queue->Dequeue();
  NS_TEST_ASSERT_MSG_NE(item, nullptr, "I want to remove the third packet");
  NS_TEST_ASSERT_MSG_EQ(queue->GetCurrentSize().GetValue(), 5 * modeSize,
                        "There should be five packets in there");
  NS_TEST_ASSERT_MSG_EQ(item->GetPacket()->GetUid(), p3->GetUid(),
                        "Was this the third packet ?");

  item = queue->Dequeue();
  item = queue->Dequeue();
  item = queue->Dequeue();
  item = queue->Dequeue();
  item = queue->Dequeue();

  item = queue->Dequeue();
  NS_TEST_ASSERT_MSG_EQ(item, nullptr, "There are really no packets in there");

  queue = CreateObject<PieQueueDisc>();
  testAttributes =
      Create<PieQueueDiscTestItem>(Create<Packet>(pktSize), dest, false);
  pktSize = 1000;
  NS_TEST_ASSERT_MSG_EQ(
      queue->SetAttributeFailSafe("MaxSize",
                                  QueueSizeValue(QueueSize(mode, qSize))),
      true, "Verify that we can actually set the attribute MaxSize");
  NS_TEST_ASSERT_MSG_EQ(
      queue->SetAttributeFailSafe("Tupdate", TimeValue(Seconds(0.03))), true,
      "Verify that we can actually set the attribute Tupdate");
  NS_TEST_ASSERT_MSG_EQ(
      queue->SetAttributeFailSafe("DequeueThreshold", UintegerValue(10000)),
      true, "Verify that we can actually set the attribute DequeueThreshold");
  NS_TEST_ASSERT_MSG_EQ(
      queue->SetAttributeFailSafe("QueueDelayReference",
                                  TimeValue(Seconds(0.02))),
      true,
      "Verify that we can actually set the attribute QueueDelayReference");
  NS_TEST_ASSERT_MSG_EQ(
      queue->SetAttributeFailSafe("MaxBurstAllowance", TimeValue(Seconds(0.1))),
      true, "Verify that we can actually set the attribute MaxBurstAllowance");
  queue->Initialize();
  EnqueueWithDelay(queue, pktSize, 400, testAttributes);
  DequeueWithDelay(queue, 0.012, 400);
  Simulator::Stop(Seconds(8.0));
  Simulator::Run();
  QueueDisc::Stats st = queue->GetStats();
  uint32_t test2 = st.GetNDroppedPackets(PieQueueDisc::UNFORCED_DROP);
  NS_TEST_ASSERT_MSG_NE(test2, 0, "There should be some unforced drops");
  NS_TEST_ASSERT_MSG_EQ(st.GetNDroppedPackets(PieQueueDisc::FORCED_DROP), 0,
                        "There should be zero forced drops");

  queue = CreateObject<PieQueueDisc>();
  testAttributes =
      Create<PieQueueDiscTestItem>(Create<Packet>(pktSize), dest, false);
  NS_TEST_ASSERT_MSG_EQ(
      queue->SetAttributeFailSafe("MaxSize",
                                  QueueSizeValue(QueueSize(mode, qSize))),
      true, "Verify that we can actually set the attribute MaxSize");
  NS_TEST_ASSERT_MSG_EQ(
      queue->SetAttributeFailSafe("Tupdate", TimeValue(Seconds(0.03))), true,
      "Verify that we can actually set the attribute Tupdate");
  NS_TEST_ASSERT_MSG_EQ(
      queue->SetAttributeFailSafe("DequeueThreshold", UintegerValue(10000)),
      true, "Verify that we can actually set the attribute DequeueThreshold");
  NS_TEST_ASSERT_MSG_EQ(
      queue->SetAttributeFailSafe("QueueDelayReference",
                                  TimeValue(Seconds(0.08))),
      true,
      "Verify that we can actually set the attribute QueueDelayReference");
  NS_TEST_ASSERT_MSG_EQ(
      queue->SetAttributeFailSafe("MaxBurstAllowance", TimeValue(Seconds(0.1))),
      true, "Verify that we can actually set the attribute MaxBurstAllowance");
  queue->Initialize();
  EnqueueWithDelay(queue, pktSize, 400, testAttributes);
  DequeueWithDelay(queue, 0.012, 400);
  Simulator::Stop(Seconds(8.0));
  Simulator::Run();
  st = queue->GetStats();
  uint32_t test3 = st.GetNDroppedPackets(PieQueueDisc::UNFORCED_DROP);
  NS_TEST_ASSERT_MSG_LT(test3, test2,
                        "Test 3 should have less unforced drops than test 2");
  NS_TEST_ASSERT_MSG_EQ(st.GetNDroppedPackets(PieQueueDisc::FORCED_DROP), 0,
                        "There should be zero forced drops");

  queue = CreateObject<PieQueueDisc>();
  testAttributes =
      Create<PieQueueDiscTestItem>(Create<Packet>(pktSize), dest, false);
  NS_TEST_ASSERT_MSG_EQ(
      queue->SetAttributeFailSafe("MaxSize",
                                  QueueSizeValue(QueueSize(mode, qSize))),
      true, "Verify that we can actually set the attribute MaxSize");
  NS_TEST_ASSERT_MSG_EQ(
      queue->SetAttributeFailSafe("Tupdate", TimeValue(Seconds(0.03))), true,
      "Verify that we can actually set the attribute Tupdate");
  NS_TEST_ASSERT_MSG_EQ(
      queue->SetAttributeFailSafe("DequeueThreshold", UintegerValue(10000)),
      true, "Verify that we can actually set the attribute DequeueThreshold");
  NS_TEST_ASSERT_MSG_EQ(
      queue->SetAttributeFailSafe("QueueDelayReference",
                                  TimeValue(Seconds(0.02))),
      true,
      "Verify that we can actually set the attribute QueueDelayReference");
  NS_TEST_ASSERT_MSG_EQ(
      queue->SetAttributeFailSafe("MaxBurstAllowance", TimeValue(Seconds(0.1))),
      true, "Verify that we can actually set the attribute MaxBurstAllowance");
  queue->Initialize();
  EnqueueWithDelay(queue, pktSize, 400, testAttributes);
  DequeueWithDelay(queue, 0.015, 400);
  Simulator::Stop(Seconds(8.0));
  Simulator::Run();
  st = queue->GetStats();
  uint32_t test4 = st.GetNDroppedPackets(PieQueueDisc::UNFORCED_DROP);
  NS_TEST_ASSERT_MSG_GT(test4, test2,
                        "Test 4 should have more unforced drops than test 2");
  NS_TEST_ASSERT_MSG_EQ(st.GetNDroppedPackets(PieQueueDisc::FORCED_DROP), 0,
                        "There should be zero forced drops");

  queue = CreateObject<PieQueueDisc>();
  testAttributes =
      Create<PieQueueDiscTestItem>(Create<Packet>(pktSize), dest, false);
  NS_TEST_ASSERT_MSG_EQ(
      queue->SetAttributeFailSafe("MaxSize",
                                  QueueSizeValue(QueueSize(mode, qSize))),
      true, "Verify that we can actually set the attribute MaxSize");
  NS_TEST_ASSERT_MSG_EQ(
      queue->SetAttributeFailSafe("Tupdate", TimeValue(Seconds(0.09))), true,
      "Verify that we can actually set the attribute Tupdate");
  NS_TEST_ASSERT_MSG_EQ(
      queue->SetAttributeFailSafe("DequeueThreshold", UintegerValue(10000)),
      true, "Verify that we can actually set the attribute DequeueThreshold");
  NS_TEST_ASSERT_MSG_EQ(
      queue->SetAttributeFailSafe("QueueDelayReference",
                                  TimeValue(Seconds(0.02))),
      true,
      "Verify that we can actually set the attribute QueueDelayReference");
  NS_TEST_ASSERT_MSG_EQ(
      queue->SetAttributeFailSafe("MaxBurstAllowance", TimeValue(Seconds(0.1))),
      true, "Verify that we can actually set the attribute MaxBurstAllowance");
  queue->Initialize();
  EnqueueWithDelay(queue, pktSize, 400, testAttributes);
  DequeueWithDelay(queue, 0.015, 400);
  Simulator::Stop(Seconds(8.0));
  Simulator::Run();
  st = queue->GetStats();
  uint32_t test5 = st.GetNDroppedPackets(PieQueueDisc::UNFORCED_DROP);
  NS_TEST_ASSERT_MSG_LT(test5, test4,
                        "Test 5 should have less unforced drops than test 4");
  NS_TEST_ASSERT_MSG_EQ(st.GetNDroppedPackets(PieQueueDisc::FORCED_DROP), 0,
                        "There should be zero forced drops");

  queue = CreateObject<PieQueueDisc>();
  testAttributes =
      Create<PieQueueDiscTestItem>(Create<Packet>(pktSize), dest, false);
  NS_TEST_ASSERT_MSG_EQ(
      queue->SetAttributeFailSafe("MaxSize",
                                  QueueSizeValue(QueueSize(mode, qSize))),
      true, "Verify that we can actually set the attribute MaxSize");
  NS_TEST_ASSERT_MSG_EQ(
      queue->SetAttributeFailSafe("UseDequeueRateEstimator",
                                  BooleanValue(true)),
      true, "Verify that we can actually set the attribute UseTimestamp");
  queue->Initialize();
  EnqueueWithDelay(queue, pktSize, 400, testAttributes);
  DequeueWithDelay(queue, 0.014, 400);
  Simulator::Stop(Seconds(8.0));
  Simulator::Run();
  st = queue->GetStats();
  uint32_t test6 = st.GetNDroppedPackets(PieQueueDisc::UNFORCED_DROP);
  NS_TEST_ASSERT_MSG_NE(test6, 0, "There should be some unforced drops");
  NS_TEST_ASSERT_MSG_EQ(st.GetNDroppedPackets(PieQueueDisc::FORCED_DROP), 0,
                        "There should be zero forced drops");

  queue = CreateObject<PieQueueDisc>();
  testAttributes =
      Create<PieQueueDiscTestItem>(Create<Packet>(pktSize), dest, false);
  NS_TEST_ASSERT_MSG_EQ(
      queue->SetAttributeFailSafe("MaxSize",
                                  QueueSizeValue(QueueSize(mode, qSize))),
      true, "Verify that we can actually set the attribute MaxSize");
  NS_TEST_ASSERT_MSG_EQ(
      queue->SetAttributeFailSafe("UseCapDropAdjustment", BooleanValue(false)),
      true,
      "Verify that we can actually set the attribute UseCapDropAdjustment");
  queue->Initialize();
  testAttributes->m_checkProb = true;
  EnqueueWithDelay(queue, pktSize, 400, testAttributes);
  DequeueWithDelay(queue, 0.014, 400);
  Simulator::Stop(Seconds(8.0));
  Simulator::Run();
  st = queue->GetStats();
  uint32_t test7 = st.GetNDroppedPackets(PieQueueDisc::UNFORCED_DROP);
  NS_TEST_ASSERT_MSG_NE(test7, 0, "There should be some unforced drops");
  NS_TEST_ASSERT_MSG_EQ(st.GetNDroppedPackets(PieQueueDisc::FORCED_DROP), 0,
                        "There should be zero forced drops");
  NS_TEST_ASSERT_MSG_GT(
      testAttributes->m_maxDropProbDiff, 0.02,
      "Maximum increase in drop probability should be greater than 0.02");

  queue = CreateObject<PieQueueDisc>();
  testAttributes =
      Create<PieQueueDiscTestItem>(Create<Packet>(pktSize), dest, false);
  NS_TEST_ASSERT_MSG_EQ(
      queue->SetAttributeFailSafe("MaxSize",
                                  QueueSizeValue(QueueSize(mode, qSize))),
      true, "Verify that we can actually set the attribute MaxSize");
  NS_TEST_ASSERT_MSG_EQ(
      queue->SetAttributeFailSafe("UseCapDropAdjustment", BooleanValue(true)),
      true,
      "Verify that we can actually set the attribute UseCapDropAdjustment");
  queue->Initialize();
  testAttributes->m_checkProb = true;
  EnqueueWithDelay(queue, pktSize, 400, testAttributes);
  DequeueWithDelay(queue, 0.014, 400);
  Simulator::Stop(Seconds(8.0));
  Simulator::Run();
  st = queue->GetStats();
  uint32_t test8 = st.GetNDroppedPackets(PieQueueDisc::UNFORCED_DROP);
  NS_TEST_ASSERT_MSG_NE(test8, 0, "There should be some unforced drops");
  NS_TEST_ASSERT_MSG_EQ(st.GetNDroppedPackets(PieQueueDisc::FORCED_DROP), 0,
                        "There should be zero forced drops");
  NS_TEST_ASSERT_MSG_LT(testAttributes->m_maxDropProbDiff, 0.0200000000000001,
                        "Maximum increase in drop probability should be less "
                        "than or equal to 0.02");

  queue = CreateObject<PieQueueDisc>();
  testAttributes =
      Create<PieQueueDiscTestItem>(Create<Packet>(pktSize), dest, false);
  NS_TEST_ASSERT_MSG_EQ(
      queue->SetAttributeFailSafe("MaxSize",
                                  QueueSizeValue(QueueSize(mode, qSize))),
      true, "Verify that we can actually set the attribute MaxSize");
  NS_TEST_ASSERT_MSG_EQ(
      queue->SetAttributeFailSafe("UseEcn", BooleanValue(true)), true,
      "Verify that we can actually set the attribute UseEcn");
  NS_TEST_ASSERT_MSG_EQ(
      queue->SetAttributeFailSafe("MarkEcnThreshold", DoubleValue(0.3)), true,
      "Verify that we can actually set the attribute MarkEcnThreshold");
  queue->Initialize();
  EnqueueWithDelay(queue, pktSize, 400, testAttributes);
  DequeueWithDelay(queue, 0.014, 400);
  Simulator::Stop(Seconds(8.0));
  Simulator::Run();
  st = queue->GetStats();
  uint32_t test9 = st.GetNMarkedPackets(PieQueueDisc::UNFORCED_MARK);
  NS_TEST_ASSERT_MSG_EQ(test9, 0, "There should be zero unforced marks");
  NS_TEST_ASSERT_MSG_NE(st.GetNDroppedPackets(PieQueueDisc::UNFORCED_DROP), 0,
                        "There should be some unforced drops");
  NS_TEST_ASSERT_MSG_EQ(st.GetNDroppedPackets(PieQueueDisc::FORCED_DROP), 0,
                        "There should be zero forced drops");

  queue = CreateObject<PieQueueDisc>();
  testAttributes =
      Create<PieQueueDiscTestItem>(Create<Packet>(pktSize), dest, false);
  NS_TEST_ASSERT_MSG_EQ(
      queue->SetAttributeFailSafe("MaxSize",
                                  QueueSizeValue(QueueSize(mode, qSize))),
      true, "Verify that we can actually set the attribute MaxSize");
  NS_TEST_ASSERT_MSG_EQ(
      queue->SetAttributeFailSafe("UseEcn", BooleanValue(false)), true,
      "Verify that we can actually set the attribute UseEcn");
  NS_TEST_ASSERT_MSG_EQ(
      queue->SetAttributeFailSafe("MarkEcnThreshold", DoubleValue(0.3)), true,
      "Verify that we can actually set the attribute MarkEcnThreshold");
  queue->Initialize();
  testAttributes->m_ecnCapable = true;
  EnqueueWithDelay(queue, pktSize, 400, testAttributes);
  DequeueWithDelay(queue, 0.014, 400);
  Simulator::Stop(Seconds(8.0));
  Simulator::Run();
  st = queue->GetStats();
  uint32_t test10 = st.GetNMarkedPackets(PieQueueDisc::UNFORCED_MARK);
  NS_TEST_ASSERT_MSG_EQ(test10, 0, "There should be zero unforced marks");
  NS_TEST_ASSERT_MSG_NE(st.GetNDroppedPackets(PieQueueDisc::UNFORCED_DROP), 0,
                        "There should be some unforced drops");
  NS_TEST_ASSERT_MSG_EQ(st.GetNDroppedPackets(PieQueueDisc::FORCED_DROP), 0,
                        "There should be zero forced drops");

  queue = CreateObject<PieQueueDisc>();
  testAttributes =
      Create<PieQueueDiscTestItem>(Create<Packet>(pktSize), dest, false);
  NS_TEST_ASSERT_MSG_EQ(
      queue->SetAttributeFailSafe("MaxSize",
                                  QueueSizeValue(QueueSize(mode, qSize))),
      true, "Verify that we can actually set the attribute MaxSize");
  NS_TEST_ASSERT_MSG_EQ(
      queue->SetAttributeFailSafe("UseEcn", BooleanValue(true)), true,
      "Verify that we can actually set the attribute UseEcn");
  NS_TEST_ASSERT_MSG_EQ(
      queue->SetAttributeFailSafe("MarkEcnThreshold", DoubleValue(0.3)), true,
      "Verify that we can actually set the attribute MarkEcnThreshold");
  queue->Initialize();
  testAttributes->m_ecnCapable = true;
  testAttributes->m_checkProb = true;
  EnqueueWithDelay(queue, pktSize, 400, testAttributes);
  DequeueWithDelay(queue, 0.014, 400);
  Simulator::Stop(Seconds(8.0));
  Simulator::Run();
  st = queue->GetStats();
  uint32_t test11 = st.GetNMarkedPackets(PieQueueDisc::UNFORCED_MARK);
  NS_TEST_ASSERT_MSG_NE(test11, 0, "There should be some unforced marks");
  NS_TEST_ASSERT_MSG_NE(st.GetNDroppedPackets(PieQueueDisc::UNFORCED_DROP), 0,
                        "There should be some unforced drops");
  NS_TEST_ASSERT_MSG_GT(testAttributes->m_maxDropProb, 0.3,
                        "Maximum Drop probability should be greater than 0.3");
  NS_TEST_ASSERT_MSG_EQ(st.GetNDroppedPackets(PieQueueDisc::FORCED_DROP), 0,
                        "There should be zero forced drops");

  queue = CreateObject<PieQueueDisc>();
  testAttributes =
      Create<PieQueueDiscTestItem>(Create<Packet>(pktSize), dest, false);
  NS_TEST_ASSERT_MSG_EQ(
      queue->SetAttributeFailSafe("MaxSize",
                                  QueueSizeValue(QueueSize(mode, qSize))),
      true, "Verify that we can actually set the attribute MaxSize");
  NS_TEST_ASSERT_MSG_EQ(
      queue->SetAttributeFailSafe("UseDerandomization", BooleanValue(true)),
      true, "Verify that we can actually set the attribute UseDerandomization");
  queue->Initialize();
  testAttributes->m_checkAccuProb = true;
  EnqueueWithDelay(queue, pktSize, 400, testAttributes);
  DequeueWithDelay(queue, 0.014, 400);
  Simulator::Stop(Seconds(8.0));
  Simulator::Run();
  st = queue->GetStats();
  uint32_t test12 = st.GetNDroppedPackets(PieQueueDisc::UNFORCED_DROP);
  NS_TEST_ASSERT_MSG_NE(test12, 0, "There should be some unforced drops");
  NS_TEST_ASSERT_MSG_EQ(st.GetNDroppedPackets(PieQueueDisc::FORCED_DROP), 0,
                        "There should be zero forced drops");
  NS_TEST_ASSERT_MSG_EQ(testAttributes->m_accuProbError, 0.0,
                        "There should not be any error in setting accuProb");

  queue = CreateObject<PieQueueDisc>();
  testAttributes =
      Create<PieQueueDiscTestItem>(Create<Packet>(pktSize), dest, false);
  NS_TEST_ASSERT_MSG_EQ(
      queue->SetAttributeFailSafe("MaxSize",
                                  QueueSizeValue(QueueSize(mode, qSize))),
      true, "Verify that we can actually set the attribute MaxSize");
  NS_TEST_ASSERT_MSG_EQ(
      queue->SetAttributeFailSafe("UseDerandomization", BooleanValue(true)),
      true, "Verify that we can actually set the attribute UseDerandomization");
  queue->Initialize();
  testAttributes->m_constAccuProb = true;
  testAttributes->m_setAccuProb = -0.16;
  EnqueueWithDelay(queue, pktSize, 400, testAttributes);
  DequeueWithDelay(queue, 0.014, 400);
  Simulator::Stop(Seconds(8.0));
  Simulator::Run();
  st = queue->GetStats();
  uint32_t test13 = st.GetNDroppedPackets(PieQueueDisc::UNFORCED_DROP);
  NS_TEST_ASSERT_MSG_EQ(test13, 0, "There should be zero unforced drops");
  NS_TEST_ASSERT_MSG_EQ(st.GetNDroppedPackets(PieQueueDisc::FORCED_DROP), 0,
                        "There should be zero forced drops");

  queue = CreateObject<PieQueueDisc>();
  testAttributes =
      Create<PieQueueDiscTestItem>(Create<Packet>(pktSize), dest, false);
  NS_TEST_ASSERT_MSG_EQ(
      queue->SetAttributeFailSafe("MaxSize",
                                  QueueSizeValue(QueueSize(mode, qSize))),
      true, "Verify that we can actually set the attribute MaxSize");
  NS_TEST_ASSERT_MSG_EQ(
      queue->SetAttributeFailSafe("MaxBurstAllowance", TimeValue(Seconds(0.0))),
      true, "Verify that we can actually set the attribute MaxBurstAllowance");
  NS_TEST_ASSERT_MSG_EQ(
      queue->SetAttributeFailSafe("UseDerandomization", BooleanValue(true)),
      true, "Verify that we can actually set the attribute UseDerandomization");
  queue->Initialize();
  testAttributes->m_constAccuProb = true;
  testAttributes->m_checkMaxAccuProb = true;
  testAttributes->m_setAccuProb = 8.6;
  EnqueueWithDelay(queue, pktSize, 400, testAttributes);
  DequeueWithDelay(queue, 0.014, 400);
  Simulator::Stop(Seconds(8.0));
  Simulator::Run();
  st = queue->GetStats();
  uint32_t test14 = st.GetNDroppedPackets(PieQueueDisc::UNFORCED_DROP);
  NS_TEST_ASSERT_MSG_EQ(test14, testAttributes->m_expectedDrops,
                        "The number of unforced drops should be equal to "
                        "number of expected unforced drops");
  NS_TEST_ASSERT_MSG_EQ(st.GetNDroppedPackets(PieQueueDisc::FORCED_DROP), 0,
                        "There should be zero forced drops");

  queue = CreateObject<PieQueueDisc>();
  testAttributes =
      Create<PieQueueDiscTestItem>(Create<Packet>(pktSize), dest, false);
  queue->SetAttributeFailSafe("MaxSize",
                              QueueSizeValue(QueueSize(mode, qSize)));
  queue->SetAttributeFailSafe("ActiveThreshold", TimeValue(Seconds(1)));
  queue->Initialize();

  EnqueueWithDelay(queue, pktSize, 100, testAttributes);
  DequeueWithDelay(queue, 0.02, 100);
  Simulator::Stop(Seconds(8.0));
  Simulator::Run();
  st = queue->GetStats();
  uint32_t test15 = st.GetNDroppedPackets(PieQueueDisc::UNFORCED_DROP);
  NS_TEST_ASSERT_MSG_EQ(test15, 0, "There should not be any drops.");
  NS_TEST_ASSERT_MSG_EQ(st.GetNMarkedPackets(PieQueueDisc::UNFORCED_MARK), 0,
                        "There should be zero marks");

  queue = CreateObject<PieQueueDisc>();
  testAttributes =
      Create<PieQueueDiscTestItem>(Create<Packet>(pktSize), dest, false);
  queue->SetAttributeFailSafe("MaxSize",
                              QueueSizeValue(QueueSize(mode, qSize)));
  queue->SetAttributeFailSafe("ActiveThreshold", TimeValue(Seconds(0.001)));
  queue->Initialize();

  EnqueueWithDelay(queue, pktSize, 100, testAttributes);
  DequeueWithDelay(queue, 0.02, 100);
  Simulator::Stop(Seconds(8.0));
  Simulator::Run();
  st = queue->GetStats();
  uint32_t test16 = st.GetNDroppedPackets(PieQueueDisc::UNFORCED_DROP);
  NS_TEST_ASSERT_MSG_NE(test16, 0, "There should be some drops.");
  NS_TEST_ASSERT_MSG_EQ(st.GetNMarkedPackets(PieQueueDisc::UNFORCED_MARK), 0,
                        "There should be zero marks");
}

void PieQueueDiscTestCase::Enqueue(Ptr<PieQueueDisc> queue, uint32_t size,
                                   uint32_t nPkt,
                                   Ptr<PieQueueDiscTestItem> testAttributes) {
  Address dest;
  for (uint32_t i = 0; i < nPkt; i++) {
    if (testAttributes->m_constAccuProb) {
      queue->m_accuProb = testAttributes->m_setAccuProb;
      if (testAttributes->m_checkMaxAccuProb) {
        CheckMaxAccuProb(queue, testAttributes);
      }
    }
    queue->Enqueue(Create<PieQueueDiscTestItem>(Create<Packet>(size), dest,
                                                testAttributes->m_ecnCapable));
    if (testAttributes->m_checkProb) {
      CheckDropProb(queue, testAttributes);
    }
    if (testAttributes->m_checkAccuProb) {
      CheckAccuProb(queue, testAttributes);
    }
  }
}

void PieQueueDiscTestCase::CheckDropProb(
    Ptr<PieQueueDisc> queue, Ptr<PieQueueDiscTestItem> testAttributes) {
  double dropProb = queue->m_dropProb;
  if (testAttributes->m_maxDropProb < dropProb) {
    testAttributes->m_maxDropProb = dropProb;
  }
  if (testAttributes->m_prevDropProb > 0.1) {
    double currentDiff = dropProb - testAttributes->m_prevDropProb;
    if (testAttributes->m_maxDropProbDiff < currentDiff) {
      testAttributes->m_maxDropProbDiff = currentDiff;
    }
  }
  testAttributes->m_prevDropProb = dropProb;
}

void PieQueueDiscTestCase::CheckAccuProb(
    Ptr<PieQueueDisc> queue, Ptr<PieQueueDiscTestItem> testAttributes) {
  double dropProb = queue->m_dropProb;
  double accuProb = queue->m_accuProb;
  if (accuProb != 0) {
    double expectedAccuProb = testAttributes->m_prevAccuProb + dropProb;
    testAttributes->m_accuProbError = accuProb - expectedAccuProb;
  }
  testAttributes->m_prevAccuProb = accuProb;
}

void PieQueueDiscTestCase::CheckMaxAccuProb(
    Ptr<PieQueueDisc> queue, Ptr<PieQueueDiscTestItem> testAttributes) {
  queue->m_dropProb = 0.001;
  QueueSize queueSize = queue->GetCurrentSize();
  if ((queueSize.GetUnit() == QueueSizeUnit::PACKETS &&
       queueSize.GetValue() > 2) ||
      (queueSize.GetUnit() == QueueSizeUnit::BYTES &&
       queueSize.GetValue() > 2000)) {
    testAttributes->m_expectedDrops = testAttributes->m_expectedDrops + 1;
  }
}

void PieQueueDiscTestCase::EnqueueWithDelay(
    Ptr<PieQueueDisc> queue, uint32_t size, uint32_t nPkt,
    Ptr<PieQueueDiscTestItem> testAttributes) {
  Address dest;
  double delay = 0.01;
  for (uint32_t i = 0; i < nPkt; i++) {
    Simulator::Schedule(Time(Seconds((i + 1) * delay)),
                        &PieQueueDiscTestCase::Enqueue, this, queue, size, 1,
                        testAttributes);
  }
}

void PieQueueDiscTestCase::Dequeue(Ptr<PieQueueDisc> queue, uint32_t nPkt) {
  for (uint32_t i = 0; i < nPkt; i++) {
    Ptr<QueueDiscItem> item = queue->Dequeue();
  }
}

void PieQueueDiscTestCase::DequeueWithDelay(Ptr<PieQueueDisc> queue,
                                            double delay, uint32_t nPkt) {
  for (uint32_t i = 0; i < nPkt; i++) {
    Simulator::Schedule(Time(Seconds((i + 1) * delay)),
                        &PieQueueDiscTestCase::Dequeue, this, queue, 1);
  }
}

void PieQueueDiscTestCase::DoRun() {
  RunPieTest(QueueSizeUnit::PACKETS);
  RunPieTest(QueueSizeUnit::BYTES);
  Simulator::Destroy();
}

static class PieQueueDiscTestSuite : public TestSuite {
public:
  PieQueueDiscTestSuite() : TestSuite("pie-queue-disc", UNIT) {
    AddTestCase(new PieQueueDiscTestCase(), TestCase::QUICK);
  }
} g_pieQueueTestSuite;
