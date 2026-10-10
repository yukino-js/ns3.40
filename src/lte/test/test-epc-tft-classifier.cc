
#include "ns3/epc-tft-classifier.h"
#include "ns3/ipv4-header.h"
#include "ns3/ipv4-l3-protocol.h"
#include "ns3/ipv6-header.h"
#include "ns3/ipv6-l3-protocol.h"
#include "ns3/log.h"
#include "ns3/packet.h"
#include "ns3/tcp-header.h"
#include "ns3/tcp-l4-protocol.h"
#include "ns3/test.h"
#include "ns3/udp-header.h"
#include "ns3/udp-l4-protocol.h"

#include <iomanip>

using namespace ns3;

NS_LOG_COMPONENT_DEFINE("TestEpcTftClassifier");

class EpcTftClassifierTestCase : public TestCase {
public:
  EpcTftClassifierTestCase(Ptr<EpcTftClassifier> c, EpcTft::Direction d,
                           std::string sa, std::string da, uint16_t sp,
                           uint16_t dp, uint8_t tos, uint32_t tftId,
                           bool useIpv6);

  ~EpcTftClassifierTestCase() override;

private:
  Ptr<EpcTftClassifier> m_c;
  EpcTft::Direction m_d;
  uint8_t m_tftId;
  bool m_useIpv6;
  Ipv4Header m_ipHeader;
  Ipv6Header m_ipv6Header;
  UdpHeader m_udpHeader;
  TcpHeader m_tcpHeader;

  static std::string BuildNameString(Ptr<EpcTftClassifier> c,
                                     EpcTft::Direction d, std::string sa,
                                     std::string da, uint16_t sp, uint16_t dp,
                                     uint8_t tos, uint32_t tftId, bool useIpv6);

  void DoRun() override;
};

EpcTftClassifierTestCase::EpcTftClassifierTestCase(Ptr<EpcTftClassifier> c,
                                                   EpcTft::Direction d,
                                                   std::string sa,
                                                   std::string da, uint16_t sp,
                                                   uint16_t dp, uint8_t tos,
                                                   uint32_t tftId, bool useIpv6)
    : TestCase(BuildNameString(c, d, sa, da, sp, dp, tos, tftId, useIpv6)),
      m_c(c), m_d(d), m_tftId(tftId), m_useIpv6(useIpv6) {
  NS_LOG_FUNCTION(this << c << d << sa << da << sp << dp << tos << tftId
                       << useIpv6);

  if (m_useIpv6) {
    m_ipv6Header.SetSource(
        Ipv6Address::MakeIpv4MappedAddress(Ipv4Address(sa.c_str())));
    m_ipv6Header.SetDestination(
        Ipv6Address::MakeIpv4MappedAddress(Ipv4Address(da.c_str())));
    m_ipv6Header.SetTrafficClass(tos);
    m_ipv6Header.SetPayloadLength(8);
    m_ipv6Header.SetNextHeader(UdpL4Protocol::PROT_NUMBER);
  } else {
    m_ipHeader.SetSource(Ipv4Address(sa.c_str()));
    m_ipHeader.SetDestination(Ipv4Address(da.c_str()));
    m_ipHeader.SetTos(tos);
    m_ipHeader.SetPayloadSize(8);
    m_ipHeader.SetProtocol(UdpL4Protocol::PROT_NUMBER);
  }

  m_udpHeader.SetSourcePort(sp);
  m_udpHeader.SetDestinationPort(dp);
}

EpcTftClassifierTestCase::~EpcTftClassifierTestCase() {}

std::string EpcTftClassifierTestCase::BuildNameString(
    Ptr<EpcTftClassifier> c, EpcTft::Direction d, std::string sa,
    std::string da, uint16_t sp, uint16_t dp, uint8_t tos, uint32_t tftId,
    bool useIpv6) {
  std::ostringstream oss;
  oss << c << "  d = " << d;
  if (useIpv6) {
    oss << ", sa = "
        << Ipv6Address::MakeIpv4MappedAddress(Ipv4Address(sa.c_str()))
        << ", da = "
        << Ipv6Address::MakeIpv4MappedAddress(Ipv4Address(da.c_str()));
  } else {
    oss << ", sa = " << sa << ", da = " << da;
  }
  oss << ", sp = " << sp << ", dp = " << dp << ", tos = 0x" << std::hex
      << (int)tos << " --> tftId = " << tftId;
  return oss.str();
}

