

#include "ns3/address.h"
#include "ns3/application-container.h"
#include "ns3/bridge-helper.h"
#include "ns3/callback.h"
#include "ns3/config.h"
#include "ns3/csma-helper.h"
#include "ns3/csma-star-helper.h"
#include "ns3/data-rate.h"
#include "ns3/inet-socket-address.h"
#include "ns3/internet-stack-helper.h"
#include "ns3/ipv4-address-helper.h"
#include "ns3/ipv4-global-routing-helper.h"
#include "ns3/ipv4-static-routing-helper.h"
#include "ns3/node-container.h"
#include "ns3/node.h"
#include "ns3/on-off-helper.h"
#include "ns3/packet-sink-helper.h"
#include "ns3/packet-socket-address.h"
#include "ns3/packet-socket-helper.h"
#include "ns3/packet.h"
#include "ns3/ping-helper.h"
#include "ns3/pointer.h"
#include "ns3/simple-channel.h"
#include "ns3/simulator.h"
#include "ns3/string.h"
#include "ns3/test.h"
#include "ns3/uinteger.h"

#include <string>

using namespace ns3;

class CsmaBridgeTestCase : public TestCase {
public:
  CsmaBridgeTestCase();
  ~CsmaBridgeTestCase() override;

private:
  void DoRun() override;

  void SinkRx(Ptr<const Packet> p, const Address &ad);
  uint32_t m_count;
};

CsmaBridgeTestCase::CsmaBridgeTestCase()
    : TestCase(
          "Bridge example for Carrier Sense Multiple Access (CSMA) networks"),
      m_count(0) {}

CsmaBridgeTestCase::~CsmaBridgeTestCase() {}

void CsmaBridgeTestCase::SinkRx(Ptr<const Packet> p, const Address &ad) {
  m_count++;
}

void CsmaBridgeTestCase::DoRun() {
  NodeContainer terminals;
  terminals.Create(4);

  NodeContainer csmaSwitch;
  csmaSwitch.Create(1);

  CsmaHelper csma;
  csma.SetChannelAttribute("DataRate", DataRateValue(5000000));
  csma.SetChannelAttribute("Delay", TimeValue(MilliSeconds(2)));

  NetDeviceContainer terminalDevices;
  NetDeviceContainer switchDevices;

  for (int i = 0; i < 4; i++) {
    NetDeviceContainer link =
        csma.Install(NodeContainer(terminals.Get(i), csmaSwitch));
    terminalDevices.Add(link.Get(0));
    switchDevices.Add(link.Get(1));
  }

  Ptr<Node> switchNode = csmaSwitch.Get(0);
  BridgeHelper bridge;
  bridge.Install(switchNode, switchDevices);

  InternetStackHelper internet;
  internet.Install(terminals);

  Ipv4AddressHelper ipv4;
  ipv4.SetBase("10.1.1.0", "255.255.255.0");
  ipv4.Assign(terminalDevices);

  uint16_t port = 9;

  OnOffHelper onoff("ns3::UdpSocketFactory",
                    Address(InetSocketAddress(Ipv4Address("10.1.1.2"), port)));
  onoff.SetConstantRate(DataRate(5000));

  ApplicationContainer app = onoff.Install(terminals.Get(0));
  app.Start(Seconds(1.0));
  app.Stop(Seconds(10.0));

  PacketSinkHelper sink(
      "ns3::UdpSocketFactory",
      Address(InetSocketAddress(Ipv4Address::GetAny(), port)));
  app = sink.Install(terminals.Get(1));
  app.Start(Seconds(0.0));

  Config::ConnectWithoutContext(
      "/NodeList/1/ApplicationList/0/$ns3::PacketSink/Rx",
      MakeCallback(&CsmaBridgeTestCase::SinkRx, this));

  Simulator::Run();
  Simulator::Destroy();

  NS_TEST_ASSERT_MSG_EQ(m_count, 10, "Bridge should have passed 10 packets");
}

class CsmaBroadcastTestCase : public TestCase {
public:
  CsmaBroadcastTestCase();
  ~CsmaBroadcastTestCase() override;

private:
  void DoRun() override;

  void SinkRxNode1(Ptr<const Packet> p, const Address &ad);
  void SinkRxNode2(Ptr<const Packet> p, const Address &ad);

