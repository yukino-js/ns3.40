#include "tcp-general-test.h"

#include "ns3/config.h"
#include "ns3/log.h"
#include "ns3/simple-channel.h"
#include "ns3/test.h"

using namespace ns3;

NS_LOG_COMPONENT_DEFINE("TcpPacingTestSuite");

class TcpPacingTest : public TcpGeneralTest {
public:
  TcpPacingTest(uint32_t segmentSize, uint32_t packetSize, uint32_t packets,
                uint16_t pacingSsRatio, uint16_t pacingCaRatio,
                uint32_t ssThresh, bool paceInitialWindow,
                uint32_t delAckMaxCount, const TypeId &congControl,
                const std::string &desc);

protected:
  void CWndTrace(uint32_t oldValue, uint32_t newValue) override;
  void RttTrace(Time oldTime, Time newTime) override;
  void BytesInFlightTrace(uint32_t oldValue, uint32_t newValue) override;
  void Tx(const Ptr<const Packet> p, const TcpHeader &h,
          SocketWho who) override;
  void Rx(const Ptr<const Packet> p, const TcpHeader &h,
          SocketWho who) override;
  void QueueDrop(SocketWho who) override;
  void PhyDrop(SocketWho who) override;
  void NormalClose(SocketWho who) override;

  virtual void UpdateExpectedInterval();

  void ConfigureEnvironment() override;
  void ConfigureProperties() override;

private:
  uint32_t m_segmentSize;
  uint32_t m_packetSize;
  uint32_t m_packets;
  EventId m_event;
  bool m_initial;
  uint32_t m_initialCwnd;
  uint32_t m_curCwnd;
  bool m_isFullCwndSent;
  uint32_t m_bytesInFlight;
  Time m_prevTxTime;
  uint16_t m_pacingSsRatio;
  uint16_t m_pacingCaRatio;
  uint32_t m_ssThresh;
  bool m_paceInitialWindow;
  uint32_t m_delAckMaxCount;
  bool m_isConnAboutToEnd;
  Time m_transmissionStartTime;
  Time m_expectedInterval;
  uint32_t m_packetsSent;
  Time m_nextPacketInterval;
  Time m_tracedRtt;
};

TcpPacingTest::TcpPacingTest(uint32_t segmentSize, uint32_t packetSize,
                             uint32_t packets, uint16_t pacingSsRatio,
                             uint16_t pacingCaRatio, uint32_t ssThresh,
                             bool paceInitialWindow, uint32_t delAckMaxCount,
                             const TypeId &typeId, const std::string &desc)
    : TcpGeneralTest(desc), m_segmentSize(segmentSize),
      m_packetSize(packetSize), m_packets(packets), m_initial(true),
      m_initialCwnd(10), m_curCwnd(0), m_isFullCwndSent(true),
      m_bytesInFlight(0), m_prevTxTime(0), m_pacingSsRatio(pacingSsRatio),
      m_pacingCaRatio(pacingCaRatio), m_ssThresh(ssThresh),
      m_paceInitialWindow(paceInitialWindow), m_delAckMaxCount(delAckMaxCount),
      m_isConnAboutToEnd(false), m_transmissionStartTime(Seconds(0)),
      m_expectedInterval(Seconds(0)), m_packetsSent(0),
      m_nextPacketInterval(Seconds(0)), m_tracedRtt(Seconds(0)) {
  m_congControlTypeId = typeId;
}

void TcpPacingTest::ConfigureEnvironment() {
  TcpGeneralTest::ConfigureEnvironment();
  SetAppPktSize(m_packetSize);
  SetAppPktCount(m_packets);
  SetAppPktInterval(NanoSeconds(10));
  SetMTU(1500);
  SetTransmitStart(Seconds(0));
  SetPropagationDelay(MilliSeconds(50));
}

