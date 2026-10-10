
#include "ns3/log.h"
#include "ns3/string.h"
#include "ns3/tcp-congestion-ops.h"
#include "ns3/tcp-prr-recovery.h"
#include "ns3/tcp-recovery-ops.h"
#include "ns3/tcp-socket-base.h"
#include "ns3/test.h"

using namespace ns3;

NS_LOG_COMPONENT_DEFINE("TcpPrrRecoveryTestSuite");

class PrrRecoveryTest : public TestCase {
public:
  PrrRecoveryTest(uint32_t cWnd, uint32_t segmentSize, uint32_t ssThresh,
                  uint32_t unAckDataCount, uint32_t bytesInFlight,
                  uint32_t m_deliveredBytes, uint32_t bytesSent,
                  const std::string &reductionBound, const std::string &name);

private:
  void DoRun() override;

  uint32_t m_cWnd;
  uint32_t m_segmentSize;
  uint32_t m_ssThresh;
  uint32_t m_unAckDataCount;
  uint32_t m_bytesInFlight;
  uint32_t m_deliveredBytes;
  uint32_t m_bytesSent;
  const std::string m_reductionBound;

  Ptr<TcpSocketState> m_state;
};

PrrRecoveryTest::PrrRecoveryTest(uint32_t cWnd, uint32_t segmentSize,
                                 uint32_t ssThresh, uint32_t unAckDataCount,
                                 uint32_t bytesInFlight,
                                 uint32_t deliveredBytes, uint32_t bytesSent,
                                 const std::string &reductionBound,
                                 const std::string &name)
    : TestCase(name), m_cWnd(cWnd), m_segmentSize(segmentSize),
      m_ssThresh(ssThresh), m_unAckDataCount(unAckDataCount),
      m_bytesInFlight(bytesInFlight), m_deliveredBytes(deliveredBytes),
      m_bytesSent(bytesSent), m_reductionBound(reductionBound) {}

void PrrRecoveryTest::DoRun() {
  m_state = CreateObject<TcpSocketState>();

  m_state->m_cWnd = m_cWnd;
  m_state->m_cWndInfl = m_cWnd;
  m_state->m_segmentSize = m_segmentSize;
  m_state->m_ssThresh = m_ssThresh;
  m_state->m_bytesInFlight = m_bytesInFlight;

  Ptr<TcpPrrRecovery> recovery = CreateObject<TcpPrrRecovery>();
  recovery->SetAttribute("ReductionBound", StringValue(m_reductionBound));

  recovery->EnterRecovery(m_state, 3, m_unAckDataCount, 0);

  NS_TEST_ASSERT_MSG_GT_OR_EQ(
      m_state->m_cWnd.Get(), m_cWnd + m_segmentSize,
      "There should be at least one transmission on entering recovery");

  for (uint32_t iterator = 0; iterator < m_bytesSent;) {
    recovery->UpdateBytesSent(m_segmentSize);
    iterator += m_segmentSize;
  }

  m_bytesInFlight += m_state->m_cWnd.Get() - m_cWnd;
  m_state->m_bytesInFlight = m_bytesInFlight;
  m_cWnd = m_state->m_cWnd.Get();
  recovery->DoRecovery(m_state, m_deliveredBytes);

  if (m_bytesInFlight > m_state->m_ssThresh) {
    NS_TEST_ASSERT_MSG_LT_OR_EQ(
        m_state->m_cWnd.Get(), m_cWnd,
        "Updated cwnd should be less than or equal to the existing cwnd");
  } else {
    NS_TEST_ASSERT_MSG_GT_OR_EQ(
        m_state->m_cWnd.Get(), m_cWnd,
        "Updated cwnd should be greater than or equal to the existing cwnd");
  }
}

class PrrRecoveryTestSuite : public TestSuite {
public:
  PrrRecoveryTestSuite() : TestSuite("tcp-prr-recovery-test", UNIT) {
    AddTestCase(new PrrRecoveryTest(3000, 500, 2500, 3000, 3000, 500, 1000,
                                    "SSRB",
                                    "Prr test on cWnd when bytesInFlight is "
                                    "greater than ssThresh with SSRB"),
                TestCase::QUICK);
    AddTestCase(new PrrRecoveryTest(1000, 500, 2500, 3000, 1000, 500, 1000,
                                    "SSRB",
                                    "Prr test on cWnd when bytesInFlight is "
                                    "lower than ssThresh with SSRB"),
                TestCase::QUICK);
    AddTestCase(new PrrRecoveryTest(3000, 500, 2500, 3000, 3000, 500, 1000,
                                    "CRB",
                                    "Prr test on cWnd when bytesInFlight is "
                                    "greater than ssThresh with CRB"),
                TestCase::QUICK);
    AddTestCase(new PrrRecoveryTest(1000, 500, 2500, 3000, 1000, 500, 1000,
                                    "CRB",
                                    "Prr test on cWnd when bytesInFlight is "
                                    "lower than ssThresh with CRB"),
                TestCase::QUICK);
  }
};

static PrrRecoveryTestSuite g_TcpPrrRecoveryTest;
