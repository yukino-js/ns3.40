
#include <ns3/basic-data-calculators.h>
#include <ns3/config.h>
#include <ns3/error-model.h>
#include <ns3/integer.h>
#include <ns3/internet-stack-helper.h>
#include <ns3/ipv4-address-helper.h>
#include <ns3/ipv6-address-helper.h>
#include <ns3/log.h>
#include <ns3/mac48-address.h>
#include <ns3/node.h>
#include <ns3/nstime.h>
#include <ns3/packet.h>
#include <ns3/ptr.h>
#include <ns3/simple-channel.h>
#include <ns3/simple-net-device.h>
#include <ns3/tcp-congestion-ops.h>
#include <ns3/tcp-l4-protocol.h>
#include <ns3/test.h>
#include <ns3/three-gpp-http-client.h>
#include <ns3/three-gpp-http-header.h>
#include <ns3/three-gpp-http-helper.h>
#include <ns3/three-gpp-http-server.h>

#include <list>
#include <sstream>

NS_LOG_COMPONENT_DEFINE("ThreeGppHttpClientServerTest");

using namespace ns3;

class ThreeGppHttpObjectTestCase : public TestCase {
public:
  ThreeGppHttpObjectTestCase(const std::string &name, uint32_t rngRun,
                             const TypeId &tcpType, const Time &channelDelay,
                             double bitErrorRate, uint32_t mtuSize,
                             bool useIpv6);

private:
  Ptr<Node> CreateSimpleInternetNode(Ptr<SimpleChannel> channel,
                                     Address &assignedAddress);

  void DoRun() override;
  void DoTeardown() override;

  class ThreeGppHttpObjectTracker {
  public:
    ThreeGppHttpObjectTracker();
    void ObjectSent(uint32_t size);
    void PartReceived(uint32_t size);
    bool ObjectReceived(uint32_t &txSize, uint32_t &rxSize);
    bool IsEmpty() const;
    uint16_t GetNumOfObjectsReceived() const;

  private:
    std::list<uint32_t> m_objectsSize;
    uint32_t m_rxBuffer;
    uint16_t m_numOfObjectsReceived;
  };

  ThreeGppHttpObjectTracker m_requestObjectTracker;
  ThreeGppHttpObjectTracker m_mainObjectTracker;
  ThreeGppHttpObjectTracker m_embeddedObjectTracker;

  void ClientTxMainObjectRequestCallback(Ptr<const Packet> packet);
  void ClientTxEmbeddedObjectRequestCallback(Ptr<const Packet> packet);
  void ServerRxCallback(Ptr<const Packet> packet, const Address &from);
  void ServerMainObjectCallback(uint32_t size);
  void ClientRxMainObjectPacketCallback(Ptr<const Packet> packet);
  void ClientRxMainObjectCallback(Ptr<const ThreeGppHttpClient> httpClient,
                                  Ptr<const Packet> packet);
  void ServerEmbeddedObjectCallback(uint32_t size);
  void ClientRxEmbeddedObjectPacketCallback(Ptr<const Packet> packet);
  void ClientRxEmbeddedObjectCallback(Ptr<const ThreeGppHttpClient> httpClient,
                                      Ptr<const Packet> packet);
  void ClientStateTransitionCallback(const std::string &oldState,
                                     const std::string &newState);
  void ClientRxDelayCallback(const Time &delay, const Address &from);
  void ClientRxRttCallback(const Time &rtt, const Address &from);
  void DeviceDropCallback(Ptr<const Packet> packet);
  void ProgressCallback();

  uint32_t m_rngRun;
  TypeId m_tcpType;
  Time m_channelDelay;
  uint32_t m_mtuSize;
  bool m_useIpv6;

  Ptr<RateErrorModel> m_errorModel;
  uint16_t m_numOfPagesReceived;
  uint16_t m_numOfPacketDrops;
  InternetStackHelper m_internetStackHelper;
  Ipv4AddressHelper m_ipv4AddressHelper;
  Ipv6AddressHelper m_ipv6AddressHelper;
  Ptr<MinMaxAvgTotalCalculator<double>> m_delayCalculator;
  Ptr<MinMaxAvgTotalCalculator<double>> m_rttCalculator;
};

