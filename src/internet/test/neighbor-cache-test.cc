
#include "ns3/icmpv4-l4-protocol.h"
#include "ns3/icmpv6-l4-protocol.h"
#include "ns3/internet-stack-helper.h"
#include "ns3/ipv4-address-helper.h"
#include "ns3/ipv4-l3-protocol.h"
#include "ns3/ipv4-routing-helper.h"
#include "ns3/ipv6-address-helper.h"
#include "ns3/ipv6-l3-protocol.h"
#include "ns3/ipv6-routing-helper.h"
#include "ns3/neighbor-cache-helper.h"
#include "ns3/simple-channel.h"
#include "ns3/simple-net-device-helper.h"
#include "ns3/simple-net-device.h"
#include "ns3/simulator.h"
#include "ns3/socket-factory.h"
#include "ns3/socket.h"
#include "ns3/test.h"
#include "ns3/udp-l4-protocol.h"
#include "ns3/udp-socket-factory.h"

using namespace ns3;

class DynamicNeighborCacheTest : public TestCase {
  Ptr<Packet> m_receivedPacket;

  void DoSendDatav4(Ptr<Socket> socket, Ipv4Address to);

  void DoSendDatav6(Ptr<Socket> socket, Ipv6Address to);

  void SendData(Ptr<Socket> socket, Ipv4Address to);

  void SendData(Ptr<Socket> socket, Ipv6Address to);

  void AddIpv4Address(Ptr<Ipv4Interface> ipv4Interface,
                      Ipv4InterfaceAddress ifaceAddr);

  void AddIpv6Address(Ptr<Ipv6Interface> ipv6Interface,
                      Ipv6InterfaceAddress ifaceAddr);

  void RemoveIpv4Address(Ptr<Ipv4Interface> ipv4Interface, uint32_t index);

  void RemoveIpv6Address(Ptr<Ipv6Interface> ipv6Interface, uint32_t index);

public:
  void DoRun() override;

  DynamicNeighborCacheTest();

  void ReceivePkt(Ptr<Socket> socket);

  std::vector<uint32_t> m_receivedPacketSizes;
};

DynamicNeighborCacheTest::DynamicNeighborCacheTest()
    : TestCase("The DynamicNeighborCacheTestPopulate checks if neighbor caches "
               "are correctly populated "
               "in global scope and updated when there is an IP address added "
               "or removed.") {}

void DynamicNeighborCacheTest::ReceivePkt(Ptr<Socket> socket) {
  uint32_t availableData [[maybe_unused]] = socket->GetRxAvailable();
  m_receivedPacket = socket->Recv(std::numeric_limits<uint32_t>::max(), 0);
  NS_TEST_ASSERT_MSG_EQ(
      availableData, m_receivedPacket->GetSize(),
      "Received Packet size is not equal to the Rx buffer size");
  m_receivedPacketSizes.push_back(m_receivedPacket->GetSize());
}

void DynamicNeighborCacheTest::DoSendDatav4(Ptr<Socket> socket,
                                            Ipv4Address to) {
  Address realTo = InetSocketAddress(to, 1234);
  NS_TEST_EXPECT_MSG_EQ(socket->SendTo(Create<Packet>(123), 0, realTo), 123,
                        "100");
}

void DynamicNeighborCacheTest::DoSendDatav6(Ptr<Socket> socket,
                                            Ipv6Address to) {
  Address realTo = Inet6SocketAddress(to, 1234);
  NS_TEST_EXPECT_MSG_EQ(socket->SendTo(Create<Packet>(123), 0, realTo), 123,
                        "100");
}

void DynamicNeighborCacheTest::SendData(Ptr<Socket> socket, Ipv4Address to) {
  m_receivedPacket = Create<Packet>();
  Simulator::ScheduleWithContext(socket->GetNode()->GetId(), Seconds(60),
                                 &DynamicNeighborCacheTest::DoSendDatav4, this,
                                 socket, to);
}

void DynamicNeighborCacheTest::SendData(Ptr<Socket> socket, Ipv6Address to) {
  m_receivedPacket = Create<Packet>();
  Simulator::ScheduleWithContext(socket->GetNode()->GetId(), Seconds(60),
                                 &DynamicNeighborCacheTest::DoSendDatav6, this,
                                 socket, to);
}

void DynamicNeighborCacheTest::AddIpv4Address(Ptr<Ipv4Interface> ipv4Interface,
                                              Ipv4InterfaceAddress ifaceAddr) {
  ipv4Interface->AddAddress(ifaceAddr);
}

void DynamicNeighborCacheTest::AddIpv6Address(Ptr<Ipv6Interface> ipv6Interface,
                                              Ipv6InterfaceAddress ifaceAddr) {
  ipv6Interface->AddAddress(ifaceAddr);
}

void DynamicNeighborCacheTest::RemoveIpv4Address(
    Ptr<Ipv4Interface> ipv4Interface, uint32_t index) {
  ipv4Interface->RemoveAddress(index);
}

void DynamicNeighborCacheTest::RemoveIpv6Address(
    Ptr<Ipv6Interface> ipv6Interface, uint32_t index) {
  ipv6Interface->RemoveAddress(index);
}

