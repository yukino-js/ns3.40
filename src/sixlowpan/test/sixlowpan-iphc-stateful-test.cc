
#include "mock-net-device.h"

#include "ns3/boolean.h"
#include "ns3/icmpv6-l4-protocol.h"
#include "ns3/inet6-socket-address.h"
#include "ns3/internet-stack-helper.h"
#include "ns3/ipv6-address-helper.h"
#include "ns3/log.h"
#include "ns3/node.h"
#include "ns3/simulator.h"
#include "ns3/sixlowpan-header.h"
#include "ns3/sixlowpan-helper.h"
#include "ns3/sixlowpan-net-device.h"
#include "ns3/socket-factory.h"
#include "ns3/socket.h"
#include "ns3/test.h"

#include <limits>
#include <string>
#include <vector>

using namespace ns3;

class SixlowpanIphcStatefulImplTest : public TestCase {
  struct Data {
    Ptr<Packet> packet;
    Address src;
    Address dst;
  };

  std::vector<Data> m_txPackets;
  std::vector<Data> m_rxPackets;

  bool ReceiveFromMockDevice(Ptr<NetDevice> device, Ptr<const Packet> packet,
                             uint16_t protocol, const Address &source,
                             const Address &destination,
                             NetDevice::PacketType packetType);

  bool PromiscReceiveFromSixLowPanDevice(Ptr<NetDevice> device,
                                         Ptr<const Packet> packet,
                                         uint16_t protocol,
                                         const Address &source,
                                         const Address &destination,
                                         NetDevice::PacketType packetType);

  void SendOnePacket(Ptr<NetDevice> device, Ipv6Address from, Ipv6Address to);

  NetDeviceContainer m_mockDevices;
  NetDeviceContainer m_sixDevices;

public:
  void DoRun() override;
  SixlowpanIphcStatefulImplTest();
};

SixlowpanIphcStatefulImplTest::SixlowpanIphcStatefulImplTest()
    : TestCase("Sixlowpan IPHC stateful implementation") {}

bool SixlowpanIphcStatefulImplTest::ReceiveFromMockDevice(
    Ptr<NetDevice> device, Ptr<const Packet> packet, uint16_t protocol,
    const Address &source, const Address &destination,
    NetDevice::PacketType packetType) {
  Data incomingPkt;
  incomingPkt.packet = packet->Copy();
  incomingPkt.src = source;
  incomingPkt.dst = destination;
  m_txPackets.push_back(incomingPkt);

  Ptr<MockNetDevice> mockDev = DynamicCast<MockNetDevice>(m_mockDevices.Get(1));
  if (mockDev) {
    uint32_t id = mockDev->GetNode()->GetId();
    Simulator::ScheduleWithContext(id, Time(1), &MockNetDevice::Receive,
                                   mockDev, incomingPkt.packet, protocol,
                                   destination, source, packetType);
  }
  return true;
}

bool SixlowpanIphcStatefulImplTest::PromiscReceiveFromSixLowPanDevice(
    Ptr<NetDevice> device, Ptr<const Packet> packet, uint16_t protocol,
    const Address &source, const Address &destination,
    NetDevice::PacketType packetType) {
  Data incomingPkt;
  incomingPkt.packet = packet->Copy();
  incomingPkt.src = source;
  incomingPkt.dst = destination;
  m_rxPackets.push_back(incomingPkt);

  return true;
}

void SixlowpanIphcStatefulImplTest::SendOnePacket(Ptr<NetDevice> device,
                                                  Ipv6Address from,
                                                  Ipv6Address to) {
  Ptr<Packet> pkt = Create<Packet>(10);
  Ipv6Header ipHdr;
  ipHdr.SetSource(from);
  ipHdr.SetDestination(to);
  ipHdr.SetHopLimit(64);
  ipHdr.SetPayloadLength(10);
  ipHdr.SetNextHeader(0xff);
  pkt->AddHeader(ipHdr);

  device->Send(pkt, Mac48Address("00:00:00:00:00:02"), 0);
}

