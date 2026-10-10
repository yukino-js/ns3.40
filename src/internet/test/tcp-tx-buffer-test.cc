

#include "ns3/log.h"
#include "ns3/packet.h"
#include "ns3/simulator.h"
#include "ns3/tcp-tx-buffer.h"
#include "ns3/test.h"

#include <limits>

using namespace ns3;

NS_LOG_COMPONENT_DEFINE("TcpTxBufferTestSuite");

class TcpTxBufferTestCase : public TestCase {
public:
  TcpTxBufferTestCase();

private:
  void DoRun() override;
  void DoTeardown() override;

  void TestIsLost();
  void TestNewBlock();
  void TestTransmittedBlock();
  void TestNextSeg();
  void TestMergeItemsWhenGetTransmittedSegment();
  uint32_t GetRWnd() const;
};

TcpTxBufferTestCase::TcpTxBufferTestCase() : TestCase("TcpTxBuffer Test") {}

void TcpTxBufferTestCase::DoRun() {
  Simulator::Schedule(Seconds(0.0), &TcpTxBufferTestCase::TestIsLost, this);
  Simulator::Schedule(Seconds(0.0), &TcpTxBufferTestCase::TestNewBlock, this);

  Simulator::Schedule(Seconds(0.0), &TcpTxBufferTestCase::TestTransmittedBlock,
                      this);
  Simulator::Schedule(Seconds(0.0), &TcpTxBufferTestCase::TestNextSeg, this);

  Simulator::Schedule(
      Seconds(0.0),
      &TcpTxBufferTestCase::TestMergeItemsWhenGetTransmittedSegment, this);

  Simulator::Run();
  Simulator::Destroy();
}

void TcpTxBufferTestCase::TestIsLost() {
  Ptr<TcpTxBuffer> txBuf = CreateObject<TcpTxBuffer>();
  txBuf->SetRWndCallback(MakeCallback(&TcpTxBufferTestCase::GetRWnd, this));
  SequenceNumber32 head(1);
  txBuf->SetHeadSequence(head);
  SequenceNumber32 ret;
  Ptr<TcpOptionSack> sack = CreateObject<TcpOptionSack>();
  txBuf->SetSegmentSize(1000);
  txBuf->SetDupAckThresh(3);

  txBuf->Add(Create<Packet>(10000));

  for (uint8_t i = 0; i < 10; ++i) {
    txBuf->CopyFromSequence(1000, SequenceNumber32((i * 1000) + 1));
  }

  for (uint8_t i = 0; i < 10; ++i) {
    NS_TEST_ASSERT_MSG_EQ(txBuf->IsLost(SequenceNumber32((i * 1000) + 1)),
                          false, "Lost is true, but it's not");
  }

  sack->AddSackBlock(
      TcpOptionSack::SackBlock(SequenceNumber32(1001), SequenceNumber32(2001)));
  txBuf->Update(sack->GetSackList());

  for (uint8_t i = 0; i < 10; ++i) {
    NS_TEST_ASSERT_MSG_EQ(txBuf->IsLost(SequenceNumber32((i * 1000) + 1)),
                          false, "Lost is true, but it's not");
  }

  sack->AddSackBlock(
      TcpOptionSack::SackBlock(SequenceNumber32(2001), SequenceNumber32(3001)));
  txBuf->Update(sack->GetSackList());

  for (uint8_t i = 0; i < 10; ++i) {
    NS_TEST_ASSERT_MSG_EQ(txBuf->IsLost(SequenceNumber32((i * 1000) + 1)),
                          false, "Lost is true, but it's not");
  }

  sack->AddSackBlock(
      TcpOptionSack::SackBlock(SequenceNumber32(3001), SequenceNumber32(4001)));
  txBuf->Update(sack->GetSackList());

  NS_TEST_ASSERT_MSG_EQ(txBuf->IsLost(SequenceNumber32(1)), true,
                        "Lost is true, but it's not");

  for (uint8_t i = 1; i < 10; ++i) {
    NS_TEST_ASSERT_MSG_EQ(txBuf->IsLost(SequenceNumber32((i * 1000) + 1)),
                          false, "Lost is true, but it's not");
  }
}

uint32_t TcpTxBufferTestCase::GetRWnd() const {
  return std::numeric_limits<uint32_t>::max();
}

