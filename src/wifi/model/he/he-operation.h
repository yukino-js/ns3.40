
#ifndef HE_OPERATION_H
#define HE_OPERATION_H

#include "ns3/wifi-information-element.h"

namespace ns3 {

class HeOperation : public WifiInformationElement {
public:
  HeOperation();

  WifiInformationElementId ElementId() const override;
  WifiInformationElementId ElementIdExt() const override;
  void Print(std::ostream &os) const override;

  void SetHeOperationParameters(uint32_t ctrl);
  void SetMaxHeMcsPerNss(uint8_t nss, uint8_t maxHeMcs);

  uint32_t GetHeOperationParameters() const;
  uint16_t GetBasicHeMcsAndNssSet() const;
  void SetBssColor(uint8_t bssColor);
  uint8_t GetBssColor() const;

private:
  uint16_t GetInformationFieldSize() const override;
  void SerializeInformationField(Buffer::Iterator start) const override;
  uint16_t DeserializeInformationField(Buffer::Iterator start,
                                       uint16_t length) override;

  uint8_t m_bssColor;
  uint8_t m_defaultPEDuration;
  uint8_t m_twtRequired;
  uint16_t m_heDurationBasedRtsThreshold;
  uint8_t m_partialBssColor;
  uint8_t m_maxBssidIndicator;
  uint8_t m_txBssidIndicator;
  uint8_t m_bssColorDisabled;
  uint8_t m_dualBeacon;

  uint16_t m_basicHeMcsAndNssSet;
};

} // namespace ns3

#endif
