
#include "ns3/config-store.h"
#include "ns3/core-module.h"
#include "ns3/epc-helper.h"
#include "ns3/internet-module.h"
#include "ns3/ipv4-global-routing-helper.h"
#include "ns3/ipv6-static-routing.h"
#include "ns3/lte-helper.h"
#include "ns3/lte-module.h"
#include "ns3/mobility-module.h"
#include "ns3/network-module.h"
#include "ns3/point-to-point-helper.h"
#include "ns3/udp-echo-helper.h"

#include <algorithm>

using namespace ns3;

class LteIpv6RoutingTestCase : public TestCase {
public:
  LteIpv6RoutingTestCase();
  ~LteIpv6RoutingTestCase() override;

  void Checker();

  void SentAtClient(Ptr<const Packet> p, Ptr<Ipv6> ipv6, uint32_t interface);

  void ReceivedAtClient(Ptr<const Packet> p, Ptr<Ipv6> ipv6,
                        uint32_t interface);

  void EnbToPgw(Ptr<Packet> p);

  void TunToPgw(Ptr<Packet> p);

private:
  void DoRun() override;
  Ipv6InterfaceContainer m_ueIpIface;
  Ipv6Address m_remoteHostAddr;
  std::list<uint64_t> m_pgwUidRxFrmEnb;
  std::list<uint64_t> m_pgwUidRxFrmTun;

  std::list<Ptr<Packet>> m_clientTxPkts;
  std::list<Ptr<Packet>> m_clientRxPkts;
};

LteIpv6RoutingTestCase::LteIpv6RoutingTestCase()
    : TestCase("Test IPv6 Routing at LTE") {}

LteIpv6RoutingTestCase::~LteIpv6RoutingTestCase() {}

void LteIpv6RoutingTestCase::SentAtClient(Ptr<const Packet> p, Ptr<Ipv6> ipv6,
                                          uint32_t interface) {
  Ipv6Header ipv6Header;
  p->PeekHeader(ipv6Header);
  if (ipv6Header.GetNextHeader() == UdpL4Protocol::PROT_NUMBER) {
    m_clientTxPkts.push_back(p->Copy());
  }
}

void LteIpv6RoutingTestCase::ReceivedAtClient(Ptr<const Packet> p,
                                              Ptr<Ipv6> ipv6,
                                              uint32_t interface) {
  Ipv6Header ipv6Header;
  p->PeekHeader(ipv6Header);
  if (ipv6Header.GetNextHeader() == UdpL4Protocol::PROT_NUMBER) {
    m_clientRxPkts.push_back(p->Copy());
  }
}

void LteIpv6RoutingTestCase::EnbToPgw(Ptr<Packet> p) {
  Ipv6Header ipv6Header;
  p->PeekHeader(ipv6Header);
  if (ipv6Header.GetNextHeader() == UdpL4Protocol::PROT_NUMBER) {
    m_pgwUidRxFrmEnb.push_back(p->GetUid());
  }
}

void LteIpv6RoutingTestCase::TunToPgw(Ptr<Packet> p) {
  Ipv6Header ipv6Header;
  p->PeekHeader(ipv6Header);
  if (ipv6Header.GetNextHeader() == UdpL4Protocol::PROT_NUMBER) {
    m_pgwUidRxFrmTun.push_back(p->GetUid());
  }
}

