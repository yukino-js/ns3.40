
#ifndef DSR_FS_HEADER_H
#define DSR_FS_HEADER_H

#include "dsr-option-header.h"

#include "ns3/header.h"
#include "ns3/ipv4-address.h"

#include <list>
#include <ostream>
#include <vector>

namespace ns3 {
namespace dsr {

class DsrFsHeader : public Header {
public:
  static TypeId GetTypeId();
  TypeId GetInstanceTypeId() const override;
  DsrFsHeader();
  ~DsrFsHeader() override;
  void SetNextHeader(uint8_t protocol);
  uint8_t GetNextHeader() const;
  void SetMessageType(uint8_t messageType);
  uint8_t GetMessageType() const;
  void SetSourceId(uint16_t sourceId);
  uint16_t GetSourceId() const;
  void SetDestId(uint16_t destId);
  uint16_t GetDestId() const;
  void SetPayloadLength(uint16_t length);
  uint16_t GetPayloadLength() const;
  void Print(std::ostream &os) const override;
  uint32_t GetSerializedSize() const override;
  void Serialize(Buffer::Iterator start) const override;
  uint32_t Deserialize(Buffer::Iterator start) override;

private:
  uint8_t m_nextHeader;
  uint8_t m_messageType;
  uint16_t m_payloadLen;
  uint16_t m_sourceId;
  uint16_t m_destId;
  Buffer m_data;
};

class DsrOptionField {
public:
  DsrOptionField(uint32_t optionsOffset);
  ~DsrOptionField();
  uint32_t GetSerializedSize() const;
  void Serialize(Buffer::Iterator start) const;
  uint32_t Deserialize(Buffer::Iterator start, uint32_t length);
  void AddDsrOption(const DsrOptionHeader &option);
  uint32_t GetDsrOptionsOffset() const;
  Buffer GetDsrOptionBuffer();

private:
  uint32_t CalculatePad(DsrOptionHeader::Alignment alignment) const;
  Buffer m_optionData;
  uint32_t m_optionsOffset;
};

class DsrRoutingHeader : public DsrFsHeader, public DsrOptionField {
public:
  static TypeId GetTypeId();
  TypeId GetInstanceTypeId() const override;
  DsrRoutingHeader();
  ~DsrRoutingHeader() override;
  void Print(std::ostream &os) const override;
  uint32_t GetSerializedSize() const override;
  void Serialize(Buffer::Iterator start) const override;
  uint32_t Deserialize(Buffer::Iterator start) override;
};

static inline std::ostream &operator<<(std::ostream &os,
                                       const DsrRoutingHeader &dsr) {
  dsr.Print(os);
  return os;
}

} // namespace dsr
} // namespace ns3

#endif
