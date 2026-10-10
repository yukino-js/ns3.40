
#ifndef IPV4_HEADER_H
#define IPV4_HEADER_H

#include "ns3/header.h"
#include "ns3/ipv4-address.h"

namespace ns3 {
class Ipv4Header : public Header {
public:
  Ipv4Header();
  void EnableChecksum();
  void SetPayloadSize(uint16_t size);
  void SetIdentification(uint16_t identification);
  void SetTos(uint8_t tos);

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

  void SetDscp(DscpType dscp);

  enum EcnType {
    ECN_NotECT = 0x00,
    ECN_ECT1 = 0x01,
    ECN_ECT0 = 0x02,
    ECN_CE = 0x03
  };

  void SetEcn(EcnType ecn);
  void SetMoreFragments();
  void SetLastFragment();
  void SetDontFragment();
  void SetMayFragment();
  void SetFragmentOffset(uint16_t offsetBytes);
  void SetTtl(uint8_t ttl);
  void SetProtocol(uint8_t num);
  void SetSource(Ipv4Address source);
  void SetDestination(Ipv4Address destination);
  uint16_t GetPayloadSize() const;
  uint16_t GetIdentification() const;
  uint8_t GetTos() const;
  DscpType GetDscp() const;
  std::string DscpTypeToString(DscpType dscp) const;
  EcnType GetEcn() const;
  std::string EcnTypeToString(EcnType ecn) const;
  bool IsLastFragment() const;
  bool IsDontFragment() const;
  uint16_t GetFragmentOffset() const;
  uint8_t GetTtl() const;
  uint8_t GetProtocol() const;
  Ipv4Address GetSource() const;
  Ipv4Address GetDestination() const;

  bool IsChecksumOk() const;

  static TypeId GetTypeId();
  TypeId GetInstanceTypeId() const override;
  void Print(std::ostream &os) const override;
  uint32_t GetSerializedSize() const override;
  void Serialize(Buffer::Iterator start) const override;
  uint32_t Deserialize(Buffer::Iterator start) override;

private:
  enum FlagsE { DONT_FRAGMENT = (1 << 0), MORE_FRAGMENTS = (1 << 1) };

  bool m_calcChecksum;

  uint16_t m_payloadSize;
  uint16_t m_identification;
  uint32_t m_tos : 8;
  uint32_t m_ttl : 8;
  uint32_t m_protocol : 8;
  uint32_t m_flags : 3;
  uint16_t m_fragmentOffset;
  Ipv4Address m_source;
  Ipv4Address m_destination;
  uint16_t m_checksum;
  bool m_goodChecksum;
  uint16_t m_headerSize;
};

} // namespace ns3

#endif
