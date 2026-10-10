
#ifndef LR_WPAN_MAC_TRAILER_H
#define LR_WPAN_MAC_TRAILER_H

#include <ns3/trailer.h>

namespace ns3 {

class Packet;

class LrWpanMacTrailer : public Trailer {
public:
  static TypeId GetTypeId();

  LrWpanMacTrailer();

  TypeId GetInstanceTypeId() const override;
  void Print(std::ostream &os) const override;
  uint32_t GetSerializedSize() const override;
  void Serialize(Buffer::Iterator start) const override;
  uint32_t Deserialize(Buffer::Iterator start) override;

  uint16_t GetFcs() const;

  void SetFcs(Ptr<const Packet> p);

  bool CheckFcs(Ptr<const Packet> p);

  void EnableFcs(bool enable);

  bool IsFcsEnabled() const;

private:
  uint16_t GenerateCrc16(uint8_t *data, int length);

  uint16_t m_fcs;

  bool m_calcFcs;
};

} // namespace ns3

#endif
