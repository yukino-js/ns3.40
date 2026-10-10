
#ifndef ETHERNET_HEADER_H
#define ETHERNET_HEADER_H

#include "mac48-address.h"

#include "ns3/header.h"

#include <string>

namespace ns3 {

enum ethernet_header_t { LENGTH, VLAN, QINQ };

class EthernetHeader : public Header {
public:
  EthernetHeader(bool hasPreamble);
  EthernetHeader();
  void SetLengthType(uint16_t size);
  void SetSource(Mac48Address source);
  void SetDestination(Mac48Address destination);
  void SetPreambleSfd(uint64_t preambleSfd);
  uint16_t GetLengthType() const;
  ethernet_header_t GetPacketType() const;
  Mac48Address GetSource() const;
  Mac48Address GetDestination() const;
  uint64_t GetPreambleSfd() const;
  uint32_t GetHeaderSize() const;

  static TypeId GetTypeId();
  TypeId GetInstanceTypeId() const override;
  void Print(std::ostream &os) const override;
  uint32_t GetSerializedSize() const override;
  void Serialize(Buffer::Iterator start) const override;
  uint32_t Deserialize(Buffer::Iterator start) override;

private:
  static const int PREAMBLE_SIZE = 8;
  static const int LENGTH_SIZE = 2;
  static const int MAC_ADDR_SIZE = 6;

  bool m_enPreambleSfd;
  uint64_t m_preambleSfd;
  uint16_t m_lengthType;
  Mac48Address m_source;
  Mac48Address m_destination;
};

} // namespace ns3

#endif
