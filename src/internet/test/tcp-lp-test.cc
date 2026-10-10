
#include "ns3/log.h"
#include "ns3/tcp-congestion-ops.h"
#include "ns3/tcp-lp.h"
#include "ns3/tcp-socket-base.h"
#include "ns3/test.h"

namespace ns3 {

NS_LOG_COMPONENT_DEFINE("TcpLpTestSuite");

class TcpLpToNewReno : public TestCase {
public:
  TcpLpToNewReno(uint32_t cWnd, uint32_t segmentSize, uint32_t segmentsAcked,
                 uint32_t ssThresh, Time rtt, const std::string &name);

private:
  void DoRun() override;
  uint32_t m_cWnd;
  uint32_t m_segmentSize;
  uint32_t m_ssThresh;
  uint32_t m_segmentsAcked;
  Time m_rtt;
  Ptr<TcpSocketState> m_state;
};

TcpLpToNewReno::TcpLpToNewReno(uint32_t cWnd, uint32_t segmentSize,
                               uint32_t segmentsAcked, uint32_t ssThresh,
                               Time rtt, const std::string &name)
    : TestCase(name), m_cWnd(cWnd), m_segmentSize(segmentSize),
      m_ssThresh(ssThresh), m_segmentsAcked(segmentsAcked), m_rtt(rtt) {}

void TcpLpToNewReno::DoRun() {
  m_state = CreateObject<TcpSocketState>();
  m_state->m_cWnd = m_cWnd;
  m_state->m_ssThresh = m_ssThresh;
  m_state->m_segmentSize = m_segmentSize;

  Ptr<TcpSocketState> state = CreateObject<TcpSocketState>();
  state->m_cWnd = m_cWnd;
  state->m_ssThresh = m_ssThresh;
  state->m_segmentSize = m_segmentSize;

  Ptr<TcpLp> cong = CreateObject<TcpLp>();

  m_state->m_rcvTimestampValue = 2;
  m_state->m_rcvTimestampEchoReply = 1;

  cong->PktsAcked(m_state, m_segmentsAcked, m_rtt);
  cong->IncreaseWindow(m_state, m_segmentsAcked);

  Ptr<TcpNewReno> NewRenoCong = CreateObject<TcpNewReno>();
  NewRenoCong->IncreaseWindow(state, m_segmentsAcked);

  NS_TEST_ASSERT_MSG_EQ(m_state->m_cWnd.Get(), state->m_cWnd.Get(),
                        "cWnd has not updated correctly");
  Simulator::Run();
  Simulator::Destroy();
}

class TcpLpInferenceTest1 : public TestCase {
public:
  TcpLpInferenceTest1(uint32_t cWnd, uint32_t segmentSize,
                      uint32_t segmentsAcked, Time rtt,
                      const std::string &name);

private:
  void DoRun() override;

  uint32_t m_cWnd;
  uint32_t m_segmentSize;
  uint32_t m_segmentsAcked;
  Time m_rtt;
  Ptr<TcpSocketState> m_state;
};

TcpLpInferenceTest1::TcpLpInferenceTest1(uint32_t cWnd, uint32_t segmentSize,
                                         uint32_t segmentsAcked, Time rtt,
                                         const std::string &name)
    : TestCase(name), m_cWnd(cWnd), m_segmentSize(segmentSize),
      m_segmentsAcked(segmentsAcked), m_rtt(rtt) {}

void TcpLpInferenceTest1::DoRun() {
  m_state = CreateObject<TcpSocketState>();
  m_state->m_cWnd = m_cWnd;
  m_state->m_segmentSize = m_segmentSize;

  Ptr<TcpLp> cong = CreateObject<TcpLp>();

  m_state->m_rcvTimestampValue = 2;
  m_state->m_rcvTimestampEchoReply = 1;

  cong->PktsAcked(m_state, m_segmentsAcked, m_rtt);

  m_state->m_rcvTimestampValue = 14;
  m_state->m_rcvTimestampEchoReply = 4;
  cong->PktsAcked(m_state, m_segmentsAcked, m_rtt);

  m_cWnd = m_cWnd / 2;
  NS_TEST_ASSERT_MSG_EQ(m_state->m_cWnd.Get(), m_cWnd,
                        "cWnd has not updated correctly");
  Simulator::Run();
  Simulator::Destroy();
}

class TcpLpInferenceTest2 : public TestCase {
public:
  TcpLpInferenceTest2(uint32_t cWnd, uint32_t segmentSize,
                      uint32_t segmentsAcked, Time rtt,
                      const std::string &name);

private:
  void DoRun() override;

  uint32_t m_cWnd;
  uint32_t m_segmentSize;
  uint32_t m_segmentsAcked;
  Time m_rtt;
  Ptr<TcpSocketState> m_state;
};

TcpLpInferenceTest2::TcpLpInferenceTest2(uint32_t cWnd, uint32_t segmentSize,
                                         uint32_t segmentsAcked, Time rtt,
                                         const std::string &name)
    : TestCase(name), m_cWnd(cWnd), m_segmentSize(segmentSize),
      m_segmentsAcked(segmentsAcked), m_rtt(rtt) {}

void TcpLpInferenceTest2::DoRun() {
  m_state = CreateObject<TcpSocketState>();
  m_state->m_cWnd = m_cWnd;
  m_state->m_segmentSize = m_segmentSize;

  Ptr<TcpLp> cong = CreateObject<TcpLp>();

  m_state->m_rcvTimestampValue = 2;
  m_state->m_rcvTimestampEchoReply = 1;
  cong->PktsAcked(m_state, m_segmentsAcked, m_rtt);

  m_state->m_rcvTimestampValue = 14;
  m_state->m_rcvTimestampEchoReply = 4;
  cong->PktsAcked(m_state, m_segmentsAcked, m_rtt);

  m_state->m_rcvTimestampValue = 25;
  m_state->m_rcvTimestampEchoReply = 15;
  cong->PktsAcked(m_state, m_segmentsAcked, m_rtt);

  m_cWnd = 1U * m_segmentSize;

  NS_TEST_ASSERT_MSG_EQ(m_state->m_cWnd.Get(), m_cWnd,
                        "cWnd has not updated correctly");
  Simulator::Run();
  Simulator::Destroy();
}

class TcpLpTestSuite : public TestSuite {
public:
  TcpLpTestSuite() : TestSuite("tcp-lp-test", UNIT) {
    AddTestCase(new TcpLpToNewReno(
                    4 * 1446, 1446, 2, 2 * 1446, MilliSeconds(100),
                    "LP falls to New Reno if the cwd is within threshold"),
                TestCase::QUICK);

    AddTestCase(new TcpLpInferenceTest1(2 * 1446, 1446, 2, MilliSeconds(100),
                                        "LP enters Inference phase when cwd "
                                        "exceeds threshold for the first time"),
                TestCase::QUICK);

    AddTestCase(
        new TcpLpInferenceTest2(
            2 * 1446, 1446, 2, MilliSeconds(100),
            "LP reduces cWnd to 1 if cwd exceeds threshold in inference phase"),
        TestCase::QUICK);
  }
};

static TcpLpTestSuite g_tcplpTest;

} // namespace ns3
