
#ifndef MU_SNR_TAG_H
#define MU_SNR_TAG_H

#include "ns3/tag.h"

#include <map>

namespace ns3 {

class MuSnrTag : public Tag {
public:
  static TypeId GetTypeId();
  TypeId GetInstanceTypeId() const override;

  MuSnrTag();

  uint32_t GetSerializedSize() const override;
  void Serialize(TagBuffer i) const override;
  void Deserialize(TagBuffer i) override;
  void Print(std::ostream &os) const override;

  void Reset();
  void Set(uint16_t staId, double snr);
  bool IsPresent(uint16_t staId) const;
  double Get(uint16_t staId) const;

private:
  std::map<uint16_t, double> m_snrMap;
};

} // namespace ns3

#endif