void TcpTxBufferTestCase::TestNextSeg() {
  Ptr<TcpTxBuffer> txBuf = CreateObject<TcpTxBuffer>();
  ;
  txBuf->SetRWndCallback(MakeCallback(&TcpTxBufferTestCase::GetRWnd, this));
  SequenceNumber32 head(1);
  SequenceNumber32 ret;
  SequenceNumber32 retHigh;
  txBuf->SetSegmentSize(150);
  txBuf->SetDupAckThresh(3);
  uint32_t dupThresh = 3;
  uint32_t segmentSize = 150;
  Ptr<TcpOptionSack> sack = CreateObject<TcpOptionSack>();

  NS_TEST_ASSERT_MSG_EQ(txBuf->NextSeg(&ret, &retHigh, false), false,
                        "NextSeq should not be returned at the beginning");

  txBuf->SetHeadSequence(head);
  NS_TEST_ASSERT_MSG_EQ(txBuf->NextSeg(&ret, &retHigh, false), false,
                        "NextSeq should not be returned with no data");

  txBuf->Add(Create<Packet>(30000));
  NS_TEST_ASSERT_MSG_EQ(txBuf->NextSeg(&ret, &retHigh, false), true,
                        "No NextSeq with data at beginning");
  NS_TEST_ASSERT_MSG_EQ(ret.GetValue(), head.GetValue(),
                        "Different NextSeq than expected at the beginning");

  for (uint32_t i = 0; i < 100; ++i) {
    NS_TEST_ASSERT_MSG_EQ(txBuf->NextSeg(&ret, &retHigh, false), true,
                          "No NextSeq with data while \"transmitting\"");
    NS_TEST_ASSERT_MSG_EQ(
        ret, head + (segmentSize * i),
        "Different NextSeq than expected while \"transmitting\"");
    txBuf->CopyFromSequence(segmentSize, ret);
  }

  SequenceNumber32 lastRet = ret;
  for (uint32_t i = 1; i < dupThresh; ++i) {
    SequenceNumber32 begin = head + (segmentSize * i);
    SequenceNumber32 end = begin + segmentSize;
    sack->AddSackBlock(TcpOptionSack::SackBlock(begin, end));
    txBuf->Update(sack->GetSackList());

    NS_TEST_ASSERT_MSG_EQ(txBuf->NextSeg(&ret, &retHigh, false), true,
                          "No NextSeq with SACK block while \"transmitting\"");
    NS_TEST_ASSERT_MSG_EQ(
        ret, lastRet + segmentSize,
        "Different NextSeq than expected in limited transmit");
    txBuf->CopyFromSequence(segmentSize, ret);
    sack->ClearSackList();
    lastRet = ret;
  }

  sack->AddSackBlock(TcpOptionSack::SackBlock(
      head + (segmentSize * (dupThresh)),
      head + (segmentSize * (dupThresh)) + segmentSize));
  txBuf->Update(sack->GetSackList());
  NS_TEST_ASSERT_MSG_EQ(txBuf->NextSeg(&ret, &retHigh, false), true,
                        "No NextSeq with SACK block for Fast Recovery");
  NS_TEST_ASSERT_MSG_EQ(ret, head,
                        "Different NextSeq than expected for Fast Recovery");
  txBuf->CopyFromSequence(segmentSize, ret);
  sack->ClearSackList();

  for (uint32_t i = 1; i <= 4; ++i) {
    sack->AddSackBlock(TcpOptionSack::SackBlock(
        head + (segmentSize * (dupThresh + i)),
        head + (segmentSize * (dupThresh + i)) + segmentSize));
    txBuf->Update(sack->GetSackList());
    NS_TEST_ASSERT_MSG_EQ(
        txBuf->NextSeg(&ret, &retHigh, false), true,
        "No NextSeq with SACK block after recv dupacks in FR");
    NS_TEST_ASSERT_MSG_EQ(
        ret, lastRet + segmentSize,
        "Different NextSeq than expected after recv dupacks in FR");
    txBuf->CopyFromSequence(segmentSize, ret);
    sack->ClearSackList();
    lastRet = ret;
  }

  head = head + segmentSize;
  txBuf->DiscardUpTo(head);

  NS_TEST_ASSERT_MSG_EQ(
      txBuf->NextSeg(&ret, &retHigh, false), true,
      "No NextSeq with SACK block after receiving partial ACK");
  NS_TEST_ASSERT_MSG_EQ(
      ret, head,
      "Different NextSeq than expected after receiving partial ACK ");
  txBuf->CopyFromSequence(segmentSize, ret);

  sack->AddSackBlock(TcpOptionSack::SackBlock(
      head + (segmentSize * (dupThresh + 6)),
      head + (segmentSize * (dupThresh + 6)) + segmentSize));
  txBuf->Update(sack->GetSackList());
  NS_TEST_ASSERT_MSG_EQ(
      txBuf->NextSeg(&ret, &retHigh, false), true,
      "No NextSeq with SACK block after recv dupacks after partial ack");
  NS_TEST_ASSERT_MSG_EQ(
      ret, lastRet + segmentSize,
      "Different NextSeq than expected after recv dupacks after partial ack");
  txBuf->CopyFromSequence(segmentSize, ret);
  sack->ClearSackList();
  head = lastRet = ret + segmentSize;

  txBuf->DiscardUpTo(head);

  for (uint32_t i = 0; i < 93; ++i) {
    NS_TEST_ASSERT_MSG_EQ(txBuf->NextSeg(&ret, &retHigh, false), true,
                          "No NextSeq with data while \"transmitting\"");
    NS_TEST_ASSERT_MSG_EQ(
        ret, head + (segmentSize * i),
        "Different NextSeq than expected while \"transmitting\"");
    txBuf->CopyFromSequence(segmentSize, ret);
  }

  txBuf->DiscardUpTo(ret + segmentSize);
  NS_TEST_ASSERT_MSG_EQ(txBuf->Size(), 0, "Data inside the buffer");
}

