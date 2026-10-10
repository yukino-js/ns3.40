
#include "ns3/log.h"
#include "ns3/tcp-congestion-ops.h"
#include "ns3/tcp-socket-base.h"
#include "ns3/tcp-yeah.h"
#include "ns3/test.h"

using namespace ns3;

NS_LOG_COMPONENT_DEFINE("TcpYeahTestSuite");

class TcpYeahIncrementTest : public TestCase {
public:
  TcpYeahIncrementTest(uint32_t cWnd, uint32_t ssThresh, uint32_t segmentSize,
                       SequenceNumber32 nextTxSeq,
                       SequenceNumber32 lastAckedSeq, uint32_t segmentsAcked,
                       Time minRtt, const std::string &name);

private:
  void DoRun() override;
  void IncreaseWindow(Ptr<TcpYeah> cong);

  uint32_t m_cWnd;
  uint32_t m_ssThresh;
  uint32_t m_segmentSize;
  SequenceNumber32 m_nextTxSeq;
  SequenceNumber32 m_lastAckedSeq;
  uint32_t m_segmentsAcked;
  Time m_baseRtt;
  Time m_minRtt;
  uint32_t m_doingRenoNow;
  uint32_t m_cntRtt;
  uint32_t m_renoCount;
};

TcpYeahIncrementTest::TcpYeahIncrementTest(uint32_t cWnd, uint32_t ssThresh,
                                           uint32_t segmentSize,
                                           SequenceNumber32 nextTxSeq,
                                           SequenceNumber32 lastAckedSeq,
                                           uint32_t segmentsAcked, Time minRtt,
                                           const std::string &name)
    : TestCase(name), m_cWnd(cWnd), m_ssThresh(ssThresh),
      m_segmentSize(segmentSize), m_nextTxSeq(nextTxSeq),
      m_lastAckedSeq(lastAckedSeq), m_segmentsAcked(segmentsAcked),
      m_baseRtt(MilliSeconds(100)), m_minRtt(minRtt), m_doingRenoNow(0),
      m_cntRtt(4), m_renoCount(2) {}

void TcpYeahIncrementTest::DoRun() {
  Ptr<TcpSocketState> state = CreateObject<TcpSocketState>();
  state->m_cWnd = m_cWnd;
  state->m_ssThresh = m_ssThresh;
  state->m_segmentSize = m_segmentSize;
  state->m_nextTxSequence = m_nextTxSeq;
  state->m_lastAckedSeq = m_lastAckedSeq;
  state->m_minRtt = m_minRtt;

  Ptr<TcpYeah> cong = CreateObject<TcpYeah>();

  cong->PktsAcked(state, m_segmentsAcked, m_baseRtt);

  cong->CongestionStateSet(state, TcpSocketState::CA_OPEN);
  cong->PktsAcked(state, m_segmentsAcked, m_minRtt);

  cong->PktsAcked(state, m_segmentsAcked, m_minRtt);
  cong->PktsAcked(state, m_segmentsAcked, m_minRtt);

  cong->IncreaseWindow(state, m_segmentsAcked);
  IncreaseWindow(cong);

  NS_TEST_ASSERT_MSG_EQ(state->m_cWnd.Get(), m_cWnd,
                        "CWnd has not updated correctly");
}

