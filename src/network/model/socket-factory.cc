#include "socket-factory.h"

#include "ns3/log.h"

namespace ns3 {

NS_LOG_COMPONENT_DEFINE("SocketFactory");

NS_OBJECT_ENSURE_REGISTERED(SocketFactory);

TypeId SocketFactory::GetTypeId() {
  static TypeId tid =
      TypeId("ns3::SocketFactory").SetParent<Object>().SetGroupName("Network");
  return tid;
}

SocketFactory::SocketFactory() { NS_LOG_FUNCTION(this); }

} // namespace ns3
