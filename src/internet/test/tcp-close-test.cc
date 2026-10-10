
#include "tcp-error-model.h"
#include "tcp-general-test.h"

#include "ns3/config.h"
#include "ns3/log.h"
#include "ns3/node.h"

namespace ns3 {

NS_LOG_COMPONENT_DEFINE("TcpCloseTestSuite");

class TcpCloseWithLossTestCase : public TcpGeneralTest {
public:
  TcpCloseWithLossTestCase(bool sackEnabled);

protected:
  Ptr<ErrorModel> CreateReceiverErrorModel() override;
  void ConfigureProperties() override;
  void Tx(const Ptr<const Packet> p, const TcpHeader &h,
          SocketWho who) override;
  void Rx(const Ptr<const Packet> p, const TcpHeader &h,
          SocketWho who) override;
  void FinalChecks() override;

  void NormalClose(SocketWho who) override {
    if (who == SENDER) {
      m_sendClose = true;
    } else {
      m_recvClose = true;
    }
  }

  void PktDropped(const Ipv4Header &ipH, const TcpHeader &tcpH,
                  Ptr<const Packet> pkt);

private:
  Ptr<TcpSeqErrorModel> m_errorModel;
  bool m_sendClose;
  bool m_recvClose;
  bool m_synReceived;
  bool m_sackEnabled;
};

TcpCloseWithLossTestCase::TcpCloseWithLossTestCase(bool sackEnabled)
    : TcpGeneralTest("Testing connection closing with retransmissions") {
  m_sendClose = false;
  m_recvClose = false;
  m_synReceived = false;
  m_sackEnabled = sackEnabled;
}

void TcpCloseWithLossTestCase::ConfigureProperties() {
  TcpGeneralTest::ConfigureProperties();
  SetAppPktSize(1400);
  SetAppPktCount(1);
  SetAppPktInterval(MilliSeconds(0));
  SetInitialCwnd(SENDER, 4);
  SetMTU(1452);
  SetSegmentSize(SENDER, 700);
  GetSenderSocket()->SetAttribute("Timestamp", BooleanValue(false));
  GetReceiverSocket()->SetAttribute("Timestamp", BooleanValue(false));
  GetSenderSocket()->SetAttribute("Sack", BooleanValue(m_sackEnabled));
}

void TcpCloseWithLossTestCase::FinalChecks() {
  NS_TEST_ASSERT_MSG_EQ(m_sendClose, true,
                        "Sender has not closed successfully the connection");
  NS_TEST_ASSERT_MSG_EQ(m_recvClose, true,
                        "Recv has not closed successfully the connection");
}

Ptr<ErrorModel> TcpCloseWithLossTestCase::CreateReceiverErrorModel() {
  m_errorModel = CreateObject<TcpSeqErrorModel>();
  m_errorModel->AddSeqToKill(SequenceNumber32(1));

  m_errorModel->SetDropCallback(
      MakeCallback(&TcpCloseWithLossTestCase::PktDropped, this));

  return m_errorModel;
}

void TcpCloseWithLossTestCase::PktDropped(const Ipv4Header &ipH,
                                          const TcpHeader &tcpH,
                                          Ptr<const Packet> pkt) {
  NS_LOG_INFO("Dropped " << tcpH);
}

void TcpCloseWithLossTestCase::Tx(const Ptr<const Packet> p, const TcpHeader &h,
                                  SocketWho who) {
  if (who == SENDER) {
    NS_LOG_INFO("Sender TX: " << h << " size " << p->GetSize());
  } else {
    NS_LOG_INFO("Receiver TX: " << h << " size " << p->GetSize());
  }
}

void TcpCloseWithLossTestCase::Rx(const Ptr<const Packet> p, const TcpHeader &h,
                                  SocketWho who) {
  if (who == SENDER) {
    NS_LOG_INFO("Sender RX: " << h << " size " << p->GetSize());
  } else {
    NS_LOG_INFO("Receiver RX: " << h << " size " << p->GetSize());
  }
}

class TcpTcpCloseTestSuite : public TestSuite {
public:
  TcpTcpCloseTestSuite() : TestSuite("tcp-close", UNIT) {
    AddTestCase(new TcpCloseWithLossTestCase(true), TestCase::QUICK);
    AddTestCase(new TcpCloseWithLossTestCase(false), TestCase::QUICK);
  }
};

static TcpTcpCloseTestSuite g_tcpTcpCloseTestSuite;

} // namespace ns3
