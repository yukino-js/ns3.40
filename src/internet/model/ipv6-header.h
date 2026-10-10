
#ifndef IPV6_HEADER_H
#define IPV6_HEADER_H

#include "ns3/header.h"
#include "ns3/ipv6-address.h"

namespace ns3 {

class Ipv6Header : public Header {
public:
  enum DscpType {
    DscpDefault = 0x00,

    DSCP_CS1 = 0x08,
    DSCP_AF11 = 0x0A,
    DSCP_AF12 = 0x0C,
    DSCP_AF13 = 0x0E,

    DSCP_CS2 = 0x10,
    DSCP_AF21 = 0x12,
    DSCP_AF22 = 0x14,
    DSCP_AF23 = 0x16,

    DSCP_CS3 = 0x18,
    DSCP_AF31 = 0x1A,
    DSCP_AF32 = 0x1C,
    DSCP_AF33 = 0x1E,

    DSCP_CS4 = 0x20,
    DSCP_AF41 = 0x22,
    DSCP_AF42 = 0x24,
    DSCP_AF43 = 0x26,

    DSCP_CS5 = 0x28,
    DSCP_EF = 0x2E,

    DSCP_CS6 = 0x30,
    DSCP_CS7 = 0x38
  };

  enum NextHeader_e {
    IPV6_EXT_HOP_BY_HOP = 0,
    IPV6_IPV4 = 4,
    IPV6_TCP = 6,
    IPV6_UDP = 17,
    IPV6_IPV6 = 41,
    IPV6_EXT_ROUTING = 43,
    IPV6_EXT_FRAGMENTATION = 44,
    IPV6_EXT_CONFIDENTIALITY = 50,
    IPV6_EXT_AUTHENTICATION = 51,
    IPV6_ICMPV6 = 58,
    IPV6_EXT_END = 59,
    IPV6_EXT_DESTINATION = 60,
    IPV6_SCTP = 135,
    IPV6_EXT_MOBILITY = 135,
    IPV6_UDP_LITE = 136,
  };

  static TypeId GetTypeId();

  TypeId GetInstanceTypeId() const override;

  Ipv6Header();

  void SetTrafficClass(uint8_t traffic);

  uint8_t GetTrafficClass() const;

  void SetDscp(DscpType dscp);

  DscpType GetDscp() const;

  std::string DscpTypeToString(DscpType dscp) const;

  enum EcnType {
    ECN_NotECT = 0x00,
    ECN_ECT1 = 0x01,
    ECN_ECT0 = 0x02,
    ECN_CE = 0x03
  };

  void SetEcn(EcnType ecn);

  EcnType GetEcn() const;

  std::string EcnTypeToString(EcnType ecn) const;

  void SetFlowLabel(uint32_t flow);

  uint32_t GetFlowLabel() const;

  void SetPayloadLength(uint16_t len);

  uint16_t GetPayloadLength() const;

  void SetNextHeader(uint8_t next);

  uint8_t GetNextHeader() const;

  void SetHopLimit(uint8_t limit);

  uint8_t GetHopLimit() const;

  void SetSource(Ipv6Address src);

  Ipv6Address GetSource() const;

  void SetDestination(Ipv6Address dst);

  Ipv6Address GetDestination() const;

  void Print(std::ostream &os) const override;

  uint32_t GetSerializedSize() const override;

  void Serialize(Buffer::Iterator start) const override;

  uint32_t Deserialize(Buffer::Iterator start) override;

private:
  uint32_t m_trafficClass : 8;

  uint32_t m_flowLabel : 20;

  uint16_t m_payloadLength;

  uint8_t m_nextHeader;

  uint8_t m_hopLimit;

  Ipv6Address m_sourceAddress;

  Ipv6Address m_destinationAddress;
};

} // namespace ns3

#endif
