
#ifndef AODV_DPD_H
#define AODV_DPD_H

#include "aodv-id-cache.h"

#include "ns3/ipv4-header.h"
#include "ns3/nstime.h"
#include "ns3/packet.h"

namespace ns3 {
namespace aodv {
class DuplicatePacketDetection {
public:
  DuplicatePacketDetection(Time lifetime) : m_idCache(lifetime) {}

  bool IsDuplicate(Ptr<const Packet> p, const Ipv4Header &header);
  void SetLifetime(Time lifetime);
  Time GetLifetime() const;

private:
  IdCache m_idCache;
};

} // namespace aodv
} // namespace ns3

#endif
