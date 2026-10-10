
#include "ns3/arp-l3-protocol.h"
#include "ns3/boolean.h"
#include "ns3/icmpv4-l4-protocol.h"
#include "ns3/inet-socket-address.h"
#include "ns3/internet-stack-helper.h"
#include "ns3/ipv4-l3-protocol.h"
#include "ns3/ipv4-routing-helper.h"
#include "ns3/ipv4-static-routing.h"
#include "ns3/log.h"
#include "ns3/node.h"
#include "ns3/simple-channel.h"
#include "ns3/simple-net-device.h"
#include "ns3/simulator.h"
#include "ns3/socket-factory.h"
#include "ns3/socket.h"
#include "ns3/test.h"
#include "ns3/traffic-control-layer.h"
#include "ns3/udp-l4-protocol.h"
#include "ns3/udp-socket-factory.h"

#include <limits>
#include <string>

using namespace ns3;

class Ipv4ForwardingTest : public TestCase {
  Ptr<Packet> m_receivedPacket;

  void DoSendData(Ptr<Socket> socket, std::string to);
  void SendData(Ptr<Socket> socket, std::string to);

public:
  void DoRun() override;
  Ipv4ForwardingTest();

  void ReceivePkt(Ptr<Socket> socket);
};

Ipv4ForwardingTest::Ipv4ForwardingTest()
    : TestCase("UDP socket implementation") {}

void Ipv4ForwardingTest::ReceivePkt(Ptr<Socket> socket) {
  uint32_t availableData;
  availableData = socket->GetRxAvailable();
  m_receivedPacket = socket->Recv(std::numeric_limits<uint32_t>::max(), 0);
  NS_TEST_ASSERT_MSG_EQ(availableData, m_receivedPacket->GetSize(),
                        "Received packet size is not equal to Rx buffer size");
}

void Ipv4ForwardingTest::DoSendData(Ptr<Socket> socket, std::string to) {
  Address realTo = InetSocketAddress(Ipv4Address(to.c_str()), 1234);
  NS_TEST_EXPECT_MSG_EQ(socket->SendTo(Create<Packet>(123), 0, realTo), 123,
                        "100");
}

void Ipv4ForwardingTest::SendData(Ptr<Socket> socket, std::string to) {
  m_receivedPacket = Create<Packet>();
  Simulator::ScheduleWithContext(socket->GetNode()->GetId(), Seconds(0),
                                 &Ipv4ForwardingTest::DoSendData, this, socket,
                                 to);
  Simulator::Run();
}

