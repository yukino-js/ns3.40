
#ifndef ARP_HEADER_H
#define ARP_HEADER_H

#include "ns3/address.h"
#include "ns3/header.h"
#include "ns3/ipv4-address.h"

#include <string>

namespace ns3 {
class ArpHeader : public Header {
public:
  void SetRequest(Address sourceHardwareAddress,
                  Ipv4Address sourceProtocolAddress,
                  Address destinationHardwareAddress,
                  Ipv4Address destinationProtocolAddress);
  void SetReply(Address sourceHardwareAddress,
                Ipv4Address sourceProtocolAddress,
                Address destinationHardwareAddress,
                Ipv4Address destinationProtocolAddress);

  bool IsRequest() const;

  bool IsReply() const;

  Address GetSourceHardwareAddress() const;

  Address GetDestinationHardwareAddress() const;

  Ipv4Address GetSourceIpv4Address() const;

  Ipv4Address GetDestinationIpv4Address() const;

  static TypeId GetTypeId();
  TypeId GetInstanceTypeId() const override;
  void Print(std::ostream &os) const override;
  uint32_t GetSerializedSize() const override;
  void Serialize(Buffer::Iterator start) const override;
  uint32_t Deserialize(Buffer::Iterator start) override;

  enum ArpType_e { ARP_TYPE_REQUEST = 1, ARP_TYPE_REPLY = 2 };

  uint16_t m_type;
  Address m_macSource;
  Address m_macDest;
  Ipv4Address m_ipv4Source;
  Ipv4Address m_ipv4Dest;
};

} // namespace ns3

#endif
