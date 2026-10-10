
#include "tcp-error-model.h"
#include "tcp-general-test.h"

#include "ns3/log.h"
#include "ns3/node.h"
#include "ns3/rtt-estimator.h"
#include "ns3/simple-channel.h"

NS_LOG_COMPONENT_DEFINE("TcpRtoTest");

using namespace ns3;

class TcpRtoTest : public TcpGeneralTest {
public:
  TcpRtoTest(const TypeId &congControl, const std::string &msg);

protected:
  Ptr<TcpSocketMsgBase> CreateSenderSocket(Ptr<Node> node) override;
  void AfterRTOExpired(const Ptr<const TcpSocketState> tcb,
                       SocketWho who) override;
  void RcvAck(const Ptr<const TcpSocketState> tcb, const TcpHeader &h,
              SocketWho who) override;
  void ProcessedAck(const Ptr<const TcpSocketState> tcb, const TcpHeader &h,
                    SocketWho who) override;
  void FinalChecks() override;
  void ConfigureProperties() override;
  void ConfigureEnvironment() override;

private:
  bool m_afterRTOExpired;
  bool m_segmentReceived;
};

TcpRtoTest::TcpRtoTest(const TypeId &congControl, const std::string &desc)
    : TcpGeneralTest(desc), m_afterRTOExpired(false), m_segmentReceived(false) {
  m_congControlTypeId = congControl;
}

void TcpRtoTest::ConfigureEnvironment() {
  TcpGeneralTest::ConfigureEnvironment();
  SetAppPktCount(100);
}

void TcpRtoTest::ConfigureProperties() {
  TcpGeneralTest::ConfigureProperties();
  SetInitialSsThresh(SENDER, 0);
}

Ptr<TcpSocketMsgBase> TcpRtoTest::CreateSenderSocket(Ptr<Node> node) {
  Ptr<TcpSocketMsgBase> socket = TcpGeneralTest::CreateSenderSocket(node);
  socket->SetAttribute("MinRto", TimeValue(Seconds(0.5)));

  return socket;
}

void TcpRtoTest::AfterRTOExpired(const Ptr<const TcpSocketState> tcb,
                                 SocketWho who) {
  NS_TEST_ASSERT_MSG_EQ(m_afterRTOExpired, false, "Second RTO expired");
  NS_TEST_ASSERT_MSG_EQ(tcb->m_congState.Get(), TcpSocketState::CA_LOSS,
                        "Ack state machine not in LOSS state after a loss");

  m_afterRTOExpired = true;
}

void TcpRtoTest::RcvAck(const Ptr<const TcpSocketState> tcb, const TcpHeader &h,
                        SocketWho who) {

  if (m_afterRTOExpired && who == SENDER) {
    NS_TEST_ASSERT_MSG_EQ(tcb->m_congState.Get(), TcpSocketState::CA_LOSS,
                          "Ack state machine not in LOSS state after a loss");
  } else {
    NS_TEST_ASSERT_MSG_EQ(
        tcb->m_congState.Get(), TcpSocketState::CA_OPEN,
        "Ack state machine not in OPEN state after recovering "
        "from loss");
  }
}

void TcpRtoTest::ProcessedAck(const Ptr<const TcpSocketState> tcb,
                              const TcpHeader &h, SocketWho who) {

  NS_TEST_ASSERT_MSG_EQ(tcb->m_congState.Get(), TcpSocketState::CA_OPEN,
                        "Ack state machine not in OPEN state after recovering "
                        "from loss");

  if (who == SENDER) {
    m_afterRTOExpired = false;
    m_segmentReceived = true;
  }
}

void TcpRtoTest::FinalChecks() {

  NS_TEST_ASSERT_MSG_EQ(m_segmentReceived, true,
                        "Retransmission has not been done");
}

class TcpSsThreshRtoTest : public TcpGeneralTest {
public:
  TcpSsThreshRtoTest(const TypeId &congControl, uint32_t seqToDrop, Time minRto,
                     const std::string &msg);

protected:
  Ptr<TcpSocketMsgBase> CreateSenderSocket(Ptr<Node> node) override;
  Ptr<ErrorModel> CreateReceiverErrorModel() override;
  void BytesInFlightTrace(uint32_t oldValue, uint32_t newValue) override;
  void SsThreshTrace(uint32_t oldValue, uint32_t newValue) override;
  void BeforeRTOExpired(const Ptr<const TcpSocketState> tcb,
                        SocketWho who) override;
  void AfterRTOExpired(const Ptr<const TcpSocketState> tcb,
                       SocketWho who) override;

  void ConfigureEnvironment() override;

  void PktDropped(const Ipv4Header &ipH, const TcpHeader &tcpH,
                  Ptr<const Packet> p);

private:
  uint32_t m_bytesInFlight;
  uint32_t m_bytesInFlightBeforeRto;
  uint32_t m_ssThreshSocket;
  uint32_t m_seqToDrop;
  Time m_minRtoTime;
};

