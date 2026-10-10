
#ifndef FLAME_HEADER_H
#define FLAME_HEADER_H

#include "ns3/header.h"
#include "ns3/mac48-address.h"

namespace ns3 {
namespace flame {

class FlameHeader : public Header {
public:
  FlameHeader();
  ~FlameHeader() override;

  static TypeId GetTypeId();
  TypeId GetInstanceTypeId() const override;
  void Print(std::ostream &os) const override;
  uint32_t GetSerializedSize() const override;
  void Serialize(Buffer::Iterator start) const override;
  uint32_t Deserialize(Buffer::Iterator start) override;

  void AddCost(uint8_t cost);
  uint8_t GetCost() const;
  void SetSeqno(uint16_t seqno);
  uint16_t GetSeqno() const;
  void SetOrigDst(Mac48Address dst);
  Mac48Address GetOrigDst() const;
  void SetOrigSrc(Mac48Address OrigSrc);
  Mac48Address GetOrigSrc() const;
  void SetProtocol(uint16_t protocol);
  uint16_t GetProtocol() const;

private:
  uint8_t m_cost;
  uint16_t m_seqno;
  Mac48Address m_origDst;
  Mac48Address m_origSrc;
  uint16_t m_protocol;
  friend bool operator==(const FlameHeader &a, const FlameHeader &b);
};

bool operator==(const FlameHeader &a, const FlameHeader &b);
} // namespace flame
} // namespace ns3
#endif
