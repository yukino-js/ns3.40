
#ifndef PPP_HEADER_H
#define PPP_HEADER_H

#include "ns3/header.h"

namespace ns3 {

class PppHeader : public Header {
public:
  PppHeader();

  ~PppHeader() override;

  static TypeId GetTypeId();

  TypeId GetInstanceTypeId() const override;

  void Print(std::ostream &os) const override;
  void Serialize(Buffer::Iterator start) const override;
  uint32_t Deserialize(Buffer::Iterator start) override;
  uint32_t GetSerializedSize() const override;

  void SetProtocol(uint16_t protocol);

  uint16_t GetProtocol() const;

private:
  uint16_t m_protocol;
};

} // namespace ns3

#endif
