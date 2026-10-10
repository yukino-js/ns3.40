
#include "ns3/ap-wifi-mac.h"
#include "ns3/boolean.h"
#include "ns3/config.h"
#include "ns3/ctrl-headers.h"
#include "ns3/double.h"
#include "ns3/mac-rx-middle.h"
#include "ns3/mobility-helper.h"
#include "ns3/originator-block-ack-agreement.h"
#include "ns3/packet-socket-client.h"
#include "ns3/packet-socket-helper.h"
#include "ns3/packet-socket-server.h"
#include "ns3/packet.h"
#include "ns3/pointer.h"
#include "ns3/qos-txop.h"
#include "ns3/qos-utils.h"
#include "ns3/recipient-block-ack-agreement.h"
#include "ns3/string.h"
#include "ns3/test.h"
#include "ns3/wifi-mac-header.h"
#include "ns3/wifi-mpdu.h"
#include "ns3/wifi-net-device.h"
#include "ns3/yans-wifi-helper.h"

#include <list>

using namespace ns3;

class PacketBufferingCaseA : public TestCase {
public:
  PacketBufferingCaseA();
  ~PacketBufferingCaseA() override;

private:
  void DoRun() override;
  std::list<uint16_t> m_expectedBuffer;
};

PacketBufferingCaseA::PacketBufferingCaseA()
    : TestCase("Check correct order of buffering when startSequence < endSeq") {
  m_expectedBuffer.push_back(64016);
  m_expectedBuffer.push_back(0);
  m_expectedBuffer.push_back(16);
  m_expectedBuffer.push_back(56000);
  m_expectedBuffer.push_back(63984);
}

PacketBufferingCaseA::~PacketBufferingCaseA() {}

void PacketBufferingCaseA::DoRun() {
  std::list<uint16_t> m_buffer;
  std::list<uint16_t>::iterator i;
  std::list<uint16_t>::iterator j;
  m_buffer.push_back(0);
  m_buffer.push_back(16);
  m_buffer.push_back(56000);

  uint16_t endSeq = 4000;

  uint16_t receivedSeq = 4001 * 16;
  uint32_t mappedSeq =
      QosUtilsMapSeqControlToUniqueInteger(receivedSeq, endSeq);
  for (i = m_buffer.begin(); i != m_buffer.end(); i++) {
    if (QosUtilsMapSeqControlToUniqueInteger((*i), endSeq) >= mappedSeq) {
      break;
    }
  }
  m_buffer.insert(i, receivedSeq);

  receivedSeq = 3999 * 16;
  mappedSeq = QosUtilsMapSeqControlToUniqueInteger(receivedSeq, endSeq);
  for (i = m_buffer.begin(); i != m_buffer.end(); i++) {
    if (QosUtilsMapSeqControlToUniqueInteger((*i), endSeq) >= mappedSeq) {
      break;
    }
  }
  m_buffer.insert(i, receivedSeq);

  for (i = m_buffer.begin(), j = m_expectedBuffer.begin(); i != m_buffer.end();
       i++, j++) {
    NS_TEST_EXPECT_MSG_EQ(*i, *j, "error in buffer order");
  }
}

class PacketBufferingCaseB : public TestCase {
public:
  PacketBufferingCaseB();
  ~PacketBufferingCaseB() override;

private:
  void DoRun() override;
  std::list<uint16_t> m_expectedBuffer;
};

PacketBufferingCaseB::PacketBufferingCaseB()
    : TestCase("Check correct order of buffering when startSequence > endSeq") {
  m_expectedBuffer.push_back(240);
  m_expectedBuffer.push_back(241);
  m_expectedBuffer.push_back(256);
  m_expectedBuffer.push_back(64000);
  m_expectedBuffer.push_back(64800);
  m_expectedBuffer.push_back(16);
}

PacketBufferingCaseB::~PacketBufferingCaseB() {}

void PacketBufferingCaseB::DoRun() {
  std::list<uint16_t> m_buffer;
  std::list<uint16_t>::iterator i;
  std::list<uint16_t>::iterator j;
  m_buffer.push_back(256);
  m_buffer.push_back(64000);
  m_buffer.push_back(16);

  uint16_t endSeq = 10;

  uint16_t receivedSeq = 15 * 16;
  uint32_t mappedSeq =
      QosUtilsMapSeqControlToUniqueInteger(receivedSeq, endSeq);
  for (i = m_buffer.begin(); i != m_buffer.end(); i++) {
    if (QosUtilsMapSeqControlToUniqueInteger((*i), endSeq) >= mappedSeq) {
      break;
    }
  }
  m_buffer.insert(i, receivedSeq);

  receivedSeq = 15 * 16 + 1;
  mappedSeq = QosUtilsMapSeqControlToUniqueInteger(receivedSeq, endSeq);
  for (i = m_buffer.begin(); i != m_buffer.end(); i++) {
    if (QosUtilsMapSeqControlToUniqueInteger((*i), endSeq) >= mappedSeq) {
      break;
    }
  }
  m_buffer.insert(i, receivedSeq);

  receivedSeq = 4050 * 16;
  mappedSeq = QosUtilsMapSeqControlToUniqueInteger(receivedSeq, endSeq);
  for (i = m_buffer.begin(); i != m_buffer.end(); i++) {
    if (QosUtilsMapSeqControlToUniqueInteger((*i), endSeq) >= mappedSeq) {
      break;
    }
  }
  m_buffer.insert(i, receivedSeq);

  for (i = m_buffer.begin(), j = m_expectedBuffer.begin(); i != m_buffer.end();
       i++, j++) {
    NS_TEST_EXPECT_MSG_EQ(*i, *j, "error in buffer order");
  }
}

class OriginatorBlockAckWindowTest : public TestCase {
public:
  OriginatorBlockAckWindowTest();

private:
  void DoRun() override;
};

OriginatorBlockAckWindowTest::OriginatorBlockAckWindowTest()
    : TestCase("Check the correctness of the originator block ack window") {}