void DynamicNeighborCacheTest::DoRun() {
  Ptr<Node> tx1Node = CreateObject<Node>();
  Ptr<Node> tx2Node = CreateObject<Node>();
  Ptr<Node> rxNode = CreateObject<Node>();
  Ptr<Node> snifferNode = CreateObject<Node>();

  NodeContainer all(tx1Node, tx2Node, rxNode, snifferNode);

  std::ostringstream stringStream1v4;
  Ptr<OutputStreamWrapper> arpStream =
      Create<OutputStreamWrapper>(&stringStream1v4);
  std::ostringstream stringStream1v6;
  Ptr<OutputStreamWrapper> ndiscStream =
      Create<OutputStreamWrapper>(&stringStream1v6);

  InternetStackHelper internetNodes;
  internetNodes.Install(all);

  NetDeviceContainer net;
  Ptr<SimpleNetDevice> tx1Dev;
  {
    tx1Dev = CreateObject<SimpleNetDevice>();
    tx1Dev->SetAddress(Mac48Address("00:00:00:00:00:01"));
    tx1Node->AddDevice(tx1Dev);
  }
  net.Add(tx1Dev);

  Ptr<SimpleNetDevice> tx2Dev;
  {
    tx2Dev = CreateObject<SimpleNetDevice>();
    tx2Dev->SetAddress(Mac48Address("00:00:00:00:00:02"));
    tx2Node->AddDevice(tx2Dev);
  }
  net.Add(tx2Dev);

  Ptr<SimpleNetDevice> rxDev;
  {
    rxDev = CreateObject<SimpleNetDevice>();
    rxDev->SetAddress(Mac48Address("00:00:00:00:00:03"));
    rxNode->AddDevice(rxDev);
  }
  net.Add(rxDev);

  Ptr<SimpleNetDevice> snifferDev;
  {
    snifferDev = CreateObject<SimpleNetDevice>();
    snifferDev->SetAddress(Mac48Address("00:00:00:00:00:04"));
    snifferNode->AddDevice(snifferDev);
  }
  net.Add(snifferDev);

  Ptr<SimpleChannel> channel = CreateObject<SimpleChannel>();
  tx1Dev->SetChannel(channel);
  tx2Dev->SetChannel(channel);
  rxDev->SetChannel(channel);
  snifferDev->SetChannel(channel);

  Ipv4AddressHelper ipv4;
  ipv4.SetBase(Ipv4Address("10.0.1.0"), Ipv4Mask("255.255.255.0"));
  Ipv4InterfaceContainer icv4 = ipv4.Assign(net);

  Ptr<Node> n1 = rxNode;
  uint32_t ipv4ifIndex = 1;
  Ptr<Ipv4Interface> ipv4Interface =
      n1->GetObject<Ipv4L3Protocol>()->GetInterface(ipv4ifIndex);
  Ipv4InterfaceAddress ifaceAddr =
      Ipv4InterfaceAddress("10.0.1.5", "255.255.255.0");
  Simulator::Schedule(Seconds(0.5), &DynamicNeighborCacheTest::AddIpv4Address,
                      this, ipv4Interface, ifaceAddr);

  uint32_t addressIndex = 0;
  Simulator::Schedule(Seconds(1.5),
                      &DynamicNeighborCacheTest::RemoveIpv4Address, this,
                      ipv4Interface, addressIndex);

  Ipv6AddressHelper ipv6;
  ipv6.SetBase(Ipv6Address("2001:0::"), Ipv6Prefix(64));
  Ipv6InterfaceContainer icv6 = ipv6.Assign(net);

  uint32_t ipv6ifIndex = 1;
  Ptr<Ipv6Interface> ipv6Interface =
      n1->GetObject<Ipv6L3Protocol>()->GetInterface(ipv6ifIndex);
  Ipv6InterfaceAddress ifaceAddrv6 =
      Ipv6InterfaceAddress("2001:0::200:ff:fe00:5", Ipv6Prefix(64));
  Simulator::Schedule(Seconds(0.5), &DynamicNeighborCacheTest::AddIpv6Address,
                      this, ipv6Interface, ifaceAddrv6);

  addressIndex = 1;
  Simulator::Schedule(Seconds(1.5),
                      &DynamicNeighborCacheTest::RemoveIpv6Address, this,
                      ipv6Interface, addressIndex);

  NeighborCacheHelper neighborCache;
  neighborCache.SetDynamicNeighborCache(true);
  neighborCache.PopulateNeighborCache();

  Ipv4RoutingHelper::PrintNeighborCacheAllAt(Seconds(0), arpStream);
  Ipv6RoutingHelper::PrintNeighborCacheAllAt(Seconds(0), ndiscStream);
  Ipv4RoutingHelper::PrintNeighborCacheAllAt(Seconds(1), arpStream);
  Ipv6RoutingHelper::PrintNeighborCacheAllAt(Seconds(1), ndiscStream);
  Ipv4RoutingHelper::PrintNeighborCacheAllAt(Seconds(2), arpStream);
  Ipv6RoutingHelper::PrintNeighborCacheAllAt(Seconds(2), ndiscStream);

  Ptr<SocketFactory> rxSocketFactory = rxNode->GetObject<UdpSocketFactory>();
  Ptr<Socket> rxSocketv4 = rxSocketFactory->CreateSocket();
  Ptr<Socket> rxSocketv6 = rxSocketFactory->CreateSocket();
  NS_TEST_EXPECT_MSG_EQ(
      rxSocketv4->Bind(InetSocketAddress(Ipv4Address("10.0.1.5"), 1234)), 0,
      "trivial");
  NS_TEST_EXPECT_MSG_EQ(rxSocketv6->Bind(Inet6SocketAddress(
                            Ipv6Address("2001:0::200:ff:fe00:5"), 1234)),
                        0, "trivial");
  rxSocketv4->SetRecvCallback(
      MakeCallback(&DynamicNeighborCacheTest::ReceivePkt, this));
  rxSocketv6->SetRecvCallback(
      MakeCallback(&DynamicNeighborCacheTest::ReceivePkt, this));

  Ptr<SocketFactory> snifferSocketFactory =
      snifferNode->GetObject<UdpSocketFactory>();
  Ptr<Socket> snifferSocketv4 = snifferSocketFactory->CreateSocket();
  Ptr<Socket> snifferSocketv6 = snifferSocketFactory->CreateSocket();
  NS_TEST_EXPECT_MSG_EQ(
      snifferSocketv4->Bind(InetSocketAddress(Ipv4Address("10.0.1.4"), 1234)),
      0, "trivial");
  NS_TEST_EXPECT_MSG_EQ(snifferSocketv6->Bind(Inet6SocketAddress(
                            Ipv6Address("2001:0::200:ff:fe00:4"), 1234)),
                        0, "trivial");
  snifferSocketv4->SetRecvCallback(
      MakeCallback(&DynamicNeighborCacheTest::ReceivePkt, this));
  snifferSocketv6->SetRecvCallback(
      MakeCallback(&DynamicNeighborCacheTest::ReceivePkt, this));

  Ptr<SocketFactory> tx1SocketFactory = tx1Node->GetObject<UdpSocketFactory>();
  Ptr<Socket> tx1Socket = tx1SocketFactory->CreateSocket();
  tx1Socket->SetAllowBroadcast(true);

  Ptr<SocketFactory> tx2SocketFactory = tx2Node->GetObject<UdpSocketFactory>();
  Ptr<Socket> tx2Socket = tx2SocketFactory->CreateSocket();
  tx2Socket->SetAllowBroadcast(true);

  SendData(tx1Socket, Ipv4Address("10.0.1.5"));
  SendData(tx1Socket, Ipv6Address("2001:0::200:ff:fe00:5"));

  SendData(tx1Socket, Ipv4Address("10.0.1.3"));
  SendData(tx1Socket, Ipv6Address("2001:0::200:ff:fe00:3"));

  Simulator::Stop(Seconds(66));
  Simulator::Run();

  NS_TEST_EXPECT_MSG_EQ(
      m_receivedPacketSizes[0], 123,
      "Should receive packet sending to the added IPv4 address.");
  NS_TEST_EXPECT_MSG_EQ(
      m_receivedPacketSizes[1], 123,
      "Should receive packet sending to the added IPv6 address.");
  NS_TEST_EXPECT_MSG_EQ(m_receivedPacketSizes.size(), 2,
                        "Should receive only 1 packet from IPv4 interface and "
                        "only 1 packet from IPv6 interface.");

  constexpr auto arpCache =
      "ARP Cache of node 0 at time 0\n"
      "10.0.1.2 dev 1 lladdr 04-06-00:00:00:00:00:02 STATIC_AUTOGENERATED\n"
      "10.0.1.3 dev 1 lladdr 04-06-00:00:00:00:00:03 STATIC_AUTOGENERATED\n"
      "10.0.1.4 dev 1 lladdr 04-06-00:00:00:00:00:04 STATIC_AUTOGENERATED\n"
      "ARP Cache of node 1 at time 0\n"
      "10.0.1.1 dev 1 lladdr 04-06-00:00:00:00:00:01 STATIC_AUTOGENERATED\n"
      "10.0.1.3 dev 1 lladdr 04-06-00:00:00:00:00:03 STATIC_AUTOGENERATED\n"
      "10.0.1.4 dev 1 lladdr 04-06-00:00:00:00:00:04 STATIC_AUTOGENERATED\n"
      "ARP Cache of node 2 at time 0\n"
      "10.0.1.1 dev 1 lladdr 04-06-00:00:00:00:00:01 STATIC_AUTOGENERATED\n"
      "10.0.1.2 dev 1 lladdr 04-06-00:00:00:00:00:02 STATIC_AUTOGENERATED\n"
      "10.0.1.4 dev 1 lladdr 04-06-00:00:00:00:00:04 STATIC_AUTOGENERATED\n"
      "ARP Cache of node 3 at time 0\n"
      "10.0.1.1 dev 1 lladdr 04-06-00:00:00:00:00:01 STATIC_AUTOGENERATED\n"
      "10.0.1.2 dev 1 lladdr 04-06-00:00:00:00:00:02 STATIC_AUTOGENERATED\n"
      "10.0.1.3 dev 1 lladdr 04-06-00:00:00:00:00:03 STATIC_AUTOGENERATED\n"
      "ARP Cache of node 0 at time 1\n"
      "10.0.1.2 dev 1 lladdr 04-06-00:00:00:00:00:02 STATIC_AUTOGENERATED\n"
      "10.0.1.3 dev 1 lladdr 04-06-00:00:00:00:00:03 STATIC_AUTOGENERATED\n"
      "10.0.1.4 dev 1 lladdr 04-06-00:00:00:00:00:04 STATIC_AUTOGENERATED\n"
      "10.0.1.5 dev 1 lladdr 04-06-00:00:00:00:00:03 STATIC_AUTOGENERATED\n"
      "ARP Cache of node 1 at time 1\n"
      "10.0.1.1 dev 1 lladdr 04-06-00:00:00:00:00:01 STATIC_AUTOGENERATED\n"
      "10.0.1.3 dev 1 lladdr 04-06-00:00:00:00:00:03 STATIC_AUTOGENERATED\n"
      "10.0.1.4 dev 1 lladdr 04-06-00:00:00:00:00:04 STATIC_AUTOGENERATED\n"
      "10.0.1.5 dev 1 lladdr 04-06-00:00:00:00:00:03 STATIC_AUTOGENERATED\n"
      "ARP Cache of node 2 at time 1\n"
      "10.0.1.1 dev 1 lladdr 04-06-00:00:00:00:00:01 STATIC_AUTOGENERATED\n"
      "10.0.1.2 dev 1 lladdr 04-06-00:00:00:00:00:02 STATIC_AUTOGENERATED\n"
      "10.0.1.4 dev 1 lladdr 04-06-00:00:00:00:00:04 STATIC_AUTOGENERATED\n"
      "ARP Cache of node 3 at time 1\n"
      "10.0.1.1 dev 1 lladdr 04-06-00:00:00:00:00:01 STATIC_AUTOGENERATED\n"
      "10.0.1.2 dev 1 lladdr 04-06-00:00:00:00:00:02 STATIC_AUTOGENERATED\n"
      "10.0.1.3 dev 1 lladdr 04-06-00:00:00:00:00:03 STATIC_AUTOGENERATED\n"
      "10.0.1.5 dev 1 lladdr 04-06-00:00:00:00:00:03 STATIC_AUTOGENERATED\n"
      "ARP Cache of node 0 at time 2\n"
      "10.0.1.2 dev 1 lladdr 04-06-00:00:00:00:00:02 STATIC_AUTOGENERATED\n"
      "10.0.1.4 dev 1 lladdr 04-06-00:00:00:00:00:04 STATIC_AUTOGENERATED\n"
      "10.0.1.5 dev 1 lladdr 04-06-00:00:00:00:00:03 STATIC_AUTOGENERATED\n"
      "ARP Cache of node 1 at time 2\n"
      "10.0.1.1 dev 1 lladdr 04-06-00:00:00:00:00:01 STATIC_AUTOGENERATED\n"
      "10.0.1.4 dev 1 lladdr 04-06-00:00:00:00:00:04 STATIC_AUTOGENERATED\n"
      "10.0.1.5 dev 1 lladdr 04-06-00:00:00:00:00:03 STATIC_AUTOGENERATED\n"
      "ARP Cache of node 2 at time 2\n"
      "10.0.1.1 dev 1 lladdr 04-06-00:00:00:00:00:01 STATIC_AUTOGENERATED\n"
      "10.0.1.2 dev 1 lladdr 04-06-00:00:00:00:00:02 STATIC_AUTOGENERATED\n"
      "10.0.1.4 dev 1 lladdr 04-06-00:00:00:00:00:04 STATIC_AUTOGENERATED\n"
      "ARP Cache of node 3 at time 2\n"
      "10.0.1.1 dev 1 lladdr 04-06-00:00:00:00:00:01 STATIC_AUTOGENERATED\n"
      "10.0.1.2 dev 1 lladdr 04-06-00:00:00:00:00:02 STATIC_AUTOGENERATED\n"
      "10.0.1.5 dev 1 lladdr 04-06-00:00:00:00:00:03 STATIC_AUTOGENERATED\n";
  NS_TEST_EXPECT_MSG_EQ(stringStream1v4.str(), arpCache,
                        "Arp cache is incorrect.");

  constexpr auto NdiscCache = "NDISC Cache of node 0 at time +0s\n"
                              "2001::200:ff:fe00:2 dev 1 lladdr "
                              "04-06-00:00:00:00:00:02 STATIC_AUTOGENERATED\n"
                              "2001::200:ff:fe00:3 dev 1 lladdr "
                              "04-06-00:00:00:00:00:03 STATIC_AUTOGENERATED\n"
                              "2001::200:ff:fe00:4 dev 1 lladdr "
                              "04-06-00:00:00:00:00:04 STATIC_AUTOGENERATED\n"
                              "fe80::200:ff:fe00:2 dev 1 lladdr "
                              "04-06-00:00:00:00:00:02 STATIC_AUTOGENERATED\n"
                              "fe80::200:ff:fe00:3 dev 1 lladdr "
                              "04-06-00:00:00:00:00:03 STATIC_AUTOGENERATED\n"
                              "fe80::200:ff:fe00:4 dev 1 lladdr "
                              "04-06-00:00:00:00:00:04 STATIC_AUTOGENERATED\n"
                              "NDISC Cache of node 1 at time +0s\n"
                              "2001::200:ff:fe00:1 dev 1 lladdr "
                              "04-06-00:00:00:00:00:01 STATIC_AUTOGENERATED\n"
                              "2001::200:ff:fe00:3 dev 1 lladdr "
                              "04-06-00:00:00:00:00:03 STATIC_AUTOGENERATED\n"
                              "2001::200:ff:fe00:4 dev 1 lladdr "
                              "04-06-00:00:00:00:00:04 STATIC_AUTOGENERATED\n"
                              "fe80::200:ff:fe00:1 dev 1 lladdr "
                              "04-06-00:00:00:00:00:01 STATIC_AUTOGENERATED\n"
                              "fe80::200:ff:fe00:3 dev 1 lladdr "
                              "04-06-00:00:00:00:00:03 STATIC_AUTOGENERATED\n"
                              "fe80::200:ff:fe00:4 dev 1 lladdr "
                              "04-06-00:00:00:00:00:04 STATIC_AUTOGENERATED\n"
                              "NDISC Cache of node 2 at time +0s\n"
                              "2001::200:ff:fe00:1 dev 1 lladdr "
                              "04-06-00:00:00:00:00:01 STATIC_AUTOGENERATED\n"
                              "2001::200:ff:fe00:2 dev 1 lladdr "
                              "04-06-00:00:00:00:00:02 STATIC_AUTOGENERATED\n"
                              "2001::200:ff:fe00:4 dev 1 lladdr "
                              "04-06-00:00:00:00:00:04 STATIC_AUTOGENERATED\n"
                              "fe80::200:ff:fe00:1 dev 1 lladdr "
                              "04-06-00:00:00:00:00:01 STATIC_AUTOGENERATED\n"
                              "fe80::200:ff:fe00:2 dev 1 lladdr "
                              "04-06-00:00:00:00:00:02 STATIC_AUTOGENERATED\n"
                              "fe80::200:ff:fe00:4 dev 1 lladdr "
                              "04-06-00:00:00:00:00:04 STATIC_AUTOGENERATED\n"
                              "NDISC Cache of node 3 at time +0s\n"
                              "2001::200:ff:fe00:1 dev 1 lladdr "
                              "04-06-00:00:00:00:00:01 STATIC_AUTOGENERATED\n"
                              "2001::200:ff:fe00:2 dev 1 lladdr "
                              "04-06-00:00:00:00:00:02 STATIC_AUTOGENERATED\n"
                              "2001::200:ff:fe00:3 dev 1 lladdr "
                              "04-06-00:00:00:00:00:03 STATIC_AUTOGENERATED\n"
                              "fe80::200:ff:fe00:1 dev 1 lladdr "
                              "04-06-00:00:00:00:00:01 STATIC_AUTOGENERATED\n"
                              "fe80::200:ff:fe00:2 dev 1 lladdr "
                              "04-06-00:00:00:00:00:02 STATIC_AUTOGENERATED\n"
                              "fe80::200:ff:fe00:3 dev 1 lladdr "
                              "04-06-00:00:00:00:00:03 STATIC_AUTOGENERATED\n"
                              "NDISC Cache of node 0 at time +1s\n"
                              "2001::200:ff:fe00:2 dev 1 lladdr "
                              "04-06-00:00:00:00:00:02 STATIC_AUTOGENERATED\n"
                              "2001::200:ff:fe00:3 dev 1 lladdr "
                              "04-06-00:00:00:00:00:03 STATIC_AUTOGENERATED\n"
                              "2001::200:ff:fe00:4 dev 1 lladdr "
                              "04-06-00:00:00:00:00:04 STATIC_AUTOGENERATED\n"
                              "2001::200:ff:fe00:5 dev 1 lladdr "
                              "04-06-00:00:00:00:00:03 STATIC_AUTOGENERATED\n"
                              "fe80::200:ff:fe00:2 dev 1 lladdr "
                              "04-06-00:00:00:00:00:02 STATIC_AUTOGENERATED\n"
                              "fe80::200:ff:fe00:3 dev 1 lladdr "
                              "04-06-00:00:00:00:00:03 STATIC_AUTOGENERATED\n"
                              "fe80::200:ff:fe00:4 dev 1 lladdr "
                              "04-06-00:00:00:00:00:04 STATIC_AUTOGENERATED\n"
                              "NDISC Cache of node 1 at time +1s\n"
                              "2001::200:ff:fe00:1 dev 1 lladdr "
                              "04-06-00:00:00:00:00:01 STATIC_AUTOGENERATED\n"
                              "2001::200:ff:fe00:3 dev 1 lladdr "
                              "04-06-00:00:00:00:00:03 STATIC_AUTOGENERATED\n"
                              "2001::200:ff:fe00:4 dev 1 lladdr "
                              "04-06-00:00:00:00:00:04 STATIC_AUTOGENERATED\n"
                              "2001::200:ff:fe00:5 dev 1 lladdr "
                              "04-06-00:00:00:00:00:03 STATIC_AUTOGENERATED\n"
                              "fe80::200:ff:fe00:1 dev 1 lladdr "
                              "04-06-00:00:00:00:00:01 STATIC_AUTOGENERATED\n"
                              "fe80::200:ff:fe00:3 dev 1 lladdr "
                              "04-06-00:00:00:00:00:03 STATIC_AUTOGENERATED\n"
                              "fe80::200:ff:fe00:4 dev 1 lladdr "
                              "04-06-00:00:00:00:00:04 STATIC_AUTOGENERATED\n"
                              "NDISC Cache of node 2 at time +1s\n"
                              "2001::200:ff:fe00:1 dev 1 lladdr "
                              "04-06-00:00:00:00:00:01 STATIC_AUTOGENERATED\n"
                              "2001::200:ff:fe00:2 dev 1 lladdr "
                              "04-06-00:00:00:00:00:02 STATIC_AUTOGENERATED\n"
                              "2001::200:ff:fe00:4 dev 1 lladdr "
                              "04-06-00:00:00:00:00:04 STATIC_AUTOGENERATED\n"
                              "fe80::200:ff:fe00:1 dev 1 lladdr "
                              "04-06-00:00:00:00:00:01 STATIC_AUTOGENERATED\n"
                              "fe80::200:ff:fe00:2 dev 1 lladdr "
                              "04-06-00:00:00:00:00:02 STATIC_AUTOGENERATED\n"
                              "fe80::200:ff:fe00:4 dev 1 lladdr "
                              "04-06-00:00:00:00:00:04 STATIC_AUTOGENERATED\n"
                              "NDISC Cache of node 3 at time +1s\n"
                              "2001::200:ff:fe00:1 dev 1 lladdr "
                              "04-06-00:00:00:00:00:01 STATIC_AUTOGENERATED\n"
                              "2001::200:ff:fe00:2 dev 1 lladdr "
                              "04-06-00:00:00:00:00:02 STATIC_AUTOGENERATED\n"
                              "2001::200:ff:fe00:3 dev 1 lladdr "
                              "04-06-00:00:00:00:00:03 STATIC_AUTOGENERATED\n"
                              "2001::200:ff:fe00:5 dev 1 lladdr "
                              "04-06-00:00:00:00:00:03 STATIC_AUTOGENERATED\n"
                              "fe80::200:ff:fe00:1 dev 1 lladdr "
                              "04-06-00:00:00:00:00:01 STATIC_AUTOGENERATED\n"
                              "fe80::200:ff:fe00:2 dev 1 lladdr "
                              "04-06-00:00:00:00:00:02 STATIC_AUTOGENERATED\n"
                              "fe80::200:ff:fe00:3 dev 1 lladdr "
                              "04-06-00:00:00:00:00:03 STATIC_AUTOGENERATED\n"
                              "NDISC Cache of node 0 at time +2s\n"
                              "2001::200:ff:fe00:2 dev 1 lladdr "
                              "04-06-00:00:00:00:00:02 STATIC_AUTOGENERATED\n"
                              "2001::200:ff:fe00:4 dev 1 lladdr "
                              "04-06-00:00:00:00:00:04 STATIC_AUTOGENERATED\n"
                              "2001::200:ff:fe00:5 dev 1 lladdr "
                              "04-06-00:00:00:00:00:03 STATIC_AUTOGENERATED\n"
                              "fe80::200:ff:fe00:2 dev 1 lladdr "
                              "04-06-00:00:00:00:00:02 STATIC_AUTOGENERATED\n"
                              "fe80::200:ff:fe00:3 dev 1 lladdr "
                              "04-06-00:00:00:00:00:03 STATIC_AUTOGENERATED\n"
                              "fe80::200:ff:fe00:4 dev 1 lladdr "
                              "04-06-00:00:00:00:00:04 STATIC_AUTOGENERATED\n"
                              "NDISC Cache of node 1 at time +2s\n"
                              "2001::200:ff:fe00:1 dev 1 lladdr "
                              "04-06-00:00:00:00:00:01 STATIC_AUTOGENERATED\n"
                              "2001::200:ff:fe00:4 dev 1 lladdr "
                              "04-06-00:00:00:00:00:04 STATIC_AUTOGENERATED\n"
                              "2001::200:ff:fe00:5 dev 1 lladdr "
                              "04-06-00:00:00:00:00:03 STATIC_AUTOGENERATED\n"
                              "fe80::200:ff:fe00:1 dev 1 lladdr "
                              "04-06-00:00:00:00:00:01 STATIC_AUTOGENERATED\n"
                              "fe80::200:ff:fe00:3 dev 1 lladdr "
                              "04-06-00:00:00:00:00:03 STATIC_AUTOGENERATED\n"
                              "fe80::200:ff:fe00:4 dev 1 lladdr "
                              "04-06-00:00:00:00:00:04 STATIC_AUTOGENERATED\n"
                              "NDISC Cache of node 2 at time +2s\n"
                              "2001::200:ff:fe00:1 dev 1 lladdr "
                              "04-06-00:00:00:00:00:01 STATIC_AUTOGENERATED\n"
                              "2001::200:ff:fe00:2 dev 1 lladdr "
                              "04-06-00:00:00:00:00:02 STATIC_AUTOGENERATED\n"
                              "2001::200:ff:fe00:4 dev 1 lladdr "
                              "04-06-00:00:00:00:00:04 STATIC_AUTOGENERATED\n"
                              "fe80::200:ff:fe00:1 dev 1 lladdr "
                              "04-06-00:00:00:00:00:01 STATIC_AUTOGENERATED\n"
                              "fe80::200:ff:fe00:2 dev 1 lladdr "
                              "04-06-00:00:00:00:00:02 STATIC_AUTOGENERATED\n"
                              "fe80::200:ff:fe00:4 dev 1 lladdr "
                              "04-06-00:00:00:00:00:04 STATIC_AUTOGENERATED\n"
                              "NDISC Cache of node 3 at time +2s\n"
                              "2001::200:ff:fe00:1 dev 1 lladdr "
                              "04-06-00:00:00:00:00:01 STATIC_AUTOGENERATED\n"
                              "2001::200:ff:fe00:2 dev 1 lladdr "
                              "04-06-00:00:00:00:00:02 STATIC_AUTOGENERATED\n"
                              "2001::200:ff:fe00:5 dev 1 lladdr "
                              "04-06-00:00:00:00:00:03 STATIC_AUTOGENERATED\n"
                              "fe80::200:ff:fe00:1 dev 1 lladdr "
                              "04-06-00:00:00:00:00:01 STATIC_AUTOGENERATED\n"
                              "fe80::200:ff:fe00:2 dev 1 lladdr "
                              "04-06-00:00:00:00:00:02 STATIC_AUTOGENERATED\n"
                              "fe80::200:ff:fe00:3 dev 1 lladdr "
                              "04-06-00:00:00:00:00:03 STATIC_AUTOGENERATED\n";
  NS_TEST_EXPECT_MSG_EQ(stringStream1v6.str(), NdiscCache,
                        "Ndisc cache is incorrect.");

  m_receivedPacket->RemoveAllByteTags();

  Simulator::Destroy();
}

