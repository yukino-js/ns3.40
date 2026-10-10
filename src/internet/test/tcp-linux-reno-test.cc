#include "tcp-general-test.h"

#include "ns3/config.h"
#include "ns3/log.h"
#include "ns3/node.h"
#include "ns3/simple-channel.h"
#include "ns3/tcp-header.h"
#include "ns3/tcp-linux-reno.h"
#include "ns3/test.h"

using namespace ns3;

NS_LOG_COMPONENT_DEFINE("TcpLinuxRenoTest");

class TcpLinuxRenoSSTest : public TcpGeneralTest {
public:
  TcpLinuxRenoSSTest(uint32_t segmentSize, uint32_t packetSize,
                     uint32_t packets, uint32_t initialCwnd,
                     uint32_t delayedAck, uint32_t expectedCwnd,
                     TypeId &congControl, const std::string &desc);

protected:
  void CWndTrace(uint32_t oldValue, uint32_t newValue) override;
  void QueueDrop(SocketWho who) override;
  void PhyDrop(SocketWho who) override;

  void ConfigureEnvironment() override;
  void ConfigureProperties() override;
  void DoTeardown() override;

  bool m_initial;

private:
  void Tx(const Ptr<const Packet> p, const TcpHeader &h,
          SocketWho who) override;
  void Rx(const Ptr<const Packet> p, const TcpHeader &h,
          SocketWho who) override;
  uint32_t m_segmentSize;
  uint32_t m_packetSize;
  uint32_t m_packets;
  uint32_t m_initialCwnd;
  uint32_t m_delayedAck;
  uint32_t m_lastCwnd;
  uint32_t m_expectedCwnd;
};

TcpLinuxRenoSSTest::TcpLinuxRenoSSTest(uint32_t segmentSize,
                                       uint32_t packetSize, uint32_t packets,
                                       uint32_t initialCwnd,
                                       uint32_t delayedAck,
                                       uint32_t expectedCwnd, TypeId &typeId,
                                       const std::string &desc)
    : TcpGeneralTest(desc), m_initial(true), m_segmentSize(segmentSize),
      m_packetSize(packetSize), m_packets(packets), m_initialCwnd(initialCwnd),
      m_delayedAck(delayedAck), m_lastCwnd(0), m_expectedCwnd(expectedCwnd) {
  m_congControlTypeId = typeId;
}

void TcpLinuxRenoSSTest::ConfigureEnvironment() {
  TcpGeneralTest::ConfigureEnvironment();
  SetPropagationDelay(MilliSeconds(5));
  SetAppPktCount(m_packets);
  SetAppPktSize(m_packetSize);
}

void TcpLinuxRenoSSTest::ConfigureProperties() {
  TcpGeneralTest::ConfigureProperties();
  SetInitialCwnd(SENDER, m_initialCwnd);
  SetDelAckMaxCount(RECEIVER, m_delayedAck);
  SetSegmentSize(SENDER, m_segmentSize);
  SetSegmentSize(RECEIVER, m_segmentSize);
}

void TcpLinuxRenoSSTest::QueueDrop(SocketWho who) {
  NS_FATAL_ERROR("Drop on the queue; cannot validate slow start");
}

void TcpLinuxRenoSSTest::PhyDrop(SocketWho who) {
  NS_FATAL_ERROR("Drop on the phy: cannot validate slow start");
}

void TcpLinuxRenoSSTest::CWndTrace(uint32_t oldValue, uint32_t newValue) {
  NS_LOG_FUNCTION(this << oldValue << newValue);
  uint32_t segSize = GetSegSize(TcpGeneralTest::SENDER);
  uint32_t increase = newValue - oldValue;
  m_lastCwnd = newValue;

  if (m_initial) {
    m_initial = false;
    NS_TEST_ASSERT_MSG_EQ(
        newValue, m_initialCwnd * m_segmentSize,
        "The first update is for ACK of SYN and should initialize cwnd");
    return;
  }

  if (oldValue == m_initialCwnd * m_segmentSize) {
    return;
  }

  NS_TEST_ASSERT_MSG_EQ(increase, m_delayedAck * segSize,
                        "Increase different than segsize");
  NS_TEST_ASSERT_MSG_LT_OR_EQ(newValue, GetInitialSsThresh(SENDER),
                              "cWnd increased over ssth");

  NS_LOG_INFO("Incremented cWnd by " << m_delayedAck * segSize
                                     << " bytes in Slow Start "
                                     << "achieving a value of " << newValue);
}

