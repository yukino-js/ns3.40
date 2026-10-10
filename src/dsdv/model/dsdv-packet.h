
#ifndef DSDV_PACKET_H
#define DSDV_PACKET_H

#include "ns3/header.h"
#include "ns3/ipv4-address.h"
#include "ns3/nstime.h"

#include <iostream>

namespace ns3 {
namespace dsdv {

class DsdvHeader : public Header {
public:
  DsdvHeader(Ipv4Address dst = Ipv4Address(), uint32_t hopcount = 0,
             uint32_t dstSeqNo = 0);
  ~DsdvHeader() override;
  static TypeId GetTypeId();
  TypeId GetInstanceTypeId() const override;
  uint32_t GetSerializedSize() const override;
  void Serialize(Buffer::Iterator start) const override;
  uint32_t Deserialize(Buffer::Iterator start) override;
  void Print(std::ostream &os) const override;

  void SetDst(Ipv4Address destination) { m_dst = destination; }

  Ipv4Address GetDst() const { return m_dst; }

  void SetHopCount(uint32_t hopCount) { m_hopCount = hopCount; }

  uint32_t GetHopCount() const { return m_hopCount; }

  void SetDstSeqno(uint32_t sequenceNumber) { m_dstSeqNo = sequenceNumber; }

  uint32_t GetDstSeqno() const { return m_dstSeqNo; }

private:
  Ipv4Address m_dst;
  uint32_t m_hopCount;
  uint32_t m_dstSeqNo;
};

static inline std::ostream &operator<<(std::ostream &os,
                                       const DsdvHeader &packet) {
  packet.Print(os);
  return os;
}
} // namespace dsdv
} // namespace ns3

#endif