ThreeGppHttpObjectTestCase::ThreeGppHttpObjectTestCase(
    const std::string &name, uint32_t rngRun, const TypeId &tcpType,
    const Time &channelDelay, double bitErrorRate, uint32_t mtuSize,
    bool useIpv6)
    : TestCase(name), m_rngRun(rngRun), m_tcpType(tcpType),
      m_channelDelay(channelDelay), m_mtuSize(mtuSize), m_useIpv6(useIpv6),
      m_numOfPagesReceived(0), m_numOfPacketDrops(0) {
  NS_LOG_FUNCTION(this << GetName());

  NS_ASSERT(channelDelay.IsPositive());

  m_errorModel = CreateObject<RateErrorModel>();
  m_errorModel->SetRate(bitErrorRate);
  m_errorModel->SetUnit(RateErrorModel::ERROR_UNIT_BIT);

  m_ipv4AddressHelper.SetBase(Ipv4Address("10.0.0.0"), Ipv4Mask("255.0.0.0"),
                              Ipv4Address("0.0.0.1"));
  m_ipv6AddressHelper.SetBase(Ipv6Address("2001:1::"), Ipv6Prefix(64),
                              Ipv6Address("::1"));

  m_delayCalculator = CreateObject<MinMaxAvgTotalCalculator<double>>();
  m_rttCalculator = CreateObject<MinMaxAvgTotalCalculator<double>>();
}

Ptr<Node>
ThreeGppHttpObjectTestCase::CreateSimpleInternetNode(Ptr<SimpleChannel> channel,
                                                     Address &assignedAddress) {
  NS_LOG_FUNCTION(this << channel);

  Ptr<SimpleNetDevice> dev = CreateObject<SimpleNetDevice>();
  dev->SetAddress(Mac48Address::Allocate());
  dev->SetChannel(channel);
  dev->SetReceiveErrorModel(m_errorModel);

  Ptr<Node> node = CreateObject<Node>();
  node->AddDevice(dev);
  m_internetStackHelper.Install(node);

  if (m_useIpv6) {
    Ipv6InterfaceContainer ipv6Ifs =
        m_ipv6AddressHelper.Assign(NetDeviceContainer(dev));
    NS_ASSERT(ipv6Ifs.GetN() == 1);
    assignedAddress = ipv6Ifs.GetAddress(0, 0);
  } else {
    Ipv4InterfaceContainer ipv4Ifs =
        m_ipv4AddressHelper.Assign(NetDeviceContainer(dev));
    NS_ASSERT(ipv4Ifs.GetN() == 1);
    assignedAddress = ipv4Ifs.GetAddress(0, 0);
  }

  NS_LOG_DEBUG(this << " node is assigned to " << assignedAddress << ".");

  Ptr<TcpL4Protocol> tcp = node->GetObject<TcpL4Protocol>();
  tcp->SetAttribute("SocketType", TypeIdValue(m_tcpType));

  dev->TraceConnectWithoutContext(
      "PhyRxDrop",
      MakeCallback(&ThreeGppHttpObjectTestCase::DeviceDropCallback, this));

  return node;
}