void TcpLinuxRenoSSTest::Tx(const Ptr<const Packet> p, const TcpHeader &h,
                            SocketWho who) {
  NS_LOG_FUNCTION(this << p << h << who);
}

void TcpLinuxRenoSSTest::Rx(const Ptr<const Packet> p, const TcpHeader &h,
                            SocketWho who) {
  NS_LOG_FUNCTION(this << p << h << who);
}

void TcpLinuxRenoSSTest::DoTeardown() {
  NS_TEST_ASSERT_MSG_EQ(m_lastCwnd, m_expectedCwnd,
                        "Congestion window did not evolve as expected");
  TcpGeneralTest::DoTeardown();
}

class TcpLinuxRenoCongAvoidTest : public TcpGeneralTest {
public:
  TcpLinuxRenoCongAvoidTest(uint32_t segmentSize, uint32_t packetSize,
                            uint32_t packets, uint32_t initialCwnd,
                            uint32_t initialSSThresh, uint32_t delayedAck,
                            uint32_t expectedCwnd, TypeId &congControl,
                            const std::string &desc);

protected:
  void CWndTrace(uint32_t oldValue, uint32_t newValue) override;
  void QueueDrop(SocketWho who) override;
  void PhyDrop(SocketWho who) override;

  void ConfigureEnvironment() override;
  void ConfigureProperties() override;
  void DoTeardown() override;

private:
  void Tx(const Ptr<const Packet> p, const TcpHeader &h,
          SocketWho who) override;
  void Rx(const Ptr<const Packet> p, const TcpHeader &h,
          SocketWho who) override;
  uint32_t m_segmentSize;
  uint32_t m_packetSize;
  uint32_t m_packets;
  uint32_t m_initialCwnd;
  uint32_t m_initialSSThresh;
  uint32_t m_delayedAck;
  uint32_t m_lastCwnd;
  uint32_t m_expectedCwnd;
  uint32_t m_increment;
  bool m_initial;
  bool m_inCongAvoidance;
  bool m_inSlowStartPhase;
};

TcpLinuxRenoCongAvoidTest::TcpLinuxRenoCongAvoidTest(
    uint32_t segmentSize, uint32_t packetSize, uint32_t packets,
    uint32_t initialCwnd, uint32_t initialSSThresh, uint32_t delayedAck,
    uint32_t expectedCwnd, TypeId &typeId, const std::string &desc)
    : TcpGeneralTest(desc), m_segmentSize(segmentSize),
      m_packetSize(packetSize), m_packets(packets), m_initialCwnd(initialCwnd),
      m_initialSSThresh(initialSSThresh), m_delayedAck(delayedAck),
      m_lastCwnd(0), m_expectedCwnd(expectedCwnd), m_increment(0),
      m_initial(true), m_inCongAvoidance(false), m_inSlowStartPhase(true) {
  m_congControlTypeId = typeId;
}

void TcpLinuxRenoCongAvoidTest::ConfigureEnvironment() {
  TcpGeneralTest::ConfigureEnvironment();
  SetAppPktSize(m_packetSize);
  SetAppPktCount(m_packets);
  SetMTU(1500);
}

void TcpLinuxRenoCongAvoidTest::ConfigureProperties() {
  TcpGeneralTest::ConfigureProperties();
  SetSegmentSize(SENDER, m_segmentSize);
  SetSegmentSize(RECEIVER, m_segmentSize);
  SetInitialCwnd(SENDER, m_initialCwnd);
  SetDelAckMaxCount(RECEIVER, m_delayedAck);
  SetInitialSsThresh(SENDER, m_initialSSThresh);
}

void TcpLinuxRenoCongAvoidTest::CWndTrace(uint32_t oldValue,
                                          uint32_t newValue) {
  NS_LOG_FUNCTION(this << oldValue << newValue);
  m_lastCwnd = newValue;
  if (m_initial) {
    m_initial = false;
    NS_TEST_ASSERT_MSG_EQ(
        newValue, m_initialCwnd * m_segmentSize,
        "The first update is for ACK of SYN and should initialize cwnd");
    return;
  }

  if ((newValue >= m_initialSSThresh * m_segmentSize) && !m_inCongAvoidance &&
      (oldValue != m_initialSSThresh)) {
    m_inCongAvoidance = true;
    m_inSlowStartPhase = false;
    return;
  }

  if (m_inSlowStartPhase) {
    return;
  }

  m_increment = newValue - oldValue;

  NS_TEST_ASSERT_MSG_EQ(m_increment, m_segmentSize,
                        "Increase different than segsize");
}

