

#include "ns3/boolean.h"
#include "ns3/bridge-helper.h"
#include "ns3/config.h"
#include "ns3/inet-socket-address.h"
#include "ns3/internet-stack-helper.h"
#include "ns3/ipv4-address-helper.h"
#include "ns3/ipv4-global-routing-helper.h"
#include "ns3/ipv4-global-routing.h"
#include "ns3/ipv4-l3-protocol.h"
#include "ns3/ipv4-packet-info-tag.h"
#include "ns3/ipv4-routing-protocol.h"
#include "ns3/ipv4-routing-table-entry.h"
#include "ns3/ipv4-static-routing-helper.h"
#include "ns3/log.h"
#include "ns3/node-container.h"
#include "ns3/node.h"
#include "ns3/packet.h"
#include "ns3/pointer.h"
#include "ns3/simple-channel.h"
#include "ns3/simple-net-device-helper.h"
#include "ns3/simple-net-device.h"
#include "ns3/simulator.h"
#include "ns3/socket-factory.h"
#include "ns3/string.h"
#include "ns3/test.h"
#include "ns3/udp-socket-factory.h"
#include "ns3/uinteger.h"

#include <vector>

using namespace ns3;

NS_LOG_COMPONENT_DEFINE("Ipv4GlobalRoutingTestSuite");

class LinkTest : public TestCase {
public:
  void DoSetup() override;
  void DoRun() override;
  LinkTest();

private:
  NodeContainer m_nodes;
};

LinkTest::LinkTest() : TestCase("Global routing on point-to-point link") {}

void LinkTest::DoSetup() {
  m_nodes.Create(2);

  Ptr<SimpleChannel> channel = CreateObject<SimpleChannel>();
  SimpleNetDeviceHelper simpleHelper;
  simpleHelper.SetNetDevicePointToPointMode(true);
  NetDeviceContainer net = simpleHelper.Install(m_nodes, channel);

  InternetStackHelper internet;
  Ipv4GlobalRoutingHelper ipv4RoutingHelper;
  internet.SetRoutingHelper(ipv4RoutingHelper);
  internet.Install(m_nodes);

  Ipv4AddressHelper ipv4;
  ipv4.SetBase("10.1.1.0", "255.255.255.252");
  Ipv4InterfaceContainer i = ipv4.Assign(net);
}

void LinkTest::DoRun() {
  Ipv4GlobalRoutingHelper::PopulateRoutingTables();

  Ptr<Ipv4L3Protocol> ip0 = m_nodes.Get(0)->GetObject<Ipv4L3Protocol>();
  NS_TEST_ASSERT_MSG_NE(ip0, nullptr, "Error-- no Ipv4 object");
  Ptr<Ipv4L3Protocol> ip1 = m_nodes.Get(1)->GetObject<Ipv4L3Protocol>();
  NS_TEST_ASSERT_MSG_NE(ip1, nullptr, "Error-- no Ipv4 object");
  Ptr<Ipv4RoutingProtocol> routing0 = ip0->GetRoutingProtocol();
  Ptr<Ipv4GlobalRouting> globalRouting0 =
      routing0->GetObject<Ipv4GlobalRouting>();
  NS_TEST_ASSERT_MSG_NE(globalRouting0, nullptr,
                        "Error-- no Ipv4GlobalRouting object");
  Ptr<Ipv4RoutingProtocol> routing1 = ip1->GetRoutingProtocol();
  Ptr<Ipv4GlobalRouting> globalRouting1 =
      routing1->GetObject<Ipv4GlobalRouting>();
  NS_TEST_ASSERT_MSG_NE(globalRouting1, nullptr,
                        "Error-- no Ipv4GlobalRouting object");

  uint32_t nRoutes0 = globalRouting0->GetNRoutes();
  NS_LOG_DEBUG("LinkTest nRoutes0 " << nRoutes0);
  NS_TEST_ASSERT_MSG_EQ(nRoutes0, 1, "Error-- not one route");
  Ipv4RoutingTableEntry *route = globalRouting0->GetRoute(0);
  NS_LOG_DEBUG("entry dest " << route->GetDest() << " gw "
                             << route->GetGateway());
  NS_TEST_ASSERT_MSG_EQ(route->GetDest(), Ipv4Address("0.0.0.0"),
                        "Error-- wrong destination");
  NS_TEST_ASSERT_MSG_EQ(route->GetGateway(), Ipv4Address("10.1.1.2"),
                        "Error-- wrong gateway");

  uint32_t nRoutes1 = globalRouting1->GetNRoutes();
  NS_TEST_ASSERT_MSG_EQ(nRoutes1, 1, "Error-- not one route");
  NS_LOG_DEBUG("LinkTest nRoutes1 " << nRoutes1);
  route = globalRouting1->GetRoute(0);
  NS_LOG_DEBUG("entry dest " << route->GetDest() << " gw "
                             << route->GetGateway());
  NS_TEST_ASSERT_MSG_EQ(route->GetDest(), Ipv4Address("0.0.0.0"),
                        "Error-- wrong destination");
  NS_TEST_ASSERT_MSG_EQ(route->GetGateway(), Ipv4Address("10.1.1.1"),
                        "Error-- wrong gateway");

  bool result = true;

  NS_TEST_ASSERT_MSG_EQ(result, true, "Message");
  Simulator::Run();
  Simulator::Destroy();
}

class LanTest : public TestCase {
public:
  void DoSetup() override;
  void DoRun() override;
  LanTest();

private:
  NodeContainer m_nodes;
};

LanTest::LanTest() : TestCase("Global routing on broadcast link") {}

void LanTest::DoSetup() {
  m_nodes.Create(2);

  Ptr<SimpleChannel> channel = CreateObject<SimpleChannel>();
  SimpleNetDeviceHelper simpleHelper;
  NetDeviceContainer net = simpleHelper.Install(m_nodes, channel);

  InternetStackHelper internet;
  Ipv4GlobalRoutingHelper ipv4RoutingHelper;
  internet.SetRoutingHelper(ipv4RoutingHelper);
  internet.Install(m_nodes);

  Ipv4AddressHelper ipv4;
  ipv4.SetBase("10.1.1.0", "255.255.255.0");
  Ipv4InterfaceContainer i = ipv4.Assign(net);
}