void Ipv4ForwardingTest::DoRun() {

  Ptr<Node> rxNode = CreateObject<Node>();

  InternetStackHelper internet;
  internet.SetIpv6StackInstall(false);

  internet.Install(rxNode);
  Ptr<SimpleNetDevice> rxDev;
  {
    rxDev = CreateObject<SimpleNetDevice>();
    rxDev->SetAddress(Mac48Address::ConvertFrom(Mac48Address::Allocate()));
    rxNode->AddDevice(rxDev);
    Ptr<Ipv4> ipv4 = rxNode->GetObject<Ipv4>();
    uint32_t netdev_idx = ipv4->AddInterface(rxDev);
    Ipv4InterfaceAddress ipv4Addr =
        Ipv4InterfaceAddress(Ipv4Address("10.0.0.2"), Ipv4Mask(0xffff0000U));
    ipv4->AddAddress(netdev_idx, ipv4Addr);
    ipv4->SetUp(netdev_idx);
  }

  Ptr<Node> fwNode = CreateObject<Node>();

  internet.Install(fwNode);
  Ptr<SimpleNetDevice> fwDev1;
  Ptr<SimpleNetDevice> fwDev2;
  {
    fwDev1 = CreateObject<SimpleNetDevice>();
    fwDev1->SetAddress(Mac48Address::ConvertFrom(Mac48Address::Allocate()));
    fwNode->AddDevice(fwDev1);
    Ptr<Ipv4> ipv4 = fwNode->GetObject<Ipv4>();
    uint32_t netdev_idx = ipv4->AddInterface(fwDev1);
    Ipv4InterfaceAddress ipv4Addr =
        Ipv4InterfaceAddress(Ipv4Address("10.0.0.1"), Ipv4Mask(0xffff0000U));
    ipv4->AddAddress(netdev_idx, ipv4Addr);
    ipv4->SetUp(netdev_idx);
  }

  {
    fwDev2 = CreateObject<SimpleNetDevice>();
    fwDev2->SetAddress(Mac48Address::ConvertFrom(Mac48Address::Allocate()));
    fwNode->AddDevice(fwDev2);
    Ptr<Ipv4> ipv4 = fwNode->GetObject<Ipv4>();
    uint32_t netdev_idx = ipv4->AddInterface(fwDev2);
    Ipv4InterfaceAddress ipv4Addr =
        Ipv4InterfaceAddress(Ipv4Address("10.1.0.1"), Ipv4Mask(0xffff0000U));
    ipv4->AddAddress(netdev_idx, ipv4Addr);
    ipv4->SetUp(netdev_idx);
  }

  Ptr<Node> txNode = CreateObject<Node>();

  internet.Install(txNode);
  Ptr<SimpleNetDevice> txDev;
  {
    txDev = CreateObject<SimpleNetDevice>();
    txDev->SetAddress(Mac48Address::ConvertFrom(Mac48Address::Allocate()));
    txNode->AddDevice(txDev);
    Ptr<Ipv4> ipv4 = txNode->GetObject<Ipv4>();
    uint32_t netdev_idx = ipv4->AddInterface(txDev);
    Ipv4InterfaceAddress ipv4Addr =
        Ipv4InterfaceAddress(Ipv4Address("10.1.0.2"), Ipv4Mask(0xffff0000U));
    ipv4->AddAddress(netdev_idx, ipv4Addr);
    ipv4->SetUp(netdev_idx);
    Ptr<Ipv4StaticRouting> ipv4StaticRouting =
        Ipv4RoutingHelper::GetRouting<Ipv4StaticRouting>(
            txNode->GetObject<Ipv4>()->GetRoutingProtocol());
    ipv4StaticRouting->SetDefaultRoute(Ipv4Address("10.1.0.1"), netdev_idx);
  }

  Ptr<SimpleChannel> channel1 = CreateObject<SimpleChannel>();
  rxDev->SetChannel(channel1);
  fwDev1->SetChannel(channel1);

  Ptr<SimpleChannel> channel2 = CreateObject<SimpleChannel>();
  fwDev2->SetChannel(channel2);
  txDev->SetChannel(channel2);

  Ptr<SocketFactory> rxSocketFactory = rxNode->GetObject<UdpSocketFactory>();
  Ptr<Socket> rxSocket = rxSocketFactory->CreateSocket();
  NS_TEST_EXPECT_MSG_EQ(
      rxSocket->Bind(InetSocketAddress(Ipv4Address("10.0.0.2"), 1234)), 0,
      "trivial");
  rxSocket->SetRecvCallback(
      MakeCallback(&Ipv4ForwardingTest::ReceivePkt, this));

  Ptr<SocketFactory> txSocketFactory = txNode->GetObject<UdpSocketFactory>();
  Ptr<Socket> txSocket = txSocketFactory->CreateSocket();
  txSocket->SetAllowBroadcast(true);

  SendData(txSocket, "10.0.0.2");
  NS_TEST_EXPECT_MSG_EQ(m_receivedPacket->GetSize(), 123, "IPv4 Forwarding on");

  m_receivedPacket->RemoveAllByteTags();
  m_receivedPacket = nullptr;

  Ptr<Ipv4> ipv4 = fwNode->GetObject<Ipv4>();
  ipv4->SetAttribute("IpForward", BooleanValue(false));
  SendData(txSocket, "10.0.0.2");
  NS_TEST_EXPECT_MSG_EQ(m_receivedPacket->GetSize(), 0, "IPv4 Forwarding off");

  Simulator::Destroy();
}

class Ipv4ForwardingTestSuite : public TestSuite {
public:
  Ipv4ForwardingTestSuite();

private:
};

Ipv4ForwardingTestSuite::Ipv4ForwardingTestSuite()
    : TestSuite("ipv4-forwarding", UNIT) {
  AddTestCase(new Ipv4ForwardingTest, TestCase::QUICK);
}

static Ipv4ForwardingTestSuite g_ipv4forwardingTestSuite;