class ChannelTest : public TestCase {
public:
  void DoRun() override;
  ChannelTest();

private:
  NodeContainer m_nodes;
};

ChannelTest::ChannelTest()
    : TestCase("The ChannelTest Check if neighbor caches are correctly "
               "populated on specific channel.") {}

void ChannelTest::DoRun() {
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
  internet.Install(m_nodes);

  Ipv4AddressHelper ipv4;
  ipv4.SetBase("10.1.1.0", "255.255.255.252");
  Ipv4InterfaceContainer i = ipv4.Assign(net);
  ipv4.SetBase("10.1.2.0", "255.255.255.252");
  Ipv4InterfaceContainer i2 = ipv4.Assign(net2);

  Ipv6AddressHelper ipv6;
  ipv6.SetBase(Ipv6Address("2001:0::"), Ipv6Prefix(64));
  Ipv6InterfaceContainer icv61 = ipv6.Assign(net);
  ipv6.SetBase(Ipv6Address("2001:1::"), Ipv6Prefix(64));
  Ipv6InterfaceContainer icv62 = ipv6.Assign(net2);

  NeighborCacheHelper neighborCache;
  neighborCache.PopulateNeighborCache(channel);

  std::ostringstream stringStream1v4;
  Ptr<OutputStreamWrapper> arpStream =
      Create<OutputStreamWrapper>(&stringStream1v4);
  std::ostringstream stringStream1v6;
  Ptr<OutputStreamWrapper> ndiscStream =
      Create<OutputStreamWrapper>(&stringStream1v6);

  Ipv4RoutingHelper::PrintNeighborCacheAllAt(Seconds(0), arpStream);
  Ipv6RoutingHelper::PrintNeighborCacheAllAt(Seconds(0), ndiscStream);

  Simulator::Run();

  constexpr auto arpCache =
      "ARP Cache of node 0 at time 0\n"
      "10.1.1.2 dev 0 lladdr 04-06-00:00:00:00:00:02 STATIC_AUTOGENERATED\n"
      "ARP Cache of node 1 at time 0\n"
      "10.1.1.1 dev 0 lladdr 04-06-00:00:00:00:00:01 STATIC_AUTOGENERATED\n"
      "ARP Cache of node 2 at time 0\n";
  NS_TEST_EXPECT_MSG_EQ(stringStream1v4.str(), arpCache,
                        "Arp cache is incorrect.");

  constexpr auto NdiscCache = "NDISC Cache of node 0 at time +0s\n"
                              "2001::200:ff:fe00:2 dev 0 lladdr "
                              "04-06-00:00:00:00:00:02 STATIC_AUTOGENERATED\n"
                              "fe80::200:ff:fe00:2 dev 0 lladdr "
                              "04-06-00:00:00:00:00:02 STATIC_AUTOGENERATED\n"
                              "NDISC Cache of node 1 at time +0s\n"
                              "2001::200:ff:fe00:1 dev 0 lladdr "
                              "04-06-00:00:00:00:00:01 STATIC_AUTOGENERATED\n"
                              "fe80::200:ff:fe00:1 dev 0 lladdr "
                              "04-06-00:00:00:00:00:01 STATIC_AUTOGENERATED\n"
                              "NDISC Cache of node 2 at time +0s\n";
  NS_TEST_EXPECT_MSG_EQ(stringStream1v6.str(), NdiscCache,
                        "Ndisc cache is incorrect.");
  Simulator::Destroy();
}