  void DropEvent(Ptr<const Packet> p);

  uint32_t m_countNode1;
  uint32_t m_countNode2;
  uint32_t m_drops;
};

CsmaBroadcastTestCase::CsmaBroadcastTestCase()
    : TestCase("Broadcast example for Carrier Sense Multiple Access (CSMA) "
               "networks"),
      m_countNode1(0), m_countNode2(0), m_drops(0) {}

CsmaBroadcastTestCase::~CsmaBroadcastTestCase() {}

void CsmaBroadcastTestCase::SinkRxNode1(Ptr<const Packet> p,
                                        const Address &ad) {
  m_countNode1++;
}

void CsmaBroadcastTestCase::SinkRxNode2(Ptr<const Packet> p,
                                        const Address &ad) {
  m_countNode2++;
}

void CsmaBroadcastTestCase::DropEvent(Ptr<const Packet> p) { m_drops++; }

void CsmaBroadcastTestCase::DoRun() {
  NodeContainer c;
  c.Create(3);
  NodeContainer c0 = NodeContainer(c.Get(0), c.Get(1));
  NodeContainer c1 = NodeContainer(c.Get(0), c.Get(2));

  CsmaHelper csma;
  csma.SetChannelAttribute("DataRate", DataRateValue(DataRate(5000000)));
  csma.SetChannelAttribute("Delay", TimeValue(MilliSeconds(2)));

  NetDeviceContainer n0 = csma.Install(c0);
  NetDeviceContainer n1 = csma.Install(c1);

  InternetStackHelper internet;
  internet.Install(c);

  Ipv4AddressHelper ipv4;
  ipv4.SetBase("10.1.0.0", "255.255.255.0");
  ipv4.Assign(n0);
  ipv4.SetBase("192.168.1.0", "255.255.255.0");
  ipv4.Assign(n1);

  uint16_t port = 9;

  OnOffHelper onoff(
      "ns3::UdpSocketFactory",
      Address(InetSocketAddress(Ipv4Address("255.255.255.255"), port)));
  onoff.SetConstantRate(DataRate(5000));

  ApplicationContainer app = onoff.Install(c0.Get(0));
  app.Start(Seconds(1.0));
  app.Stop(Seconds(10.0));

  PacketSinkHelper sink(
      "ns3::UdpSocketFactory",
      Address(InetSocketAddress(Ipv4Address::GetAny(), port)));
  app = sink.Install(c0.Get(1));
  app.Add(sink.Install(c1.Get(1)));
  app.Start(Seconds(1.0));
  app.Stop(Seconds(10.0));

  Config::ConnectWithoutContext(
      "/NodeList/1/ApplicationList/0/$ns3::PacketSink/Rx",
      MakeCallback(&CsmaBroadcastTestCase::SinkRxNode1, this));
  Config::ConnectWithoutContext(
      "/NodeList/2/ApplicationList/0/$ns3::PacketSink/Rx",
      MakeCallback(&CsmaBroadcastTestCase::SinkRxNode2, this));

  Simulator::Run();
  Simulator::Destroy();

  NS_TEST_ASSERT_MSG_EQ(m_countNode1, 10,
                        "Node 1 should have received 10 packets");
  NS_TEST_ASSERT_MSG_EQ(m_countNode2, 10,
                        "Node 2 should have received 10 packets");
}

class CsmaMulticastTestCase : public TestCase {
public:
  CsmaMulticastTestCase();
  ~CsmaMulticastTestCase() override;

private:
  void DoRun() override;

  void SinkRx(Ptr<const Packet> p, const Address &ad);

  void DropEvent(Ptr<const Packet> p);

  uint32_t m_count;
  uint32_t m_drops;
};

CsmaMulticastTestCase::CsmaMulticastTestCase()
    : TestCase("Multicast example for Carrier Sense Multiple Access (CSMA) "
               "networks"),
      m_count(0), m_drops(0) {}

CsmaMulticastTestCase::~CsmaMulticastTestCase() {}

void CsmaMulticastTestCase::SinkRx(Ptr<const Packet> p, const Address &ad) {
  m_count++;
}

void CsmaMulticastTestCase::DropEvent(Ptr<const Packet> p) { m_drops++; }