void LanTest::DoRun() {
  Ipv4GlobalRoutingHelper::PopulateRoutingTables();

  Ptr<Ipv4L3Protocol> ip0 = m_nodes.Get(0)->GetObject<Ipv4L3Protocol>();
  NS_TEST_ASSERT_MSG_NE(ip0, nullptr, "Error-- no Ipv4 object");
  Ptr<Ipv4L3Protocol> ip1 = m_nodes.Get(1)->GetObject<Ipv4L3Protocol>();
  NS_TEST_ASSERT_MSG_NE(ip1, nullptr, "Error-- no Ipv4 object");
  Ptr<Ipv4RoutingProtocol> routing0 = ip0->GetRoutingProtocol();
  Ptr<Ipv4GlobalRouting> globalRouting0 =
      routing0->GetObject<Ipv4GlobalRouting>();
  NS_TEST_ASSERT_MSG_NE(globalRouting0, nullptr,
                        "Error-- no Ipv4GlobalRouting object");
  Ptr<Ipv4RoutingProtocol> routing1 = ip1->GetRoutingProtocol();
  Ptr<Ipv4GlobalRouting> globalRouting1 =
      routing1->GetObject<Ipv4GlobalRouting>();
  NS_TEST_ASSERT_MSG_NE(globalRouting1, nullptr,
                        "Error-- no Ipv4GlobalRouting object");

  uint32_t nRoutes0 = globalRouting0->GetNRoutes();
  NS_LOG_DEBUG("LanTest nRoutes0 " << nRoutes0);
  NS_TEST_ASSERT_MSG_EQ(nRoutes0, 1, "Error-- more than one entry");
  for (uint32_t i = 0; i < globalRouting0->GetNRoutes(); i++) {
    Ipv4RoutingTableEntry *route = globalRouting0->GetRoute(i);
    NS_LOG_DEBUG("entry dest " << route->GetDest() << " gw "
                               << route->GetGateway());
  }

  uint32_t nRoutes1 = globalRouting1->GetNRoutes();
  NS_LOG_DEBUG("LanTest nRoutes1 " << nRoutes1);
  NS_TEST_ASSERT_MSG_EQ(nRoutes1, 1, "Error-- more than one entry");
  for (uint32_t i = 0; i < globalRouting0->GetNRoutes(); i++) {
    Ipv4RoutingTableEntry *route = globalRouting1->GetRoute(i);
    NS_LOG_DEBUG("entry dest " << route->GetDest() << " gw "
                               << route->GetGateway());
  }

  Simulator::Destroy();
}

class TwoLinkTest : public TestCase {
public:
  void DoSetup() override;
  void DoRun() override;
  TwoLinkTest();

private:
  NodeContainer m_nodes;
};

TwoLinkTest::TwoLinkTest()
    : TestCase("Global routing across two hops (point-to-point links)") {}

void TwoLinkTest::DoSetup() {
  m_nodes.Create(3);

  Ptr<SimpleChannel> channel = CreateObject<SimpleChannel>();
  SimpleNetDeviceHelper simpleHelper;
  simpleHelper.SetNetDevicePointToPointMode(true);
  NetDeviceContainer net = simpleHelper.Install(m_nodes.Get(0), channel);
  net.Add(simpleHelper.Install(m_nodes.Get(1), channel));

  Ptr<SimpleChannel> channel2 = CreateObject<SimpleChannel>();
  SimpleNetDeviceHelper simpleHelper2;
  simpleHelper2.SetNetDevicePointToPointMode(true);
  NetDeviceContainer net2 = simpleHelper.Install(m_nodes.Get(1), channel2);
  net2.Add(simpleHelper2.Install(m_nodes.Get(2), channel2));

  InternetStackHelper internet;
  Ipv4GlobalRoutingHelper ipv4RoutingHelper;
  internet.SetRoutingHelper(ipv4RoutingHelper);
  internet.Install(m_nodes);

  Ipv4AddressHelper ipv4;
  ipv4.SetBase("10.1.1.0", "255.255.255.252");
  Ipv4InterfaceContainer i = ipv4.Assign(net);
  ipv4.SetBase("10.1.2.0", "255.255.255.252");
  Ipv4InterfaceContainer i2 = ipv4.Assign(net2);
}

void TwoLinkTest::DoRun() {
  Ipv4GlobalRoutingHelper::PopulateRoutingTables();

  Ptr<Ipv4L3Protocol> ip0 = m_nodes.Get(0)->GetObject<Ipv4L3Protocol>();
  NS_TEST_ASSERT_MSG_NE(ip0, nullptr, "Error-- no Ipv4 object");
  Ptr<Ipv4L3Protocol> ip1 = m_nodes.Get(1)->GetObject<Ipv4L3Protocol>();
  NS_TEST_ASSERT_MSG_NE(ip1, nullptr, "Error-- no Ipv4 object");
  Ptr<Ipv4L3Protocol> ip2 = m_nodes.Get(2)->GetObject<Ipv4L3Protocol>();
  NS_TEST_ASSERT_MSG_NE(ip2, nullptr, "Error-- no Ipv4 object");
  Ptr<Ipv4RoutingProtocol> routing0 = ip0->GetRoutingProtocol();
  Ptr<Ipv4GlobalRouting> globalRouting0 =
      routing0->GetObject<Ipv4GlobalRouting>();
  NS_TEST_ASSERT_MSG_NE(globalRouting0, nullptr,
                        "Error-- no Ipv4GlobalRouting object");
  Ptr<Ipv4RoutingProtocol> routing1 = ip1->GetRoutingProtocol();
  Ptr<Ipv4GlobalRouting> globalRouting1 =
      routing1->GetObject<Ipv4GlobalRouting>();
  NS_TEST_ASSERT_MSG_NE(globalRouting1, nullptr,
                        "Error-- no Ipv4GlobalRouting object");
  Ptr<Ipv4RoutingProtocol> routing2 = ip2->GetRoutingProtocol();
  Ptr<Ipv4GlobalRouting> globalRouting2 =
      routing2->GetObject<Ipv4GlobalRouting>();
  NS_TEST_ASSERT_MSG_NE(globalRouting2, nullptr,
                        "Error-- no Ipv4GlobalRouting object");

  uint32_t nRoutes0 = globalRouting0->GetNRoutes();
  NS_LOG_DEBUG("TwoLinkTest nRoutes0 " << nRoutes0);
  NS_TEST_ASSERT_MSG_EQ(nRoutes0, 1, "Error-- wrong number of links");

  Ipv4RoutingTableEntry *route = globalRouting0->GetRoute(0);
  NS_LOG_DEBUG("entry dest " << route->GetDest() << " gw "
                             << route->GetGateway());
  NS_TEST_ASSERT_MSG_EQ(route->GetDest(), Ipv4Address("0.0.0.0"),
                        "Error-- wrong destination");
  NS_TEST_ASSERT_MSG_EQ(route->GetGateway(), Ipv4Address("10.1.1.2"),
                        "Error-- wrong gateway");

  uint32_t nRoutes1 = globalRouting1->GetNRoutes();
  NS_LOG_DEBUG("TwoLinkTest nRoutes1 " << nRoutes1);
  route = globalRouting1->GetRoute(0);
  NS_LOG_DEBUG("TwoLinkTest entry dest " << route->GetDest() << " gw "
                                         << route->GetGateway());
  NS_TEST_ASSERT_MSG_EQ(route->GetDest(), Ipv4Address("10.1.1.1"),
                        "Error-- wrong destination");
  NS_TEST_ASSERT_MSG_EQ(route->GetGateway(), Ipv4Address("10.1.1.1"),
                        "Error-- wrong gateway");
  route = globalRouting1->GetRoute(1);
  NS_LOG_DEBUG("TwoLinkTest entry dest " << route->GetDest() << " gw "
                                         << route->GetGateway());
  NS_TEST_ASSERT_MSG_EQ(route->GetDest(), Ipv4Address("10.1.2.2"),
                        "Error-- wrong destination");
  NS_TEST_ASSERT_MSG_EQ(route->GetGateway(), Ipv4Address("10.1.2.2"),
                        "Error-- wrong gateway");
  route = globalRouting1->GetRoute(2);
  NS_LOG_DEBUG("TwoLinkTest entry dest " << route->GetDest() << " gw "
                                         << route->GetGateway());
  NS_TEST_ASSERT_MSG_EQ(route->GetDest(), Ipv4Address("10.1.1.0"),
                        "Error-- wrong destination");
  NS_TEST_ASSERT_MSG_EQ(route->GetGateway(), Ipv4Address("10.1.1.1"),
                        "Error-- wrong gateway");
  route = globalRouting1->GetRoute(3);
  NS_LOG_DEBUG("TwoLinkTest entry dest " << route->GetDest() << " gw "
                                         << route->GetGateway());
  NS_TEST_ASSERT_MSG_EQ(route->GetDest(), Ipv4Address("10.1.2.0"),
                        "Error-- wrong destination");
  NS_TEST_ASSERT_MSG_EQ(route->GetGateway(), Ipv4Address("10.1.2.2"),
                        "Error-- wrong gateway");

  uint32_t nRoutes2 = globalRouting2->GetNRoutes();
  NS_LOG_DEBUG("TwoLinkTest nRoutes2 " << nRoutes2);
  NS_TEST_ASSERT_MSG_EQ(nRoutes2, 1, "Error-- wrong number of links");

  route = globalRouting2->GetRoute(0);
  NS_LOG_DEBUG("entry dest " << route->GetDest() << " gw "
                             << route->GetGateway());
  NS_TEST_ASSERT_MSG_EQ(route->GetDest(), Ipv4Address("0.0.0.0"),
                        "Error-- wrong destination");
  NS_TEST_ASSERT_MSG_EQ(route->GetGateway(), Ipv4Address("10.1.2.1"),
                        "Error-- wrong gateway");

  Simulator::Destroy();
}