void OriginatorBlockAckWindowTest::DoRun() {
  uint16_t winSize = 16;
  uint16_t startingSeq = 4090;

  OriginatorBlockAckAgreement agreement(Mac48Address("00:00:00:00:00:01"), 0);
  agreement.SetBufferSize(winSize);
  agreement.SetStartingSequence(startingSeq);
  agreement.InitTxWindow();

  NS_TEST_EXPECT_MSG_EQ(agreement.m_txWindow.GetWinSize(), winSize,
                        "Incorrect window size");
  NS_TEST_EXPECT_MSG_EQ(agreement.m_txWindow.GetWinStart(), startingSeq,
                        "Incorrect winStart");
  for (uint16_t i = 0; i < winSize; i++) {
    NS_TEST_EXPECT_MSG_EQ(agreement.m_txWindow.At(i), false,
                          "Not all flags are cleared after initialization");
  }

  WifiMacHeader hdr;
  hdr.SetType(WIFI_MAC_QOSDATA);
  Ptr<WifiMpdu> mpdu = Create<WifiMpdu>(Create<Packet>(), hdr);
  uint16_t seqNumber = startingSeq;
  mpdu->GetHeader().SetSequenceNumber(seqNumber);
  agreement.NotifyAckedMpdu(mpdu);

  mpdu->GetHeader().SetSequenceNumber(++seqNumber %= SEQNO_SPACE_SIZE);
  agreement.NotifyAckedMpdu(mpdu);

  mpdu->GetHeader().SetSequenceNumber(++seqNumber %= SEQNO_SPACE_SIZE);
  agreement.NotifyAckedMpdu(mpdu);

  mpdu->GetHeader().SetSequenceNumber(++seqNumber %= SEQNO_SPACE_SIZE);
  agreement.NotifyAckedMpdu(mpdu);

  mpdu->GetHeader().SetSequenceNumber(++seqNumber %= SEQNO_SPACE_SIZE);
  agreement.NotifyAckedMpdu(mpdu);

  startingSeq = (seqNumber + 1) % SEQNO_SPACE_SIZE;
  NS_TEST_EXPECT_MSG_EQ(agreement.GetStartingSequence(), startingSeq,
                        "Incorrect starting sequence after 5 acknowledgments");
  for (uint16_t i = 0; i < winSize; i++) {
    NS_TEST_EXPECT_MSG_EQ(agreement.m_txWindow.At(i), false,
                          "Not all flags are cleared after 5 acknowledgments");
  }

  ++seqNumber %= SEQNO_SPACE_SIZE;
  mpdu->GetHeader().SetSequenceNumber(++seqNumber %= SEQNO_SPACE_SIZE);
  agreement.NotifyAckedMpdu(mpdu);

  mpdu->GetHeader().SetSequenceNumber(++seqNumber %= SEQNO_SPACE_SIZE);
  agreement.NotifyAckedMpdu(mpdu);

  mpdu->GetHeader().SetSequenceNumber(++seqNumber %= SEQNO_SPACE_SIZE);
  agreement.NotifyAckedMpdu(mpdu);

  mpdu->GetHeader().SetSequenceNumber(++seqNumber %= SEQNO_SPACE_SIZE);
  agreement.NotifyAckedMpdu(mpdu);

  NS_TEST_EXPECT_MSG_EQ(
      agreement.GetStartingSequence(), startingSeq,
      "Incorrect starting sequence after 1 unacknowledged MPDU");
  NS_TEST_EXPECT_MSG_EQ(agreement.m_txWindow.At(0), false,
                        "Incorrect flag after 1 unacknowledged MPDU");
  NS_TEST_EXPECT_MSG_EQ(agreement.m_txWindow.At(1), true,
                        "Incorrect flag after 1 unacknowledged MPDU");
  NS_TEST_EXPECT_MSG_EQ(agreement.m_txWindow.At(2), true,
                        "Incorrect flag after 1 unacknowledged MPDU");
  NS_TEST_EXPECT_MSG_EQ(agreement.m_txWindow.At(3), true,
                        "Incorrect flag after 1 unacknowledged MPDU");
  NS_TEST_EXPECT_MSG_EQ(agreement.m_txWindow.At(4), true,
                        "Incorrect flag after 1 unacknowledged MPDU");
  for (uint16_t i = 5; i < winSize; i++) {
    NS_TEST_EXPECT_MSG_EQ(agreement.m_txWindow.At(i), false,
                          "Incorrect flag after 1 unacknowledged MPDU");
  }

  mpdu->GetHeader().SetSequenceNumber(startingSeq);
  agreement.NotifyAckedMpdu(mpdu);

  startingSeq = (seqNumber + 1) % SEQNO_SPACE_SIZE;
  NS_TEST_EXPECT_MSG_EQ(
      agreement.GetStartingSequence(), startingSeq,
      "Incorrect starting sequence after acknowledgment of missing MPDU");
  for (uint16_t i = 0; i < winSize; i++) {
    NS_TEST_EXPECT_MSG_EQ(
        agreement.m_txWindow.At(i), false,
        "Not all flags are cleared after acknowledgment of missing MPDU");
  }

  seqNumber = (seqNumber + 4) % SEQNO_SPACE_SIZE;
  mpdu->GetHeader().SetSequenceNumber(seqNumber);
  agreement.NotifyAckedMpdu(mpdu);

  mpdu->GetHeader().SetSequenceNumber(++seqNumber %= SEQNO_SPACE_SIZE);
  agreement.NotifyAckedMpdu(mpdu);

  mpdu->GetHeader().SetSequenceNumber(++seqNumber %= SEQNO_SPACE_SIZE);
  agreement.NotifyAckedMpdu(mpdu);

  mpdu->GetHeader().SetSequenceNumber(++seqNumber %= SEQNO_SPACE_SIZE);
  agreement.NotifyAckedMpdu(mpdu);

  seqNumber = (seqNumber + 3) % SEQNO_SPACE_SIZE;
  mpdu->GetHeader().SetSequenceNumber(seqNumber);
  agreement.NotifyAckedMpdu(mpdu);

  mpdu->GetHeader().SetSequenceNumber(++seqNumber %= SEQNO_SPACE_SIZE);
  agreement.NotifyAckedMpdu(mpdu);

  mpdu->GetHeader().SetSequenceNumber(++seqNumber %= SEQNO_SPACE_SIZE);
  agreement.NotifyAckedMpdu(mpdu);

  NS_TEST_EXPECT_MSG_EQ(
      agreement.GetStartingSequence(), startingSeq,
      "Incorrect starting sequence after 3 unacknowledged MPDUs");
  NS_TEST_EXPECT_MSG_EQ(agreement.m_txWindow.At(0), false,
                        "Incorrect flag after 3 unacknowledged MPDUs");
  NS_TEST_EXPECT_MSG_EQ(agreement.m_txWindow.At(1), false,
                        "Incorrect flag after 3 unacknowledged MPDUs");
  NS_TEST_EXPECT_MSG_EQ(agreement.m_txWindow.At(2), false,
                        "Incorrect flag after 3 unacknowledged MPDUs");
  NS_TEST_EXPECT_MSG_EQ(agreement.m_txWindow.At(3), true,
                        "Incorrect flag after 3 unacknowledged MPDUs");
  NS_TEST_EXPECT_MSG_EQ(agreement.m_txWindow.At(4), true,
                        "Incorrect flag after 3 unacknowledged MPDUs");
  NS_TEST_EXPECT_MSG_EQ(agreement.m_txWindow.At(5), true,
                        "Incorrect flag after 3 unacknowledged MPDUs");
  NS_TEST_EXPECT_MSG_EQ(agreement.m_txWindow.At(6), true,
                        "Incorrect flag after 3 unacknowledged MPDUs");
  NS_TEST_EXPECT_MSG_EQ(agreement.m_txWindow.At(7), false,
                        "Incorrect flag after 3 unacknowledged MPDUs");
  NS_TEST_EXPECT_MSG_EQ(agreement.m_txWindow.At(8), false,
                        "Incorrect flag after 3 unacknowledged MPDUs");
  NS_TEST_EXPECT_MSG_EQ(agreement.m_txWindow.At(9), true,
                        "Incorrect flag after 3 unacknowledged MPDUs");
  NS_TEST_EXPECT_MSG_EQ(agreement.m_txWindow.At(10), true,
                        "Incorrect flag after 3 unacknowledged MPDUs");
  NS_TEST_EXPECT_MSG_EQ(agreement.m_txWindow.At(11), true,
                        "Incorrect flag after 3 unacknowledged MPDUs");
  for (uint16_t i = 12; i < winSize; i++) {
    NS_TEST_EXPECT_MSG_EQ(agreement.m_txWindow.At(i), false,
                          "Incorrect flag after 3 unacknowledged MPDUs");
  }

  seqNumber = (agreement.m_txWindow.GetWinEnd() + 2) % SEQNO_SPACE_SIZE;
  mpdu->GetHeader().SetSequenceNumber(seqNumber);
  agreement.NotifyTransmittedMpdu(mpdu);

  startingSeq = (startingSeq + 2) % SEQNO_SPACE_SIZE;
  NS_TEST_EXPECT_MSG_EQ(agreement.GetStartingSequence(), startingSeq,
                        "Incorrect starting sequence after transmitting an "
                        "MPDU beyond the current window");
  NS_TEST_EXPECT_MSG_EQ(
      agreement.m_txWindow.At(0), false,
      "Incorrect flag after transmitting an MPDU beyond the current window");
  NS_TEST_EXPECT_MSG_EQ(
      agreement.m_txWindow.At(1), true,
      "Incorrect flag after transmitting an MPDU beyond the current window");
  NS_TEST_EXPECT_MSG_EQ(
      agreement.m_txWindow.At(2), true,
      "Incorrect flag after transmitting an MPDU beyond the current window");
  NS_TEST_EXPECT_MSG_EQ(
      agreement.m_txWindow.At(3), true,
      "Incorrect flag after transmitting an MPDU beyond the current window");
  NS_TEST_EXPECT_MSG_EQ(
      agreement.m_txWindow.At(4), true,
      "Incorrect flag after transmitting an MPDU beyond the current window");
  NS_TEST_EXPECT_MSG_EQ(
      agreement.m_txWindow.At(5), false,
      "Incorrect flag after transmitting an MPDU beyond the current window");
  NS_TEST_EXPECT_MSG_EQ(
      agreement.m_txWindow.At(6), false,
      "Incorrect flag after transmitting an MPDU beyond the current window");
  NS_TEST_EXPECT_MSG_EQ(
      agreement.m_txWindow.At(7), true,
      "Incorrect flag after transmitting an MPDU beyond the current window");
  NS_TEST_EXPECT_MSG_EQ(
      agreement.m_txWindow.At(8), true,
      "Incorrect flag after transmitting an MPDU beyond the current window");
  NS_TEST_EXPECT_MSG_EQ(
      agreement.m_txWindow.At(9), true,
      "Incorrect flag after transmitting an MPDU beyond the current window");
  for (uint16_t i = 10; i < winSize; i++) {
    NS_TEST_EXPECT_MSG_EQ(
        agreement.m_txWindow.At(i), false,
        "Incorrect flag after transmitting an MPDU beyond the current window");
  }

  seqNumber = (agreement.m_txWindow.GetWinEnd() + 1) % SEQNO_SPACE_SIZE;
  mpdu->GetHeader().SetSequenceNumber(seqNumber);
  agreement.NotifyTransmittedMpdu(mpdu);

  startingSeq = (startingSeq + 5) % SEQNO_SPACE_SIZE;
  NS_TEST_EXPECT_MSG_EQ(agreement.GetStartingSequence(), startingSeq,
                        "Incorrect starting sequence after transmitting "
                        "another MPDU beyond the current window");
  NS_TEST_EXPECT_MSG_EQ(agreement.m_txWindow.At(0), false,
                        "Incorrect flag after transmitting another MPDU beyond "
                        "the current window");
  NS_TEST_EXPECT_MSG_EQ(agreement.m_txWindow.At(1), false,
                        "Incorrect flag after transmitting another MPDU beyond "
                        "the current window");
  NS_TEST_EXPECT_MSG_EQ(agreement.m_txWindow.At(2), true,
                        "Incorrect flag after transmitting another MPDU beyond "
                        "the current window");
  NS_TEST_EXPECT_MSG_EQ(agreement.m_txWindow.At(3), true,
                        "Incorrect flag after transmitting another MPDU beyond "
                        "the current window");
  NS_TEST_EXPECT_MSG_EQ(agreement.m_txWindow.At(4), true,
                        "Incorrect flag after transmitting another MPDU beyond "
                        "the current window");
  for (uint16_t i = 5; i < winSize; i++) {
    NS_TEST_EXPECT_MSG_EQ(agreement.m_txWindow.At(i), false,
                          "Incorrect flag after transmitting another MPDU "
                          "beyond the current window");
  }

  seqNumber = (startingSeq + 1) % SEQNO_SPACE_SIZE;
  mpdu->GetHeader().SetSequenceNumber(seqNumber);
  agreement.NotifyDiscardedMpdu(mpdu);

  startingSeq = (startingSeq + 5) % SEQNO_SPACE_SIZE;
  NS_TEST_EXPECT_MSG_EQ(agreement.GetStartingSequence(), startingSeq,
                        "Incorrect starting sequence after discarding an MPDU");
  for (uint16_t i = 0; i < winSize; i++) {
    NS_TEST_EXPECT_MSG_EQ(agreement.m_txWindow.At(i), false,
                          "Incorrect flag after discarding an MPDU");
  }

  for (uint16_t i = 1; i < winSize; i++) {
    mpdu->GetHeader().SetSequenceNumber((startingSeq + i) % SEQNO_SPACE_SIZE);
    agreement.NotifyAckedMpdu(mpdu);
  }

  NS_TEST_EXPECT_MSG_EQ(
      agreement.GetStartingSequence(), startingSeq,
      "Incorrect starting sequence after acknowledging all but the first MPDU");
  NS_TEST_EXPECT_MSG_EQ(
      agreement.m_txWindow.At(0), false,
      "Incorrect flag after acknowledging all but the first MPDU");
  for (uint16_t i = 1; i < winSize; i++) {
    NS_TEST_EXPECT_MSG_EQ(
        agreement.m_txWindow.At(i), true,
        "Incorrect flag after acknowledging all but the first MPDU");
  }

  mpdu->GetHeader().SetSequenceNumber(startingSeq % SEQNO_SPACE_SIZE);
  agreement.NotifyAckedMpdu(mpdu);

  startingSeq = (startingSeq + winSize) % SEQNO_SPACE_SIZE;
  NS_TEST_EXPECT_MSG_EQ(
      agreement.GetStartingSequence(), startingSeq,
      "Incorrect starting sequence after acknowledging the first MPDU");
  for (uint16_t i = 0; i < winSize; i++) {
    NS_TEST_EXPECT_MSG_EQ(agreement.m_txWindow.At(i), false,
                          "Incorrect flag after acknowledging the first MPDU");
  }
}

