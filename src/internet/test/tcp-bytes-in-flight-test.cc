
#include "tcp-error-model.h"
#include "tcp-general-test.h"

#include "ns3/config.h"
#include "ns3/log.h"
#include "ns3/node.h"

using namespace ns3;

NS_LOG_COMPONENT_DEFINE("TcpBytesInFlightTestSuite");

class TcpBytesInFlightTest : public TcpGeneralTest {
public:
  TcpBytesInFlightTest(const std::string &desc, std::vector<uint32_t> &toDrop);

protected:
  Ptr<ErrorModel> CreateReceiverErrorModel() override;
  void Rx(const Ptr<const Packet> p, const TcpHeader &h,
          SocketWho who) override;
  void Tx(const Ptr<const Packet> p, const TcpHeader &h,
          SocketWho who) override;
  void BytesInFlightTrace(uint32_t oldValue, uint32_t newValue) override;

  void PktDropped(const Ipv4Header &ipH, const TcpHeader &tcpH,
                  Ptr<const Packet> p);

  void ConfigureEnvironment() override;

  void BeforeRTOExpired(const Ptr<const TcpSocketState> tcb,
                        SocketWho who) override;

  void RTOExpired(Time oldVal, Time newVal);

  void FinalChecks() override;

private:
  uint32_t m_guessedBytesInFlight;
  uint32_t m_dupAckRecv;
  SequenceNumber32 m_lastAckRecv;
  SequenceNumber32 m_greatestSeqSent;
  std::vector<uint32_t> m_toDrop;
};

TcpBytesInFlightTest::TcpBytesInFlightTest(const std::string &desc,
                                           std::vector<uint32_t> &toDrop)
    : TcpGeneralTest(desc), m_guessedBytesInFlight(0), m_dupAckRecv(0),
      m_lastAckRecv(1), m_greatestSeqSent(0), m_toDrop(toDrop) {}

void TcpBytesInFlightTest::ConfigureEnvironment() {
  TcpGeneralTest::ConfigureEnvironment();
  SetAppPktCount(30);
  SetPropagationDelay(MilliSeconds(50));
  SetTransmitStart(Seconds(2.0));

  Config::SetDefault("ns3::TcpSocketBase::Sack", BooleanValue(false));
}

Ptr<ErrorModel> TcpBytesInFlightTest::CreateReceiverErrorModel() {
  Ptr<TcpSeqErrorModel> m_errorModel = CreateObject<TcpSeqErrorModel>();
  for (auto it = m_toDrop.begin(); it != m_toDrop.end(); ++it) {
    m_errorModel->AddSeqToKill(SequenceNumber32(*it));
  }

  m_errorModel->SetDropCallback(
      MakeCallback(&TcpBytesInFlightTest::PktDropped, this));

  return m_errorModel;
}

void TcpBytesInFlightTest::BeforeRTOExpired(const Ptr<const TcpSocketState> tcb,
                                            SocketWho who) {
  NS_LOG_DEBUG("Before RTO for " << who);
  GetSenderSocket()->TraceConnectWithoutContext(
      "RTO", MakeCallback(&TcpBytesInFlightTest::RTOExpired, this));
}

void TcpBytesInFlightTest::RTOExpired(Time oldVal, Time newVal) {
  NS_LOG_DEBUG("RTO expired at " << newVal.GetSeconds());
  m_guessedBytesInFlight = 0;
}

void TcpBytesInFlightTest::PktDropped(const Ipv4Header &ipH,
                                      const TcpHeader &tcpH,
                                      Ptr<const Packet> p) {
  NS_LOG_DEBUG("Drop seq= " << tcpH.GetSequenceNumber() << " size "
                            << p->GetSize());
}