class TwoLanTest : public TestCase {
public:
  void DoSetup() override;
  void DoRun() override;
  TwoLanTest();

private:
  NodeContainer m_nodes;
};

TwoLanTest::TwoLanTest()
    : TestCase("Global routing across two hops (broadcast links)") {}

void TwoLanTest::DoSetup() {
  m_nodes.Create(3);

  Ptr<SimpleChannel> channel = CreateObject<SimpleChannel>();
  SimpleNetDeviceHelper simpleHelper;
  NetDeviceContainer net = simpleHelper.Install(m_nodes.Get(0), channel);
  net.Add(simpleHelper.Install(m_nodes.Get(1), channel));

  Ptr<SimpleChannel> channel2 = CreateObject<SimpleChannel>();
  SimpleNetDeviceHelper simpleHelper2;
  NetDeviceContainer net2 = simpleHelper.Install(m_nodes.Get(1), channel2);
  net2.Add(simpleHelper2.Install(m_nodes.Get(2), channel2));

  InternetStackHelper internet;
  Ipv4GlobalRoutingHelper ipv4RoutingHelper;
  internet.SetRoutingHelper(ipv4RoutingHelper);
  internet.Install(m_nodes);

  Ipv4AddressHelper ipv4;
  ipv4.SetBase("10.1.1.0", "255.255.255.0");
  Ipv4InterfaceContainer i = ipv4.Assign(net);
  ipv4.SetBase("10.1.2.0", "255.255.255.0");
  Ipv4InterfaceContainer i2 = ipv4.Assign(net2);
}

void TwoLanTest::DoRun() {
  Ipv4GlobalRoutingHelper::PopulateRoutingTables();

  Ptr<Ipv4L3Protocol> ip0 = m_nodes.Get(0)->GetObject<Ipv4L3Protocol>();
  NS_TEST_ASSERT_MSG_NE(ip0, nullptr, "Error-- no Ipv4 object");
  Ptr<Ipv4L3Protocol> ip1 = m_nodes.Get(1)->GetObject<Ipv4L3Protocol>();
  NS_TEST_ASSERT_MSG_NE(ip1, nullptr, "Error-- no Ipv4 object");
  Ptr<Ipv4L3Protocol> ip2 = m_nodes.Get(2)->GetObject<Ipv4L3Protocol>();
  NS_TEST_ASSERT_MSG_NE(ip2, nullptr, "Error-- no Ipv4 object");
  Ptr<Ipv4RoutingProtocol> routing0 = ip0->GetRoutingProtocol();
  Ptr<Ipv4GlobalRouting> globalRouting0 =
      routing0->GetObject<Ipv4GlobalRouting>();
  NS_TEST_ASSERT_MSG_NE(globalRouting0, nullptr,
                        "Error-- no Ipv4GlobalRouting object");
  Ptr<Ipv4RoutingProtocol> routing1 = ip1->GetRoutingProtocol();
  Ptr<Ipv4GlobalRouting> globalRouting1 =
      routing1->GetObject<Ipv4GlobalRouting>();
  NS_TEST_ASSERT_MSG_NE(globalRouting1, nullptr,
                        "Error-- no Ipv4GlobalRouting object");
  Ptr<Ipv4RoutingProtocol> routing2 = ip2->GetRoutingProtocol();
  Ptr<Ipv4GlobalRouting> globalRouting2 =
      routing2->GetObject<Ipv4GlobalRouting>();
  NS_TEST_ASSERT_MSG_NE(globalRouting2, nullptr,
                        "Error-- no Ipv4GlobalRouting object");

  uint32_t nRoutes0 = globalRouting0->GetNRoutes();
  NS_LOG_DEBUG("TwoLanTest nRoutes0 " << nRoutes0);
  NS_TEST_ASSERT_MSG_EQ(nRoutes0, 2, "Error-- not two entries");
  Ipv4RoutingTableEntry *route = globalRouting0->GetRoute(0);
  NS_LOG_DEBUG("entry dest " << route->GetDest() << " gw "
                             << route->GetGateway());
  NS_TEST_ASSERT_MSG_EQ(route->GetDest(), Ipv4Address("10.1.1.0"),
                        "Error-- wrong destination");
  NS_TEST_ASSERT_MSG_EQ(route->GetGateway(), Ipv4Address("0.0.0.0"),
                        "Error-- wrong gateway");
  route = globalRouting0->GetRoute(1);
  NS_LOG_DEBUG("entry dest " << route->GetDest() << " gw "
                             << route->GetGateway());
  NS_TEST_ASSERT_MSG_EQ(route->GetDest(), Ipv4Address("10.1.2.0"),
                        "Error-- wrong destination");
  NS_TEST_ASSERT_MSG_EQ(route->GetGateway(), Ipv4Address("10.1.1.2"),
                        "Error-- wrong gateway");

  uint32_t nRoutes1 = globalRouting1->GetNRoutes();
  NS_LOG_DEBUG("TwoLanTest nRoutes1 " << nRoutes1);
  NS_TEST_ASSERT_MSG_EQ(nRoutes1, 2, "Error-- not two entries");
  route = globalRouting1->GetRoute(0);
  NS_LOG_DEBUG("TwoLanTest entry dest " << route->GetDest() << " gw "
                                        << route->GetGateway());
  NS_TEST_ASSERT_MSG_EQ(route->GetDest(), Ipv4Address("10.1.1.0"),
                        "Error-- wrong destination");
  NS_TEST_ASSERT_MSG_EQ(route->GetGateway(), Ipv4Address("0.0.0.0"),
                        "Error-- wrong gateway");
  route = globalRouting1->GetRoute(1);
  NS_LOG_DEBUG("TwoLanTest entry dest " << route->GetDest() << " gw "
                                        << route->GetGateway());
  NS_TEST_ASSERT_MSG_EQ(route->GetDest(), Ipv4Address("10.1.2.0"),
                        "Error-- wrong destination");
  NS_TEST_ASSERT_MSG_EQ(route->GetGateway(), Ipv4Address("0.0.0.0"),
                        "Error-- wrong gateway");

  Simulator::Destroy();
}

