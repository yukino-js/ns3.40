
#ifndef SEQ_TS_HEADER_H
#define SEQ_TS_HEADER_H

#include "ns3/header.h"
#include "ns3/nstime.h"

namespace ns3 {
class SeqTsHeader : public Header {
public:
  SeqTsHeader();

  void SetSeq(uint32_t seq);
  uint32_t GetSeq() const;
  Time GetTs() const;

  static TypeId GetTypeId();

  TypeId GetInstanceTypeId() const override;
  void Print(std::ostream &os) const override;
  uint32_t GetSerializedSize() const override;
  void Serialize(Buffer::Iterator start) const override;
  uint32_t Deserialize(Buffer::Iterator start) override;

private:
  uint32_t m_seq;
  uint64_t m_ts;
};

} // namespace ns3

#endif
