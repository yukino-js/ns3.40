
#ifndef IPV6_PACKET_INFO_TAG_H
#define IPV6_PACKET_INFO_TAG_H

#include "ns3/ipv6-address.h"
#include "ns3/tag.h"

namespace ns3 {

class Node;
class Packet;

class Ipv6PacketInfoTag : public Tag {
public:
  Ipv6PacketInfoTag();

  static TypeId GetTypeId();

  void SetAddress(Ipv6Address addr);

  Ipv6Address GetAddress() const;

  void SetRecvIf(uint32_t ifindex);

  uint32_t GetRecvIf() const;

  void SetHoplimit(uint8_t ttl);

  uint8_t GetHoplimit() const;

  void SetTrafficClass(uint8_t tclass);

  uint8_t GetTrafficClass() const;

  TypeId GetInstanceTypeId() const override;
  uint32_t GetSerializedSize() const override;
  void Serialize(TagBuffer i) const override;
  void Deserialize(TagBuffer i) override;
  void Print(std::ostream &os) const override;

private:
  Ipv6Address m_addr;
  uint8_t m_ifindex;
  uint8_t m_hoplimit;
  uint8_t m_tclass;
};
} // namespace ns3

#endif