class BridgeTest : public TestCase {
public:
  void DoSetup() override;
  void DoRun() override;
  BridgeTest();

private:
  NodeContainer m_nodes;
};

BridgeTest::BridgeTest()
    : TestCase("Global routing across bridging topology (bug 2102)") {}

void BridgeTest::DoSetup() {
  m_nodes.Create(5);

  Ptr<SimpleChannel> channel = CreateObject<SimpleChannel>();
  SimpleNetDeviceHelper simpleHelper;
  NetDeviceContainer net = simpleHelper.Install(m_nodes.Get(0), channel);
  net.Add(simpleHelper.Install(m_nodes.Get(1), channel));

  NetDeviceContainer bridgeFacingDevices;
  NetDeviceContainer switchDevices;

  Ptr<SimpleChannel> channel2 = CreateObject<SimpleChannel>();
  SimpleNetDeviceHelper simpleHelper2;
  NetDeviceContainer net2 = simpleHelper2.Install(m_nodes.Get(1), channel2);
  net2.Add(simpleHelper2.Install(m_nodes.Get(2), channel2));
  bridgeFacingDevices.Add(net2.Get(0));
  switchDevices.Add(net2.Get(1));

  Ptr<SimpleChannel> channel3 = CreateObject<SimpleChannel>();
  SimpleNetDeviceHelper simpleHelper3;
  NetDeviceContainer net3 = simpleHelper3.Install(m_nodes.Get(2), channel3);
  net3.Add(simpleHelper3.Install(m_nodes.Get(3), channel3));
  bridgeFacingDevices.Add(net3.Get(1));
  switchDevices.Add(net3.Get(0));

  Ptr<SimpleChannel> channel4 = CreateObject<SimpleChannel>();
  SimpleNetDeviceHelper simpleHelper4;
  NetDeviceContainer net4 = simpleHelper4.Install(m_nodes.Get(3), channel4);
  net4.Add(simpleHelper4.Install(m_nodes.Get(4), channel4));

  Ptr<Node> switchNode = m_nodes.Get(2);
  BridgeHelper bridge;
  bridge.Install(switchNode, switchDevices);

  InternetStackHelper internet;
  Ipv4GlobalRoutingHelper ipv4RoutingHelper;
  internet.SetRoutingHelper(ipv4RoutingHelper);

  internet.Install(m_nodes.Get(0));
  internet.Install(m_nodes.Get(1));
  internet.Install(m_nodes.Get(3));
  internet.Install(m_nodes.Get(4));

  Ipv4AddressHelper address;
  address.SetBase("10.1.1.0", "255.255.255.0");
  address.Assign(net);

  address.SetBase("10.1.2.0", "255.255.255.0");
  address.Assign(bridgeFacingDevices);

  address.SetBase("10.1.3.0", "255.255.255.0");
  address.Assign(net4);
}