class CtrlBAckResponseHeaderTest : public TestCase {
public:
  CtrlBAckResponseHeaderTest();

private:
  void DoRun() override;
  CtrlBAckResponseHeader m_blockAckHdr;
};

CtrlBAckResponseHeaderTest::CtrlBAckResponseHeaderTest()
    : TestCase("Check the correctness of block ack compressed bitmap") {}

void CtrlBAckResponseHeaderTest::DoRun() {
  m_blockAckHdr.SetType(BlockAckType::COMPRESSED);

  m_blockAckHdr.SetStartingSequence(179);
  for (uint16_t i = 179; i < 220; i++) {
    m_blockAckHdr.SetReceivedPacket(i);
  }
  for (uint16_t i = 225; i <= 242; i++) {
    m_blockAckHdr.SetReceivedPacket(i);
  }
  NS_TEST_EXPECT_MSG_EQ(m_blockAckHdr.GetBitmap()[0], 0xff,
                        "error in compressed bitmap");
  NS_TEST_EXPECT_MSG_EQ(m_blockAckHdr.GetBitmap()[1], 0xff,
                        "error in compressed bitmap");
  NS_TEST_EXPECT_MSG_EQ(m_blockAckHdr.GetBitmap()[2], 0xff,
                        "error in compressed bitmap");
  NS_TEST_EXPECT_MSG_EQ(m_blockAckHdr.GetBitmap()[3], 0xff,
                        "error in compressed bitmap");
  NS_TEST_EXPECT_MSG_EQ(m_blockAckHdr.GetBitmap()[4], 0xff,
                        "error in compressed bitmap");
  NS_TEST_EXPECT_MSG_EQ(m_blockAckHdr.GetBitmap()[5], 0xc1,
                        "error in compressed bitmap");
  NS_TEST_EXPECT_MSG_EQ(m_blockAckHdr.GetBitmap()[6], 0xff,
                        "error in compressed bitmap");
  NS_TEST_EXPECT_MSG_EQ(m_blockAckHdr.GetBitmap()[7], 0xff,
                        "error in compressed bitmap");
  m_blockAckHdr.SetReceivedPacket(1500);
  NS_TEST_EXPECT_MSG_EQ(m_blockAckHdr.GetBitmap()[0], 0xff,
                        "error in compressed bitmap");
  NS_TEST_EXPECT_MSG_EQ(m_blockAckHdr.GetBitmap()[1], 0xff,
                        "error in compressed bitmap");
  NS_TEST_EXPECT_MSG_EQ(m_blockAckHdr.GetBitmap()[2], 0xff,
                        "error in compressed bitmap");
  NS_TEST_EXPECT_MSG_EQ(m_blockAckHdr.GetBitmap()[3], 0xff,
                        "error in compressed bitmap");
  NS_TEST_EXPECT_MSG_EQ(m_blockAckHdr.GetBitmap()[4], 0xff,
                        "error in compressed bitmap");
  NS_TEST_EXPECT_MSG_EQ(m_blockAckHdr.GetBitmap()[5], 0xc1,
                        "error in compressed bitmap");
  NS_TEST_EXPECT_MSG_EQ(m_blockAckHdr.GetBitmap()[6], 0xff,
                        "error in compressed bitmap");
  NS_TEST_EXPECT_MSG_EQ(m_blockAckHdr.GetBitmap()[7], 0xff,
                        "error in compressed bitmap");
  NS_TEST_EXPECT_MSG_EQ(m_blockAckHdr.IsPacketReceived(220), false,
                        "error in compressed bitmap");
  NS_TEST_EXPECT_MSG_EQ(m_blockAckHdr.IsPacketReceived(225), true,
                        "error in compressed bitmap");
  NS_TEST_EXPECT_MSG_EQ(m_blockAckHdr.IsPacketReceived(1500), false,
                        "error in compressed bitmap");

  m_blockAckHdr.ResetBitmap();

  m_blockAckHdr.SetStartingSequence(4090);
  for (uint16_t i = 4090; i != 10; i = (i + 1) % 4096) {
    m_blockAckHdr.SetReceivedPacket(i);
  }
  for (uint16_t i = 22; i < 25; i++) {
    m_blockAckHdr.SetReceivedPacket(i);
  }
  NS_TEST_EXPECT_MSG_EQ(m_blockAckHdr.GetBitmap()[0], 0xff,
                        "error in compressed bitmap");
  NS_TEST_EXPECT_MSG_EQ(m_blockAckHdr.GetBitmap()[1], 0xff,
                        "error in compressed bitmap");
  NS_TEST_EXPECT_MSG_EQ(m_blockAckHdr.GetBitmap()[2], 0x00,
                        "error in compressed bitmap");
  NS_TEST_EXPECT_MSG_EQ(m_blockAckHdr.GetBitmap()[3], 0x70,
                        "error in compressed bitmap");
  NS_TEST_EXPECT_MSG_EQ(m_blockAckHdr.GetBitmap()[4], 0x00,
                        "error in compressed bitmap");
  NS_TEST_EXPECT_MSG_EQ(m_blockAckHdr.GetBitmap()[5], 0x00,
                        "error in compressed bitmap");
  NS_TEST_EXPECT_MSG_EQ(m_blockAckHdr.GetBitmap()[6], 0x00,
                        "error in compressed bitmap");
  NS_TEST_EXPECT_MSG_EQ(m_blockAckHdr.GetBitmap()[7], 0x00,
                        "error in compressed bitmap");
  m_blockAckHdr.SetReceivedPacket(80);
  NS_TEST_EXPECT_MSG_EQ(m_blockAckHdr.GetBitmap()[0], 0xff,
                        "error in compressed bitmap");
  NS_TEST_EXPECT_MSG_EQ(m_blockAckHdr.GetBitmap()[1], 0xff,
                        "error in compressed bitmap");
  NS_TEST_EXPECT_MSG_EQ(m_blockAckHdr.GetBitmap()[2], 0x00,
                        "error in compressed bitmap");
  NS_TEST_EXPECT_MSG_EQ(m_blockAckHdr.GetBitmap()[3], 0x70,
                        "error in compressed bitmap");
  NS_TEST_EXPECT_MSG_EQ(m_blockAckHdr.GetBitmap()[4], 0x00,
                        "error in compressed bitmap");
  NS_TEST_EXPECT_MSG_EQ(m_blockAckHdr.GetBitmap()[5], 0x00,
                        "error in compressed bitmap");
  NS_TEST_EXPECT_MSG_EQ(m_blockAckHdr.GetBitmap()[6], 0x00,
                        "error in compressed bitmap");
  NS_TEST_EXPECT_MSG_EQ(m_blockAckHdr.GetBitmap()[7], 0x00,
                        "error in compressed bitmap");
  NS_TEST_EXPECT_MSG_EQ(m_blockAckHdr.IsPacketReceived(4090), true,
                        "error in compressed bitmap");
  NS_TEST_EXPECT_MSG_EQ(m_blockAckHdr.IsPacketReceived(4095), true,
                        "error in compressed bitmap");
  NS_TEST_EXPECT_MSG_EQ(m_blockAckHdr.IsPacketReceived(10), false,
                        "error in compressed bitmap");
  NS_TEST_EXPECT_MSG_EQ(m_blockAckHdr.IsPacketReceived(35), false,
                        "error in compressed bitmap");
  NS_TEST_EXPECT_MSG_EQ(m_blockAckHdr.IsPacketReceived(80), false,
                        "error in compressed bitmap");
}

