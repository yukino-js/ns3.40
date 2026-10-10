#ifndef LR_WPAN_LQI_TAG_H
#define LR_WPAN_LQI_TAG_H

#include <ns3/tag.h>

namespace ns3 {

class LrWpanLqiTag : public Tag {
public:
  static TypeId GetTypeId();

  TypeId GetInstanceTypeId() const override;

  LrWpanLqiTag();

  LrWpanLqiTag(uint8_t lqi);

  uint32_t GetSerializedSize() const override;
  void Serialize(TagBuffer i) const override;
  void Deserialize(TagBuffer i) override;
  void Print(std::ostream &os) const override;

  void Set(uint8_t lqi);

  uint8_t Get() const;

private:
  uint8_t m_lqi;
};

} // namespace ns3
#endif
