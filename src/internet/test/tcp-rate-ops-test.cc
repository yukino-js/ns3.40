
#include "tcp-error-model.h"
#include "tcp-general-test.h"

#include "ns3/config.h"
#include "ns3/log.h"
#include "ns3/tcp-rate-ops.h"
#include "ns3/tcp-tx-buffer.h"
#include "ns3/tcp-tx-item.h"
#include "ns3/test.h"

using namespace ns3;

NS_LOG_COMPONENT_DEFINE("TcpRateOpsTestSuite");

class MimicCongControl;

class TcpRateLinuxBasicTest : public TestCase {
public:
  TcpRateLinuxBasicTest(uint32_t cWnd, SequenceNumber32 tailSeq,
                        SequenceNumber32 nextTx, uint32_t testCase,
                        std::string testName);

private:
  void DoRun() override;

  void SendSkb(TcpTxItem *skb);
  void SkbDelivered(TcpTxItem *skb);

  TcpRateLinux m_rateOps;
  uint32_t m_cWnd;
  uint32_t m_inFlight;
  uint32_t m_segmentSize;
  uint32_t m_delivered;
  Time m_deliveredTime;
  SequenceNumber32 m_tailSeq;
  SequenceNumber32 m_nextTx;
  uint32_t m_testCase;
  std::vector<TcpTxItem *> m_skbs;
};

TcpRateLinuxBasicTest::TcpRateLinuxBasicTest(uint32_t cWnd,
                                             SequenceNumber32 tailSeq,
                                             SequenceNumber32 nextTx,
                                             uint32_t testCase,
                                             std::string testName)
    : TestCase(testName), m_cWnd(cWnd), m_inFlight(0), m_segmentSize(1),
      m_delivered(0), m_deliveredTime(Seconds(0)), m_tailSeq(tailSeq),
      m_nextTx(nextTx), m_testCase(testCase) {}

void TcpRateLinuxBasicTest::DoRun() {
  for (uint8_t i = 0; i < 100; ++i) {
    m_skbs.push_back(new TcpTxItem());
    Simulator::Schedule(Time(Seconds(i * 0.01)),
                        &TcpRateLinuxBasicTest::SendSkb, this, m_skbs[i]);
  }

  for (uint8_t i = 0; i < 100; ++i) {
    Simulator::Schedule(Time(Seconds((i + 1) * 0.1)),
                        &TcpRateLinuxBasicTest::SkbDelivered, this, m_skbs[i]);
  }

  Simulator::Run();
  Simulator::Destroy();

  for (uint8_t i = 0; i < 100; ++i) {
    delete m_skbs[i];
  }
}

void TcpRateLinuxBasicTest::SendSkb(TcpTxItem *skb) {
  bool isStartOfTransmission = m_inFlight == 0;
  m_rateOps.CalculateAppLimited(m_cWnd, m_inFlight, m_segmentSize, m_tailSeq,
                                m_nextTx, 0, 0);
  m_rateOps.SkbSent(skb, isStartOfTransmission);
  m_inFlight += skb->GetSeqSize();

  NS_TEST_ASSERT_MSG_EQ(
      skb->GetRateInformation().m_delivered, m_delivered,
      "SKB should have delivered equal to current value of total delivered");

  if (isStartOfTransmission) {
    NS_TEST_ASSERT_MSG_EQ(
        skb->GetRateInformation().m_deliveredTime, Simulator::Now(),
        "SKB should have updated the delivered time to current value");
  } else {
    NS_TEST_ASSERT_MSG_EQ(
        skb->GetRateInformation().m_deliveredTime, m_deliveredTime,
        "SKB should have updated the delivered time to current value");
  }
}

void TcpRateLinuxBasicTest::SkbDelivered(TcpTxItem *skb) {
  m_rateOps.SkbDelivered(skb);
  m_inFlight -= skb->GetSeqSize();
  m_delivered += skb->GetSeqSize();
  m_deliveredTime = Simulator::Now();

  NS_TEST_ASSERT_MSG_EQ(skb->GetRateInformation().m_deliveredTime, Time::Max(),
                        "SKB should have delivered time as Time::Max ()");

  if (m_testCase == 1) {
    NS_TEST_ASSERT_MSG_EQ(skb->GetRateInformation().m_isAppLimited, false,
                          "Socket should not be applimited");
  } else if (m_testCase == 2) {
    NS_TEST_ASSERT_MSG_EQ(skb->GetRateInformation().m_isAppLimited, true,
                          "Socket should be applimited");
  }
}

class MimicCongControl : public TcpNewReno {
public:
  static TypeId GetTypeId();

  MimicCongControl() {}

  bool HasCongControl() const override { return true; }
};

