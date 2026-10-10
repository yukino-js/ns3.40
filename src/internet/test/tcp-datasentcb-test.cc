
#include "tcp-general-test.h"

#include "ns3/log.h"
#include "ns3/node.h"
#include "ns3/tcp-header.h"

using namespace ns3;

NS_LOG_COMPONENT_DEFINE("TcpDatSentCbTest");

class TcpSocketHalfAck : public TcpSocketMsgBase {
public:
  static TypeId GetTypeId();

  TcpSocketHalfAck() : TcpSocketMsgBase() {}

protected:
  Ptr<TcpSocketBase> Fork() override;
  void ReceivedData(Ptr<Packet> packet, const TcpHeader &tcpHeader) override;
};

NS_OBJECT_ENSURE_REGISTERED(TcpSocketHalfAck);

TypeId TcpSocketHalfAck::GetTypeId() {
  static TypeId tid = TypeId("ns3::TcpSocketHalfAck")
                          .SetParent<TcpSocketMsgBase>()
                          .SetGroupName("Internet")
                          .AddConstructor<TcpSocketHalfAck>();
  return tid;
}

Ptr<TcpSocketBase> TcpSocketHalfAck::Fork() {
  return CopyObject<TcpSocketHalfAck>(this);
}

void TcpSocketHalfAck::ReceivedData(Ptr<Packet> packet,
                                    const TcpHeader &tcpHeader) {
  NS_LOG_FUNCTION(this << packet << tcpHeader);
  static uint32_t times = 1;

  Ptr<Packet> halved = packet->Copy();

  if (times % 2 == 0) {
    halved->RemoveAtEnd(packet->GetSize() / 2);
  }

  times++;

  TcpSocketMsgBase::ReceivedData(halved, tcpHeader);
}

class TcpDataSentCbTestCase : public TcpGeneralTest {
public:
  TcpDataSentCbTestCase(const std::string &desc, uint32_t size,
                        uint32_t packets)
      : TcpGeneralTest(desc), m_pktSize(size), m_pktCount(packets),
        m_notifiedData(0) {}

protected:
  Ptr<TcpSocketMsgBase> CreateReceiverSocket(Ptr<Node> node) override;

  void DataSent(uint32_t size, SocketWho who) override;
  void ConfigureEnvironment() override;
  void FinalChecks() override;

private:
  uint32_t m_pktSize;
  uint32_t m_pktCount;
  uint32_t m_notifiedData;
};

void TcpDataSentCbTestCase::ConfigureEnvironment() {
  TcpGeneralTest::ConfigureEnvironment();
  SetAppPktCount(m_pktCount);
  SetAppPktSize(m_pktSize);
}

void TcpDataSentCbTestCase::DataSent(uint32_t size, SocketWho who) {
  NS_LOG_FUNCTION(this << who << size);

  m_notifiedData += size;
}

void TcpDataSentCbTestCase::FinalChecks() {
  NS_TEST_ASSERT_MSG_EQ(m_notifiedData, GetPktSize() * GetPktCount(),
                        "Notified more data than application sent");
}

Ptr<TcpSocketMsgBase>
TcpDataSentCbTestCase::CreateReceiverSocket(Ptr<Node> node) {
  NS_LOG_FUNCTION(this);

  return CreateSocket(node, TcpSocketHalfAck::GetTypeId(), m_congControlTypeId);
}

class TcpDataSentCbTestSuite : public TestSuite {
public:
  TcpDataSentCbTestSuite() : TestSuite("tcp-datasentcb", UNIT) {
    AddTestCase(
        new TcpDataSentCbTestCase("Check the data sent callback", 500, 10),
        TestCase::QUICK);
    AddTestCase(
        new TcpDataSentCbTestCase("Check the data sent callback", 100, 100),
        TestCase::QUICK);
    AddTestCase(
        new TcpDataSentCbTestCase("Check the data sent callback", 1000, 50),
        TestCase::QUICK);
    AddTestCase(
        new TcpDataSentCbTestCase("Check the data sent callback", 855, 18),
        TestCase::QUICK);
    AddTestCase(
        new TcpDataSentCbTestCase("Check the data sent callback", 1243, 59),
        TestCase::QUICK);
  }
};

static TcpDataSentCbTestSuite g_tcpDataSentCbTestSuite;
