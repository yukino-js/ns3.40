#ifndef SLL_HEADER_H
#define SLL_HEADER_H

#include "ns3/buffer.h"
#include "ns3/header.h"

#include <stdint.h>

namespace ns3 {

class SllHeader : public Header {
public:
  enum PacketType {
    UNICAST_FROM_PEER_TO_ME = 0,
    BROADCAST_BY_PEER = 1,
    MULTICAST_BY_PEER = 2,
    INTERCEPTED_PACKET = 3,
    SENT_BY_US
  };

  static TypeId GetTypeId();

  SllHeader();
  ~SllHeader() override;

  uint16_t GetArpType() const;

  void SetArpType(uint16_t arphdType);

  PacketType GetPacketType() const;

  void SetPacketType(PacketType type);

  TypeId GetInstanceTypeId() const override;
  uint32_t GetSerializedSize() const override;
  void Serialize(Buffer::Iterator start) const override;
  uint32_t Deserialize(Buffer::Iterator start) override;
  void Print(std::ostream &os) const override;

protected:
  PacketType m_packetType;
  uint16_t m_arphdType;
  uint16_t m_addressLength;
  uint64_t m_address;
  uint16_t m_protocolType;
};

} // namespace ns3

#endif