TypeId MimicCongControl::GetTypeId() {
  static TypeId tid = TypeId("ns3::MimicCongControl")
                          .SetParent<TcpNewReno>()
                          .AddConstructor<MimicCongControl>()
                          .SetGroupName("Internet");
  return tid;
}

class TcpRateLinuxWithSocketsTest : public TcpGeneralTest {
public:
  TcpRateLinuxWithSocketsTest(const std::string &desc, bool sackEnabled,
                              std::vector<uint32_t> &toDrop);

protected:
  Ptr<TcpSocketMsgBase> CreateSenderSocket(Ptr<Node> node) override;

  Ptr<ErrorModel> CreateReceiverErrorModel() override;

  void Rx(const Ptr<const Packet> p, const TcpHeader &h,
          SocketWho who) override;

  void BytesInFlightTrace(uint32_t oldValue, uint32_t newValue) override;

  void PktDropped(const Ipv4Header &ipH, const TcpHeader &tcpH,
                  Ptr<const Packet> p);

  void ConfigureEnvironment() override;

  void FinalChecks() override;

  void RateUpdatedTrace(const TcpRateLinux::TcpRateConnection &rate) override;

  void
  RateSampleUpdatedTrace(const TcpRateLinux::TcpRateSample &sample) override;

private:
  Ptr<MimicCongControl> m_congCtl;
  bool m_sackEnabled;
  std::vector<uint32_t> m_toDrop;
  uint32_t m_bytesInFlight{0};
  SequenceNumber32 m_lastAckRecv{SequenceNumber32(1)};
  bool m_isDupAck;
  TcpRateLinux::TcpRateConnection m_prevRate;
  TcpRateLinux::TcpRateSample m_prevRateSample;
};

TcpRateLinuxWithSocketsTest::TcpRateLinuxWithSocketsTest(
    const std::string &desc, bool sackEnabled, std::vector<uint32_t> &toDrop)
    : TcpGeneralTest(desc), m_sackEnabled(sackEnabled), m_toDrop(toDrop) {}

Ptr<TcpSocketMsgBase>
TcpRateLinuxWithSocketsTest::CreateSenderSocket(Ptr<Node> node) {
  Ptr<TcpSocketMsgBase> s = TcpGeneralTest::CreateSenderSocket(node);
  m_congCtl = CreateObject<MimicCongControl>();
  s->SetCongestionControlAlgorithm(m_congCtl);
  return s;
}

void TcpRateLinuxWithSocketsTest::ConfigureEnvironment() {
  TcpGeneralTest::ConfigureEnvironment();
  SetAppPktCount(300);
  SetPropagationDelay(MilliSeconds(50));
  SetTransmitStart(Seconds(2.0));

  Config::SetDefault("ns3::TcpSocketBase::Sack", BooleanValue(m_sackEnabled));
}

Ptr<ErrorModel> TcpRateLinuxWithSocketsTest::CreateReceiverErrorModel() {
  Ptr<TcpSeqErrorModel> m_errorModel = CreateObject<TcpSeqErrorModel>();
  for (auto it = m_toDrop.begin(); it != m_toDrop.end(); ++it) {
    m_errorModel->AddSeqToKill(SequenceNumber32(*it));
  }

  m_errorModel->SetDropCallback(
      MakeCallback(&TcpRateLinuxWithSocketsTest::PktDropped, this));

  return m_errorModel;
}

void TcpRateLinuxWithSocketsTest::PktDropped(const Ipv4Header &ipH,
                                             const TcpHeader &tcpH,
                                             Ptr<const Packet> p) {
  NS_LOG_DEBUG("Drop seq= " << tcpH.GetSequenceNumber() << " size "
                            << p->GetSize());
}

void TcpRateLinuxWithSocketsTest::Rx(const Ptr<const Packet> p,
                                     const TcpHeader &h, SocketWho who) {
  if (who == SENDER) {
    if (h.GetAckNumber() == m_lastAckRecv &&
        m_lastAckRecv != SequenceNumber32(1) &&
        (h.GetFlags() & TcpHeader::FIN) == 0) {
      m_isDupAck = true;
    } else {
      m_isDupAck = false;
      m_lastAckRecv = h.GetAckNumber();
    }
  }
}

void TcpRateLinuxWithSocketsTest::BytesInFlightTrace(uint32_t,
                                                     uint32_t newValue) {
  m_bytesInFlight = newValue;
}