class BlockAckRecipientBufferTest : public TestCase {
public:
  BlockAckRecipientBufferTest(uint16_t ssn);
  ~BlockAckRecipientBufferTest() override;

  void DoRun() override;

  void ForwardUp(Ptr<const WifiMpdu> mpdu, uint8_t linkId);

private:
  uint16_t m_ssn;
  std::list<Ptr<const WifiMpdu>> m_fwup;
};

BlockAckRecipientBufferTest::BlockAckRecipientBufferTest(uint16_t ssn)
    : TestCase(
          "Test case for Block Ack recipient reordering buffer operations"),
      m_ssn(ssn) {}

BlockAckRecipientBufferTest::~BlockAckRecipientBufferTest() {}

void BlockAckRecipientBufferTest::ForwardUp(Ptr<const WifiMpdu> mpdu,
                                            uint8_t linkId) {
  m_fwup.push_back(mpdu);
}

void BlockAckRecipientBufferTest::DoRun() {
  Ptr<MacRxMiddle> rxMiddle = Create<MacRxMiddle>();
  rxMiddle->SetForwardCallback(
      MakeCallback(&BlockAckRecipientBufferTest::ForwardUp, this));

  RecipientBlockAckAgreement agreement(Mac48Address::Allocate(), true, 0, 10, 0,
                                       m_ssn, true);
  agreement.SetMacRxMiddle(rxMiddle);

  WifiMacHeader hdr;
  hdr.SetType(WIFI_MAC_QOSDATA);
  hdr.SetAddr1(Mac48Address::Allocate());
  hdr.SetQosTid(0);

  hdr.SetSequenceNumber(m_ssn);
  agreement.NotifyReceivedMpdu(Create<WifiMpdu>(Create<Packet>(), hdr));

  NS_TEST_ASSERT_MSG_EQ(m_fwup.size(), 1,
                        "MPDU with SN=SSN must have been forwarded up");
  NS_TEST_ASSERT_MSG_EQ(m_fwup.front()->GetHeader().GetSequenceNumber(), m_ssn,
                        "The MPDU forwarded up is not the expected one");

  m_fwup.clear();

  hdr.SetSequenceNumber((m_ssn + 4) % SEQNO_SPACE_SIZE);
  agreement.NotifyReceivedMpdu(Create<WifiMpdu>(Create<Packet>(), hdr));
  hdr.SetSequenceNumber((m_ssn + 2) % SEQNO_SPACE_SIZE);
  agreement.NotifyReceivedMpdu(Create<WifiMpdu>(Create<Packet>(), hdr));
  hdr.SetSequenceNumber((m_ssn + 5) % SEQNO_SPACE_SIZE);
  agreement.NotifyReceivedMpdu(Create<WifiMpdu>(Create<Packet>(), hdr));
  hdr.SetSequenceNumber((m_ssn + 3) % SEQNO_SPACE_SIZE);
  agreement.NotifyReceivedMpdu(Create<WifiMpdu>(Create<Packet>(), hdr));
  hdr.SetSequenceNumber((m_ssn + 10) % SEQNO_SPACE_SIZE);
  agreement.NotifyReceivedMpdu(Create<WifiMpdu>(Create<Packet>(), hdr));
  hdr.SetSequenceNumber((m_ssn + 7) % SEQNO_SPACE_SIZE);
  agreement.NotifyReceivedMpdu(Create<WifiMpdu>(Create<Packet>(), hdr));

  NS_TEST_ASSERT_MSG_EQ(m_fwup.empty(), true,
                        "No MPDU must have been forwarded up");

  hdr.SetSequenceNumber(m_ssn);
  agreement.NotifyReceivedMpdu(Create<WifiMpdu>(Create<Packet>(), hdr));

  NS_TEST_ASSERT_MSG_EQ(m_fwup.empty(), true,
                        "No MPDU must have been forwarded up");

  hdr.SetSequenceNumber((m_ssn + 2) % SEQNO_SPACE_SIZE);
  agreement.NotifyReceivedMpdu(Create<WifiMpdu>(Create<Packet>(10), hdr));

  NS_TEST_ASSERT_MSG_EQ(m_fwup.empty(), true,
                        "No MPDU must have been forwarded up");

  hdr.SetSequenceNumber((m_ssn + 1) % SEQNO_SPACE_SIZE);
  agreement.NotifyReceivedMpdu(Create<WifiMpdu>(Create<Packet>(), hdr));

  NS_TEST_ASSERT_MSG_EQ(m_fwup.size(), 5,
                        "5 MPDUs must have been forwarded up");

  NS_TEST_ASSERT_MSG_EQ(m_fwup.front()->GetHeader().GetSequenceNumber(),
                        (m_ssn + 1) % SEQNO_SPACE_SIZE,
                        "The MPDU forwarded up is not the expected one");
  m_fwup.pop_front();

  NS_TEST_ASSERT_MSG_EQ(m_fwup.front()->GetHeader().GetSequenceNumber(),
                        (m_ssn + 2) % SEQNO_SPACE_SIZE,
                        "The MPDU forwarded up is not the expected one");
  NS_TEST_ASSERT_MSG_EQ(m_fwup.front()->GetPacketSize(), 0,
                        "The MPDU forwarded up is not the expected one");
  m_fwup.pop_front();

  NS_TEST_ASSERT_MSG_EQ(m_fwup.front()->GetHeader().GetSequenceNumber(),
                        (m_ssn + 3) % SEQNO_SPACE_SIZE,
                        "The MPDU forwarded up is not the expected one");
  m_fwup.pop_front();

  NS_TEST_ASSERT_MSG_EQ(m_fwup.front()->GetHeader().GetSequenceNumber(),
                        (m_ssn + 4) % SEQNO_SPACE_SIZE,
                        "The MPDU forwarded up is not the expected one");
  m_fwup.pop_front();

  NS_TEST_ASSERT_MSG_EQ(m_fwup.front()->GetHeader().GetSequenceNumber(),
                        (m_ssn + 5) % SEQNO_SPACE_SIZE,
                        "The MPDU forwarded up is not the expected one");
  m_fwup.pop_front();

  hdr.SetSequenceNumber((m_ssn + 17) % SEQNO_SPACE_SIZE);
  agreement.NotifyReceivedMpdu(Create<WifiMpdu>(Create<Packet>(), hdr));

  NS_TEST_ASSERT_MSG_EQ(m_fwup.size(), 1,
                        "One MPDU must have been forwarded up");

  NS_TEST_ASSERT_MSG_EQ(m_fwup.front()->GetHeader().GetSequenceNumber(),
                        (m_ssn + 7) % SEQNO_SPACE_SIZE,
                        "The MPDU forwarded up is not the expected one");
  m_fwup.pop_front();

  agreement.NotifyReceivedBar((m_ssn + 7) % SEQNO_SPACE_SIZE);

  NS_TEST_ASSERT_MSG_EQ(m_fwup.empty(), true,
                        "No MPDU must have been forwarded up");

  agreement.NotifyReceivedBar((m_ssn + 8) % SEQNO_SPACE_SIZE);

  NS_TEST_ASSERT_MSG_EQ(m_fwup.empty(), true,
                        "No MPDU must have been forwarded up");

  hdr.SetSequenceNumber((m_ssn + 9) % SEQNO_SPACE_SIZE);
  agreement.NotifyReceivedMpdu(Create<WifiMpdu>(Create<Packet>(), hdr));
  hdr.SetSequenceNumber((m_ssn + 11) % SEQNO_SPACE_SIZE);
  agreement.NotifyReceivedMpdu(Create<WifiMpdu>(Create<Packet>(), hdr));

  NS_TEST_ASSERT_MSG_EQ(m_fwup.empty(), true,
                        "No MPDU must have been forwarded up");

  agreement.NotifyReceivedBar((m_ssn + 10) % SEQNO_SPACE_SIZE);

  NS_TEST_ASSERT_MSG_EQ(m_fwup.size(), 3,
                        "3 MPDUs must have been forwarded up");

  NS_TEST_ASSERT_MSG_EQ(m_fwup.front()->GetHeader().GetSequenceNumber(),
                        (m_ssn + 9) % SEQNO_SPACE_SIZE,
                        "The MPDU forwarded up is not the expected one");
  m_fwup.pop_front();

  NS_TEST_ASSERT_MSG_EQ(m_fwup.front()->GetHeader().GetSequenceNumber(),
                        (m_ssn + 10) % SEQNO_SPACE_SIZE,
                        "The MPDU forwarded up is not the expected one");
  m_fwup.pop_front();

  NS_TEST_ASSERT_MSG_EQ(m_fwup.front()->GetHeader().GetSequenceNumber(),
                        (m_ssn + 11) % SEQNO_SPACE_SIZE,
                        "The MPDU forwarded up is not the expected one");
  m_fwup.pop_front();

  Simulator::Run();
  Simulator::Destroy();
}

