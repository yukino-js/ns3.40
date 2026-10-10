
#include "tcp-general-test.h"

#include "ns3/log.h"
#include "ns3/node.h"
#include "ns3/tcp-header.h"

using namespace ns3;

NS_LOG_COMPONENT_DEFINE("TcpPktsAckedTestSuite");

class DummyCongControl;

class TcpPktsAckedOpenTest : public TcpGeneralTest {
public:
  TcpPktsAckedOpenTest(const std::string &desc);

  void PktsAckedCalled(uint32_t segmentsAcked);

protected:
  Ptr<TcpSocketMsgBase> CreateSenderSocket(Ptr<Node> node) override;
  void Rx(const Ptr<const Packet> p, const TcpHeader &h,
          SocketWho who) override;

  void ConfigureEnvironment() override;

  void FinalChecks() override;

private:
  uint32_t m_segmentsAcked;
  uint32_t m_segmentsReceived;

  Ptr<DummyCongControl> m_congCtl;
};

class DummyCongControl : public TcpNewReno {
public:
  static TypeId GetTypeId();

  DummyCongControl() {}

  void SetCallback(Callback<void, uint32_t> test) { m_test = test; }

  void PktsAcked(Ptr<TcpSocketState> tcb, uint32_t segmentsAcked,
                 const Time &rtt) override {
    m_test(segmentsAcked);
  }

private:
  Callback<void, uint32_t> m_test;
};

TypeId DummyCongControl::GetTypeId() {
  static TypeId tid = TypeId("ns3::DummyCongControl")
                          .SetParent<TcpNewReno>()
                          .AddConstructor<DummyCongControl>()
                          .SetGroupName("Internet");
  return tid;
}

TcpPktsAckedOpenTest::TcpPktsAckedOpenTest(const std::string &desc)
    : TcpGeneralTest(desc), m_segmentsAcked(0), m_segmentsReceived(0) {}

void TcpPktsAckedOpenTest::ConfigureEnvironment() {
  TcpGeneralTest::ConfigureEnvironment();
  SetAppPktCount(20);
  SetMTU(500);
}

Ptr<TcpSocketMsgBase> TcpPktsAckedOpenTest::CreateSenderSocket(Ptr<Node> node) {
  Ptr<TcpSocketMsgBase> s = TcpGeneralTest::CreateSenderSocket(node);
  m_congCtl = CreateObject<DummyCongControl>();
  m_congCtl->SetCallback(
      MakeCallback(&TcpPktsAckedOpenTest::PktsAckedCalled, this));
  s->SetCongestionControlAlgorithm(m_congCtl);

  return s;
}

void TcpPktsAckedOpenTest::PktsAckedCalled(uint32_t segmentsAcked) {
  m_segmentsAcked += segmentsAcked;
}

void TcpPktsAckedOpenTest::Rx(const Ptr<const Packet> p, const TcpHeader &h,
                              SocketWho who) {
  if (who == SENDER && (!(h.GetFlags() & TcpHeader::SYN))) {
    m_segmentsReceived = h.GetAckNumber().GetValue();
  }
}

void TcpPktsAckedOpenTest::FinalChecks() {
  NS_TEST_ASSERT_MSG_EQ(
      m_segmentsReceived / GetSegSize(SENDER), m_segmentsAcked,
      "Not all acked segments have been passed to PktsAcked method");
}

class TcpPktsAckedTestSuite : public TestSuite {
public:
  TcpPktsAckedTestSuite() : TestSuite("tcp-pkts-acked-test", UNIT) {
    AddTestCase(new TcpPktsAckedOpenTest("PktsAcked check while in OPEN state"),
                TestCase::QUICK);
  }
};

static TcpPktsAckedTestSuite g_TcpPktsAckedTestSuite;
