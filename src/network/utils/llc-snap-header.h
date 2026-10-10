
#ifndef LLC_SNAP_HEADER_H
#define LLC_SNAP_HEADER_H

#include "ns3/header.h"

#include <stdint.h>
#include <string>

namespace ns3 {

static const uint16_t LLC_SNAP_HEADER_LENGTH = 8;

class LlcSnapHeader : public Header {
public:
  LlcSnapHeader();

  void SetType(uint16_t type);
  uint16_t GetType();

  static TypeId GetTypeId();
  TypeId GetInstanceTypeId() const override;
  void Print(std::ostream &os) const override;
  uint32_t GetSerializedSize() const override;
  void Serialize(Buffer::Iterator start) const override;
  uint32_t Deserialize(Buffer::Iterator start) override;

private:
  uint16_t m_etherType;
};

} // namespace ns3

#endif