void CsmaMulticastTestCase::DoRun() {
  Config::SetDefault("ns3::CsmaNetDevice::EncapsulationMode",
                     StringValue("Dix"));

  NodeContainer c;
  c.Create(5);
  NodeContainer c0 = NodeContainer(c.Get(0), c.Get(1), c.Get(2));
  NodeContainer c1 = NodeContainer(c.Get(2), c.Get(3), c.Get(4));

  CsmaHelper csma;
  csma.SetChannelAttribute("DataRate", DataRateValue(DataRate(5000000)));
  csma.SetChannelAttribute("Delay", TimeValue(MilliSeconds(2)));

  NetDeviceContainer nd0 = csma.Install(c0);
  NetDeviceContainer nd1 = csma.Install(c1);

  InternetStackHelper internet;
  internet.Install(c);

  Ipv4AddressHelper ipv4Addr;
  ipv4Addr.SetBase("10.1.1.0", "255.255.255.0");
  ipv4Addr.Assign(nd0);
  ipv4Addr.SetBase("10.1.2.0", "255.255.255.0");
  ipv4Addr.Assign(nd1);

  Ipv4Address multicastSource("10.1.1.1");
  Ipv4Address multicastGroup("225.1.2.4");

  Ipv4StaticRoutingHelper multicast;

  Ptr<Node> multicastRouter = c.Get(2);
  Ptr<NetDevice> inputIf = nd0.Get(2);
  NetDeviceContainer outputDevices;
  outputDevices.Add(nd1.Get(0));

  multicast.AddMulticastRoute(multicastRouter, multicastSource, multicastGroup,
                              inputIf, outputDevices);

  Ptr<Node> sender = c.Get(0);
  Ptr<NetDevice> senderIf = nd0.Get(0);
  multicast.SetDefaultMulticastRoute(sender, senderIf);

  uint16_t multicastPort = 9;

  OnOffHelper onoff("ns3::UdpSocketFactory",
                    Address(InetSocketAddress(multicastGroup, multicastPort)));
  onoff.SetConstantRate(DataRate(5000));

  ApplicationContainer srcC = onoff.Install(c0.Get(0));

  srcC.Start(Seconds(1.));
  srcC.Stop(Seconds(10.));

  PacketSinkHelper sink(
      "ns3::UdpSocketFactory",
      InetSocketAddress(Ipv4Address::GetAny(), multicastPort));

  ApplicationContainer sinkC = sink.Install(c1.Get(2));
  sinkC.Start(Seconds(1.0));
  sinkC.Stop(Seconds(10.0));

  Config::ConnectWithoutContext(
      "/NodeList/4/ApplicationList/0/$ns3::PacketSink/Rx",
      MakeCallback(&CsmaMulticastTestCase::SinkRx, this));

  Simulator::Run();
  Simulator::Destroy();

  NS_TEST_ASSERT_MSG_EQ(m_count, 10, "Node 4 should have received 10 packets");
}

class CsmaOneSubnetTestCase : public TestCase {
public:
  CsmaOneSubnetTestCase();
  ~CsmaOneSubnetTestCase() override;

private:
  void DoRun() override;

  void SinkRxNode0(Ptr<const Packet> p, const Address &ad);
  void SinkRxNode1(Ptr<const Packet> p, const Address &ad);

  void DropEvent(Ptr<const Packet> p);
  uint32_t m_countNode0;
  uint32_t m_countNode1;
  uint32_t m_drops;
};

CsmaOneSubnetTestCase::CsmaOneSubnetTestCase()
    : TestCase("One subnet example for Carrier Sense Multiple Access (CSMA) "
               "networks"),
      m_countNode0(0), m_countNode1(0), m_drops(0) {}

CsmaOneSubnetTestCase::~CsmaOneSubnetTestCase() {}

void CsmaOneSubnetTestCase::SinkRxNode0(Ptr<const Packet> p,
                                        const Address &ad) {
  m_countNode0++;
}

void CsmaOneSubnetTestCase::SinkRxNode1(Ptr<const Packet> p,
                                        const Address &ad) {
  m_countNode1++;
}

void CsmaOneSubnetTestCase::DropEvent(Ptr<const Packet> p) { m_drops++; }