void ThreeGppHttpObjectTestCase::DoRun() {
  NS_LOG_FUNCTION(this << GetName());
  Config::SetGlobal("RngRun", UintegerValue(m_rngRun));
  NS_LOG_INFO(this << " Running test case " << GetName());

  Ptr<SimpleChannel> channel = CreateObject<SimpleChannel>();
  channel->SetAttribute("Delay", TimeValue(m_channelDelay));

  Address serverAddress;
  Ptr<Node> serverNode = CreateSimpleInternetNode(channel, serverAddress);
  ThreeGppHttpServerHelper serverHelper(serverAddress);
  ApplicationContainer serverApplications = serverHelper.Install(serverNode);
  NS_TEST_ASSERT_MSG_EQ(serverApplications.GetN(), 1,
                        "Invalid number of HTTP servers has been installed");
  Ptr<ThreeGppHttpServer> httpServer =
      serverApplications.Get(0)->GetObject<ThreeGppHttpServer>();
  NS_TEST_ASSERT_MSG_NE(
      httpServer, nullptr,
      "HTTP server installation fails to produce a proper type");
  httpServer->SetMtuSize(m_mtuSize);

  Address clientAddress;
  Ptr<Node> clientNode = CreateSimpleInternetNode(channel, clientAddress);
  ThreeGppHttpClientHelper clientHelper(serverAddress);
  ApplicationContainer clientApplications = clientHelper.Install(clientNode);
  NS_TEST_ASSERT_MSG_EQ(clientApplications.GetN(), 1,
                        "Invalid number of HTTP clients has been installed");
  Ptr<ThreeGppHttpClient> httpClient =
      clientApplications.Get(0)->GetObject<ThreeGppHttpClient>();
  NS_TEST_ASSERT_MSG_NE(
      httpClient, nullptr,
      "HTTP client installation fails to produce a proper type");

  bool traceSourceConnected = httpClient->TraceConnectWithoutContext(
      "TxMainObjectRequest",
      MakeCallback(
          &ThreeGppHttpObjectTestCase::ClientTxMainObjectRequestCallback,
          this));
  NS_ASSERT(traceSourceConnected);
  traceSourceConnected = httpClient->TraceConnectWithoutContext(
      "TxEmbeddedObjectRequest",
      MakeCallback(
          &ThreeGppHttpObjectTestCase::ClientTxEmbeddedObjectRequestCallback,
          this));
  NS_ASSERT(traceSourceConnected);
  traceSourceConnected = httpServer->TraceConnectWithoutContext(
      "Rx", MakeCallback(&ThreeGppHttpObjectTestCase::ServerRxCallback, this));
  NS_ASSERT(traceSourceConnected);

  traceSourceConnected = httpServer->TraceConnectWithoutContext(
      "MainObject",
      MakeCallback(&ThreeGppHttpObjectTestCase::ServerMainObjectCallback,
                   this));
  NS_ASSERT(traceSourceConnected);
  traceSourceConnected = httpClient->TraceConnectWithoutContext(
      "RxMainObjectPacket",
      MakeCallback(
          &ThreeGppHttpObjectTestCase::ClientRxMainObjectPacketCallback, this));
  NS_ASSERT(traceSourceConnected);
  traceSourceConnected = httpClient->TraceConnectWithoutContext(
      "RxMainObject",
      MakeCallback(&ThreeGppHttpObjectTestCase::ClientRxMainObjectCallback,
                   this));
  NS_ASSERT(traceSourceConnected);

  traceSourceConnected = httpServer->TraceConnectWithoutContext(
      "EmbeddedObject",
      MakeCallback(&ThreeGppHttpObjectTestCase::ServerEmbeddedObjectCallback,
                   this));
  NS_ASSERT(traceSourceConnected);

  traceSourceConnected = httpClient->TraceConnectWithoutContext(
      "RxEmbeddedObjectPacket",
      MakeCallback(
          &ThreeGppHttpObjectTestCase::ClientRxEmbeddedObjectPacketCallback,
          this));
  NS_ASSERT(traceSourceConnected);

  traceSourceConnected = httpClient->TraceConnectWithoutContext(
      "RxEmbeddedObject",
      MakeCallback(&ThreeGppHttpObjectTestCase::ClientRxEmbeddedObjectCallback,
                   this));
  NS_ASSERT(traceSourceConnected);

  traceSourceConnected = httpClient->TraceConnectWithoutContext(
      "StateTransition",
      MakeCallback(&ThreeGppHttpObjectTestCase::ClientStateTransitionCallback,
                   this));
  NS_ASSERT(traceSourceConnected);
  traceSourceConnected = httpClient->TraceConnectWithoutContext(
      "RxDelay",
      MakeCallback(&ThreeGppHttpObjectTestCase::ClientRxDelayCallback, this));
  NS_ASSERT(traceSourceConnected);
  traceSourceConnected = httpClient->TraceConnectWithoutContext(
      "RxRtt",
      MakeCallback(&ThreeGppHttpObjectTestCase::ClientRxRttCallback, this));
  NS_ASSERT(traceSourceConnected);

  Simulator::Schedule(Seconds(1.0),
                      &ThreeGppHttpObjectTestCase::ProgressCallback, this);

  Simulator::Run();

  NS_LOG_INFO(this << " Total request objects received: "
                   << m_requestObjectTracker.GetNumOfObjectsReceived()
                   << " object(s).");
  NS_LOG_INFO(this << " Total main objects received: "
                   << m_mainObjectTracker.GetNumOfObjectsReceived()
                   << " object(s).");
  NS_LOG_INFO(this << " Total embedded objects received: "
                   << m_embeddedObjectTracker.GetNumOfObjectsReceived()
                   << " object(s).");
  NS_LOG_INFO(this << " One-trip delays:"
                   << " average=" << m_delayCalculator->getMean()
                   << " min=" << m_delayCalculator->getMin()
                   << " max=" << m_delayCalculator->getMax());
  NS_LOG_INFO(this << " Round-trip delays:"
                   << " average=" << m_rttCalculator->getMean()
                   << " min=" << m_rttCalculator->getMin()
                   << " max=" << m_rttCalculator->getMax());
  NS_LOG_INFO(this << " Number of packets dropped by the devices: "
                   << m_numOfPacketDrops << " packet(s).");

  NS_TEST_EXPECT_MSG_EQ(m_numOfPagesReceived, 3,
                        "Unexpected number of web pages processed.");
  NS_TEST_EXPECT_MSG_EQ(
      m_requestObjectTracker.IsEmpty(), true,
      "Tracker of request objects detected irrelevant packet(s).");
  NS_TEST_EXPECT_MSG_EQ(
      m_mainObjectTracker.IsEmpty(), true,
      "Tracker of main objects detected irrelevant packet(s).");
  NS_TEST_EXPECT_MSG_EQ(
      m_embeddedObjectTracker.IsEmpty(), true,
      "Tracker of embedded objects detected irrelevant packet(s).");

  Simulator::Destroy();
}

