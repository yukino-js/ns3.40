

#include "ns3/log.h"
#include "ns3/packet.h"
#include "ns3/tcp-rx-buffer.h"
#include "ns3/test.h"

using namespace ns3;

NS_LOG_COMPONENT_DEFINE("TcpRxBufferTestSuite");

class TcpRxBufferTestCase : public TestCase {
public:
  TcpRxBufferTestCase();

private:
  void DoRun() override;
  void DoTeardown() override;

  void TestUpdateSACKList();
};

TcpRxBufferTestCase::TcpRxBufferTestCase() : TestCase("TcpRxBuffer Test") {}

void TcpRxBufferTestCase::DoRun() { TestUpdateSACKList(); }

void TcpRxBufferTestCase::TestUpdateSACKList() {
  TcpRxBuffer rxBuf;
  TcpOptionSack::SackList sackList;
  Ptr<Packet> p = Create<Packet>(100);
  TcpHeader h;

  h.SetSequenceNumber(SequenceNumber32(1));
  rxBuf.SetNextRxSequence(SequenceNumber32(1));
  rxBuf.Add(p, h);

  NS_TEST_ASSERT_MSG_EQ(rxBuf.NextRxSequence(), SequenceNumber32(101),
                        "Sequence number differs from expected");
  NS_TEST_ASSERT_MSG_EQ(sackList.size(), 0,
                        "SACK list with an element, while should be empty");

  h.SetSequenceNumber(SequenceNumber32(501));
  rxBuf.Add(p, h);

  NS_TEST_ASSERT_MSG_EQ(rxBuf.NextRxSequence(), SequenceNumber32(101),
                        "Sequence number differs from expected");
  sackList = rxBuf.GetSackList();
  NS_TEST_ASSERT_MSG_EQ(sackList.size(), 1,
                        "SACK list should contain one element");
  auto it = sackList.begin();
  NS_TEST_ASSERT_MSG_EQ(it->first, SequenceNumber32(501),
                        "SACK block different than expected");
  NS_TEST_ASSERT_MSG_EQ(it->second, SequenceNumber32(601),
                        "SACK block different than expected");

  h.SetSequenceNumber(SequenceNumber32(101));
  rxBuf.Add(p, h);

  NS_TEST_ASSERT_MSG_EQ(rxBuf.NextRxSequence(), SequenceNumber32(201),
                        "Sequence number differs from expected");
  sackList = rxBuf.GetSackList();
  NS_TEST_ASSERT_MSG_EQ(sackList.size(), 1,
                        "SACK list should contain one element");
  it = sackList.begin();
  NS_TEST_ASSERT_MSG_EQ(it->first, SequenceNumber32(501),
                        "SACK block different than expected");
  NS_TEST_ASSERT_MSG_EQ(it->second, SequenceNumber32(601),
                        "SACK block different than expected");

  h.SetSequenceNumber(SequenceNumber32(401));
  rxBuf.Add(p, h);

  NS_TEST_ASSERT_MSG_EQ(rxBuf.NextRxSequence(), SequenceNumber32(201),
                        "Sequence number differs from expected");
  sackList = rxBuf.GetSackList();
  NS_TEST_ASSERT_MSG_EQ(sackList.size(), 1,
                        "SACK list should contain one element");
  it = sackList.begin();
  NS_TEST_ASSERT_MSG_EQ(it->first, SequenceNumber32(401),
                        "SACK block different than expected");
  NS_TEST_ASSERT_MSG_EQ(it->second, SequenceNumber32(601),
                        "SACK block different than expected");

  h.SetSequenceNumber(SequenceNumber32(601));
  rxBuf.Add(p, h);

  NS_TEST_ASSERT_MSG_EQ(rxBuf.NextRxSequence(), SequenceNumber32(201),
                        "Sequence number differs from expected");
  sackList = rxBuf.GetSackList();
  NS_TEST_ASSERT_MSG_EQ(sackList.size(), 1,
                        "SACK list should contain one element");
  it = sackList.begin();
  NS_TEST_ASSERT_MSG_EQ(it->first, SequenceNumber32(401),
                        "SACK block different than expected");
  NS_TEST_ASSERT_MSG_EQ(it->second, SequenceNumber32(701),
                        "SACK block different than expected");

  h.SetSequenceNumber(SequenceNumber32(901));
  rxBuf.Add(p, h);

  NS_TEST_ASSERT_MSG_EQ(rxBuf.NextRxSequence(), SequenceNumber32(201),
                        "Sequence number differs from expected");
  sackList = rxBuf.GetSackList();
  NS_TEST_ASSERT_MSG_EQ(sackList.size(), 2,
                        "SACK list should contain two element");
  it = sackList.begin();
  NS_TEST_ASSERT_MSG_EQ(it->first, SequenceNumber32(901),
                        "SACK block different than expected");
  NS_TEST_ASSERT_MSG_EQ(it->second, SequenceNumber32(1001),
                        "SACK block different than expected");
  ++it;
  NS_TEST_ASSERT_MSG_EQ(it->first, SequenceNumber32(401),
                        "SACK block different than expected");
  NS_TEST_ASSERT_MSG_EQ(it->second, SequenceNumber32(701),
                        "SACK block different than expected");

  h.SetSequenceNumber(SequenceNumber32(1201));
  rxBuf.Add(p, h);

  NS_TEST_ASSERT_MSG_EQ(rxBuf.NextRxSequence(), SequenceNumber32(201),
                        "Sequence number differs from expected");
  sackList = rxBuf.GetSackList();
  NS_TEST_ASSERT_MSG_EQ(sackList.size(), 3,
                        "SACK list should contain three element");
  it = sackList.begin();
  NS_TEST_ASSERT_MSG_EQ(it->first, SequenceNumber32(1201),
                        "SACK block different than expected");
  NS_TEST_ASSERT_MSG_EQ(it->second, SequenceNumber32(1301),
                        "SACK block different than expected");
  ++it;
  NS_TEST_ASSERT_MSG_EQ(it->first, SequenceNumber32(901),
                        "SACK block different than expected");
  NS_TEST_ASSERT_MSG_EQ(it->second, SequenceNumber32(1001),
                        "SACK block different than expected");
  ++it;
  NS_TEST_ASSERT_MSG_EQ(it->first, SequenceNumber32(401),
                        "SACK block different than expected");
  NS_TEST_ASSERT_MSG_EQ(it->second, SequenceNumber32(701),
                        "SACK block different than expected");

  h.SetSequenceNumber(SequenceNumber32(1401));
  rxBuf.Add(p, h);

  NS_TEST_ASSERT_MSG_EQ(rxBuf.NextRxSequence(), SequenceNumber32(201),
                        "Sequence number differs from expected");
  sackList = rxBuf.GetSackList();
  NS_TEST_ASSERT_MSG_EQ(sackList.size(), 4,
                        "SACK list should contain four element");
  it = sackList.begin();
  NS_TEST_ASSERT_MSG_EQ(it->first, SequenceNumber32(1401),
                        "SACK block different than expected");
  NS_TEST_ASSERT_MSG_EQ(it->second, SequenceNumber32(1501),
                        "SACK block different than expected");
  ++it;
  NS_TEST_ASSERT_MSG_EQ(it->first, SequenceNumber32(1201),
                        "SACK block different than expected");
  NS_TEST_ASSERT_MSG_EQ(it->second, SequenceNumber32(1301),
                        "SACK block different than expected");
  ++it;
  NS_TEST_ASSERT_MSG_EQ(it->first, SequenceNumber32(901),
                        "SACK block different than expected");
  NS_TEST_ASSERT_MSG_EQ(it->second, SequenceNumber32(1001),
                        "SACK block different than expected");
  ++it;
  NS_TEST_ASSERT_MSG_EQ(it->first, SequenceNumber32(401),
                        "SACK block different than expected");
  NS_TEST_ASSERT_MSG_EQ(it->second, SequenceNumber32(701),
                        "SACK block different than expected");

  h.SetSequenceNumber(SequenceNumber32(201));
  rxBuf.Add(p, h);

  NS_TEST_ASSERT_MSG_EQ(rxBuf.NextRxSequence(), SequenceNumber32(301),
                        "Sequence number differs from expected");
  sackList = rxBuf.GetSackList();
  NS_TEST_ASSERT_MSG_EQ(sackList.size(), 4,
                        "SACK list should contain four element");

  h.SetSequenceNumber(SequenceNumber32(301));
  rxBuf.Add(p, h);

  NS_TEST_ASSERT_MSG_EQ(rxBuf.NextRxSequence(), SequenceNumber32(701),
                        "Sequence number differs from expected");
  sackList = rxBuf.GetSackList();
  NS_TEST_ASSERT_MSG_EQ(sackList.size(), 3,
                        "SACK list should contain three element");

  it = sackList.begin();
  NS_TEST_ASSERT_MSG_EQ(it->first, SequenceNumber32(1401),
                        "SACK block different than expected");
  NS_TEST_ASSERT_MSG_EQ(it->second, SequenceNumber32(1501),
                        "SACK block different than expected");
  ++it;
  NS_TEST_ASSERT_MSG_EQ(it->first, SequenceNumber32(1201),
                        "SACK block different than expected");
  NS_TEST_ASSERT_MSG_EQ(it->second, SequenceNumber32(1301),
                        "SACK block different than expected");
  ++it;
  NS_TEST_ASSERT_MSG_EQ(it->first, SequenceNumber32(901),
                        "SACK block different than expected");
  NS_TEST_ASSERT_MSG_EQ(it->second, SequenceNumber32(1001),
                        "SACK block different than expected");

  h.SetSequenceNumber(SequenceNumber32(801));
  rxBuf.Add(p, h);

  NS_TEST_ASSERT_MSG_EQ(rxBuf.NextRxSequence(), SequenceNumber32(701),
                        "Sequence number differs from expected");
  sackList = rxBuf.GetSackList();
  NS_TEST_ASSERT_MSG_EQ(sackList.size(), 3,
                        "SACK list should contain three element");

  it = sackList.begin();
  NS_TEST_ASSERT_MSG_EQ(it->first, SequenceNumber32(801),
                        "SACK block different than expected");
  NS_TEST_ASSERT_MSG_EQ(it->second, SequenceNumber32(1001),
                        "SACK block different than expected");
  ++it;
  NS_TEST_ASSERT_MSG_EQ(it->first, SequenceNumber32(1401),
                        "SACK block different than expected");
  NS_TEST_ASSERT_MSG_EQ(it->second, SequenceNumber32(1501),
                        "SACK block different than expected");
  ++it;
  NS_TEST_ASSERT_MSG_EQ(it->first, SequenceNumber32(1201),
                        "SACK block different than expected");
  NS_TEST_ASSERT_MSG_EQ(it->second, SequenceNumber32(1301),
                        "SACK block different than expected");

  h.SetSequenceNumber(SequenceNumber32(701));
  rxBuf.Add(p, h);

  NS_TEST_ASSERT_MSG_EQ(rxBuf.NextRxSequence(), SequenceNumber32(1001),
                        "Sequence number differs from expected");
  sackList = rxBuf.GetSackList();
  NS_TEST_ASSERT_MSG_EQ(sackList.size(), 2,
                        "SACK list should contain two element");

  it = sackList.begin();
  NS_TEST_ASSERT_MSG_EQ(it->first, SequenceNumber32(1401),
                        "SACK block different than expected");
  NS_TEST_ASSERT_MSG_EQ(it->second, SequenceNumber32(1501),
                        "SACK block different than expected");
  ++it;
  NS_TEST_ASSERT_MSG_EQ(it->first, SequenceNumber32(1201),
                        "SACK block different than expected");
  NS_TEST_ASSERT_MSG_EQ(it->second, SequenceNumber32(1301),
                        "SACK block different than expected");

  h.SetSequenceNumber(SequenceNumber32(1301));
  rxBuf.Add(p, h);

  NS_TEST_ASSERT_MSG_EQ(rxBuf.NextRxSequence(), SequenceNumber32(1001),
                        "Sequence number differs from expected");
  sackList = rxBuf.GetSackList();
  NS_TEST_ASSERT_MSG_EQ(sackList.size(), 1,
                        "SACK list should contain one element");

  it = sackList.begin();
  NS_TEST_ASSERT_MSG_EQ(it->first, SequenceNumber32(1201),
                        "SACK block different than expected");
  NS_TEST_ASSERT_MSG_EQ(it->second, SequenceNumber32(1501),
                        "SACK block different than expected");

  h.SetSequenceNumber(SequenceNumber32(1001));
  rxBuf.Add(p, h);

  NS_TEST_ASSERT_MSG_EQ(rxBuf.NextRxSequence(), SequenceNumber32(1101),
                        "Sequence number differs from expected");
  sackList = rxBuf.GetSackList();
  NS_TEST_ASSERT_MSG_EQ(sackList.size(), 1,
                        "SACK list should contain one element");

  it = sackList.begin();
  NS_TEST_ASSERT_MSG_EQ(it->first, SequenceNumber32(1201),
                        "SACK block different than expected");
  NS_TEST_ASSERT_MSG_EQ(it->second, SequenceNumber32(1501),
                        "SACK block different than expected");

  h.SetSequenceNumber(SequenceNumber32(1101));
  rxBuf.Add(p, h);

  NS_TEST_ASSERT_MSG_EQ(rxBuf.NextRxSequence(), SequenceNumber32(1501),
                        "Sequence number differs from expected");
  sackList = rxBuf.GetSackList();
  NS_TEST_ASSERT_MSG_EQ(sackList.size(), 0,
                        "SACK list should contain no element");
}

void TcpRxBufferTestCase::DoTeardown() {}

class TcpRxBufferTestSuite : public TestSuite {
public:
  TcpRxBufferTestSuite() : TestSuite("tcp-rx-buffer", UNIT) {
    AddTestCase(new TcpRxBufferTestCase, TestCase::QUICK);
  }
};

static TcpRxBufferTestSuite g_tcpRxBufferTestSuite;
