
#ifndef NON_INHERITANCE_H
#define NON_INHERITANCE_H

#include "wifi-information-element.h"

#include <set>

namespace ns3 {

class NonInheritance : public WifiInformationElement {
public:
  WifiInformationElementId ElementId() const override;
  WifiInformationElementId ElementIdExt() const override;
  void Print(std::ostream &os) const override;

  void Add(uint8_t elemId, uint8_t elemIdExt = 0);

  bool IsPresent(uint8_t elemId, uint8_t elemIdExt = 0) const;

  std::set<uint8_t> m_elemIdList;
  std::set<uint8_t> m_elemIdExtList;

private:
  uint16_t GetInformationFieldSize() const override;
  void SerializeInformationField(Buffer::Iterator start) const override;
  uint16_t DeserializeInformationField(Buffer::Iterator start,
                                       uint16_t length) override;
};

} // namespace ns3

#endif
