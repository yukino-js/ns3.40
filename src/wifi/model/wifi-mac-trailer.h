
#ifndef WIFI_MAC_TRAILER_H
#define WIFI_MAC_TRAILER_H

#include "ns3/trailer.h"

namespace ns3 {

static const uint16_t WIFI_MAC_FCS_LENGTH = 4;

class WifiMacTrailer : public Trailer {
public:
  WifiMacTrailer();
  ~WifiMacTrailer() override;

  static TypeId GetTypeId();
  TypeId GetInstanceTypeId() const override;
  void Print(std::ostream &os) const override;
  uint32_t GetSerializedSize() const override;
  void Serialize(Buffer::Iterator start) const override;
  uint32_t Deserialize(Buffer::Iterator start) override;
};

} // namespace ns3

#endif
