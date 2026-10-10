#ifndef EPS_BEARER_TAG_H
#define EPS_BEARER_TAG_H

#include "ns3/tag.h"

namespace ns3 {

class Tag;

class EpsBearerTag : public Tag {
public:
  static TypeId GetTypeId();
  TypeId GetInstanceTypeId() const override;

  EpsBearerTag();

  EpsBearerTag(uint16_t rnti, uint8_t bid);

  void SetRnti(uint16_t rnti);

  void SetBid(uint8_t bid);

  void Serialize(TagBuffer i) const override;
  void Deserialize(TagBuffer i) override;
  uint32_t GetSerializedSize() const override;
  void Print(std::ostream &os) const override;

  uint16_t GetRnti() const;
  uint8_t GetBid() const;

private:
  uint16_t m_rnti;
  uint8_t m_bid;
};

} // namespace ns3

#endif