void TcpPacingTest::ConfigureProperties() {
  TcpGeneralTest::ConfigureProperties();
  SetSegmentSize(SENDER, m_segmentSize);
  SetInitialSsThresh(SENDER, m_ssThresh);
  SetInitialCwnd(SENDER, m_initialCwnd);
  SetPacingStatus(SENDER, true);
  SetPaceInitialWindow(SENDER, m_paceInitialWindow);
  SetDelAckMaxCount(RECEIVER, m_delAckMaxCount);
  NS_LOG_DEBUG("segSize: " << m_segmentSize << " ssthresh: " << m_ssThresh
                           << " paceInitialWindow: " << m_paceInitialWindow
                           << " delAckMaxCount " << m_delAckMaxCount);
}

void TcpPacingTest::RttTrace(Time oldTime, Time newTime) {
  NS_LOG_FUNCTION(this << oldTime << newTime);
  m_tracedRtt = newTime;
  UpdateExpectedInterval();
}

void TcpPacingTest::CWndTrace(uint32_t oldValue, uint32_t newValue) {
  NS_LOG_FUNCTION(this << oldValue << newValue);
  m_curCwnd = newValue;
  if (m_initial) {
    m_initial = false;
  }
  UpdateExpectedInterval();
}

void TcpPacingTest::BytesInFlightTrace(uint32_t oldValue, uint32_t newValue) {
  m_bytesInFlight = newValue;
}

void TcpPacingTest::UpdateExpectedInterval() {
  double_t factor;
  Time rtt = 2 * GetPropagationDelay();
  if (m_curCwnd < m_ssThresh / 2) {
    factor = static_cast<double>(m_pacingSsRatio) / 100;
  } else {
    factor = static_cast<double>(m_pacingCaRatio) / 100;
  }

  if (!m_paceInitialWindow && (m_curCwnd == m_initialCwnd * m_segmentSize)) {
    m_expectedInterval = Seconds(0);
  } else {
    m_expectedInterval = Seconds((m_segmentSize * m_tracedRtt.GetSeconds()) /
                                 (factor * m_curCwnd));
  }
}

void TcpPacingTest::Rx(const Ptr<const Packet> p, const TcpHeader &h,
                       SocketWho who) {
  if (who == SENDER) {
    uint8_t flags = h.GetFlags();
    uint8_t hasFin = flags & TcpHeader::FIN;
    uint8_t hasAck = flags & TcpHeader::ACK;
    if (hasFin && hasAck) {
      m_isConnAboutToEnd = true;
      NS_LOG_DEBUG("Sender received a FIN/ACK packet");
    } else {
      m_isConnAboutToEnd = false;
      NS_LOG_DEBUG("Sender received an ACK packet");
    }
  }
}

void TcpPacingTest::Tx(const Ptr<const Packet> p, const TcpHeader &h,
                       SocketWho who) {
  NS_LOG_FUNCTION(this << p << h << who);

  if (who == SENDER) {
    m_packetsSent++;
    bool beyondInitialDataSegment = (m_packetsSent > 3);
    Time actualInterval = Simulator::Now() - m_prevTxTime;
    NS_LOG_DEBUG("TX sent: packetsSent: "
                 << m_packetsSent << " fullCwnd: " << m_isFullCwndSent
                 << " nearEnd: " << m_isConnAboutToEnd
                 << " beyondInitialDataSegment " << beyondInitialDataSegment);
    if (!m_isFullCwndSent && !m_isConnAboutToEnd && beyondInitialDataSegment) {
      Time errorMargin = NanoSeconds(10);
      NS_TEST_ASSERT_MSG_LT_OR_EQ(
          std::abs((actualInterval - m_nextPacketInterval).GetSeconds()),
          errorMargin.GetSeconds(),
          "Packet delivery in slow start didn't match pacing rate");
      NS_LOG_DEBUG(
          "Pacing Check: interval (s): "
          << actualInterval.GetSeconds() << " expected interval (s): "
          << m_nextPacketInterval.GetSeconds() << " difference (s): "
          << std::abs((actualInterval - m_nextPacketInterval).GetSeconds())
          << " errorMargin (s): " << errorMargin.GetSeconds());
    }

    m_prevTxTime = Simulator::Now();
    uint32_t soonBytesInFlight = m_bytesInFlight + m_segmentSize;
    bool canPacketBeSent = ((m_curCwnd - soonBytesInFlight) >= m_segmentSize);
    m_isFullCwndSent = (!canPacketBeSent || m_curCwnd == 0);
    m_nextPacketInterval = m_expectedInterval;
    NS_LOG_DEBUG(
        "Next expected interval (s): " << m_nextPacketInterval.GetSeconds());
  }
}

