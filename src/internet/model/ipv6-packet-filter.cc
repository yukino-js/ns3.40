
#include "ipv6-packet-filter.h"

#include "ipv6-queue-disc-item.h"

#include "ns3/enum.h"
#include "ns3/log.h"
#include "ns3/uinteger.h"

namespace ns3 {

NS_LOG_COMPONENT_DEFINE("Ipv6PacketFilter");

NS_OBJECT_ENSURE_REGISTERED(Ipv6PacketFilter);

TypeId Ipv6PacketFilter::GetTypeId() {
  static TypeId tid = TypeId("ns3::Ipv6PacketFilter")
                          .SetParent<PacketFilter>()
                          .SetGroupName("Internet");
  return tid;
}

Ipv6PacketFilter::Ipv6PacketFilter() { NS_LOG_FUNCTION(this); }

Ipv6PacketFilter::~Ipv6PacketFilter() { NS_LOG_FUNCTION(this); }

bool Ipv6PacketFilter::CheckProtocol(Ptr<QueueDiscItem> item) const {
  NS_LOG_FUNCTION(this << item);
  return bool(DynamicCast<Ipv6QueueDiscItem>(item));
}

} // namespace ns3