class NetDeviceContainerTest : public TestCase {
public:
  void DoRun() override;
  NetDeviceContainerTest();

private:
  NodeContainer m_nodes;
};

NetDeviceContainerTest::NetDeviceContainerTest()
    : TestCase("The NetDeviceContainerTest check if neighbor caches are "
               "populated correctly on "
               "specific netDeviceContainer.") {}

void NetDeviceContainerTest::DoRun() {
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
  internet.Install(m_nodes);

  Ipv4AddressHelper ipv4;
  ipv4.SetBase("10.1.1.0", "255.255.255.252");
  Ipv4InterfaceContainer i = ipv4.Assign(net);
  ipv4.SetBase("10.1.2.0", "255.255.255.252");
  Ipv4InterfaceContainer i2 = ipv4.Assign(net2);

  Ipv6AddressHelper ipv6;
  ipv6.SetBase(Ipv6Address("2001:0::"), Ipv6Prefix(64));
  Ipv6InterfaceContainer icv61 = ipv6.Assign(net);
  ipv6.SetBase(Ipv6Address("2001:1::"), Ipv6Prefix(64));
  Ipv6InterfaceContainer icv62 = ipv6.Assign(net2);

  NeighborCacheHelper neighborCache;
  neighborCache.PopulateNeighborCache(net2);

  std::ostringstream stringStream1v4;
  Ptr<OutputStreamWrapper> arpStream =
      Create<OutputStreamWrapper>(&stringStream1v4);
  std::ostringstream stringStream1v6;
  Ptr<OutputStreamWrapper> ndiscStream =
      Create<OutputStreamWrapper>(&stringStream1v6);

  Ipv4RoutingHelper::PrintNeighborCacheAllAt(Seconds(0), arpStream);
  Ipv6RoutingHelper::PrintNeighborCacheAllAt(Seconds(0), ndiscStream);

  Simulator::Run();

  constexpr auto arpCache =
      "ARP Cache of node 0 at time 0\n"
      "ARP Cache of node 1 at time 0\n"
      "10.1.2.2 dev 1 lladdr 04-06-00:00:00:00:00:04 STATIC_AUTOGENERATED\n"
      "ARP Cache of node 2 at time 0\n"
      "10.1.2.1 dev 0 lladdr 04-06-00:00:00:00:00:03 STATIC_AUTOGENERATED\n";
  NS_TEST_EXPECT_MSG_EQ(stringStream1v4.str(), arpCache,
                        "Arp cache is incorrect.");

  constexpr auto NdiscCache = "NDISC Cache of node 0 at time +0s\n"
                              "NDISC Cache of node 1 at time +0s\n"
                              "2001:1::200:ff:fe00:4 dev 1 lladdr "
                              "04-06-00:00:00:00:00:04 STATIC_AUTOGENERATED\n"
                              "fe80::200:ff:fe00:4 dev 1 lladdr "
                              "04-06-00:00:00:00:00:04 STATIC_AUTOGENERATED\n"
                              "NDISC Cache of node 2 at time +0s\n"
                              "2001:1::200:ff:fe00:3 dev 0 lladdr "
                              "04-06-00:00:00:00:00:03 STATIC_AUTOGENERATED\n"
                              "fe80::200:ff:fe00:3 dev 0 lladdr "
                              "04-06-00:00:00:00:00:03 STATIC_AUTOGENERATED\n";
  NS_TEST_EXPECT_MSG_EQ(stringStream1v6.str(), NdiscCache,
                        "Ndisc cache is incorrect.");
  Simulator::Destroy();
}