void TcpBytesInFlightTest::Rx(const Ptr<const Packet> p, const TcpHeader &h,
                              SocketWho who) {
  if (who == RECEIVER) {
  } else if (who == SENDER) {
    if (h.GetAckNumber() > m_lastAckRecv) {
      uint32_t diff = h.GetAckNumber() - m_lastAckRecv;
      NS_LOG_DEBUG("Recv ACK=" << h.GetAckNumber());

      if (m_dupAckRecv > 0) {
        if (h.GetAckNumber() >= m_greatestSeqSent) {
          m_guessedBytesInFlight = 0;
          diff = 0;
          m_dupAckRecv = 0;
        } else {
          m_dupAckRecv -= diff / GetSegSize(SENDER);
          m_guessedBytesInFlight -= GetSegSize(SENDER);
        }
      }

      if ((h.GetFlags() & TcpHeader::FIN) != 0 ||
          m_guessedBytesInFlight + 1 == diff) {
        diff -= 1;
      }
      m_guessedBytesInFlight -= diff;
      m_lastAckRecv = h.GetAckNumber();
      NS_LOG_DEBUG("Update m_guessedBytesInFlight to "
                   << m_guessedBytesInFlight);
    } else if (h.GetAckNumber() == m_lastAckRecv &&
               m_lastAckRecv != SequenceNumber32(1) &&
               (h.GetFlags() & TcpHeader::FIN) == 0) {
      m_guessedBytesInFlight -= GetSegSize(SENDER);
      m_dupAckRecv++;
      if (m_dupAckRecv == 3) {
        NS_LOG_DEBUG("Loss of a segment detected");
        m_guessedBytesInFlight -= GetSegSize(SENDER);
      }
      NS_LOG_DEBUG("Dupack received, Update m_guessedBytesInFlight to "
                   << m_guessedBytesInFlight);
    }
  }
}

void TcpBytesInFlightTest::Tx(const Ptr<const Packet> p, const TcpHeader &h,
                              SocketWho who) {
  if (who == SENDER) {
    static SequenceNumber32 retr(0);
    static uint32_t times = 0;

    if (m_greatestSeqSent <= h.GetSequenceNumber()) {
      m_greatestSeqSent = h.GetSequenceNumber();
      times = 0;
    }

    if (retr == h.GetSequenceNumber()) {
      ++times;
    }

    if (times < 2) {
      m_guessedBytesInFlight += p->GetSize();
    }
    retr = h.GetSequenceNumber();

    NS_LOG_DEBUG("TX size=" << p->GetSize() << " seq=" << h.GetSequenceNumber()
                            << " m_guessedBytesInFlight="
                            << m_guessedBytesInFlight);
  }
}

void TcpBytesInFlightTest::BytesInFlightTrace(uint32_t oldValue,
                                              uint32_t newValue) {
  NS_LOG_DEBUG("Socket BytesInFlight=" << newValue << " mine is="
                                       << m_guessedBytesInFlight);
  NS_TEST_ASSERT_MSG_EQ(
      m_guessedBytesInFlight, newValue,
      "At time " << Simulator::Now().GetSeconds()
                 << "; guessed and measured bytes in flight differs");
}

void TcpBytesInFlightTest::FinalChecks() {
  NS_TEST_ASSERT_MSG_EQ(
      m_guessedBytesInFlight, 0,
      "Still present bytes in flight at the end of the transmission");
}

class TcpBytesInFlightTestSuite : public TestSuite {
public:
  TcpBytesInFlightTestSuite() : TestSuite("tcp-bytes-in-flight-test", UNIT) {
    std::vector<uint32_t> toDrop;
    AddTestCase(
        new TcpBytesInFlightTest("BytesInFlight value, no drop", toDrop),
        TestCase::QUICK);
    toDrop.push_back(4001);
    AddTestCase(
        new TcpBytesInFlightTest("BytesInFlight value, one drop", toDrop),
        TestCase::QUICK);
    toDrop.push_back(4001);
    AddTestCase(new TcpBytesInFlightTest(
                    "BytesInFlight value, two drop of same segment", toDrop),
                TestCase::QUICK);
    toDrop.pop_back();
    toDrop.push_back(4501);
    AddTestCase(
        new TcpBytesInFlightTest(
            "BytesInFlight value, two drop of consecutive segments", toDrop),
        TestCase::QUICK);
  }
};

static TcpBytesInFlightTestSuite g_tcpBytesInFlightTestSuite;
