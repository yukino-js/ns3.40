
#ifndef AMSDU_SUBFRAME_HEADER_H
#define AMSDU_SUBFRAME_HEADER_H

#include "ns3/header.h"
#include "ns3/mac48-address.h"

namespace ns3 {

class AmsduSubframeHeader : public Header {
public:
  AmsduSubframeHeader();
  ~AmsduSubframeHeader() override;

  static TypeId GetTypeId();

  TypeId GetInstanceTypeId() const override;
  void Print(std::ostream &os) const override;
  uint32_t GetSerializedSize() const override;
  void Serialize(Buffer::Iterator start) const override;
  uint32_t Deserialize(Buffer::Iterator start) override;

  void SetDestinationAddr(Mac48Address to);
  void SetSourceAddr(Mac48Address to);
  void SetLength(uint16_t length);
  Mac48Address GetDestinationAddr() const;
  Mac48Address GetSourceAddr() const;
  uint16_t GetLength() const;

private:
  Mac48Address m_da;
  Mac48Address m_sa;
  uint16_t m_length;
};

} // namespace ns3

#endif