void ThreeGppHttpObjectTestCase::DoTeardown() {
  NS_LOG_FUNCTION(this << GetName());
}

ThreeGppHttpObjectTestCase::ThreeGppHttpObjectTracker::
    ThreeGppHttpObjectTracker()
    : m_rxBuffer(0), m_numOfObjectsReceived(0) {
  NS_LOG_FUNCTION(this);
}

void ThreeGppHttpObjectTestCase::ThreeGppHttpObjectTracker::ObjectSent(
    uint32_t size) {
  NS_LOG_FUNCTION(this << size);
  m_objectsSize.push_back(size);
}

void ThreeGppHttpObjectTestCase::ThreeGppHttpObjectTracker::PartReceived(
    uint32_t size) {
  NS_LOG_FUNCTION(this << size);
  m_rxBuffer += size;
}

bool ThreeGppHttpObjectTestCase::ThreeGppHttpObjectTracker::ObjectReceived(
    uint32_t &txSize, uint32_t &rxSize) {
  NS_LOG_FUNCTION(this);

  if (m_objectsSize.empty()) {
    return false;
  }

  txSize = m_objectsSize.front();
  rxSize = m_rxBuffer;

  m_objectsSize.pop_front();
  m_rxBuffer = 0;
  m_numOfObjectsReceived++;

  return true;
}

bool ThreeGppHttpObjectTestCase::ThreeGppHttpObjectTracker::IsEmpty() const {
  return (m_objectsSize.empty() && (m_rxBuffer == 0));
}

uint16_t
ThreeGppHttpObjectTestCase::ThreeGppHttpObjectTracker::GetNumOfObjectsReceived()
    const {
  return m_numOfObjectsReceived;
}

void ThreeGppHttpObjectTestCase::ClientTxMainObjectRequestCallback(
    Ptr<const Packet> packet) {
  NS_LOG_FUNCTION(this << packet << packet->GetSize());
  m_requestObjectTracker.ObjectSent(packet->GetSize());
}

void ThreeGppHttpObjectTestCase::ClientTxEmbeddedObjectRequestCallback(
    Ptr<const Packet> packet) {
  NS_LOG_FUNCTION(this << packet << packet->GetSize());
  m_requestObjectTracker.ObjectSent(packet->GetSize());
}

void ThreeGppHttpObjectTestCase::ServerRxCallback(Ptr<const Packet> packet,
                                                  const Address &from) {
  NS_LOG_FUNCTION(this << packet << packet->GetSize() << from);

  Ptr<Packet> copy = packet->Copy();
  ThreeGppHttpHeader httpHeader;
  NS_TEST_ASSERT_MSG_EQ(
      copy->RemoveHeader(httpHeader), 22,
      "Error finding ThreeGppHttpHeader in a packet received by the server");
  NS_TEST_ASSERT_MSG_GT(
      httpHeader.GetClientTs(), Seconds(0.0),
      "Request object's client TS is unexpectedly non-positive");

  m_requestObjectTracker.PartReceived(packet->GetSize());

  uint32_t txSize = 0;
  uint32_t rxSize = 0;
  bool isSent = m_requestObjectTracker.ObjectReceived(txSize, rxSize);
  NS_TEST_ASSERT_MSG_EQ(isSent, true,
                        "Server receives one too many request object");
  NS_TEST_ASSERT_MSG_EQ(
      txSize, rxSize,
      "Transmitted size and received size of request object differ");
}

