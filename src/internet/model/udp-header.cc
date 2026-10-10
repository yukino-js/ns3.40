
#include "udp-header.h"

#include "ns3/address-utils.h"

namespace ns3 {

NS_OBJECT_ENSURE_REGISTERED(UdpHeader);

void UdpHeader::EnableChecksums() { m_calcChecksum = true; }

void UdpHeader::SetDestinationPort(uint16_t port) { m_destinationPort = port; }

void UdpHeader::SetSourcePort(uint16_t port) { m_sourcePort = port; }

uint16_t UdpHeader::GetSourcePort() const { return m_sourcePort; }

uint16_t UdpHeader::GetDestinationPort() const { return m_destinationPort; }

void UdpHeader::InitializeChecksum(Address source, Address destination,
                                   uint8_t protocol) {
  m_source = source;
  m_destination = destination;
  m_protocol = protocol;
}

void UdpHeader::InitializeChecksum(Ipv4Address source, Ipv4Address destination,
                                   uint8_t protocol) {
  m_source = source;
  m_destination = destination;
  m_protocol = protocol;
}

void UdpHeader::InitializeChecksum(Ipv6Address source, Ipv6Address destination,
                                   uint8_t protocol) {
  m_source = source;
  m_destination = destination;
  m_protocol = protocol;
}

uint16_t UdpHeader::CalculateHeaderChecksum(uint16_t size) const {
  Buffer buf = Buffer((2 * Address::MAX_SIZE) + 8);
  buf.AddAtStart((2 * Address::MAX_SIZE) + 8);
  Buffer::Iterator it = buf.Begin();
  uint32_t hdrSize = 0;

  WriteTo(it, m_source);
  WriteTo(it, m_destination);
  if (Ipv4Address::IsMatchingType(m_source)) {
    it.WriteU8(0);
    it.WriteU8(m_protocol);
    it.WriteU8(size >> 8);
    it.WriteU8(size & 0xff);
    hdrSize = 12;
  } else if (Ipv6Address::IsMatchingType(m_source)) {
    it.WriteU16(0);
    it.WriteU8(size >> 8);
    it.WriteU8(size & 0xff);
    it.WriteU16(0);
    it.WriteU8(0);
    it.WriteU8(m_protocol);
    hdrSize = 40;
  }

  it = buf.Begin();
  return ~(it.CalculateIpChecksum(hdrSize));
}

bool UdpHeader::IsChecksumOk() const { return m_goodChecksum; }

void UdpHeader::ForceChecksum(uint16_t checksum) { m_checksum = checksum; }

void UdpHeader::ForcePayloadSize(uint16_t payloadSize) {
  m_forcedPayloadSize = payloadSize;
}

TypeId UdpHeader::GetTypeId() {
  static TypeId tid = TypeId("ns3::UdpHeader")
                          .SetParent<Header>()
                          .SetGroupName("Internet")
                          .AddConstructor<UdpHeader>();
  return tid;
}

TypeId UdpHeader::GetInstanceTypeId() const { return GetTypeId(); }

void UdpHeader::Print(std::ostream &os) const {
  os << "length: " << m_payloadSize + GetSerializedSize() << " " << m_sourcePort
     << " > " << m_destinationPort;
}

uint32_t UdpHeader::GetSerializedSize() const { return 8; }

void UdpHeader::Serialize(Buffer::Iterator start) const {
  Buffer::Iterator i = start;

  i.WriteHtonU16(m_sourcePort);
  i.WriteHtonU16(m_destinationPort);
  if (m_forcedPayloadSize == 0) {
    i.WriteHtonU16(start.GetSize());
  } else {
    i.WriteHtonU16(m_forcedPayloadSize);
  }

  if (m_checksum == 0) {
    i.WriteU16(0);

    if (m_calcChecksum) {
      uint16_t headerChecksum = CalculateHeaderChecksum(start.GetSize());
      i = start;
      uint16_t checksum =
          i.CalculateIpChecksum(start.GetSize(), headerChecksum);

      i = start;
      i.Next(6);
      i.WriteU16(checksum);
    }
  } else {
    i.WriteU16(m_checksum);
  }
}

uint32_t UdpHeader::Deserialize(Buffer::Iterator start) {
  Buffer::Iterator i = start;
  m_sourcePort = i.ReadNtohU16();
  m_destinationPort = i.ReadNtohU16();
  m_payloadSize = i.ReadNtohU16() - GetSerializedSize();
  m_checksum = i.ReadU16();

  if (m_calcChecksum) {
    uint16_t headerChecksum = CalculateHeaderChecksum(start.GetSize());
    i = start;
    uint16_t checksum = i.CalculateIpChecksum(start.GetSize(), headerChecksum);

    m_goodChecksum = (checksum == 0);
  }

  return GetSerializedSize();
}

uint16_t UdpHeader::GetChecksum() const { return m_checksum; }

} // namespace ns3
