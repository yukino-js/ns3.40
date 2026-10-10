#include "dsdv-packet.h"

#include "ns3/address-utils.h"
#include "ns3/packet.h"

namespace ns3 {
namespace dsdv {

NS_OBJECT_ENSURE_REGISTERED(DsdvHeader);

DsdvHeader::DsdvHeader(Ipv4Address dst, uint32_t hopCount, uint32_t dstSeqNo)
    : m_dst(dst), m_hopCount(hopCount), m_dstSeqNo(dstSeqNo) {}

DsdvHeader::~DsdvHeader() {}

TypeId DsdvHeader::GetTypeId() {
  static TypeId tid = TypeId("ns3::dsdv::DsdvHeader")
                          .SetParent<Header>()
                          .SetGroupName("Dsdv")
                          .AddConstructor<DsdvHeader>();
  return tid;
}

TypeId DsdvHeader::GetInstanceTypeId() const { return GetTypeId(); }

uint32_t DsdvHeader::GetSerializedSize() const { return 12; }

void DsdvHeader::Serialize(Buffer::Iterator i) const {
  WriteTo(i, m_dst);
  i.WriteHtonU32(m_hopCount);
  i.WriteHtonU32(m_dstSeqNo);
}

uint32_t DsdvHeader::Deserialize(Buffer::Iterator start) {
  Buffer::Iterator i = start;

  ReadFrom(i, m_dst);
  m_hopCount = i.ReadNtohU32();
  m_dstSeqNo = i.ReadNtohU32();

  uint32_t dist = i.GetDistanceFrom(start);
  NS_ASSERT(dist == GetSerializedSize());
  return dist;
}

void DsdvHeader::Print(std::ostream &os) const {
  os << "DestinationIpv4: " << m_dst << " Hopcount: " << m_hopCount
     << " SequenceNumber: " << m_dstSeqNo;
}
} // namespace dsdv
} // namespace ns3
