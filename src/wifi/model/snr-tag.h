
#ifndef SNR_TAG_H
#define SNR_TAG_H

#include "ns3/tag.h"

namespace ns3 {

class Tag;

class SnrTag : public Tag {
public:
  static TypeId GetTypeId();

  SnrTag();

  TypeId GetInstanceTypeId() const override;
  uint32_t GetSerializedSize() const override;
  void Serialize(TagBuffer i) const override;
  void Deserialize(TagBuffer i) override;
  void Print(std::ostream &os) const override;

  void Set(double snr);
  double Get() const;

private:
  double m_snr;
};

} // namespace ns3

#endif
