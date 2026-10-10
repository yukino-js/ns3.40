#include "tcp-error-model.h"
#include "tcp-general-test.h"

#include "ns3/ipv4-end-point.h"
#include "ns3/ipv4-interface-address.h"
#include "ns3/ipv4-route.h"
#include "ns3/ipv4-routing-protocol.h"
#include "ns3/ipv4.h"
#include "ns3/ipv6-end-point.h"
#include "ns3/ipv6-route.h"
#include "ns3/ipv6-routing-protocol.h"
#include "ns3/ipv6.h"
#include "ns3/log.h"
#include "ns3/node.h"
#include "ns3/tcp-l4-protocol.h"
#include "ns3/tcp-rx-buffer.h"
#include "ns3/tcp-tx-buffer.h"

namespace ns3 {

NS_LOG_COMPONENT_DEFINE("TcpEcnTestSuite");

class TcpEcnTest : public TcpGeneralTest {
public:
  TcpEcnTest(uint32_t testcase, const std::string &desc);

protected:
  void CWndTrace(uint32_t oldValue, uint32_t newValue) override;
  void Rx(const Ptr<const Packet> p, const TcpHeader &h,
          SocketWho who) override;
  void Tx(const Ptr<const Packet> p, const TcpHeader &h,
          SocketWho who) override;
  Ptr<TcpSocketMsgBase> CreateSenderSocket(Ptr<Node> node) override;
  void ConfigureProperties() override;

private:
  uint32_t m_cwndChangeCount;
  uint32_t m_senderSent;
  uint32_t m_senderReceived;
  uint32_t m_receiverReceived;
  uint32_t m_testcase;
};

class TcpSocketCongestedRouter : public TcpSocketMsgBase {
public:
  static TypeId GetTypeId();

  uint32_t m_dataPacketSent;
  uint8_t m_testcase;

  TcpSocketCongestedRouter() : TcpSocketMsgBase() { m_dataPacketSent = 0; }

  TcpSocketCongestedRouter(const TcpSocketCongestedRouter &other)
      : TcpSocketMsgBase(other) {}

