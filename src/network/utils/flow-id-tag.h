#ifndef FLOW_ID_TAG_H
#define FLOW_ID_TAG_H

#include "ns3/tag.h"

namespace ns3 {

class FlowIdTag : public Tag {
public:
  static TypeId GetTypeId();
  TypeId GetInstanceTypeId() const override;
  uint32_t GetSerializedSize() const override;
  void Serialize(TagBuffer buf) const override;
  void Deserialize(TagBuffer buf) override;
  void Print(std::ostream &os) const override;
  FlowIdTag();

  FlowIdTag(uint32_t flowId);
  void SetFlowId(uint32_t flowId);
  uint32_t GetFlowId() const;
  static uint32_t AllocateFlowId();

private:
  uint32_t m_flowId;
};

} // namespace ns3

#endif
