
#include "ns3/arp-l3-protocol.h"
#include "ns3/config.h"
#include "ns3/icmpv4-l4-protocol.h"
#include "ns3/icmpv6-l4-protocol.h"
#include "ns3/inet-socket-address.h"
#include "ns3/inet6-socket-address.h"
#include "ns3/ipv4-l3-protocol.h"
#include "ns3/ipv4-list-routing.h"
#include "ns3/ipv4-static-routing.h"
#include "ns3/ipv6-l3-protocol.h"
#include "ns3/ipv6-list-routing.h"
#include "ns3/ipv6-static-routing.h"
#include "ns3/log.h"
#include "ns3/node.h"
#include "ns3/simple-channel.h"
#include "ns3/simple-net-device.h"
#include "ns3/simulator.h"
#include "ns3/socket-factory.h"
#include "ns3/tcp-l4-protocol.h"
#include "ns3/tcp-socket-factory.h"
#include "ns3/test.h"
#include "ns3/traffic-control-layer.h"
#include "ns3/udp-l4-protocol.h"
#include "ns3/uinteger.h"

#include <string>

using namespace ns3;

NS_LOG_COMPONENT_DEFINE("Ipv6DualStackTestSuite");

class DualStackTestCase : public TestCase {
public:
  DualStackTestCase();

private:
  void DoRun() override;
  void DoTeardown() override;

  void SetUpSim();

  Ptr<Node> node0;
  Ptr<Node> node1;

  void ServerHandleConnectionCreated1(Ptr<Socket> s, const Address &addr);
  void ServerHandleConnectionCreated2(Ptr<Socket> s, const Address &addr);
  void ServerHandleConnectionCreated3(Ptr<Socket> s, const Address &addr);
  void ServerHandleConnectionCreated4(Ptr<Socket> s, const Address &addr);

  Ptr<Socket> server1;
  Ptr<Socket> server2;
  Ptr<Socket> server3;
  Ptr<Socket> server4;

  Ptr<Socket> source1;
  Ptr<Socket> source2;
  Ptr<Socket> source3;
  Ptr<Socket> source4;

  Address receivedAddr1;
  Address receivedAddr2;
  Address receivedAddr3;
  Address receivedAddr4;
};

Ptr<Node> CreateDualStackNode() {
  Ptr<Node> node = CreateObject<Node>();

  Ptr<TrafficControlLayer> tc = CreateObject<TrafficControlLayer>();
  node->AggregateObject(tc);

  Ptr<ArpL3Protocol> arp = CreateObject<ArpL3Protocol>();
  node->AggregateObject(arp);
  arp->SetTrafficControl(tc);

  Ptr<Ipv4L3Protocol> ipv4 = CreateObject<Ipv4L3Protocol>();
  Ptr<Ipv4ListRouting> ipv4Routing = CreateObject<Ipv4ListRouting>();
  ipv4->SetRoutingProtocol(ipv4Routing);
  Ptr<Ipv4StaticRouting> ipv4staticRouting = CreateObject<Ipv4StaticRouting>();
  ipv4Routing->AddRoutingProtocol(ipv4staticRouting, 0);
  node->AggregateObject(ipv4);

  Ptr<Icmpv4L4Protocol> icmp = CreateObject<Icmpv4L4Protocol>();
  node->AggregateObject(icmp);

  Ptr<UdpL4Protocol> udp = CreateObject<UdpL4Protocol>();
  node->AggregateObject(udp);

  Ptr<TcpL4Protocol> tcp = CreateObject<TcpL4Protocol>();
  node->AggregateObject(tcp);

  Ptr<Ipv6L3Protocol> ipv6 = CreateObject<Ipv6L3Protocol>();

  Ptr<Ipv6ListRouting> ipv6Routing = CreateObject<Ipv6ListRouting>();
  ipv6->SetRoutingProtocol(ipv6Routing);
  Ptr<Ipv6StaticRouting> ipv6staticRouting = CreateObject<Ipv6StaticRouting>();
  ipv6Routing->AddRoutingProtocol(ipv6staticRouting, 0);
  node->AggregateObject(ipv6);

  Ptr<Icmpv6L4Protocol> icmp6 = CreateObject<Icmpv6L4Protocol>();
  node->AggregateObject(icmp6);

  ipv6->RegisterExtensions();
  ipv6->RegisterOptions();

  return node;
}

Ptr<SimpleNetDevice> AddSimpleNetDevice(Ptr<Node> node, Ipv4Address v4Addr,
                                        Ipv4Mask v4Mask, Ipv6Address v6Addr,
                                        Ipv6Prefix v6Prefix) {
  Ptr<SimpleNetDevice> dev = CreateObject<SimpleNetDevice>();
  dev->SetAddress(Mac48Address::ConvertFrom(Mac48Address::Allocate()));
  node->AddDevice(dev);

  Ptr<Ipv4> ipv4 = node->GetObject<Ipv4>();
  uint32_t ndid = ipv4->AddInterface(dev);
  Ipv4InterfaceAddress ipv4Addr = Ipv4InterfaceAddress(v4Addr, v4Mask);
  ipv4->AddAddress(ndid, ipv4Addr);
  ipv4->SetUp(ndid);

  Ptr<Ipv6> ipv6 = node->GetObject<Ipv6>();
  ndid = ipv6->AddInterface(dev);
  Ipv6InterfaceAddress ipv6Addr = Ipv6InterfaceAddress(v6Addr, v6Prefix);
  ipv6->AddAddress(ndid, ipv6Addr);
  ipv6->SetUp(ndid);

  return dev;
}