void ThreeGppHttpObjectTestCase::ServerMainObjectCallback(uint32_t size) {
  NS_LOG_FUNCTION(this << size);
  m_mainObjectTracker.ObjectSent(size);
}

void ThreeGppHttpObjectTestCase::ClientRxMainObjectPacketCallback(
    Ptr<const Packet> packet) {
  NS_LOG_FUNCTION(this << packet << packet->GetSize());
  m_mainObjectTracker.PartReceived(packet->GetSize());
}

void ThreeGppHttpObjectTestCase::ClientRxMainObjectCallback(
    Ptr<const ThreeGppHttpClient> httpClient, Ptr<const Packet> packet) {
  NS_LOG_FUNCTION(this << httpClient << httpClient->GetNode()->GetId());

  Ptr<Packet> copy = packet->Copy();
  ThreeGppHttpHeader httpHeader;
  NS_TEST_ASSERT_MSG_EQ(
      copy->RemoveHeader(httpHeader), 22,
      "Error finding ThreeGppHttpHeader in a packet received by the server");
  NS_TEST_ASSERT_MSG_EQ(httpHeader.GetContentType(),
                        ThreeGppHttpHeader::MAIN_OBJECT,
                        "Invalid content type in the received packet");
  NS_TEST_ASSERT_MSG_GT(httpHeader.GetClientTs(), Seconds(0.0),
                        "Main object's client TS is unexpectedly non-positive");
  NS_TEST_ASSERT_MSG_GT(httpHeader.GetServerTs(), Seconds(0.0),
                        "Main object's server TS is unexpectedly non-positive");

  uint32_t txSize = 0;
  uint32_t rxSize = 0;
  bool isSent = m_mainObjectTracker.ObjectReceived(txSize, rxSize);
  NS_TEST_ASSERT_MSG_EQ(isSent, true,
                        "Client receives one too many main object");
  NS_TEST_ASSERT_MSG_EQ(
      txSize, rxSize,
      "Transmitted size and received size of main object differ");
  NS_TEST_ASSERT_MSG_EQ(
      httpHeader.GetContentLength(), rxSize,
      "Actual main object packet size and received size of main object differ");
}

void ThreeGppHttpObjectTestCase::ServerEmbeddedObjectCallback(uint32_t size) {
  NS_LOG_FUNCTION(this << size);
  m_embeddedObjectTracker.ObjectSent(size);
}

void ThreeGppHttpObjectTestCase::ClientRxEmbeddedObjectPacketCallback(
    Ptr<const Packet> packet) {
  NS_LOG_FUNCTION(this << packet << packet->GetSize());
  m_embeddedObjectTracker.PartReceived(packet->GetSize());
}

void ThreeGppHttpObjectTestCase::ClientRxEmbeddedObjectCallback(
    Ptr<const ThreeGppHttpClient> httpClient, Ptr<const Packet> packet) {
  NS_LOG_FUNCTION(this << httpClient << httpClient->GetNode()->GetId());

  Ptr<Packet> copy = packet->Copy();
  ThreeGppHttpHeader httpHeader;
  NS_TEST_ASSERT_MSG_EQ(
      copy->RemoveHeader(httpHeader), 22,
      "Error finding ThreeGppHttpHeader in a packet received by the server");
  NS_TEST_ASSERT_MSG_EQ(httpHeader.GetContentType(),
                        ThreeGppHttpHeader::EMBEDDED_OBJECT,
                        "Invalid content type in the received packet");
  NS_TEST_ASSERT_MSG_GT(
      httpHeader.GetClientTs(), Seconds(0.0),
      "Embedded object's client TS is unexpectedly non-positive");
  NS_TEST_ASSERT_MSG_GT(
      httpHeader.GetServerTs(), Seconds(0.0),
      "Embedded object's server TS is unexpectedly non-positive");

  uint32_t txSize = 0;
  uint32_t rxSize = 0;
  bool isSent = m_embeddedObjectTracker.ObjectReceived(txSize, rxSize);
  NS_TEST_ASSERT_MSG_EQ(isSent, true,
                        "Client receives one too many embedded object");
  NS_TEST_ASSERT_MSG_EQ(
      txSize, rxSize,
      "Transmitted size and received size of embedded object differ");
  NS_TEST_ASSERT_MSG_EQ(httpHeader.GetContentLength(), rxSize,
                        "Actual embedded object packet size and received size "
                        "of embedded object differ");
}

