

#ifndef TIMESTAMP_TAG_H
#define TIMESTAMP_TAG_H

#include "ns3/nstime.h"
#include "ns3/tag-buffer.h"
#include "ns3/tag.h"
#include "ns3/type-id.h"

#include <iostream>

namespace ns3 {

class TimestampTag : public Tag {
public:
  static TypeId GetTypeId();
  TypeId GetInstanceTypeId() const override;

  TimestampTag();

  TimestampTag(Time timestamp);

  void Serialize(TagBuffer i) const override;
  void Deserialize(TagBuffer i) override;
  uint32_t GetSerializedSize() const override;
  void Print(std::ostream &os) const override;

  Time GetTimestamp() const;

  void SetTimestamp(Time timestamp);

private:
  Time m_timestamp{0};
};

} // namespace ns3

#endif
