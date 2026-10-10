
#include "lte-test-entities.h"

#include "ns3/arp-cache.h"
#include "ns3/boolean.h"
#include "ns3/config.h"
#include "ns3/csma-helper.h"
#include "ns3/epc-enb-application.h"
#include "ns3/eps-bearer-tag.h"
#include "ns3/inet-socket-address.h"
#include "ns3/internet-stack-helper.h"
#include "ns3/ipv4-address-helper.h"
#include "ns3/log.h"
#include "ns3/packet-sink-helper.h"
#include "ns3/packet-sink.h"
#include "ns3/point-to-point-epc-helper.h"
#include "ns3/point-to-point-helper.h"
#include "ns3/seq-ts-header.h"
#include "ns3/simulator.h"
#include "ns3/test.h"
#include "ns3/uinteger.h"
#include <ns3/ipv4-interface.h>
#include <ns3/ipv4-static-routing-helper.h>
#include <ns3/ipv4-static-routing.h>
#include <ns3/mac48-address.h>

using namespace ns3;

NS_LOG_COMPONENT_DEFINE("EpcTestS1uUplink");

class EpsBearerTagUdpClient : public Application {
public:
  static TypeId GetTypeId();

  EpsBearerTagUdpClient();
  EpsBearerTagUdpClient(uint16_t rnti, uint8_t bid);

  ~EpsBearerTagUdpClient() override;

  void SetRemote(Ipv4Address ip, uint16_t port);

protected:
  void DoDispose() override;

private:
  void StartApplication() override;
  void StopApplication() override;

  void ScheduleTransmit(Time dt);
  void Send();

  uint32_t m_count;
  Time m_interval;
  uint32_t m_size;

  uint32_t m_sent;
  Ptr<Socket> m_socket;
  Ipv4Address m_peerAddress;
  uint16_t m_peerPort;
  EventId m_sendEvent;

  uint16_t m_rnti;
  uint8_t m_bid;
};

TypeId EpsBearerTagUdpClient::GetTypeId() {
  static TypeId tid =
      TypeId("ns3::EpsBearerTagUdpClient")
          .SetParent<Application>()
          .AddConstructor<EpsBearerTagUdpClient>()
          .AddAttribute("MaxPackets",
                        "The maximum number of packets the application will "
                        "send (zero means infinite)",
                        UintegerValue(100),
                        MakeUintegerAccessor(&EpsBearerTagUdpClient::m_count),
                        MakeUintegerChecker<uint32_t>())
          .AddAttribute("Interval", "The time to wait between packets",
                        TimeValue(Seconds(1.0)),
                        MakeTimeAccessor(&EpsBearerTagUdpClient::m_interval),
                        MakeTimeChecker())
          .AddAttribute(
              "RemoteAddress",
              "The destination Ipv4Address of the outbound packets",
              Ipv4AddressValue(),
              MakeIpv4AddressAccessor(&EpsBearerTagUdpClient::m_peerAddress),
              MakeIpv4AddressChecker())
          .AddAttribute(
              "RemotePort", "The destination port of the outbound packets",
              UintegerValue(100),
              MakeUintegerAccessor(&EpsBearerTagUdpClient::m_peerPort),
              MakeUintegerChecker<uint16_t>())
          .AddAttribute("PacketSize",
                        "Size of packets generated. The minimum packet size is "
                        "12 bytes which is "
                        "the size of the header carrying the sequence number "
                        "and the time stamp.",
                        UintegerValue(1024),
                        MakeUintegerAccessor(&EpsBearerTagUdpClient::m_size),
                        MakeUintegerChecker<uint32_t>());
  return tid;
}

EpsBearerTagUdpClient::EpsBearerTagUdpClient() : m_rnti(0), m_bid(0) {
  NS_LOG_FUNCTION_NOARGS();
  m_sent = 0;
  m_socket = nullptr;
  m_sendEvent = EventId();
}

EpsBearerTagUdpClient::EpsBearerTagUdpClient(uint16_t rnti, uint8_t bid)
    : m_rnti(rnti), m_bid(bid) {
  NS_LOG_FUNCTION_NOARGS();
  m_sent = 0;
  m_socket = nullptr;
  m_sendEvent = EventId();
}

EpsBearerTagUdpClient::~EpsBearerTagUdpClient() { NS_LOG_FUNCTION_NOARGS(); }

void EpsBearerTagUdpClient::SetRemote(Ipv4Address ip, uint16_t port) {
  m_peerAddress = ip;
  m_peerPort = port;
}

void EpsBearerTagUdpClient::DoDispose() {
  NS_LOG_FUNCTION_NOARGS();
  Application::DoDispose();
}

