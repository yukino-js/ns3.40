
#include "ns3/boolean.h"
#include "ns3/config.h"
#include "ns3/double.h"
#include "ns3/inet-socket-address.h"
#include "ns3/internet-stack-helper.h"
#include "ns3/ipv4-address-helper.h"
#include "ns3/ipv4-l3-protocol.h"
#include "ns3/ipv4-list-routing-helper.h"
#include "ns3/ipv4-static-routing-helper.h"
#include "ns3/ipv4-static-routing.h"
#include "ns3/log.h"
#include "ns3/names.h"
#include "ns3/node.h"
#include "ns3/random-variable-stream.h"
#include "ns3/simple-channel.h"
#include "ns3/simple-net-device-helper.h"
#include "ns3/simple-net-device.h"
#include "ns3/simulator.h"
#include "ns3/socket.h"
#include "ns3/string.h"
#include "ns3/test.h"
#include "ns3/traffic-control-layer.h"
#include "ns3/udp-socket-factory.h"
#include "ns3/udp-socket.h"
#include "ns3/uinteger.h"

#include <functional>
#include <limits>
#include <string>

using namespace ns3;

class Ipv4DeduplicationTest : public TestCase {
  void DoSendData(Ptr<Socket> socket, std::string to);
  void DoSendPacket(Ptr<Socket> socket, Ptr<Packet> packet, std::string to);
  void SendData(Ptr<Socket> socket, std::string to);

  void SendPacket(Ptr<Socket> socket, Ptr<Packet> packet, std::string to);

  void CheckPackets(const std::string &name);

  void CheckDrops(const std::string &name);

  static const Time DELAY;

  static std::string MakeName(bool enable, Time expire);

  enum MODE { ENABLED = 0, DISABLED, DEGENERATE };

  MODE m_mode;
  Time m_expire;
  std::map<std::string, uint32_t> m_packetCountMap;
  std::map<std::string, uint32_t> m_dropCountMap;

public:
  void DoRun() override;

  Ipv4DeduplicationTest(bool enable, Time expire = Seconds(1));

  void ReceivePkt(Ptr<Socket> socket);

  void DropPkt(const Ipv4Header &ipHeader, Ptr<const Packet> packet,
               Ipv4L3Protocol::DropReason reason, Ptr<Ipv4> ipv4,
               uint32_t interface);
};

const Time Ipv4DeduplicationTest::DELAY = MilliSeconds(1);

Ipv4DeduplicationTest::Ipv4DeduplicationTest(bool enable, Time expire)
    : TestCase(MakeName(enable, expire)), m_mode(ENABLED), m_expire(expire) {
  if (!enable) {
    m_mode = DISABLED;
  } else if (m_expire < DELAY) {
    m_mode = DEGENERATE;
  }
}

void Ipv4DeduplicationTest::ReceivePkt(Ptr<Socket> socket) {
  uint32_t availableData;
  availableData = socket->GetRxAvailable();
  Ptr<Packet> packet = socket->Recv(std::numeric_limits<uint32_t>::max(), 0);
  NS_TEST_ASSERT_MSG_EQ(
      availableData, packet->GetSize(),
      "Received packet size is not equal to the Rx buffer size");

  auto node = socket->GetNode();
  std::string name = Names::FindName(node);
  m_packetCountMap.insert({name, 0});
  ++m_packetCountMap[name];
}

void Ipv4DeduplicationTest::DropPkt(const Ipv4Header &ipHeader,
                                    Ptr<const Packet> packet,
                                    Ipv4L3Protocol::DropReason reason,
                                    Ptr<Ipv4> ipv4, uint32_t interface) {
  switch (m_mode) {
  case ENABLED:
    NS_TEST_EXPECT_MSG_EQ(reason, Ipv4L3Protocol::DROP_DUPLICATE,
                          "Wrong reason for drop");
    break;
  case DISABLED:
    NS_TEST_EXPECT_MSG_EQ(reason, Ipv4L3Protocol::DROP_TTL_EXPIRED,
                          "Wrong reason for drop");
    break;
  case DEGENERATE:
    break;
  };
  auto node = ipv4->GetNetDevice(interface)->GetNode();
  std::string name = Names::FindName(node);
  m_dropCountMap.insert({name, 0});
  ++m_dropCountMap[name];
}

void Ipv4DeduplicationTest::DoSendData(Ptr<Socket> socket, std::string to) {
  SendPacket(socket, Create<Packet>(123), to);
}

void Ipv4DeduplicationTest::DoSendPacket(Ptr<Socket> socket, Ptr<Packet> packet,
                                         std::string to) {
  Address realTo = InetSocketAddress(Ipv4Address(to.c_str()), 1234);
  NS_TEST_EXPECT_MSG_EQ(socket->SendTo(packet, 0, realTo), 123, "100");
}

