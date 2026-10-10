
#ifndef LTE_RLC_SDU_STATUS_TAG_H
#define LTE_RLC_SDU_STATUS_TAG_H

#include "ns3/tag.h"

namespace ns3 {

class LteRlcSduStatusTag : public Tag {
public:
  LteRlcSduStatusTag();

  void SetStatus(uint8_t status);
  uint8_t GetStatus() const;

  static TypeId GetTypeId();
  TypeId GetInstanceTypeId() const override;
  uint32_t GetSerializedSize() const override;
  void Serialize(TagBuffer i) const override;
  void Deserialize(TagBuffer i) override;
  void Print(std::ostream &os) const override;

  enum SduStatus_t {
    FULL_SDU = 1,
    FIRST_SEGMENT = 2,
    MIDDLE_SEGMENT = 3,
    LAST_SEGMENT = 4,
    ANY_SEGMENT = 5
  };

private:
  uint8_t m_sduStatus;
};

}; // namespace ns3

#endif
