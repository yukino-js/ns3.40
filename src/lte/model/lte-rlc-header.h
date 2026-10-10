
#ifndef LTE_RLC_HEADER_H
#define LTE_RLC_HEADER_H

#include "lte-rlc-sequence-number.h"

#include "ns3/header.h"

#include <list>

namespace ns3 {

class LteRlcHeader : public Header {
public:
  LteRlcHeader();
  ~LteRlcHeader() override;

  void SetFramingInfo(uint8_t framingInfo);
  void SetSequenceNumber(SequenceNumber10 sequenceNumber);

  uint8_t GetFramingInfo() const;
  SequenceNumber10 GetSequenceNumber() const;

  void PushExtensionBit(uint8_t extensionBit);
  void PushLengthIndicator(uint16_t lengthIndicator);

  uint8_t PopExtensionBit();
  uint16_t PopLengthIndicator();

  enum ExtensionBit_t { DATA_FIELD_FOLLOWS = 0, E_LI_FIELDS_FOLLOWS = 1 };

  enum FramingInfoFirstByte_t { FIRST_BYTE = 0x00, NO_FIRST_BYTE = 0x02 };

  enum FramingInfoLastByte_t { LAST_BYTE = 0x00, NO_LAST_BYTE = 0x01 };

  static TypeId GetTypeId();
  TypeId GetInstanceTypeId() const override;
  void Print(std::ostream &os) const override;
  uint32_t GetSerializedSize() const override;
  void Serialize(Buffer::Iterator start) const override;
  uint32_t Deserialize(Buffer::Iterator start) override;

private:
  uint16_t m_headerLength;
  uint8_t m_framingInfo;
  SequenceNumber10 m_sequenceNumber;

  std::list<uint8_t> m_extensionBits;
  std::list<uint16_t> m_lengthIndicators;
};

}; // namespace ns3

#endif
