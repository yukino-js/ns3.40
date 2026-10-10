
#include "ns3/arp-l3-protocol.h"
#include "ns3/boolean.h"
#include "ns3/config.h"
#include "ns3/error-channel.h"
#include "ns3/icmpv4-l4-protocol.h"
#include "ns3/icmpv6-l4-protocol.h"
#include "ns3/inet-socket-address.h"
#include "ns3/inet6-socket-address.h"
#include "ns3/internet-stack-helper.h"
#include "ns3/ipv4-l3-protocol.h"
#include "ns3/ipv4-list-routing.h"
#include "ns3/ipv4-raw-socket-factory.h"
#include "ns3/ipv4-static-routing.h"
#include "ns3/ipv6-l3-protocol.h"
#include "ns3/ipv6-list-routing.h"
#include "ns3/ipv6-raw-socket-factory.h"
#include "ns3/ipv6-static-routing.h"
#include "ns3/log.h"
#include "ns3/node.h"
#include "ns3/simple-net-device.h"
#include "ns3/simulator.h"
#include "ns3/socket-factory.h"
#include "ns3/socket.h"
#include "ns3/test.h"
#include "ns3/traffic-control-layer.h"
#include "ns3/udp-l4-protocol.h"
#include "ns3/udp-socket-factory.h"
#include "ns3/udp-socket.h"
#include "ns3/uinteger.h"

#ifdef __WIN32__
#include "ns3/win32-internet.h"
#else
#include <netinet/in.h>
#endif

#include <limits>
#include <string>

using namespace ns3;

class UdpSocketImpl;

class IPv6TestTag : public Tag {
private:
  uint64_t token;

public:
  static TypeId GetTypeId() {
    static TypeId tid = TypeId("ns3::IPv6TestTag")
                            .SetParent<Tag>()
                            .AddConstructor<IPv6TestTag>();
    return tid;
  }

  TypeId GetInstanceTypeId() const override { return GetTypeId(); }

  uint32_t GetSerializedSize() const override { return sizeof(token); }

  void Serialize(TagBuffer buffer) const override { buffer.WriteU64(token); }

  void Deserialize(TagBuffer buffer) override { token = buffer.ReadU64(); }

  void Print(std::ostream &os) const override { os << "token=" << token; }

  void SetToken(uint64_t token) { this->token = token; }

  uint64_t GetToken() const { return token; }
};

class Ipv6FragmentationTest : public TestCase {
  Ptr<Packet> m_sentPacketClient;
  Ptr<Packet> m_receivedPacketClient;
  Ptr<Packet> m_receivedPacketServer;

  Ptr<Socket> m_socketServer;
  Ptr<Socket> m_socketClient;
  uint32_t m_dataSize;
  uint8_t *m_data;
  uint32_t m_size;
  uint8_t m_icmpType;
  uint8_t m_icmpCode;

public:
  void DoRun() override;
  Ipv6FragmentationTest();
  ~Ipv6FragmentationTest() override;

  void StartServer(Ptr<Node> ServerNode);
  void HandleReadServer(Ptr<Socket> socket);

  void StartClient(Ptr<Node> ClientNode);
  void HandleReadClient(Ptr<Socket> socket);
  void HandleReadIcmpClient(Ipv6Address icmpSource, uint8_t icmpTtl,
                            uint8_t icmpType, uint8_t icmpCode,
                            uint32_t icmpInfo);

  void SetFill(uint8_t *fill, uint32_t fillSize, uint32_t dataSize);

  Ptr<Packet> SendClient();

  void HandleServerRx(Ptr<const Packet> packet, Ptr<Ipv6> ipv6,
                      uint32_t interface);

  void HandleClientTx(Ptr<const Packet> packet, Ptr<Ipv6> ipv6,
                      uint32_t interface);
};

Ipv6FragmentationTest::Ipv6FragmentationTest()
    : TestCase(
          "Verify the IPv6 layer 3 protocol fragmentation and reassembly") {
  m_socketServer = nullptr;
  m_data = nullptr;
  m_dataSize = 0;
  m_size = 0;
  m_icmpType = 0;
  m_icmpCode = 0;
}

Ipv6FragmentationTest::~Ipv6FragmentationTest() {
  if (m_data) {
    delete[] m_data;
  }
  m_data = nullptr;
  m_dataSize = 0;
}

