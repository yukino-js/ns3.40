
#ifndef AMPDU_TAG_H
#define AMPDU_TAG_H

#include "ns3/nstime.h"
#include "ns3/tag.h"

namespace ns3 {

class AmpduTag : public Tag {
public:
  static TypeId GetTypeId();

  TypeId GetInstanceTypeId() const override;
  void Serialize(TagBuffer i) const override;
  void Deserialize(TagBuffer i) override;
  uint32_t GetSerializedSize() const override;
  void Print(std::ostream &os) const override;

  AmpduTag();
  void SetRemainingNbOfMpdus(uint8_t nbOfMpdus);
  void SetRemainingAmpduDuration(Time duration);

  uint8_t GetRemainingNbOfMpdus() const;
  Time GetRemainingAmpduDuration() const;

private:
  uint8_t m_nbOfMpdus;
  Time m_duration;
};

} // namespace ns3

#endif
