
#ifndef MESH_PEERING_PROTOCOL_H
#define MESH_PEERING_PROTOCOL_H

#include "ns3/mesh-information-element-vector.h"

namespace ns3 {
namespace dot11s {

class IePeeringProtocol : public WifiInformationElement {
public:
  IePeeringProtocol();

  WifiInformationElementId ElementId() const override;
  uint16_t GetInformationFieldSize() const override;
  void SerializeInformationField(Buffer::Iterator i) const override;
  uint16_t DeserializeInformationField(Buffer::Iterator i,
                                       uint16_t length) override;
  void Print(std::ostream &os) const override;

private:
  uint8_t m_protocol;
};
} // namespace dot11s
} // namespace ns3
#endif