void CsmaOneSubnetTestCase::DoRun() {
  NodeContainer nodes;
  nodes.Create(4);

  CsmaHelper csma;
  csma.SetChannelAttribute("DataRate", DataRateValue(5000000));
  csma.SetChannelAttribute("Delay", TimeValue(MilliSeconds(2)));
  NetDeviceContainer devices = csma.Install(nodes);

  InternetStackHelper internet;
  internet.Install(nodes);

  Ipv4AddressHelper ipv4;
  ipv4.SetBase("10.1.1.0", "255.255.255.0");
  Ipv4InterfaceContainer interfaces = ipv4.Assign(devices);

  uint16_t port = 9;

  OnOffHelper onoff("ns3::UdpSocketFactory",
                    Address(InetSocketAddress(interfaces.GetAddress(1), port)));
  onoff.SetConstantRate(DataRate(5000));

  ApplicationContainer app = onoff.Install(nodes.Get(0));
  app.Start(Seconds(1.0));
  app.Stop(Seconds(10.0));

  PacketSinkHelper sink(
      "ns3::UdpSocketFactory",
      Address(InetSocketAddress(Ipv4Address::GetAny(), port)));
  app = sink.Install(nodes.Get(1));
  app.Start(Seconds(0.0));

  onoff.SetAttribute("Remote", AddressValue(InetSocketAddress(
                                   interfaces.GetAddress(0), port)));
  app = onoff.Install(nodes.Get(3));
  app.Start(Seconds(1.1));
  app.Stop(Seconds(10.0));

  app = sink.Install(nodes.Get(0));
  app.Start(Seconds(0.0));

  Config::ConnectWithoutContext(
      "/NodeList/0/ApplicationList/1/$ns3::PacketSink/Rx",
      MakeCallback(&CsmaOneSubnetTestCase::SinkRxNode0, this));
  Config::ConnectWithoutContext(
      "/NodeList/1/ApplicationList/0/$ns3::PacketSink/Rx",
      MakeCallback(&CsmaOneSubnetTestCase::SinkRxNode1, this));

  Simulator::Run();
  Simulator::Destroy();

  NS_TEST_ASSERT_MSG_EQ(m_countNode0, 10,
                        "Node 0 should have received 10 packets");
  NS_TEST_ASSERT_MSG_EQ(m_countNode1, 10,
                        "Node 1 should have received 10 packets");
}

class CsmaPacketSocketTestCase : public TestCase {
public:
  CsmaPacketSocketTestCase();
  ~CsmaPacketSocketTestCase() override;

private:
  void DoRun() override;
  void SinkRx(std::string path, Ptr<const Packet> p, const Address &ad);

  void DropEvent(Ptr<const Packet> p);

  uint32_t m_count;
  uint32_t m_drops;
};

CsmaPacketSocketTestCase::CsmaPacketSocketTestCase()
    : TestCase("Packet socket example for Carrier Sense Multiple Access (CSMA) "
               "networks"),
      m_count(0), m_drops(0) {}

CsmaPacketSocketTestCase::~CsmaPacketSocketTestCase() {}

void CsmaPacketSocketTestCase::SinkRx(std::string path, Ptr<const Packet> p,
                                      const Address &address) {
  m_count++;
}

void CsmaPacketSocketTestCase::DropEvent(Ptr<const Packet> p) { m_drops++; }

void CsmaPacketSocketTestCase::DoRun() {
  NodeContainer nodes;
  nodes.Create(4);

  PacketSocketHelper packetSocket;
  packetSocket.Install(nodes);

  Ptr<CsmaChannel> channel = CreateObjectWithAttributes<CsmaChannel>(
      "DataRate", DataRateValue(DataRate(5000000)), "Delay",
      TimeValue(MilliSeconds(2)));

  CsmaHelper csma;
  csma.SetDeviceAttribute("EncapsulationMode", StringValue("Llc"));
  NetDeviceContainer devs = csma.Install(nodes, channel);

  PacketSocketAddress socket;
  socket.SetSingleDevice(devs.Get(0)->GetIfIndex());
  socket.SetPhysicalAddress(devs.Get(1)->GetAddress());
  socket.SetProtocol(2);
  OnOffHelper onoff("ns3::PacketSocketFactory", Address(socket));
  onoff.SetConstantRate(DataRate(5000));
  ApplicationContainer apps = onoff.Install(nodes.Get(0));
  apps.Start(Seconds(1.0));
  apps.Stop(Seconds(10.0));

  socket.SetSingleDevice(devs.Get(3)->GetIfIndex());
  socket.SetPhysicalAddress(devs.Get(0)->GetAddress());
  socket.SetProtocol(3);
  onoff.SetAttribute("Remote", AddressValue(socket));
  apps = onoff.Install(nodes.Get(3));
  apps.Start(Seconds(1.0));
  apps.Stop(Seconds(10.0));

  PacketSinkHelper sink = PacketSinkHelper("ns3::PacketSocketFactory", socket);
  apps = sink.Install(nodes.Get(0));
  apps.Start(Seconds(0.0));
  apps.Stop(Seconds(20.0));

  Config::Connect("/NodeList/0/ApplicationList/*/$ns3::PacketSink/Rx",
                  MakeCallback(&CsmaPacketSocketTestCase::SinkRx, this));

  Simulator::Run();
  Simulator::Destroy();

  NS_TEST_ASSERT_MSG_EQ(m_count, 10, "Node 0 should have received 10 packets");
}

