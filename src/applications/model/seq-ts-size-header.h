
#ifndef SEQ_TS_SIZE_HEADER_H
#define SEQ_TS_SIZE_HEADER_H

#include "seq-ts-header.h"

namespace ns3 {
class SeqTsSizeHeader : public SeqTsHeader {
public:
  static TypeId GetTypeId();

  SeqTsSizeHeader();

  void SetSize(uint64_t size);

  uint64_t GetSize() const;

  TypeId GetInstanceTypeId() const override;
  void Print(std::ostream &os) const override;
  uint32_t GetSerializedSize() const override;
  void Serialize(Buffer::Iterator start) const override;
  uint32_t Deserialize(Buffer::Iterator start) override;

private:
  uint64_t m_size{0};
};

} // namespace ns3

#endif