void Ipv6FragmentationTest::StartServer(Ptr<Node> ServerNode) {
  if (!m_socketServer) {
    TypeId tid = TypeId::LookupByName("ns3::UdpSocketFactory");
    m_socketServer = Socket::CreateSocket(ServerNode, tid);
    Inet6SocketAddress local = Inet6SocketAddress(Ipv6Address("2001::1"), 9);
    m_socketServer->Bind(local);
    Ptr<UdpSocket> udpSocket = DynamicCast<UdpSocket>(m_socketServer);
  }

  m_socketServer->SetRecvCallback(
      MakeCallback(&Ipv6FragmentationTest::HandleReadServer, this));
}

void Ipv6FragmentationTest::HandleReadServer(Ptr<Socket> socket) {
  Ptr<Packet> packet;
  Address from;
  while ((packet = socket->RecvFrom(from))) {
    if (Inet6SocketAddress::IsMatchingType(from)) {
      m_receivedPacketServer = packet->Copy();
    }
  }
}

void Ipv6FragmentationTest::StartClient(Ptr<Node> ClientNode) {
  if (!m_socketClient) {
    TypeId tid = TypeId::LookupByName("ns3::UdpSocketFactory");
    m_socketClient = Socket::CreateSocket(ClientNode, tid);
    m_socketClient->Bind(Inet6SocketAddress(Ipv6Address::GetAny(), 9));
    m_socketClient->Connect(Inet6SocketAddress(Ipv6Address("2001::1"), 9));
    CallbackValue cbValue =
        MakeCallback(&Ipv6FragmentationTest::HandleReadIcmpClient, this);
    m_socketClient->SetAttribute("IcmpCallback6", cbValue);
  }

  m_socketClient->SetRecvCallback(
      MakeCallback(&Ipv6FragmentationTest::HandleReadClient, this));
}

void Ipv6FragmentationTest::HandleReadClient(Ptr<Socket> socket) {
  Ptr<Packet> packet;
  Address from;
  while ((packet = socket->RecvFrom(from))) {
    if (Inet6SocketAddress::IsMatchingType(from)) {
      m_receivedPacketClient = packet->Copy();
    }
  }
}

void Ipv6FragmentationTest::HandleReadIcmpClient(Ipv6Address icmpSource,
                                                 uint8_t icmpTtl,
                                                 uint8_t icmpType,
                                                 uint8_t icmpCode,
                                                 uint32_t icmpInfo) {
  m_icmpType = icmpType;
  m_icmpCode = icmpCode;
}

void Ipv6FragmentationTest::SetFill(uint8_t *fill, uint32_t fillSize,
                                    uint32_t dataSize) {
  if (dataSize != m_dataSize) {
    delete[] m_data;
    m_data = new uint8_t[dataSize];
    m_dataSize = dataSize;
  }

  if (fillSize >= dataSize) {
    memcpy(m_data, fill, dataSize);
    return;
  }

  uint32_t filled = 0;
  while (filled + fillSize < dataSize) {
    memcpy(&m_data[filled], fill, fillSize);
    filled += fillSize;
  }

  memcpy(&m_data[filled], fill, dataSize - filled);

  m_size = dataSize;
}

Ptr<Packet> Ipv6FragmentationTest::SendClient() {
  Ptr<Packet> p;
  if (m_dataSize) {
    p = Create<Packet>(m_data, m_dataSize);
  } else {
    p = Create<Packet>(m_size);
  }
  IPv6TestTag tag;
  tag.SetToken(42);
  p->AddPacketTag(tag);
  p->AddByteTag(tag);

  m_socketClient->Send(p);

  return p;
}

void Ipv6FragmentationTest::HandleServerRx(Ptr<const Packet> packet,
                                           Ptr<Ipv6> ipv6, uint32_t interface) {
  NS_TEST_EXPECT_MSG_LT_OR_EQ(
      packet->GetSize(), ipv6->GetMtu(interface),
      "Received packet size > MTU: packetSizes: " << packet->GetSize());
}

void Ipv6FragmentationTest::HandleClientTx(Ptr<const Packet> packet,
                                           Ptr<Ipv6> ipv6, uint32_t interface) {
  NS_TEST_EXPECT_MSG_LT_OR_EQ(
      packet->GetSize(), ipv6->GetMtu(interface),
      "Transmitted packet size > MTU: packetSizes: " << packet->GetSize());
}