void TcpYeahIncrementTest::IncreaseWindow(Ptr<TcpYeah> cong) {
  uint32_t segCwnd = m_cWnd / m_segmentSize;

  if (m_cWnd < m_ssThresh) {
    if (m_segmentsAcked >= 1) {
      m_cWnd += m_segmentSize;
      m_segmentsAcked -= 1;
      NS_LOG_INFO("In SlowStart, updated to cwnd " << m_cWnd << " ssthresh "
                                                   << m_ssThresh);
    }
  } else if (!m_doingRenoNow) {
    UintegerValue aiFactor;
    cong->GetAttribute("StcpAiFactor", aiFactor);
    uint32_t w = std::min(segCwnd, (uint32_t)aiFactor.Get());
    uint32_t delta = m_segmentsAcked / w;
    m_cWnd += delta * m_segmentSize;
    NS_LOG_INFO("In Fast mode, updated to cwnd " << m_cWnd << " ssthresh "
                                                 << m_ssThresh);
  } else {
    if (m_segmentsAcked > 0) {
      double adder =
          static_cast<double>(m_segmentSize * m_segmentSize) / m_cWnd;
      adder = std::max(1.0, adder);
      m_cWnd += static_cast<uint32_t>(adder);
      NS_LOG_INFO("In CongAvoid, updated to cwnd " << m_cWnd << " ssthresh "
                                                   << m_ssThresh);
    }
  }
  if (m_lastAckedSeq >= m_nextTxSeq) {
    if (m_cntRtt > 2) {
      Time rttQueue = m_minRtt - m_baseRtt;
      double bw = segCwnd / m_minRtt.GetSeconds();
      uint32_t queue = bw * rttQueue.GetSeconds();

      double L = rttQueue.GetSeconds() / m_baseRtt.GetSeconds();

      UintegerValue alpha;
      cong->GetAttribute("Alpha", alpha);
      UintegerValue phy;
      cong->GetAttribute("Phy", phy);
      UintegerValue gamma;
      cong->GetAttribute("Gamma", gamma);
      UintegerValue epsilon;
      cong->GetAttribute("Epsilon", epsilon);
      UintegerValue zeta;
      cong->GetAttribute("Zeta", zeta);

      if (queue > alpha.Get() || L > (1 / phy.Get())) {
        NS_LOG_INFO("Enter Slow mode");
        if (queue > alpha.Get() && segCwnd > m_renoCount) {
          uint32_t reduction = std::min(queue / (uint32_t)gamma.Get(),
                                        segCwnd >> (uint32_t)epsilon.Get());
          segCwnd -= reduction;
          segCwnd = std::max(segCwnd, m_renoCount);
          m_cWnd = segCwnd * m_segmentSize;
          m_ssThresh = m_cWnd;
          NS_LOG_INFO("In Slow mode, after precautionary decongestion, "
                      "updated to cwnd "
                      << m_cWnd << " ssthresh " << m_ssThresh);
        }
      }
    }
  }
}

class TcpYeahDecrementTest : public TestCase {
public:
  TcpYeahDecrementTest(uint32_t cWnd, uint32_t ssThresh, uint32_t segmentSize,
                       SequenceNumber32 nextTxSeq,
                       SequenceNumber32 lastAckedSeq, Time minRtt,
                       UintegerValue rho, const std::string &name);

private:
  void DoRun() override;
  uint32_t CalculateSsThresh(Ptr<TcpYeah> cong);

  uint32_t m_cWnd;
  uint32_t m_ssThresh;
  uint32_t m_segmentSize;
  uint32_t m_doingRenoNow;
  SequenceNumber32 m_nextTxSeq;
  SequenceNumber32 m_lastAckedSeq;
  Time m_minRtt;
  Time m_baseRtt;
  uint32_t m_segmentsAcked;
  UintegerValue m_rho;
};

TcpYeahDecrementTest::TcpYeahDecrementTest(uint32_t cWnd, uint32_t ssThresh,
                                           uint32_t segmentSize,
                                           SequenceNumber32 nextTxSeq,
                                           SequenceNumber32 lastAckedSeq,
                                           Time minRtt, UintegerValue rho,
                                           const std::string &name)
    : TestCase(name), m_cWnd(cWnd), m_ssThresh(ssThresh),
      m_segmentSize(segmentSize), m_doingRenoNow(0), m_nextTxSeq(nextTxSeq),
      m_lastAckedSeq(lastAckedSeq), m_minRtt(minRtt),
      m_baseRtt(MilliSeconds(100)), m_segmentsAcked(2), m_rho(rho) {}

