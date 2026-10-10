#include "udp-socket-factory.h"

#include "ns3/uinteger.h"

namespace ns3 {

NS_OBJECT_ENSURE_REGISTERED(UdpSocketFactory);

TypeId UdpSocketFactory::GetTypeId() {
  static TypeId tid = TypeId("ns3::UdpSocketFactory")
                          .SetParent<SocketFactory>()
                          .SetGroupName("Internet");
  return tid;
}

} // namespace ns3
