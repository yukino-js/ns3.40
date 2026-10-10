
#ifndef PERR_INFORMATION_ELEMENT_H
#define PERR_INFORMATION_ELEMENT_H

#include "hwmp-protocol.h"

#include "ns3/mac48-address.h"
#include "ns3/mesh-information-element-vector.h"

namespace ns3 {
namespace dot11s {
class IePerr : public WifiInformationElement {
public:
  IePerr();
  ~IePerr() override;
  uint8_t GetNumOfDest() const;
  void AddAddressUnit(HwmpProtocol::FailedDestination unit);
  bool IsFull() const;
  std::vector<HwmpProtocol::FailedDestination> GetAddressUnitVector() const;
  void DeleteAddressUnit(Mac48Address address);
  void ResetPerr();

  WifiInformationElementId ElementId() const override;
  void SerializeInformationField(Buffer::Iterator i) const override;
  uint16_t DeserializeInformationField(Buffer::Iterator start,
                                       uint16_t length) override;
  void Print(std::ostream &os) const override;
  uint16_t GetInformationFieldSize() const override;

private:
  std::vector<HwmpProtocol::FailedDestination> m_addressUnits;
  friend bool operator==(const IePerr &a, const IePerr &b);
};

bool operator==(const IePerr &a, const IePerr &b);
std::ostream &operator<<(std::ostream &os, const IePerr &perr);
} // namespace dot11s
} // namespace ns3
#endif