void TcpLinuxRenoCongAvoidTest::QueueDrop(SocketWho who) {
  NS_FATAL_ERROR("Drop on the queue; cannot validate congestion avoidance");
}

void TcpLinuxRenoCongAvoidTest::PhyDrop(SocketWho who) {
  NS_FATAL_ERROR("Drop on the phy: cannot validate congestion avoidance");
}

void TcpLinuxRenoCongAvoidTest::Tx(const Ptr<const Packet> p,
                                   const TcpHeader &h, SocketWho who) {
  NS_LOG_FUNCTION(this << p << h << who);
}

void TcpLinuxRenoCongAvoidTest::Rx(const Ptr<const Packet> p,
                                   const TcpHeader &h, SocketWho who) {
  NS_LOG_FUNCTION(this << p << h << who);
}

void TcpLinuxRenoCongAvoidTest::DoTeardown() {
  NS_TEST_ASSERT_MSG_EQ(m_lastCwnd, m_expectedCwnd,
                        "Congestion window did not evolve as expected");
  TcpGeneralTest::DoTeardown();
}

class TcpLinuxRenoTestSuite : public TestSuite {
public:
  TcpLinuxRenoTestSuite() : TestSuite("tcp-linux-reno-test", UNIT) {
    TypeId cong_control_type = TcpLinuxReno::GetTypeId();
    AddTestCase(
        new TcpLinuxRenoSSTest(
            524, 524, 7, 2, 1, 9 * 524, cong_control_type,
            "Slow Start MSS = 524, socket send size = 524, delack = 1 " +
                cong_control_type.GetName()),
        TestCase::QUICK);

    AddTestCase(
        new TcpLinuxRenoSSTest(
            524, 524, 7, 2, 2, 9 * 524, cong_control_type,
            "Slow Start MSS = 524, socket send size = 524, delack = 2 " +
                cong_control_type.GetName()),
        TestCase::QUICK);

    AddTestCase(
        new TcpLinuxRenoSSTest(
            1500, 1500, 7, 2, 1, 9 * 1500, cong_control_type,
            "Slow Start MSS = 1500, socket send size = 524, delack = 1 " +
                cong_control_type.GetName()),
        TestCase::QUICK);

    AddTestCase(
        new TcpLinuxRenoSSTest(
            1500, 1500, 7, 2, 2, 9 * 1500, cong_control_type,
            "Slow Start MSS = 1500, socket send size = 524, delack = 2 " +
                cong_control_type.GetName()),
        TestCase::QUICK);

    AddTestCase(new TcpLinuxRenoCongAvoidTest(
                    524, 524, 6, 1, 2 * 524, 1, 4 * 524, cong_control_type,
                    "Congestion Avoidance MSS = 524, socket "
                    "send size = 524, delack = 1 " +
                        cong_control_type.GetName()),
                TestCase::QUICK);

    AddTestCase(new TcpLinuxRenoCongAvoidTest(
                    524, 524, 6, 1, 2, 2, 4 * 524, cong_control_type,
                    "Congestion Avoidance MSS = 524, socket "
                    "send size = 524, delack = 2 " +
                        cong_control_type.GetName()),
                TestCase::QUICK);

    AddTestCase(new TcpLinuxRenoCongAvoidTest(
                    1500, 1500, 6, 1, 2, 1, 4 * 1500, cong_control_type,
                    "Congestion Avoidance MSS = 1500, socket "
                    "send size = 1500, delack = 1 " +
                        cong_control_type.GetName()),
                TestCase::QUICK);

    AddTestCase(new TcpLinuxRenoCongAvoidTest(
                    1500, 1500, 6, 1, 2, 2, 4 * 1500, cong_control_type,
                    "Congestion Avoidance MSS = 1500, socket "
                    "send size = 1500, delack = 2 " +
                        cong_control_type.GetName()),
                TestCase::QUICK);
  }
};

static TcpLinuxRenoTestSuite g_tcpLinuxRenoTestSuite;