void DualStackTestCase::SetUpSim() {
  node0 = CreateDualStackNode();
  node1 = CreateDualStackNode();

  Ptr<SimpleNetDevice> dev0 =
      AddSimpleNetDevice(node0, Ipv4Address("10.0.0.1"), Ipv4Mask(0xffffff00),
                         Ipv6Address("2001::1"), Ipv6Prefix(64));
  Ptr<SimpleNetDevice> dev1 =
      AddSimpleNetDevice(node1, Ipv4Address("10.0.0.2"), Ipv4Mask(0xffffff00),
                         Ipv6Address("2001::2"), Ipv6Prefix(64));

  Ptr<SimpleChannel> channel = CreateObject<SimpleChannel>();
  dev0->SetChannel(channel);
  dev1->SetChannel(channel);

  Ptr<SocketFactory> sockFactory0 = node0->GetObject<TcpSocketFactory>();
  Ptr<SocketFactory> sockFactory1 = node1->GetObject<TcpSocketFactory>();

  server1 = sockFactory0->CreateSocket();
  server2 = sockFactory0->CreateSocket();
  server3 = sockFactory0->CreateSocket();
  server4 = sockFactory0->CreateSocket();

  source1 = sockFactory1->CreateSocket();
  source2 = sockFactory1->CreateSocket();
  source3 = sockFactory1->CreateSocket();
  source4 = sockFactory1->CreateSocket();
}

void DualStackTestCase::ServerHandleConnectionCreated1(Ptr<Socket> s,
                                                       const Address &addr) {
  receivedAddr1 = addr;
}

void DualStackTestCase::ServerHandleConnectionCreated2(Ptr<Socket> s,
                                                       const Address &addr) {
  receivedAddr2 = addr;
}

void DualStackTestCase::ServerHandleConnectionCreated3(Ptr<Socket> s,
                                                       const Address &addr) {
  receivedAddr3 = addr;
}

void DualStackTestCase::ServerHandleConnectionCreated4(Ptr<Socket> s,
                                                       const Address &addr) {
  receivedAddr4 = addr;
}

DualStackTestCase::DualStackTestCase() : TestCase("DualStackTestCase") {
  receivedAddr1 = Address();
  receivedAddr2 = Address();
  receivedAddr3 = Address();
  receivedAddr4 = Address();
}

void DualStackTestCase::DoRun() {
  SetUpSim();

  uint16_t port1 = 5000;
  uint16_t port2 = 5001;
  uint16_t port3 = 5002;
  uint16_t port4 = 5003;

  server1->Bind(InetSocketAddress(Ipv4Address::GetAny(), port1));
  server1->Listen();
  server1->SetAcceptCallback(
      MakeNullCallback<bool, Ptr<Socket>, const Address &>(),
      MakeCallback(&DualStackTestCase::ServerHandleConnectionCreated1, this));

  server2->Bind(InetSocketAddress(Ipv4Address::GetAny(), port2));
  server2->Listen();
  server2->SetAcceptCallback(
      MakeNullCallback<bool, Ptr<Socket>, const Address &>(),
      MakeCallback(&DualStackTestCase::ServerHandleConnectionCreated2, this));

  server3->Bind(Inet6SocketAddress(Ipv6Address::GetAny(), port3));
  server3->Listen();
  server3->SetAcceptCallback(
      MakeNullCallback<bool, Ptr<Socket>, const Address &>(),
      MakeCallback(&DualStackTestCase::ServerHandleConnectionCreated3, this));

  server4->Bind(Inet6SocketAddress(Ipv6Address::GetAny(), port4));
  server4->Listen();
  server4->SetAcceptCallback(
      MakeNullCallback<bool, Ptr<Socket>, const Address &>(),
      MakeCallback(&DualStackTestCase::ServerHandleConnectionCreated4, this));

  source1->Connect(InetSocketAddress(Ipv4Address("10.0.0.1"), port1));

  source2->Connect(Inet6SocketAddress(Ipv6Address("2001::1"), port2));

  source3->Connect(InetSocketAddress(Ipv4Address("10.0.0.1"), port3));

  source4->Connect(Inet6SocketAddress(Ipv6Address("2001::1"), port4));

  Simulator::Run();

  NS_TEST_EXPECT_MSG_EQ(InetSocketAddress::IsMatchingType(receivedAddr1), true,
                        "Accepted address is of proper type");
  NS_TEST_EXPECT_MSG_EQ(InetSocketAddress::ConvertFrom(receivedAddr1).GetIpv4(),
                        Ipv4Address("10.0.0.2"), "Accepted address is correct");

  NS_TEST_EXPECT_MSG_EQ(receivedAddr2.IsInvalid(), true,
                        "IPv4 socket correctly ignored IPv6 connection");

  NS_TEST_EXPECT_MSG_EQ(Inet6SocketAddress::IsMatchingType(receivedAddr3), true,
                        "Accepted address is of proper type");
  NS_TEST_EXPECT_MSG_EQ(
      Inet6SocketAddress::ConvertFrom(receivedAddr3).GetIpv6(),
      Ipv6Address("::ffff:0a00:0002"), "Accepted address is correct");

  NS_TEST_EXPECT_MSG_EQ(Inet6SocketAddress::IsMatchingType(receivedAddr4), true,
                        "Accepted address is of proper type");
  NS_TEST_EXPECT_MSG_EQ(
      Inet6SocketAddress::ConvertFrom(receivedAddr4).GetIpv6(),
      Ipv6Address("2001::2"), "Accepted address is correct");
}

void DualStackTestCase::DoTeardown() { Simulator::Destroy(); }

class Ipv6DualStackTestSuite : public TestSuite {
public:
  Ipv6DualStackTestSuite() : TestSuite("ipv6-dual-stack", UNIT) {
    AddTestCase(new DualStackTestCase(), TestCase::QUICK);
  }
};

static Ipv6DualStackTestSuite g_ipv6DualStackTestSuite;
