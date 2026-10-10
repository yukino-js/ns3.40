
#ifndef MESH_ID_H
#define MESH_ID_H

#include "ns3/buffer.h"
#include "ns3/mesh-information-element-vector.h"

#include <stdint.h>

namespace ns3 {
namespace dot11s {

class IeMeshId : public WifiInformationElement {
public:
  IeMeshId();
  IeMeshId(std::string s);

  bool IsEqual(const IeMeshId &o) const;
  bool IsBroadcast() const;
  char *PeekString() const;

  WifiInformationElementId ElementId() const override;
  void SerializeInformationField(Buffer::Iterator i) const override;
  uint16_t DeserializeInformationField(Buffer::Iterator start,
                                       uint16_t length) override;
  void Print(std::ostream &os) const override;
  uint16_t GetInformationFieldSize() const override;

private:
  uint8_t m_meshId[33];
  friend bool operator==(const IeMeshId &a, const IeMeshId &b);
};

std::ostream &operator<<(std::ostream &os, const IeMeshId &meshId);

} // namespace dot11s
} // namespace ns3
#endif