void EpsBearerTagUdpClient::StartApplication() {
  NS_LOG_FUNCTION_NOARGS();

  if (!m_socket) {
    TypeId tid = TypeId::LookupByName("ns3::UdpSocketFactory");
    m_socket = Socket::CreateSocket(GetNode(), tid);
    m_socket->Bind();
    m_socket->Connect(InetSocketAddress(m_peerAddress, m_peerPort));
  }

  m_socket->SetRecvCallback(MakeNullCallback<void, Ptr<Socket>>());
  m_sendEvent =
      Simulator::Schedule(Seconds(0.0), &EpsBearerTagUdpClient::Send, this);
}

void EpsBearerTagUdpClient::StopApplication() {
  NS_LOG_FUNCTION_NOARGS();
  Simulator::Cancel(m_sendEvent);
}

void EpsBearerTagUdpClient::Send() {
  NS_LOG_FUNCTION_NOARGS();
  NS_ASSERT(m_sendEvent.IsExpired());
  SeqTsHeader seqTs;
  seqTs.SetSeq(m_sent);
  Ptr<Packet> p = Create<Packet>(m_size - (8 + 4));
  p->AddHeader(seqTs);

  EpsBearerTag tag(m_rnti, m_bid);
  p->AddPacketTag(tag);

  if ((m_socket->Send(p)) >= 0) {
    ++m_sent;
    NS_LOG_INFO("TraceDelay TX " << m_size << " bytes to " << m_peerAddress
                                 << " Uid: " << p->GetUid() << " Time: "
                                 << (Simulator::Now()).As(Time::S));
  } else {
    NS_LOG_INFO("Error while sending " << m_size << " bytes to "
                                       << m_peerAddress);
  }

  if (m_sent < m_count || m_count == 0) {
    m_sendEvent =
        Simulator::Schedule(m_interval, &EpsBearerTagUdpClient::Send, this);
  }
}

struct UeUlTestData {
  UeUlTestData(uint32_t n, uint32_t s, uint16_t r, uint8_t l);

  uint32_t numPkts;
  uint32_t pktSize;
  uint16_t rnti;
  uint8_t bid;

  Ptr<PacketSink> serverApp;
  Ptr<Application> clientApp;
};

UeUlTestData::UeUlTestData(uint32_t n, uint32_t s, uint16_t r, uint8_t l)
    : numPkts(n), pktSize(s), rnti(r), bid(l) {}

struct EnbUlTestData {
  std::vector<UeUlTestData> ues;
};

class EpcS1uUlTestCase : public TestCase {
public:
  EpcS1uUlTestCase(std::string name, std::vector<EnbUlTestData> v);
  ~EpcS1uUlTestCase() override;

private:
  void DoRun() override;
  std::vector<EnbUlTestData> m_enbUlTestData;
};

EpcS1uUlTestCase::EpcS1uUlTestCase(std::string name,
                                   std::vector<EnbUlTestData> v)
    : TestCase(name), m_enbUlTestData(v) {}

EpcS1uUlTestCase::~EpcS1uUlTestCase() {}