void BridgeTest::DoRun() {
  Ipv4GlobalRoutingHelper::PopulateRoutingTables();

  Ptr<Ipv4L3Protocol> ip0 = m_nodes.Get(0)->GetObject<Ipv4L3Protocol>();
  NS_TEST_ASSERT_MSG_NE(ip0, nullptr, "Error-- no Ipv4 object");
  Ptr<Ipv4RoutingProtocol> routing0 = ip0->GetRoutingProtocol();
  NS_TEST_ASSERT_MSG_NE(routing0, nullptr,
                        "Error-- no Ipv4 routing protocol object");
  Ptr<Ipv4GlobalRouting> globalRouting0 =
      routing0->GetObject<Ipv4GlobalRouting>();
  NS_TEST_ASSERT_MSG_NE(globalRouting0, nullptr,
                        "Error-- no Ipv4GlobalRouting object");

  Ptr<Ipv4L3Protocol> ip1 = m_nodes.Get(1)->GetObject<Ipv4L3Protocol>();
  NS_TEST_ASSERT_MSG_NE(ip1, nullptr, "Error-- no Ipv4 object");
  Ptr<Ipv4RoutingProtocol> routing1 = ip1->GetRoutingProtocol();
  NS_TEST_ASSERT_MSG_NE(routing1, nullptr,
                        "Error-- no Ipv4 routing protocol object");
  Ptr<Ipv4GlobalRouting> globalRouting1 =
      routing1->GetObject<Ipv4GlobalRouting>();
  NS_TEST_ASSERT_MSG_NE(globalRouting1, nullptr,
                        "Error-- no Ipv4GlobalRouting object");

  Ptr<Ipv4L3Protocol> ip4 = m_nodes.Get(4)->GetObject<Ipv4L3Protocol>();
  NS_TEST_ASSERT_MSG_NE(ip4, nullptr, "Error-- no Ipv4 object");
  Ptr<Ipv4RoutingProtocol> routing4 = ip4->GetRoutingProtocol();
  NS_TEST_ASSERT_MSG_NE(routing4, nullptr,
                        "Error-- no Ipv4 routing protocol object");
  Ptr<Ipv4GlobalRouting> globalRouting4 =
      routing4->GetObject<Ipv4GlobalRouting>();
  NS_TEST_ASSERT_MSG_NE(globalRouting4, nullptr,
                        "Error-- no Ipv4GlobalRouting object");

  Ipv4RoutingTableEntry *route = nullptr;
  uint32_t nRoutes0 = globalRouting0->GetNRoutes();
  NS_LOG_DEBUG("BridgeTest nRoutes0 " << nRoutes0);
  NS_TEST_ASSERT_MSG_EQ(nRoutes0, 3, "Error-- not three entries");
  for (uint32_t i = 0; i < globalRouting0->GetNRoutes(); i++) {
    route = globalRouting0->GetRoute(i);
    NS_LOG_DEBUG("entry dest " << route->GetDest() << " gw "
                               << route->GetGateway());
  }
  if (route) {
    NS_TEST_ASSERT_MSG_EQ(route->GetDest(), Ipv4Address("10.1.3.0"),
                          "Error-- wrong destination");
    NS_TEST_ASSERT_MSG_EQ(route->GetGateway(), Ipv4Address("10.1.1.2"),
                          "Error-- wrong gateway");
  }

  route = nullptr;
  uint32_t nRoutes1 = globalRouting1->GetNRoutes();
  NS_LOG_DEBUG("BridgeTest nRoutes1 " << nRoutes1);
  NS_TEST_ASSERT_MSG_EQ(nRoutes1, 3, "Error-- not three entries");
  for (uint32_t i = 0; i < globalRouting1->GetNRoutes(); i++) {
    route = globalRouting1->GetRoute(i);
    NS_LOG_DEBUG("entry dest " << route->GetDest() << " gw "
                               << route->GetGateway());
  }
  if (route) {
    NS_TEST_ASSERT_MSG_EQ(route->GetDest(), Ipv4Address("10.1.3.0"),
                          "Error-- wrong destination");
    NS_TEST_ASSERT_MSG_EQ(route->GetGateway(), Ipv4Address("10.1.2.2"),
                          "Error-- wrong gateway");
  }

  NS_LOG_DEBUG("BridgeTest skip print out of n2 and n3, go next to node n4");

  route = nullptr;
  uint32_t nRoutes4 = globalRouting4->GetNRoutes();
  NS_LOG_DEBUG("BridgeTest nRoutes4 " << nRoutes4);
  NS_TEST_ASSERT_MSG_EQ(nRoutes4, 3, "Error-- not three entries");
  for (uint32_t i = 0; i < globalRouting4->GetNRoutes(); i++) {
    route = globalRouting4->GetRoute(i);
    NS_LOG_DEBUG("entry dest " << route->GetDest() << " gw "
                               << route->GetGateway());
  }
  if (route) {
    NS_TEST_ASSERT_MSG_EQ(route->GetDest(), Ipv4Address("10.1.1.0"),
                          "Error-- wrong destination");
    NS_TEST_ASSERT_MSG_EQ(route->GetGateway(), Ipv4Address("10.1.3.1"),
                          "Error-- wrong gateway");
  }

  Simulator::Destroy();
}

class TwoBridgeTest : public TestCase {
public:
  void DoSetup() override;
  void DoRun() override;
  TwoBridgeTest();

private:
  NodeContainer m_nodes;
};

TwoBridgeTest::TwoBridgeTest()
    : TestCase("Global routing across two bridges") {}

void TwoBridgeTest::DoSetup() {
  m_nodes.Create(5);

  Ptr<SimpleChannel> channel = CreateObject<SimpleChannel>();
  SimpleNetDeviceHelper simpleHelper;
  NetDeviceContainer net = simpleHelper.Install(m_nodes.Get(0), channel);
  net.Add(simpleHelper.Install(m_nodes.Get(1), channel));

  NetDeviceContainer bridgeFacingDevices;
  NetDeviceContainer switchn2Devices;
  NetDeviceContainer switchn3Devices;

  Ptr<SimpleChannel> channel2 = CreateObject<SimpleChannel>();
  SimpleNetDeviceHelper simpleHelper2;
  NetDeviceContainer net2 = simpleHelper2.Install(m_nodes.Get(1), channel2);
  net2.Add(simpleHelper2.Install(m_nodes.Get(2), channel2));
  bridgeFacingDevices.Add(net2.Get(0));
  switchn2Devices.Add(net2.Get(1));

  Ptr<SimpleChannel> channel3 = CreateObject<SimpleChannel>();
  SimpleNetDeviceHelper simpleHelper3;
  NetDeviceContainer net3 = simpleHelper3.Install(m_nodes.Get(2), channel3);
  net3.Add(simpleHelper3.Install(m_nodes.Get(3), channel3));
  switchn2Devices.Add(net3.Get(0));
  switchn3Devices.Add(net3.Get(1));

  Ptr<SimpleChannel> channel4 = CreateObject<SimpleChannel>();
  SimpleNetDeviceHelper simpleHelper4;
  NetDeviceContainer net4 = simpleHelper4.Install(m_nodes.Get(3), channel4);
  net4.Add(simpleHelper4.Install(m_nodes.Get(4), channel4));
  switchn3Devices.Add(net4.Get(0));
  bridgeFacingDevices.Add(net4.Get(1));

  Ptr<Node> switchn2Node = m_nodes.Get(2);
  BridgeHelper bridgen2Helper;
  bridgen2Helper.Install(switchn2Node, switchn2Devices);

  Ptr<Node> switchn3Node = m_nodes.Get(3);
  BridgeHelper bridgen3Helper;
  bridgen3Helper.Install(switchn3Node, switchn3Devices);

  InternetStackHelper internet;
  Ipv4GlobalRoutingHelper ipv4RoutingHelper;
  internet.SetRoutingHelper(ipv4RoutingHelper);

  internet.Install(m_nodes.Get(0));
  internet.Install(m_nodes.Get(1));
  internet.Install(m_nodes.Get(4));

  Ipv4AddressHelper address;
  address.SetBase("10.1.1.0", "255.255.255.0");
  address.Assign(net);

  address.SetBase("10.1.2.0", "255.255.255.0");
  address.Assign(bridgeFacingDevices);
}