  void SetTestCase(uint8_t testCase);

protected:
  uint32_t SendDataPacket(SequenceNumber32 seq, uint32_t maxSize,
                          bool withAck) override;
  void ReTxTimeout() override;
  Ptr<TcpSocketBase> Fork() override;
};

NS_OBJECT_ENSURE_REGISTERED(TcpSocketCongestedRouter);

TypeId TcpSocketCongestedRouter::GetTypeId() {
  static TypeId tid = TypeId("ns3::TcpSocketCongestedRouter")
                          .SetParent<TcpSocketMsgBase>()
                          .SetGroupName("Internet")
                          .AddConstructor<TcpSocketCongestedRouter>();
  return tid;
}

void TcpSocketCongestedRouter::ReTxTimeout() { TcpSocketBase::ReTxTimeout(); }

void TcpSocketCongestedRouter::SetTestCase(uint8_t testCase) {
  m_testcase = testCase;
}

uint32_t TcpSocketCongestedRouter::SendDataPacket(SequenceNumber32 seq,
                                                  uint32_t maxSize,
                                                  bool withAck) {
  NS_LOG_FUNCTION(this << seq << maxSize << withAck);
  m_dataPacketSent++;

  bool isRetransmission = false;
  if (seq != m_tcb->m_highTxMark) {
    isRetransmission = true;
  }

  Ptr<Packet> p = m_txBuffer->CopyFromSequence(maxSize, seq)->GetPacketCopy();
  uint32_t sz = p->GetSize();
  uint8_t flags = withAck ? TcpHeader::ACK : 0;
  uint32_t remainingData =
      m_txBuffer->SizeFromSequence(seq + SequenceNumber32(sz));

  if (withAck) {
    m_delAckEvent.Cancel();
    m_delAckCount = 0;
  }

  if (m_tcb->m_ecnState == TcpSocketState::ECN_ECE_RCVD &&
      m_ecnEchoSeq.Get() > m_ecnCWRSeq.Get() && !isRetransmission) {
    NS_LOG_DEBUG(TcpSocketState::EcnStateName[m_tcb->m_ecnState]
                 << " -> ECN_CWR_SENT");
    m_tcb->m_ecnState = TcpSocketState::ECN_CWR_SENT;
    m_ecnCWRSeq = seq;
    flags |= TcpHeader::CWR;
    NS_LOG_INFO("CWR flags set");
  }
  if (GetIpTos()) {
    SocketIpTosTag ipTosTag;
    if (m_testcase == 5 && (m_dataPacketSent == 1 || m_dataPacketSent == 3)) {
      ipTosTag.SetTos(MarkEcnCe(GetIpTos()));
    } else if (m_testcase == 6 &&
               (m_dataPacketSent == 4 || m_dataPacketSent == 5)) {
      ipTosTag.SetTos(MarkEcnCe(GetIpTos()));
    } else {
      if (m_tcb->m_ecnState != TcpSocketState::ECN_DISABLED &&
          (GetIpTos() & 0x3) == 0) {
        ipTosTag.SetTos(MarkEcnEct0(GetIpTos()));
      } else {
        ipTosTag.SetTos(GetIpTos());
      }
    }
    p->AddPacketTag(ipTosTag);
  } else {
    SocketIpTosTag ipTosTag;
    if (m_testcase == 5 && (m_dataPacketSent == 1 || m_dataPacketSent == 3)) {
      ipTosTag.SetTos(MarkEcnCe(GetIpTos()));
    } else if (m_testcase == 6 &&
               (m_dataPacketSent == 4 || m_dataPacketSent == 5)) {
      ipTosTag.SetTos(MarkEcnCe(GetIpTos()));
    } else {
      if (m_tcb->m_ecnState != TcpSocketState::ECN_DISABLED) {
        ipTosTag.SetTos(MarkEcnEct0(GetIpTos()));
      }
    }
    p->AddPacketTag(ipTosTag);
  }

  if (IsManualIpv6Tclass()) {
    SocketIpv6TclassTag ipTclassTag;
    if (m_testcase == 5 && (m_dataPacketSent == 1 || m_dataPacketSent == 3)) {
      ipTclassTag.SetTclass(MarkEcnCe(GetIpv6Tclass()));
    } else if (m_testcase == 6 &&
               (m_dataPacketSent == 4 || m_dataPacketSent == 5)) {
      ipTclassTag.SetTclass(MarkEcnCe(GetIpv6Tclass()));
    } else {
      if (m_tcb->m_ecnState != TcpSocketState::ECN_DISABLED &&
          (GetIpv6Tclass() & 0x3) == 0) {
        ipTclassTag.SetTclass(MarkEcnEct0(GetIpv6Tclass()));
      } else {
        ipTclassTag.SetTclass(GetIpv6Tclass());
      }
    }
    p->AddPacketTag(ipTclassTag);
  } else {
    SocketIpv6TclassTag ipTclassTag;
    if (m_testcase == 5 && (m_dataPacketSent == 1 || m_dataPacketSent == 3)) {
      ipTclassTag.SetTclass(MarkEcnCe(GetIpv6Tclass()));
    } else if (m_testcase == 6 &&
               (m_dataPacketSent == 4 || m_dataPacketSent == 5)) {
      ipTclassTag.SetTclass(MarkEcnCe(GetIpv6Tclass()));
    } else {
      if (m_tcb->m_ecnState != TcpSocketState::ECN_DISABLED) {
        ipTclassTag.SetTclass(MarkEcnEct0(GetIpv6Tclass()));
      }
    }
    p->AddPacketTag(ipTclassTag);
  }

  if (IsManualIpTtl()) {
    SocketIpTtlTag ipTtlTag;
    ipTtlTag.SetTtl(GetIpTtl());
    p->AddPacketTag(ipTtlTag);
  }

  if (IsManualIpv6HopLimit()) {
    SocketIpv6HopLimitTag ipHopLimitTag;
    ipHopLimitTag.SetHopLimit(GetIpv6HopLimit());
    p->AddPacketTag(ipHopLimitTag);
  }

  uint8_t priority = GetPriority();
  if (priority) {
    SocketPriorityTag priorityTag;
    priorityTag.SetPriority(priority);
    p->ReplacePacketTag(priorityTag);
  }

  if (m_closeOnEmpty && (remainingData == 0)) {
    flags |= TcpHeader::FIN;
    if (m_state == ESTABLISHED) {
      NS_LOG_DEBUG("ESTABLISHED -> FIN_WAIT_1");
      m_state = FIN_WAIT_1;
    } else if (m_state == CLOSE_WAIT) {
      NS_LOG_DEBUG("CLOSE_WAIT -> LAST_ACK");
      m_state = LAST_ACK;
    }
  }
  TcpHeader header;
  header.SetFlags(flags);
  header.SetSequenceNumber(seq);
  header.SetAckNumber(m_tcb->m_rxBuffer->NextRxSequence());
  if (m_endPoint) {
    header.SetSourcePort(m_endPoint->GetLocalPort());
    header.SetDestinationPort(m_endPoint->GetPeerPort());
  } else {
    header.SetSourcePort(m_endPoint6->GetLocalPort());
    header.SetDestinationPort(m_endPoint6->GetPeerPort());
  }
  header.SetWindowSize(AdvertisedWindowSize());
  AddOptions(header);

  if (m_retxEvent.IsExpired()) {

    NS_LOG_LOGIC(this << " SendDataPacket Schedule ReTxTimeout at time "
                      << Simulator::Now().GetSeconds() << " to expire at time "
                      << (Simulator::Now() + m_rto.Get()).GetSeconds());
    m_retxEvent = Simulator::Schedule(
        m_rto, &TcpSocketCongestedRouter::ReTxTimeout, this);
  }

  m_txTrace(p, header, this);

  if (m_endPoint) {
    m_tcp->SendPacket(p, header, m_endPoint->GetLocalAddress(),
                      m_endPoint->GetPeerAddress(), m_boundnetdevice);
    NS_LOG_DEBUG("Send segment of size "
                 << sz << " with remaining data " << remainingData
                 << " via TcpL4Protocol to " << m_endPoint->GetPeerAddress()
                 << ". Header " << header);
  } else {
    m_tcp->SendPacket(p, header, m_endPoint6->GetLocalAddress(),
                      m_endPoint6->GetPeerAddress(), m_boundnetdevice);
    NS_LOG_DEBUG("Send segment of size "
                 << sz << " with remaining data " << remainingData
                 << " via TcpL4Protocol to " << m_endPoint6->GetPeerAddress()
                 << ". Header " << header);
  }

  UpdateRttHistory(seq, sz, isRetransmission);

  if (seq + sz > m_tcb->m_highTxMark) {
    Simulator::ScheduleNow(&TcpSocketCongestedRouter::NotifyDataSent, this,
                           (seq + sz - m_tcb->m_highTxMark.Get()));
  }
  m_tcb->m_highTxMark = std::max(seq + sz, m_tcb->m_highTxMark.Get());
  return sz;
}

Ptr<TcpSocketBase> TcpSocketCongestedRouter::Fork() {
  return CopyObject<TcpSocketCongestedRouter>(this);
}

TcpEcnTest::TcpEcnTest(uint32_t testcase, const std::string &desc)
    : TcpGeneralTest(desc), m_cwndChangeCount(0), m_senderSent(0),
      m_senderReceived(0), m_receiverReceived(0), m_testcase(testcase) {}

void TcpEcnTest::ConfigureProperties() {
  TcpGeneralTest::ConfigureProperties();
  if (m_testcase == 2 || m_testcase == 4 || m_testcase == 5 ||
      m_testcase == 6) {
    SetUseEcn(SENDER, TcpSocketState::On);
  }
  if (m_testcase == 3 || m_testcase == 4 || m_testcase == 5 ||
      m_testcase == 6) {
    SetUseEcn(RECEIVER, TcpSocketState::On);
  }
}

void TcpEcnTest::CWndTrace(uint32_t oldValue, uint32_t newValue) {
  if (m_testcase == 6) {
    if (newValue < oldValue) {
      m_cwndChangeCount++;
      NS_TEST_ASSERT_MSG_EQ(
          m_cwndChangeCount, 1,
          "Congestion window should be reduced once per every window");
      NS_TEST_ASSERT_MSG_EQ(
          newValue, 1000, "Congestion window should not drop below 2 segments");
    }
  }
}

void TcpEcnTest::Rx(const Ptr<const Packet> p, const TcpHeader &h,
                    SocketWho who) {
  if (who == RECEIVER) {
    if (m_receiverReceived == 0) {
      NS_TEST_ASSERT_MSG_NE(
          ((h.GetFlags()) & TcpHeader::SYN), 0,
          "SYN should be received as first message at the receiver");
      if (m_testcase == 2 || m_testcase == 4 || m_testcase == 5 ||
          m_testcase == 6) {
        NS_TEST_ASSERT_MSG_NE(((h.GetFlags()) & TcpHeader::ECE) &&
                                  ((h.GetFlags()) & TcpHeader::CWR),
                              0,
                              "The flags ECE + CWR should be set in the TCP "
                              "header of first message received "
                              "at receiver when sender is ECN Capable");
      } else {
        NS_TEST_ASSERT_MSG_EQ(
            ((h.GetFlags()) & TcpHeader::ECE) &&
                ((h.GetFlags()) & TcpHeader::CWR),
            0,
            "The flags ECE + CWR should not be set in the TCP header of first "
            "message "
            "received at receiver when sender is not ECN Capable");
      }
    } else if (m_receiverReceived == 1) {
      NS_TEST_ASSERT_MSG_NE(
          ((h.GetFlags()) & TcpHeader::ACK), 0,
          "ACK should be received as second message at receiver");
    } else if (m_receiverReceived == 3 && m_testcase == 5) {
      NS_TEST_ASSERT_MSG_NE(((h.GetFlags()) & TcpHeader::CWR), 0,
                            "Sender should send CWR on receipt of ECE");
    }
    m_receiverReceived++;
  } else if (who == SENDER) {
    if (m_senderReceived == 0) {
      NS_TEST_ASSERT_MSG_NE(((h.GetFlags()) & TcpHeader::SYN) &&
                                ((h.GetFlags()) & TcpHeader::ACK),
                            0, "SYN+ACK received as first message at sender");
      if (m_testcase == 4 || m_testcase == 5 || m_testcase == 6) {
        NS_TEST_ASSERT_MSG_NE(
            (h.GetFlags() & TcpHeader::ECE), 0,
            "The flag ECE should be set in the TCP header of first message "
            "received at "
            "sender when both receiver and sender are ECN Capable");
      } else {
        NS_TEST_ASSERT_MSG_EQ(
            ((h.GetFlags()) & TcpHeader::ECE), 0,
            "The flag ECE should not be set in the TCP header of first message "
            "received at "
            "sender when  either receiver or sender are not ECN Capable");
      }
    }
    if (m_testcase == 5 && m_receiverReceived > 12) {
      NS_TEST_ASSERT_MSG_EQ(
          ((h.GetFlags()) & TcpHeader::ECE), 0,
          "The flag ECE should not be set in TCP header of the packet sent "
          "by the receiver after sender sends CWR flags to receiver and "
          "receiver receives a packet without CE bit set in IP header");
    }
    m_senderReceived++;
  }
}

void TcpEcnTest::Tx(const Ptr<const Packet> p, const TcpHeader &h,
                    SocketWho who) {
  if (who == SENDER) {
    m_senderSent++;
    if (m_senderSent == 3) {
      SocketIpTosTag ipTosTag;
      bool found = p->PeekPacketTag(ipTosTag);
      uint16_t ipTos = 0;
      if (found) {
        ipTos = static_cast<uint16_t>(ipTosTag.GetTos());
      }
      if (m_testcase == 4 || m_testcase == 6) {
        NS_TEST_ASSERT_MSG_EQ(
            ipTos, 0x2,
            "IP TOS should have ECT set if ECN negotiation between "
            "endpoints is successful");
      } else if (m_testcase == 5) {
        if (m_senderSent == 3 || m_senderSent == 5) {
          NS_TEST_ASSERT_MSG_EQ(ipTos, 0x3,
                                "IP TOS should have CE bit set for 3rd and 5th "
                                "packet sent in test case 5");
        } else {
          NS_TEST_ASSERT_MSG_EQ(
              ipTos, 0x2,
              "IP TOS should have ECT set if ECN negotiation between "
              "endpoints is successful");
        }
      } else {
        NS_TEST_ASSERT_MSG_NE(
            ipTos, 0x2,
            "IP TOS should not have ECT set if ECN negotiation between "
            "endpoints is unsuccessful");
      }
    }
  }
}

Ptr<TcpSocketMsgBase> TcpEcnTest::CreateSenderSocket(Ptr<Node> node) {
  if (m_testcase == 5 || m_testcase == 6) {
    Ptr<TcpSocketCongestedRouter> socket =
        DynamicCast<TcpSocketCongestedRouter>(CreateSocket(
            node, TcpSocketCongestedRouter::GetTypeId(), m_congControlTypeId));
    socket->SetTestCase(m_testcase);
    return socket;
  } else {
    return TcpGeneralTest::CreateSenderSocket(node);
  }
}

class TcpEcnTestSuite : public TestSuite {
public:
  TcpEcnTestSuite() : TestSuite("tcp-ecn-test", UNIT) {
    AddTestCase(new TcpEcnTest(1, "ECN Negotiation Test : ECN incapable sender "
                                  "and ECN incapable receiver"),
                TestCase::QUICK);
    AddTestCase(new TcpEcnTest(2, "ECN Negotiation Test : ECN capable sender "
                                  "and ECN incapable receiver"),
                TestCase::QUICK);
    AddTestCase(new TcpEcnTest(3, "ECN Negotiation Test : ECN incapable sender "
                                  "and ECN capable receiver"),
                TestCase::QUICK);
    AddTestCase(new TcpEcnTest(4, "ECN Negotiation Test : ECN capable sender "
                                  "and ECN capable receiver"),
                TestCase::QUICK);
    AddTestCase(new TcpEcnTest(5, "ECE and CWR Functionality Test: ECN capable "
                                  "sender and ECN capable receiver"),
                TestCase::QUICK);
    AddTestCase(new TcpEcnTest(6, "Congestion Window Reduction Test :ECN "
                                  "capable sender and ECN capable receiver"),
                TestCase::QUICK);
  }
};

static TcpEcnTestSuite g_tcpECNTestSuite;

} // namespace ns3