void EpcS1uUlTestCase::DoRun() {
  Ptr<PointToPointEpcHelper> epcHelper = CreateObject<PointToPointEpcHelper>();
  Ptr<Node> pgw = epcHelper->GetPgwNode();

  Config::SetDefault("ns3::CsmaNetDevice::Mtu", UintegerValue(30000));
  Config::SetDefault("ns3::PointToPointNetDevice::Mtu", UintegerValue(30000));
  epcHelper->SetAttribute("S1uLinkMtu", UintegerValue(30000));

  NodeContainer remoteHostContainer;
  remoteHostContainer.Create(1);
  Ptr<Node> remoteHost = remoteHostContainer.Get(0);
  InternetStackHelper internet;
  internet.Install(remoteHostContainer);

  PointToPointHelper p2ph;
  p2ph.SetDeviceAttribute("DataRate", DataRateValue(DataRate("100Gb/s")));
  NetDeviceContainer internetDevices = p2ph.Install(pgw, remoteHost);
  Ipv4AddressHelper ipv4h;
  ipv4h.SetBase("1.0.0.0", "255.0.0.0");
  Ipv4InterfaceContainer internetNodesIpIfaceContainer =
      ipv4h.Assign(internetDevices);

  Ipv4StaticRoutingHelper ipv4RoutingHelper;
  Ptr<Ipv4StaticRouting> remoteHostStaticRouting =
      ipv4RoutingHelper.GetStaticRouting(remoteHost->GetObject<Ipv4>());

  remoteHostStaticRouting->AddNetworkRouteTo(Ipv4Address("7.0.0.0"),
                                             Ipv4Mask("255.255.255.0"), 1);

  uint16_t udpSinkPort = 1234;

  NodeContainer enbs;
  uint16_t cellIdCounter = 0;
  uint64_t imsiCounter = 0;

  for (auto enbit = m_enbUlTestData.begin(); enbit < m_enbUlTestData.end();
       ++enbit) {
    Ptr<Node> enb = CreateObject<Node>();
    enbs.Add(enb);

    uint16_t cellId = ++cellIdCounter;

    NodeContainer ues;
    ues.Create(enbit->ues.size());

    NodeContainer cell;
    cell.Add(ues);
    cell.Add(enb);

    CsmaHelper csmaCell;
    NetDeviceContainer cellDevices = csmaCell.Install(cell);

    Ptr<NetDevice> enbDevice = cellDevices.Get(cellDevices.GetN() - 1);

    std::vector<uint16_t> cellIds;
    cellIds.push_back(cellId);
    epcHelper->AddEnb(enb, enbDevice, cellIds);

    Ptr<EpcEnbApplication> enbApp =
        enb->GetApplication(0)->GetObject<EpcEnbApplication>();
    NS_ASSERT_MSG(enbApp, "cannot retrieve EpcEnbApplication");
    Ptr<EpcTestRrc> rrc = CreateObject<EpcTestRrc>();
    enb->AggregateObject(rrc);
    rrc->SetS1SapProvider(enbApp->GetS1SapProvider());
    enbApp->SetS1SapUser(rrc->GetS1SapUser());

    InternetStackHelper internet;
    internet.Install(ues);

    for (uint32_t u = 0; u < ues.GetN(); ++u) {
      Ptr<NetDevice> ueLteDevice = cellDevices.Get(u);
      Ipv4InterfaceContainer ueIpIface =
          epcHelper->AssignUeIpv4Address(NetDeviceContainer(ueLteDevice));

      Ptr<Node> ue = ues.Get(u);

      Ptr<Ipv4> ueIpv4 = ue->GetObject<Ipv4>();
      ueIpv4->SetAttribute("IpForward", BooleanValue(false));

      Ptr<Ipv4StaticRouting> ueStaticRouting =
          ipv4RoutingHelper.GetStaticRouting(ueIpv4);
      Ipv4Address gwAddr = epcHelper->GetUeDefaultGatewayAddress();
      NS_LOG_INFO("GW address: " << gwAddr);
      ueStaticRouting->SetDefaultRoute(gwAddr, 1);

      int32_t ueLteIpv4IfIndex = ueIpv4->GetInterfaceForDevice(ueLteDevice);
      Ptr<Ipv4L3Protocol> ueIpv4L3Protocol = ue->GetObject<Ipv4L3Protocol>();
      Ptr<Ipv4Interface> ueLteIpv4Iface =
          ueIpv4L3Protocol->GetInterface(ueLteIpv4IfIndex);
      Ptr<ArpCache> ueArpCache = ueLteIpv4Iface->GetArpCache();
      ueArpCache->SetAliveTimeout(Seconds(1000));
      ArpCache::Entry *arpCacheEntry = ueArpCache->Add(gwAddr);
      arpCacheEntry->SetMacAddress(Mac48Address::GetBroadcast());
      arpCacheEntry->MarkPermanent();

      PacketSinkHelper packetSinkHelper(
          "ns3::UdpSocketFactory",
          InetSocketAddress(Ipv4Address::GetAny(), udpSinkPort));
      ApplicationContainer sinkApp = packetSinkHelper.Install(remoteHost);
      sinkApp.Start(Seconds(1.0));
      sinkApp.Stop(Seconds(10.0));
      enbit->ues[u].serverApp = sinkApp.Get(0)->GetObject<PacketSink>();

      Time interPacketInterval = Seconds(0.01);
      Ptr<EpsBearerTagUdpClient> client = CreateObject<EpsBearerTagUdpClient>(
          enbit->ues[u].rnti, enbit->ues[u].bid);
      client->SetAttribute(
          "RemoteAddress",
          Ipv4AddressValue(internetNodesIpIfaceContainer.GetAddress(1)));
      client->SetAttribute("RemotePort", UintegerValue(udpSinkPort));
      client->SetAttribute("MaxPackets", UintegerValue(enbit->ues[u].numPkts));
      client->SetAttribute("Interval", TimeValue(interPacketInterval));
      client->SetAttribute("PacketSize", UintegerValue(enbit->ues[u].pktSize));
      ue->AddApplication(client);
      ApplicationContainer clientApp;
      clientApp.Add(client);
      clientApp.Start(Seconds(2.0));
      clientApp.Stop(Seconds(10.0));
      enbit->ues[u].clientApp = client;

      uint64_t imsi = ++imsiCounter;
      epcHelper->AddUe(ueLteDevice, imsi);
      epcHelper->ActivateEpsBearer(
          ueLteDevice, imsi, EpcTft::Default(),
          EpsBearer(EpsBearer::NGBR_VIDEO_TCP_DEFAULT));
      Simulator::Schedule(MilliSeconds(10),
                          &EpcEnbS1SapProvider::InitialUeMessage,
                          enbApp->GetS1SapProvider(), imsi, enbit->ues[u].rnti);
      ++udpSinkPort;
    }
  }

  Simulator::Run();

  for (auto enbit = m_enbUlTestData.begin(); enbit < m_enbUlTestData.end();
       ++enbit) {
    for (auto ueit = enbit->ues.begin(); ueit < enbit->ues.end(); ++ueit) {
      NS_TEST_ASSERT_MSG_EQ(ueit->serverApp->GetTotalRx(),
                            (ueit->numPkts) * (ueit->pktSize),
                            "wrong total received bytes");
    }
  }

  Simulator::Destroy();
}

