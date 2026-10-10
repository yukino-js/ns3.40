
#include "tcp-general-test.h"

#include "ns3/log.h"
#include "ns3/node.h"
#include "ns3/tcp-header.h"
#include "ns3/tcp-option-ts.h"

using namespace ns3;

NS_LOG_COMPONENT_DEFINE("TimestampTestSuite");

class TimestampTestCase : public TcpGeneralTest {
public:
  enum Configuration { DISABLED, ENABLED_RECEIVER, ENABLED_SENDER, ENABLED };

  TimestampTestCase(TimestampTestCase::Configuration conf);

protected:
  Ptr<TcpSocketMsgBase> CreateReceiverSocket(Ptr<Node> node) override;
  Ptr<TcpSocketMsgBase> CreateSenderSocket(Ptr<Node> node) override;

  void Tx(const Ptr<const Packet> p, const TcpHeader &h,
          SocketWho who) override;
  void Rx(const Ptr<const Packet> p, const TcpHeader &h,
          SocketWho who) override;

  Configuration m_configuration;
};

TimestampTestCase::TimestampTestCase(TimestampTestCase::Configuration conf)
    : TcpGeneralTest("Testing the TCP Timestamp option") {
  m_configuration = conf;
}

Ptr<TcpSocketMsgBase> TimestampTestCase::CreateReceiverSocket(Ptr<Node> node) {
  Ptr<TcpSocketMsgBase> socket = TcpGeneralTest::CreateReceiverSocket(node);

  switch (m_configuration) {
  case DISABLED:
    socket->SetAttribute("Timestamp", BooleanValue(false));
    break;

  case ENABLED_RECEIVER:
    socket->SetAttribute("Timestamp", BooleanValue(true));
    break;

  case ENABLED_SENDER:
    socket->SetAttribute("Timestamp", BooleanValue(false));
    break;

  case ENABLED:
    socket->SetAttribute("Timestamp", BooleanValue(true));
    break;
  }

  return socket;
}

Ptr<TcpSocketMsgBase> TimestampTestCase::CreateSenderSocket(Ptr<Node> node) {
  Ptr<TcpSocketMsgBase> socket = TcpGeneralTest::CreateSenderSocket(node);

  switch (m_configuration) {
  case DISABLED:
    socket->SetAttribute("Timestamp", BooleanValue(false));
    break;

  case ENABLED_RECEIVER:
    socket->SetAttribute("Timestamp", BooleanValue(false));
    break;

  case ENABLED_SENDER:
    socket->SetAttribute("Timestamp", BooleanValue(true));
    break;

  case ENABLED:
    socket->SetAttribute("Timestamp", BooleanValue(true));
    break;
  }

  return socket;
}

void TimestampTestCase::Tx(const Ptr<const Packet> p, const TcpHeader &h,
                           SocketWho who) {
  if (m_configuration == DISABLED) {
    NS_TEST_ASSERT_MSG_EQ(h.HasOption(TcpOption::TS), false,
                          "timestamp disabled but option enabled");
  } else if (m_configuration == ENABLED) {
    NS_TEST_ASSERT_MSG_EQ(h.HasOption(TcpOption::TS), true,
                          "timestamp enabled but option disabled");
  }

  NS_LOG_INFO(h);
  if (who == SENDER) {
    if (h.GetFlags() & TcpHeader::SYN) {
      if (m_configuration == ENABLED_RECEIVER) {
        NS_TEST_ASSERT_MSG_EQ(h.HasOption(TcpOption::TS), false,
                              "timestamp disabled but option enabled");
      } else if (m_configuration == ENABLED_SENDER) {
        NS_TEST_ASSERT_MSG_EQ(h.HasOption(TcpOption::TS), true,
                              "timestamp enabled but option disabled");
      }
    } else {
      if (m_configuration != ENABLED) {
        NS_TEST_ASSERT_MSG_EQ(h.HasOption(TcpOption::TS), false,
                              "timestamp disabled but option enabled");
      }
    }
  } else if (who == RECEIVER) {
    if (h.GetFlags() & TcpHeader::SYN) {
      if (m_configuration == ENABLED_RECEIVER) {
        NS_TEST_ASSERT_MSG_EQ(h.HasOption(TcpOption::TS), false,
                              "sender has not ts, but receiver sent anyway");
      } else if (m_configuration == ENABLED_SENDER) {
        NS_TEST_ASSERT_MSG_EQ(h.HasOption(TcpOption::TS), false,
                              "receiver has not ts enabled but sent anyway");
      }
    } else {
      if (m_configuration != ENABLED) {
        NS_TEST_ASSERT_MSG_EQ(h.HasOption(TcpOption::TS), false,
                              "timestamp disabled but option enabled");
      }
    }
  }
}