void Ipv4DeduplicationTest::SendData(Ptr<Socket> socket, std::string to) {
  DoSendData(socket, to);
}

void Ipv4DeduplicationTest::SendPacket(Ptr<Socket> socket, Ptr<Packet> packet,
                                       std::string to) {
  Simulator::ScheduleWithContext(socket->GetNode()->GetId(), MilliSeconds(50),
                                 &Ipv4DeduplicationTest::DoSendPacket, this,
                                 socket, packet, to);
}

std::string Ipv4DeduplicationTest::MakeName(bool enabled, Time expire) {
  std::ostringstream oss;
  oss << "IP v4 RFC 6621 De-duplication: ";
  if (!enabled) {
    oss << "disabled";
  } else if (expire > DELAY) {
    oss << "enabled";
  } else {
    oss << "degenerate";
  }
  oss << ", expire = " << expire.ToDouble(Time::MS) << "ms";

  return oss.str();
}

void Ipv4DeduplicationTest::DoRun() {
  const std::string targetAddr = "239.192.100.1";
  Config::SetDefault("ns3::Ipv4L3Protocol::EnableDuplicatePacketDetection",
                     BooleanValue(m_mode != DISABLED));
  Config::SetDefault("ns3::Ipv4L3Protocol::DuplicateExpire",
                     TimeValue(m_expire));

  auto nodes = NodeContainer();
  nodes.Create(5);

  Names::Add("A", nodes.Get(0));
  Names::Add("B", nodes.Get(1));
  Names::Add("C", nodes.Get(2));
  Names::Add("D", nodes.Get(3));
  Names::Add("E", nodes.Get(4));

  SimpleNetDeviceHelper simplenet;
  auto devices = simplenet.Install(nodes);
  Names::Add("A/dev", devices.Get(0));
  Names::Add("B/dev", devices.Get(1));
  Names::Add("C/dev", devices.Get(2));
  Names::Add("D/dev", devices.Get(3));
  Names::Add("E/dev", devices.Get(4));

  Ipv4ListRoutingHelper listRouting;
  Ipv4StaticRoutingHelper staticRouting;
  listRouting.Add(staticRouting, 0);

  InternetStackHelper internet;
  internet.SetIpv6StackInstall(false);
  internet.SetIpv4ArpJitter(true);
  internet.SetRoutingHelper(listRouting);
  internet.Install(nodes);

  Ipv4AddressHelper ipv4address;
  ipv4address.SetBase("10.0.0.0", "255.255.255.0");
  ipv4address.Assign(devices);

  auto diter = devices.Begin();
  for (auto end = nodes.End(), iter = nodes.Begin(); iter != end; ++iter) {
    staticRouting.AddMulticastRoute(*iter, Ipv4Address::GetAny(),
                                    targetAddr.c_str(), *diter,
                                    NetDeviceContainer(*diter));

    auto ipv4 = (*iter)->GetObject<Ipv4>();
    NS_TEST_ASSERT_MSG_EQ((bool)ipv4, true,
                          "Node " << Names::FindName(*iter)
                                  << " does not have Ipv4 aggregate");
    auto routing = staticRouting.GetStaticRouting(ipv4);
    routing->AddHostRouteTo(targetAddr.c_str(),
                            ipv4->GetInterfaceForDevice(*diter), 0);

    ++diter;
  }

  auto channel = devices.Get(0)->GetChannel();
  auto simplechannel = channel->GetObject<SimpleChannel>();
  simplechannel->SetAttribute("Delay", TimeValue(DELAY));
  simplechannel->BlackList(Names::Find<SimpleNetDevice>("A/dev"),
                           Names::Find<SimpleNetDevice>("D/dev"));
  simplechannel->BlackList(Names::Find<SimpleNetDevice>("D/dev"),
                           Names::Find<SimpleNetDevice>("A/dev"));

  simplechannel->BlackList(Names::Find<SimpleNetDevice>("A/dev"),
                           Names::Find<SimpleNetDevice>("E/dev"));
  simplechannel->BlackList(Names::Find<SimpleNetDevice>("E/dev"),
                           Names::Find<SimpleNetDevice>("A/dev"));

  simplechannel->BlackList(Names::Find<SimpleNetDevice>("B/dev"),
                           Names::Find<SimpleNetDevice>("E/dev"));
  simplechannel->BlackList(Names::Find<SimpleNetDevice>("E/dev"),
                           Names::Find<SimpleNetDevice>("B/dev"));

  simplechannel->BlackList(Names::Find<SimpleNetDevice>("C/dev"),
                           Names::Find<SimpleNetDevice>("E/dev"));
  simplechannel->BlackList(Names::Find<SimpleNetDevice>("E/dev"),
                           Names::Find<SimpleNetDevice>("C/dev"));

  std::list<Ptr<Socket>> sockets;
  for (auto end = nodes.End(), iter = nodes.Begin(); iter != end; ++iter) {
    auto SocketFactory = (*iter)->GetObject<UdpSocketFactory>();
    auto socket = SocketFactory->CreateSocket();
    socket->SetAllowBroadcast(true);
    NS_TEST_EXPECT_MSG_EQ(
        socket->Bind(InetSocketAddress(Ipv4Address::GetAny(), 1234)), 0,
        "Could not bind socket for node " << Names::FindName(*iter));

    auto udpSocket = socket->GetObject<UdpSocket>();
    udpSocket->MulticastJoinGroup(0, Ipv4Address(targetAddr.c_str()));
    udpSocket->SetAttribute("IpMulticastTtl", StringValue("4"));

    socket->SetRecvCallback(
        MakeCallback(&Ipv4DeduplicationTest::ReceivePkt, this));
    sockets.push_back(socket);
  }

  Config::ConnectWithoutContext(
      "/NodeList/*/$ns3::Ipv4L3Protocol/Drop",
      MakeCallback(&Ipv4DeduplicationTest::DropPkt, this));

  auto txSocket = sockets.front();

  SendData(txSocket, targetAddr);
  Simulator::Run();
  Simulator::Destroy();

  for (auto end = nodes.End(), iter = nodes.Begin(); iter != end; ++iter) {
    std::string name = Names::FindName(*iter);
    CheckPackets(name);
    CheckDrops(name);
  }

  m_packetCountMap.clear();
  sockets.clear();
  Names::Clear();
}