class CsmaPingTestCase : public TestCase {
public:
  CsmaPingTestCase();
  ~CsmaPingTestCase() override;

private:
  void DoRun() override;
  void SinkRx(Ptr<const Packet>, const Address &);

  void PingRtt(std::string, uint16_t, Time);

  void DropEvent(Ptr<const Packet>);

  uint32_t m_countSinkRx;
  uint32_t m_countPingRtt;
  uint32_t m_drops;
};

CsmaPingTestCase::CsmaPingTestCase()
    : TestCase(
          "Ping example for Carrier Sense Multiple Access (CSMA) networks"),
      m_countSinkRx(0), m_countPingRtt(0), m_drops(0) {}

CsmaPingTestCase::~CsmaPingTestCase() {}

void CsmaPingTestCase::SinkRx(Ptr<const Packet>, const Address &) {
  m_countSinkRx++;
}

void CsmaPingTestCase::PingRtt(std::string, uint16_t, Time) {
  m_countPingRtt++;
}

void CsmaPingTestCase::DropEvent(Ptr<const Packet>) { m_drops++; }

void CsmaPingTestCase::DoRun() {
  NodeContainer c;
  c.Create(4);

  CsmaHelper csma;
  csma.SetChannelAttribute("DataRate", DataRateValue(DataRate(5000000)));
  csma.SetChannelAttribute("Delay", TimeValue(MilliSeconds(2)));
  csma.SetDeviceAttribute("EncapsulationMode", StringValue("Llc"));
  NetDeviceContainer devs = csma.Install(c);

  InternetStackHelper ipStack;
  ipStack.Install(c);

  Ipv4AddressHelper ip;
  ip.SetBase("192.168.1.0", "255.255.255.0");
  Ipv4InterfaceContainer addresses = ip.Assign(devs);

  Config::SetDefault("ns3::Ipv4RawSocketImpl::Protocol", StringValue("2"));
  InetSocketAddress dst(addresses.GetAddress(3));
  OnOffHelper onoff = OnOffHelper("ns3::Ipv4RawSocketFactory", dst);
  onoff.SetConstantRate(DataRate(5000));

  ApplicationContainer apps = onoff.Install(c.Get(0));
  apps.Start(Seconds(1.0));
  apps.Stop(Seconds(10.0));

  PacketSinkHelper sink = PacketSinkHelper("ns3::Ipv4RawSocketFactory", dst);
  apps = sink.Install(c.Get(3));
  apps.Start(Seconds(0.0));
  apps.Stop(Seconds(11.0));

  PingHelper ping(addresses.GetAddress(2));
  NodeContainer pingers;
  pingers.Add(c.Get(0));
  pingers.Add(c.Get(1));
  pingers.Add(c.Get(3));
  apps = ping.Install(pingers);
  apps.Start(Seconds(2.0));
  apps.Stop(Seconds(5.0));

  Config::ConnectWithoutContext(
      "/NodeList/3/ApplicationList/0/$ns3::PacketSink/Rx",
      MakeCallback(&CsmaPingTestCase::SinkRx, this));

  Config::Connect("/NodeList/*/ApplicationList/*/$ns3::Ping/Rtt",
                  MakeCallback(&CsmaPingTestCase::PingRtt, this));

  Simulator::Run();
  Simulator::Destroy();

  NS_TEST_ASSERT_MSG_EQ(m_countSinkRx, 10,
                        "Node 3 should have received 10 packets");

  NS_TEST_ASSERT_MSG_EQ(m_countPingRtt, 9,
                        "Node 2 should have been pinged 9 times");
}