void TwoBridgeTest::DoRun() {
  Ipv4GlobalRoutingHelper::PopulateRoutingTables();

  Ptr<Ipv4L3Protocol> ip0 = m_nodes.Get(0)->GetObject<Ipv4L3Protocol>();
  NS_TEST_ASSERT_MSG_NE(ip0, nullptr, "Error-- no Ipv4 object");
  Ptr<Ipv4RoutingProtocol> routing0 = ip0->GetRoutingProtocol();
  NS_TEST_ASSERT_MSG_NE(routing0, nullptr,
                        "Error-- no Ipv4 routing protocol object");
  Ptr<Ipv4GlobalRouting> globalRouting0 =
      routing0->GetObject<Ipv4GlobalRouting>();
  NS_TEST_ASSERT_MSG_NE(globalRouting0, nullptr,
                        "Error-- no Ipv4GlobalRouting object");

  Ptr<Ipv4L3Protocol> ip4 = m_nodes.Get(4)->GetObject<Ipv4L3Protocol>();
  NS_TEST_ASSERT_MSG_NE(ip4, nullptr, "Error-- no Ipv4 object");
  Ptr<Ipv4RoutingProtocol> routing4 = ip4->GetRoutingProtocol();
  NS_TEST_ASSERT_MSG_NE(routing4, nullptr,
                        "Error-- no Ipv4 routing protocol object");
  Ptr<Ipv4GlobalRouting> globalRouting4 =
      routing4->GetObject<Ipv4GlobalRouting>();
  NS_TEST_ASSERT_MSG_NE(globalRouting4, nullptr,
                        "Error-- no Ipv4GlobalRouting object");

  Ipv4RoutingTableEntry *route = nullptr;
  uint32_t nRoutes0 = globalRouting0->GetNRoutes();
  NS_LOG_DEBUG("BridgeTest nRoutes0 " << nRoutes0);
  NS_TEST_ASSERT_MSG_EQ(nRoutes0, 2, "Error-- not two entries");
  for (uint32_t i = 0; i < globalRouting0->GetNRoutes(); i++) {
    route = globalRouting0->GetRoute(i);
    NS_LOG_DEBUG("entry dest " << route->GetDest() << " gw "
                               << route->GetGateway());
  }
  if (route) {
    NS_TEST_ASSERT_MSG_EQ(route->GetDest(), Ipv4Address("10.1.2.0"),
                          "Error-- wrong destination");
    NS_TEST_ASSERT_MSG_EQ(route->GetGateway(), Ipv4Address("10.1.1.2"),
                          "Error-- wrong gateway");
  }
  NS_LOG_DEBUG("BridgeTest skip print out of n1-n3, go next to node n4");

  route = nullptr;
  uint32_t nRoutes4 = globalRouting4->GetNRoutes();
  NS_LOG_DEBUG("BridgeTest nRoutes4 " << nRoutes4);
  NS_TEST_ASSERT_MSG_EQ(nRoutes4, 2, "Error-- not two entries");
  for (uint32_t i = 0; i < globalRouting4->GetNRoutes(); i++) {
    route = globalRouting4->GetRoute(i);
    NS_LOG_DEBUG("entry dest " << route->GetDest() << " gw "
                               << route->GetGateway());
  }
  if (route) {
    NS_TEST_ASSERT_MSG_EQ(route->GetDest(), Ipv4Address("10.1.1.0"),
                          "Error-- wrong destination");
    NS_TEST_ASSERT_MSG_EQ(route->GetGateway(), Ipv4Address("10.1.2.1"),
                          "Error-- wrong gateway");
  }

  Simulator::Destroy();
}

class Ipv4DynamicGlobalRoutingTestCase : public TestCase {
public:
  Ipv4DynamicGlobalRoutingTestCase();
  ~Ipv4DynamicGlobalRoutingTestCase() override;

private:
  void SendData(uint8_t index);

  void ShutDownSock(uint8_t index);

  void HandleRead(Ptr<Socket> socket);
  void DoRun() override;

  uint16_t m_count;
  std::vector<std::pair<Ptr<Socket>, bool>> m_sendSocks;
  DataRate m_dataRate;
  uint16_t m_packetSize;
  std::vector<uint8_t> m_firstInterface;
  std::vector<uint8_t> m_secondInterface;
};

Ipv4DynamicGlobalRoutingTestCase::Ipv4DynamicGlobalRoutingTestCase()
    : TestCase("Dynamic global routing example"), m_count(0) {
  m_firstInterface.resize(16);
  m_secondInterface.resize(16);
  m_dataRate = DataRate("2kbps");
  m_packetSize = 50;
}

Ipv4DynamicGlobalRoutingTestCase::~Ipv4DynamicGlobalRoutingTestCase() {
  for (auto iter = m_sendSocks.begin(); iter != m_sendSocks.end(); iter++) {
    if (iter->second) {
      iter->second = false;
      iter->first->Close();
      iter->first = nullptr;
    }
  }
}

void Ipv4DynamicGlobalRoutingTestCase::HandleRead(Ptr<Socket> socket) {
  Ptr<Packet> packet;
  Address from;
  while ((packet = socket->RecvFrom(from))) {
    if (packet->GetSize() == 0) {
      break;
    }
    Ipv4PacketInfoTag tag;
    bool found;
    found = packet->PeekPacketTag(tag);
    uint8_t now = static_cast<uint8_t>(Simulator::Now().GetSeconds());
    if (found) {
      if (tag.GetRecvIf() == 1) {
        m_firstInterface[now]++;
      }
      if (tag.GetRecvIf() == 2) {
        m_secondInterface[now]++;
      }
      m_count++;
    }
  }
}

void Ipv4DynamicGlobalRoutingTestCase::SendData(uint8_t index) {
  if (!m_sendSocks[index].second) {
    return;
  }
  Ptr<Packet> packet = Create<Packet>(m_packetSize);
  m_sendSocks[index].first->Send(packet);

  Time tNext(MicroSeconds(m_packetSize * 8 * 1e6 / m_dataRate.GetBitRate()));
  Simulator::Schedule(tNext, &Ipv4DynamicGlobalRoutingTestCase::SendData, this,
                      index);
}

void Ipv4DynamicGlobalRoutingTestCase::ShutDownSock(uint8_t index) {
  m_sendSocks[index].second = false;
  m_sendSocks[index].first->Close();
  m_sendSocks[index].first = nullptr;
}