class MultiStaCtrlBAckResponseHeaderTest : public TestCase {
public:
  MultiStaCtrlBAckResponseHeaderTest();

private:
  void DoRun() override;
};

MultiStaCtrlBAckResponseHeaderTest::MultiStaCtrlBAckResponseHeaderTest()
    : TestCase("Check the correctness of Multi-STA block ack") {}

void MultiStaCtrlBAckResponseHeaderTest::DoRun() {
  BlockAckType baType(BlockAckType::MULTI_STA, {0, 4, 8, 16, 32, 8});

  CtrlBAckResponseHeader blockAck;
  blockAck.SetType(baType);

  uint16_t aid1 = 100;
  bool ackType1 = true;
  uint8_t tid1 = 1;

  blockAck.SetAid11(aid1, 0);
  blockAck.SetAckType(ackType1, 0);
  blockAck.SetTidInfo(tid1, 0);

  uint16_t aid2 = 200;
  bool ackType2 = false;
  uint8_t tid2 = 2;
  uint16_t startSeq2 = 1000;

  blockAck.SetAid11(aid2, 1);
  blockAck.SetAckType(ackType2, 1);
  blockAck.SetTidInfo(tid2, 1);
  blockAck.SetStartingSequence(startSeq2, 1);
  for (uint16_t i = startSeq2; i < startSeq2 + 8; i += 2) {
    blockAck.SetReceivedPacket(i, 1);
  }
  for (uint16_t i = startSeq2 + 9; i < startSeq2 + 16; i += 2) {
    blockAck.SetReceivedPacket(i, 1);
  }
  for (uint16_t i = startSeq2 + 24; i < startSeq2 + 32; i++) {
    blockAck.SetReceivedPacket(i, 1);
  }

  uint16_t aid3 = 300;
  bool ackType3 = false;
  uint8_t tid3 = 3;
  uint16_t startSeq3 = 2000;

  blockAck.SetAid11(aid3, 2);
  blockAck.SetAckType(ackType3, 2);
  blockAck.SetTidInfo(tid3, 2);
  blockAck.SetStartingSequence(startSeq3, 2);
  for (uint16_t i = startSeq3; i < startSeq3 + 8; i += 2) {
    blockAck.SetReceivedPacket(i, 2);
  }
  for (uint16_t i = startSeq3 + 9; i < startSeq3 + 16; i += 2) {
    blockAck.SetReceivedPacket(i, 2);
  }
  for (uint16_t i = startSeq3 + 24; i < startSeq3 + 32; i++) {
    blockAck.SetReceivedPacket(i, 2);
  }
  for (uint16_t i = startSeq3 + 32; i < startSeq3 + 36; i++) {
    blockAck.SetReceivedPacket(i, 2);
  }
  for (uint16_t i = startSeq3 + 44; i < startSeq3 + 48; i++) {
    blockAck.SetReceivedPacket(i, 2);
  }
  for (uint16_t i = startSeq3 + 56; i < startSeq3 + 64; i++) {
    blockAck.SetReceivedPacket(i, 2);
  }

  uint16_t aid4 = 400;
  bool ackType4 = false;
  uint8_t tid4 = 4;
  uint16_t startSeq4 = 3000;

  blockAck.SetAid11(aid4, 3);
  blockAck.SetAckType(ackType4, 3);
  blockAck.SetTidInfo(tid4, 3);
  blockAck.SetStartingSequence(startSeq4, 3);
  for (uint16_t i = startSeq4; i < startSeq4 + 8; i += 2) {
    blockAck.SetReceivedPacket(i, 3);
  }
  for (uint16_t i = startSeq4 + 9; i < startSeq4 + 16; i += 2) {
    blockAck.SetReceivedPacket(i, 3);
  }
  for (uint16_t i = startSeq4 + 24; i < startSeq4 + 32; i++) {
    blockAck.SetReceivedPacket(i, 3);
  }
  for (uint16_t i = startSeq4 + 32; i < startSeq4 + 36; i++) {
    blockAck.SetReceivedPacket(i, 3);
  }
  for (uint16_t i = startSeq4 + 44; i < startSeq4 + 48; i++) {
    blockAck.SetReceivedPacket(i, 3);
  }
  for (uint16_t i = startSeq4 + 56; i < startSeq4 + 64; i++) {
    blockAck.SetReceivedPacket(i, 3);
  }
  for (uint16_t i = startSeq4 + 72; i < startSeq4 + 80; i++) {
    blockAck.SetReceivedPacket(i, 3);
  }
  for (uint16_t i = startSeq4 + 88; i < startSeq4 + 96; i++) {
    blockAck.SetReceivedPacket(i, 3);
  }
  for (uint16_t i = startSeq4 + 104; i < startSeq4 + 112; i++) {
    blockAck.SetReceivedPacket(i, 3);
  }
  for (uint16_t i = startSeq4 + 120; i < startSeq4 + 128; i++) {
    blockAck.SetReceivedPacket(i, 3);
  }

  uint16_t aid5 = 500;
  bool ackType5 = false;
  uint8_t tid5 = 5;
  uint16_t startSeq5 = 4000;

  blockAck.SetAid11(aid5, 4);
  blockAck.SetAckType(ackType5, 4);
  blockAck.SetTidInfo(tid5, 4);
  blockAck.SetStartingSequence(startSeq5, 4);
  for (int i = startSeq5; i < startSeq5 + 8; i += 2) {
    blockAck.SetReceivedPacket(i, 4);
  }
  for (int i = startSeq5 + 9; i < startSeq5 + 16; i += 2) {
    blockAck.SetReceivedPacket(i, 4);
  }
  for (int i = startSeq5 + 24; i < startSeq5 + 32; i++) {
    blockAck.SetReceivedPacket(i, 4);
  }
  for (int i = startSeq5 + 32; i < startSeq5 + 36; i++) {
    blockAck.SetReceivedPacket(i, 4);
  }
  for (int i = startSeq5 + 44; i < startSeq5 + 48; i++) {
    blockAck.SetReceivedPacket(i, 4);
  }
  for (int i = startSeq5 + 56; i < startSeq5 + 64; i++) {
    blockAck.SetReceivedPacket(i, 4);
  }
  for (int i = startSeq5 + 72; i < startSeq5 + 80; i++) {
    blockAck.SetReceivedPacket(i, 4);
  }
  for (int i = startSeq5 + 88; i < startSeq5 + 96; i++) {
    blockAck.SetReceivedPacket(i, 4);
  }
  for (int i = (startSeq5 + 104) % 4096; i < (startSeq5 + 112) % 4096; i++) {
    blockAck.SetReceivedPacket(i, 4);
  }
  for (int i = (startSeq5 + 120) % 4096; i < (startSeq5 + 128) % 4096; i++) {
    blockAck.SetReceivedPacket(i, 4);
  }
  for (int i = (startSeq5 + 136) % 4096; i < (startSeq5 + 144) % 4096; i++) {
    blockAck.SetReceivedPacket(i, 4);
  }
  for (int i = (startSeq5 + 152) % 4096; i < (startSeq5 + 160) % 4096; i++) {
    blockAck.SetReceivedPacket(i, 4);
  }
  for (int i = (startSeq5 + 168) % 4096; i < (startSeq5 + 176) % 4096; i++) {
    blockAck.SetReceivedPacket(i, 4);
  }
  for (int i = (startSeq5 + 184) % 4096; i < (startSeq5 + 192) % 4096; i++) {
    blockAck.SetReceivedPacket(i, 4);
  }
  for (int i = (startSeq5 + 200) % 4096; i < (startSeq5 + 208) % 4096; i++) {
    blockAck.SetReceivedPacket(i, 4);
  }
  for (int i = (startSeq5 + 216) % 4096; i < (startSeq5 + 224) % 4096; i++) {
    blockAck.SetReceivedPacket(i, 4);
  }
  for (int i = (startSeq5 + 232) % 4096; i < (startSeq5 + 240) % 4096; i++) {
    blockAck.SetReceivedPacket(i, 4);
  }
  for (int i = (startSeq5 + 248) % 4096; i < (startSeq5 + 256) % 4096; i++) {
    blockAck.SetReceivedPacket(i, 4);
  }

  uint16_t aid6 = 2045;
  bool ackType6 = true;
  uint8_t tid6 = 6;
  Mac48Address address6("00:00:00:00:00:01");

  blockAck.SetAid11(aid6, 5);
  blockAck.SetAckType(ackType6, 5);
  blockAck.SetTidInfo(tid6, 5);
  blockAck.SetUnassociatedStaAddress(address6, 5);

  Ptr<Packet> packet = Create<Packet>();
  packet->AddHeader(blockAck);

  CtrlBAckResponseHeader blockAckCopy;
  packet->RemoveHeader(blockAckCopy);

  BlockAckType baTypeCopy = blockAckCopy.GetType();

  NS_TEST_EXPECT_MSG_EQ(baTypeCopy.m_variant, BlockAckType::MULTI_STA,
                        "Different block ack variant");
  NS_TEST_EXPECT_MSG_EQ(baTypeCopy.m_bitmapLen.size(), 6,
                        "Different number of bitmaps");
  NS_TEST_EXPECT_MSG_EQ(baTypeCopy.m_bitmapLen[0], 0,
                        "Different length of the first bitmap");
  NS_TEST_EXPECT_MSG_EQ(baTypeCopy.m_bitmapLen[1], 4,
                        "Different length of the second bitmap");
  NS_TEST_EXPECT_MSG_EQ(baTypeCopy.m_bitmapLen[2], 8,
                        "Different length of the third bitmap");
  NS_TEST_EXPECT_MSG_EQ(baTypeCopy.m_bitmapLen[3], 16,
                        "Different length of the fourth bitmap");
  NS_TEST_EXPECT_MSG_EQ(baTypeCopy.m_bitmapLen[4], 32,
                        "Different length of the fifth bitmap");
  NS_TEST_EXPECT_MSG_EQ(baTypeCopy.m_bitmapLen[5], 8,
                        "Different length for the sixth bitmap");

  NS_TEST_EXPECT_MSG_EQ(
      blockAckCopy.GetAid11(0), aid1,
      "Different AID for the first Per AID TID Info subfield");
  NS_TEST_EXPECT_MSG_EQ(
      blockAckCopy.GetAckType(0), ackType1,
      "Different Ack Type for the first Per AID TID Info subfield");
  NS_TEST_EXPECT_MSG_EQ(
      blockAckCopy.GetTidInfo(0), tid1,
      "Different TID for the first Per AID TID Info subfield");

  NS_TEST_EXPECT_MSG_EQ(
      blockAckCopy.GetAid11(1), aid2,
      "Different AID for the second Per AID TID Info subfield");
  NS_TEST_EXPECT_MSG_EQ(
      blockAckCopy.GetAckType(1), ackType2,
      "Different Ack Type for the second Per AID TID Info subfield");
  NS_TEST_EXPECT_MSG_EQ(
      blockAckCopy.GetTidInfo(1), tid2,
      "Different TID for the second Per AID TID Info subfield");
  NS_TEST_EXPECT_MSG_EQ(blockAckCopy.GetStartingSequence(1), startSeq2,
                        "Different starting sequence number for the second Per "
                        "AID TID Info subfield");

  auto &bitmap2 = blockAckCopy.GetBitmap(1);
  NS_TEST_EXPECT_MSG_EQ(
      bitmap2.size(), 4,
      "Different bitmap length for the second Per AID TID Info subfield");
  NS_TEST_EXPECT_MSG_EQ(bitmap2[0], 0x55,
                        "Error in the 1st byte of the bitmap for the second "
                        "Per AID TID Info subfield");
  NS_TEST_EXPECT_MSG_EQ(bitmap2[1], 0xaa,
                        "Error in the 2nd byte of the bitmap for the second "
                        "Per AID TID Info subfield");
  NS_TEST_EXPECT_MSG_EQ(bitmap2[2], 0x00,
                        "Error in the 3rd byte of the bitmap for the second "
                        "Per AID TID Info subfield");
  NS_TEST_EXPECT_MSG_EQ(bitmap2[3], 0xff,
                        "Error in the 4th byte of the bitmap for the second "
                        "Per AID TID Info subfield");

  NS_TEST_EXPECT_MSG_EQ(
      blockAckCopy.GetAid11(2), aid3,
      "Different AID for the third Per AID TID Info subfield");
  NS_TEST_EXPECT_MSG_EQ(
      blockAckCopy.GetAckType(2), ackType3,
      "Different Ack Type for the third Per AID TID Info subfield");
  NS_TEST_EXPECT_MSG_EQ(
      blockAckCopy.GetTidInfo(2), tid3,
      "Different TID for the third Per AID TID Info subfield");
  NS_TEST_EXPECT_MSG_EQ(blockAckCopy.GetStartingSequence(2), startSeq3,
                        "Different starting sequence number for the third Per "
                        "AID TID Info subfield");

  auto &bitmap3 = blockAckCopy.GetBitmap(2);
  NS_TEST_EXPECT_MSG_EQ(
      bitmap3.size(), 8,
      "Different bitmap length for the third Per AID TID Info subfield");
  NS_TEST_EXPECT_MSG_EQ(bitmap3[0], 0x55,
                        "Error in the 1st byte of the bitmap for the third Per "
                        "AID TID Info subfield");
  NS_TEST_EXPECT_MSG_EQ(bitmap3[1], 0xaa,
                        "Error in the 2nd byte of the bitmap for the third Per "
                        "AID TID Info subfield");
  NS_TEST_EXPECT_MSG_EQ(bitmap3[2], 0x00,
                        "Error in the 3rd byte of the bitmap for the third Per "
                        "AID TID Info subfield");
  NS_TEST_EXPECT_MSG_EQ(bitmap3[3], 0xff,
                        "Error in the 4th byte of the bitmap for the third Per "
                        "AID TID Info subfield");
  NS_TEST_EXPECT_MSG_EQ(bitmap3[4], 0x0f,
                        "Error in the 5th byte of the bitmap for the third Per "
                        "AID TID Info subfield");
  NS_TEST_EXPECT_MSG_EQ(bitmap3[5], 0xf0,
                        "Error in the 6th byte of the bitmap for the third Per "
                        "AID TID Info subfield");
  NS_TEST_EXPECT_MSG_EQ(bitmap3[6], 0x00,
                        "Error in the 7th byte of the bitmap for the third Per "
                        "AID TID Info subfield");
  NS_TEST_EXPECT_MSG_EQ(bitmap3[7], 0xff,
                        "Error in the 8th byte of the bitmap for the third Per "
                        "AID TID Info subfield");

  NS_TEST_EXPECT_MSG_EQ(
      blockAckCopy.GetAid11(3), aid4,
      "Different AID for the fourth Per AID TID Info subfield");
  NS_TEST_EXPECT_MSG_EQ(
      blockAckCopy.GetAckType(3), ackType4,
      "Different Ack Type for the fourth Per AID TID Info subfield");
  NS_TEST_EXPECT_MSG_EQ(
      blockAckCopy.GetTidInfo(3), tid4,
      "Different TID for the fourth Per AID TID Info subfield");
  NS_TEST_EXPECT_MSG_EQ(blockAckCopy.GetStartingSequence(3), startSeq4,
                        "Different starting sequence number for the fourth Per "
                        "AID TID Info subfield");

  auto &bitmap4 = blockAckCopy.GetBitmap(3);
  NS_TEST_EXPECT_MSG_EQ(
      bitmap4.size(), 16,
      "Different bitmap length for the fourth Per AID TID Info subfield");
  NS_TEST_EXPECT_MSG_EQ(bitmap4[0], 0x55,
                        "Error in the 1st byte of the bitmap for the fourth "
                        "Per AID TID Info subfield");
  NS_TEST_EXPECT_MSG_EQ(bitmap4[1], 0xaa,
                        "Error in the 2nd byte of the bitmap for the fourth "
                        "Per AID TID Info subfield");
  NS_TEST_EXPECT_MSG_EQ(bitmap4[2], 0x00,
                        "Error in the 3rd byte of the bitmap for the fourth "
                        "Per AID TID Info subfield");
  NS_TEST_EXPECT_MSG_EQ(bitmap4[3], 0xff,
                        "Error in the 4th byte of the bitmap for the fourth "
                        "Per AID TID Info subfield");
  NS_TEST_EXPECT_MSG_EQ(bitmap4[4], 0x0f,
                        "Error in the 5th byte of the bitmap for the fourth "
                        "Per AID TID Info subfield");
  NS_TEST_EXPECT_MSG_EQ(bitmap4[5], 0xf0,
                        "Error in the 6th byte of the bitmap for the fourth "
                        "Per AID TID Info subfield");
  NS_TEST_EXPECT_MSG_EQ(bitmap4[6], 0x00,
                        "Error in the 7th byte of the bitmap for the fourth "
                        "Per AID TID Info subfield");
  NS_TEST_EXPECT_MSG_EQ(bitmap4[7], 0xff,
                        "Error in the 8th byte of the bitmap for the fourth "
                        "Per AID TID Info subfield");
  NS_TEST_EXPECT_MSG_EQ(bitmap4[8], 0x00,
                        "Error in the 9th byte of the bitmap for the fourth "
                        "Per AID TID Info subfield");
  NS_TEST_EXPECT_MSG_EQ(bitmap4[9], 0xff,
                        "Error in the 10th byte of the bitmap for the fourth "
                        "Per AID TID Info subfield");
  NS_TEST_EXPECT_MSG_EQ(bitmap4[10], 0x00,
                        "Error in the 11th byte of the bitmap for the fourth "
                        "Per AID TID Info subfield");
  NS_TEST_EXPECT_MSG_EQ(bitmap4[11], 0xff,
                        "Error in the 12th byte of the bitmap for the fourth "
                        "Per AID TID Info subfield");
  NS_TEST_EXPECT_MSG_EQ(bitmap4[12], 0x00,
                        "Error in the 13th byte of the bitmap for the fourth "
                        "Per AID TID Info subfield");
  NS_TEST_EXPECT_MSG_EQ(bitmap4[13], 0xff,
                        "Error in the 14th byte of the bitmap for the fourth "
                        "Per AID TID Info subfield");
  NS_TEST_EXPECT_MSG_EQ(bitmap4[14], 0x00,
                        "Error in the 15th byte of the bitmap for the fourth "
                        "Per AID TID Info subfield");
  NS_TEST_EXPECT_MSG_EQ(bitmap4[15], 0xff,
                        "Error in the 16th byte of the bitmap for the fourth "
                        "Per AID TID Info subfield");

  NS_TEST_EXPECT_MSG_EQ(
      blockAckCopy.GetAid11(4), aid5,
      "Different AID for the fifth Per AID TID Info subfield");
  NS_TEST_EXPECT_MSG_EQ(
      blockAckCopy.GetAckType(4), ackType5,
      "Different Ack Type for the fifth Per AID TID Info subfield");
  NS_TEST_EXPECT_MSG_EQ(
      blockAckCopy.GetTidInfo(4), tid5,
      "Different TID for the fifth Per AID TID Info subfield");
  NS_TEST_EXPECT_MSG_EQ(blockAckCopy.GetStartingSequence(4), startSeq5,
                        "Different starting sequence number for the fifth Per "
                        "AID TID Info subfield");

  auto &bitmap5 = blockAckCopy.GetBitmap(4);
  NS_TEST_EXPECT_MSG_EQ(
      bitmap5.size(), 32,
      "Different bitmap length for the fifth Per AID TID Info subfield");
  NS_TEST_EXPECT_MSG_EQ(bitmap5[0], 0x55,
                        "Error in the 1st byte of the bitmap for the fifth Per "
                        "AID TID Info subfield");
  NS_TEST_EXPECT_MSG_EQ(bitmap5[1], 0xaa,
                        "Error in the 2nd byte of the bitmap for the fifth Per "
                        "AID TID Info subfield");
  NS_TEST_EXPECT_MSG_EQ(bitmap5[2], 0x00,
                        "Error in the 3rd byte of the bitmap for the fifth Per "
                        "AID TID Info subfield");
  NS_TEST_EXPECT_MSG_EQ(bitmap5[3], 0xff,
                        "Error in the 4th byte of the bitmap for the fifth Per "
                        "AID TID Info subfield");
  NS_TEST_EXPECT_MSG_EQ(bitmap5[4], 0x0f,
                        "Error in the 5th byte of the bitmap for the fifth Per "
                        "AID TID Info subfield");
  NS_TEST_EXPECT_MSG_EQ(bitmap5[5], 0xf0,
                        "Error in the 6th byte of the bitmap for the fifth Per "
                        "AID TID Info subfield");
  NS_TEST_EXPECT_MSG_EQ(bitmap5[6], 0x00,
                        "Error in the 7th byte of the bitmap for the fifth Per "
                        "AID TID Info subfield");
  NS_TEST_EXPECT_MSG_EQ(bitmap5[7], 0xff,
                        "Error in the 8th byte of the bitmap for the fifth Per "
                        "AID TID Info subfield");
  NS_TEST_EXPECT_MSG_EQ(bitmap5[8], 0x00,
                        "Error in the 9th byte of the bitmap for the fifth Per "
                        "AID TID Info subfield");
  NS_TEST_EXPECT_MSG_EQ(bitmap5[9], 0xff,
                        "Error in the 10th byte of the bitmap for the fifth "
                        "Per AID TID Info subfield");
  NS_TEST_EXPECT_MSG_EQ(bitmap5[10], 0x00,
                        "Error in the 11th byte of the bitmap for the fifth "
                        "Per AID TID Info subfield");
  NS_TEST_EXPECT_MSG_EQ(bitmap5[11], 0xff,
                        "Error in the 12th byte of the bitmap for the fifth "
                        "Per AID TID Info subfield");
  NS_TEST_EXPECT_MSG_EQ(bitmap5[12], 0x00,
                        "Error in the 13th byte of the bitmap for the fifth "
                        "Per AID TID Info subfield");
  NS_TEST_EXPECT_MSG_EQ(bitmap5[13], 0xff,
                        "Error in the 14th byte of the bitmap for the fifth "
                        "Per AID TID Info subfield");
  NS_TEST_EXPECT_MSG_EQ(bitmap5[14], 0x00,
                        "Error in the 15th byte of the bitmap for the fifth "
                        "Per AID TID Info subfield");
  NS_TEST_EXPECT_MSG_EQ(bitmap5[15], 0xff,
                        "Error in the 16th byte of the bitmap for the fifth "
                        "Per AID TID Info subfield");
  NS_TEST_EXPECT_MSG_EQ(bitmap5[16], 0x00,
                        "Error in the 17th byte of the bitmap for the fifth "
                        "Per AID TID Info subfield");
  NS_TEST_EXPECT_MSG_EQ(bitmap5[17], 0xff,
                        "Error in the 18th byte of the bitmap for the fifth "
                        "Per AID TID Info subfield");
  NS_TEST_EXPECT_MSG_EQ(bitmap5[18], 0x00,
                        "Error in the 19th byte of the bitmap for the fifth "
                        "Per AID TID Info subfield");
  NS_TEST_EXPECT_MSG_EQ(bitmap5[19], 0xff,
                        "Error in the 20th byte of the bitmap for the fifth "
                        "Per AID TID Info subfield");
  NS_TEST_EXPECT_MSG_EQ(bitmap5[20], 0x00,
                        "Error in the 21th byte of the bitmap for the fifth "
                        "Per AID TID Info subfield");
  NS_TEST_EXPECT_MSG_EQ(bitmap5[21], 0xff,
                        "Error in the 22th byte of the bitmap for the fifth "
                        "Per AID TID Info subfield");
  NS_TEST_EXPECT_MSG_EQ(bitmap5[22], 0x00,
                        "Error in the 23th byte of the bitmap for the fifth "
                        "Per AID TID Info subfield");
  NS_TEST_EXPECT_MSG_EQ(bitmap5[23], 0xff,
                        "Error in the 24th byte of the bitmap for the fifth "
                        "Per AID TID Info subfield");
  NS_TEST_EXPECT_MSG_EQ(bitmap5[24], 0x00,
                        "Error in the 25th byte of the bitmap for the fifth "
                        "Per AID TID Info subfield");
  NS_TEST_EXPECT_MSG_EQ(bitmap5[25], 0xff,
                        "Error in the 26th byte of the bitmap for the fifth "
                        "Per AID TID Info subfield");
  NS_TEST_EXPECT_MSG_EQ(bitmap5[26], 0x00,
                        "Error in the 27th byte of the bitmap for the fifth "
                        "Per AID TID Info subfield");
  NS_TEST_EXPECT_MSG_EQ(bitmap5[27], 0xff,
                        "Error in the 28th byte of the bitmap for the fifth "
                        "Per AID TID Info subfield");
  NS_TEST_EXPECT_MSG_EQ(bitmap5[28], 0x00,
                        "Error in the 29th byte of the bitmap for the fifth "
                        "Per AID TID Info subfield");
  NS_TEST_EXPECT_MSG_EQ(bitmap5[29], 0xff,
                        "Error in the 30th byte of the bitmap for the fifth "
                        "Per AID TID Info subfield");
  NS_TEST_EXPECT_MSG_EQ(bitmap5[30], 0x00,
                        "Error in the 31th byte of the bitmap for the fifth "
                        "Per AID TID Info subfield");
  NS_TEST_EXPECT_MSG_EQ(bitmap5[31], 0xff,
                        "Error in the 32th byte of the bitmap for the fifth "
                        "Per AID TID Info subfield");

  NS_TEST_EXPECT_MSG_EQ(
      blockAckCopy.GetAid11(5), aid6,
      "Different AID for the sixth Per AID TID Info subfield");
  NS_TEST_EXPECT_MSG_EQ(
      blockAckCopy.GetAckType(5), ackType6,
      "Different Ack Type for the sixth Per AID TID Info subfield");
  NS_TEST_EXPECT_MSG_EQ(
      blockAckCopy.GetTidInfo(5), tid6,
      "Different TID for the sixth Per AID TID Info subfield");
  NS_TEST_EXPECT_MSG_EQ(blockAckCopy.GetUnassociatedStaAddress(5), address6,
                        "Different starting sequence number for the sixth Per "
                        "AID TID Info subfield");
}