void TcpTxBufferTestCase::TestNewBlock() {
  Ptr<TcpTxBuffer> txBuf = CreateObject<TcpTxBuffer>();
  txBuf->SetRWndCallback(MakeCallback(&TcpTxBufferTestCase::GetRWnd, this));
  txBuf->SetHeadSequence(SequenceNumber32(1));
  txBuf->SetSegmentSize(100);

  Ptr<Packet> p1 = Create<Packet>(100);
  txBuf->Add(p1);

  NS_TEST_ASSERT_MSG_EQ(txBuf->SizeFromSequence(SequenceNumber32(1)), 100,
                        "TxBuf miscalculates size");
  NS_TEST_ASSERT_MSG_EQ(txBuf->BytesInFlight(), 0,
                        "TxBuf miscalculates size of in flight segments");

  Ptr<Packet> ret =
      txBuf->CopyFromSequence(100, SequenceNumber32(1))->GetPacketCopy();
  NS_TEST_ASSERT_MSG_EQ(ret->GetSize(), 100,
                        "Returned packet has different size than requested");
  NS_TEST_ASSERT_MSG_EQ(txBuf->SizeFromSequence(SequenceNumber32(1)), 100,
                        "TxBuf miscalculates size");
  NS_TEST_ASSERT_MSG_EQ(txBuf->BytesInFlight(), 100,
                        "TxBuf miscalculates size of in flight segments");

  txBuf->DiscardUpTo(SequenceNumber32(101));
  NS_TEST_ASSERT_MSG_EQ(txBuf->SizeFromSequence(SequenceNumber32(101)), 0,
                        "TxBuf miscalculates size");
  NS_TEST_ASSERT_MSG_EQ(txBuf->BytesInFlight(), 0,
                        "TxBuf miscalculates size of in flight segments");

  Ptr<Packet> p2 = Create<Packet>(100);
  txBuf->Add(p2);

  ret = txBuf->CopyFromSequence(50, SequenceNumber32(101))->GetPacketCopy();
  NS_TEST_ASSERT_MSG_EQ(ret->GetSize(), 50,
                        "Returned packet has different size than requested");
  NS_TEST_ASSERT_MSG_EQ(txBuf->SizeFromSequence(SequenceNumber32(151)), 50,
                        "TxBuf miscalculates size");
  NS_TEST_ASSERT_MSG_EQ(txBuf->BytesInFlight(), 50,
                        "TxBuf miscalculates size of in flight segments");

  Ptr<Packet> p3 = Create<Packet>(100);
  txBuf->Add(p3);

  ret = txBuf->CopyFromSequence(70, SequenceNumber32(151))->GetPacketCopy();
  NS_TEST_ASSERT_MSG_EQ(ret->GetSize(), 70,
                        "Returned packet has different size than requested");
  NS_TEST_ASSERT_MSG_EQ(txBuf->SizeFromSequence(SequenceNumber32(221)), 80,
                        "TxBuf miscalculates size");
  NS_TEST_ASSERT_MSG_EQ(txBuf->BytesInFlight(), 120,
                        "TxBuf miscalculates size of in flight segments");

  ret = txBuf->CopyFromSequence(3000, SequenceNumber32(221))->GetPacketCopy();
  NS_TEST_ASSERT_MSG_EQ(ret->GetSize(), 80,
                        "Returned packet has different size than requested");
  NS_TEST_ASSERT_MSG_EQ(txBuf->SizeFromSequence(SequenceNumber32(301)), 0,
                        "TxBuf miscalculates size");
  NS_TEST_ASSERT_MSG_EQ(txBuf->BytesInFlight(), 200,
                        "TxBuf miscalculates size of in flight segments");

  txBuf->DiscardUpTo(SequenceNumber32(381));
  NS_TEST_ASSERT_MSG_EQ(txBuf->Size(), 0, "Size is different than expected");
}

void TcpTxBufferTestCase::TestMergeItemsWhenGetTransmittedSegment() {
  TcpTxBuffer txBuf;
  SequenceNumber32 head(1);
  txBuf.SetHeadSequence(head);
  txBuf.SetSegmentSize(2000);

  txBuf.Add(Create<Packet>(2000));
  txBuf.CopyFromSequence(1000, SequenceNumber32(1));
  txBuf.CopyFromSequence(1000, SequenceNumber32(1001));
  txBuf.MarkHeadAsLost();

  txBuf.CopyFromSequence(2000, SequenceNumber32(1));
}

void TcpTxBufferTestCase::TestTransmittedBlock() {}

void TcpTxBufferTestCase::DoTeardown() {}

class TcpTxBufferTestSuite : public TestSuite {
public:
  TcpTxBufferTestSuite() : TestSuite("tcp-tx-buffer", UNIT) {
    AddTestCase(new TcpTxBufferTestCase, TestCase::QUICK);
  }
};

static TcpTxBufferTestSuite g_tcpTxBufferTestSuite;