class InterfaceContainerTest : public TestCase {
public:
  void DoRun() override;
  InterfaceContainerTest();

private:
  NodeContainer m_nodes;
};

InterfaceContainerTest::InterfaceContainerTest()
    : TestCase("The InterfaceContainerTest check if neighbor caches are "
               "populated correctly on "
               "specific interfaceContainer.") {}

void InterfaceContainerTest::DoRun() {
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
  internet.Install(m_nodes);

  Ipv4AddressHelper ipv4;
  ipv4.SetBase("10.1.1.0", "255.255.255.252");
  Ipv4InterfaceContainer i = ipv4.Assign(net);
  ipv4.SetBase("10.1.2.0", "255.255.255.252");
  Ipv4InterfaceContainer i2 = ipv4.Assign(net2);

  Ipv6AddressHelper ipv6;
  ipv6.SetBase(Ipv6Address("2001:0::"), Ipv6Prefix(64));
  Ipv6InterfaceContainer icv61 = ipv6.Assign(net);
  ipv6.SetBase(Ipv6Address("2001:1::"), Ipv6Prefix(64));
  Ipv6InterfaceContainer icv62 = ipv6.Assign(net2);

  NeighborCacheHelper neighborCache;
  neighborCache.PopulateNeighborCache(i);
  neighborCache.PopulateNeighborCache(icv62);

  std::ostringstream stringStream1v4;
  Ptr<OutputStreamWrapper> arpStream =
      Create<OutputStreamWrapper>(&stringStream1v4);
  std::ostringstream stringStream1v6;
  Ptr<OutputStreamWrapper> ndiscStream =
      Create<OutputStreamWrapper>(&stringStream1v6);

  Ipv4RoutingHelper::PrintNeighborCacheAllAt(Seconds(0), arpStream);
  Ipv6RoutingHelper::PrintNeighborCacheAllAt(Seconds(0), ndiscStream);

  Simulator::Run();

  constexpr auto arpCache =
      "ARP Cache of node 0 at time 0\n"
      "10.1.1.2 dev 0 lladdr 04-06-00:00:00:00:00:02 STATIC_AUTOGENERATED\n"
      "ARP Cache of node 1 at time 0\n"
      "10.1.1.1 dev 0 lladdr 04-06-00:00:00:00:00:01 STATIC_AUTOGENERATED\n"
      "ARP Cache of node 2 at time 0\n";
  NS_TEST_EXPECT_MSG_EQ(stringStream1v4.str(), arpCache,
                        "Arp cache is incorrect.");

  constexpr auto NdiscCache = "NDISC Cache of node 0 at time +0s\n"
                              "NDISC Cache of node 1 at time +0s\n"
                              "2001:1::200:ff:fe00:4 dev 1 lladdr "
                              "04-06-00:00:00:00:00:04 STATIC_AUTOGENERATED\n"
                              "fe80::200:ff:fe00:4 dev 1 lladdr "
                              "04-06-00:00:00:00:00:04 STATIC_AUTOGENERATED\n"
                              "NDISC Cache of node 2 at time +0s\n"
                              "2001:1::200:ff:fe00:3 dev 0 lladdr "
                              "04-06-00:00:00:00:00:03 STATIC_AUTOGENERATED\n"
                              "fe80::200:ff:fe00:3 dev 0 lladdr "
                              "04-06-00:00:00:00:00:03 STATIC_AUTOGENERATED\n";
  NS_TEST_EXPECT_MSG_EQ(stringStream1v6.str(), NdiscCache,
                        "Ndisc cache is incorrect.");
  Simulator::Destroy();
}