void LteIpv6RoutingTestCase::Checker() {
  bool b = false;
  bool check = true;
  for (auto it1 = m_clientRxPkts.begin(); it1 != m_clientRxPkts.end(); it1++) {
    Ipv6Header ipv6header1;
    UdpHeader udpHeader1;
    Ptr<Packet> p1 = (*it1)->Copy();
    p1->RemoveHeader(ipv6header1);
    uint64_t uid = p1->GetUid();
    p1->RemoveHeader(udpHeader1);
    for (auto it2 = m_clientTxPkts.begin(); it2 != m_clientTxPkts.end();
         it2++) {
      Ptr<Packet> p2 = (*it2)->Copy();
      Ipv6Header ipv6header2;
      p2->RemoveHeader(ipv6header2);
      Ipv6Address sourceAddress = ipv6header2.GetSource();
      Ipv6Address destinationAddress = ipv6header2.GetDestination();
      UdpHeader udpHeader2;
      p2->RemoveHeader(udpHeader2);
      uint16_t sourcePort;
      uint16_t destinationPort;
      sourcePort = udpHeader2.GetSourcePort();
      destinationPort = udpHeader2.GetDestinationPort();
      if ((p2->GetUid() == p1->GetUid()) &&
          sourceAddress == ipv6header1.GetDestination() &&
          destinationAddress == ipv6header1.GetSource() &&
          sourcePort == udpHeader1.GetDestinationPort() &&
          destinationPort == udpHeader1.GetSourcePort()) {
        b = true;
        break;
      }
    }
    check &= b;
    if (std::find(m_pgwUidRxFrmEnb.begin(), m_pgwUidRxFrmEnb.end(), uid) !=
        m_pgwUidRxFrmEnb.end()) {
      check &= true;
      m_pgwUidRxFrmEnb.remove(uid);
    }
    if (std::find(m_pgwUidRxFrmTun.begin(), m_pgwUidRxFrmTun.end(), uid) !=
        m_pgwUidRxFrmTun.end()) {
      check &= true;
      m_pgwUidRxFrmTun.remove(uid);
    }
    b = false;
  }

  NS_TEST_ASSERT_MSG_EQ(check, true, "Failure Happens IPv6 routing of LENA");
  NS_TEST_ASSERT_MSG_EQ(m_clientTxPkts.size(), m_clientRxPkts.size(),
                        "No. of Request and Reply messages mismatch");
  NS_TEST_ASSERT_MSG_EQ(m_pgwUidRxFrmEnb.size(), 0,
                        "Route is not Redundant in Lte IPv6 test");
  NS_TEST_ASSERT_MSG_EQ(m_pgwUidRxFrmTun.size(), 0,
                        "Route is not Redundant in Lte IPv6 test");
}