TcpSsThreshRtoTest::TcpSsThreshRtoTest(const TypeId &congControl,
                                       uint32_t seqToDrop, Time minRto,
                                       const std::string &desc)
    : TcpGeneralTest(desc), m_seqToDrop(seqToDrop), m_minRtoTime(minRto) {
  m_congControlTypeId = congControl;
}

void TcpSsThreshRtoTest::ConfigureEnvironment() {
  TcpGeneralTest::ConfigureEnvironment();
  SetAppPktCount(100);
  SetAppPktInterval(MicroSeconds(100));
  SetPropagationDelay(MilliSeconds(1));
}

Ptr<TcpSocketMsgBase> TcpSsThreshRtoTest::CreateSenderSocket(Ptr<Node> node) {
  Ptr<TcpSocketMsgBase> socket = TcpGeneralTest::CreateSenderSocket(node);
  socket->SetAttribute("MinRto", TimeValue(m_minRtoTime));
  NS_LOG_DEBUG("TcpSsThreshRtoTest create sender socket");

  return socket;
}

Ptr<ErrorModel> TcpSsThreshRtoTest::CreateReceiverErrorModel() {
  NS_LOG_DEBUG("TcpSsThreshRtoTest create errorModel");

  Ptr<TcpSeqErrorModel> errorModel = CreateObject<TcpSeqErrorModel>();

  for (uint32_t i = 0; i < 3; ++i) {
    errorModel->AddSeqToKill(SequenceNumber32(m_seqToDrop));
  }

  errorModel->SetDropCallback(
      MakeCallback(&TcpSsThreshRtoTest::PktDropped, this));

  return errorModel;
}

void TcpSsThreshRtoTest::PktDropped(const Ipv4Header &ipH,
                                    const TcpHeader &tcpH,
                                    Ptr<const Packet> p) {
  NS_LOG_DEBUG("DROPPED! " << tcpH);
}

void TcpSsThreshRtoTest::BytesInFlightTrace(uint32_t oldValue,
                                            uint32_t newValue) {
  NS_LOG_DEBUG("Socket BytesInFlight=" << newValue);
  m_bytesInFlight = newValue;
}

void TcpSsThreshRtoTest::SsThreshTrace(uint32_t oldValue, uint32_t newValue) {
  NS_LOG_DEBUG("Socket ssThresh=" << newValue);
  m_ssThreshSocket = newValue;
}

void TcpSsThreshRtoTest::BeforeRTOExpired(const Ptr<const TcpSocketState> tcb,
                                          SocketWho who) {
  NS_LOG_DEBUG("Before RTO for connection " << who);

  if (who == SENDER) {
    m_bytesInFlightBeforeRto = m_bytesInFlight;
    NS_LOG_DEBUG("BytesInFlight before RTO Expired " << m_bytesInFlight);
  }
}

void TcpSsThreshRtoTest::AfterRTOExpired(const Ptr<const TcpSocketState> tcb,
                                         SocketWho who) {
  NS_LOG_DEBUG("After RTO for " << who);
  Ptr<TcpSocketMsgBase> senderSocket = GetSenderSocket();

  uint32_t ssThresh =
      std::max(m_bytesInFlightBeforeRto / 2, 2 * tcb->m_segmentSize);

  NS_LOG_DEBUG("ssThresh " << ssThresh << " m_ssThreshSocket "
                           << m_ssThreshSocket);

  NS_TEST_ASSERT_MSG_EQ(ssThresh, m_ssThreshSocket,
                        "Slow Start Threshold is incorrect");
}

class TcpTimeRtoTest : public TcpGeneralTest {
public:
  TcpTimeRtoTest(const TypeId &congControl, const std::string &msg);

protected:
  Ptr<TcpSocketMsgBase> CreateSenderSocket(Ptr<Node> node) override;
  Ptr<ErrorModel> CreateReceiverErrorModel() override;
  void ErrorClose(SocketWho who) override;
  void AfterRTOExpired(const Ptr<const TcpSocketState> tcb,
                       SocketWho who) override;
  void Tx(const Ptr<const Packet> p, const TcpHeader &h,
          SocketWho who) override;
  void FinalChecks() override;

  void ConfigureEnvironment() override;

  void PktDropped(const Ipv4Header &ipH, const TcpHeader &tcpH,
                  Ptr<const Packet> p);

private:
  uint32_t m_senderSentSegments;
  Time m_previousRTO;
  bool m_closed;
};

TcpTimeRtoTest::TcpTimeRtoTest(const TypeId &congControl,
                               const std::string &desc)
    : TcpGeneralTest(desc), m_senderSentSegments(0), m_closed(false) {
  m_congControlTypeId = congControl;
}

void TcpTimeRtoTest::ConfigureEnvironment() {
  TcpGeneralTest::ConfigureEnvironment();
  SetAppPktCount(100);
}

Ptr<TcpSocketMsgBase> TcpTimeRtoTest::CreateSenderSocket(Ptr<Node> node) {
  Ptr<TcpSocketMsgBase> s = TcpGeneralTest::CreateSenderSocket(node);
  s->SetAttribute("DataRetries", UintegerValue(6));

  return s;
}