class BlockAckAggregationDisabledTest : public TestCase {
  struct TxopDurationTracer {
    void Trace(Time startTime, Time duration, uint8_t linkId);
    Time m_max{Seconds(0)};
  };

public:
  BlockAckAggregationDisabledTest(bool txop);
  ~BlockAckAggregationDisabledTest() override;

  void DoRun() override;

private:
  bool m_txop;
  uint32_t m_received;
  uint16_t m_txTotal;
  uint16_t m_txSinceBar;
  uint16_t m_nBar;
  uint16_t m_nBa;

  void L7Receive(std::string context, Ptr<const Packet> p, const Address &adr);
  void Transmit(std::string context, Ptr<const Packet> p, double power);
  void Receive(std::string context, Ptr<const Packet> p,
               RxPowerWattPerChannelBand rxPowersW);
};

void BlockAckAggregationDisabledTest::TxopDurationTracer::Trace(
    Time startTime, Time duration, uint8_t linkId) {
  if (duration > m_max) {
    m_max = duration;
  }
}

BlockAckAggregationDisabledTest::BlockAckAggregationDisabledTest(bool txop)
    : TestCase("Test case for Block Ack Policy with aggregation disabled"),
      m_txop(txop), m_received(0), m_txTotal(0), m_txSinceBar(0), m_nBar(0),
      m_nBa(0) {}