void SixlowpanIphcStatefulImplTest::DoRun() {
  NodeContainer nodes;
  nodes.Create(2);

  Ptr<MockNetDevice> mockNetDevice0 = CreateObject<MockNetDevice>();
  nodes.Get(0)->AddDevice(mockNetDevice0);
  mockNetDevice0->SetNode(nodes.Get(0));
  mockNetDevice0->SetAddress(Mac48Address("00:00:00:00:00:01"));
  mockNetDevice0->SetMtu(150);
  mockNetDevice0->SetSendCallback(MakeCallback(
      &SixlowpanIphcStatefulImplTest::ReceiveFromMockDevice, this));
  m_mockDevices.Add(mockNetDevice0);

  Ptr<MockNetDevice> mockNetDevice1 = CreateObject<MockNetDevice>();
  nodes.Get(1)->AddDevice(mockNetDevice1);
  mockNetDevice1->SetNode(nodes.Get(1));
  mockNetDevice1->SetAddress(Mac48Address("00:00:00:00:00:02"));
  mockNetDevice1->SetMtu(150);
  m_mockDevices.Add(mockNetDevice1);

  InternetStackHelper internetv6;
  internetv6.Install(nodes);

  SixLowPanHelper sixlowpan;
  m_sixDevices = sixlowpan.Install(m_mockDevices);
  m_sixDevices.Get(1)->SetPromiscReceiveCallback(MakeCallback(
      &SixlowpanIphcStatefulImplTest::PromiscReceiveFromSixLowPanDevice, this));

  Ipv6AddressHelper ipv6;
  ipv6.SetBase(Ipv6Address("2001:2::"), Ipv6Prefix(64));
  Ipv6InterfaceContainer deviceInterfaces;
  deviceInterfaces = ipv6.Assign(m_sixDevices);

  for (auto i = nodes.Begin(); i != nodes.End(); i++) {
    Ptr<Node> node = *i;
    Ptr<Ipv6L3Protocol> ipv6L3 = (*i)->GetObject<Ipv6L3Protocol>();
    if (ipv6L3) {
      ipv6L3->SetAttribute("IpForward", BooleanValue(true));
      ipv6L3->SetAttribute("SendIcmpv6Redirect", BooleanValue(false));
    }
    Ptr<Icmpv6L4Protocol> icmpv6 = (*i)->GetObject<Icmpv6L4Protocol>();
    if (icmpv6) {
      icmpv6->SetAttribute("DAD", BooleanValue(false));
    }
  }

  sixlowpan.AddContext(m_sixDevices, 0, Ipv6Prefix("2001:2::", 64),
                       Time(Minutes(30)));
  sixlowpan.AddContext(m_sixDevices, 1, Ipv6Prefix("2001:1::", 64),
                       Time(Minutes(30)));

  Ipv6Address srcElided = deviceInterfaces.GetAddress(0, 1);
  Ipv6Address dstElided = Ipv6Address::MakeAutoconfiguredAddress(
      Mac48Address("00:00:00:00:00:02"), Ipv6Prefix("2001:2::", 64));

  Simulator::Schedule(Seconds(1), &SixlowpanIphcStatefulImplTest::SendOnePacket,
                      this, m_sixDevices.Get(0), Ipv6Address::GetAny(),
                      dstElided);

  Simulator::Schedule(Seconds(2), &SixlowpanIphcStatefulImplTest::SendOnePacket,
                      this, m_sixDevices.Get(0),
                      Ipv6Address("2001:2::f00d:f00d:cafe:cafe"),
                      Ipv6Address("2001:1::0000:00ff:fe00:cafe"));

  Simulator::Schedule(Seconds(3), &SixlowpanIphcStatefulImplTest::SendOnePacket,
                      this, m_sixDevices.Get(0),
                      Ipv6Address("2001:1::0000:00ff:fe00:cafe"),
                      Ipv6Address("2001:1::f00d:f00d:cafe:cafe"));

  Simulator::Schedule(Seconds(4), &SixlowpanIphcStatefulImplTest::SendOnePacket,
                      this, m_sixDevices.Get(0), srcElided,
                      Ipv6Address("2001:1::f00d:f00d:cafe:cafe"));

  Simulator::Stop(Seconds(10));

  Simulator::Run();
  Simulator::Destroy();

  SixLowPanIphc iphcHdr;
  Ipv6Header ipv6Hdr;

  m_txPackets[0].packet->RemoveHeader(iphcHdr);
  NS_TEST_EXPECT_MSG_EQ(iphcHdr.GetCid(), false,
                        "CID should be false, is true");
  NS_TEST_EXPECT_MSG_EQ(iphcHdr.GetSac(), true, "SAC should be true, is false");
  NS_TEST_EXPECT_MSG_EQ(iphcHdr.GetSam(), SixLowPanIphc::HC_INLINE,
                        "SAM should be HC_INLINE, it is not");
  NS_TEST_EXPECT_MSG_EQ(iphcHdr.GetM(), false, "M should be false, is true");
  NS_TEST_EXPECT_MSG_EQ(iphcHdr.GetDac(), true, "DAC should be true, is false");
  NS_TEST_EXPECT_MSG_EQ(iphcHdr.GetSrcContextId(), 0,
                        "Src context should be 0, it is not");
  NS_TEST_EXPECT_MSG_EQ(iphcHdr.GetDstContextId(), 0,
                        "Dst context should be 0, it is not");
  NS_TEST_EXPECT_MSG_EQ(iphcHdr.GetDam(), SixLowPanIphc::HC_COMPR_0,
                        "DAM should be HC_COMPR_0, it is not");

  m_rxPackets[0].packet->RemoveHeader(ipv6Hdr);
  NS_TEST_EXPECT_MSG_EQ(ipv6Hdr.GetSource(), Ipv6Address::GetAny(),
                        "Src address wrongly rebuilt");
  NS_TEST_EXPECT_MSG_EQ(ipv6Hdr.GetDestination(), dstElided,
                        "Dst address wrongly rebuilt");

  m_txPackets[1].packet->RemoveHeader(iphcHdr);
  NS_TEST_EXPECT_MSG_EQ(iphcHdr.GetCid(), true, "CID should be true, is false");
  NS_TEST_EXPECT_MSG_EQ(iphcHdr.GetSac(), true, "SAC should be true, is false");
  NS_TEST_EXPECT_MSG_EQ(iphcHdr.GetSam(), SixLowPanIphc::HC_COMPR_64,
                        "SAM should be HC_COMPR_64, it is not");
  NS_TEST_EXPECT_MSG_EQ(iphcHdr.GetM(), false, "M should be false, is true");
  NS_TEST_EXPECT_MSG_EQ(iphcHdr.GetDac(), true, "DAC should be true, is false");
  NS_TEST_EXPECT_MSG_EQ(iphcHdr.GetSrcContextId(), 0,
                        "Src context should be 0, it is not");
  NS_TEST_EXPECT_MSG_EQ(iphcHdr.GetDstContextId(), 1,
                        "Dst context should be 1, it is not");
  NS_TEST_EXPECT_MSG_EQ(iphcHdr.GetDam(), SixLowPanIphc::HC_COMPR_16,
                        "DAM should be HC_COMPR_16, it is not");

  m_rxPackets[1].packet->RemoveHeader(ipv6Hdr);
  NS_TEST_EXPECT_MSG_EQ(ipv6Hdr.GetSource(),
                        Ipv6Address("2001:2::f00d:f00d:cafe:cafe"),
                        "Src address wrongly rebuilt");
  NS_TEST_EXPECT_MSG_EQ(ipv6Hdr.GetDestination(),
                        Ipv6Address("2001:1::0000:00ff:fe00:cafe"),
                        "Dst address wrongly rebuilt");

  m_txPackets[2].packet->RemoveHeader(iphcHdr);
  NS_TEST_EXPECT_MSG_EQ(iphcHdr.GetCid(), true, "CID should be true, is false");
  NS_TEST_EXPECT_MSG_EQ(iphcHdr.GetSac(), true, "SAC should be true, is false");
  NS_TEST_EXPECT_MSG_EQ(iphcHdr.GetSam(), SixLowPanIphc::HC_COMPR_16,
                        "SAM should be HC_COMPR_16, it is not");
  NS_TEST_EXPECT_MSG_EQ(iphcHdr.GetM(), false, "M should be false, is true");
  NS_TEST_EXPECT_MSG_EQ(iphcHdr.GetDac(), true, "DAC should be true, is false");
  NS_TEST_EXPECT_MSG_EQ(iphcHdr.GetSrcContextId(), 1,
                        "Src context should be 1, it is not");
  NS_TEST_EXPECT_MSG_EQ(iphcHdr.GetDstContextId(), 1,
                        "Dst context should be 1, it is not");
  NS_TEST_EXPECT_MSG_EQ(iphcHdr.GetDam(), SixLowPanIphc::HC_COMPR_64,
                        "DAM should be HC_COMPR_64, it is not");

  m_rxPackets[2].packet->RemoveHeader(ipv6Hdr);
  NS_TEST_EXPECT_MSG_EQ(ipv6Hdr.GetSource(),
                        Ipv6Address("2001:1::0000:00ff:fe00:cafe"),
                        "Src address wrongly rebuilt");
  NS_TEST_EXPECT_MSG_EQ(ipv6Hdr.GetDestination(),
                        Ipv6Address("2001:1::f00d:f00d:cafe:cafe"),
                        "Dst address wrongly rebuilt");

  m_txPackets[3].packet->RemoveHeader(iphcHdr);
  NS_TEST_EXPECT_MSG_EQ(iphcHdr.GetCid(), true, "CID should be true, is false");
  NS_TEST_EXPECT_MSG_EQ(iphcHdr.GetSac(), true, "SAC should be true, is false");
  NS_TEST_EXPECT_MSG_EQ(iphcHdr.GetSam(), SixLowPanIphc::HC_COMPR_0,
                        "SAM should be HC_COMPR_0, it is not");
  NS_TEST_EXPECT_MSG_EQ(iphcHdr.GetM(), false, "M should be false, is true");
  NS_TEST_EXPECT_MSG_EQ(iphcHdr.GetDac(), true, "DAC should be true, is false");
  NS_TEST_EXPECT_MSG_EQ(iphcHdr.GetSrcContextId(), 0,
                        "Src context should be 0, it is not");
  NS_TEST_EXPECT_MSG_EQ(iphcHdr.GetDstContextId(), 1,
                        "Dst context should be 1, it is not");
  NS_TEST_EXPECT_MSG_EQ(iphcHdr.GetDam(), SixLowPanIphc::HC_COMPR_64,
                        "DAM should be HC_COMPR_64, it is not");

  m_rxPackets[3].packet->RemoveHeader(ipv6Hdr);
  NS_TEST_EXPECT_MSG_EQ(ipv6Hdr.GetSource(), srcElided,
                        "Src address wrongly rebuilt");
  NS_TEST_EXPECT_MSG_EQ(ipv6Hdr.GetDestination(),
                        Ipv6Address("2001:1::f00d:f00d:cafe:cafe"),
                        "Dst address wrongly rebuilt");

  m_rxPackets.clear();
  m_txPackets.clear();
}

class SixlowpanIphcStatefulTestSuite : public TestSuite {
public:
  SixlowpanIphcStatefulTestSuite();

private:
};

SixlowpanIphcStatefulTestSuite::SixlowpanIphcStatefulTestSuite()
    : TestSuite("sixlowpan-iphc-stateful", UNIT) {
  AddTestCase(new SixlowpanIphcStatefulImplTest(), TestCase::QUICK);
}

static SixlowpanIphcStatefulTestSuite g_sixlowpanIphcStatefulTestSuite;
