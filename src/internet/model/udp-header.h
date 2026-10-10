
#ifndef UDP_HEADER_H
#define UDP_HEADER_H

#include "ns3/header.h"
#include "ns3/ipv4-address.h"
#include "ns3/ipv6-address.h"

#include <stdint.h>
#include <string>

namespace ns3 {
class UdpHeader : public Header {
public:
  void EnableChecksums();
  void SetDestinationPort(uint16_t port);
  void SetSourcePort(uint16_t port);
  uint16_t GetSourcePort() const;
  uint16_t GetDestinationPort() const;

  void InitializeChecksum(Address source, Address destination,
                          uint8_t protocol);

  void InitializeChecksum(Ipv4Address source, Ipv4Address destination,
                          uint8_t protocol);

  void InitializeChecksum(Ipv6Address source, Ipv6Address destination,
                          uint8_t protocol);

  static TypeId GetTypeId();
  TypeId GetInstanceTypeId() const override;
  void Print(std::ostream &os) const override;
  uint32_t GetSerializedSize() const override;
  void Serialize(Buffer::Iterator start) const override;
  uint32_t Deserialize(Buffer::Iterator start) override;

  bool IsChecksumOk() const;

  void ForceChecksum(uint16_t checksum);

  void ForcePayloadSize(uint16_t payloadSize);

  uint16_t GetChecksum() const;

private:
  uint16_t CalculateHeaderChecksum(uint16_t size) const;

  uint16_t m_sourcePort{0xfffd};
  uint16_t m_destinationPort{0xfffd};
  uint16_t m_payloadSize{0};
  uint16_t m_forcedPayloadSize{0};

  Address m_source;
  Address m_destination;
  uint8_t m_protocol{17};
  uint16_t m_checksum{0};
  bool m_calcChecksum{false};
  bool m_goodChecksum{true};
};

} // namespace ns3

#endif
