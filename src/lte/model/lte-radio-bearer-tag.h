#ifndef LTE_RADIO_BEARER_TAG_H
#define LTE_RADIO_BEARER_TAG_H

#include "ns3/tag.h"

namespace ns3 {

class Tag;

class LteRadioBearerTag : public Tag {
public:
  static TypeId GetTypeId();
  TypeId GetInstanceTypeId() const override;

  LteRadioBearerTag();

  LteRadioBearerTag(uint16_t rnti, uint8_t lcId);

  LteRadioBearerTag(uint16_t rnti, uint8_t lcId, uint8_t layer);

  void SetRnti(uint16_t rnti);

  void SetLcid(uint8_t lcid);

  void SetLayer(uint8_t layer);

  void Serialize(TagBuffer i) const override;
  void Deserialize(TagBuffer i) override;
  uint32_t GetSerializedSize() const override;
  void Print(std::ostream &os) const override;

  uint16_t GetRnti() const;
  uint8_t GetLcid() const;
  uint8_t GetLayer() const;

private:
  uint16_t m_rnti;
  uint8_t m_lcid;
  uint8_t m_layer;
};

} // namespace ns3

#endif
