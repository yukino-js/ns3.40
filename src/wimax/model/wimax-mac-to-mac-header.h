#ifndef WIMAX_MAC_TO_MAC_HEADER_H
#define WIMAX_MAC_TO_MAC_HEADER_H

#include "ns3/header.h"

#include <stdint.h>

namespace ns3 {

class WimaxMacToMacHeader : public Header {
public:
  WimaxMacToMacHeader();
  ~WimaxMacToMacHeader() override;
  WimaxMacToMacHeader(uint32_t len);

  static TypeId GetTypeId();
  TypeId GetInstanceTypeId() const override;
  uint32_t GetSerializedSize() const override;
  void Serialize(Buffer::Iterator start) const override;
  uint32_t Deserialize(Buffer::Iterator start) override;
  uint8_t GetSizeOfLen() const;
  void Print(std::ostream &os) const override;

private:
  uint32_t m_len;
};
}; // namespace ns3
#endif
