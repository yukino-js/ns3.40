
#ifndef LTE_PDCP_HEADER_H
#define LTE_PDCP_HEADER_H

#include "ns3/header.h"

#include <list>

namespace ns3 {

class LtePdcpHeader : public Header {
public:
  LtePdcpHeader();
  ~LtePdcpHeader() override;

  void SetDcBit(uint8_t dcBit);
  void SetSequenceNumber(uint16_t sequenceNumber);

  uint8_t GetDcBit() const;
  uint16_t GetSequenceNumber() const;

  enum { CONTROL_PDU = 0, DATA_PDU = 1 } DcBit_t;

  static TypeId GetTypeId();
  TypeId GetInstanceTypeId() const override;
  void Print(std::ostream &os) const override;
  uint32_t GetSerializedSize() const override;
  void Serialize(Buffer::Iterator start) const override;
  uint32_t Deserialize(Buffer::Iterator start) override;

private:
  uint8_t m_dcBit;
  uint16_t m_sequenceNumber;
};

} // namespace ns3

#endif
