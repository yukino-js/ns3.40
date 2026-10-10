#include "tcp-general-test.h"

#include "ns3/config.h"
#include "ns3/log.h"
#include "ns3/simple-channel.h"
#include "ns3/test.h"

using namespace ns3;

NS_LOG_COMPONENT_DEFINE("TcpNewRenoCongAvoidTest");

class TcpNewRenoCongAvoidNormalTest : public TcpGeneralTest {
public:
  TcpNewRenoCongAvoidNormalTest(uint32_t segmentSize, uint32_t packetSize,
                                uint32_t packets, const TypeId &congControl,
                                const std::string &desc);

protected:
  void CWndTrace(uint32_t oldValue, uint32_t newValue) override;
  void QueueDrop(SocketWho who) override;
  void PhyDrop(SocketWho who) override;
  void NormalClose(SocketWho who) override;
  void Check();

  void ConfigureEnvironment() override;
  void ConfigureProperties() override;

private:
  uint32_t m_segmentSize;
  uint32_t m_packetSize;
  uint32_t m_packets;
  uint32_t m_increment;
  EventId m_event;
  bool m_initial;
};

TcpNewRenoCongAvoidNormalTest::TcpNewRenoCongAvoidNormalTest(
    uint32_t segmentSize, uint32_t packetSize, uint32_t packets,
    const TypeId &typeId, const std::string &desc)
    : TcpGeneralTest(desc), m_segmentSize(segmentSize),
      m_packetSize(packetSize), m_packets(packets), m_increment(0),
      m_initial(true) {
  m_congControlTypeId = typeId;
}

void TcpNewRenoCongAvoidNormalTest::ConfigureEnvironment() {
  TcpGeneralTest::ConfigureEnvironment();
  SetAppPktSize(m_packetSize);
  SetAppPktCount(m_packets);
  SetMTU(1500);
}

void TcpNewRenoCongAvoidNormalTest::ConfigureProperties() {
  TcpGeneralTest::ConfigureProperties();
  SetSegmentSize(SENDER, m_segmentSize);
  SetInitialSsThresh(SENDER, 0);
}

void TcpNewRenoCongAvoidNormalTest::CWndTrace(uint32_t oldValue,
                                              uint32_t newValue) {
  if (m_initial) {
    m_initial = false;
    return;
  }

  if (!m_event.IsRunning()) {
    m_event = Simulator::Schedule(Seconds(1.0),
                                  &TcpNewRenoCongAvoidNormalTest::Check, this);
  }

  m_increment += newValue - oldValue;
}

void TcpNewRenoCongAvoidNormalTest::QueueDrop(SocketWho who) {
  NS_FATAL_ERROR("Drop on the queue; cannot validate congestion avoidance");
}

void TcpNewRenoCongAvoidNormalTest::PhyDrop(SocketWho who) {
  NS_FATAL_ERROR("Drop on the phy: cannot validate congestion avoidance");
}

void TcpNewRenoCongAvoidNormalTest::Check() {
  uint32_t segSize = GetSegSize(TcpGeneralTest::SENDER);

  if (m_increment != 0) {
    NS_TEST_ASSERT_MSG_LT_OR_EQ(m_increment, segSize,
                                "Increment exceeded segment size in one RTT");
  }

  m_increment = 0;

  m_event = Simulator::Schedule(Seconds(1.0),
                                &TcpNewRenoCongAvoidNormalTest::Check, this);
}

void TcpNewRenoCongAvoidNormalTest::NormalClose(SocketWho who) {
  if (who == SENDER) {
    m_event.Cancel();
  }
}

class TcpRenoCongAvoidTestSuite : public TestSuite {
public:
  TcpRenoCongAvoidTestSuite() : TestSuite("tcp-cong-avoid-test", UNIT) {
    std::list<TypeId> types = {
        TcpNewReno::GetTypeId(),
    };

    for (const auto &t : types) {
      std::string typeName = t.GetName();

      for (uint32_t i = 10; i <= 50; i += 10) {
        AddTestCase(
            new TcpNewRenoCongAvoidNormalTest(
                500, 500, i, t, "cong avoid MSS=500, pkt_size=500," + typeName),
            TestCase::QUICK);
        AddTestCase(new TcpNewRenoCongAvoidNormalTest(
                        500, 1000, i, t,
                        "cong avoid MSS=500, pkt_size=1000," + typeName),
                    TestCase::QUICK);
      }
    }
  }
};

static TcpRenoCongAvoidTestSuite g_tcpCongAvoidNormalTest;
