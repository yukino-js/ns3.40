
#include "ns3/log.h"
#include "ns3/string.h"
#include "ns3/tcp-congestion-ops.h"
#include "ns3/tcp-ledbat.h"
#include "ns3/tcp-socket-base.h"
#include "ns3/test.h"

using namespace ns3;

NS_LOG_COMPONENT_DEFINE("TcpLedbatTestSuite");

class TcpLedbatToNewReno : public TestCase {
public:
  TcpLedbatToNewReno(uint32_t cWnd, uint32_t segmentSize, uint32_t ssThresh,
                     uint32_t segmentsAcked, SequenceNumber32 highTxMark,
                     SequenceNumber32 lastAckedSeq, Time rtt,
                     const std::string &name);

private:
  void DoRun() override;
  void ExecuteTest();

  uint32_t m_cWnd;
  uint32_t m_segmentSize;
  uint32_t m_segmentsAcked;
  uint32_t m_ssThresh;
  Time m_rtt;
  SequenceNumber32 m_highTxMark;
  SequenceNumber32 m_lastAckedSeq;
  Ptr<TcpSocketState> m_state;
};

TcpLedbatToNewReno::TcpLedbatToNewReno(uint32_t cWnd, uint32_t segmentSize,
                                       uint32_t ssThresh,
                                       uint32_t segmentsAcked,
                                       SequenceNumber32 highTxMark,
                                       SequenceNumber32 lastAckedSeq, Time rtt,
                                       const std::string &name)
    : TestCase(name), m_cWnd(cWnd), m_segmentSize(segmentSize),
      m_segmentsAcked(segmentsAcked), m_ssThresh(ssThresh), m_rtt(rtt),
      m_highTxMark(highTxMark), m_lastAckedSeq(lastAckedSeq) {}

void TcpLedbatToNewReno::DoRun() {
  Simulator::Schedule(Seconds(0.0), &TcpLedbatToNewReno::ExecuteTest, this);
  Simulator::Run();
  Simulator::Destroy();
}

void TcpLedbatToNewReno::ExecuteTest() {
  m_state = CreateObject<TcpSocketState>();
  m_state->m_cWnd = m_cWnd;
  m_state->m_ssThresh = m_ssThresh;
  m_state->m_segmentSize = m_segmentSize;
  m_state->m_highTxMark = m_highTxMark;
  m_state->m_lastAckedSeq = m_lastAckedSeq;

  Ptr<TcpSocketState> state = CreateObject<TcpSocketState>();
  state->m_cWnd = m_cWnd;
  state->m_ssThresh = m_ssThresh;
  state->m_segmentSize = m_segmentSize;
  state->m_highTxMark = m_highTxMark;
  state->m_lastAckedSeq = m_lastAckedSeq;

  Ptr<TcpLedbat> cong = CreateObject<TcpLedbat>();
  cong->IncreaseWindow(m_state, m_segmentsAcked);

  Ptr<TcpNewReno> NewRenoCong = CreateObject<TcpNewReno>();
  NewRenoCong->IncreaseWindow(state, m_segmentsAcked);

  NS_TEST_ASSERT_MSG_EQ(m_state->m_cWnd.Get(), state->m_cWnd.Get(),
                        "cWnd has not updated correctly");
}

class TcpLedbatIncrementTest : public TestCase {
public:
  TcpLedbatIncrementTest(uint32_t cWnd, uint32_t segmentSize, uint32_t ssThresh,
                         uint32_t segmentsAcked, SequenceNumber32 highTxMark,
                         SequenceNumber32 lastAckedSeq, Time rtt,
                         const std::string &name);

private:
  void DoRun() override;
  void ExecuteTest();

  uint32_t m_cWnd;
  uint32_t m_segmentSize;
  uint32_t m_segmentsAcked;
  uint32_t m_ssThresh;
  Time m_rtt;
  SequenceNumber32 m_highTxMark;
  SequenceNumber32 m_lastAckedSeq;
  Ptr<TcpSocketState> m_state;
};

TcpLedbatIncrementTest::TcpLedbatIncrementTest(
    uint32_t cWnd, uint32_t segmentSize, uint32_t ssThresh,
    uint32_t segmentsAcked, SequenceNumber32 highTxMark,
    SequenceNumber32 lastAckedSeq, Time rtt, const std::string &name)
    : TestCase(name), m_cWnd(cWnd), m_segmentSize(segmentSize),
      m_segmentsAcked(segmentsAcked), m_ssThresh(ssThresh), m_rtt(rtt),
      m_highTxMark(highTxMark), m_lastAckedSeq(lastAckedSeq) {}

void TcpLedbatIncrementTest::DoRun() {
  Simulator::Schedule(Seconds(0.0), &TcpLedbatIncrementTest::ExecuteTest, this);
  Simulator::Run();
  Simulator::Destroy();
}