void Ipv6FragmentationTest::DoRun() {

  InternetStackHelper internet;
  internet.SetIpv4StackInstall(false);

  Ptr<Node> serverNode = CreateObject<Node>();
  internet.Install(serverNode);
  Ptr<SimpleNetDevice> serverDev;
  Ptr<BinaryErrorModel> serverDevErrorModel = CreateObject<BinaryErrorModel>();
  {
    serverDev = CreateObject<SimpleNetDevice>();
    serverDev->SetAddress(Mac48Address::ConvertFrom(Mac48Address::Allocate()));
    serverDev->SetMtu(1500);
    serverDev->SetReceiveErrorModel(serverDevErrorModel);
    serverDevErrorModel->Disable();
    serverNode->AddDevice(serverDev);
    Ptr<Ipv6> ipv6 = serverNode->GetObject<Ipv6>();
    uint32_t netdev_idx = ipv6->AddInterface(serverDev);
    Ipv6InterfaceAddress ipv6Addr =
        Ipv6InterfaceAddress(Ipv6Address("2001::1"), Ipv6Prefix(32));
    ipv6->AddAddress(netdev_idx, ipv6Addr);
    ipv6->SetUp(netdev_idx);
    ipv6->TraceConnectWithoutContext(
        "Rx", MakeCallback(&Ipv6FragmentationTest::HandleServerRx, this));
  }
  StartServer(serverNode);

  Ptr<Node> clientNode = CreateObject<Node>();
  internet.Install(clientNode);
  Ptr<SimpleNetDevice> clientDev;
  Ptr<BinaryErrorModel> clientDevErrorModel = CreateObject<BinaryErrorModel>();
  {
    clientDev = CreateObject<SimpleNetDevice>();
    clientDev->SetAddress(Mac48Address::ConvertFrom(Mac48Address::Allocate()));
    clientDev->SetMtu(1500);
    clientDev->SetReceiveErrorModel(clientDevErrorModel);
    clientDevErrorModel->Disable();
    clientNode->AddDevice(clientDev);
    Ptr<Ipv6> ipv6 = clientNode->GetObject<Ipv6>();
    uint32_t netdev_idx = ipv6->AddInterface(clientDev);
    Ipv6InterfaceAddress ipv6Addr =
        Ipv6InterfaceAddress(Ipv6Address("2001::2"), Ipv6Prefix(32));
    ipv6->AddAddress(netdev_idx, ipv6Addr);
    ipv6->SetUp(netdev_idx);
    ipv6->TraceConnectWithoutContext(
        "Tx", MakeCallback(&Ipv6FragmentationTest::HandleClientTx, this));
  }
  StartClient(clientNode);

  Ptr<ErrorChannel> channel = CreateObject<ErrorChannel>();
  serverDev->SetChannel(channel);
  clientDev->SetChannel(channel);
  channel->SetJumpingTime(Seconds(0.5));

  uint32_t packetSizes[5] = {1500, 2000, 5000, 10000, 65000};

  uint8_t fillData[78];
  for (uint32_t k = 48; k <= 125; k++) {
    fillData[k - 48] = k;
  }

  for (int i = 0; i < 5; i++) {
    uint32_t packetSize = packetSizes[i];

    SetFill(fillData, 78, packetSize);

    m_receivedPacketServer = Create<Packet>();
    Simulator::ScheduleWithContext(m_socketClient->GetNode()->GetId(),
                                   Seconds(0),
                                   &Ipv6FragmentationTest::SendClient, this);
    Simulator::Run();

    uint8_t recvBuffer[65000];

    uint16_t recvSize = m_receivedPacketServer->GetSize();

    NS_TEST_EXPECT_MSG_EQ(recvSize, packetSizes[i],
                          "Packet size not correct: recvSize: "
                              << recvSize << " packetSizes[" << i
                              << "]: " << packetSizes[i]);

    m_receivedPacketServer->CopyData(recvBuffer, 65000);
    NS_TEST_EXPECT_MSG_EQ(
        memcmp(m_data, recvBuffer, m_receivedPacketServer->GetSize()), 0,
        "Packet content differs");
  }

  channel->SetJumpingMode(true);
  for (int i = 0; i < 5; i++) {
    uint32_t packetSize = packetSizes[i];

    SetFill(fillData, 78, packetSize);

    m_receivedPacketServer = Create<Packet>();
    Simulator::ScheduleWithContext(m_socketClient->GetNode()->GetId(),
                                   Seconds(0),
                                   &Ipv6FragmentationTest::SendClient, this);
    Simulator::Run();

    uint8_t recvBuffer[65000];

    uint16_t recvSize = m_receivedPacketServer->GetSize();

    NS_TEST_EXPECT_MSG_EQ(recvSize, packetSizes[i],
                          "Packet size not correct: recvSize: "
                              << recvSize << " packetSizes[" << i
                              << "]: " << packetSizes[i]);

    m_receivedPacketServer->CopyData(recvBuffer, 65000);
    NS_TEST_EXPECT_MSG_EQ(
        memcmp(m_data, recvBuffer, m_receivedPacketServer->GetSize()), 0,
        "Packet content differs");
  }
  channel->SetJumpingMode(false);

  clientDevErrorModel->Disable();
  serverDevErrorModel->Enable();
  for (int i = 1; i < 5; i++) {
    uint32_t packetSize = packetSizes[i];

    SetFill(fillData, 78, packetSize);

    serverDevErrorModel->Reset();

    m_receivedPacketServer = Create<Packet>();
    m_icmpType = 0;
    m_icmpCode = 0;
    Simulator::ScheduleWithContext(m_socketClient->GetNode()->GetId(),
                                   Seconds(0),
                                   &Ipv6FragmentationTest::SendClient, this);
    Simulator::Run();

    uint16_t recvSize = m_receivedPacketServer->GetSize();

    NS_TEST_EXPECT_MSG_EQ((recvSize == 0), true,
                          "Server got a packet, something wrong");
    NS_TEST_EXPECT_MSG_EQ(
        (m_icmpType == Icmpv6Header::ICMPV6_ERROR_TIME_EXCEEDED) &&
            (m_icmpCode == Icmpv6Header::ICMPV6_FRAGTIME),
        true,
        "Client did not receive ICMPv6::TIME_EXCEEDED " << int(m_icmpType)
                                                        << int(m_icmpCode));
  }

  clientDevErrorModel->Disable();
  serverDevErrorModel->Disable();
  for (int i = 0; i < 5; i++) {
    uint32_t packetSize = packetSizes[i];

    SetFill(fillData, 78, packetSize);

    m_receivedPacketServer = Create<Packet>();
    Simulator::ScheduleWithContext(m_socketClient->GetNode()->GetId(),
                                   Seconds(0),
                                   &Ipv6FragmentationTest::SendClient, this);
    Simulator::Run();

    IPv6TestTag packetTag;
    bool found = m_receivedPacketServer->PeekPacketTag(packetTag);

    NS_TEST_EXPECT_MSG_EQ(found, true, "PacketTag not found");
    NS_TEST_EXPECT_MSG_EQ(packetTag.GetToken(), 42,
                          "PacketTag value not correct");

    ByteTagIterator iter = m_receivedPacketServer->GetByteTagIterator();

    uint32_t end = 0;
    uint32_t tagStart = 0;
    uint32_t tagEnd = 0;
    while (iter.HasNext()) {
      ByteTagIterator::Item item = iter.Next();
      NS_TEST_EXPECT_MSG_EQ(item.GetTypeId().GetName(), "ns3::IPv6TestTag",
                            "ByteTag name not correct");
      tagStart = item.GetStart();
      tagEnd = item.GetEnd();
      if (end == 0) {
        NS_TEST_EXPECT_MSG_EQ(tagStart, 0, "First ByteTag Start not correct");
      }
      if (end != 0) {
        NS_TEST_EXPECT_MSG_EQ(tagStart, end, "ByteTag End not correct");
      }
      end = tagEnd;
      IPv6TestTag *byteTag =
          dynamic_cast<IPv6TestTag *>(item.GetTypeId().GetConstructor()());
      NS_TEST_EXPECT_MSG_NE(byteTag, 0, "ByteTag not found");
      item.GetTag(*byteTag);
      NS_TEST_EXPECT_MSG_EQ(byteTag->GetToken(), 42,
                            "ByteTag value not correct");
      delete byteTag;
    }
    NS_TEST_EXPECT_MSG_EQ(end, m_receivedPacketServer->GetSize(), "trivial");
  }

  Simulator::Destroy();
}

class Ipv6FragmentationTestSuite : public TestSuite {
public:
  Ipv6FragmentationTestSuite() : TestSuite("ipv6-fragmentation", UNIT) {
    AddTestCase(new Ipv6FragmentationTest, TestCase::QUICK);
  }
};

static Ipv6FragmentationTestSuite g_ipv6fragmentationTestSuite;
