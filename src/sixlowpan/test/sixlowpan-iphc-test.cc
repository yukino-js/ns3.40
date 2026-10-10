
#include "ns3/boolean.h"
#include "ns3/inet6-socket-address.h"
#include "ns3/internet-stack-helper.h"
#include "ns3/log.h"
#include "ns3/node.h"
#include "ns3/simple-channel.h"
#include "ns3/simple-net-device.h"
#include "ns3/simulator.h"
#include "ns3/sixlowpan-net-device.h"
#include "ns3/socket-factory.h"
#include "ns3/socket.h"
#include "ns3/test.h"
#include "ns3/udp-socket-factory.h"

#include <limits>
#include <string>

using namespace ns3;

class SixlowpanIphcImplTest : public TestCase {
  Ptr<Packet> m_receivedPacket;

  void DoSendData(Ptr<Socket> socket, std::string to);

  void SendData(Ptr<Socket> socket, std::string to);

public:
  void DoRun() override;
  SixlowpanIphcImplTest();

  void ReceivePacket(Ptr<Socket> socket, Ptr<Packet> packet,
                     const Address &from);
  void ReceivePkt(Ptr<Socket> socket);
};

SixlowpanIphcImplTest::SixlowpanIphcImplTest()
    : TestCase("Sixlowpan implementation") {}

void SixlowpanIphcImplTest::ReceivePacket(Ptr<Socket> socket,
                                          Ptr<Packet> packet,
                                          const Address &from) {
  m_receivedPacket = packet;
}

void SixlowpanIphcImplTest::ReceivePkt(Ptr<Socket> socket) {
  uint32_t availableData [[maybe_unused]] = socket->GetRxAvailable();
  m_receivedPacket = socket->Recv(std::numeric_limits<uint32_t>::max(), 0);
  NS_ASSERT(availableData == m_receivedPacket->GetSize());
}

void SixlowpanIphcImplTest::DoSendData(Ptr<Socket> socket, std::string to) {
  Address realTo = Inet6SocketAddress(Ipv6Address(to.c_str()), 1234);
  uint8_t buffer[] = "\"Can you tell me where my country lies?\" \\ said the "
                     "unifaun to his true "
                     "love's eyes. \\ \"It lies with me!\" cried the Queen of "
                     "Maybe \\ - for her "
                     "merchandise, he traded in his prize.";

  Ptr<Packet> packet = Create<Packet>(buffer, 180);
  NS_TEST_EXPECT_MSG_EQ(socket->SendTo(packet, 0, realTo), 180, "200");
}

void SixlowpanIphcImplTest::SendData(Ptr<Socket> socket, std::string to) {
  m_receivedPacket = Create<Packet>();
  Simulator::ScheduleWithContext(socket->GetNode()->GetId(), Seconds(0),
                                 &SixlowpanIphcImplTest::DoSendData, this,
                                 socket, to);
  Simulator::Run();
}

void SixlowpanIphcImplTest::DoRun() {
  InternetStackHelper internet;
  internet.SetIpv4StackInstall(false);

  Ptr<Node> rxNode = CreateObject<Node>();
  internet.Install(rxNode);
  Ptr<SimpleNetDevice> rxDev;
  {
    rxDev = CreateObject<SimpleNetDevice>();
    rxDev->SetAddress(Mac48Address::ConvertFrom(Mac48Address::Allocate()));
    rxNode->AddDevice(rxDev);

    Ptr<SixLowPanNetDevice> rxSix = CreateObject<SixLowPanNetDevice>();
    rxSix->SetAttribute("ForceEtherType", BooleanValue(true));
    rxNode->AddDevice(rxSix);
    rxSix->SetNetDevice(rxDev);

    Ptr<Ipv6> ipv6 = rxNode->GetObject<Ipv6>();
    ipv6->AddInterface(rxDev);
    uint32_t netdev_idx = ipv6->AddInterface(rxSix);
    Ipv6InterfaceAddress ipv6Addr =
        Ipv6InterfaceAddress(Ipv6Address("2001:0100::1"), Ipv6Prefix(64));
    ipv6->AddAddress(netdev_idx, ipv6Addr);
    ipv6->SetUp(netdev_idx);
  }

  Ptr<Node> txNode = CreateObject<Node>();
  internet.Install(txNode);
  Ptr<SimpleNetDevice> txDev;
  {
    txDev = CreateObject<SimpleNetDevice>();
    txDev->SetAddress(Mac48Address::ConvertFrom(Mac48Address::Allocate()));
    txNode->AddDevice(txDev);

    Ptr<SixLowPanNetDevice> txSix = CreateObject<SixLowPanNetDevice>();
    txSix->SetAttribute("ForceEtherType", BooleanValue(true));
    txNode->AddDevice(txSix);
    txSix->SetNetDevice(txDev);

    Ptr<Ipv6> ipv6 = txNode->GetObject<Ipv6>();
    ipv6->AddInterface(txDev);
    uint32_t netdev_idx = ipv6->AddInterface(txSix);
    Ipv6InterfaceAddress ipv6Addr =
        Ipv6InterfaceAddress(Ipv6Address("2001:0100::2"), Ipv6Prefix(64));
    ipv6->AddAddress(netdev_idx, ipv6Addr);
    ipv6->SetUp(netdev_idx);
  }

  Ptr<SimpleChannel> channel1 = CreateObject<SimpleChannel>();
  rxDev->SetChannel(channel1);
  txDev->SetChannel(channel1);

  Ptr<SocketFactory> rxSocketFactory = rxNode->GetObject<UdpSocketFactory>();
  Ptr<Socket> rxSocket = rxSocketFactory->CreateSocket();
  NS_TEST_EXPECT_MSG_EQ(
      rxSocket->Bind(Inet6SocketAddress(Ipv6Address("2001:0100::1"), 1234)), 0,
      "trivial");
  rxSocket->SetRecvCallback(
      MakeCallback(&SixlowpanIphcImplTest::ReceivePkt, this));

  Ptr<SocketFactory> txSocketFactory = txNode->GetObject<UdpSocketFactory>();
  Ptr<Socket> txSocket = txSocketFactory->CreateSocket();
  txSocket->SetAllowBroadcast(true);

  SendData(txSocket, "2001:0100::1");
  NS_TEST_EXPECT_MSG_EQ(m_receivedPacket->GetSize(), 180, "trivial");
  uint8_t rxBuffer[180];
  uint8_t txBuffer[180] =
      "\"Can you tell me where my country lies?\" \\ said the unifaun to his "
      "true love's eyes. \\ \"It lies with me!\" cried the Queen of Maybe \\ "
      "- for her merchandise, he traded in his prize.";
  m_receivedPacket->CopyData(rxBuffer, 180);
  NS_TEST_EXPECT_MSG_EQ(memcmp(rxBuffer, txBuffer, 180), 0, "trivial");

  m_receivedPacket->RemoveAllByteTags();

  Simulator::Destroy();
}

class SixlowpanIphcTestSuite : public TestSuite {
public:
  SixlowpanIphcTestSuite();

private:
};

SixlowpanIphcTestSuite::SixlowpanIphcTestSuite()
    : TestSuite("sixlowpan-iphc", UNIT) {
  AddTestCase(new SixlowpanIphcImplTest(), TestCase::QUICK);
}

static SixlowpanIphcTestSuite g_sixlowpanIphcTestSuite;