class FlushTest : public TestCase {
public:
  void DoRun() override;
  FlushTest();

private:
  NodeContainer m_nodes;
};

FlushTest::FlushTest()
    : TestCase(
          "The FlushTest checks that FlushAutoGenerated() will only remove "
          "STATIC_AUTOGENERATED entries.") {}

void FlushTest::DoRun() {
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
  internet.Install(m_nodes);

  Ipv4AddressHelper ipv4;
  ipv4.SetBase("10.1.1.0", "255.255.255.252");
  Ipv4InterfaceContainer i = ipv4.Assign(net);
  ipv4.SetBase("10.1.2.0", "255.255.255.252");
  Ipv4InterfaceContainer i2 = ipv4.Assign(net2);

  Ipv6AddressHelper ipv6;
  ipv6.SetBase(Ipv6Address("2001:0::"), Ipv6Prefix(64));
  Ipv6InterfaceContainer icv61 = ipv6.Assign(net);
  ipv6.SetBase(Ipv6Address("2001:1::"), Ipv6Prefix(64));
  Ipv6InterfaceContainer icv62 = ipv6.Assign(net2);

  NeighborCacheHelper neighborCache;
  neighborCache.PopulateNeighborCache();

  std::pair<Ptr<Ipv4>, uint32_t> returnValue = i.Get(0);
  Ptr<Ipv4> v4 = returnValue.first;
  uint32_t index = returnValue.second;
  Ptr<Ipv4Interface> iface =
      DynamicCast<Ipv4L3Protocol>(v4)->GetInterface(index);
  Ptr<ArpCache> arpCache = iface->GetArpCache();
  ArpCache::Entry *arpCacheEntry = arpCache->Add(Ipv4Address("10.1.1.4"));
  arpCacheEntry->SetMacAddress(Mac48Address("04-06-00:00:00:00:00:01"));
  arpCacheEntry->MarkPermanent();

  std::pair<Ptr<Ipv6>, uint32_t> returnValue2 = icv61.Get(0);
  Ptr<Ipv6> v6 = returnValue2.first;
  index = returnValue2.second;
  Ptr<Ipv6Interface> ifacev6 =
      DynamicCast<Ipv6L3Protocol>(v6)->GetInterface(index);
  Ptr<NdiscCache> ndiscCache = ifacev6->GetNdiscCache();
  NdiscCache::Entry *ndiscCacheEntry =
      ndiscCache->Add(Ipv6Address("2001::200:ff:fe00:4"));
  ndiscCacheEntry->SetMacAddress(Mac48Address("04-06-00:00:00:00:00:01"));
  ndiscCacheEntry->MarkPermanent();

  neighborCache.FlushAutoGenerated();

  std::ostringstream stringStream1v4;
  Ptr<OutputStreamWrapper> arpStream =
      Create<OutputStreamWrapper>(&stringStream1v4);
  std::ostringstream stringStream1v6;
  Ptr<OutputStreamWrapper> ndiscStream =
      Create<OutputStreamWrapper>(&stringStream1v6);

  Ipv4RoutingHelper::PrintNeighborCacheAllAt(Seconds(0), arpStream);
  Ipv6RoutingHelper::PrintNeighborCacheAllAt(Seconds(0), ndiscStream);

  Simulator::Run();
  constexpr auto ArpCache =
      "ARP Cache of node 0 at time 0\n"
      "10.1.1.4 dev 0 lladdr 04-06-00:00:00:00:00:01 PERMANENT\n"
      "ARP Cache of node 1 at time 0\n"
      "ARP Cache of node 2 at time 0\n";
  NS_TEST_EXPECT_MSG_EQ(stringStream1v4.str(), ArpCache,
                        "Arp cache is incorrect.");

  constexpr auto NdiscCache =
      "NDISC Cache of node 0 at time +0s\n"
      "2001::200:ff:fe00:4 dev 0 lladdr 04-06-00:00:00:00:00:01 PERMANENT\n"
      "NDISC Cache of node 1 at time +0s\n"
      "NDISC Cache of node 2 at time +0s\n";
  NS_TEST_EXPECT_MSG_EQ(stringStream1v6.str(), NdiscCache,
                        "Ndisc cache is incorrect.");
  Simulator::Destroy();
}