class EpcS1uUlTestSuite : public TestSuite {
public:
  EpcS1uUlTestSuite();

} g_epcS1uUlTestSuiteInstance;

EpcS1uUlTestSuite::EpcS1uUlTestSuite() : TestSuite("epc-s1u-uplink", SYSTEM) {
  std::vector<EnbUlTestData> v1;
  EnbUlTestData e1;
  UeUlTestData f1(1, 100, 1, 1);
  e1.ues.push_back(f1);
  v1.push_back(e1);
  AddTestCase(new EpcS1uUlTestCase("1 eNB, 1UE", v1), TestCase::QUICK);

  std::vector<EnbUlTestData> v2;
  EnbUlTestData e2;
  UeUlTestData f2_1(1, 100, 1, 1);
  e2.ues.push_back(f2_1);
  UeUlTestData f2_2(2, 200, 2, 1);
  e2.ues.push_back(f2_2);
  v2.push_back(e2);
  AddTestCase(new EpcS1uUlTestCase("1 eNB, 2UEs", v2), TestCase::QUICK);

  std::vector<EnbUlTestData> v3;
  v3.push_back(e1);
  v3.push_back(e2);
  AddTestCase(new EpcS1uUlTestCase("2 eNBs", v3), TestCase::QUICK);

  EnbUlTestData e3;
  UeUlTestData f3_1(3, 50, 1, 1);
  e3.ues.push_back(f3_1);
  UeUlTestData f3_2(5, 1472, 2, 1);
  e3.ues.push_back(f3_2);
  UeUlTestData f3_3(1, 1, 3, 1);
  e3.ues.push_back(f3_2);
  std::vector<EnbUlTestData> v4;
  v4.push_back(e3);
  v4.push_back(e1);
  v4.push_back(e2);
  AddTestCase(new EpcS1uUlTestCase("3 eNBs", v4), TestCase::QUICK);

  std::vector<EnbUlTestData> v5;
  EnbUlTestData e5;
  UeUlTestData f5(10, 3000, 1, 1);
  e5.ues.push_back(f5);
  v5.push_back(e5);
  AddTestCase(new EpcS1uUlTestCase("1 eNB, 10 pkts 3000 bytes each", v5),
              TestCase::QUICK);

  std::vector<EnbUlTestData> v6;
  EnbUlTestData e6;
  UeUlTestData f6(50, 3000, 1, 1);
  e6.ues.push_back(f6);
  v6.push_back(e6);
  AddTestCase(new EpcS1uUlTestCase("1 eNB, 50 pkts 3000 bytes each", v6),
              TestCase::QUICK);

  std::vector<EnbUlTestData> v7;
  EnbUlTestData e7;
  UeUlTestData f7(10, 15000, 1, 1);
  e7.ues.push_back(f7);
  v7.push_back(e7);
  AddTestCase(new EpcS1uUlTestCase("1 eNB, 10 pkts 15000 bytes each", v7),
              TestCase::QUICK);

  std::vector<EnbUlTestData> v8;
  EnbUlTestData e8;
  UeUlTestData f8(100, 15000, 1, 1);
  e8.ues.push_back(f8);
  v8.push_back(e8);
  AddTestCase(new EpcS1uUlTestCase("1 eNB, 100 pkts 15000 bytes each", v8),
              TestCase::QUICK);
}
