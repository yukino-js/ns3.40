

#include "ns3/brite-module.h"
#include "ns3/core-module.h"
#include "ns3/internet-module.h"
#include "ns3/network-module.h"
#include "ns3/on-off-helper.h"
#include "ns3/packet-sink-helper.h"
#include "ns3/packet-sink.h"
#include "ns3/point-to-point-module.h"
#include "ns3/random-variable-stream.h"
#include "ns3/test.h"

#include <fstream>
#include <iostream>
#include <string>

using namespace ns3;

class BriteTopologyStructureTestCase : public TestCase {
public:
  BriteTopologyStructureTestCase();
  ~BriteTopologyStructureTestCase() override;

private:
  void DoRun() override;
};

BriteTopologyStructureTestCase::BriteTopologyStructureTestCase()
    : TestCase("Test that two brite topologies created with same seed value "
               "produce same graph "
               "(not an exact test)") {}

BriteTopologyStructureTestCase::~BriteTopologyStructureTestCase() {}

void BriteTopologyStructureTestCase::DoRun() {
  std::string confFile = "src/brite/test/test.conf";

  SeedManager::SetRun(1);
  SeedManager::SetSeed(1);
  BriteTopologyHelper bthA(confFile);
  bthA.AssignStreams(1);

  SeedManager::SetRun(1);
  SeedManager::SetSeed(1);
  BriteTopologyHelper bthB(confFile);
  bthB.AssignStreams(1);

  InternetStackHelper stack;

  bthA.BuildBriteTopology(stack);
  bthB.BuildBriteTopology(stack);

  int numAsA = bthA.GetNAs();
  int numAsB = bthB.GetNAs();

  NS_TEST_ASSERT_MSG_EQ(numAsA, 2, "Number of AS for this topology must be 2");
  NS_TEST_ASSERT_MSG_EQ(numAsA, numAsB,
                        "Number of AS should be same for both test topologies");
  NS_TEST_ASSERT_MSG_EQ(
      bthA.GetNNodesTopology(), bthB.GetNNodesTopology(),
      "Total number of nodes for each topology should be equal");
  NS_TEST_ASSERT_MSG_EQ(
      bthA.GetNEdgesTopology(), bthB.GetNEdgesTopology(),
      "Total number of edges for each topology should be equal");

  for (unsigned int i = 0; i < bthA.GetNAs(); ++i) {
    NS_TEST_ASSERT_MSG_EQ(bthA.GetNLeafNodesForAs(i),
                          bthB.GetNLeafNodesForAs(i),
                          "Total number of leaf nodes different for AS " << i);
  }
}

class BriteTopologyFunctionTestCase : public TestCase {
public:
  BriteTopologyFunctionTestCase();
  ~BriteTopologyFunctionTestCase() override;

private:
  void DoRun() override;
};

BriteTopologyFunctionTestCase::BriteTopologyFunctionTestCase()
    : TestCase(
          "Test that packets can be send across a BRITE topology using UDP") {}

BriteTopologyFunctionTestCase::~BriteTopologyFunctionTestCase() {}

void BriteTopologyFunctionTestCase::DoRun() {
  std::string confFile = "src/brite/test/test.conf";
  BriteTopologyHelper bth(confFile);

  PointToPointHelper p2p;
  InternetStackHelper stack;
  Ipv4AddressHelper address;

  address.SetBase("10.0.0.0", "255.255.255.0");

  bth.BuildBriteTopology(stack);
  bth.AssignIpv4Addresses(address);

  NodeContainer source;
  NodeContainer sink;

  source.Create(1);
  stack.Install(source);

  int numNodesInAsZero = bth.GetNNodesForAs(0);
  source.Add(bth.GetNodeForAs(0, numNodesInAsZero - 1));

  sink.Create(1);
  stack.Install(sink);

  int numNodesInAsOne = bth.GetNNodesForAs(1);
  sink.Add(bth.GetNodeForAs(1, numNodesInAsOne - 1));

  p2p.SetDeviceAttribute("DataRate", StringValue("5Mbps"));
  p2p.SetChannelAttribute("Delay", StringValue("2ms"));

  NetDeviceContainer p2pSourceDevices;
  NetDeviceContainer p2pSinkDevices;

  p2pSourceDevices = p2p.Install(source);
  p2pSinkDevices = p2p.Install(sink);

  address.SetBase("10.1.0.0", "255.255.0.0");
  Ipv4InterfaceContainer sourceInterfaces;
  sourceInterfaces = address.Assign(p2pSourceDevices);

  address.SetBase("10.2.0.0", "255.255.0.0");
  Ipv4InterfaceContainer sinkInterfaces;
  sinkInterfaces = address.Assign(p2pSinkDevices);

  uint16_t port = 9;

  OnOffHelper onOff(
      "ns3::UdpSocketFactory",
      Address(InetSocketAddress(sinkInterfaces.GetAddress(0), port)));
  onOff.SetAttribute("OnTime",
                     StringValue("ns3::ConstantRandomVariable[Constant=1]"));
  onOff.SetAttribute("OffTime",
                     StringValue("ns3::ConstantRandomVariable[Constant=0]"));
  onOff.SetAttribute("DataRate", DataRateValue(DataRate(6000)));

  ApplicationContainer apps = onOff.Install(source.Get(0));

  apps.Start(Seconds(1.0));
  apps.Stop(Seconds(10.0));

  PacketSinkHelper sinkHelper(
      "ns3::UdpSocketFactory",
      Address(InetSocketAddress(Ipv4Address::GetAny(), port)));
  apps = sinkHelper.Install(sink.Get(0));

  apps.Start(Seconds(1.0));
  apps.Stop(Seconds(10.0));

  Ipv4GlobalRoutingHelper::PopulateRoutingTables();

  Simulator::Stop(Seconds(10.0));
  Simulator::Run();

  Ptr<PacketSink> sink1 = DynamicCast<PacketSink>(apps.Get(0));

  Simulator::Destroy();
}

class BriteTestSuite : public TestSuite {
public:
  BriteTestSuite() : TestSuite("brite-testing", UNIT) {
    AddTestCase(new BriteTopologyStructureTestCase, TestCase::QUICK);
    AddTestCase(new BriteTopologyFunctionTestCase, TestCase::QUICK);
  }
};

static BriteTestSuite g_briteTestSuite;
