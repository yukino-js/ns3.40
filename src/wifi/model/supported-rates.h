
#ifndef SUPPORTED_RATES_H
#define SUPPORTED_RATES_H

#include "wifi-information-element.h"

#include <optional>
#include <vector>

namespace ns3 {

class SupportedRates : public WifiInformationElement {
  friend struct AllSupportedRates;

public:
  SupportedRates();

  WifiInformationElementId ElementId() const override;
  void Print(std::ostream &os) const override;

  uint32_t GetRate(uint8_t i) const;

protected:
  uint16_t GetInformationFieldSize() const override;
  void SerializeInformationField(Buffer::Iterator start) const override;
  uint16_t DeserializeInformationField(Buffer::Iterator start,
                                       uint16_t length) override;

  std::vector<uint8_t> m_rates;
};

class ExtendedSupportedRatesIE : public SupportedRates {
public:
  WifiInformationElementId ElementId() const override;
};

struct AllSupportedRates {
  void AddSupportedRate(uint64_t bs);
  void SetBasicRate(uint64_t bs);
  void AddBssMembershipSelectorRate(uint64_t bs);
  bool IsSupportedRate(uint64_t bs) const;
  bool IsBasicRate(uint64_t bs) const;
  bool IsBssMembershipSelectorRate(uint64_t bs) const;
  uint8_t GetNRates() const;

  SupportedRates rates;
  std::optional<ExtendedSupportedRatesIE> extendedRates;
};

} // namespace ns3

#endif
