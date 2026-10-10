
#include "ns3/log.h"
#include "ns3/tcp-congestion-ops.h"
#include "ns3/tcp-htcp.h"
#include "ns3/tcp-socket-base.h"
#include "ns3/test.h"

using namespace ns3;

NS_LOG_COMPONENT_DEFINE("TcpHtcpTestSuite");

class TcpHtcpIncrementTest : public TestCase {
public:
  TcpHtcpIncrementTest(uint32_t cWnd, uint32_t segmentSize,
                       uint32_t segmentsAcked, Time lastCongestion,
                       Time firstAck, Time secondAck, uint32_t expectedCwnd,
                       const std::string &name);

private:
  void DoRun() override;

  uint32_t m_cWnd;
  uint32_t m_segmentSize;
  uint32_t m_segmentsAcked;
  Time m_lastCongestion;
  Time m_firstAck;
  Time m_secondAck;
  uint32_t m_expectedCwnd;
  Ptr<TcpSocketState> m_state;
};

TcpHtcpIncrementTest::TcpHtcpIncrementTest(uint32_t cWnd, uint32_t segmentSize,
                                           uint32_t segmentsAcked,
                                           Time lastCongestion, Time firstAck,
                                           Time secondAck,
                                           uint32_t expectedCwnd,
                                           const std::string &name)
    : TestCase(name), m_cWnd(cWnd), m_segmentSize(segmentSize),
      m_segmentsAcked(segmentsAcked), m_lastCongestion(lastCongestion),
      m_firstAck(firstAck), m_secondAck(secondAck),
      m_expectedCwnd(expectedCwnd) {}

void TcpHtcpIncrementTest::DoRun() {
  NS_LOG_FUNCTION(this);
  m_state = CreateObject<TcpSocketState>();

  m_state->m_cWnd = m_cWnd;
  m_state->m_segmentSize = m_segmentSize;

  Ptr<TcpHtcp> cong = CreateObject<TcpHtcp>();
  Time lastCongestion;

  NS_LOG_DEBUG("m_cWnd: " << m_cWnd << " m_segmentSize: " << m_segmentSize
                          << " m_segmentsAcked: " << m_segmentsAcked
                          << " m_lastCongestion" << m_lastCongestion);
  Simulator::Schedule(Time(m_lastCongestion), &TcpHtcp::GetSsThresh, cong,
                      m_state, m_state->m_cWnd);
  lastCongestion = m_lastCongestion;
  Simulator::Schedule(Time(m_firstAck), &TcpHtcp::PktsAcked, cong, m_state,
                      m_segmentsAcked, Time(ns3::MilliSeconds(80)));
  Simulator::Schedule(Time(m_secondAck), &TcpHtcp::PktsAcked, cong, m_state,
                      m_segmentsAcked, Time(ns3::MilliSeconds(100)));

  Simulator::Run();
  NS_LOG_DEBUG("Simulation ran for the scheduled events");

  cong->IncreaseWindow(m_state, m_segmentsAcked);
  NS_LOG_DEBUG("m_cwnd from function: " << m_state->m_cWnd
                                        << " expected cWnd calculated: "
                                        << m_expectedCwnd);

  NS_TEST_ASSERT_MSG_EQ(m_state->m_cWnd.Get(), m_expectedCwnd,
                        "CWnd has not updated correctly");

  Simulator::Destroy();
}

class TcpHtcpTestSuite : public TestSuite {
public:
  TcpHtcpTestSuite() : TestSuite("tcp-htcp-test", UNIT) {
    AddTestCase(new TcpHtcpIncrementTest(
                    38 * 536, 536, 38, ns3::MilliSeconds(1),
                    ns3::MilliSeconds(900), ns3::MilliSeconds(1000), 20383,
                    "TcpHtcp increment test on cWnd "),
                TestCase::QUICK);
    AddTestCase(new TcpHtcpIncrementTest(38, 1, 100, ns3::MilliSeconds(1),
                                         ns3::MilliSeconds(900),
                                         ns3::MilliSeconds(1100), 40,
                                         "TcpHtcp increment test on cWnd "),
                TestCase::QUICK);
    AddTestCase(new TcpHtcpIncrementTest(
                    53 * 1446, 1446, 50, ns3::MilliSeconds(1),
                    ns3::MilliSeconds(900), ns3::MilliSeconds(1500), 76671,
                    "TcpHtcp increment test on cWnd "),
                TestCase::QUICK);
  }
};

static TcpHtcpTestSuite g_TcpHtcpTest;
