
#include "ns3/log.h"
#include "ns3/tcp-congestion-ops.h"
#include "ns3/tcp-hybla.h"
#include "ns3/tcp-socket-base.h"
#include "ns3/test.h"

using namespace ns3;

NS_LOG_COMPONENT_DEFINE("TcpHyblaTestSuite");

class TcpHyblaIncrementTest : public TestCase {
public:
  TcpHyblaIncrementTest(uint32_t cWnd, uint32_t ssThresh, uint32_t segmentSize,
                        const Time &rtt, const std::string &name);

private:
  void DoRun() override;

  void RhoUpdated(double oldVal, double newVal);

  uint32_t m_cWnd;
  uint32_t m_ssThresh;
  uint32_t m_segmentSize;
  Time m_rtt;
  double m_rho;
  Ptr<TcpSocketState> m_state;
};

TcpHyblaIncrementTest::TcpHyblaIncrementTest(uint32_t cWnd, uint32_t ssThresh,
                                             uint32_t segmentSize,
                                             const Time &rtt,
                                             const std::string &name)
    : TestCase(name), m_cWnd(cWnd), m_ssThresh(ssThresh),
      m_segmentSize(segmentSize), m_rtt(rtt), m_rho(0) {}

void TcpHyblaIncrementTest::RhoUpdated(double, double newVal) {
  m_rho = newVal;
}

void TcpHyblaIncrementTest::DoRun() {
  m_state = CreateObject<TcpSocketState>();

  m_state->m_cWnd = m_cWnd;
  m_state->m_ssThresh = m_ssThresh;
  m_state->m_segmentSize = m_segmentSize;
  m_state->m_minRtt = m_rtt;

  Ptr<TcpHybla> cong = CreateObject<TcpHybla>();

  cong->TraceConnectWithoutContext(
      "Rho", MakeCallback(&TcpHyblaIncrementTest::RhoUpdated, this));

  TimeValue rRtt;
  cong->GetAttribute("RRTT", rRtt);
  cong->PktsAcked(m_state, 1, m_rtt);

  double calcRho = std::max(m_rtt.GetSeconds() / rRtt.Get().GetSeconds(), 1.0);

  NS_TEST_ASSERT_MSG_NE(m_rho, 0.0, "Rho never updated by implementation");
  NS_TEST_ASSERT_MSG_EQ_TOL(
      calcRho, m_rho, 0.01,
      "Different rho values between implementation and test");

  cong->IncreaseWindow(m_state, 1);

  if (m_cWnd <= m_ssThresh) {
    double inc = std::pow(2, calcRho) - 1.0;
    uint32_t cWndExpected = m_cWnd + (inc * m_segmentSize);
    NS_TEST_ASSERT_MSG_LT_OR_EQ(m_state->m_cWnd.Get(),
                                m_state->m_ssThresh.Get(),
                                "Congestion window has gone too far");
    NS_TEST_ASSERT_MSG_EQ(m_state->m_cWnd.Get(), cWndExpected,
                          "Congestion window different than expected");
  } else {
    uint32_t segCwnd = m_cWnd / m_segmentSize;
    double inc = std::pow(m_rho, 2) / ((double)segCwnd);
    uint32_t cWndExpected = m_cWnd + (inc * m_segmentSize);

    if (inc >= 1.0) {
      NS_TEST_ASSERT_MSG_LT_OR_EQ(m_state->m_cWnd.Get(), cWndExpected,
                                  "Congestion window different than expected");
    }
  }
}

class TcpHyblaTestSuite : public TestSuite {
public:
  TcpHyblaTestSuite() : TestSuite("tcp-hybla-test", UNIT) {
    AddTestCase(new TcpHyblaIncrementTest(1000, 0xFFFFFFFF, 500,
                                          MilliSeconds(55),
                                          "Rho=1.1, slow start"),
                TestCase::QUICK);
    AddTestCase(new TcpHyblaIncrementTest(1000, 0xFFFFFFFF, 500,
                                          MilliSeconds(100),
                                          "Rho=2, slow start"),
                TestCase::QUICK);
    AddTestCase(new TcpHyblaIncrementTest(1000, 0xFFFFFFFF, 500,
                                          MilliSeconds(750),
                                          "Rho=30, slow start"),
                TestCase::QUICK);
    AddTestCase(new TcpHyblaIncrementTest(1000, 500, 500, Seconds(0.55),
                                          "Rho=1.1, cong avoid"),
                TestCase::QUICK);
    AddTestCase(new TcpHyblaIncrementTest(1000, 500, 500, Seconds(0.1),
                                          "Rho=2, cong avoid"),
                TestCase::QUICK);
    AddTestCase(new TcpHyblaIncrementTest(1000, 500, 500, Seconds(0.75),
                                          "Rho=30, cong avoid"),
                TestCase::QUICK);
  }
};

static TcpHyblaTestSuite g_tcpHyblaTest;
