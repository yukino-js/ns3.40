
#ifndef ETHERNET_TRAILER_H
#define ETHERNET_TRAILER_H

#include "ns3/packet.h"
#include "ns3/trailer.h"

#include <string>

namespace ns3 {

class EthernetTrailer : public Trailer {
public:
  EthernetTrailer();

  void EnableFcs(bool enable);

  void CalcFcs(Ptr<const Packet> p);

  void SetFcs(uint32_t fcs);

  uint32_t GetFcs() const;

  bool CheckFcs(Ptr<const Packet> p) const;

  uint32_t GetTrailerSize() const;

  static TypeId GetTypeId();
  TypeId GetInstanceTypeId() const override;
  void Print(std::ostream &os) const override;
  uint32_t GetSerializedSize() const override;
  void Serialize(Buffer::Iterator end) const override;
  uint32_t Deserialize(Buffer::Iterator end) override;

private:
  bool m_calcFcs;
  uint32_t m_fcs;
};

} // namespace ns3

#endif