void TcpLedbatIncrementTest::ExecuteTest() {
  m_state = CreateObject<TcpSocketState>();
  m_state->m_cWnd = m_cWnd;
  m_state->m_ssThresh = m_ssThresh;
  m_state->m_segmentSize = m_segmentSize;
  m_state->m_highTxMark = m_highTxMark;
  m_state->m_lastAckedSeq = m_lastAckedSeq;

  Ptr<TcpLedbat> cong = CreateObject<TcpLedbat>();
  cong->SetAttribute("SSParam", StringValue("no"));
  cong->SetAttribute("NoiseFilterLen", UintegerValue(1));

  m_state->m_rcvTimestampValue = 2;
  m_state->m_rcvTimestampEchoReply = 1;
  cong->PktsAcked(m_state, m_segmentsAcked, m_rtt);

  m_state->m_rcvTimestampValue = 7;
  m_state->m_rcvTimestampEchoReply = 4;
  cong->PktsAcked(m_state, m_segmentsAcked, m_rtt);

  cong->IncreaseWindow(m_state, m_segmentsAcked);

  m_cWnd = m_cWnd +
           ((0.98 * m_segmentsAcked * m_segmentSize * m_segmentSize) / m_cWnd);

  NS_TEST_ASSERT_MSG_EQ(m_state->m_cWnd.Get(), m_cWnd,
                        "cWnd has not updated correctly");
}

class TcpLedbatDecrementTest : public TestCase {
public:
  TcpLedbatDecrementTest(uint32_t cWnd, uint32_t segmentSize, uint32_t ssThresh,
                         uint32_t segmentsAcked, SequenceNumber32 highTxMark,
                         SequenceNumber32 lastAckedSeq, Time rtt,
                         const std::string &name);

private:
  void DoRun() override;
  void ExecuteTest();

  uint32_t m_cWnd;
  uint32_t m_segmentSize;
  uint32_t m_segmentsAcked;
  uint32_t m_ssThresh;
  Time m_rtt;
  SequenceNumber32 m_highTxMark;
  SequenceNumber32 m_lastAckedSeq;
  Ptr<TcpSocketState> m_state;
};

TcpLedbatDecrementTest::TcpLedbatDecrementTest(
    uint32_t cWnd, uint32_t segmentSize, uint32_t ssThresh,
    uint32_t segmentsAcked, SequenceNumber32 highTxMark,
    SequenceNumber32 lastAckedSeq, Time rtt, const std::string &name)
    : TestCase(name), m_cWnd(cWnd), m_segmentSize(segmentSize),
      m_segmentsAcked(segmentsAcked), m_ssThresh(ssThresh), m_rtt(rtt),
      m_highTxMark(highTxMark), m_lastAckedSeq(lastAckedSeq) {}

void TcpLedbatDecrementTest::DoRun() {
  Simulator::Schedule(Seconds(0.0), &TcpLedbatDecrementTest::ExecuteTest, this);
  Simulator::Run();
  Simulator::Destroy();
}

void TcpLedbatDecrementTest::ExecuteTest() {
  UintegerValue minCwnd;
  m_state = CreateObject<TcpSocketState>();
  m_state->m_cWnd = m_cWnd;
  m_state->m_ssThresh = m_ssThresh;
  m_state->m_segmentSize = m_segmentSize;
  m_state->m_highTxMark = m_highTxMark;
  m_state->m_lastAckedSeq = m_lastAckedSeq;

  Ptr<TcpLedbat> cong = CreateObject<TcpLedbat>();
  cong->SetAttribute("SSParam", StringValue("no"));
  cong->SetAttribute("NoiseFilterLen", UintegerValue(1));
  cong->GetAttribute("MinCwnd", minCwnd);

  m_state->m_rcvTimestampValue = 2;
  m_state->m_rcvTimestampEchoReply = 1;
  cong->PktsAcked(m_state, m_segmentsAcked, m_rtt);

  m_state->m_rcvTimestampValue = 205;
  m_state->m_rcvTimestampEchoReply = 6;
  cong->PktsAcked(m_state, m_segmentsAcked, m_rtt);

  cong->IncreaseWindow(m_state, m_segmentsAcked);

  m_cWnd = m_cWnd -
           ((0.98 * m_segmentsAcked * m_segmentSize * m_segmentSize) / m_cWnd);
  m_cWnd =
      std::max(m_cWnd, static_cast<uint32_t>(m_segmentSize * minCwnd.Get()));

  NS_TEST_ASSERT_MSG_EQ(m_state->m_cWnd.Get(), m_cWnd,
                        "cWnd has not updated correctly");
}

class TcpLedbatTestSuite : public TestSuite {
public:
  TcpLedbatTestSuite() : TestSuite("tcp-ledbat-test", UNIT) {
    AddTestCase(new TcpLedbatToNewReno(
                    2 * 1446, 1446, 4 * 1446, 2, SequenceNumber32(4753),
                    SequenceNumber32(3216), MilliSeconds(100),
                    "LEDBAT falls to New Reno for slowstart"),
                TestCase::QUICK);
    AddTestCase(new TcpLedbatToNewReno(
                    4 * 1446, 1446, 2 * 1446, 2, SequenceNumber32(4753),
                    SequenceNumber32(3216), MilliSeconds(100),
                    "LEDBAT falls to New Reno if timestamps are not found"),
                TestCase::QUICK);
    AddTestCase(new TcpLedbatIncrementTest(
                    2 * 1446, 1446, 4 * 1446, 2, SequenceNumber32(4753),
                    SequenceNumber32(3216), MilliSeconds(100),
                    "LEDBAT increment test"),
                TestCase::QUICK);
    AddTestCase(new TcpLedbatDecrementTest(
                    2 * 1446, 1446, 4 * 1446, 2, SequenceNumber32(4753),
                    SequenceNumber32(3216), MilliSeconds(100),
                    "LEDBAT decrement test"),
                TestCase::QUICK);
  }
};

static TcpLedbatTestSuite g_tcpledbatTest;
