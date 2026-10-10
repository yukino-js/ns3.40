
#ifndef ERP_INFORMATION_H
#define ERP_INFORMATION_H

#include "ns3/wifi-information-element.h"

namespace ns3 {

class ErpInformation : public WifiInformationElement {
public:
  ErpInformation();

  WifiInformationElementId ElementId() const override;
  uint16_t GetInformationFieldSize() const override;
  void SerializeInformationField(Buffer::Iterator start) const override;
  uint16_t DeserializeInformationField(Buffer::Iterator start,
                                       uint16_t length) override;

  void SetBarkerPreambleMode(uint8_t barkerPreambleMode);
  void SetUseProtection(uint8_t useProtection);
  void SetNonErpPresent(uint8_t nonErpPresent);

  uint8_t GetBarkerPreambleMode() const;
  uint8_t GetUseProtection() const;
  uint8_t GetNonErpPresent() const;

private:
  uint8_t m_erpInformation;
};

std::ostream &operator<<(std::ostream &os,
                         const ErpInformation &erpInformation);

} // namespace ns3

#endif
