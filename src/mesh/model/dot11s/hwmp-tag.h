
#ifndef HWMP_TAG_H
#define HWMP_TAG_H

#include "ns3/mac48-address.h"
#include "ns3/object.h"
#include "ns3/tag.h"

namespace ns3 {
namespace dot11s {
class HwmpTag : public Tag {
public:
  HwmpTag();
  ~HwmpTag() override;
  void SetAddress(Mac48Address retransmitter);
  Mac48Address GetAddress();
  void SetTtl(uint8_t ttl);
  uint8_t GetTtl() const;
  void SetMetric(uint32_t metric);
  uint32_t GetMetric() const;
  void SetSeqno(uint32_t seqno);
  uint32_t GetSeqno() const;
  void DecrementTtl();

  static TypeId GetTypeId();
  TypeId GetInstanceTypeId() const override;
  uint32_t GetSerializedSize() const override;
  void Serialize(TagBuffer i) const override;
  void Deserialize(TagBuffer i) override;
  void Print(std::ostream &os) const override;

private:
  Mac48Address m_address;
  uint8_t m_ttl;
  uint32_t m_metric;
  uint32_t m_seqno;
};
} // namespace dot11s
} // namespace ns3
#endif
