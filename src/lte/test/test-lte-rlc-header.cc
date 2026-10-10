
#include "ns3/log.h"
#include "ns3/lte-rlc-am-header.h"
#include "ns3/packet.h"
#include "ns3/ptr.h"
#include "ns3/test.h"

#include <bitset>
#include <iomanip>
#include <list>

NS_LOG_COMPONENT_DEFINE("TestLteRlcHeader");

namespace ns3 {

class TestUtils {
public:
  static std::string sprintPacketContentsHex(Ptr<Packet> pkt) {
    uint32_t psize = pkt->GetSize();
    uint8_t buffer[psize];
    std::ostringstream oss(std::ostringstream::out);
    pkt->CopyData(buffer, psize);
    for (uint32_t i = 0; i < psize; i++) {
      oss << std::setfill('0') << std::setw(2) << std::hex
          << (uint32_t)buffer[i];
    }
    return oss.str();
  }

  static std::string sprintPacketContentsBin(Ptr<Packet> pkt) {
    uint32_t psize = pkt->GetSize();
    uint8_t buffer[psize];
    std::ostringstream oss(std::ostringstream::out);
    pkt->CopyData(buffer, psize);
    for (uint32_t i = 0; i < psize; i++) {
      oss << (std::bitset<8>(buffer[i]));
    }
    return std::string(oss.str() + "\n");
  }

  static void LogPacketContents(Ptr<Packet> pkt) {
    NS_LOG_DEBUG("---- SERIALIZED PACKET CONTENTS (HEX): -------");
    NS_LOG_DEBUG("Hex: " << TestUtils::sprintPacketContentsHex(pkt));
    NS_LOG_DEBUG("Bin: " << TestUtils::sprintPacketContentsBin(pkt));
  }

  template <class T> static void LogPacketInfo(T source, std::string s) {
    NS_LOG_DEBUG("--------- " << s.data() << " INFO: -------");
    std::ostringstream oss(std::ostringstream::out);
    source.Print(oss);
    NS_LOG_DEBUG(oss.str());
  }
};

class RlcAmStatusPduTestCase : public TestCase {
public:
  RlcAmStatusPduTestCase(SequenceNumber10 ackSn,
                         std::list<SequenceNumber10> nackSnList,
                         std::string hex);

protected:
  void DoRun() override;

  SequenceNumber10 m_ackSn;
  std::list<SequenceNumber10> m_nackSnList;
  std::string m_hex;
};

RlcAmStatusPduTestCase::RlcAmStatusPduTestCase(
    SequenceNumber10 ackSn, std::list<SequenceNumber10> nackSnList,
    std::string hex)
    : TestCase(hex), m_ackSn(ackSn), m_nackSnList(nackSnList), m_hex(hex) {
  NS_LOG_FUNCTION(this << hex);
}

void

RlcAmStatusPduTestCase::DoRun()
{
  NS_LOG_FUNCTION(this);

  Ptr<Packet> p = Create<Packet>();
  LteRlcAmHeader h;
  h.SetControlPdu(LteRlcAmHeader::STATUS_PDU);
  h.SetAckSn(m_ackSn);
  for (auto it = m_nackSnList.begin(); it != m_nackSnList.end(); ++it) {
    h.PushNack(it->GetValue());
  }
  p->AddHeader(h);

  TestUtils::LogPacketContents(p);
  std::string hex = TestUtils::sprintPacketContentsHex(p);
  NS_TEST_ASSERT_MSG_EQ(m_hex, hex,
                        "serialized packet content "
                            << hex << " differs from test vector " << m_hex);

  LteRlcAmHeader h2;
  p->RemoveHeader(h2);
  SequenceNumber10 ackSn = h2.GetAckSn();
  NS_TEST_ASSERT_MSG_EQ(ackSn, m_ackSn,
                        "deserialized ACK SN differs from test vector");

  for (auto it = m_nackSnList.begin(); it != m_nackSnList.end(); ++it) {
    int nackSn = h2.PopNack();
    NS_TEST_ASSERT_MSG_GT(nackSn, -1,
                          "not enough elements in deserialized NACK list");
    NS_TEST_ASSERT_MSG_EQ(nackSn, it->GetValue(),
                          "deserialized NACK SN  differs from test vector");
  }
  int retVal = h2.PopNack();
  NS_TEST_ASSERT_MSG_LT(retVal, 0,
                        "too many elements in deserialized NACK list");
}

class LteRlcHeaderTestSuite : public TestSuite {
public:
  LteRlcHeaderTestSuite();
} staticLteRlcHeaderTestSuiteInstance;

LteRlcHeaderTestSuite::LteRlcHeaderTestSuite()
    : TestSuite("lte-rlc-header", UNIT) {
  NS_LOG_FUNCTION(this);

  {
    SequenceNumber10 ackSn(8);
    std::list<SequenceNumber10> nackSnList;
    std::string hex("0020");
    AddTestCase(new RlcAmStatusPduTestCase(ackSn, nackSnList, hex),
                TestCase::QUICK);
  }

  {
    SequenceNumber10 ackSn(873);
    std::list<SequenceNumber10> nackSnList;
    std::string hex("0da4");
    AddTestCase(new RlcAmStatusPduTestCase(ackSn, nackSnList, hex),
                TestCase::QUICK);
  }

  {
    SequenceNumber10 ackSn(2);
    const std::list<SequenceNumber10> nackSnList{
        SequenceNumber10(873),
    };
    std::string hex("000bb480");
    AddTestCase(new RlcAmStatusPduTestCase(ackSn, nackSnList, hex),
                TestCase::QUICK);
  }

  {
    SequenceNumber10 ackSn(2);
    const std::list<SequenceNumber10> nackSnList{
        SequenceNumber10(1021),
        SequenceNumber10(754),
    };
    std::string hex("000bfed790");
    AddTestCase(new RlcAmStatusPduTestCase(ackSn, nackSnList, hex),
                TestCase::QUICK);
  }

  {
    SequenceNumber10 ackSn(2);
    const std::list<SequenceNumber10> nackSnList{
        SequenceNumber10(1021),
        SequenceNumber10(754),
        SequenceNumber10(947),
    };
    std::string hex("000bfed795d980");
    AddTestCase(new RlcAmStatusPduTestCase(ackSn, nackSnList, hex),
                TestCase::QUICK);
  }

  {
    SequenceNumber10 ackSn(2);
    const std::list<SequenceNumber10> nackSnList{
        SequenceNumber10(1021),
        SequenceNumber10(754),
        SequenceNumber10(947),
        SequenceNumber10(347),
    };
    std::string hex("000bfed795d9cad8");
    AddTestCase(new RlcAmStatusPduTestCase(ackSn, nackSnList, hex),
                TestCase::QUICK);
  }
}

} // namespace ns3