void Ipv4DeduplicationTest::CheckPackets(const std::string &name) {
  std::map<std::string, uint32_t> packets = {
      {"A", 14}, {"B", 16}, {"C", 16}, {"D", 16}, {"E", 4},
  };

  std::map<std::string, uint32_t> packetsDuped = {
      {"A", 0}, {"B", 1}, {"C", 1}, {"D", 1}, {"E", 1},
  };
  std::map<std::string, uint32_t> degenerates = {
      {"A", 3}, {"B", 4}, {"C", 4}, {"D", 3}, {"E", 2},
  };

  switch (m_mode) {
  case ENABLED:
    NS_TEST_ASSERT_MSG_EQ(m_packetCountMap[name], packetsDuped[name],
                          "Wrong number of packets received for node " << name);
    break;
  case DISABLED:
    NS_TEST_EXPECT_MSG_EQ(m_packetCountMap[name], packets[name],
                          "Wrong number of packets received for node " << name);
    break;
  case DEGENERATE:
    NS_TEST_EXPECT_MSG_EQ(m_packetCountMap[name], degenerates[name],
                          "Wrong number of packets received for node " << name);
    break;
  };
}

void Ipv4DeduplicationTest::CheckDrops(const std::string &name) {
  std::map<std::string, uint32_t> drops;
  switch (m_mode) {
  case ENABLED:
    drops = {{"A", 2}, {"B", 2}, {"C", 2}, {"D", 2}, {"E", 0}};
    break;
  case DISABLED:
    drops = {{"A", 10}, {"B", 9}, {"C", 9}, {"D", 12}, {"E", 2}};
    break;
  case DEGENERATE:
    drops = {{"A", 4}, {"B", 5}, {"C", 5}, {"D", 5}, {"E", 1}};
    break;
  }

  if (drops[name]) {
    NS_TEST_ASSERT_MSG_NE((m_dropCountMap.find(name) == m_dropCountMap.end()),
                          true, "No drops for node " << name);
    NS_TEST_EXPECT_MSG_EQ(m_dropCountMap[name], drops[name],
                          "Wrong number of drops for node " << name);
  } else {
    NS_TEST_EXPECT_MSG_EQ((m_dropCountMap.find(name) == m_dropCountMap.end()),
                          true, "Non-0 drops for node " << name);
  }
}

class Ipv4DeduplicationTestSuite : public TestSuite {
public:
  Ipv4DeduplicationTestSuite();

private:
};

Ipv4DeduplicationTestSuite::Ipv4DeduplicationTestSuite()
    : TestSuite("ipv4-deduplication", UNIT) {
  AddTestCase(new Ipv4DeduplicationTest(true), TestCase::QUICK);
  AddTestCase(new Ipv4DeduplicationTest(false), TestCase::QUICK);
  AddTestCase(new Ipv4DeduplicationTest(true, MicroSeconds(50)),
              TestCase::QUICK);
}

static Ipv4DeduplicationTestSuite g_ipv4DeduplicationTestSuite;

