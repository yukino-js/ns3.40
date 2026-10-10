
#ifndef IPV4_PACKET_FILTER_H
#define IPV4_PACKET_FILTER_H

#include "ns3/object.h"
#include "ns3/packet-filter.h"

namespace ns3 {

class Ipv4PacketFilter : public PacketFilter {
public:
  static TypeId GetTypeId();

  Ipv4PacketFilter();
  ~Ipv4PacketFilter() override;

private:
  bool CheckProtocol(Ptr<QueueDiscItem> item) const override;
  int32_t DoClassify(Ptr<QueueDiscItem> item) const override = 0;
};

} // namespace ns3

#endif
