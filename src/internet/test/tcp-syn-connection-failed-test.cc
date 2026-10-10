
#include "ns3/core-module.h"
#include "ns3/internet-module.h"
#include "ns3/network-module.h"
#include "ns3/test.h"

#include <iostream>

using namespace ns3;

class TcpSynConnectionFailedTest : public TestCase {
public:
  TcpSynConnectionFailedTest(std::string desc, bool useEcn);

  void HandleConnectionFailed(Ptr<Socket> socket);
  void DoRun() override;

private:
  bool m_connectionFailed{false};
  bool m_useEcn{false};
};

TcpSynConnectionFailedTest::TcpSynConnectionFailedTest(std::string desc,
                                                       bool useEcn)
    : TestCase(desc), m_useEcn(useEcn) {}

void TcpSynConnectionFailedTest::HandleConnectionFailed(Ptr<Socket> socket) {
  m_connectionFailed = true;
}

void TcpSynConnectionFailedTest::DoRun() {
  Ptr<Node> node = CreateObject<Node>();

  InternetStackHelper internet;
  internet.Install(node);

  TypeId tid = TcpSocketFactory::GetTypeId();

  Ptr<Socket> socket = Socket::CreateSocket(node, tid);
  if (m_useEcn) {
    Ptr<TcpSocketBase> tcpSocket = DynamicCast<TcpSocketBase>(socket);
    tcpSocket->SetUseEcn(TcpSocketState::On);
  }
  socket->Bind();
  socket->SetConnectCallback(
      MakeNullCallback<void, Ptr<Socket>>(),
      MakeCallback(&TcpSynConnectionFailedTest::HandleConnectionFailed, this));
  socket->Connect(InetSocketAddress(Ipv4Address::GetLoopback(), 9));

  Simulator::Run();
  Simulator::Destroy();

  NS_TEST_ASSERT_MSG_EQ(m_connectionFailed, true,
                        "Connection failed callback was not called");
}

class TcpSynConnectionFailedTestSuite : public TestSuite {
public:
  TcpSynConnectionFailedTestSuite()
      : TestSuite("tcp-syn-connection-failed-test", UNIT) {
    AddTestCase(new TcpSynConnectionFailedTest(
                    "TCP SYN connection failed test no ECN", false),
                TestCase::QUICK);
    AddTestCase(new TcpSynConnectionFailedTest(
                    "TCP SYN connection failed test with ECN", true),
                TestCase::QUICK);
  }
};

static TcpSynConnectionFailedTestSuite g_TcpSynConnectionFailedTestSuite;
