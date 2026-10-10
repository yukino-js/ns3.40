
#include "ns3/log.h"
#include "ns3/tcp-congestion-ops.h"
#include "ns3/tcp-socket-base.h"
#include "ns3/tcp-veno.h"
#include "ns3/test.h"

using namespace ns3;

NS_LOG_COMPONENT_DEFINE("TcpVenoTestSuite");

class TcpVenoTest : public TestCase {
public:
  TcpVenoTest(uint32_t cWnd, uint32_t segmentSize, uint32_t ssThresh, Time rtt,
              uint32_t segmentsAcked, uint32_t numRtt, const std::string &name);

private:
  void DoRun() override;

  void AdditiveIncrease(Ptr<TcpSocketState> state, uint32_t diff,
                        UintegerValue beta);

  uint32_t MultiplicativeDecrease(uint32_t diff, const UintegerValue &beta,
                                  uint32_t bytesInFlight) const;

  void NewReno_IncreaseWindow(Ptr<TcpSocketState> state,
                              uint32_t segmentsAcked);

  uint32_t NewReno_SlowStart(Ptr<TcpSocketState> state, uint32_t segmentsAcked);

  void NewReno_CongestionAvoidance(Ptr<TcpSocketState> state,
                                   uint32_t segmentsAcked);

  uint32_t m_cWnd;
  uint32_t m_segmentSize;
  uint32_t m_ssThresh;
  Time m_rtt;
  uint32_t m_segmentsAcked;
  uint32_t m_numRtt;
  bool m_inc;
  Ptr<TcpSocketState> m_state;
};

TcpVenoTest::TcpVenoTest(uint32_t cWnd, uint32_t segmentSize, uint32_t ssThresh,
                         Time rtt, uint32_t segmentsAcked, uint32_t numRtt,
                         const std::string &name)
    : TestCase(name), m_cWnd(cWnd), m_segmentSize(segmentSize),
      m_ssThresh(ssThresh), m_rtt(rtt), m_segmentsAcked(segmentsAcked),
      m_numRtt(numRtt), m_inc(true) {}

void TcpVenoTest::DoRun() {
  m_state = CreateObject<TcpSocketState>();

  m_state->m_cWnd = m_cWnd;
  m_state->m_segmentSize = m_segmentSize;
  m_state->m_ssThresh = m_ssThresh;
  m_state->m_minRtt = m_rtt;

  Ptr<TcpVeno> cong = CreateObject<TcpVeno>();

  Time baseRtt = MilliSeconds(100);
  cong->PktsAcked(m_state, m_segmentsAcked, baseRtt);

  cong->CongestionStateSet(m_state, TcpSocketState::CA_OPEN);

  uint32_t segCwnd = m_cWnd / m_segmentSize;

  uint32_t expectedCwnd;
  double tmp = baseRtt.GetSeconds() / m_rtt.GetSeconds();
  expectedCwnd = segCwnd * tmp;

  uint32_t diff;
  diff = segCwnd - expectedCwnd;

  UintegerValue beta;
  cong->GetAttribute("Beta", beta);

  uint32_t cntRtt = 0;

  TcpSocketState state;
  state.m_cWnd = m_cWnd;
  state.m_ssThresh = m_ssThresh;
  state.m_segmentSize = m_segmentSize;

  while (m_numRtt != 0) {
    cong->PktsAcked(m_state, m_segmentsAcked, m_rtt);
    cong->IncreaseWindow(m_state, m_segmentsAcked);

    if (cntRtt == 0) {
      uint32_t ssThresh = cong->GetSsThresh(m_state, m_state->m_cWnd);

      uint32_t calculatedSsThresh =
          MultiplicativeDecrease(diff, beta, m_state->m_cWnd.Get());

      NS_TEST_ASSERT_MSG_EQ(
          ssThresh, calculatedSsThresh,
          "Veno has not decremented cWnd correctly based on its"
          "multiplicative decrease algo.");
    }

    if (cntRtt <= 2) {
      NewReno_IncreaseWindow(&state, 1);
    } else {
      AdditiveIncrease(&state, diff, beta);
    }

    NS_TEST_ASSERT_MSG_EQ(m_state->m_cWnd.Get(), state.m_cWnd.Get(),
                          "CWnd has not updated correctly based on Veno linear "
                          "increase algorithm");
    m_numRtt--;
    cntRtt++;
  }
}

void TcpVenoTest::AdditiveIncrease(Ptr<TcpSocketState> state, uint32_t diff,
                                   UintegerValue beta) {
  if (m_cWnd < m_ssThresh) {
    NewReno_SlowStart(state, 1);
  } else {
    if (diff < beta.Get()) {
      NewReno_CongestionAvoidance(state, 1);
    } else {
      if (m_inc) {
        NewReno_CongestionAvoidance(state, 1);
        m_inc = false;
      } else {
        m_inc = true;
      }
    }
  }
}

uint32_t TcpVenoTest::MultiplicativeDecrease(uint32_t diff,
                                             const UintegerValue &beta,
                                             uint32_t bytesInFlight) const {
  uint32_t calculatedSsThresh;
  if (diff < beta.Get()) {
    static double tmp = 4.0 / 5.0;
    calculatedSsThresh =
        std::max(2 * m_segmentSize, static_cast<uint32_t>(bytesInFlight * tmp));
  } else {
    calculatedSsThresh = std::max(2 * m_segmentSize, bytesInFlight / 2);
  }
  return calculatedSsThresh;
}

void TcpVenoTest::NewReno_IncreaseWindow(Ptr<TcpSocketState> state,
                                         uint32_t segmentsAcked) {
  if (state->m_cWnd < state->m_ssThresh) {
    segmentsAcked = NewReno_SlowStart(state, segmentsAcked);
  }

  if (state->m_cWnd >= state->m_ssThresh) {
    NewReno_CongestionAvoidance(state, segmentsAcked);
  }
}

uint32_t TcpVenoTest::NewReno_SlowStart(Ptr<TcpSocketState> state,
                                        uint32_t segmentsAcked) {
  if (segmentsAcked >= 1) {
    state->m_cWnd += state->m_segmentSize;
    return segmentsAcked - 1;
  }

  return 0;
}

void TcpVenoTest::NewReno_CongestionAvoidance(Ptr<TcpSocketState> state,
                                              uint32_t segmentsAcked) {
  if (segmentsAcked > 0) {
    double adder =
        static_cast<double>(state->m_segmentSize * state->m_segmentSize) /
        state->m_cWnd.Get();
    adder = std::max(1.0, adder);
    state->m_cWnd += static_cast<uint32_t>(adder);
  }
}

class TcpVenoTestSuite : public TestSuite {
public:
  TcpVenoTestSuite() : TestSuite("tcp-veno-test", UNIT) {
    AddTestCase(new TcpVenoTest(
                    38 * 1446, 1446, 40 * 1446, MilliSeconds(100), 1, 1,
                    "Veno test on cWnd in slow start and non-congestive loss"),
                TestCase::QUICK);
    AddTestCase(new TcpVenoTest(30 * 536, 536, 20 * 536, MilliSeconds(106), 1,
                                1, "Veno test on cWnd with diff < beta"),
                TestCase::QUICK);
    AddTestCase(new TcpVenoTest(60 * 536, 536, 40 * 536, MilliSeconds(106), 1,
                                3,
                                "Veno increment test on cWnd with diff > beta"),
                TestCase::QUICK);
  }
};

static TcpVenoTestSuite g_tcpVenoTest;
