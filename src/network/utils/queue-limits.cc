
#include "queue-limits.h"

#include "ns3/log.h"

namespace ns3 {

NS_LOG_COMPONENT_DEFINE("QueueLimits");

NS_OBJECT_ENSURE_REGISTERED(QueueLimits);

TypeId QueueLimits::GetTypeId() {
  static TypeId tid =
      TypeId("ns3::QueueLimits").SetParent<Object>().SetGroupName("Network");
  return tid;
}

QueueLimits::~QueueLimits() { NS_LOG_FUNCTION(this); }

} // namespace ns3