void Ipv4DynamicGlobalRoutingTestCase::DoRun() {
  Config::SetDefault("ns3::Ipv4GlobalRouting::RespondToInterfaceEvents",
                     BooleanValue(true));

  NodeContainer c;
  c.Create(7);
  NodeContainer n0n2 = NodeContainer(c.Get(0), c.Get(2));
  NodeContainer n1n2 = NodeContainer(c.Get(1), c.Get(2));
  NodeContainer n5n6 = NodeContainer(c.Get(5), c.Get(6));
  NodeContainer n1n6 = NodeContainer(c.Get(1), c.Get(6));
  NodeContainer n2345 = NodeContainer(c.Get(2), c.Get(3), c.Get(4), c.Get(5));

  InternetStackHelper internet;
  internet.Install(c);

  SimpleNetDeviceHelper devHelper;

  devHelper.SetNetDevicePointToPointMode(true);
  NetDeviceContainer d0d2 = devHelper.Install(n0n2);
  devHelper.SetNetDevicePointToPointMode(false);

  NetDeviceContainer d1d6 = devHelper.Install(n1n6);
  NetDeviceContainer d1d2 = devHelper.Install(n1n2);
  NetDeviceContainer d5d6 = devHelper.Install(n5n6);
  NetDeviceContainer d2345 = devHelper.Install(n2345);

  Ipv4AddressHelper ipv4;
  ipv4.SetBase("10.1.1.0", "255.255.255.0");
  ipv4.Assign(d0d2);

  ipv4.SetBase("10.1.2.0", "255.255.255.0");
  ipv4.Assign(d1d2);

  ipv4.SetBase("10.1.3.0", "255.255.255.0");
  Ipv4InterfaceContainer i5i6 = ipv4.Assign(d5d6);

  ipv4.SetBase("10.250.1.0", "255.255.255.0");
  ipv4.Assign(d2345);

  ipv4.SetBase("172.16.1.0", "255.255.255.0");
  Ipv4InterfaceContainer i1i6 = ipv4.Assign(d1d6);

  Ipv4GlobalRoutingHelper::PopulateRoutingTables();

  TypeId tid = TypeId::LookupByName("ns3::UdpSocketFactory");
  uint16_t port = 9;

  std::pair<Ptr<Socket>, bool> sendSockA;
  sendSockA.first = Socket::CreateSocket(c.Get(1), tid);
  sendSockA.first->Bind();
  sendSockA.first->Connect(InetSocketAddress(i5i6.GetAddress(1), port));
  sendSockA.second = true;
  m_sendSocks.push_back(sendSockA);
  Simulator::Schedule(Seconds(1.0), &Ipv4DynamicGlobalRoutingTestCase::SendData,
                      this, 0);
  Simulator::Schedule(Seconds(10.0),
                      &Ipv4DynamicGlobalRoutingTestCase::ShutDownSock, this, 0);

  std::pair<Ptr<Socket>, bool> sendSockB;
  sendSockB.first = Socket::CreateSocket(c.Get(1), tid);
  sendSockB.first->Bind();
  sendSockB.first->Connect(InetSocketAddress(i1i6.GetAddress(1), port));
  sendSockB.second = true;
  m_sendSocks.push_back(sendSockB);
  Simulator::Schedule(Seconds(11.0),
                      &Ipv4DynamicGlobalRoutingTestCase::SendData, this, 1);
  Simulator::Schedule(Seconds(16.0),
                      &Ipv4DynamicGlobalRoutingTestCase::ShutDownSock, this, 1);

  Ptr<Socket> sink2 = Socket::CreateSocket(c.Get(6), tid);
  sink2->Bind(Address(InetSocketAddress(Ipv4Address::GetAny(), port)));
  sink2->Listen();
  sink2->ShutdownSend();

  sink2->SetRecvPktInfo(true);
  sink2->SetRecvCallback(
      MakeCallback(&Ipv4DynamicGlobalRoutingTestCase::HandleRead, this));

  Ptr<Node> n1 = c.Get(1);
  Ptr<Ipv4> ipv41 = n1->GetObject<Ipv4>();
  uint32_t ipv4ifIndex1 = 2;

  Simulator::Schedule(Seconds(2), &Ipv4::SetDown, ipv41, ipv4ifIndex1);
  Simulator::Schedule(Seconds(4), &Ipv4::SetUp, ipv41, ipv4ifIndex1);

  Ptr<Node> n6 = c.Get(6);
  Ptr<Ipv4> ipv46 = n6->GetObject<Ipv4>();
  uint32_t ipv4ifIndex6 = 2;
  Simulator::Schedule(Seconds(6), &Ipv4::SetDown, ipv46, ipv4ifIndex6);
  Simulator::Schedule(Seconds(8), &Ipv4::SetUp, ipv46, ipv4ifIndex6);

  Simulator::Schedule(Seconds(12), &Ipv4::SetDown, ipv41, ipv4ifIndex1);
  Simulator::Schedule(Seconds(14), &Ipv4::SetUp, ipv41, ipv4ifIndex1);

  Simulator::Run();

  NS_TEST_ASSERT_MSG_EQ(m_count, 70,
                        "Dynamic global routing did not deliver all packets");
  NS_TEST_ASSERT_MSG_EQ(m_firstInterface[1], 5,
                        "Dynamic global routing did not deliver all packets "
                            << int(m_firstInterface[1]));
  NS_TEST_ASSERT_MSG_EQ(m_secondInterface[2], 5,
                        "Dynamic global routing did not deliver all packets "
                            << int(m_secondInterface[2]));
  NS_TEST_ASSERT_MSG_EQ(m_secondInterface[3], 5,
                        "Dynamic global routing did not deliver all packets "
                            << int(m_secondInterface[3]));
  NS_TEST_ASSERT_MSG_EQ(m_firstInterface[4], 5,
                        "Dynamic global routing did not deliver all packets "
                            << int(m_firstInterface[4]));
  NS_TEST_ASSERT_MSG_EQ(m_firstInterface[5], 5,
                        "Dynamic global routing did not deliver all packets "
                            << int(m_firstInterface[5]));
  NS_TEST_ASSERT_MSG_EQ(m_secondInterface[6], 5,
                        "Dynamic global routing did not deliver all packets "
                            << int(m_secondInterface[6]));
  NS_TEST_ASSERT_MSG_EQ(m_secondInterface[7], 5,
                        "Dynamic global routing did not deliver all packets "
                            << int(m_secondInterface[7]));
  NS_TEST_ASSERT_MSG_EQ(m_firstInterface[8], 5,
                        "Dynamic global routing did not deliver all packets "
                            << int(m_firstInterface[8]));
  NS_TEST_ASSERT_MSG_EQ(m_firstInterface[9], 5,
                        "Dynamic global routing did not deliver all packets "
                            << int(m_firstInterface[9]));
  NS_TEST_ASSERT_MSG_EQ(m_firstInterface[10], 0,
                        "Dynamic global routing did not deliver all packets "
                            << int(m_firstInterface[10]));
  NS_TEST_ASSERT_MSG_EQ(m_firstInterface[11], 5,
                        "Dynamic global routing did not deliver all packets "
                            << int(m_firstInterface[11]));
  NS_TEST_ASSERT_MSG_EQ(m_secondInterface[12], 5,
                        "Dynamic global routing did not deliver all packets "
                            << int(m_secondInterface[12]));
  NS_TEST_ASSERT_MSG_EQ(m_secondInterface[13], 5,
                        "Dynamic global routing did not deliver all packets "
                            << int(m_secondInterface[13]));
  NS_TEST_ASSERT_MSG_EQ(m_firstInterface[14], 5,
                        "Dynamic global routing did not deliver all packets "
                            << int(m_firstInterface[14]));
  NS_TEST_ASSERT_MSG_EQ(m_firstInterface[15], 5,
                        "Dynamic global routing did not deliver all packets "
                            << int(m_firstInterface[15]));
  Simulator::Destroy();
}