void TcpRateLinuxWithSocketsTest::RateUpdatedTrace(
    const TcpRateLinux::TcpRateConnection &rate) {
  NS_LOG_DEBUG("Rate updated " << rate);
  if (m_bytesInFlight == 0) {
    NS_TEST_ASSERT_MSG_EQ(
        rate.m_firstSentTime, Simulator::Now(),
        "FirstSentTime should be current time when bytes inflight is zero");
    NS_TEST_ASSERT_MSG_EQ(
        rate.m_deliveredTime, Simulator::Now(),
        "Delivered time should be current time when bytes inflight is zero");
  }
  NS_TEST_ASSERT_MSG_GT_OR_EQ(
      rate.m_delivered, m_prevRate.m_delivered,
      "Total delivered should not be lesser than previous values");
  m_prevRate = rate;
}

void TcpRateLinuxWithSocketsTest::RateSampleUpdatedTrace(
    const TcpRateLinux::TcpRateSample &sample) {
  NS_LOG_DEBUG("Rate sample updated " << sample);
  if (m_isDupAck) {
    if (!m_sackEnabled) {
      NS_TEST_ASSERT_MSG_EQ(m_prevRateSample, sample,
                            "RateSample should not update due to DupAcks");
    } else {
      if (sample.m_ackedSacked == 0) {
        NS_TEST_ASSERT_MSG_EQ(
            m_prevRateSample, sample,
            "RateSample should not update as nothing is acked or sacked");
      }
    }
  }
  m_prevRateSample = sample;
}

void TcpRateLinuxWithSocketsTest::FinalChecks() {}

class TcpRateLinuxWithBufferTest : public TestCase {
public:
  TcpRateLinuxWithBufferTest(uint32_t segmentSize, std::string desc);

private:
  void DoRun() override;
  void DoTeardown() override;

  virtual void RateUpdatedTrace(const TcpRateLinux::TcpRateConnection &rate);

  virtual void
  RateSampleUpdatedTrace(const TcpRateLinux::TcpRateSample &sample);

  void TestWithStraightAcks();

  void TestWithSackBlocks();

  uint32_t m_expectedDelivered{0};
  uint32_t m_expectedAckedSacked{0};
  uint32_t m_segmentSize;
  TcpTxBuffer m_txBuf;
  Ptr<TcpRateOps> m_rateOps;
};

TcpRateLinuxWithBufferTest::TcpRateLinuxWithBufferTest(uint32_t segmentSize,
                                                       std::string testString)
    : TestCase(testString), m_segmentSize(segmentSize) {
  m_rateOps = CreateObject<TcpRateLinux>();
  m_rateOps->TraceConnectWithoutContext(
      "TcpRateUpdated",
      MakeCallback(&TcpRateLinuxWithBufferTest::RateUpdatedTrace, this));
  m_rateOps->TraceConnectWithoutContext(
      "TcpRateSampleUpdated",
      MakeCallback(&TcpRateLinuxWithBufferTest::RateSampleUpdatedTrace, this));
}

void TcpRateLinuxWithBufferTest::DoRun() {
  Simulator::Schedule(Seconds(0.0),
                      &TcpRateLinuxWithBufferTest::TestWithSackBlocks, this);
  Simulator::Run();
  Simulator::Destroy();
}

void TcpRateLinuxWithBufferTest::RateUpdatedTrace(
    const TcpRateLinux::TcpRateConnection &rate) {
  NS_LOG_DEBUG("Rate updated " << rate);
  NS_TEST_ASSERT_MSG_EQ(rate.m_delivered, m_expectedDelivered,
                        "Delivered data is not equal to expected delivered");
}

void TcpRateLinuxWithBufferTest::RateSampleUpdatedTrace(
    const TcpRateLinux::TcpRateSample &sample) {
  NS_LOG_DEBUG("Rate sample updated " << sample);
  NS_TEST_ASSERT_MSG_EQ(
      sample.m_ackedSacked, m_expectedAckedSacked,
      "AckedSacked bytes is not equal to expected AckedSacked bytes");
}