class Ipv4DeduplicationPerformanceTest : public TestCase {
public:
  Ipv4DeduplicationPerformanceTest();
  void DoRun() override;

private:
  std::vector<Ptr<Socket>> m_sockets;
  std::vector<uint8_t> m_txPackets;
  uint8_t m_target;

  void DoSendData(Ptr<Socket> socket, Address to, uint8_t socketIndex);
};

Ipv4DeduplicationPerformanceTest::Ipv4DeduplicationPerformanceTest()
    : TestCase("Ipv4Deduplication performance test") {
  m_target = 40;
}

void Ipv4DeduplicationPerformanceTest::DoRun() {
  const std::string targetAddr = "239.192.100.1";
  Config::SetDefault("ns3::Ipv4L3Protocol::EnableDuplicatePacketDetection",
                     BooleanValue(true));
  Config::SetDefault("ns3::Ipv4L3Protocol::DuplicateExpire",
                     TimeValue(Time("10s")));

  auto nodes = NodeContainer();
  nodes.Create(20);

  SimpleNetDeviceHelper simplenet;
  auto devices = simplenet.Install(nodes);

  Ipv4ListRoutingHelper listRouting;
  Ipv4StaticRoutingHelper staticRouting;
  listRouting.Add(staticRouting, 0);

  InternetStackHelper internet;
  internet.SetIpv6StackInstall(false);
  internet.SetIpv4ArpJitter(true);
  internet.SetRoutingHelper(listRouting);
  internet.Install(nodes);

  Ipv4AddressHelper ipv4address;
  ipv4address.SetBase("10.0.0.0", "255.255.255.0");
  ipv4address.Assign(devices);

  auto diter = devices.Begin();
  for (auto iter = nodes.Begin(); iter != nodes.End(); ++iter) {
    staticRouting.AddMulticastRoute(*iter, Ipv4Address::GetAny(),
                                    targetAddr.c_str(), *diter,
                                    NetDeviceContainer(*diter));

    auto ipv4 = (*iter)->GetObject<Ipv4>();
    NS_TEST_ASSERT_MSG_EQ((bool)ipv4, true,
                          "Node " << (*iter)->GetId()
                                  << " does not have Ipv4 aggregate");
    auto routing = staticRouting.GetStaticRouting(ipv4);
    routing->AddHostRouteTo(targetAddr.c_str(),
                            ipv4->GetInterfaceForDevice(*diter), 0);

    ++diter;
  }

  Ptr<UniformRandomVariable> jitter =
      CreateObjectWithAttributes<UniformRandomVariable>("Max", DoubleValue(4));
  Address to = InetSocketAddress(Ipv4Address(targetAddr.c_str()), 1234);
  for (auto iter = nodes.Begin(); iter != nodes.End(); ++iter) {
    Ptr<SocketFactory> udpSocketFactory =
        (*iter)->GetObject<UdpSocketFactory>();
    m_sockets.push_back(udpSocketFactory->CreateSocket());
    m_txPackets.push_back(0);
  }

  for (uint32_t i = 0; i < nodes.GetN(); i++) {
    Simulator::ScheduleWithContext(
        m_sockets[i]->GetNode()->GetId(), Seconds(4 + jitter->GetValue()),
        &Ipv4DeduplicationPerformanceTest::DoSendData, this, m_sockets[i], to,
        i);
  }

  Simulator::Run();
  NS_LOG_UNCOND("Executed " << Simulator::GetEventCount() << " events");

  for (auto iter = m_sockets.begin(); iter != m_sockets.end(); iter++) {
    (*iter)->Close();
  }

  Simulator::Destroy();
}

void Ipv4DeduplicationPerformanceTest::DoSendData(Ptr<Socket> socket,
                                                  Address to,
                                                  uint8_t socketIndex) {
  socket->SendTo(Create<Packet>(512), 0, to);
  if (m_txPackets[socketIndex] < m_target) {
    m_txPackets[socketIndex] += 1;
    Simulator::ScheduleWithContext(
        m_sockets[socketIndex]->GetNode()->GetId(), Seconds(.5),
        &Ipv4DeduplicationPerformanceTest::DoSendData, this,
        m_sockets[socketIndex], to, socketIndex);
  }
}

class Ipv4DeduplicationPerformanceTestSuite : public TestSuite {
public:
  Ipv4DeduplicationPerformanceTestSuite();
};

Ipv4DeduplicationPerformanceTestSuite::Ipv4DeduplicationPerformanceTestSuite()
    : TestSuite("ipv4-deduplication-performance", PERFORMANCE) {
  AddTestCase(new Ipv4DeduplicationPerformanceTest, TestCase::EXTENSIVE);
}

static Ipv4DeduplicationPerformanceTestSuite
    g_ipv4DeduplicationPerformanceTestSuite;
