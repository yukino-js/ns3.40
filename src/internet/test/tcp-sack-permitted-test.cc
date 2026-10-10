
#include "tcp-general-test.h"

#include "ns3/log.h"
#include "ns3/node.h"
#include "ns3/tcp-header.h"
#include "ns3/tcp-option-sack-permitted.h"

using namespace ns3;

NS_LOG_COMPONENT_DEFINE("SackPermittedTestSuite");

class SackPermittedTestCase : public TcpGeneralTest {
public:
  enum Configuration { DISABLED, ENABLED_RECEIVER, ENABLED_SENDER, ENABLED };

  SackPermittedTestCase(SackPermittedTestCase::Configuration conf);

protected:
  Ptr<TcpSocketMsgBase> CreateReceiverSocket(Ptr<Node> node) override;
  Ptr<TcpSocketMsgBase> CreateSenderSocket(Ptr<Node> node) override;

  void Tx(const Ptr<const Packet> p, const TcpHeader &h,
          SocketWho who) override;

  Configuration m_configuration;
};

SackPermittedTestCase::SackPermittedTestCase(
    SackPermittedTestCase::Configuration conf)
    : TcpGeneralTest("Testing the TCP Sack Permitted option") {
  m_configuration = conf;
}

Ptr<TcpSocketMsgBase>
SackPermittedTestCase::CreateReceiverSocket(Ptr<Node> node) {
  Ptr<TcpSocketMsgBase> socket = TcpGeneralTest::CreateReceiverSocket(node);

  switch (m_configuration) {
  case DISABLED:
    socket->SetAttribute("Sack", BooleanValue(false));
    break;

  case ENABLED_RECEIVER:
    socket->SetAttribute("Sack", BooleanValue(true));
    break;

  case ENABLED_SENDER:
    socket->SetAttribute("Sack", BooleanValue(false));
    break;

  case ENABLED:
    socket->SetAttribute("Sack", BooleanValue(true));
    break;
  }

  return socket;
}

Ptr<TcpSocketMsgBase>
SackPermittedTestCase::CreateSenderSocket(Ptr<Node> node) {
  Ptr<TcpSocketMsgBase> socket = TcpGeneralTest::CreateSenderSocket(node);

  switch (m_configuration) {
  case DISABLED:
    socket->SetAttribute("Sack", BooleanValue(false));
    break;

  case ENABLED_RECEIVER:
    socket->SetAttribute("Sack", BooleanValue(false));
    break;

  case ENABLED_SENDER:
    socket->SetAttribute("Sack", BooleanValue(true));
    break;

  case ENABLED:
    socket->SetAttribute("Sack", BooleanValue(true));
    break;
  }

  return socket;
}

void SackPermittedTestCase::Tx(const Ptr<const Packet> p, const TcpHeader &h,
                               SocketWho who) {
  if (!(h.GetFlags() & TcpHeader::SYN)) {
    NS_TEST_ASSERT_MSG_EQ(h.HasOption(TcpOption::SACKPERMITTED), false,
                          "SackPermitted in non-SYN segment");
    return;
  }

  if (m_configuration == DISABLED) {
    NS_TEST_ASSERT_MSG_EQ(h.HasOption(TcpOption::SACKPERMITTED), false,
                          "SackPermitted disabled but option enabled");
  } else if (m_configuration == ENABLED) {
    NS_TEST_ASSERT_MSG_EQ(h.HasOption(TcpOption::SACKPERMITTED), true,
                          "SackPermitted enabled but option disabled");
  }

  NS_LOG_INFO(h);
  if (who == SENDER) {
    if (h.GetFlags() & TcpHeader::SYN) {
      if (m_configuration == ENABLED_RECEIVER) {
        NS_TEST_ASSERT_MSG_EQ(h.HasOption(TcpOption::SACKPERMITTED), false,
                              "SackPermitted disabled but option enabled");
      } else if (m_configuration == ENABLED_SENDER) {
        NS_TEST_ASSERT_MSG_EQ(h.HasOption(TcpOption::SACKPERMITTED), true,
                              "SackPermitted enabled but option disabled");
      }
    } else {
      if (m_configuration != ENABLED) {
        NS_TEST_ASSERT_MSG_EQ(h.HasOption(TcpOption::SACKPERMITTED), false,
                              "SackPermitted disabled but option enabled");
      }
    }
  } else if (who == RECEIVER) {
    if (h.GetFlags() & TcpHeader::SYN) {
      if (m_configuration == ENABLED_RECEIVER) {
        NS_TEST_ASSERT_MSG_EQ(h.HasOption(TcpOption::SACKPERMITTED), false,
                              "sender has not ts, but receiver sent anyway");
      } else if (m_configuration == ENABLED_SENDER) {
        NS_TEST_ASSERT_MSG_EQ(h.HasOption(TcpOption::SACKPERMITTED), false,
                              "receiver has not ts enabled but sent anyway");
      }
    } else {
      if (m_configuration != ENABLED) {
        NS_TEST_ASSERT_MSG_EQ(h.HasOption(TcpOption::SACKPERMITTED), false,
                              "SackPermitted disabled but option enabled");
      }
    }
  }
}

class TcpSackPermittedTestSuite : public TestSuite {
public:
  TcpSackPermittedTestSuite() : TestSuite("tcp-sack-permitted", UNIT) {
    AddTestCase(new SackPermittedTestCase(SackPermittedTestCase::DISABLED),
                TestCase::QUICK);
    AddTestCase(
        new SackPermittedTestCase(SackPermittedTestCase::ENABLED_RECEIVER),
        TestCase::QUICK);
    AddTestCase(
        new SackPermittedTestCase(SackPermittedTestCase::ENABLED_SENDER),
        TestCase::QUICK);
    AddTestCase(new SackPermittedTestCase(SackPermittedTestCase::ENABLED),
                TestCase::QUICK);
  }
};

static TcpSackPermittedTestSuite g_tcpSackPermittedTestSuite;