void TcpYeahDecrementTest::DoRun() {
  Ptr<TcpSocketState> state = CreateObject<TcpSocketState>();
  state->m_cWnd = m_cWnd;
  state->m_nextTxSequence = m_nextTxSeq;
  state->m_lastAckedSeq = m_lastAckedSeq;
  state->m_segmentSize = m_segmentSize;
  state->m_ssThresh = m_ssThresh;

  Ptr<TcpYeah> cong = CreateObject<TcpYeah>();

  cong->SetAttribute("Rho", UintegerValue(m_rho));

  cong->PktsAcked(state, m_segmentsAcked, m_baseRtt);

  cong->CongestionStateSet(state, TcpSocketState::CA_OPEN);
  cong->PktsAcked(state, m_segmentsAcked, m_minRtt);

  cong->PktsAcked(state, m_segmentsAcked, m_minRtt);
  cong->PktsAcked(state, m_segmentsAcked, m_minRtt);

  cong->IncreaseWindow(state, m_segmentsAcked);

  uint32_t ssThresh = cong->GetSsThresh(state, m_cWnd);

  uint32_t ssThreshVal = CalculateSsThresh(cong);

  NS_TEST_ASSERT_MSG_EQ(ssThresh, ssThreshVal,
                        "SsThresh has not updated correctly");
}

uint32_t TcpYeahDecrementTest::CalculateSsThresh(Ptr<TcpYeah> cong) {
  uint32_t segCwnd = m_cWnd / m_segmentSize;
  uint32_t reduction;

  UintegerValue delta;
  cong->GetAttribute("Delta", delta);

  Time rttQueue = m_minRtt - m_baseRtt;

  double bw = segCwnd / m_minRtt.GetSeconds();
  uint32_t queue = bw * rttQueue.GetSeconds();

  NS_LOG_LOGIC("queue backlog" << queue);

  if (m_doingRenoNow < m_rho.Get()) {
    reduction = std::max(queue, segCwnd >> delta.Get());
    reduction = std::min(reduction, std::max(segCwnd >> 1, (uint32_t)2));
    NS_LOG_INFO("Reduction amount for yeah upon loss = " << reduction);
  } else {
    reduction = std::max(segCwnd >> 1, (uint32_t)2);
    NS_LOG_INFO("Reduction amount for reno upon loss = " << reduction);
  }

  return (m_cWnd - (reduction * m_segmentSize));
}

class TcpYeahTestSuite : public TestSuite {
public:
  TcpYeahTestSuite() : TestSuite("tcp-yeah-test", UNIT) {
    AddTestCase(new TcpYeahIncrementTest(
                    20 * 1446, 25 * 1446, 1446, SequenceNumber32(2893),
                    SequenceNumber32(1447), 1, MilliSeconds(105),
                    "YeAH test on cWnd when in slow start"),
                TestCase::QUICK);
    AddTestCase(new TcpYeahIncrementTest(
                    30 * 1446, 25 * 1446, 1446, SequenceNumber32(2893),
                    SequenceNumber32(1447), 30, MilliSeconds(105),
                    "YeAH test on cWnd when in Fast mode"),
                TestCase::QUICK);
    AddTestCase(new TcpYeahIncrementTest(
                    40 * 356, 30 * 356, 356, SequenceNumber32(20761),
                    SequenceNumber32(21117), 1, MilliSeconds(120),
                    "YeAH test on cWnd when in slow mode without precautionary "
                    "decongestion"),
                TestCase::QUICK);
    AddTestCase(new TcpYeahIncrementTest(
                    100 * 356, 70 * 356, 356, SequenceNumber32(20761),
                    SequenceNumber32(21117), 1, MilliSeconds(600),
                    "YeAH test on cWnd when in slow mode with precautionary "
                    "decongestion"),
                TestCase::QUICK);
    AddTestCase(
        new TcpYeahDecrementTest(
            40 * 1446, 30 * 1446, 1446, SequenceNumber32(2893),
            SequenceNumber32(7230), MilliSeconds(120), UintegerValue(0),
            "YeAH test on ssThresh upon loss while competing with Reno flows"),
        TestCase::QUICK);
    AddTestCase(new TcpYeahDecrementTest(
                    57 * 1446, 42 * 1446, 1446, SequenceNumber32(2893),
                    SequenceNumber32(7230), MilliSeconds(200), UintegerValue(2),
                    "YeAH test on ssThresh upon loss while not competing with "
                    "Reno flows"),
                TestCase::QUICK);
  }
};

static TcpYeahTestSuite g_tcpYeahTest;
