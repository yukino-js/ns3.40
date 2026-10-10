#include "tcp-general-test.h"

#include "ns3/config.h"
#include "ns3/log.h"
#include "ns3/node.h"
#include "ns3/simple-channel.h"
#include "ns3/tcp-header.h"
#include "ns3/test.h"

using namespace ns3;

NS_LOG_COMPONENT_DEFINE("TcpSlowStartTest");

class TcpSlowStartNormalTest : public TcpGeneralTest {
public:
  TcpSlowStartNormalTest(uint32_t segmentSize, uint32_t packetSize,
                         uint32_t initSsTh, uint32_t packets,
                         const TypeId &congControl, const std::string &desc);

protected:
  void CWndTrace(uint32_t oldValue, uint32_t newValue) override;
  void Tx(const Ptr<const Packet> p, const TcpHeader &h,
          SocketWho who) override;
  void Rx(const Ptr<const Packet> p, const TcpHeader &h,
          SocketWho who) override;
  void QueueDrop(SocketWho who) override;
  void PhyDrop(SocketWho who) override;

  void ConfigureEnvironment() override;
  void ConfigureProperties() override;

  uint32_t m_ackedBytes;
  uint32_t m_sentBytes;
  uint32_t m_totalAckedBytes;
  uint32_t m_allowedIncrease;

  bool m_initial;

private:
  uint32_t m_segmentSize;
  uint32_t m_packetSize;
  uint32_t m_packets;
};

TcpSlowStartNormalTest::TcpSlowStartNormalTest(
    uint32_t segmentSize, uint32_t packetSize, uint32_t initSsTh,
    uint32_t packets, const TypeId &typeId, const std::string &desc)
    : TcpGeneralTest(desc), m_ackedBytes(0), m_sentBytes(0),
      m_totalAckedBytes(0), m_allowedIncrease(0), m_initial(true),
      m_segmentSize(segmentSize), m_packetSize(packetSize), m_packets(packets) {
  m_congControlTypeId = typeId;
}

void TcpSlowStartNormalTest::ConfigureEnvironment() {
  TcpGeneralTest::ConfigureEnvironment();
  SetAppPktCount(m_packets);
  SetAppPktSize(m_packetSize);
}

void TcpSlowStartNormalTest::ConfigureProperties() {
  TcpGeneralTest::ConfigureProperties();
  SetInitialSsThresh(SENDER, 400000);
  SetSegmentSize(SENDER, m_segmentSize);
  SetSegmentSize(RECEIVER, m_segmentSize);
}

void TcpSlowStartNormalTest::QueueDrop(SocketWho who) {
  NS_FATAL_ERROR("Drop on the queue; cannot validate slow start");
}

void TcpSlowStartNormalTest::PhyDrop(SocketWho who) {
  NS_FATAL_ERROR("Drop on the phy: cannot validate slow start");
}

void TcpSlowStartNormalTest::CWndTrace(uint32_t oldValue, uint32_t newValue) {
  uint32_t segSize = GetSegSize(TcpGeneralTest::SENDER);
  uint32_t increase = newValue - oldValue;

  if (m_initial) {
    m_initial = false;
    NS_LOG_INFO("Ignored update to " << newValue << " with a segsize of "
                                     << segSize);
    return;
  }

  NS_TEST_ASSERT_MSG_EQ(increase, segSize, "Increase different than segsize");
  NS_TEST_ASSERT_MSG_LT_OR_EQ(newValue, GetInitialSsThresh(SENDER),
                              "cWnd increased over ssth");

  NS_LOG_INFO("Incremented cWnd by " << segSize << " bytes in Slow Start "
                                     << "achieving a value of " << newValue);

  NS_TEST_ASSERT_MSG_GT_OR_EQ(m_allowedIncrease, 1, "Increase not allowed");
  m_allowedIncrease--;
}