void LteIpv6RoutingTestCase::DoRun() {
  double distance = 60.0;

  Ptr<LteHelper> lteHelper = CreateObject<LteHelper>();
  Ptr<PointToPointEpcHelper> epcHelper = CreateObject<PointToPointEpcHelper>();
  lteHelper->SetEpcHelper(epcHelper);

  ConfigStore inputConfig;
  inputConfig.ConfigureDefaults();

  Ptr<Node> pgw = epcHelper->GetPgwNode();

  NodeContainer remoteHostContainer;
  remoteHostContainer.Create(1);
  Ptr<Node> remoteHost = remoteHostContainer.Get(0);
  InternetStackHelper internet;
  internet.Install(remoteHostContainer);

  PointToPointHelper p2ph;
  p2ph.SetDeviceAttribute("DataRate", DataRateValue(DataRate("100Gb/s")));
  p2ph.SetDeviceAttribute("Mtu", UintegerValue(1500));
  p2ph.SetChannelAttribute("Delay", TimeValue(Seconds(0.010)));
  NetDeviceContainer internetDevices = p2ph.Install(pgw, remoteHost);

  NodeContainer ueNodes;
  NodeContainer enbNodes;
  enbNodes.Create(2);
  ueNodes.Create(3);

  Ptr<ListPositionAllocator> positionAlloc1 =
      CreateObject<ListPositionAllocator>();
  Ptr<ListPositionAllocator> positionAlloc2 =
      CreateObject<ListPositionAllocator>();

  positionAlloc1->Add(Vector(distance * 0, 0, 0));
  positionAlloc1->Add(Vector(distance * 0 + 5, 0, 0));
  positionAlloc1->Add(Vector(distance * 1, 0, 0));

  positionAlloc2->Add(Vector(distance * 0, 0, 0));
  positionAlloc2->Add(Vector(distance * 1, 0, 0));

  MobilityHelper mobility;
  mobility.SetMobilityModel("ns3::ConstantPositionMobilityModel");
  mobility.SetPositionAllocator(positionAlloc1);
  mobility.Install(ueNodes);

  mobility.SetPositionAllocator(positionAlloc2);
  mobility.Install(enbNodes);

  internet.Install(ueNodes);

  NetDeviceContainer enbLteDevs = lteHelper->InstallEnbDevice(enbNodes);
  NetDeviceContainer ueLteDevs = lteHelper->InstallUeDevice(ueNodes);

  m_ueIpIface = epcHelper->AssignUeIpv6Address(NetDeviceContainer(ueLteDevs));

  Ipv6StaticRoutingHelper ipv6RoutingHelper;

  for (uint32_t u = 0; u < ueNodes.GetN(); ++u) {
    Ptr<Node> ueNode = ueNodes.Get(u);
    Ptr<Ipv6StaticRouting> ueStaticRouting =
        ipv6RoutingHelper.GetStaticRouting(ueNode->GetObject<Ipv6>());
    ueStaticRouting->SetDefaultRoute(epcHelper->GetUeDefaultGatewayAddress6(),
                                     1);
  }

  lteHelper->Attach(ueLteDevs.Get(0), enbLteDevs.Get(0));
  lteHelper->Attach(ueLteDevs.Get(1), enbLteDevs.Get(0));
  lteHelper->Attach(ueLteDevs.Get(2), enbLteDevs.Get(1));

  Ipv6AddressHelper ipv6h;
  ipv6h.SetBase(Ipv6Address("6001:db80::"), Ipv6Prefix(64));
  Ipv6InterfaceContainer internetIpIfaces = ipv6h.Assign(internetDevices);

  internetIpIfaces.SetForwarding(0, true);
  internetIpIfaces.SetDefaultRouteInAllNodes(0);

  Ptr<Ipv6StaticRouting> remoteHostStaticRouting =
      ipv6RoutingHelper.GetStaticRouting(remoteHost->GetObject<Ipv6>());
  remoteHostStaticRouting->AddNetworkRouteTo(
      "7777:f00d::", Ipv6Prefix(64), internetIpIfaces.GetAddress(0, 1), 1, 0);

  m_remoteHostAddr = internetIpIfaces.GetAddress(1, 1);

  UdpEchoServerHelper echoServer1(10);
  UdpEchoServerHelper echoServer2(11);
  UdpEchoServerHelper echoServer3(12);

  ApplicationContainer serverApps = echoServer1.Install(remoteHost);
  serverApps.Add(echoServer2.Install(ueNodes.Get(1)));
  serverApps.Add(echoServer3.Install(ueNodes.Get(2)));

  serverApps.Start(Seconds(4.0));
  serverApps.Stop(Seconds(12.0));

  UdpEchoClientHelper echoClient1(m_remoteHostAddr, 10);
  UdpEchoClientHelper echoClient2(m_ueIpIface.GetAddress(1, 1), 11);
  UdpEchoClientHelper echoClient3(m_ueIpIface.GetAddress(2, 1), 12);

  echoClient1.SetAttribute("MaxPackets", UintegerValue(1000));
  echoClient1.SetAttribute("Interval", TimeValue(Seconds(0.2)));
  echoClient1.SetAttribute("PacketSize", UintegerValue(1024));

  echoClient2.SetAttribute("MaxPackets", UintegerValue(1000));
  echoClient2.SetAttribute("Interval", TimeValue(Seconds(0.2)));
  echoClient2.SetAttribute("PacketSize", UintegerValue(1024));

  echoClient3.SetAttribute("MaxPackets", UintegerValue(1000));
  echoClient3.SetAttribute("Interval", TimeValue(Seconds(0.2)));
  echoClient3.SetAttribute("PacketSize", UintegerValue(1024));

  ApplicationContainer clientApps1 = echoClient1.Install(ueNodes.Get(0));
  ApplicationContainer clientApps2 = echoClient2.Install(ueNodes.Get(0));
  ApplicationContainer clientApps3 = echoClient3.Install(ueNodes.Get(0));

  clientApps1.Start(Seconds(4.0));
  clientApps1.Stop(Seconds(6.0));

  clientApps2.Start(Seconds(6.1));
  clientApps2.Stop(Seconds(8.0));

  clientApps3.Start(Seconds(8.1));
  clientApps3.Stop(Seconds(10.0));

  Ptr<Ipv6L3Protocol> ipL3 = (ueNodes.Get(0))->GetObject<Ipv6L3Protocol>();
  ipL3->TraceConnectWithoutContext(
      "Tx", MakeCallback(&LteIpv6RoutingTestCase::SentAtClient, this));
  ipL3->TraceConnectWithoutContext(
      "Rx", MakeCallback(&LteIpv6RoutingTestCase::ReceivedAtClient, this));

  Ptr<Application> appPgw = pgw->GetApplication(0);
  appPgw->TraceConnectWithoutContext(
      "RxFromS1u", MakeCallback(&LteIpv6RoutingTestCase::EnbToPgw, this));
  appPgw->TraceConnectWithoutContext(
      "RxFromTun", MakeCallback(&LteIpv6RoutingTestCase::TunToPgw, this));

  Simulator::Schedule(Time(Seconds(12.0)), &LteIpv6RoutingTestCase::Checker,
                      this);

  Simulator::Stop(Seconds(14));
  Simulator::Run();

  Simulator::Destroy();
}

class LteIpv6RoutingTestSuite : public TestSuite {
public:
  LteIpv6RoutingTestSuite();
};

LteIpv6RoutingTestSuite::LteIpv6RoutingTestSuite()
    : TestSuite("lte-ipv6-routing-test", UNIT) {
  AddTestCase(new LteIpv6RoutingTestCase, TestCase::QUICK);
}

static LteIpv6RoutingTestSuite g_lteipv6testsuite;
