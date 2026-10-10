
#ifndef MESH_INFORMATION_ELEMENT_VECTOR_H
#define MESH_INFORMATION_ELEMENT_VECTOR_H

#include "ns3/wifi-information-element.h"

namespace ns3 {

#define IE11S_MESH_PEERING_PROTOCOL_VERSION ((WifiInformationElementId)74)

class MeshInformationElementVector : public Header {
public:
  MeshInformationElementVector();
  ~MeshInformationElementVector() override;

  static TypeId GetTypeId();

  TypeId GetInstanceTypeId() const override;
  uint32_t GetSerializedSize() const override;
  void Serialize(Buffer::Iterator start) const override;
  uint32_t Deserialize(Buffer::Iterator start) override;
  uint32_t Deserialize(Buffer::Iterator start, Buffer::Iterator end) override;
  void Print(std::ostream &os) const override;

  uint32_t DeserializeSingleIe(Buffer::Iterator start);

  typedef std::vector<Ptr<WifiInformationElement>>::iterator Iterator;
  Iterator Begin();
  Iterator End();
  bool AddInformationElement(Ptr<WifiInformationElement> element);
  Ptr<WifiInformationElement> FindFirst(WifiInformationElementId id) const;

  virtual bool operator==(const MeshInformationElementVector &a) const;

protected:
  typedef std::vector<Ptr<WifiInformationElement>> IE_VECTOR;
  uint32_t GetSize() const;
  IE_VECTOR m_elements;
  uint16_t m_maxSize;
};

} // namespace ns3

#endif
