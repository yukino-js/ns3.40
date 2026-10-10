
#ifndef MESH_WIFI_MAC_HEADER_H
#define MESH_WIFI_MAC_HEADER_H

#include "ns3/header.h"
#include "ns3/mac48-address.h"

namespace ns3 {
namespace dot11s {
class MeshHeader : public Header {
public:
  MeshHeader();
  ~MeshHeader() override;
  static TypeId GetTypeId();
  TypeId GetInstanceTypeId() const override;
  void Print(std::ostream &os) const override;

  void SetAddr4(Mac48Address address);
  void SetAddr5(Mac48Address address);
  void SetAddr6(Mac48Address address);
  Mac48Address GetAddr4() const;
  Mac48Address GetAddr5() const;
  Mac48Address GetAddr6() const;

  void SetMeshSeqno(uint32_t seqno);
  uint32_t GetMeshSeqno() const;

  void SetMeshTtl(uint8_t TTL);
  uint8_t GetMeshTtl() const;

  void SetAddressExt(uint8_t num_of_addresses);
  uint8_t GetAddressExt() const;

  uint32_t GetSerializedSize() const override;
  void Serialize(Buffer::Iterator start) const override;
  uint32_t Deserialize(Buffer::Iterator start) override;

private:
  uint8_t m_meshFlags;
  uint8_t m_meshTtl;
  uint32_t m_meshSeqno;
  Mac48Address m_addr4;
  Mac48Address m_addr5;
  Mac48Address m_addr6;
  friend bool operator==(const MeshHeader &a, const MeshHeader &b);
};

bool operator==(const MeshHeader &a, const MeshHeader &b);

} // namespace dot11s
} // namespace ns3
#endif
