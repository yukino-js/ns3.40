
#ifndef IPV4_PACKET_INFO_TAG_H
#define IPV4_PACKET_INFO_TAG_H

#include "ns3/ipv4-address.h"
#include "ns3/tag.h"

namespace ns3 {

class Node;
class Packet;

class Ipv4PacketInfoTag : public Tag {
public:
  Ipv4PacketInfoTag();

  void SetAddress(Ipv4Address addr);

  Ipv4Address GetAddress() const;

  void SetRecvIf(uint32_t ifindex);
  uint32_t GetRecvIf() const;

  void SetTtl(uint8_t ttl);
  uint8_t GetTtl() const;

  static TypeId GetTypeId();
  TypeId GetInstanceTypeId() const override;
  uint32_t GetSerializedSize() const override;
  void Serialize(TagBuffer i) const override;
  void Deserialize(TagBuffer i) override;
  void Print(std::ostream &os) const override;

private:
  Ipv4Address m_addr;
  uint32_t m_ifindex;

  uint8_t m_ttl;
};
} // namespace ns3

#endif