BlockAckAggregationDisabledTest::~BlockAckAggregationDisabledTest() {}

void BlockAckAggregationDisabledTest::L7Receive(std::string context,
                                                Ptr<const Packet> p,
                                                const Address &adr) {
  if (p->GetSize() == 1400) {
    m_received++;
  }
}

void BlockAckAggregationDisabledTest::Transmit(std::string context,
                                               Ptr<const Packet> p,
                                               double power) {
  WifiMacHeader hdr;
  p->PeekHeader(hdr);

  if (m_nBar < 2 && (m_txSinceBar == 9 || m_txTotal == 14)) {
    NS_TEST_ASSERT_MSG_EQ(hdr.IsBlockAckReq(), true,
                          "Didn't get a BlockAckReq when expected");
  } else {
    NS_TEST_ASSERT_MSG_EQ(hdr.IsBlockAckReq(), false,
                          "Got a BlockAckReq when not expected");
  }

  if (hdr.IsQosData()) {
    m_txTotal++;
    if (hdr.IsQosBlockAck()) {
      m_txSinceBar++;
    }

    if (!m_txop) {
      NS_TEST_EXPECT_MSG_EQ((m_txTotal == 1 || hdr.IsQosBlockAck()), true,
                            "Unexpected QoS ack policy");
    } else {
      NS_TEST_EXPECT_MSG_EQ((m_txTotal <= 2 || hdr.IsQosBlockAck()), true,
                            "Unexpected QoS ack policy");
    }
  } else if (hdr.IsBlockAckReq()) {
    m_txSinceBar = 0;
    m_nBar++;
  }
}