void TimestampTestCase::Rx(const Ptr<const Packet> p, const TcpHeader &h,
                           SocketWho who) {
  if (who == SENDER) {
  } else if (who == RECEIVER) {
  }
}

class TimestampValueTestCase : public TestCase {
public:
  TimestampValueTestCase(double startTime, double timeToWait, std::string name);

private:
  void DoRun() override;
  void DoTeardown() override;

  void Check();
  void Init();

  double m_startTime;
  double m_timeToWait;
  double m_initValue;
};

TimestampValueTestCase::TimestampValueTestCase(double startTime,
                                               double timeToWait,
                                               std::string name)
    : TestCase(name) {
  m_startTime = startTime;
  m_timeToWait = timeToWait;
}

void TimestampValueTestCase::DoRun() {
  Simulator::Schedule(Seconds(m_startTime + m_timeToWait),
                      &TimestampValueTestCase::Check, this);
  Simulator::Schedule(Seconds(m_startTime), &TimestampValueTestCase::Init,
                      this);

  Simulator::Run();
}

void TimestampValueTestCase::DoTeardown() { Simulator::Destroy(); }

void TimestampValueTestCase::Init() {
  m_initValue = TcpOptionTS::NowToTsValue();
}

void TimestampValueTestCase::Check() {
  uint32_t lastValue = TcpOptionTS::NowToTsValue();

  NS_TEST_ASSERT_MSG_EQ_TOL(MilliSeconds(lastValue - m_initValue),
                            Seconds(m_timeToWait), MilliSeconds(1),
                            "Different TS values");

  NS_TEST_ASSERT_MSG_EQ_TOL(TcpOptionTS::ElapsedTimeFromTsValue(m_initValue),
                            Seconds(m_timeToWait), MilliSeconds(1),
                            "Estimating Wrong RTT");
}

class TcpTimestampTestSuite : public TestSuite {
public:
  TcpTimestampTestSuite() : TestSuite("tcp-timestamp", UNIT) {
    AddTestCase(new TimestampTestCase(TimestampTestCase::DISABLED),
                TestCase::QUICK);
    AddTestCase(new TimestampTestCase(TimestampTestCase::ENABLED_RECEIVER),
                TestCase::QUICK);
    AddTestCase(new TimestampTestCase(TimestampTestCase::ENABLED_SENDER),
                TestCase::QUICK);
    AddTestCase(new TimestampTestCase(TimestampTestCase::ENABLED),
                TestCase::QUICK);
    AddTestCase(new TimestampValueTestCase(0.0, 0.01, "Value Check"),
                TestCase::QUICK);
    AddTestCase(new TimestampValueTestCase(3.0, 0.5, "Value Check"),
                TestCase::QUICK);
    AddTestCase(new TimestampValueTestCase(5.5, 1.0, "Value Check"),
                TestCase::QUICK);
    AddTestCase(new TimestampValueTestCase(6.0, 2.0, "Value Check"),
                TestCase::QUICK);
    AddTestCase(new TimestampValueTestCase(2.4, 0.7, "Value Check"),
                TestCase::QUICK);
  }
};

static TcpTimestampTestSuite g_tcpTimestampTestSuite;