Ptr<ErrorModel> TcpTimeRtoTest::CreateReceiverErrorModel() {
  Ptr<TcpSeqErrorModel> errorModel = CreateObject<TcpSeqErrorModel>();

  for (uint32_t i = 0; i < 7; ++i) {
    errorModel->AddSeqToKill(SequenceNumber32(1));
  }

  errorModel->SetDropCallback(MakeCallback(&TcpTimeRtoTest::PktDropped, this));

  return errorModel;
}

void TcpTimeRtoTest::Tx(const Ptr<const Packet> p, const TcpHeader &h,
                        SocketWho who) {
  NS_LOG_FUNCTION(this << p << h << who);

  if (who == SENDER) {
    ++m_senderSentSegments;
    NS_LOG_INFO("Measured RTO:" << GetRto(SENDER).GetSeconds());

    if (h.GetFlags() & TcpHeader::SYN) {
      NS_TEST_ASSERT_MSG_EQ(m_senderSentSegments, 1,
                            "Number of segments sent is different than 1");

      Time s_rto = GetRto(SENDER);
      NS_TEST_ASSERT_MSG_EQ(s_rto, GetConnTimeout(SENDER),
                            "SYN packet sent without respecting "
                            "ConnTimeout attribute");
    } else {
      NS_LOG_INFO("TX: " << h << m_senderSentSegments);

      NS_TEST_ASSERT_MSG_EQ(h.GetSequenceNumber().GetValue(), 1,
                            "First packet was not correctly sent");

      if (m_senderSentSegments == 2) {

        Ptr<RttEstimator> rttEstimator = GetRttEstimator(SENDER);
        Time clockGranularity = GetClockGranularity(SENDER);
        m_previousRTO = rttEstimator->GetEstimate();

        if (clockGranularity > rttEstimator->GetVariation() * 4) {
          m_previousRTO += clockGranularity;
        } else {
          m_previousRTO += rttEstimator->GetVariation() * 4;
        }

        m_previousRTO = Max(m_previousRTO, GetMinRto(SENDER));

        NS_TEST_ASSERT_MSG_EQ_TOL(GetRto(SENDER), m_previousRTO, Seconds(0.01),
                                  "RTO value differs from calculation");
      } else if (m_senderSentSegments == 3) {

        NS_TEST_ASSERT_MSG_EQ_TOL(GetRto(SENDER), m_previousRTO, Seconds(0.01),
                                  "RTO value has changed unexpectedly");
      }
    }
  } else if (who == RECEIVER) {
  }
}

void TcpTimeRtoTest::ErrorClose(SocketWho who) { m_closed = true; }

void TcpTimeRtoTest::AfterRTOExpired(const Ptr<const TcpSocketState> tcb,
                                     SocketWho who) {
  NS_TEST_ASSERT_MSG_EQ(who, SENDER, "RTO in Receiver. That's unexpected");

  Time actualRto = GetRto(SENDER);

  if (actualRto < Seconds(60)) {
    NS_TEST_ASSERT_MSG_EQ_TOL(actualRto, m_previousRTO + m_previousRTO,
                              Seconds(0.01),
                              "RTO has not doubled after an expiration");
    m_previousRTO += m_previousRTO;
  } else {
    NS_TEST_ASSERT_MSG_EQ(actualRto, Seconds(60),
                          "RTO goes beyond 60 second limit");
  }
}

void TcpTimeRtoTest::PktDropped(const Ipv4Header &ipH, const TcpHeader &tcpH,
                                Ptr<const Packet> p) {
  NS_LOG_INFO("DROPPED! " << tcpH);
}

void TcpTimeRtoTest::FinalChecks() {
  NS_TEST_ASSERT_MSG_EQ(
      m_closed, true,
      "Socket has not been closed after retrying data retransmissions");
}

class TcpRtoTestSuite : public TestSuite {
public:
  TcpRtoTestSuite() : TestSuite("tcp-rto-test", UNIT) {
    std::list<TypeId> types = {
        TcpNewReno::GetTypeId(),
    };

    for (const auto &t : types) {
      AddTestCase(new TcpRtoTest(t, t.GetName() + " RTO retransmit testing"),
                  TestCase::QUICK);

      constexpr uint32_t seqToDrop = 25001;

      AddTestCase(new TcpSsThreshRtoTest(
                      t, seqToDrop, Seconds(0.5),
                      t.GetName() + " RTO ssthresh testing, set to 2*MSL"),
                  TestCase::QUICK);

      AddTestCase(
          new TcpSsThreshRtoTest(
              t, seqToDrop, Seconds(0.005),
              t.GetName() +
                  " RTO ssthresh testing, set to half of BytesInFlight"),
          TestCase::QUICK);

      AddTestCase(new TcpTimeRtoTest(t, t.GetName() + " RTO timing testing"),
                  TestCase::QUICK);
    }
  }
};

static TcpRtoTestSuite g_TcpRtoTestSuite;