class Ipv4GlobalRoutingSlash32TestCase : public TestCase {
public:
  Ipv4GlobalRoutingSlash32TestCase();
  ~Ipv4GlobalRoutingSlash32TestCase() override;

  Ptr<Packet> m_receivedPacket;

  void ReceivePkt(Ptr<Socket> socket);
  void DoSendData(Ptr<Socket> socket, std::string to);
  void SendData(Ptr<Socket> socket, std::string to);

private:
  void DoRun() override;
};

Ipv4GlobalRoutingSlash32TestCase::Ipv4GlobalRoutingSlash32TestCase()
    : TestCase("Slash 32 global routing example") {}

Ipv4GlobalRoutingSlash32TestCase::~Ipv4GlobalRoutingSlash32TestCase() {}

void Ipv4GlobalRoutingSlash32TestCase::ReceivePkt(Ptr<Socket> socket) {
  uint32_t availableData [[maybe_unused]] = socket->GetRxAvailable();
  m_receivedPacket = socket->Recv(std::numeric_limits<uint32_t>::max(), 0);
  NS_TEST_ASSERT_MSG_EQ(availableData, m_receivedPacket->GetSize(),
                        "Received packet size is not equal to Rx buffer size");
}

void Ipv4GlobalRoutingSlash32TestCase::DoSendData(Ptr<Socket> socket,
                                                  std::string to) {
  Address realTo = InetSocketAddress(Ipv4Address(to.c_str()), 1234);
  NS_TEST_EXPECT_MSG_EQ(socket->SendTo(Create<Packet>(123), 0, realTo), 123,
                        "100");
}

void Ipv4GlobalRoutingSlash32TestCase::SendData(Ptr<Socket> socket,
                                                std::string to) {
  m_receivedPacket = Create<Packet>();
  Simulator::ScheduleWithContext(socket->GetNode()->GetId(), Seconds(60),
                                 &Ipv4GlobalRoutingSlash32TestCase::DoSendData,
                                 this, socket, to);
  Simulator::Stop(Seconds(66));
  Simulator::Run();
}

void Ipv4GlobalRoutingSlash32TestCase::DoRun() {
  Ptr<Node> nA = CreateObject<Node>();
  Ptr<Node> nB = CreateObject<Node>();
  Ptr<Node> nC = CreateObject<Node>();

  NodeContainer c = NodeContainer(nA, nB, nC);

  InternetStackHelper internet;
  internet.Install(c);

  NodeContainer nAnB = NodeContainer(nA, nB);
  NodeContainer nBnC = NodeContainer(nB, nC);

  SimpleNetDeviceHelper devHelper;

  Ptr<SimpleNetDevice> deviceA = CreateObject<SimpleNetDevice>();
  deviceA->SetAddress(Mac48Address::Allocate());
  nA->AddDevice(deviceA);

  NetDeviceContainer dAdB = devHelper.Install(nAnB);
  NetDeviceContainer dBdC = devHelper.Install(nBnC);

  Ptr<SimpleNetDevice> deviceC = CreateObject<SimpleNetDevice>();
  deviceC->SetAddress(Mac48Address::Allocate());
  nC->AddDevice(deviceC);

  Ipv4AddressHelper ipv4;
  ipv4.SetBase("10.1.1.0", "255.255.255.252");
  Ipv4InterfaceContainer iAiB = ipv4.Assign(dAdB);

  ipv4.SetBase("10.1.1.4", "255.255.255.252");
  Ipv4InterfaceContainer iBiC = ipv4.Assign(dBdC);

  Ptr<Ipv4> ipv4A = nA->GetObject<Ipv4>();
  Ptr<Ipv4> ipv4B = nB->GetObject<Ipv4>();
  Ptr<Ipv4> ipv4C = nC->GetObject<Ipv4>();

  int32_t ifIndexA = ipv4A->AddInterface(deviceA);
  int32_t ifIndexC = ipv4C->AddInterface(deviceC);

  Ipv4InterfaceAddress ifInAddrA =
      Ipv4InterfaceAddress(Ipv4Address("172.16.1.1"), Ipv4Mask("/32"));
  ipv4A->AddAddress(ifIndexA, ifInAddrA);
  ipv4A->SetMetric(ifIndexA, 1);
  ipv4A->SetUp(ifIndexA);

  Ipv4InterfaceAddress ifInAddrC =
      Ipv4InterfaceAddress(Ipv4Address("192.168.1.1"), Ipv4Mask("/32"));
  ipv4C->AddAddress(ifIndexC, ifInAddrC);
  ipv4C->SetMetric(ifIndexC, 1);
  ipv4C->SetUp(ifIndexC);

  Ipv4GlobalRoutingHelper::PopulateRoutingTables();

  Ptr<SocketFactory> rxSocketFactory = nC->GetObject<UdpSocketFactory>();
  Ptr<Socket> rxSocket = rxSocketFactory->CreateSocket();
  NS_TEST_EXPECT_MSG_EQ(
      rxSocket->Bind(InetSocketAddress(Ipv4Address("192.168.1.1"), 1234)), 0,
      "trivial");
  rxSocket->SetRecvCallback(
      MakeCallback(&Ipv4GlobalRoutingSlash32TestCase::ReceivePkt, this));

  Ptr<SocketFactory> txSocketFactory = nA->GetObject<UdpSocketFactory>();
  Ptr<Socket> txSocket = txSocketFactory->CreateSocket();
  txSocket->SetAllowBroadcast(true);

  SendData(txSocket, "192.168.1.1");
  NS_TEST_EXPECT_MSG_EQ(m_receivedPacket->GetSize(), 123,
                        "Static routing with /32 did not deliver all packets.");

  Simulator::Destroy();
}

class Ipv4GlobalRoutingTestSuite : public TestSuite {
public:
  Ipv4GlobalRoutingTestSuite();
};

Ipv4GlobalRoutingTestSuite::Ipv4GlobalRoutingTestSuite()
    : TestSuite("ipv4-global-routing", UNIT) {
  AddTestCase(new LinkTest, TestCase::QUICK);
  AddTestCase(new LanTest, TestCase::QUICK);
  AddTestCase(new TwoLinkTest, TestCase::QUICK);
  AddTestCase(new TwoLanTest, TestCase::QUICK);
  AddTestCase(new BridgeTest, TestCase::QUICK);
  AddTestCase(new TwoBridgeTest, TestCase::QUICK);
  AddTestCase(new Ipv4DynamicGlobalRoutingTestCase, TestCase::QUICK);
  AddTestCase(new Ipv4GlobalRoutingSlash32TestCase, TestCase::QUICK);
}

static Ipv4GlobalRoutingTestSuite g_globalRoutingTestSuite;
