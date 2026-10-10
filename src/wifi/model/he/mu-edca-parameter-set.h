
#ifndef MU_EDCA_PARAMETER_SET_H
#define MU_EDCA_PARAMETER_SET_H

#include "ns3/nstime.h"
#include "ns3/wifi-information-element.h"

#include <array>

namespace ns3 {

class MuEdcaParameterSet : public WifiInformationElement {
public:
  MuEdcaParameterSet();

  WifiInformationElementId ElementId() const override;
  WifiInformationElementId ElementIdExt() const override;

  void SetQosInfo(uint8_t qosInfo);
  void SetMuAifsn(uint8_t aci, uint8_t aifsn);
  void SetMuCwMin(uint8_t aci, uint16_t cwMin);
  void SetMuCwMax(uint8_t aci, uint16_t cwMax);
  void SetMuEdcaTimer(uint8_t aci, Time timer);

  uint8_t GetQosInfo() const;
  uint8_t GetMuAifsn(uint8_t aci) const;
  uint16_t GetMuCwMin(uint8_t aci) const;
  uint16_t GetMuCwMax(uint8_t aci) const;
  Time GetMuEdcaTimer(uint8_t aci) const;

private:
  uint16_t GetInformationFieldSize() const override;
  void SerializeInformationField(Buffer::Iterator start) const override;
  uint16_t DeserializeInformationField(Buffer::Iterator start,
                                       uint16_t length) override;

  struct ParameterRecord {
    uint8_t aifsnField;
    uint8_t cwMinMax;
    uint8_t muEdcaTimer;
  };

  uint8_t m_qosInfo;
  std::array<ParameterRecord, 4> m_records;
};

} // namespace ns3

#endif