class DuplicateTest : public TestCase {
public:
  void DoRun() override;
  DuplicateTest();

private:
  NodeContainer m_nodes;
};

DuplicateTest::DuplicateTest()
    : TestCase("The DuplicateTest checks that populate neighbor caches in "
               "overlapped scope does "
               "not raise an error or generate duplicate entries.") {}

void DuplicateTest::DoRun() {
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
  internet.Install(m_nodes);

  Ipv4AddressHelper ipv4;
  ipv4.SetBase("10.1.1.0", "255.255.255.252");
  Ipv4InterfaceContainer i = ipv4.Assign(net);
  ipv4.SetBase("10.1.2.0", "255.255.255.252");
  Ipv4InterfaceContainer i2 = ipv4.Assign(net2);

  Ipv6AddressHelper ipv6;
  ipv6.SetBase(Ipv6Address("2001:0::"), Ipv6Prefix(64));
  Ipv6InterfaceContainer icv61 = ipv6.Assign(net);
  ipv6.SetBase(Ipv6Address("2001:1::"), Ipv6Prefix(64));
  Ipv6InterfaceContainer icv62 = ipv6.Assign(net2);

  NeighborCacheHelper neighborCache;
  neighborCache.PopulateNeighborCache();
  neighborCache.PopulateNeighborCache(channel);
  neighborCache.PopulateNeighborCache(net2);
  neighborCache.PopulateNeighborCache(icv61);
  neighborCache.PopulateNeighborCache();

  std::ostringstream stringStream1v4;
  Ptr<OutputStreamWrapper> arpStream =
      Create<OutputStreamWrapper>(&stringStream1v4);
  std::ostringstream stringStream1v6;
  Ptr<OutputStreamWrapper> ndiscStream =
      Create<OutputStreamWrapper>(&stringStream1v6);

  Ipv4RoutingHelper::PrintNeighborCacheAllAt(Seconds(0), arpStream);
  Ipv6RoutingHelper::PrintNeighborCacheAllAt(Seconds(0), ndiscStream);

  Simulator::Run();
  constexpr auto ArpCache =
      "ARP Cache of node 0 at time 0\n"
      "10.1.1.2 dev 0 lladdr 04-06-00:00:00:00:00:02 STATIC_AUTOGENERATED\n"
      "ARP Cache of node 1 at time 0\n"
      "10.1.1.1 dev 0 lladdr 04-06-00:00:00:00:00:01 STATIC_AUTOGENERATED\n"
      "10.1.2.2 dev 1 lladdr 04-06-00:00:00:00:00:04 STATIC_AUTOGENERATED\n"
      "ARP Cache of node 2 at time 0\n"
      "10.1.2.1 dev 0 lladdr 04-06-00:00:00:00:00:03 STATIC_AUTOGENERATED\n";
  NS_TEST_EXPECT_MSG_EQ(stringStream1v4.str(), ArpCache,
                        "Arp cache is incorrect.");

  constexpr auto NdiscCache = "NDISC Cache of node 0 at time +0s\n"
                              "2001::200:ff:fe00:2 dev 0 lladdr "
                              "04-06-00:00:00:00:00:02 STATIC_AUTOGENERATED\n"
                              "fe80::200:ff:fe00:2 dev 0 lladdr "
                              "04-06-00:00:00:00:00:02 STATIC_AUTOGENERATED\n"
                              "NDISC Cache of node 1 at time +0s\n"
                              "2001::200:ff:fe00:1 dev 0 lladdr "
                              "04-06-00:00:00:00:00:01 STATIC_AUTOGENERATED\n"
                              "fe80::200:ff:fe00:1 dev 0 lladdr "
                              "04-06-00:00:00:00:00:01 STATIC_AUTOGENERATED\n"
                              "2001:1::200:ff:fe00:4 dev 1 lladdr "
                              "04-06-00:00:00:00:00:04 STATIC_AUTOGENERATED\n"
                              "fe80::200:ff:fe00:4 dev 1 lladdr "
                              "04-06-00:00:00:00:00:04 STATIC_AUTOGENERATED\n"
                              "NDISC Cache of node 2 at time +0s\n"
                              "2001:1::200:ff:fe00:3 dev 0 lladdr "
                              "04-06-00:00:00:00:00:03 STATIC_AUTOGENERATED\n"
                              "fe80::200:ff:fe00:3 dev 0 lladdr "
                              "04-06-00:00:00:00:00:03 STATIC_AUTOGENERATED\n";
  NS_TEST_EXPECT_MSG_EQ(stringStream1v6.str(), NdiscCache,
                        "Ndisc cache is incorrect.");
  Simulator::Destroy();
}

class DynamicPartialTest : public TestCase {
public:
  void DoRun() override;
  DynamicPartialTest();

  void AddIpv4Address(Ptr<Ipv4Interface> ipv4Interface,
                      Ipv4InterfaceAddress ifaceAddr);

  void AddIpv6Address(Ptr<Ipv6Interface> ipv6Interface,
                      Ipv6InterfaceAddress ifaceAddr);

  void RemoveIpv4Address(Ptr<Ipv4Interface> ipv4Interface, uint32_t index);

  void RemoveIpv6Address(Ptr<Ipv6Interface> ipv6Interface, uint32_t index);

private:
  NodeContainer m_nodes;
};

DynamicPartialTest::DynamicPartialTest()
    : TestCase("The DynamicPartialTest checks if dynamic neighbor cache update "
               "correctly when "
               "generating on a non-global scope.") {}

void DynamicPartialTest::AddIpv4Address(Ptr<Ipv4Interface> ipv4Interface,
                                        Ipv4InterfaceAddress ifaceAddr) {
  ipv4Interface->AddAddress(ifaceAddr);
}

void DynamicPartialTest::AddIpv6Address(Ptr<Ipv6Interface> ipv6Interface,
                                        Ipv6InterfaceAddress ifaceAddr) {
  ipv6Interface->AddAddress(ifaceAddr);
}

void DynamicPartialTest::RemoveIpv4Address(Ptr<Ipv4Interface> ipv4Interface,
                                           uint32_t index) {
  ipv4Interface->RemoveAddress(index);
}

void DynamicPartialTest::RemoveIpv6Address(Ptr<Ipv6Interface> ipv6Interface,
                                           uint32_t index) {
  ipv6Interface->RemoveAddress(index);
}