void EpcTftClassifierTestCase::DoRun() {
  ns3::PacketMetadata::Enable();

  Ptr<Packet> udpPacket = Create<Packet>();
  udpPacket->AddHeader(m_udpHeader);
  if (m_useIpv6) {
    udpPacket->AddHeader(m_ipv6Header);
  } else {
    udpPacket->AddHeader(m_ipHeader);
  }
  NS_LOG_LOGIC(this << *udpPacket);
  uint32_t obtainedTftId = m_c->Classify(
      udpPacket, m_d,
      m_useIpv6 ? Ipv6L3Protocol::PROT_NUMBER : Ipv4L3Protocol::PROT_NUMBER);
  NS_TEST_ASSERT_MSG_EQ(obtainedTftId, (uint16_t)m_tftId,
                        "bad classification of UDP packet");
}

class EpcTftClassifierTestSuite : public TestSuite {
public:
  EpcTftClassifierTestSuite();
};

static EpcTftClassifierTestSuite g_lteTftClassifierTestSuite;

EpcTftClassifierTestSuite::EpcTftClassifierTestSuite()
    : TestSuite("eps-tft-classifier", UNIT) {
  NS_LOG_FUNCTION(this);

  for (bool useIpv6 : {false, true}) {

    Ptr<EpcTftClassifier> c1 = Create<EpcTftClassifier>();

    Ptr<EpcTft> tft1_1 = Create<EpcTft>();

    EpcTft::PacketFilter pf1_1_1;
    if (useIpv6) {
      pf1_1_1.remoteIpv6Address.Set("0::ffff:0100:0000");
      pf1_1_1.remoteIpv6Prefix = Ipv6Prefix(96 + 8);
      pf1_1_1.localIpv6Address.Set("0::ffff:0200:0000");
      pf1_1_1.localIpv6Prefix = Ipv6Prefix(96 + 8);
    } else {
      pf1_1_1.remoteAddress.Set("1.0.0.0");
      pf1_1_1.remoteMask.Set(0xff000000);
      pf1_1_1.localAddress.Set("2.0.0.0");
      pf1_1_1.localMask.Set(0xff000000);
    }
    tft1_1->Add(pf1_1_1);

    EpcTft::PacketFilter pf1_1_2;
    if (useIpv6) {
      pf1_1_2.remoteIpv6Address.Set("0::ffff:0303:0300");
      pf1_1_2.remoteIpv6Prefix = Ipv6Prefix(96 + 24);
      pf1_1_2.localIpv6Address.Set("0::ffff:0404:0400");
      pf1_1_2.localIpv6Prefix = Ipv6Prefix(96 + 24);
    } else {
      pf1_1_2.remoteAddress.Set("3.3.3.0");
      pf1_1_2.remoteMask.Set(0xffffff00);
      pf1_1_2.localAddress.Set("4.4.4.0");
      pf1_1_2.localMask.Set(0xffffff00);
    }
    tft1_1->Add(pf1_1_2);

    c1->Add(tft1_1, 1);

    Ptr<EpcTft> tft1_2 = Create<EpcTft>();

    EpcTft::PacketFilter pf1_2_1;
    pf1_2_1.remotePortStart = 1024;
    pf1_2_1.remotePortEnd = 1035;
    tft1_2->Add(pf1_2_1);

    EpcTft::PacketFilter pf1_2_2;
    pf1_2_2.localPortStart = 3456;
    pf1_2_2.localPortEnd = 3489;
    tft1_2->Add(pf1_2_2);

    EpcTft::PacketFilter pf1_2_3;
    pf1_2_3.localPortStart = 7895;
    pf1_2_3.localPortEnd = 7895;
    tft1_2->Add(pf1_2_3);

    EpcTft::PacketFilter pf1_2_4;
    pf1_2_4.remotePortStart = 5897;
    pf1_2_4.remotePortEnd = 5897;
    tft1_2->Add(pf1_2_4);

    c1->Add(tft1_2, 2);

    AddTestCase(new EpcTftClassifierTestCase(c1, EpcTft::UPLINK, "2.2.3.4",
                                             "1.1.1.1", 4, 1234, 0, 1, useIpv6),
                TestCase::QUICK);
    AddTestCase(new EpcTftClassifierTestCase(c1, EpcTft::UPLINK, "2.2.3.4",
                                             "1.0.0.0", 2, 123, 5, 1, useIpv6),
                TestCase::QUICK);
    AddTestCase(new EpcTftClassifierTestCase(c1, EpcTft::UPLINK, "6.2.3.4",
                                             "1.1.1.1", 4, 1234, 0, 0, useIpv6),
                TestCase::QUICK);
    AddTestCase(new EpcTftClassifierTestCase(c1, EpcTft::DOWNLINK, "3.3.3.4",
                                             "4.4.4.1", 4, 1234, 0, 1, useIpv6),
                TestCase::QUICK);
    AddTestCase(new EpcTftClassifierTestCase(c1, EpcTft::DOWNLINK, "3.3.4.4",
                                             "4.4.4.1", 4, 1234, 0, 0, useIpv6),
                TestCase::QUICK);
    AddTestCase(new EpcTftClassifierTestCase(c1, EpcTft::UPLINK, "3.3.3.4",
                                             "4.4.2.1", 4, 1234, 0, 0, useIpv6),
                TestCase::QUICK);

    AddTestCase(new EpcTftClassifierTestCase(c1, EpcTft::UPLINK, "9.1.1.1",
                                             "8.1.1.1", 4, 1024, 0, 2, useIpv6),
                TestCase::QUICK);
    AddTestCase(new EpcTftClassifierTestCase(c1, EpcTft::UPLINK, "9.1.1.1",
                                             "8.1.1.1", 4, 1025, 0, 2, useIpv6),
                TestCase::QUICK);
    AddTestCase(new EpcTftClassifierTestCase(c1, EpcTft::UPLINK, "9.1.1.1",
                                             "8.1.1.1", 4, 1035, 0, 2, useIpv6),
                TestCase::QUICK);
    AddTestCase(new EpcTftClassifierTestCase(c1, EpcTft::UPLINK, "9.1.1.1",
                                             "8.1.1.1", 4, 1234, 0, 0, useIpv6),
                TestCase::QUICK);
    AddTestCase(new EpcTftClassifierTestCase(c1, EpcTft::DOWNLINK, "9.1.1.1",
                                             "8.1.1.1", 4, 1024, 0, 0, useIpv6),
                TestCase::QUICK);
    AddTestCase(new EpcTftClassifierTestCase(c1, EpcTft::DOWNLINK, "9.1.1.1",
                                             "8.1.1.1", 4, 1025, 0, 0, useIpv6),
                TestCase::QUICK);
    AddTestCase(new EpcTftClassifierTestCase(c1, EpcTft::DOWNLINK, "9.1.1.1",
                                             "8.1.1.1", 4, 1035, 0, 0, useIpv6),
                TestCase::QUICK);

    AddTestCase(new EpcTftClassifierTestCase(c1, EpcTft::UPLINK, "9.1.1.1",
                                             "8.1.1.1", 4, 3456, 0, 0, useIpv6),
                TestCase::QUICK);
    AddTestCase(new EpcTftClassifierTestCase(c1, EpcTft::UPLINK, "9.1.1.1",
                                             "8.1.1.1", 4, 3457, 0, 0, useIpv6),
                TestCase::QUICK);
    AddTestCase(new EpcTftClassifierTestCase(c1, EpcTft::UPLINK, "9.1.1.1",
                                             "8.1.1.1", 4, 3489, 0, 0, useIpv6),
                TestCase::QUICK);
    AddTestCase(new EpcTftClassifierTestCase(c1, EpcTft::UPLINK, "9.1.1.1",
                                             "8.1.1.1", 3456, 6, 0, 2, useIpv6),
                TestCase::QUICK);
    AddTestCase(new EpcTftClassifierTestCase(c1, EpcTft::DOWNLINK, "9.1.1.1",
                                             "8.1.1.1", 3461, 3461, 0, 2,
                                             useIpv6),
                TestCase::QUICK);
    AddTestCase(new EpcTftClassifierTestCase(c1, EpcTft::DOWNLINK, "9.1.1.1",
                                             "8.1.1.1", 9, 3489, 0, 2, useIpv6),
                TestCase::QUICK);
    AddTestCase(new EpcTftClassifierTestCase(c1, EpcTft::DOWNLINK, "9.1.1.1",
                                             "8.1.1.1", 9, 7895, 0, 2, useIpv6),
                TestCase::QUICK);
    AddTestCase(new EpcTftClassifierTestCase(c1, EpcTft::UPLINK, "9.1.1.1",
                                             "8.1.1.1", 7895, 10, 0, 2,
                                             useIpv6),
                TestCase::QUICK);
    AddTestCase(new EpcTftClassifierTestCase(c1, EpcTft::UPLINK, "9.1.1.1",
                                             "8.1.1.1", 9, 5897, 0, 2, useIpv6),
                TestCase::QUICK);
    AddTestCase(new EpcTftClassifierTestCase(c1, EpcTft::DOWNLINK, "9.1.1.1",
                                             "8.1.1.1", 5897, 10, 0, 2,
                                             useIpv6),
                TestCase::QUICK);

    Ptr<EpcTftClassifier> c2 = Create<EpcTftClassifier>();
    c2->Add(EpcTft::Default(), 1);

    AddTestCase(new EpcTftClassifierTestCase(c2, EpcTft::UPLINK, "2.2.3.4",
                                             "1.1.1.1", 4, 1234, 0, 1, useIpv6),
                TestCase::QUICK);
    AddTestCase(new EpcTftClassifierTestCase(c2, EpcTft::UPLINK, "2.2.3.4",
                                             "1.0.0.0", 2, 123, 5, 1, useIpv6),
                TestCase::QUICK);
    AddTestCase(new EpcTftClassifierTestCase(c2, EpcTft::UPLINK, "6.2.3.4",
                                             "1.1.1.1", 4, 1234, 0, 1, useIpv6),
                TestCase::QUICK);
    AddTestCase(new EpcTftClassifierTestCase(c2, EpcTft::DOWNLINK, "3.3.3.4",
                                             "4.4.4.1", 4, 1234, 0, 1, useIpv6),
                TestCase::QUICK);
    AddTestCase(new EpcTftClassifierTestCase(c2, EpcTft::DOWNLINK, "3.3.4.4",
                                             "4.4.4.1", 4, 1234, 0, 1, useIpv6),
                TestCase::QUICK);
    AddTestCase(new EpcTftClassifierTestCase(c2, EpcTft::UPLINK, "3.3.3.4",
                                             "4.4.2.1", 4, 1234, 0, 1, useIpv6),
                TestCase::QUICK);

    AddTestCase(new EpcTftClassifierTestCase(c2, EpcTft::UPLINK, "9.1.1.1",
                                             "8.1.1.1", 4, 1024, 0, 1, useIpv6),
                TestCase::QUICK);
    AddTestCase(new EpcTftClassifierTestCase(c2, EpcTft::UPLINK, "9.1.1.1",
                                             "8.1.1.1", 4, 1025, 0, 1, useIpv6),
                TestCase::QUICK);
    AddTestCase(new EpcTftClassifierTestCase(c2, EpcTft::UPLINK, "9.1.1.1",
                                             "8.1.1.1", 4, 1035, 0, 1, useIpv6),
                TestCase::QUICK);
    AddTestCase(new EpcTftClassifierTestCase(c2, EpcTft::UPLINK, "9.1.1.1",
                                             "8.1.1.1", 4, 1234, 0, 1, useIpv6),
                TestCase::QUICK);
    AddTestCase(new EpcTftClassifierTestCase(c2, EpcTft::DOWNLINK, "9.1.1.1",
                                             "8.1.1.1", 4, 1024, 0, 1, useIpv6),
                TestCase::QUICK);
    AddTestCase(new EpcTftClassifierTestCase(c2, EpcTft::DOWNLINK, "9.1.1.1",
                                             "8.1.1.1", 4, 1025, 0, 1, useIpv6),
                TestCase::QUICK);
    AddTestCase(new EpcTftClassifierTestCase(c2, EpcTft::DOWNLINK, "9.1.1.1",
                                             "8.1.1.1", 4, 1035, 0, 1, useIpv6),
                TestCase::QUICK);

    AddTestCase(new EpcTftClassifierTestCase(c2, EpcTft::UPLINK, "9.1.1.1",
                                             "8.1.1.1", 4, 3456, 0, 1, useIpv6),
                TestCase::QUICK);
    AddTestCase(new EpcTftClassifierTestCase(c2, EpcTft::UPLINK, "9.1.1.1",
                                             "8.1.1.1", 4, 3457, 0, 1, useIpv6),
                TestCase::QUICK);
    AddTestCase(new EpcTftClassifierTestCase(c2, EpcTft::UPLINK, "9.1.1.1",
                                             "8.1.1.1", 4, 3489, 0, 1, useIpv6),
                TestCase::QUICK);
    AddTestCase(new EpcTftClassifierTestCase(c2, EpcTft::UPLINK, "9.1.1.1",
                                             "8.1.1.1", 3456, 6, 0, 1, useIpv6),
                TestCase::QUICK);
    AddTestCase(new EpcTftClassifierTestCase(c2, EpcTft::DOWNLINK, "9.1.1.1",
                                             "8.1.1.1", 3461, 3461, 0, 1,
                                             useIpv6),
                TestCase::QUICK);
    AddTestCase(new EpcTftClassifierTestCase(c2, EpcTft::DOWNLINK, "9.1.1.1",
                                             "8.1.1.1", 9, 3489, 0, 1, useIpv6),
                TestCase::QUICK);

    Ptr<EpcTftClassifier> c3 = Create<EpcTftClassifier>();
    c3->Add(EpcTft::Default(), 1);
    c3->Add(tft1_1, 2);
    c3->Add(tft1_2, 3);

    AddTestCase(new EpcTftClassifierTestCase(c3, EpcTft::UPLINK, "2.2.3.4",
                                             "1.1.1.1", 4, 1234, 0, 2, useIpv6),
                TestCase::QUICK);
    AddTestCase(new EpcTftClassifierTestCase(c3, EpcTft::UPLINK, "2.2.3.4",
                                             "1.0.0.0", 2, 123, 5, 2, useIpv6),
                TestCase::QUICK);
    AddTestCase(new EpcTftClassifierTestCase(c3, EpcTft::UPLINK, "6.2.3.4",
                                             "1.1.1.1", 4, 1234, 0, 1, useIpv6),
                TestCase::QUICK);
    AddTestCase(new EpcTftClassifierTestCase(c3, EpcTft::DOWNLINK, "3.3.3.4",
                                             "4.4.4.1", 4, 1234, 0, 2, useIpv6),
                TestCase::QUICK);
    AddTestCase(new EpcTftClassifierTestCase(c3, EpcTft::DOWNLINK, "3.3.4.4",
                                             "4.4.4.1", 4, 1234, 0, 1, useIpv6),
                TestCase::QUICK);
    AddTestCase(new EpcTftClassifierTestCase(c3, EpcTft::UPLINK, "3.3.3.4",
                                             "4.4.2.1", 4, 1234, 0, 1, useIpv6),
                TestCase::QUICK);

    AddTestCase(new EpcTftClassifierTestCase(c3, EpcTft::UPLINK, "9.1.1.1",
                                             "8.1.1.1", 4, 1024, 0, 3, useIpv6),
                TestCase::QUICK);
    AddTestCase(new EpcTftClassifierTestCase(c3, EpcTft::UPLINK, "9.1.1.1",
                                             "8.1.1.1", 4, 1025, 0, 3, useIpv6),
                TestCase::QUICK);
    AddTestCase(new EpcTftClassifierTestCase(c3, EpcTft::UPLINK, "9.1.1.1",
                                             "8.1.1.1", 4, 1035, 0, 3, useIpv6),
                TestCase::QUICK);
    AddTestCase(new EpcTftClassifierTestCase(c3, EpcTft::UPLINK, "9.1.1.1",
                                             "8.1.1.1", 4, 1234, 0, 1, useIpv6),
                TestCase::QUICK);
    AddTestCase(new EpcTftClassifierTestCase(c3, EpcTft::DOWNLINK, "9.1.1.1",
                                             "8.1.1.1", 4, 1024, 0, 1, useIpv6),
                TestCase::QUICK);
    AddTestCase(new EpcTftClassifierTestCase(c3, EpcTft::DOWNLINK, "9.1.1.1",
                                             "8.1.1.1", 4, 1025, 0, 1, useIpv6),
                TestCase::QUICK);
    AddTestCase(new EpcTftClassifierTestCase(c3, EpcTft::DOWNLINK, "9.1.1.1",
                                             "8.1.1.1", 4, 1035, 0, 1, useIpv6),
                TestCase::QUICK);

    AddTestCase(new EpcTftClassifierTestCase(c3, EpcTft::UPLINK, "9.1.1.1",
                                             "8.1.1.1", 4, 3456, 0, 1, useIpv6),
                TestCase::QUICK);
    AddTestCase(new EpcTftClassifierTestCase(c3, EpcTft::UPLINK, "9.1.1.1",
                                             "8.1.1.1", 4, 3457, 0, 1, useIpv6),
                TestCase::QUICK);
    AddTestCase(new EpcTftClassifierTestCase(c3, EpcTft::UPLINK, "9.1.1.1",
                                             "8.1.1.1", 4, 3489, 0, 1, useIpv6),
                TestCase::QUICK);
    AddTestCase(new EpcTftClassifierTestCase(c3, EpcTft::UPLINK, "9.1.1.1",
                                             "8.1.1.1", 3456, 6, 0, 3, useIpv6),
                TestCase::QUICK);
    AddTestCase(new EpcTftClassifierTestCase(c3, EpcTft::DOWNLINK, "9.1.1.1",
                                             "8.1.1.1", 3461, 3461, 0, 3,
                                             useIpv6),
                TestCase::QUICK);
    AddTestCase(new EpcTftClassifierTestCase(c3, EpcTft::DOWNLINK, "9.1.1.1",
                                             "8.1.1.1", 9, 3489, 0, 3, useIpv6),
                TestCase::QUICK);

    Ptr<EpcTftClassifier> c4 = Create<EpcTftClassifier>();
    Ptr<EpcTft> tft4_1 = Create<EpcTft>();
    tft4_1->Add(pf1_2_3);
    c4->Add(tft4_1, 1);
    Ptr<EpcTft> tft4_2 = Create<EpcTft>();
    tft4_2->Add(pf1_2_4);
    c4->Add(tft4_2, 2);
    AddTestCase(new EpcTftClassifierTestCase(c4, EpcTft::DOWNLINK, "9.1.1.1",
                                             "8.1.1.1", 9, 3489, 0, 0, useIpv6),
                TestCase::QUICK);
    AddTestCase(new EpcTftClassifierTestCase(c4, EpcTft::DOWNLINK, "9.1.1.1",
                                             "8.1.1.1", 9, 7895, 0, 1, useIpv6),
                TestCase::QUICK);
    AddTestCase(new EpcTftClassifierTestCase(c4, EpcTft::UPLINK, "9.1.1.1",
                                             "8.1.1.1", 7895, 10, 0, 1,
                                             useIpv6),
                TestCase::QUICK);
    AddTestCase(new EpcTftClassifierTestCase(c4, EpcTft::UPLINK, "9.1.1.1",
                                             "8.1.1.1", 9, 5897, 0, 2, useIpv6),
                TestCase::QUICK);
    AddTestCase(new EpcTftClassifierTestCase(c4, EpcTft::DOWNLINK, "9.1.1.1",
                                             "8.1.1.1", 5897, 10, 0, 2,
                                             useIpv6),
                TestCase::QUICK);
  }
}
