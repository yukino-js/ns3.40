
#include "tcp-general-test.h"

#include "ns3/error-model.h"
#include "ns3/log.h"

#include <list>

using namespace ns3;

NS_LOG_COMPONENT_DEFINE("TcpLossTestSuite");

class TcpLargeTransferLossTest : public TcpGeneralTest {
public:
  TcpLargeTransferLossTest(uint32_t firstLoss, uint32_t secondLoss,
                           uint32_t lastSegment, const std::string &desc);

protected:
  void ConfigureProperties() override;
  void ConfigureEnvironment() override;
  void FinalChecks() override;
  void Tx(const Ptr<const Packet> p, const TcpHeader &h,
          SocketWho who) override;
  void Rx(const Ptr<const Packet> p, const TcpHeader &h,
          SocketWho who) override;
  void CongStateTrace(const TcpSocketState::TcpCongState_t oldValue,
                      const TcpSocketState::TcpCongState_t newValue) override;
  Ptr<ErrorModel> CreateReceiverErrorModel() override;

private:
  uint32_t m_firstLoss;
  uint32_t m_secondLoss;
  uint32_t m_sent{0};
  uint32_t m_received{0};
  uint32_t m_lastSegment{0};
  std::list<int> m_expectedStates;
};

TcpLargeTransferLossTest::TcpLargeTransferLossTest(uint32_t firstLoss,
                                                   uint32_t secondLoss,
                                                   uint32_t lastSegment,
                                                   const std::string &desc)
    : TcpGeneralTest(desc), m_firstLoss(firstLoss), m_secondLoss(secondLoss),
      m_lastSegment(lastSegment) {
  NS_TEST_ASSERT_MSG_NE(m_lastSegment, 0, "Last segment should be > 0");
  NS_TEST_ASSERT_MSG_GT(m_secondLoss, m_firstLoss,
                        "Second segment number should be greater than first");
  m_expectedStates.push_back(TcpSocketState::CA_OPEN);
  m_expectedStates.push_back(TcpSocketState::CA_DISORDER);
  m_expectedStates.push_back(TcpSocketState::CA_RECOVERY);
  m_expectedStates.push_back(TcpSocketState::CA_OPEN);
  m_expectedStates.push_back(TcpSocketState::CA_DISORDER);
  m_expectedStates.push_back(TcpSocketState::CA_RECOVERY);
  m_expectedStates.push_back(TcpSocketState::CA_OPEN);
}

void TcpLargeTransferLossTest::CongStateTrace(
    const TcpSocketState::TcpCongState_t oldValue,
    const TcpSocketState::TcpCongState_t newValue) {
  int expectedOldState = m_expectedStates.front();
  m_expectedStates.pop_front();
  NS_TEST_ASSERT_MSG_EQ(oldValue, expectedOldState, "State transition wrong");
  NS_TEST_ASSERT_MSG_EQ(newValue, m_expectedStates.front(),
                        "State transition wrong");
}

void TcpLargeTransferLossTest::ConfigureEnvironment() {
  NS_LOG_FUNCTION(this);
  TcpGeneralTest::ConfigureEnvironment();
  SetPropagationDelay(MicroSeconds(1));
  SetTransmitStart(Seconds(1));
  SetAppPktSize(1000);
  SetAppPktCount(m_lastSegment);
  SetAppPktInterval(MicroSeconds(8));
}

void TcpLargeTransferLossTest::ConfigureProperties() {
  NS_LOG_FUNCTION(this);
  TcpGeneralTest::ConfigureProperties();
  SetSegmentSize(SENDER, 1000);
  SetSegmentSize(RECEIVER, 1000);
}

Ptr<ErrorModel> TcpLargeTransferLossTest::CreateReceiverErrorModel() {
  Ptr<ReceiveListErrorModel> rem = CreateObject<ReceiveListErrorModel>();
  std::list<uint32_t> errorList;
  errorList.push_back(m_firstLoss);
  errorList.push_back(m_secondLoss);
  rem->SetList(errorList);
  return rem;
}

void TcpLargeTransferLossTest::Tx(const Ptr<const Packet> p, const TcpHeader &h,
                                  SocketWho who) {
  m_sent++;
}

void TcpLargeTransferLossTest::Rx(const Ptr<const Packet> p, const TcpHeader &h,
                                  SocketWho who) {
  m_received++;
}

void TcpLargeTransferLossTest::FinalChecks() {
  NS_TEST_ASSERT_MSG_EQ(m_sent, (m_received + 2),
                        "Did not observe expected number of sent packets");
}

class TcpLossTestSuite : public TestSuite {
public:
  TcpLossTestSuite() : TestSuite("tcp-loss-test", UNIT) {
    AddTestCase(new TcpLargeTransferLossTest(
                    1000, 2000, 2500, "large-transfer-loss-without-wrap"),
                TestCase::EXTENSIVE);
    AddTestCase(new TcpLargeTransferLossTest(1000, 3294967, 3295100,
                                             "large-transfer-loss-with-wrap"),
                TestCase::EXTENSIVE);
  }
};

static TcpLossTestSuite g_tcpLossTest;