void DynamicPartialTest::DoRun() {
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
  internet.Install(m_nodes);

  Ipv4AddressHelper ipv4;
  ipv4.SetBase("10.1.1.0", "255.255.255.252");
  Ipv4InterfaceContainer i = ipv4.Assign(net);
  ipv4.SetBase("10.1.2.0", "255.255.255.252");
  Ipv4InterfaceContainer i2 = ipv4.Assign(net2);

  Ptr<Node> n1 = m_nodes.Get(0);
  uint32_t ipv4ifIndex = 1;
  Ptr<Ipv4Interface> ipv4Interface =
      n1->GetObject<Ipv4L3Protocol>()->GetInterface(ipv4ifIndex);
  Ipv4InterfaceAddress ifaceAddr =
      Ipv4InterfaceAddress("10.1.1.5", "255.255.255.0");
  Simulator::Schedule(Seconds(0.5), &DynamicPartialTest::AddIpv4Address, this,
                      ipv4Interface, ifaceAddr);

  uint32_t addressIndex = 0;
  Simulator::Schedule(Seconds(1.5), &DynamicPartialTest::RemoveIpv4Address,
                      this, ipv4Interface, addressIndex);

  Ipv6AddressHelper ipv6;
  ipv6.SetBase(Ipv6Address("2001:0::"), Ipv6Prefix(64));
  Ipv6InterfaceContainer icv61 = ipv6.Assign(net);
  ipv6.SetBase(Ipv6Address("2001:1::"), Ipv6Prefix(64));
  Ipv6InterfaceContainer icv62 = ipv6.Assign(net2);

  uint32_t ipv6ifIndex = 1;
  Ptr<Ipv6Interface> ipv6Interface =
      n1->GetObject<Ipv6L3Protocol>()->GetInterface(ipv6ifIndex);
  Ipv6InterfaceAddress ifaceAddrv6 =
      Ipv6InterfaceAddress("2001:0::200:ff:fe00:5", Ipv6Prefix(64));
  Simulator::Schedule(Seconds(0.5), &DynamicPartialTest::AddIpv6Address, this,
                      ipv6Interface, ifaceAddrv6);

  addressIndex = 1;
  Simulator::Schedule(Seconds(1.5), &DynamicPartialTest::RemoveIpv6Address,
                      this, ipv6Interface, addressIndex);

  NeighborCacheHelper neighborCache;
  neighborCache.SetDynamicNeighborCache(true);
  neighborCache.PopulateNeighborCache(channel);

  std::ostringstream stringStream1v4;
  Ptr<OutputStreamWrapper> arpStream =
      Create<OutputStreamWrapper>(&stringStream1v4);
  std::ostringstream stringStream1v6;
  Ptr<OutputStreamWrapper> ndiscStream =
      Create<OutputStreamWrapper>(&stringStream1v6);

  Ipv4RoutingHelper::PrintNeighborCacheAllAt(Seconds(0), arpStream);
  Ipv6RoutingHelper::PrintNeighborCacheAllAt(Seconds(0), ndiscStream);
  Ipv4RoutingHelper::PrintNeighborCacheAllAt(Seconds(1), arpStream);
  Ipv6RoutingHelper::PrintNeighborCacheAllAt(Seconds(1), ndiscStream);
  Ipv4RoutingHelper::PrintNeighborCacheAllAt(Seconds(2), arpStream);
  Ipv6RoutingHelper::PrintNeighborCacheAllAt(Seconds(2), ndiscStream);

  Simulator::Run();
  constexpr auto ArpCache =
      "ARP Cache of node 0 at time 0\n"
      "10.1.1.2 dev 0 lladdr 04-06-00:00:00:00:00:02 STATIC_AUTOGENERATED\n"
      "ARP Cache of node 1 at time 0\n"
      "10.1.1.1 dev 0 lladdr 04-06-00:00:00:00:00:01 STATIC_AUTOGENERATED\n"
      "ARP Cache of node 2 at time 0\n"
      "ARP Cache of node 0 at time 1\n"
      "10.1.1.2 dev 0 lladdr 04-06-00:00:00:00:00:02 STATIC_AUTOGENERATED\n"
      "ARP Cache of node 1 at time 1\n"
      "10.1.1.1 dev 0 lladdr 04-06-00:00:00:00:00:01 STATIC_AUTOGENERATED\n"
      "ARP Cache of node 2 at time 1\n"
      "ARP Cache of node 0 at time 2\n"
      "10.1.1.2 dev 0 lladdr 04-06-00:00:00:00:00:02 STATIC_AUTOGENERATED\n"
      "ARP Cache of node 1 at time 2\n"
      "ARP Cache of node 2 at time 2\n";
  NS_TEST_EXPECT_MSG_EQ(stringStream1v4.str(), ArpCache,
                        "Arp cache is incorrect.");

  constexpr auto NdiscCache = "NDISC Cache of node 0 at time +0s\n"
                              "2001::200:ff:fe00:2 dev 0 lladdr "
                              "04-06-00:00:00:00:00:02 STATIC_AUTOGENERATED\n"
                              "fe80::200:ff:fe00:2 dev 0 lladdr "
                              "04-06-00:00:00:00:00:02 STATIC_AUTOGENERATED\n"
                              "NDISC Cache of node 1 at time +0s\n"
                              "2001::200:ff:fe00:1 dev 0 lladdr "
                              "04-06-00:00:00:00:00:01 STATIC_AUTOGENERATED\n"
                              "fe80::200:ff:fe00:1 dev 0 lladdr "
                              "04-06-00:00:00:00:00:01 STATIC_AUTOGENERATED\n"
                              "NDISC Cache of node 2 at time +0s\n"
                              "NDISC Cache of node 0 at time +1s\n"
                              "2001::200:ff:fe00:2 dev 0 lladdr "
                              "04-06-00:00:00:00:00:02 STATIC_AUTOGENERATED\n"
                              "fe80::200:ff:fe00:2 dev 0 lladdr "
                              "04-06-00:00:00:00:00:02 STATIC_AUTOGENERATED\n"
                              "NDISC Cache of node 1 at time +1s\n"
                              "2001::200:ff:fe00:1 dev 0 lladdr "
                              "04-06-00:00:00:00:00:01 STATIC_AUTOGENERATED\n"
                              "fe80::200:ff:fe00:1 dev 0 lladdr "
                              "04-06-00:00:00:00:00:01 STATIC_AUTOGENERATED\n"
                              "NDISC Cache of node 2 at time +1s\n"
                              "NDISC Cache of node 0 at time +2s\n"
                              "2001::200:ff:fe00:2 dev 0 lladdr "
                              "04-06-00:00:00:00:00:02 STATIC_AUTOGENERATED\n"
                              "fe80::200:ff:fe00:2 dev 0 lladdr "
                              "04-06-00:00:00:00:00:02 STATIC_AUTOGENERATED\n"
                              "NDISC Cache of node 1 at time +2s\n"
                              "fe80::200:ff:fe00:1 dev 0 lladdr "
                              "04-06-00:00:00:00:00:01 STATIC_AUTOGENERATED\n"
                              "NDISC Cache of node 2 at time +2s\n";
  NS_TEST_EXPECT_MSG_EQ(stringStream1v6.str(), NdiscCache,
                        "Ndisc cache is incorrect.");
  Simulator::Destroy();
}

class NeighborCacheTestSuite : public TestSuite {
public:
  NeighborCacheTestSuite() : TestSuite("neighbor-cache", UNIT) {
    AddTestCase(new DynamicNeighborCacheTest, TestCase::QUICK);
    AddTestCase(new ChannelTest, TestCase::QUICK);
    AddTestCase(new NetDeviceContainerTest, TestCase::QUICK);
    AddTestCase(new InterfaceContainerTest, TestCase::QUICK);
    AddTestCase(new FlushTest, TestCase::QUICK);
    AddTestCase(new DuplicateTest, TestCase::QUICK);
    AddTestCase(new DynamicPartialTest, TestCase::QUICK);
  }
};

static NeighborCacheTestSuite g_neighborcacheTestSuite;