class CsmaRawIpSocketTestCase : public TestCase {
public:
  CsmaRawIpSocketTestCase();
  ~CsmaRawIpSocketTestCase() override;

private:
  void DoRun() override;

  void SinkRx(Ptr<const Packet> p, const Address &ad);

  void DropEvent(Ptr<const Packet> p);

  uint32_t m_count;
  uint32_t m_drops;
};

CsmaRawIpSocketTestCase::CsmaRawIpSocketTestCase()
    : TestCase("Raw internet protocol socket example for Carrier Sense "
               "Multiple Access (CSMA) networks"),
      m_count(0), m_drops(0) {}

CsmaRawIpSocketTestCase::~CsmaRawIpSocketTestCase() {}

void CsmaRawIpSocketTestCase::SinkRx(Ptr<const Packet> p, const Address &ad) {
  m_count++;
}

void CsmaRawIpSocketTestCase::DropEvent(Ptr<const Packet> p) { m_drops++; }

void CsmaRawIpSocketTestCase::DoRun() {
  NodeContainer c;
  c.Create(4);

  CsmaHelper csma;
  csma.SetChannelAttribute("DataRate", DataRateValue(DataRate(5000000)));
  csma.SetChannelAttribute("Delay", TimeValue(MilliSeconds(2)));
  csma.SetDeviceAttribute("EncapsulationMode", StringValue("Llc"));
  NetDeviceContainer devs = csma.Install(c);

  InternetStackHelper ipStack;
  ipStack.Install(c);

  Ipv4AddressHelper ip;
  ip.SetBase("192.168.1.0", "255.255.255.0");
  Ipv4InterfaceContainer addresses = ip.Assign(devs);

  Config::SetDefault("ns3::Ipv4RawSocketImpl::Protocol", StringValue("2"));
  InetSocketAddress dst(addresses.GetAddress(3));
  OnOffHelper onoff = OnOffHelper("ns3::Ipv4RawSocketFactory", dst);
  onoff.SetConstantRate(DataRate(5000));

  ApplicationContainer apps = onoff.Install(c.Get(0));
  apps.Start(Seconds(1.0));
  apps.Stop(Seconds(10.0));

  PacketSinkHelper sink = PacketSinkHelper("ns3::Ipv4RawSocketFactory", dst);
  apps = sink.Install(c.Get(3));
  apps.Start(Seconds(0.0));
  apps.Stop(Seconds(12.0));

  Config::ConnectWithoutContext(
      "/NodeList/3/ApplicationList/0/$ns3::PacketSink/Rx",
      MakeCallback(&CsmaRawIpSocketTestCase::SinkRx, this));

  Simulator::Run();
  Simulator::Destroy();

  NS_TEST_ASSERT_MSG_EQ(m_count, 10, "Node 3 should have received 10 packets");
}

class CsmaStarTestCase : public TestCase {
public:
  CsmaStarTestCase();
  ~CsmaStarTestCase() override;

private:
  void DoRun() override;

  void SinkRx(Ptr<const Packet> p, const Address &ad);

  void DropEvent(Ptr<const Packet> p);

  uint32_t m_count;
  uint32_t m_drops;
};

CsmaStarTestCase::CsmaStarTestCase()
    : TestCase(
          "Star example for Carrier Sense Multiple Access (CSMA) networks"),
      m_count(0), m_drops(0) {}

CsmaStarTestCase::~CsmaStarTestCase() {}

void CsmaStarTestCase::SinkRx(Ptr<const Packet> p, const Address &ad) {
  m_count++;
}

void CsmaStarTestCase::DropEvent(Ptr<const Packet> p) { m_drops++; }