void TcpRateLinuxWithBufferTest::TestWithSackBlocks() {
  SequenceNumber32 head(1);
  m_txBuf.SetHeadSequence(head);
  SequenceNumber32 ret;
  Ptr<TcpOptionSack> sack = CreateObject<TcpOptionSack>();
  m_txBuf.SetSegmentSize(m_segmentSize);
  m_txBuf.SetDupAckThresh(3);

  m_txBuf.Add(Create<Packet>(10 * m_segmentSize));

  for (uint8_t i = 0; i < 10; ++i) {
    bool isStartOfTransmission = m_txBuf.BytesInFlight() == 0;
    TcpTxItem *outItem = m_txBuf.CopyFromSequence(
        m_segmentSize, SequenceNumber32((i * m_segmentSize) + 1));
    m_rateOps->SkbSent(outItem, isStartOfTransmission);
  }

  uint32_t priorInFlight = m_txBuf.BytesInFlight();
  for (uint8_t i = 1; i <= 2; ++i) {
    priorInFlight = m_txBuf.BytesInFlight();
    m_expectedDelivered += m_segmentSize;
    m_txBuf.DiscardUpTo(SequenceNumber32(m_segmentSize * i + 1),
                        MakeCallback(&TcpRateOps::SkbDelivered, m_rateOps));
    m_expectedAckedSacked = m_segmentSize;
    m_rateOps->GenerateSample(m_segmentSize, 0, false, priorInFlight,
                              Seconds(0));
  }

  priorInFlight = m_txBuf.BytesInFlight();
  sack->AddSackBlock(
      TcpOptionSack::SackBlock(SequenceNumber32(m_segmentSize * 4 + 1),
                               SequenceNumber32(m_segmentSize * 5 + 1)));
  m_expectedDelivered += m_segmentSize;
  m_txBuf.Update(sack->GetSackList(),
                 MakeCallback(&TcpRateOps::SkbDelivered, m_rateOps));
  m_expectedAckedSacked = m_segmentSize;
  m_rateOps->GenerateSample(m_segmentSize, 0, false, priorInFlight, Seconds(0));

  priorInFlight = m_txBuf.BytesInFlight();
  sack->AddSackBlock(
      TcpOptionSack::SackBlock(SequenceNumber32(m_segmentSize * 3 + 1),
                               SequenceNumber32(m_segmentSize * 4 + 1)));
  m_expectedDelivered += m_segmentSize;
  m_txBuf.Update(sack->GetSackList(),
                 MakeCallback(&TcpRateOps::SkbDelivered, m_rateOps));
  m_rateOps->GenerateSample(m_segmentSize, 0, false, priorInFlight, Seconds(0));

  priorInFlight = m_txBuf.BytesInFlight();
  m_expectedDelivered += m_segmentSize;
  m_txBuf.DiscardUpTo(SequenceNumber32(m_segmentSize * 5 + 1),
                      MakeCallback(&TcpRateOps::SkbDelivered, m_rateOps));
  m_rateOps->GenerateSample(m_segmentSize, 0, false, priorInFlight, Seconds(0));

  priorInFlight = m_txBuf.BytesInFlight();
  for (uint8_t i = 6; i <= 10; ++i) {
    m_expectedDelivered += m_segmentSize;
    m_txBuf.DiscardUpTo(SequenceNumber32(m_segmentSize * i + 1),
                        MakeCallback(&TcpRateOps::SkbDelivered, m_rateOps));
  }
  m_expectedAckedSacked = 5 * m_segmentSize;
  TcpRateOps::TcpRateSample rateSample = m_rateOps->GenerateSample(
      5 * m_segmentSize, 0, false, priorInFlight, Seconds(0));
}

void TcpRateLinuxWithBufferTest::DoTeardown() {}

class TcpRateOpsTestSuite : public TestSuite {
public:
  TcpRateOpsTestSuite() : TestSuite("tcp-rate-ops", UNIT) {
    AddTestCase(new TcpRateLinuxBasicTest(1000, SequenceNumber32(20),
                                          SequenceNumber32(11), 1,
                                          "Testing SkbDelivered and SkbSent"),
                TestCase::QUICK);
    AddTestCase(new TcpRateLinuxBasicTest(
                    1000, SequenceNumber32(11), SequenceNumber32(11), 2,
                    "Testing SkbDelivered and SkbSent with app limited data"),
                TestCase::QUICK);

    std::vector<uint32_t> toDrop;
    toDrop.push_back(4001);
    AddTestCase(
        new TcpRateLinuxWithSocketsTest(
            "Checking Rate sample value without SACK, one drop", false, toDrop),
        TestCase::QUICK);

    AddTestCase(
        new TcpRateLinuxWithSocketsTest(
            "Checking Rate sample value with SACK, one drop", true, toDrop),
        TestCase::QUICK);
    toDrop.push_back(4001);
    AddTestCase(
        new TcpRateLinuxWithSocketsTest(
            "Checking Rate sample value without SACK, two drop", false, toDrop),
        TestCase::QUICK);

    AddTestCase(
        new TcpRateLinuxWithSocketsTest(
            "Checking Rate sample value with SACK, two drop", true, toDrop),
        TestCase::QUICK);

    AddTestCase(
        new TcpRateLinuxWithBufferTest(
            1000, "Checking rate sample values with arbitrary SACK Block"),
        TestCase::QUICK);

    AddTestCase(
        new TcpRateLinuxWithBufferTest(
            500, "Checking rate sample values with arbitrary SACK Block"),
        TestCase::QUICK);
  }
};

static TcpRateOpsTestSuite g_TcpRateOpsTestSuite;