void TcpPacingTest::QueueDrop(SocketWho who) {
  NS_FATAL_ERROR("Drop on the queue; cannot validate congestion avoidance");
}

void TcpPacingTest::PhyDrop(SocketWho who) {
  NS_FATAL_ERROR("Drop on the phy: cannot validate congestion avoidance");
}

void TcpPacingTest::NormalClose(SocketWho who) {
  if (who == SENDER) {
    m_event.Cancel();
  }
}

class TcpPacingTestSuite : public TestSuite {
public:
  TcpPacingTestSuite() : TestSuite("tcp-pacing-test", UNIT) {
    uint16_t pacingSsRatio = 200;
    uint16_t pacingCaRatio = 120;
    uint32_t segmentSize = 1000;
    uint32_t packetSize = 1000;
    uint32_t numPackets = 40;
    uint32_t delAckMaxCount = 1;
    TypeId tid = TcpNewReno::GetTypeId();
    uint32_t ssThresh = 1e9;
    bool paceInitialWindow = false;
    std::string description;

    description =
        std::string("Pacing case 1: Slow start only, no initial pacing");
    AddTestCase(new TcpPacingTest(segmentSize, packetSize, numPackets,
                                  pacingSsRatio, pacingCaRatio, ssThresh,
                                  paceInitialWindow, delAckMaxCount, tid,
                                  description),
                TestCase::QUICK);

    paceInitialWindow = true;
    description = std::string("Pacing case 2: Slow start only, initial pacing");
    AddTestCase(new TcpPacingTest(segmentSize, packetSize, numPackets,
                                  pacingSsRatio, pacingCaRatio, ssThresh,
                                  paceInitialWindow, delAckMaxCount, tid,
                                  description),
                TestCase::QUICK);

    description = std::string(
        "Pacing case 3: Slow start, followed by transition to Congestion "
        "avoidance, no initial pacing");
    paceInitialWindow = false;
    ssThresh = 40;
    numPackets = 60;
    AddTestCase(new TcpPacingTest(segmentSize, packetSize, numPackets,
                                  pacingSsRatio, pacingCaRatio, ssThresh,
                                  paceInitialWindow, delAckMaxCount, tid,
                                  description),
                TestCase::QUICK);

    delAckMaxCount = 2;
    paceInitialWindow = false;
    ssThresh = 1e9;
    numPackets = 40;
    description = std::string(
        "Pacing case 4: Slow start only, no initial pacing, delayed ACKs");
    AddTestCase(new TcpPacingTest(segmentSize, packetSize, numPackets,
                                  pacingSsRatio, pacingCaRatio, ssThresh,
                                  paceInitialWindow, delAckMaxCount, tid,
                                  description),
                TestCase::QUICK);

    paceInitialWindow = true;
    description = std::string(
        "Pacing case 5: Slow start only, initial pacing, delayed ACKs");
    AddTestCase(new TcpPacingTest(segmentSize, packetSize, numPackets,
                                  pacingSsRatio, pacingCaRatio, ssThresh,
                                  paceInitialWindow, delAckMaxCount, tid,
                                  description),
                TestCase::QUICK);

    description = std::string(
        "Pacing case 6: Slow start, followed by transition to Congestion "
        "avoidance, no initial pacing, delayed ACKs");
    paceInitialWindow = false;
    ssThresh = 40;
    numPackets = 60;
    AddTestCase(new TcpPacingTest(segmentSize, packetSize, numPackets,
                                  pacingSsRatio, pacingCaRatio, ssThresh,
                                  paceInitialWindow, delAckMaxCount, tid,
                                  description),
                TestCase::QUICK);
  }
};

static TcpPacingTestSuite g_tcpPacingTest;