void TcpSlowStartNormalTest::Tx(const Ptr<const Packet> p, const TcpHeader &h,
                                SocketWho who) {
  NS_LOG_FUNCTION(this << p << h << who);

  if (who == SENDER && Simulator::Now().GetSeconds() > 5.0) {
    m_sentBytes += GetSegSize(TcpGeneralTest::SENDER);
  }
}

void TcpSlowStartNormalTest::Rx(const Ptr<const Packet> p, const TcpHeader &h,
                                SocketWho who) {
  NS_LOG_FUNCTION(this << p << h << who);

  if (who == SENDER && Simulator::Now().GetSeconds() > 5.0) {
    uint32_t acked = h.GetAckNumber().GetValue() - m_totalAckedBytes - 1;
    m_totalAckedBytes += acked;
    m_ackedBytes += acked;

    NS_LOG_INFO("Ack of " << acked
                          << " bytes, acked this round=" << m_ackedBytes);

    if (m_ackedBytes >= GetSegSize(SENDER)) {
      NS_LOG_INFO("FULL ACK achieved, bytes=" << m_ackedBytes);
      m_allowedIncrease += 1;
      m_ackedBytes -= GetSegSize(SENDER);
    }

    while (m_ackedBytes >= GetSegSize(SENDER)) {
      m_ackedBytes -= GetSegSize(SENDER);
    }
  }
}

class TcpSlowStartAttackerTest : public TcpSlowStartNormalTest {
public:
  TcpSlowStartAttackerTest(uint32_t segmentSize, uint32_t packetSize,
                           uint32_t initSsTh, uint32_t packets,
                           const TypeId &congControl, const std::string &desc);

protected:
  Ptr<TcpSocketMsgBase> CreateReceiverSocket(Ptr<Node> node) override;
};

TcpSlowStartAttackerTest::TcpSlowStartAttackerTest(
    uint32_t segmentSize, uint32_t packetSize, uint32_t initSsTh,
    uint32_t packets, const TypeId &typeId, const std::string &msg)
    : TcpSlowStartNormalTest(segmentSize, packetSize, initSsTh, packets, typeId,
                             msg) {}

Ptr<TcpSocketMsgBase>
TcpSlowStartAttackerTest::CreateReceiverSocket(Ptr<Node> node) {
  Ptr<TcpSocketSmallAcks> socket = DynamicCast<TcpSocketSmallAcks>(
      CreateSocket(node, TcpSocketSmallAcks::GetTypeId(), m_congControlTypeId));
  socket->SetBytesToAck(125);

  return socket;
}

class TcpSlowStartTestSuite : public TestSuite {
public:
  TcpSlowStartTestSuite() : TestSuite("tcp-slow-start-test", UNIT) {
    std::list<TypeId> types = {
        TcpNewReno::GetTypeId(),
    };

    for (const auto &t : types) {
      std::string typeName = t.GetName();

      AddTestCase(
          new TcpSlowStartNormalTest(500, 500, 10000, 10, t,
                                     "slow start 500 byte, " + typeName),
          TestCase::QUICK);
      AddTestCase(
          new TcpSlowStartNormalTest(1000, 1000, 10000, 9, t,
                                     "slow start 1000 byte, " + typeName),
          TestCase::QUICK);
      AddTestCase(
          new TcpSlowStartNormalTest(500, 250, 10000, 10, t,
                                     "slow start small packets, " + typeName),
          TestCase::QUICK);
      AddTestCase(new TcpSlowStartAttackerTest(
                      500, 500, 10000, 10, t,
                      "slow start ack attacker, 500 byte, " + typeName),
                  TestCase::QUICK);
      AddTestCase(new TcpSlowStartAttackerTest(
                      1000, 1000, 10000, 9, t,
                      "slow start ack attacker, 1000 byte, " + typeName),
                  TestCase::QUICK);
    }
  }
};

static TcpSlowStartTestSuite g_tcpSlowStartTestSuite;
