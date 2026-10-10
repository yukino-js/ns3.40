#ifndef LTE_PHY_TAG_H
#define LTE_PHY_TAG_H

#include "ns3/tag.h"

namespace ns3 {

class LtePhyTag : public Tag {
public:
  static TypeId GetTypeId();
  TypeId GetInstanceTypeId() const override;

  LtePhyTag();

  LtePhyTag(uint16_t cellId);

  ~LtePhyTag() override;

  void Serialize(TagBuffer i) const override;
  void Deserialize(TagBuffer i) override;
  uint32_t GetSerializedSize() const override;
  void Print(std::ostream &os) const override;

  uint16_t GetCellId() const;

private:
  uint16_t m_cellId;
};

} // namespace ns3

#endif