void CsmaStarTestCase::DoRun() {
  uint32_t nSpokes = 7;

  CsmaHelper csma;
  csma.SetChannelAttribute("DataRate", StringValue("100Mbps"));
  csma.SetChannelAttribute("Delay", StringValue("1ms"));
  CsmaStarHelper star(nSpokes, csma);

  NodeContainer fillNodes;

  NetDeviceContainer fillDevices;

  uint32_t nFill = 14;
  for (uint32_t i = 0; i < star.GetSpokeDevices().GetN(); ++i) {
    Ptr<Channel> channel = star.GetSpokeDevices().Get(i)->GetChannel();
    Ptr<CsmaChannel> csmaChannel = channel->GetObject<CsmaChannel>();
    NodeContainer newNodes;
    newNodes.Create(nFill);
    fillNodes.Add(newNodes);
    fillDevices.Add(csma.Install(newNodes, csmaChannel));
  }

  InternetStackHelper internet;
  star.InstallStack(internet);
  internet.Install(fillNodes);

  star.AssignIpv4Addresses(Ipv4AddressHelper("10.1.0.0", "255.255.255.0"));

  Ipv4AddressHelper address;
  for (uint32_t i = 0; i < star.SpokeCount(); ++i) {
    std::ostringstream subnet;
    subnet << "10.1." << i << ".0";
    address.SetBase(subnet.str().c_str(), "255.255.255.0", "0.0.0.3");

    for (uint32_t j = 0; j < nFill; ++j) {
      address.Assign(fillDevices.Get(i * nFill + j));
    }
  }

  uint16_t port = 50000;
  Address hubLocalAddress(InetSocketAddress(Ipv4Address::GetAny(), port));
  PacketSinkHelper packetSinkHelper("ns3::TcpSocketFactory", hubLocalAddress);
  ApplicationContainer hubApp = packetSinkHelper.Install(star.GetHub());
  hubApp.Start(Seconds(1.0));
  hubApp.Stop(Seconds(10.0));

  OnOffHelper onOffHelper("ns3::TcpSocketFactory", Address());
  onOffHelper.SetConstantRate(DataRate(5000));

  ApplicationContainer spokeApps;

  for (uint32_t i = 0; i < star.SpokeCount(); ++i) {
    AddressValue remoteAddress(
        InetSocketAddress(star.GetHubIpv4Address(i), port));
    onOffHelper.SetAttribute("Remote", remoteAddress);
    spokeApps.Add(onOffHelper.Install(star.GetSpokeNode(i)));
  }

  spokeApps.Start(Seconds(1.0));
  spokeApps.Stop(Seconds(10.0));

  ApplicationContainer fillApps;

  for (uint32_t i = 0; i < fillNodes.GetN(); ++i) {
    AddressValue remoteAddress(
        InetSocketAddress(star.GetHubIpv4Address(i / nFill), port));
    onOffHelper.SetAttribute("Remote", remoteAddress);
    fillApps.Add(onOffHelper.Install(fillNodes.Get(i)));
  }

  fillApps.Start(Seconds(1.0));
  fillApps.Stop(Seconds(10.0));

  Ipv4GlobalRoutingHelper::PopulateRoutingTables();

  Config::ConnectWithoutContext(
      "/NodeList/0/ApplicationList/*/$ns3::PacketSink/Rx",
      MakeCallback(&CsmaStarTestCase::SinkRx, this));

  Simulator::Run();
  Simulator::Destroy();

  NS_TEST_ASSERT_MSG_EQ(
      m_count, 10 * (nSpokes * (nFill + 1)),
      "Hub node did not receive the proper number of packets");
}

class CsmaSystemTestSuite : public TestSuite {
public:
  CsmaSystemTestSuite();
};

CsmaSystemTestSuite::CsmaSystemTestSuite() : TestSuite("csma-system", UNIT) {
  AddTestCase(new CsmaBridgeTestCase, TestCase::QUICK);
  AddTestCase(new CsmaBroadcastTestCase, TestCase::QUICK);
  AddTestCase(new CsmaMulticastTestCase, TestCase::QUICK);
  AddTestCase(new CsmaOneSubnetTestCase, TestCase::QUICK);
  AddTestCase(new CsmaPacketSocketTestCase, TestCase::QUICK);
  AddTestCase(new CsmaPingTestCase, TestCase::QUICK);
  AddTestCase(new CsmaRawIpSocketTestCase, TestCase::QUICK);
  AddTestCase(new CsmaStarTestCase, TestCase::QUICK);
}

static CsmaSystemTestSuite csmaSystemTestSuite;
