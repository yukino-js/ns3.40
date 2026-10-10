
#include "ns3/log.h"
#include "ns3/tcp-bbr.h"
#include "ns3/tcp-congestion-ops.h"
#include "ns3/tcp-socket-base.h"
#include "ns3/test.h"

namespace ns3 {

NS_LOG_COMPONENT_DEFINE("TcpBbrTestSuite");

class TcpBbrPacingEnableTest : public TestCase {
public:
  TcpBbrPacingEnableTest(bool pacing, const std::string &name);

private:
  void DoRun() override;
  void ExecuteTest();
  bool m_pacing;
};

TcpBbrPacingEnableTest::TcpBbrPacingEnableTest(bool pacing,
                                               const std::string &name)
    : TestCase(name), m_pacing(pacing) {}

void TcpBbrPacingEnableTest::DoRun() {
  Simulator::Schedule(Seconds(0.0), &TcpBbrPacingEnableTest::ExecuteTest, this);
  Simulator::Run();
  Simulator::Destroy();
}

void TcpBbrPacingEnableTest::ExecuteTest() {
  Ptr<TcpSocketState> state = CreateObject<TcpSocketState>();
  state->m_pacing = m_pacing;

  Ptr<TcpBbr> cong = CreateObject<TcpBbr>();

  cong->CongestionStateSet(state, TcpSocketState::CA_OPEN);

  NS_TEST_ASSERT_MSG_EQ(state->m_pacing, true,
                        "BBR has not updated pacing value");
}

class TcpBbrCheckGainValuesTest : public TestCase {
public:
  TcpBbrCheckGainValuesTest(TcpBbr::BbrMode_t state, double highGain,
                            const std::string &name);

private:
  void DoRun() override;
  void ExecuteTest();
  TcpBbr::BbrMode_t m_mode;
  double m_highGain;
};

TcpBbrCheckGainValuesTest::TcpBbrCheckGainValuesTest(TcpBbr::BbrMode_t state,
                                                     double highGain,
                                                     const std::string &name)
    : TestCase(name), m_mode(state), m_highGain(highGain) {}

void TcpBbrCheckGainValuesTest::DoRun() {
  Simulator::Schedule(Seconds(0.0), &TcpBbrCheckGainValuesTest::ExecuteTest,
                      this);
  Simulator::Run();
  Simulator::Destroy();
}

void TcpBbrCheckGainValuesTest::ExecuteTest() {
  Ptr<TcpBbr> cong = CreateObject<TcpBbr>();
  cong->SetAttribute("HighGain", DoubleValue(m_highGain));
  double actualPacingGain;
  double actualCwndGain;
  double desiredPacingGain = m_highGain;
  double desiredCwndGain = m_highGain;
  TcpBbr::BbrMode_t desiredMode = TcpBbr::BBR_STARTUP;
  switch (m_mode) {
  case TcpBbr::BBR_STARTUP:
    cong->EnterStartup();
    desiredPacingGain = m_highGain;
    desiredCwndGain = m_highGain;
    actualPacingGain = cong->GetPacingGain();
    actualCwndGain = cong->GetCwndGain();
    desiredMode = TcpBbr::BBR_STARTUP;
    break;
  case TcpBbr::BBR_DRAIN:
    cong->EnterDrain();
    desiredPacingGain = 1 / m_highGain;
    desiredCwndGain = m_highGain;
    desiredMode = TcpBbr::BBR_DRAIN;
    break;
  case TcpBbr::BBR_PROBE_BW:
    cong->EnterProbeBW();
    desiredPacingGain = 1.25;
    desiredCwndGain = 2;
    desiredMode = TcpBbr::BBR_PROBE_BW;
    break;
  case TcpBbr::BBR_PROBE_RTT:
    cong->EnterProbeRTT();
    desiredPacingGain = 1;
    desiredCwndGain = 1;
    desiredMode = TcpBbr::BBR_PROBE_RTT;
    break;
  default:
    NS_ASSERT(false);
  }

  actualPacingGain = cong->GetPacingGain();
  actualCwndGain = cong->GetCwndGain();
  NS_TEST_ASSERT_MSG_EQ(m_mode, desiredMode,
                        "BBR has not entered into desired state");
  NS_TEST_ASSERT_MSG_EQ(actualPacingGain, desiredPacingGain,
                        "BBR has not updated into desired pacing gain");
  NS_TEST_ASSERT_MSG_EQ(actualCwndGain, desiredCwndGain,
                        "BBR has not updated into desired cwnd gain");
}

class TcpBbrTestSuite : public TestSuite {
public:
  TcpBbrTestSuite() : TestSuite("tcp-bbr-test", UNIT) {
    AddTestCase(
        new TcpBbrPacingEnableTest(true, "BBR must keep pacing feature on"),
        TestCase::QUICK);

    AddTestCase(
        new TcpBbrPacingEnableTest(false, "BBR must turn on pacing feature"),
        TestCase::QUICK);

    AddTestCase(
        new TcpBbrCheckGainValuesTest(TcpBbr::BBR_STARTUP, 4,
                                      "BBR should enter to STARTUP phase and "
                                      "set cwnd and pacing gain accordingly"),
        TestCase::QUICK);

    AddTestCase(
        new TcpBbrCheckGainValuesTest(TcpBbr::BBR_DRAIN, 4,
                                      "BBR should enter to DRAIN phase and set "
                                      "cwnd and pacing gain accordingly"),
        TestCase::QUICK);

    AddTestCase(new TcpBbrCheckGainValuesTest(
                    TcpBbr::BBR_PROBE_BW, 4,
                    "BBR should enter to BBR_PROBE_BW phase and set cwnd and "
                    "pacing gain accordingly"),
                TestCase::QUICK);

    AddTestCase(new TcpBbrCheckGainValuesTest(
                    TcpBbr::BBR_PROBE_RTT, 4,
                    "BBR should enter to BBR_PROBE_RTT phase and set cwnd and "
                    "pacing gain accordingly"),
                TestCase::QUICK);
  }
};

static TcpBbrTestSuite g_tcpBbrTest;
} // namespace ns3