void BlockAckAggregationDisabledTest::Receive(
    std::string context, Ptr<const Packet> p,
    RxPowerWattPerChannelBand rxPowersW) {
  WifiMacHeader hdr;
  p->PeekHeader(hdr);

  if (hdr.IsBlockAck()) {
    m_nBa++;
  }
}

void BlockAckAggregationDisabledTest::DoRun() {
  NodeContainer wifiStaNode;
  wifiStaNode.Create(1);

  NodeContainer wifiApNode;
  wifiApNode.Create(1);

  YansWifiChannelHelper channel = YansWifiChannelHelper::Default();
  YansWifiPhyHelper phy;
  phy.SetChannel(channel.Create());

  WifiHelper wifi;
  wifi.SetStandard(WIFI_STANDARD_80211n);
  Config::SetDefault("ns3::WifiDefaultAckManager::BaThreshold",
                     DoubleValue(0.125));
  wifi.SetRemoteStationManager("ns3::IdealWifiManager");

  WifiMacHelper mac;
  Ssid ssid = Ssid("ns-3-ssid");
  mac.SetType("ns3::StaWifiMac", "BE_MaxAmsduSize", UintegerValue(0),
              "BE_MaxAmpduSize", UintegerValue(0), "Ssid", SsidValue(ssid),
              "BE_BlockAckThreshold", UintegerValue(2), "ActiveProbing",
              BooleanValue(false));

  NetDeviceContainer staDevices;
  staDevices = wifi.Install(phy, mac, wifiStaNode);

  mac.SetType("ns3::ApWifiMac", "BE_MaxAmsduSize", UintegerValue(0),
              "BE_MaxAmpduSize", UintegerValue(0), "Ssid", SsidValue(ssid),
              "BeaconGeneration", BooleanValue(true));

  NetDeviceContainer apDevices;
  apDevices = wifi.Install(phy, mac, wifiApNode);

  MobilityHelper mobility;
  Ptr<ListPositionAllocator> positionAlloc =
      CreateObject<ListPositionAllocator>();

  positionAlloc->Add(Vector(0.0, 0.0, 0.0));
  positionAlloc->Add(Vector(1.0, 0.0, 0.0));
  mobility.SetPositionAllocator(positionAlloc);

  mobility.SetMobilityModel("ns3::ConstantPositionMobilityModel");
  mobility.Install(wifiApNode);
  mobility.Install(wifiStaNode);

  Ptr<WifiNetDevice> ap_device = DynamicCast<WifiNetDevice>(apDevices.Get(0));
  Ptr<WifiNetDevice> sta_device = DynamicCast<WifiNetDevice>(staDevices.Get(0));

  sta_device->GetMac()->SetAttribute("BE_MaxAmpduSize", UintegerValue(0));
  TxopDurationTracer txopTracer;

  if (m_txop) {
    PointerValue ptr;
    sta_device->GetMac()->GetAttribute("BE_Txop", ptr);
    ptr.Get<QosTxop>()->TraceConnectWithoutContext(
        "TxopTrace", MakeCallback(&TxopDurationTracer::Trace, &txopTracer));

    ap_device->GetMac()->GetAttribute("BE_Txop", ptr);
    ptr.Get<QosTxop>()->SetTxopLimit(MicroSeconds(4800));
  }

  PacketSocketAddress socket;
  socket.SetSingleDevice(sta_device->GetIfIndex());
  socket.SetPhysicalAddress(ap_device->GetAddress());
  socket.SetProtocol(1);

  PacketSocketHelper packetSocket;
  packetSocket.Install(wifiStaNode);
  packetSocket.Install(wifiApNode);

  Ptr<PacketSocketClient> client1 = CreateObject<PacketSocketClient>();
  client1->SetAttribute("PacketSize", UintegerValue(1400));
  client1->SetAttribute("MaxPackets", UintegerValue(1));
  client1->SetAttribute("Interval", TimeValue(MicroSeconds(0)));
  client1->SetRemote(socket);
  wifiStaNode.Get(0)->AddApplication(client1);
  client1->SetStartTime(Seconds(1));
  client1->SetStopTime(Seconds(3.0));

  Ptr<PacketSocketClient> client2 = CreateObject<PacketSocketClient>();
  client2->SetAttribute("PacketSize", UintegerValue(1400));
  client2->SetAttribute("MaxPackets", UintegerValue(13));
  client2->SetAttribute("Interval", TimeValue(MicroSeconds(0)));
  client2->SetRemote(socket);
  wifiStaNode.Get(0)->AddApplication(client2);
  client2->SetStartTime(Seconds(1.5));
  client2->SetStopTime(Seconds(3.0));

  Ptr<PacketSocketServer> server = CreateObject<PacketSocketServer>();
  server->SetLocal(socket);
  wifiApNode.Get(0)->AddApplication(server);
  server->SetStartTime(Seconds(0.0));
  server->SetStopTime(Seconds(4.0));

  Config::Connect(
      "/NodeList/*/ApplicationList/0/$ns3::PacketSocketServer/Rx",
      MakeCallback(&BlockAckAggregationDisabledTest::L7Receive, this));
  Config::Connect(
      "/NodeList/0/DeviceList/0/Phy/PhyTxBegin",
      MakeCallback(&BlockAckAggregationDisabledTest::Transmit, this));
  Config::Connect(
      "/NodeList/0/DeviceList/0/Phy/PhyRxBegin",
      MakeCallback(&BlockAckAggregationDisabledTest::Receive, this));

  Simulator::Stop(Seconds(5));
  Simulator::Run();

  Simulator::Destroy();

  NS_TEST_EXPECT_MSG_EQ(m_txTotal, 14,
                        "Unexpected number of transmitted packets");
  NS_TEST_EXPECT_MSG_EQ(m_received, 14,
                        "Unexpected number of received packets");
  NS_TEST_EXPECT_MSG_EQ(m_nBar, 2, "Unexpected number of Block Ack Requests");
  NS_TEST_EXPECT_MSG_EQ(m_nBa, 2, "Unexpected number of Block Ack Responses");
  if (m_txop) {
    NS_TEST_EXPECT_MSG_LT(txopTracer.m_max, MicroSeconds(4800),
                          "TXOP duration exceeded!");
    NS_TEST_EXPECT_MSG_GT(txopTracer.m_max, MicroSeconds(3008),
                          "The maximum TXOP duration is too short!");
  }
}

class BlockAckTestSuite : public TestSuite {
public:
  BlockAckTestSuite();
};

BlockAckTestSuite::BlockAckTestSuite() : TestSuite("wifi-block-ack", UNIT) {
  AddTestCase(new PacketBufferingCaseA, TestCase::QUICK);
  AddTestCase(new PacketBufferingCaseB, TestCase::QUICK);
  AddTestCase(new OriginatorBlockAckWindowTest, TestCase::QUICK);
  AddTestCase(new CtrlBAckResponseHeaderTest, TestCase::QUICK);
  AddTestCase(new BlockAckRecipientBufferTest(0), TestCase::QUICK);
  AddTestCase(new BlockAckRecipientBufferTest(4090), TestCase::QUICK);
  AddTestCase(new MultiStaCtrlBAckResponseHeaderTest, TestCase::QUICK);
  AddTestCase(new BlockAckAggregationDisabledTest(false), TestCase::QUICK);
  AddTestCase(new BlockAckAggregationDisabledTest(true), TestCase::QUICK);
}

static BlockAckTestSuite g_blockAckTestSuite;
