

#include "ns3/abort.h"
#include "ns3/attribute.h"
#include "ns3/boolean.h"
#include "ns3/icmpv6-l4-protocol.h"
#include "ns3/inet6-socket-address.h"
#include "ns3/internet-stack-helper.h"
#include "ns3/ipv6-address.h"
#include "ns3/ipv6-interface.h"
#include "ns3/ipv6-l3-protocol.h"
#include "ns3/ipv6-list-routing.h"
#include "ns3/ipv6-packet-info-tag.h"
#include "ns3/ipv6-raw-socket-factory.h"
#include "ns3/ipv6-static-routing.h"
#include "ns3/log.h"
#include "ns3/node.h"
#include "ns3/object-factory.h"
#include "ns3/simple-net-device-helper.h"
#include "ns3/simple-net-device.h"
#include "ns3/simulator.h"
#include "ns3/socket-factory.h"
#include "ns3/test.h"
#include "ns3/traffic-control-layer.h"
#include "ns3/udp-socket-factory.h"
#include "ns3/udp-socket.h"
#include "ns3/uinteger.h"

using namespace ns3;

class Ipv6PacketInfoTagTest : public TestCase {
public:
  Ipv6PacketInfoTagTest();

private:
  void DoRun() override;
  void RxCb(Ptr<Socket> socket);
  void DoSendData(Ptr<Socket> socket, std::string to);
};

Ipv6PacketInfoTagTest::Ipv6PacketInfoTagTest()
    : TestCase("Ipv6PacketInfoTagTest") {}

void Ipv6PacketInfoTagTest::RxCb(Ptr<Socket> socket) {
  uint32_t availableData;
  Ptr<Packet> m_receivedPacket;

  availableData = socket->GetRxAvailable();
  m_receivedPacket = socket->Recv(std::numeric_limits<uint32_t>::max(), 0);
  NS_TEST_ASSERT_MSG_EQ(availableData, m_receivedPacket->GetSize(),
                        "Did not read expected data");

  Ipv6PacketInfoTag tag;
  bool found;
  found = m_receivedPacket->RemovePacketTag(tag);
  NS_TEST_ASSERT_MSG_EQ(found, true, "Could not find tag");
}

void Ipv6PacketInfoTagTest::DoSendData(Ptr<Socket> socket, std::string to) {
  Address realTo = Inet6SocketAddress(Ipv6Address(to.c_str()), 200);
  if (DynamicCast<UdpSocket>(socket)) {
    NS_TEST_EXPECT_MSG_EQ(socket->SendTo(Create<Packet>(123), 0, realTo), 123,
                          "100");
  } else {
    socket->SendTo(Create<Packet>(123), 0, realTo);
  }
}

void Ipv6PacketInfoTagTest::DoRun() {
  Ptr<Node> node0 = CreateObject<Node>();
  Ptr<Node> node1 = CreateObject<Node>();

  SimpleNetDeviceHelper simpleNetDevHelper;
  NetDeviceContainer devs =
      simpleNetDevHelper.Install(NodeContainer(node0, node1));
  Ptr<SimpleNetDevice> device = DynamicCast<SimpleNetDevice>(devs.Get(0));
  Ptr<SimpleNetDevice> device2 = DynamicCast<SimpleNetDevice>(devs.Get(1));

  InternetStackHelper internet;
  internet.SetIpv4StackInstall(false);

  node0->AddDevice(device);
  internet.Install(node0);
  Ptr<Ipv6> ipv6 = node0->GetObject<Ipv6>();
  Ptr<Icmpv6L4Protocol> icmpv6 = node0->GetObject<Icmpv6L4Protocol>();
  icmpv6->SetAttribute("DAD", BooleanValue(false));

  uint32_t index = ipv6->AddInterface(device);
  Ipv6InterfaceAddress ifaceAddr1 =
      Ipv6InterfaceAddress(Ipv6Address("2000:1000:0:2000::1"), Ipv6Prefix(64));
  ipv6->AddAddress(index, ifaceAddr1);
  ipv6->SetMetric(index, 1);
  ipv6->SetUp(index);

  node1->AddDevice(device2);
  internet.Install(node1);
  ipv6 = node1->GetObject<Ipv6>();
  icmpv6 = node0->GetObject<Icmpv6L4Protocol>();
  icmpv6->SetAttribute("DAD", BooleanValue(false));

  index = ipv6->AddInterface(device2);
  Ipv6InterfaceAddress ifaceAddr2 =
      Ipv6InterfaceAddress(Ipv6Address("2000:1000:0:2000::2"), Ipv6Prefix(64));
  ipv6->AddAddress(index, ifaceAddr2);
  ipv6->SetMetric(index, 1);
  ipv6->SetUp(index);

  Ptr<SocketFactory> factory =
      node0->GetObject<SocketFactory>(Ipv6RawSocketFactory::GetTypeId());
  Ptr<Socket> socket = factory->CreateSocket();
  Inet6SocketAddress local = Inet6SocketAddress(Ipv6Address::GetAny(), 0);
  socket->SetAttribute("Protocol", UintegerValue(Ipv6Header::IPV6_ICMPV6));
  socket->Bind(local);
  socket->SetRecvPktInfo(true);
  socket->SetRecvCallback(MakeCallback(&Ipv6PacketInfoTagTest::RxCb, this));

  Simulator::ScheduleWithContext(socket->GetNode()->GetId(), Seconds(0),
                                 &Ipv6PacketInfoTagTest::DoSendData, this,
                                 socket, "::1");
  Simulator::Run();

  Ptr<SocketFactory> factory2 =
      node1->GetObject<SocketFactory>(Ipv6RawSocketFactory::GetTypeId());
  Ptr<Socket> socket2 = factory2->CreateSocket();
  std::stringstream dst;
  dst << ifaceAddr1.GetAddress();
  Simulator::ScheduleWithContext(socket2->GetNode()->GetId(), Seconds(0),
                                 &Ipv6PacketInfoTagTest::DoSendData, this,
                                 socket, dst.str());
  Simulator::Run();

#ifdef UDP6_SUPPORTED
  factory = node0->GetObject<SocketFactory>(UdpSocketFactory::GetTypeId());
  socket = factory->CreateSocket();
  local = Inet6SocketAddress(Ipv6Address::GetAny(), 200);
  socket->Bind(local);
  socket->SetRecvPktInfo(true);
  socket->SetRecvCallback(MakeCallback(&Ipv6PacketInfoTagTest::RxCb, this));

  Simulator::ScheduleWithContext(socket->GetNode()->GetId(), Seconds(0),
                                 &Ipv6PacketInfoTagTest::DoSendData, this,
                                 socket, "::1");
  Simulator::Run();

  factory2 = node1->GetObject<SocketFactory>(UdpSocketFactory::GetTypeId());
  socket2 = factory2->CreateSocket();
  Simulator::ScheduleWithContext(socket2->GetNode()->GetId(), Seconds(0),
                                 &Ipv6PacketInfoTagTest::DoSendData, this,
                                 socket, "10.1.1.1");
  Simulator::Run();

#endif

  Simulator::Destroy();
}

class Ipv6PacketInfoTagTestSuite : public TestSuite {
public:
  Ipv6PacketInfoTagTestSuite();

private:
};

Ipv6PacketInfoTagTestSuite::Ipv6PacketInfoTagTestSuite()
    : TestSuite("ipv6-packet-info-tag", UNIT) {
  AddTestCase(new Ipv6PacketInfoTagTest(), TestCase::QUICK);
}

static Ipv6PacketInfoTagTestSuite g_packetinfotagTests;