void ThreeGppHttpObjectTestCase::ClientStateTransitionCallback(
    const std::string &oldState, const std::string &newState) {
  NS_LOG_FUNCTION(this << oldState << newState);

  if (newState == "READING") {
    m_numOfPagesReceived++;

    if (m_numOfPagesReceived >= 3) {
      NS_LOG_LOGIC(this << " Test is stopping now.");
      Simulator::Stop();
    }
  }
}

void ThreeGppHttpObjectTestCase::ProgressCallback() {
  NS_LOG_INFO("Simulator time now: " << Simulator::Now().As(Time::S) << ".");
  Simulator::Schedule(Seconds(1.0),
                      &ThreeGppHttpObjectTestCase::ProgressCallback, this);
}

void ThreeGppHttpObjectTestCase::ClientRxDelayCallback(const Time &delay,
                                                       const Address &from) {
  NS_LOG_FUNCTION(this << delay.As(Time::S) << from);
  m_delayCalculator->Update(delay.GetSeconds());
}

void ThreeGppHttpObjectTestCase::ClientRxRttCallback(const Time &rtt,
                                                     const Address &from) {
  NS_LOG_FUNCTION(this << rtt.As(Time::S) << from);
  m_rttCalculator->Update(rtt.GetSeconds());
}

void ThreeGppHttpObjectTestCase::DeviceDropCallback(Ptr<const Packet> packet) {
  NS_LOG_FUNCTION(this << packet << packet->GetSize());
  m_numOfPacketDrops++;
}

class ThreeGppHttpClientServerTestSuite : public TestSuite {
public:
  ThreeGppHttpClientServerTestSuite()
      : TestSuite("three-gpp-http-client-server-test", SYSTEM) {

    Time channelDelay[] = {MilliSeconds(3), MilliSeconds(30),
                           MilliSeconds(300)};
    double bitErrorRate[] = {0.0, 5.0e-6};
    uint32_t mtuSize[] = {536, 1460};

    uint32_t run = 1;
    while (run <= 100) {
      for (uint32_t i1 = 0; i1 < 3; i1++) {
        for (uint32_t i2 = 0; i2 < 2; i2++) {
          for (uint32_t i3 = 0; i3 < 2; i3++) {
            AddHttpObjectTestCase(run++, channelDelay[i1], bitErrorRate[i2],
                                  mtuSize[i3], false);
            AddHttpObjectTestCase(run++, channelDelay[i1], bitErrorRate[i2],
                                  mtuSize[i3], true);
          }
        }
      }
    }
  }

private:
  void AddHttpObjectTestCase(uint32_t rngRun, const Time &channelDelay,
                             double bitErrorRate, uint32_t mtuSize,
                             bool useIpv6) {
    std::ostringstream name;
    name << "Run #" << rngRun;
    name << " delay=" << channelDelay.As(Time::MS);
    name << " ber=" << bitErrorRate;
    name << " mtu=" << mtuSize;

    if (useIpv6) {
      name << " IPv6";
    } else {
      name << " IPv4";
    }

    TestCase::TestDuration testDuration = TestCase::QUICK;
    if (rngRun > 20) {
      testDuration = TestCase::EXTENSIVE;
    }
    if (rngRun > 50) {
      testDuration = TestCase::TAKES_FOREVER;
    }

    AddTestCase(new ThreeGppHttpObjectTestCase(
                    name.str(), rngRun, TcpNewReno::GetTypeId(), channelDelay,
                    bitErrorRate, mtuSize, useIpv6),
                testDuration);
  }
};

static ThreeGppHttpClientServerTestSuite g_httpClientServerTestSuiteInstance;
