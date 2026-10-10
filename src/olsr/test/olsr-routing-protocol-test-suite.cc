
#include "ns3/ipv4-header.h"
#include "ns3/olsr-repositories.h"
#include "ns3/olsr-routing-protocol.h"
#include "ns3/test.h"

using namespace ns3;
using namespace olsr;

class OlsrMprTestCase : public TestCase {
public:
  OlsrMprTestCase();
  ~OlsrMprTestCase() override;
  void DoRun() override;
};

OlsrMprTestCase::OlsrMprTestCase()
    : TestCase("Check OLSR MPR computing mechanism") {}

OlsrMprTestCase::~OlsrMprTestCase() {}

void OlsrMprTestCase::DoRun() {
  Ptr<RoutingProtocol> protocol = CreateObject<RoutingProtocol>();
  protocol->m_mainAddress = Ipv4Address("10.0.0.1");
  OlsrState &state = protocol->m_state;

  NeighborTuple neighbor;
  neighbor.status = NeighborTuple::STATUS_SYM;
  neighbor.willingness = Willingness::DEFAULT;
  neighbor.neighborMainAddr = Ipv4Address("10.0.0.2");
  protocol->m_state.InsertNeighborTuple(neighbor);
  neighbor.neighborMainAddr = Ipv4Address("10.0.0.3");
  protocol->m_state.InsertNeighborTuple(neighbor);
  TwoHopNeighborTuple tuple;
  tuple.expirationTime = Seconds(3600);
  tuple.neighborMainAddr = Ipv4Address("10.0.0.2");
  tuple.twoHopNeighborAddr = Ipv4Address("10.0.0.4");
  protocol->m_state.InsertTwoHopNeighborTuple(tuple);
  tuple.neighborMainAddr = Ipv4Address("10.0.0.3");
  tuple.twoHopNeighborAddr = Ipv4Address("10.0.0.4");
  protocol->m_state.InsertTwoHopNeighborTuple(tuple);

  protocol->MprComputation();
  NS_TEST_EXPECT_MSG_EQ(state.GetMprSet().size(), 1,
                        "An only address must be chosen.");
  tuple.neighborMainAddr = Ipv4Address("10.0.0.2");
  tuple.twoHopNeighborAddr = Ipv4Address("10.0.0.5");
  protocol->m_state.InsertTwoHopNeighborTuple(tuple);

  protocol->MprComputation();
  MprSet mpr = state.GetMprSet();
  NS_TEST_EXPECT_MSG_EQ(mpr.size(), 1, "An only address must be chosen.");
  NS_TEST_EXPECT_MSG_EQ((mpr.find("10.0.0.2") != mpr.end()), true,
                        "Node 1 must select node 2 as MPR");
  tuple.neighborMainAddr = Ipv4Address("10.0.0.3");
  tuple.twoHopNeighborAddr = Ipv4Address("10.0.0.6");
  protocol->m_state.InsertTwoHopNeighborTuple(tuple);

  protocol->MprComputation();
  mpr = state.GetMprSet();
  NS_TEST_EXPECT_MSG_EQ(mpr.size(), 2, "An only address must be chosen.");
  NS_TEST_EXPECT_MSG_EQ((mpr.find("10.0.0.2") != mpr.end()), true,
                        "Node 1 must select node 2 as MPR");
  NS_TEST_EXPECT_MSG_EQ((mpr.find("10.0.0.3") != mpr.end()), true,
                        "Node 1 must select node 3 as MPR");
  neighbor.willingness = olsr::Willingness::ALWAYS;
  neighbor.neighborMainAddr = Ipv4Address("10.0.0.7");
  protocol->m_state.InsertNeighborTuple(neighbor);

  protocol->MprComputation();
  mpr = state.GetMprSet();
  NS_TEST_EXPECT_MSG_EQ(mpr.size(), 3, "An only address must be chosen.");
  NS_TEST_EXPECT_MSG_EQ((mpr.find("10.0.0.7") != mpr.end()), true,
                        "Node 1 must select node 7 as MPR");
  neighbor.willingness = Willingness::NEVER;
  neighbor.neighborMainAddr = Ipv4Address("10.0.0.8");
  protocol->m_state.InsertNeighborTuple(neighbor);
  tuple.neighborMainAddr = Ipv4Address("10.0.0.8");
  tuple.twoHopNeighborAddr = Ipv4Address("10.0.0.9");
  protocol->m_state.InsertTwoHopNeighborTuple(tuple);

  protocol->MprComputation();
  mpr = state.GetMprSet();
  NS_TEST_EXPECT_MSG_EQ(mpr.size(), 3, "An only address must be chosen.");
  NS_TEST_EXPECT_MSG_EQ((mpr.find("10.0.0.9") == mpr.end()), true,
                        "Node 1 must NOT select node 8 as MPR");
}

class OlsrProtocolTestSuite : public TestSuite {
public:
  OlsrProtocolTestSuite();
};

OlsrProtocolTestSuite::OlsrProtocolTestSuite()
    : TestSuite("routing-olsr", UNIT) {
  AddTestCase(new OlsrMprTestCase(), TestCase::QUICK);
}

static OlsrProtocolTestSuite g_olsrProtocolTestSuite;
