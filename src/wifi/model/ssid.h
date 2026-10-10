
#ifndef SSID_H
#define SSID_H

#include "wifi-information-element.h"

namespace ns3 {

class Ssid : public WifiInformationElement {
public:
  Ssid();
  Ssid(std::string s);

  WifiInformationElementId ElementId() const override;
  void Print(std::ostream &os) const override;

  bool IsEqual(const Ssid &o) const;
  bool IsBroadcast() const;

  char *PeekString() const;

private:
  uint16_t GetInformationFieldSize() const override;
  void SerializeInformationField(Buffer::Iterator start) const override;
  uint16_t DeserializeInformationField(Buffer::Iterator start,
                                       uint16_t length) override;

  uint8_t m_ssid[33];
  uint8_t m_length;
};

std::istream &operator>>(std::istream &is, Ssid &ssid);

ATTRIBUTE_HELPER_HEADER(Ssid);

} // namespace ns3

#endif
